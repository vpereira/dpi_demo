/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef __NLM_STDINT_H
#define __NLM_STDINT_H

#if defined (__linux__)
#ifdef __KERNEL__
#  include <linux/types.h>
#else
#  include <inttypes.h>
#endif
#elif defined (__unix__)
# include <inttypes.h>
#else
# if !defined (__int8_t_defined)
#  define __int8_t_defined
typedef signed char             int8_t;
typedef short int               int16_t;
typedef int                     int32_t;
typedef long long int           int64_t;
# endif

typedef unsigned char           uint8_t;
typedef unsigned short int      uint16_t;
# if !defined (__uint32_t_defined)
#  define __uint32_t_defined
typedef unsigned int            uint32_t;
# endif
typedef unsigned long long int  uint64_t;
#endif 

#if defined (__x86_64__) || defined (_LP64) || defined (__LP64__)
/* 64-bit */
typedef unsigned long nlm_uintptr_t;
typedef long nlm_intptr_t;
#define NLM_PRIdPTR "ld"
#define NLM_PRIxPTR "lx"
#define NLM_PRId32 "d"
#define NLM_PRIu32 "u"
#define NLM_PRIx32 "x"
#define NLM_PRId64 "ld"
#define NLM_PRIu64 "lu"
#define NLM_PRIx64 "lx"
#define NLM_PRIxPHYS "llx"
#else
/* 32-bit */
typedef unsigned int nlm_uintptr_t;
typedef int nlm_intptr_t;
#define NLM_PRIdPTR "d"
#define NLM_PRIxPTR "x"
#define NLM_PRId32 "d"
#define NLM_PRIu32 "u"
#define NLM_PRIx32 "x"
#define NLM_PRId64 "lld"
#define NLM_PRIu64 "llu"
#define NLM_PRIx64 "llx"
#define NLM_PRIxPHYS "llx"
#endif

#endif /* __NLM_STDINT_H */

