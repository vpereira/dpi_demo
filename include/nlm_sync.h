/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef __NLM_SYNC_H
#define __NLM_SYNC_H

/* ==================================================
   SYNCHRONIZATION PRIMITIVES
   ================================================== */
typedef struct spinlock_t
{
  union
  {
    char pad[64];
    uint32_t lock;
  } u;
} spinlock_t;

typedef struct atomic_t
{ 
  volatile uint32_t counter;
} atomic_t;

#if defined(__KERNEL__) && defined(__arm__)

/* Minimum effort to make arm kernel demo work.
 * gcc intrinsics __sync_add_and_fetch are defined
 * as an external function in the library. These functions
 * are not satisfied at the kernel mode.
 */
#define NLM_SPIN_LOCK(lockm)
#define NLM_SPIN_UNLOCK(lockm)
#define MEMORY_BARRIER()
#define NLM_ATOMIC_ADD32_RETURN(addr, value) \
    ((addr)->counter = (addr)->counter + value, (addr)->counter)
#define NLM_ATOMIC_ADD32(addr, value) \
    ((addr)->counter = (addr)->counter + value, (addr)->counter - value)
#define NLM_ATOMIC_AND_RETURN(addr, value) \
    ((addr)->counter = (addr)->counter & value, (addr)->counter)
#define NLM_ATOMIC_OR_RETURN(addr, value) \
    ((addr)->counter = (addr)->counter | value, (addr)->counter)

#elif defined(__i386) || defined (__amd64) || defined (__arm__)

/* The following use the gcc's builtin atomic primitives.
   Replace if being used with anything else */
#define NLM_SPIN_LOCK(lockm)\
  do\
    {\
      while (__sync_lock_test_and_set(&((lockm)->u.lock), 1) != 0);\
    }\
  while (0)
  
#define NLM_SPIN_UNLOCK(lockm) __sync_lock_release(&((lockm)->u.lock))

#define MEMORY_BARRIER()

#define NLM_ATOMIC_ADD32_RETURN(addr, value)  __sync_add_and_fetch (&((addr)->counter), value)
#define NLM_ATOMIC_ADD32(addr, value) __sync_fetch_and_add (&((addr)->counter), value)

#define NLM_ATOMIC_AND_RETURN(addr, value) __sync_and_and_fetch (&((addr)->counter), value)
#define NLM_ATOMIC_AND(addr, value) __sync_fetch_and_and (&((addr)->counter), value)

#define NLM_ATOMIC_OR_RETURN(addr, value) __sync_or_and_fetch (&((addr)->counter), value)
#define NLM_ATOMIC_OR(addr, value) __sync_fetch_and_or (&((addr)->counter), value)

#elif defined (_MIPS_ARCH_XLR) || defined (_MIPS_ARCH_OCTEON)

static __inline__ 
void NLM_SPIN_LOCK (spinlock_t *lock)
{
  unsigned int tmp;

  __asm__ __volatile__("      .set    noreorder # NLM_SPIN_LOCK         \n"
                       "1:    ll      %1, %2                            \n"
                       "      bnez    %1, 1b                            \n"
                       "      li      %1,  1                            \n"
                       "      sc      %1, %0                            \n"
                       "      beqz    %1, 1b                            \n"
                       "      sync                                      \n"
                       "      .set    reorder                             "
                       : "=m" (lock->u.lock), "=&r" (tmp)
                       : "m" (lock->u.lock)
                       : "memory");
}

static __inline__ 
void NLM_SPIN_UNLOCK (spinlock_t *lock)
{
  __asm__ __volatile__("      .set    noreorder #NLM_SPIN_UNLOCK         \n"
                       "      sync                                       \n"
                       "      sw      $0, %0                             \n"
                       "      .set    reorder                              "
                       : "=m" (lock->u.lock)
                       : "m" (lock->u.lock)
                       : "memory");
}

