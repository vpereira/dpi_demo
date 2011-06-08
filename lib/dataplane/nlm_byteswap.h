/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef NLM_BYTESWAP_H
#define NLM_BYTESWAP_H

#ifdef __mips

#define __byteswap_constant_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |     \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define __byteswap_constant_64(x) \
     ((((x) & 0xff00000000000000ull) >> 56)  | (((x) & 0x00ff000000000000ull) >>  40) |     \
      (((x) & 0x0000ff0000000000ull) >> 24)  | (((x) & 0x000000ff00000000ull) >>   8) |     \
      (((x) & 0x00000000ff000000ull) <<  8)  | (((x) & 0x0000000000ff0000ull) <<  24) |     \
      (((x) & 0x000000000000ff00ull) << 40)  | (((x) & 0x00000000000000ffull) <<  56))

#ifdef __GNUC__
#define __byteswap32(x) \
     (__extension__                   \
      ({ register unsigned int __x = (x); __byteswap_constant_32 (__x); }))

#define __byteswap64(x) \
     (__extension__                   \
      ({ register uint64_t __x = (x); __byteswap_constant_64 (__x); }))
#else
static __inline unsigned int
__byteswap32 (unsigned int __bsx)
{
  return __byteswap_constant_32 (__bsx);
}

static __inline uint64_t
__byteswap64 (uint64_t __bsx)
{
  return __byteswap_constant_64 (__bsx);
}
#endif /* __GNUC__ */

/* big endian */
#define SWAP32 __byteswap32
#define SWAP64 __byteswap64

/* gcc is packing bits differently on big and little endian boxes */
#define __ENDIAN_ORDER14(A,B,C,D,E,F,G,H,I,J,K,L,M,N) A;B;C;D;E;F;G;H;I;J;K;L;M;N;
#define __ENDIAN_ORDER13(A,B,C,D,E,F,G,H,I,J,K,L,M) A;B;C;D;E;F;G;H;I;J;K;L;M;
#define __ENDIAN_ORDER12(A,B,C,D,E,F,G,H,I,J,K,L) A;B;C;D;E;F;G;H;I;J;K;L;
#define __ENDIAN_ORDER11(A,B,C,D,E,F,G,H,I,J,K) A;B;C;D;E;F;G;H;I;J;K;
#define __ENDIAN_ORDER10(A,B,C,D,E,F,G,H,I,J) A;B;C;D;E;F;G;H;I;J;
#define __ENDIAN_ORDER9(A,B,C,D,E,F,G,H,I) A;B;C;D;E;F;G;H;I;
#define __ENDIAN_ORDER8(A,B,C,D,E,F,G,H) A;B;C;D;E;F;G;H;
#define __ENDIAN_ORDER7(A,B,C,D,E,F,G) A;B;C;D;E;F;G;
#define __ENDIAN_ORDER6(A,B,C,D,E,F) A;B;C;D;E;F;
#define __ENDIAN_ORDER5(A,B,C,D,E) A;B;C;D;E;
#define __ENDIAN_ORDER4(A,B,C,D) A;B;C;D;
#define __ENDIAN_ORDER3(A,B,C) A;B;C;
#define __ENDIAN_ORDER2(A,B) A;B;
#else
/* little endian */
#define SWAP32 /**/
#define SWAP64 /**/
#define __ENDIAN_ORDER14(A,B,C,D,E,F,G,H,I,J,K,L,M,N) N;M;L;K;J;I;H;G;F;E;D;C;B;A;
#define __ENDIAN_ORDER13(A,B,C,D,E,F,G,H,I,J,K,L,M) M;L;K;J;I;H;G;F;E;D;C;B;A;
#define __ENDIAN_ORDER12(A,B,C,D,E,F,G,H,I,J,K,L) L;K;J;I;H;G;F;E;D;C;B;A;
#define __ENDIAN_ORDER11(A,B,C,D,E,F,G,H,I,J,K) K;J;I;H;G;F;E;D;C;B;A;
#define __ENDIAN_ORDER10(A,B,C,D,E,F,G,H,I,J) J;I;H;G;F;E;D;C;B;A;
#define __ENDIAN_ORDER9(A,B,C,D,E,F,G,H,I) I;H;G;F;E;D;C;B;A;
#define __ENDIAN_ORDER8(A,B,C,D,E,F,G,H) H;G;F;E;D;C;B;A;
#define __ENDIAN_ORDER7(A,B,C,D,E,F,G) G;F;E;D;C;B;A;
#define __ENDIAN_ORDER6(A,B,C,D,E,F) F;E;D;C;B;A;
#define __ENDIAN_ORDER5(A,B,C,D,E) E;D;C;B;A;
#define __ENDIAN_ORDER4(A,B,C,D) D;C;B;A;
#define __ENDIAN_ORDER3(A,B,C) C;B;A;
#define __ENDIAN_ORDER2(A,B) B;A;
#endif /* __mips */

#endif /* NLM_BYTESWAP_H */
