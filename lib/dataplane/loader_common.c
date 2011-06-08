/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

//#define DEBUG
#include "nlm_loader_common.h"
#include "nlm_system.h"

uint32_t
nlm_read_misaligned_uint32 (uint8_t *buf)
{
  uint32_t i;
  i = buf[0] << 24; i |= buf[1] << 16; i |= buf[2] << 8; i |= buf[3];
  return i;
}

uint64_t
nlm_read_misaligned_uint64 (uint8_t *buf)
{
  uint64_t i;
  i = (uint64_t) buf[0] << 56; i |= (uint64_t) buf[1] << 48;
  i |= (uint64_t) buf[2] << 40; i |= (uint64_t) buf[3] << 32;
  i |= buf[4] << 24; i |= buf[5] << 16; i |= buf[6] << 8; i |= buf[7];
  return i;
}

int
nlm_get_entry_header (uint8_t *buf, uint64_t *addr, uint32_t *num_entry, uint32_t *entry_size,
                      uint32_t *addr_inc)
{
  uint32_t t = buf[0];

  if (t == 0xff)
    return 0xff;

  *entry_size = buf[1];
  *addr_inc = buf[2];

  if (buf[3] != (~t & 0xff))
    return -1;

  *addr = nlm_read_misaligned_uint64 (&buf[4]);
  *num_entry = nlm_read_misaligned_uint32 (&buf[12]);

  return t;
}

nlm_status
nlm_check_preamble (const void *buffer)
{
  nlm_db_file_preamble *preamble = (nlm_db_file_preamble *) buffer;

  if (BIO_MAGIC != preamble->magic || BIO_MAGIC_ != preamble->magic_)
    /* It is possible to detect invalid endianness here */
  {
    DPRINT ("Error! Bad Magic. Expected %x, got %x and %x\n", BIO_MAGIC, preamble->magic,
            preamble->magic_);
    return NLM_INVALID_DATABASE_FILE_MAGIC;
  }

  if (BIO_MAJOR != preamble->major_rev)
  {
    DPRINT ("Error! Major revisions differ. Expected %d, got %d\n", BIO_MAJOR,
            preamble->major_rev);
    return NLM_INVALID_DATABASE_VERSION;
  }

  if (BIO_MINOR != preamble->minor_rev)
  {
    /* do whatever is needed to signal conversion */
    DPRINT ("Warning! Minor revisions differ. Expected %d, got %d\n", BIO_MINOR,
            preamble->minor_rev);
  }

  return NLM_OK;
}
 
/*  Check integrity of the database and set database_id and the database buffer.
    Return NLM_OK if good, NLM_INVALID_DATABASE otherwise.
*/
nlm_status
nlm_check_database (const void *buffer, uint32_t db_size)
{
  nlm_chksum chksum;
  uint8_t *buf = (uint8_t*) buffer;
  uint32_t i;
  uint32_t chksum_start = db_size - 32;
  nlm_status status = NLM_OK;

  nlm_chksum_sha256 (buffer, chksum_start, &chksum);

  DPRINT ("Calculated database checksum is:\n%08x_%08x_%08x_%08x_%08x_%08x_%08x_%08x\n",
          chksum.word[0], chksum.word[1], chksum.word[2], chksum.word[3],
          chksum.word[4], chksum.word[5], chksum.word[6], chksum.word[7]);
  DPRINT ("Actual     database checksum is:\n%08x_%08x_%08x_%08x_%08x_%08x_%08x_%08x\n",
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 0 * 4),
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 1 * 4),
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 2 * 4),
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 3 * 4),
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 4 * 4),
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 5 * 4),
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 6 * 4),
                                      nlm_read_misaligned_uint32 (buf + chksum_start + 7 * 4));

  for (i = 0; i < 8; i++)
  {
    if (chksum.word[i] != nlm_read_misaligned_uint32 (buf + chksum_start + i * 4))
    {
      DPRINT ("The database has invalid checksum\n");
      return NLM_INVALID_DATABASE_CHECKSUM;
    }
  }

  status = nlm_check_preamble (buffer);

  return status;
}