static __inline__
int32_t NLM_ATOMIC_ADD32_RETURN (atomic_t *v, int32_t value)
{
  int32_t temp, result;
  
  __asm__ __volatile__("1:     ll      %1, %2   #NLM_ATOMIC_ADD32_RETURN \n"
                       "       addu    %0, %1, %3                        \n"
                       "       sc      %0, %2                            \n"
                       "       beqz    %0, 2f                            \n"
                       "       addu    %0, %1, %3                        \n"
                       "       .subsection 2                             \n"
                       "2:     b       1b                                \n"
                       "       .previous                                 \n"
                       "       .set    mips0                               "
                       : "=&r" (result), "=&r" (temp), "=m" (v->counter)
                       : "Ir" (value), "m" (v->counter)
                       : "memory");
  return result;
}

static __inline__ 
void NLM_ATOMIC_ADD32 (atomic_t *v, int32_t value)
{
  int32_t temp;

  __asm__ __volatile__("1:     ll      %0, %1    #NLM_ATOMIC_ADD32      \n"
                       "       addu    %0, %2                           \n"
                       "       sc      %0, %1                           \n"
                       "       beqz    %0, 1b                             "
                       : "=&r" (temp), "=m" (v->counter)
                       : "Ir" (value), "m" (v->counter));
}

static __inline__
int32_t NLM_ATOMIC_AND_RETURN (atomic_t *v, int32_t value)
{
  int32_t temp, result;
  
  __asm__ __volatile__("1:     ll      %1, %2   #NLM_ATOMIC_AND_RETURN \n"
                       "       and     %0, %1, %3                        \n"
                       "       sc      %0, %2                            \n"
                       "       beqz    %0, 2f                            \n"
                       "       and     %0, %1, %3                        \n"
                       "       .subsection 2                             \n"
                       "2:     b       1b                                \n"
                       "       .previous                                 \n"
                       "       .set    mips0                               "
                       : "=&r" (result), "=&r" (temp), "=m" (v->counter)
                       : "Ir" (value), "m" (v->counter)
                       : "memory");
  return result;
}

static __inline__
void NLM_ATOMIC_AND (atomic_t *v, int32_t value)
{
  int32_t temp;

  __asm__ __volatile__("1:     ll      %0, %1    #NLM_ATOMIC_ADD32      \n"
                       "       and     %0, %2                           \n"
                       "       sc      %0, %1                           \n"
                       "       beqz    %0, 1b                             "
                       : "=&r" (temp), "=m" (v->counter)
                       : "Ir" (value), "m" (v->counter));
}

static __inline__
void NLM_ATOMIC_OR (atomic_t *v, int32_t value)
{
  int32_t temp;
  
  __asm__ __volatile__("1:     ll      %0, %1    #NLM_ATOMIC_ADD32      \n"
                       "       or      %0, %2                           \n"
                       "       sc      %0, %1                           \n"
                       "       beqz    %0, 1b                             "
                       : "=&r" (temp), "=m" (v->counter)
                       : "Ir" (value), "m" (v->counter));
}

static __inline__
int32_t NLM_ATOMIC_OR_RETURN (atomic_t *v, int32_t value)
{
  int32_t temp, result;
  
  __asm__ __volatile__("1:     ll      %1, %2   #NLM_ATOMIC_AND_RETURN \n"
                       "       or      %0, %1, %3                        \n"
                       "       sc      %0, %2                            \n"
                       "       beqz    %0, 2f                            \n"
                       "       or      %0, %1, %3                        \n"
                       "       .subsection 2                             \n"
                       "2:     b       1b                                \n"
                       "       .previous                                 \n"
                       "       .set    mips0                               "
                       : "=&r" (result), "=&r" (temp), "=m" (v->counter)
                       : "Ir" (value), "m" (v->counter)
                       : "memory");
  return result;
}

#endif /* XLR OCTEON */

#if defined(_MIPS_ARCH_XLR)

#define MEMORY_BARRIER()  __asm__ __volatile__("sync": : :"memory")

#elif defined(_MIPS_ARCH_OCTEON)

static __inline__ 
void MEMORY_BARRIER (void)
{
  __asm__ __volatile__ ("      .set    push                             \n"
                        "      .set    arch=octeon                      \n"
                        "      syncw                                    \n"
                        "      .set    pop                                ");
}
#endif /* XLR OCTEON */

#endif /* __NLM_SYNC_H */





