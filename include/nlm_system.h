/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef __NLM_SYSTEM_H
#define __NLM_SYSTEM_H

#include "nlm_logger.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

extern void pretty_abort (const char *, const char *, int, const char *);
extern void warning (const char *);
extern void * default_xmalloc (void *cookie, unsigned int byte_cnt);
extern void default_xfree (void *cookie, void *mem_ptr);

typedef struct nlm_allocator
{
  void *cookie;
  void * (*xmalloc) (void *cookie, unsigned int byte_cnt);
  void (*xfree) (void *cookie, void *mem_ptr);
} nlm_allocator;

extern nlm_allocator default_allocator;

#ifdef __KERNEL__
# include <linux/kernel.h>
# ifdef DEBUG
#  define DPRINT(STR, ARGS...) printk ("%s():" STR, __FUNCTION__, ##ARGS)
#  define DDPRINT(STR, ARGS...) printk (STR, ##ARGS)
# else
#  define DPRINT(STR, ...) ((void)(0 && (STR)))
#  define DDPRINT(STR, ...) ((void)(0 && (STR)))
# endif
#else
# ifdef DEBUG
#  include <stdio.h>
#  ifdef __GNUC__
#   define DPRINT(STR, ARGS...) printf ("%s():" STR, __FUNCTION__, ##ARGS)
#   define DDPRINT(STR, ARGS...) printf (STR, ##ARGS)
#  else
#   define DPRINT(STR, ...) printf (":"STR, __VA_ARGS__)
#   define DDPRINT(STR, ...) printf (STR, __VA_ARGS__)
#  endif
# else
#  include <stdio.h>
#  ifdef __GNUC__
#   define DPRINT(STR, ARGS...) ((void) (0 && printf ("%s():" STR, __FUNCTION__, ##ARGS)))
#   define DDPRINT(STR, ARGS...) ((void) (0 && printf (STR, ##ARGS)))
#  else
#   define DPRINT(STR, ...) ((void)(0 && (STR)))
#   define DDPRINT(STR, ...) ((void)(0 && (STR)))
#  endif
# endif
#endif

#ifndef NDEBUG

#ifdef __GNUC__
#define pretty_assert(EXP) \
  ((void)(!(EXP) ? pretty_abort ("assert("#EXP")", __FILE__, __LINE__, __FUNCTION__), 0 : 0))
#else
#define pretty_assert(EXP) \
  ((void)(!(EXP) ? pretty_abort ("assert", __FILE__, __LINE__, ""), 0 : 0))
#endif

#else
#define pretty_assert(EXP) ((void)(0 && (EXP)))
#endif

#ifdef __GNUC__
#define abort() pretty_abort ("abort", __FILE__, __LINE__, __FUNCTION__)
#else
#define abort() pretty_abort ("abort", __FILE__, __LINE__, "")
#endif


#undef MAX
#define MAX(A, B) ((A)>(B)?(A):(B))

#undef MIN
#define MIN(A, B) ((A)<(B)?(A):(B))

#define DEF_BITMAP(N) typedef unsigned char BITMAP_##N[N/8]
#define BITMAP(N) BITMAP_##N
#define SET_BIT(BITMAP, BITNO) (BITMAP[BITNO/8] |= 1 << (BITNO) % 8)
#define TEST_BIT(BITMAP, BITNO) (BITMAP[BITNO/8] >> (BITNO) % 8 & 1)
#define CLEAR_BIT(BITMAP, BITNO) (BITMAP[BITNO/8] &= ~(1 << (BITNO) % 8))
#define CLEAR_BITMAP(TYPE, BITMAP) do{memset (BITMAP, 0, sizeof (TYPE));}while(0)
#define SET_ALL_BITMAP_BITS(TYPE, BITMAP) do{memset (BITMAP, 0xff, sizeof (TYPE));}while(0)

#define OR_BITMAP(TYPE, BMP_DST, BMP_SRC) \
        do {int i; for (i = 0; i < sizeof (TYPE); i++) \
            BMP_DST[i] |= BMP_SRC[i]; } while (0)

#define AND_BITMAP(TYPE, BMP_DST, BMP_SRC) \
        do {int i; for (i = 0; i < sizeof (TYPE); i++) \
            BMP_DST[i] &= BMP_SRC[i]; } while (0)
             
#define COPY_BITMAP(TYPE, BMP_DST, BMP_SRC) \
        do {int i; for (i = 0; i < sizeof (TYPE); i++) \
            BMP_DST[i] = BMP_SRC[i]; } while (0)

#define NEGATE_BITMAP(TYPE, BITMAP) \
        do {int i; for (i = 0; i < sizeof (TYPE); i++) \
            BITMAP[i] = ~ BITMAP[i]; } while (0)

#define COMPARE_BITMAP(TYPE, MAPL, MAPR) \
        memcmp (MAPL, MAPR, sizeof (TYPE)) == 0

#define COUNT_BITMAP_BITS(TYPE, BITMAP, RESULT) \
  do \
    { \
      int i;  \
      unsigned char c; \
      RESULT = 0; \
      for (i = 0; i < sizeof (TYPE); i++) \
        { \
          c = BITMAP[i]; \
          while (c) \
          { \
            RESULT++; \
            c &= c -1; \
          } \
        } \
      } \
    while (0);
            
#define TRY(A) \
  { \
    nlm_status __tmp_status = A; \
    if (__tmp_status != NLM_OK) \
      { \
        DDPRINT (#A" failed: %s\n", nlm_get_status_string (__tmp_status)); \
        return __tmp_status; \
      } \
  }

#define CHECK_NULL(ADDR) do { if ((ADDR) == NULL) return NLM_OUT_OF_MEMORY; } while (0)

#endif /* __NLM_SYSTEM_H */

