/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef __NLM_SHA2_H
#define __NLM_SHA2_H

#include "nlm_byteswap.h"
#include "nlm_stdint.h"

typedef struct nlm_chksum
{
  uint32_t word[8];
} nlm_chksum;

void
nlm_chksum_sha256_reset (void);

uint32_t
nlm_chksum_sha256_add_byte (uint8_t byte);

void
nlm_chksum_sha256_post (nlm_chksum *chksum);

uint32_t
nlm_chksum_sha256 (const void *buffer, uint32_t size, nlm_chksum *chksum);

#endif /* __NLM_SHA2_H */
