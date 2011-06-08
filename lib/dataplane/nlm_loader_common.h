/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef _NLM_LOADER_COMMON_H_
#define _NLM_LOADER_COMMON_H_

#include "nlm_stdint.h"
#include "nlm_packet_api.h"
#include "nlm_sha2.h"

#define BIO_MAJOR        1
#define BIO_MINOR        1
#define BIO_MAGIC        0x5d
#define BIO_MAGIC_       0xa2

/* These are pseudo/option registers and are part of the SW register address space.
   It's addressed like HW register space in order to be able to share code between them. */

#define NLML7_SW_NULL   (0x00)
#define NLML7_SW_MXLE   (0x04)
#define NLML7_SW_SIZE   (0x08)
#define NLML7_SW_MEMLIM (0x0c)
#define NLML7_SW_DB     (0x10)
#define NLML7_SW_ID     (0x20)

#define NLML7_OPTION_NULL       NLML7_SW_NULL
#define NLML7_OPTION_MXLE       NLML7_SW_MXLE
#define NLML7_OPTION_SIZE       NLML7_SW_SIZE
#define NLML7_OPTION_MEMLIM     NLML7_SW_MEMLIM
#define NLML7_OPTION_DB         NLML7_SW_DB
#define NLML7_OPTION_ID         NLML7_SW_ID

#define NLML7_PSEUDO_MXLE       NLML7_SW_MXLE
#define NLML7_PSEUDO_SIZE       NLML7_SW_SIZE

typedef enum nlm_image_bio_type 
{
  BIO_REG       = 0x00, /* configuration register writes */
  BIO_CCARRAY   = 0x01, /* cc bit array memory */
  BIO_PSEUDO    = 0x02, /* required pseudo configuration register writes */
  BIO_OPTION    = 0x03, /* debug/comment/optional pseudo configuration register writes */
  BIO_SRAM0     = 0x10, /* sram0 writes (DB A)*/
  BIO_SRAM1     = 0x11, /* sram1 writes (DB B)*/
  BIO_SRAM2     = 0x12, /* sram1 writes (DB C)*/
  BIO_SRAM3     = 0x13, /* sram1 writes (DB D)*/
  BIO_SRAM4     = 0x14, /* sram1 writes (DB E)*/
  BIO_SRAM5     = 0x15, /* sram1 writes (DB F)*/
  BIO_SRAM6     = 0x16, /* sram1 writes (DB G)*/
  BIO_SRAM7     = 0x17, /* sram1 writes (DB H)*/
  BIO_DRAM      = 0x20, /* currently not implemented */
  BIO_HMEM0     = 0x30, /* spill over area 0 writes (DB A)-> host memory writes for overflow (DBA)*/
  BIO_HMEM1     = 0x31, /* spill over area 1 writes (DB B)-> host memory writes for overflow (DBB)*/
  BIO_HMEM2     = 0x32, /* spill over area 2 writes (DB C)-> host memory writes for overflow (DBC)*/
  BIO_HMEM3     = 0x33, /* spill over area 3 writes (DB D)-> host memory writes for overflow (DBD)*/
  BIO_HMEM4     = 0x34, /* spill over area 4 writes (DB E)-> host memory writes for overflow (DBE)*/
  BIO_HMEM5     = 0x35, /* spill over area 5 writes (DB F)-> host memory writes for overflow (DBF)*/
  BIO_HMEM6     = 0x36, /* spill over area 6 writes (DB G)-> host memory writes for overflow (DBG)*/
  BIO_HMEM7     = 0x37, /* spill over area 7 writes (DB H)-> host memory writes for overflow (DBH)*/
  BIO_VIRT      = 0x40, /* relocatable database */
  BIO_VIRT_DB0  = 0x40, /* virtual (flat) database write (DB A)*/
  BIO_VIRT_DB1  = 0x41, /* virtual (flat) database write (DB B)*/
  BIO_EOF       = 0xff  /* End of File record.*/
} nlm_image_bio_type;

typedef struct nlm_db_file_preamble
{
  uint8_t magic;
  uint8_t major_rev;
  uint8_t minor_rev;
  uint8_t magic_;
} nlm_db_file_preamble;

uint32_t nlm_read_misaligned_uint32 (uint8_t *buf);
uint64_t nlm_read_misaligned_uint64 (uint8_t *buf);
int nlm_get_entry_header (uint8_t *buf, uint64_t *addr, uint32_t *num_entry,
                          uint32_t *entry_size, uint32_t *addr_inc);
nlm_status nlm_check_preamble (const void *buffer);
nlm_status nlm_check_database (const void *buffer, uint32_t db_size);
#endif
