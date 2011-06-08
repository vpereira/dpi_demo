/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include "nlm_sha2.h"

#define ror(a, n)  (((a) >> n) | ((a) << (32 - n)))
#define E0(a)  (ror(a, 2) ^ ror(a, 13) ^ ror(a, 22))
#define E1(a)  (ror(a, 6) ^ ror(a, 11) ^ ror(a, 25))
#define e0(a)  (ror(a, 7) ^ ror(a, 18) ^ ((a) >> 3))
#define e1(a)  (ror(a, 17) ^ ror(a, 19) ^ ((a) >> 10))
#define c(x, y, z)  ((z) ^ ((x) & ((y) ^ (z))))
#define m(x, y, z)  (((x) & (y)) ^ ((z) & ((x) ^ (y))))

static const uint32_t hac[8] = {  0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                  0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

static const uint32_t sha256_seed[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

static uint32_t cnt;
static uint32_t total_size;
static uint32_t ha[8];
static uint8_t  pad[64];

void
nlm_sha256_xform (void)
{
  uint32_t i;
  uint8_t  *buf = pad;
  uint32_t tar[8], t[2], w[64];
 
  for (i = 0; i < 0x10; i++, buf += 4)
    w[i] = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
 
  for (i = 16; i < 0x40; i++)
    w[i] = e1(w[i - 2]) + w[i - 7]
  + e0(w[i - 15]) + w[i - 16];

  for (i = 0; i < 8; i++)
    tar[i] = ha[i];
 
  for (i = 0; i < 0x40; i++)
    {
      t[0]   = tar[7] + E1(tar[4]) + c(tar[4], tar[5], tar[6]) + sha256_seed[i] + w[i];
      t[1]   = E0(tar[0]) + m(tar[0], tar[1], tar[2]);
      tar[7] = tar[6];
      tar[6] = tar[5];
      tar[5] = tar[4];
      tar[4] = tar[3] + t[0];
      tar[3] = tar[2];
      tar[2] = tar[1];
      tar[1] = tar[0];
      tar[0] = t[0] + t[1];
    }
 
  for (i = 0; i < 8; i++)
    ha[i] += tar[i];
}
 
void
nlm_chksum_sha256_reset (void)
{
  uint32_t i;
  cnt = 0;
  total_size = 0;
  for (i = 0; i < 8; i++)
    ha[i] = hac[i];
}

uint32_t
nlm_chksum_sha256_add_byte (uint8_t byte)
{
  cnt &= 0x3f;
  pad[cnt++] = byte;
  total_size++;
  return cnt;
}

void
nlm_chksum_sha256_post (nlm_chksum *chksum)
{
  uint32_t i;

  for (nlm_chksum_sha256_add_byte (0x80); (0x40 - sizeof (uint32_t)) > nlm_chksum_sha256_add_byte (0););

  i = 0x20;
  do
    {
      i -= 8;
      nlm_chksum_sha256_add_byte ((total_size << 3) >> i);
    }
  while (i > 0);

  nlm_sha256_xform ();

  for (i = 0; i < 8; i++)
    chksum->word[i] = ha[i];
}

uint32_t
nlm_chksum_sha256 (const void *buffer, uint32_t size, nlm_chksum *chksum)
{
  uint32_t i;
  uint8_t *buf = (uint8_t *)buffer;

  if (0 == buffer || 0 == chksum || 0 == size )
    return 6;

  nlm_chksum_sha256_reset ();

  for (i = 0; i < size; i++)
  {
    if (0x40 == nlm_chksum_sha256_add_byte (buf[i]))
      nlm_sha256_xform ();
  }

  nlm_chksum_sha256_post (chksum);

  return 0;
}
