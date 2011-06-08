/*******************************************************************************
* Legal Notice:
* This product is the confidential property of NetLogic Microsystems Inc.
* ("NetLogic"), is provided under a non-disclosure agreement, and is protected
* under applicable copyright, patent, and trade secret laws. Unauthorized use,
* reproduction, distribution or other dissemination without the prior written
* authorization from NetLogic is strictly prohibited.
*
* NETLOGIC DISCLAIMS ALL WARRANTIES OF ANY NATURE, EXPRESS OR IMPLIED,
* INCLUDING, WITHOUT LIMITATION, THE WARRANTIES OF FITNESS FOR A PARTICULAR
* PURPOSE, MERCHANTABILITY AND/OR NON-INFRINGEMENT OF THIRD PARTY RIGHTS.
* NETLOGIC ASSUMES NO LIABILITY FOR ANY ERROR OR OMISSIONS IN THIS PRODUCT,OR
* FOR THE USE OF THIS PRODUCT. IN NO EVENT SHALL NETLOGIC BE LIABLE TO ANY OTHER
* PARTY FOR ANY SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL DAMAGES, WHETHER
* BASED ON BREACH OF CONTRACT, TORT, PRODUCT LIABILITY, INFRINGEMENT OF
* INTELLECTUAL PROPERTY RIGHTS OR OTHERWISE. NETLOGIC RESERVES THE RIGHT TO MAKE
* CHANGES TO, OR DISCONTINUE, ITS PRODUCTS AT ANY TIME.
*
* Distribution of the product herein does not convey a license or any other
* right in any patent, trademark, or other intellectual property of NetLogic.
*
* Use of the product shall serve as acceptance of these terms and conditions.
* If you do not accept these terms, you should return or destroy the product
* and any other accompanying information immediately.
*
* Copyright, 2008, NetLogic Microsystems, Inc. All rights reserved.
*******************************************************************************/
#include <string.h>
#ifndef DEBUG
// #define DEBUG
#endif
#define FMSUSEDMA 1
#include "nlm_system.h"
#include "nlm_packet_api_impl.h"
#include "nlm_fms_formats.h"

#define BLOCK_DENY_MASK 0x0

#define WRITE_ARRAY_ENTRY(BLK, ADDR, DATA_0, DATA_1, DATA_2) \
  do { \
    DPRINT ("Writing addr: 0x%08x 0x%08x_0x%08x_0x%08x\n", \
                           ((BLK) << 24) | (ADDR), (DATA_2), (DATA_1), (DATA_0));\
    SET_HW_REGISTER (FAMOS_BLK_ADDR,  ((BLK) << 24) | (ADDR)); \
    SET_HW_REGISTER (FAMOS_BLK_DATA0, (DATA_0)); \
    SET_HW_REGISTER (FAMOS_BLK_DATA1, (DATA_1)); \
    SET_HW_REGISTER (FAMOS_BLK_DATA2, (DATA_2)); \
    SET_HW_REGISTER (FAMOS_CTRL, 1); \
    while (HW_REGISTER (FAMOS_CTRL) & 1); \
  } while (0)

#define READ_ARRAY_ENTRY(BLK, ADDR, DATA_0, DATA_1, DATA_2) \
  do { \
    SET_HW_REGISTER (FAMOS_BLK_ADDR,  ((BLK) << 24) | (ADDR)); \
    SET_HW_REGISTER (FAMOS_CTRL, 2); \
    while (HW_REGISTER (FAMOS_CTRL) & 2); \
    (DATA_0) = HW_REGISTER (FAMOS_BLK_DATA0); \
    (DATA_1) = HW_REGISTER (FAMOS_BLK_DATA1); \
    (DATA_2) = HW_REGISTER (FAMOS_BLK_DATA2); \
    DPRINT ("Read addr: 0x%08x 0x%08x_0x%08x_0x%08x\n", \
                           ((BLK) << 24) | (ADDR), (DATA_2), (DATA_1), (DATA_0));\
  } while (0)

#define WRITE_ACTION_TABLE_ENTRY(BLK, ADDR, DATA_0, DATA_1, DATA_2, DATA_3) \
  do { \
/* \
    DPRINT ("Writing addr: 0x%08x 0x%08x_0x%08x_0x%08x_0x%08x\n", \
                           ((BLK) << 11) | (ADDR), (DATA_3), (DATA_2), (DATA_1), (DATA_0)); \
*/ \
    SET_HW_REGISTER (ACTION_DATA0, (DATA_0)); \
    SET_HW_REGISTER (ACTION_DATA1, (DATA_1)); \
    SET_HW_REGISTER (ACTION_DATA2, (DATA_2)); \
    SET_HW_REGISTER (ACTION_DATA3, (DATA_3)); \
    SET_HW_REGISTER (ACTION_CTRL, (((((BLK) << 11) | (ADDR)) & 0x7fff) << 16) | 1); \
    while (HW_REGISTER (ACTION_CTRL) & 1); \
  } while (0)

#define WRITE_GID_BMAP_ENTRY(DB_ID, GID, MAP) \
  do { \
    uint32_t data_0 = (MAP).w0.reg; \
    uint32_t data_1 = (MAP).w1.reg; \
    while (HW_REGISTER (GID_BMAP_CTRL) & 1); \
    DPRINT ("Writing addr: 0x%08x 0x%08x_0x%08x\n", \
                           ((DB_ID) << 26) | ((GID) << 16), data_0, data_1); \
    SET_HW_REGISTER (GID_BMAP_DATA0, data_0); \
    SET_HW_REGISTER (GID_BMAP_DATA1, data_1); \
    SET_HW_REGISTER (GID_BMAP_CTRL, (((DB_ID) << 26) | ((GID) << 16)) | 1); \
  } while (0)

#define DELETE_GID_BMAP_ENTRY(DB_ID, GID) \
  do { \
    DPRINT ("Deleting GID entry addr: 0x%08x\n", \
                           ((DB_ID) << 26) | ((GID) << 16)); \
    SET_HW_REGISTER (GID_BMAP_DATA0, 0); \
    SET_HW_REGISTER (GID_BMAP_DATA1, 0); \
    SET_HW_REGISTER (GID_BMAP_CTRL, (((DB_ID) << 26) | ((GID) << 16)) | 1); \
    while (HW_REGISTER (GID_BMAP_CTRL) & 1); \
  } while (0)

#define BLOCK_PAUSE(BLK_NO) \
  do { \
    register uint32_t map; \
    DPRINT ("Pausing block # %d\n", (BLK_NO)); \
    pretty_assert (0 == ((HW_REGISTER (BLOCK_IDLE_READY)) & (1 << (BLK_NO)))); \
    map = HW_REGISTER (BLOCK_IDLE_CHECK); \
    SET_HW_REGISTER (BLOCK_IDLE_CHECK, (map | (1 << (BLK_NO)))); \
  } while (0)

#define BLOCKS_PAUSE(BLK_MAP) \
  do { \
    register uint32_t map; \
    DPRINT ("Pausing blocks # %0x03x\n", (BLK_MAP)); \
    map = HW_REGISTER (BLOCK_IDLE_CHECK); \
    SET_HW_REGISTER (BLOCK_IDLE_CHECK, map | (BLK_MAP)); \
  } while (0)

#define BLOCK_RESUME(BLK_NO) \
  do { \
    register uint32_t map; \
    DPRINT ("Resuming block # %d\n", (BLK_NO)); \
    map = HW_REGISTER (BLOCK_IDLE_CHECK); \
    SET_HW_REGISTER (BLOCK_IDLE_CHECK, map & ~(1 << (BLK_NO))); \
  } while (0)

#define BLOCKS_RESUME(BLK_MAP) \
  do { \
    register uint32_t map; \
    DPRINT ("Resuming blocks # %0x03x\n", (BLK_MAP)); \
    map = HW_REGISTER (BLOCK_IDLE_CHECK); \
    SET_HW_REGISTER (BLOCK_IDLE_CHECK, map & ~(BLK_MAP)); \
  } while (0)

#define IS_BLOCK_PAUSED(BLK_NO) \
    (HW_REGISTER (BLOCK_IDLE_READY)) & (1 << (BLK_NO))

#define ARE_BLOCKS_PAUSED(BLK_MAP) \
    (0 == (HW_REGISTER (BLOCK_IDLE_READY)) ^ (BLK_MAP))

#define LOCK_GID_BMAP() \
  do { \
    SET_HW_REGISTER (GID_BMAP_LOCK, 3); \
    while (HW_REGISTER (GID_BMAP_LOCK) & 1); \
    DPRINT ("GID_BMAP Locked\n"); \
  } while (0)

#define UNLOCK_GID_BMAP() \
  do { \
    SET_HW_REGISTER (GID_BMAP_LOCK, 1); \
    while (HW_REGISTER (GID_BMAP_LOCK) & 1); \
    DPRINT ("GID_BMAP Unlocked\n"); \
  } while (0)

#define IS_GID_BMAP_LOCKED() \
  (((HW_REGISTER (GID_LOCK_ACTIVE)) >> 24) & 1)

#define BLOCK_ADD(BLK_NO) \
  do { \
    DPRINT ("Adding block %d\n", BLK_NO); \
    SET_HW_REGISTER (BLOCK_CTRL_MAP, 1 << (BLK_NO)); \
    SET_HW_REGISTER (BLOCK_CTRL, 3); \
    while (HW_REGISTER (BLOCK_CTRL) & 1); \
    if (HW_REGISTER (BLOCK_CTRL) & (1 << 2)) \
      { \
        DPRINT ("Adding block failed : %08x\n", (HW_REGISTER (BLOCK_CTRL) >> 3) & 0x7); \
      } \
  } while (0)

#define BLOCK_DELETE(BLK_NO) \
  do { \
    DPRINT ("Deleting array block # %d\n", (BLK_NO)); \
    SET_HW_REGISTER (BLOCK_CTRL_MAP, 1 << (BLK_NO)); \
    SET_HW_REGISTER (BLOCK_CTRL, 1); \
    while (HW_REGISTER (BLOCK_CTRL) & 1); \
    if (HW_REGISTER (BLOCK_CTRL) & (1 << 2)) \
      { \
        DPRINT ("Deleting array block failed : %08x\n", (HW_REGISTER (BLOCK_CTRL) >> 3) & 0x1); \
        pretty_assert (0); \
      } \
  } while (0)

#define BLOCK_SET_VIRT_ID_ASYNC(BLK_NO, VB_ID) \
  do { \
    DPRINT ("Assigning virt id %d to block # %d\n", (VB_ID), (BLK_NO)); \
    SET_HW_REGISTER (FAMOS_BLK_ADDR,  ((BLK_NO) << 24) | 0xC00000); \
    SET_HW_REGISTER (FAMOS_BLK_DATA0, (VB_ID) & 0xFul); \
    SET_HW_REGISTER (FAMOS_BLK_DATA1, 0); \
    SET_HW_REGISTER (FAMOS_BLK_DATA2, 0); \
    SET_HW_REGISTER (FAMOS_CTRL, 1); \
  } while (0)

#define BLOCK_GET_VIRT_ID(BLK_NO, VB_ID) \
  do { \
    DPRINT ("Reading virt id of block # %d: ", (BLK_NO)); \
    SET_HW_REGISTER (FAMOS_BLK_ADDR,  ((BLK_NO) << 24) | 0xC00000); \
    SET_HW_REGISTER (FAMOS_CTRL, 2); \
    while (HW_REGISTER (FAMOS_CTRL) & 2); \
    (VB_ID) = SET_HW_REGISTER (FAMOS_BLK_DATA0, (VB_ID) & 0xFul); \
    DDPRINT ("%d\n", (VB_ID)); \
  } while (0)

#define IS_DMA_BUSY() \
    (HW_REGISTER (BLOCK_DMA_CONFIG2) & 1)


static void
nlm_device_create_shadow_block_list_by_db_id (nlm_phy_device_t *phy_device, uint32_t database_id,
                                              uint32_t *list_len, uint32_t *list)
{
  int i, len = 0;
  for (i = 0; i < phy_device->num_fms_blocks; i++)
    if (phy_device->database_shadow_map[i].valid && phy_device->database_shadow_map[i].db_id == database_id)
      list[len++] = i;
  *list_len = len;
}

static int32_t
nlm_device_get_num_shadow_blocks_free (nlm_phy_device_t *phy_device)
{
  int32_t i;
  int32_t shadow_blocks_free = -1;

  for (shadow_blocks_free = 0, i = 0; i < phy_device->num_fms_blocks; i++)
    shadow_blocks_free += (0 == phy_device->database_shadow_map[i].valid);

  return shadow_blocks_free;
}

static nlm_status
nlm_loader_fms_check_params_and_head (nlm_phy_device_t *phy_device, const void *buffer, uint32_t db_size)
{
  struct fms_file_sig *header = (struct fms_file_sig *) buffer;
  struct fms_block_hw *block = (struct fms_block_hw *) (buffer + sizeof (struct fms_file_sig));
  uint32_t block_cnt = (db_size - sizeof (struct fms_file_sig)) / sizeof (struct fms_block_hw);
  int i, j;

  if (NULL == buffer || 0 == db_size)
    return NLM_INVALID_ARGUMENT;

  if (db_size != sizeof (struct fms_file_sig) + sizeof (struct fms_block_hw) * block_cnt)
    {
      DPRINT ("Database size is incorrect.\n");
      return NLM_INVALID_DATABASE_SIZE;
    }

  if ((block_cnt < 1) || (block_cnt > phy_device->num_fms_blocks))
    {
      DPRINT ("Device has %d blocks, but database is %d blocks big\n",
              phy_device->num_fms_blocks, block_cnt);
      return NLM_INVALID_DATABASE_SIZE;
    }

  for (i = 0; i < block_cnt; i++, block++)
    for (j = phy_device->num_rows_per_fms_block; j < FAMOS_BLOCK_SIZE; j++)
      if (block->row[j].data[0] || block->row[j].data[1] || block->row[j].data[2])
        {
          DPRINT ("Unexpected useful data at position %d past the last device row at %d\n",
                  j, phy_device->num_rows_per_fms_block);
          return NLM_INVALID_DATABASE_SIZE;
        }

  if (SWAP32 (FMS_ASM_FAMOS_FPGA) != header->product_id)
    return NLM_INVALID_DATABASE_VERSION;

  return NLM_OK;
}

#if 0
static nlm_status
nlm_device_load_fms_block_reg (nlm_phy_device_t *phy_device, uint32_t blk_no, struct fms_block_hw *block)
{
  uint32_t addr;
  /* Write array block */
  for (addr = 0; addr < phy_device->num_rows_per_fms_block; addr++)
    WRITE_ARRAY_ENTRY (blk_no, addr,  block->row[addr].data[0],
                                      block->row[addr].data[1],
                                      block->row[addr].data[2]);
  /*  */
  return NLM_OK;
}
#endif

static nlm_status
nlm_device_load_action_table (nlm_phy_device_t *phy_device, uint32_t blk_no, struct fms_block_hw *block)
{
  uint32_t addr;
  /* Write action table */
  for (addr = 0; addr < MAX_ACCEPT_PER_BLOCK; addr++)
    {
      WRITE_ACTION_TABLE_ENTRY(blk_no, addr, block->action_table[addr].w0.reg,
                                             block->action_table[addr].w1.reg,
                                             block->action_table[addr].w2.reg,
                                             block->action_table[addr].w3.reg);
    }
  return NLM_OK;
}

nlm_status
NLM_PROTO (nlm_device_try_load_database) (nlm_device_t *device, const void *buffer,
                                          uint32_t db_size, uint32_t replacing_db_id,
                                          uint32_t new_db_id, uint32_t num_copies)
{
  struct fms_block_hw *block = (struct fms_block_hw *) (buffer + sizeof (struct fms_file_sig));
  uint32_t block_cnt = (db_size - sizeof (struct fms_file_sig)) / sizeof (struct fms_block_hw);
  int32_t shadow_blocks_free;
  uint32_t block_list[TOTAL_FAMOS_BLOCKS];
  uint32_t num_blocks_in_list = 0;
  nlm_phy_device_t *phy_device;
  
  DPRINT ("Database id %d, replacing %d, size %d, %d blocks x %d copies are requested\n",
            new_db_id, replacing_db_id, db_size, block_cnt, num_copies);

  if (NULL == device)
    return NLM_INVALID_ARGUMENT;

  if (0 != device->phy_device->fms_ring_id) /* only master ring (ring #0) can load/unload database */
    return NLM_INVALID_RING_ID;

  if (device->thread_id != 0)
    return NLM_INVALID_THREAD_ID;
  
  phy_device = device->phy_device;
  shadow_blocks_free = nlm_device_get_num_shadow_blocks_free (phy_device);
  if (0 > shadow_blocks_free
      || 4 < num_copies)
    return NLM_INVALID_ARGUMENT;

  if (NLM_MAX_DATABASE_ID < new_db_id)
    {
      DPRINT ("if (NLM_MAX_DATABASE_ID < new_db_id\n");
      return NLM_INVALID_DATABASE_ID;
    }

  if (-1 != replacing_db_id && NLM_MAX_DATABASE_ID < replacing_db_id)
    {
      DPRINT ("if (-1 != replacing_db_id && NLM_MAX_DATABASE_ID < replacing_db_id)\n");
      return NLM_INVALID_DATABASE_ID;
    }

  TRY (nlm_loader_fms_check_params_and_head (phy_device, buffer, db_size))

  nlm_device_create_shadow_block_list_by_db_id (phy_device, new_db_id,
                                                &num_blocks_in_list, block_list);

  if (new_db_id != replacing_db_id && num_blocks_in_list)
    {
      DPRINT ("if (new_db_id != replacing_db_id && num_blocks_in_list)\n");
      return NLM_INVALID_DATABASE_ID;
    }
//    return device->database_shadow_map[block_list[0]].no_clones_target
//      ? NLM_INVALID_ARGUMENT : NLM_BUSY;

  if (-1 != replacing_db_id)
    {
      nlm_device_create_shadow_block_list_by_db_id (phy_device, replacing_db_id,
                                                    &num_blocks_in_list, block_list);
      if (0 == num_blocks_in_list)
        {
          DPRINT ("if (0 == num_blocks_in_list)\n");
          return NLM_INVALID_DATABASE_ID;
        }
    }
  else
    num_blocks_in_list = 0;

  if (num_copies > 0)
    {
      int i, array_blocks_free, blocks_to_reuse = 0;

      if (num_blocks_in_list)
        blocks_to_reuse
          = num_blocks_in_list * phy_device->database_shadow_map[block_list[0]].no_clones_target;

      for (array_blocks_free = 0, i = 0; i < phy_device->num_fms_blocks; i++)
        array_blocks_free += (0 == phy_device->array_block_map_target[i]);

      DPRINT ("Array blocks free %d, blocks to reuse %d\n", array_blocks_free, blocks_to_reuse);
      if (block_cnt * num_copies > array_blocks_free + blocks_to_reuse)
        return NLM_INVALID_DATABASE_SIZE;
    }
  else
    {
      DPRINT ("Shadow blocks free %d, blocks to reuse %d\n",
                  shadow_blocks_free, num_blocks_in_list);
      if (block_cnt > shadow_blocks_free + num_blocks_in_list)
        return NLM_INVALID_DATABASE_SIZE;
    }

  for ( block = (struct fms_block_hw *) (buffer + sizeof (struct fms_file_sig));
        block < (struct fms_block_hw *) (buffer + db_size);
        block++)
    {
      int i;
      nlm_chksum chksum;
      nlm_chksum_sha256 ( block, sizeof (struct fms_block_hw) - sizeof (nlm_chksum), &chksum);
      for (i = 0; i < 8; i++)
        if (SWAP32 (chksum.word[i]) != block->checksum.word[i])
          return NLM_INVALID_DATABASE_CHECKSUM;
    }
  return NLM_OK;
}

static nlm_status
nlm_extract_groups_from_block (struct fms_block_hw *block, uint16_t *group_list, uint32_t db_id)
{
  int i, ind = 0;
  uint8_t seen[NLM_MAX_GROUP_ID + 1] = {0};
  union fms_row_hw cur_row;
  uint32_t cur_gid = 0;

  for (i = 0; i < FAMOS_BLOCK_SIZE; i++)
  /* The tcam member of union is used with the assumption that
      cntr member has the same field reserved and initialized to 0
  */
    {
      cur_row.data[0] = SWAP32 (block->row[i].data[0]);
      cur_row.data[1] = SWAP32 (block->row[i].data[1]);
      cur_row.data[2] = SWAP32 (block->row[i].data[2]);

      cur_gid = cur_row.tcam.w0.gid & 0x3ff;

      if (cur_gid)
        {
          if (!seen[cur_gid])
            {
              DPRINT ("Adding group %03x\n", cur_gid);
              seen[cur_gid] = 1;
              group_list[ind++] = cur_gid;
            }

          cur_row.tcam.w0.gid |= (db_id << 10);
          block->row[i].data[0] = SWAP32 (cur_row.data[0]);
          block->row[i].data[1] = SWAP32 (cur_row.data[1]);
          block->row[i].data[2] = SWAP32 (cur_row.data[2]);
        }
    }

  if (NLM_MAX_GROUP_ID < ind)
    return NLM_INVALID_GROUPID;

  group_list[ind] = 0;

  for (i = 0; i < MAX_ACCEPT_PER_BLOCK; i++)
    if (block->action_table[i].w0.s.gid & 0x3ff)
      block->action_table[i].w0.s.gid |= (db_id << 10);

  return NLM_OK;
}

static nlm_status
nlm_device_block_scheduler_add (nlm_phy_device_t *phy_device, uint32_t addr_shadow,
                                uint32_t addr_array, uint32_t timeslot_map)
{
  int i;
  uint32_t db_id = phy_device->database_shadow_map[addr_shadow].db_id;
  uint16_t *group_list = phy_device->block_group_list[addr_shadow];
  uint32_t bmap = 1 << addr_array;

  DPRINT ("Shadow block %d belongs to database %d; array address %d\n",
          addr_shadow, db_id, addr_array);

  for (i = 0; group_list[i] && i <= NLM_MAX_GROUP_ID; i++)
    {
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap0
        |= timeslot_map & 8 ? bmap : 0;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap1
        |= timeslot_map & 4 ? bmap : 0;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap2
        |= timeslot_map & 2 ? bmap : 0;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap3
        |= timeslot_map & 1 ? bmap : 0;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.valid0
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap0);
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.valid1
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap1);
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.valid2
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap2);
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.valid3
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap3);
      WRITE_GID_BMAP_ENTRY (db_id, group_list[i], phy_device->gid_bmap_shadow[db_id][group_list[i]]);
    }

  while (HW_REGISTER (GID_BMAP_CTRL) & 1);
  return NLM_OK;
}

static nlm_status
nlm_device_block_scheduler_delete (nlm_phy_device_t *phy_device, uint32_t addr_shadow,
                                    uint32_t addr_array, uint32_t timeslot_map)
{
  int i;
  uint32_t db_id = phy_device->database_shadow_map[addr_shadow].db_id;
  uint16_t *group_list = phy_device->block_group_list[addr_shadow];
  const uint32_t bmap = ~(1 << addr_array);
  const uint32_t ones = ~0;

  for (i = 0; group_list[i] && i <= NLM_MAX_GROUP_ID; i++)
    {
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap0
        &= timeslot_map & 8 ? bmap : ones;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap1
        &= timeslot_map & 4 ? bmap : ones;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap2
        &= timeslot_map & 2 ? bmap : ones;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap3
        &= timeslot_map & 1 ? bmap : ones;
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.valid0
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap0);
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.valid1
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w0.s.fms_block_bitmap1);
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.valid2
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap2);
      phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.valid3
         = (0 != phy_device->gid_bmap_shadow[db_id][group_list[i]].w1.s.fms_block_bitmap3);
      WRITE_GID_BMAP_ENTRY (db_id, group_list[i], phy_device->gid_bmap_shadow[db_id][group_list[i]]);
    }

  while (HW_REGISTER (GID_BMAP_CTRL) & 1);
  return NLM_OK;
}

static nlm_status
nlm_device_write_database_to_shadow (nlm_phy_device_t *phy_device, uint32_t database_id,
                                     const void *database, uint32_t num_blocks)
{
  struct fms_block_hw *block = (struct fms_block_hw *) (database + sizeof (struct fms_file_sig));
  struct fms_block_hw *db_base = (struct fms_block_hw *) phy_device->database_extension_area;
  uint32_t blk_no, i;
  uint32_t shadow_blocks_free = nlm_device_get_num_shadow_blocks_free (phy_device);

  if (0 > shadow_blocks_free)
    return NLM_INVALID_ARGUMENT;

  if (num_blocks > shadow_blocks_free)
    return NLM_INVALID_DATABASE_SIZE;

  for (blk_no = 0, i = 0;
      (blk_no < num_blocks) && (i < phy_device->num_fms_blocks);
      i++)
    {
      uint32_t *at_base, j;
      if (phy_device->database_shadow_map[i].valid)
        continue;

      DPRINT ("Extracting groups from block #%01x...\n", i);
      memcpy (db_base + i, block++, sizeof (struct fms_block_hw));

      at_base = (uint32_t *) &((struct fms_block_hw*)(db_base + i))->action_table;
      for (j = 0; j < MAX_ACCEPT_PER_BLOCK * 4; j++)
        at_base[j] = SWAP32 (at_base[j]);

      TRY (nlm_extract_groups_from_block (db_base + i, phy_device->block_group_list[i], database_id))

      phy_device->database_shadow_map[i].valid = 1;
      phy_device->database_shadow_map[i].db_id = database_id;
      phy_device->database_shadow_map[i].virt_blk_no = i;
      DPRINT ("Database %d occupied shadow block %d\n", database_id, i);
      blk_no++;
    }

  phy_device->first_free_db_block = (uint8_t *) (db_base + i);
  return NLM_OK;
}

static nlm_status
nlm_device_move_block_from_shadow (nlm_phy_device_t *phy_device, uint32_t shadow_blk_id,
                                   uint32_t array_blk_id)
{
  if (shadow_blk_id > phy_device->num_fms_blocks || array_blk_id > phy_device->num_fms_blocks)
    {
      DPRINT("(shadow_blk_id > phy_device->num_fms_blocks || array_blk_id > phy_device->num_fms_blocks)\n");
      return NLM_INVALID_ARGUMENT;
    }
  DPRINT ("Moving shadow block %02d to array block %02d\n", shadow_blk_id, array_blk_id);
  pretty_assert (0 == IS_DMA_BUSY ());

  MEMORY_BARRIER ();

  /* Point HW block to the correct shadow offset */
  SET_HW_REGISTER (BLOCK0_OFFSET + FMS_BLK_REG_STEP * array_blk_id,
                    shadow_blk_id * sizeof (struct fms_block_hw));
  /* And enable it*/
  SET_HW_REGISTER (BLOCK0_CONFIG + FMS_BLK_REG_STEP * array_blk_id, 1);

  /* Launch the DMA */
  /* Select the block, offset 0 */
  SET_HW_REGISTER (BLOCK_DMA_CONFIG0, array_blk_id << 24);
  /* Order number of entries to transfer */
  SET_HW_REGISTER (BLOCK_DMA_CONFIG1, phy_device->num_rows_per_fms_block);
  /* Start the DMA */
  SET_HW_REGISTER (BLOCK_DMA_CONFIG2, 1);
  /* The user will have to use IS_DMA_COMPLETE() to check for transfer completion */
  return NLM_OK;
}

nlm_status
nlm_device_loader_cmd_exec_fms (nlm_device_t *device)
{
  struct fms_loader_command cmd;
  struct fms_block_hw *db_base;
  nlm_phy_device_t *phy_device;
  
  if (NULL == device)
    return NLM_INVALID_ARGUMENT;

  if (0 != device->phy_device->fms_ring_id) /* only master ring (ring #0) can load/unload database */
    return NLM_INVALID_RING_ID;

  if (0 != device->thread_id)
    return NLM_INVALID_THREAD_ID;

  phy_device = device->phy_device;
  cmd = phy_device->loader_cmd_mem[phy_device->cmd_rd_ptr];
  db_base = (struct fms_block_hw *) phy_device->database_extension_area;

  if (phy_device->cmd_rd_ptr == phy_device->cmd_wr_ptr)
    return NLM_OK;

  if (FMS_LOADER_CMD_DMA_XFER_COMPLETE_WAIT != cmd.opcode)
    DPRINT ("rd_ptr: %02d, wr_ptr: %02d, opcode: %02x, addr_array: %01x, addr_shadow: %01x : ",
           phy_device->cmd_rd_ptr, phy_device->cmd_wr_ptr, cmd.opcode, cmd.addr_array, cmd.addr_shadow);

  switch (cmd.opcode)
    {
    case FMS_LOADER_CMD_NOP:
      DDPRINT ("FMS_LOADER_CMD_NOP\n");
      break;
    case FMS_LOADER_CMD_ARRAY_ADD:
      DDPRINT ("FMS_LOADER_CMD_ARRAY_ADD\n");
      BLOCK_ADD (cmd.addr_array);
      phy_device->array_block_map_current[cmd.addr_array] = 1;
      phy_device->array_block_copy_number[cmd.addr_array]
        = phy_device->database_shadow_map[cmd.addr_shadow].no_clones_current++;
      break;
    case FMS_LOADER_CMD_ARRAY_DELETE:
      DDPRINT ("FMS_LOADER_CMD_ARRAY_DELETE\n");
      BLOCK_DELETE (cmd.addr_array);
      phy_device->array_block_map_current[cmd.addr_array] = 0;
      phy_device->database_shadow_map[cmd.addr_shadow].no_clones_current--;
      break;
    case FMS_LOADER_CMD_SHADOW_DELETE:
      DDPRINT ("FMS_LOADER_CMD_SHADOW_DELETE\n");
      phy_device->database_shadow_map[cmd.addr_shadow].valid = 0;
      pretty_assert (0 == phy_device->database_shadow_map[cmd.addr_shadow].no_clones_current);
      break;
    case FMS_LOADER_CMD_PAUSE:
      DDPRINT ("FMS_LOADER_CMD_PAUSE\n");
      phy_device->wait_timeout = phy_device->pause_timeout;
      BLOCK_PAUSE (cmd.addr_array);
      break;
    case FMS_LOADER_CMD_PAUSE_WAIT:
      DDPRINT ("FMS_LOADER_CMD_PAUSE_WAIT\n");
      if (1 == phy_device->wait_timeout)
        return NLM_LOADER_BLOCK_PAUSE_TIMEOUT;
      if (IS_BLOCK_PAUSED (cmd.addr_array))
        break;
      else
        {
          if (phy_device->wait_timeout)
            phy_device->wait_timeout--;
          return NLM_LOADER_IN_PROGRESS;
        }
    case FMS_LOADER_CMD_RESUME:
      DDPRINT ("FMS_LOADER_CMD_RESUME\n");
      phy_device->wait_timeout = FMS_MAX_WAIT_TIMEOUT;
      BLOCK_RESUME (cmd.addr_array);
      break;
    case FMS_LOADER_CMD_RESUME_WAIT:
      DDPRINT ("FMS_LOADER_CMD_RESUME_WAIT\n");
      pretty_assert (phy_device->wait_timeout);
      if (IS_BLOCK_PAUSED (cmd.addr_array))
        {
          phy_device->wait_timeout--;
          return NLM_LOADER_IN_PROGRESS;
        }
      else
        break;
    case FMS_LOADER_CMD_DMA_XFER_START:
      DDPRINT ("FMS_LOADER_CMD_DMA_XFER_START\n");
      phy_device->wait_timeout = FMS_MAX_DMA_WAIT_TIMEOUT;
      nlm_device_move_block_from_shadow (phy_device, cmd.addr_shadow, cmd.addr_array);
      break;
    case FMS_LOADER_CMD_DMA_XFER_COMPLETE_WAIT:
      // DDPRINT ("FMS_LOADER_CMD_DMA_XFER_COMPLETE_WAIT\n");
      pretty_assert (phy_device->wait_timeout);
      if (IS_DMA_BUSY ())
        {
          phy_device->wait_timeout--;
          return NLM_LOADER_IN_PROGRESS;
        }
      else
        break;
    case FMS_LOADER_CMD_ACTION_TABLE_LOAD:
      DDPRINT ("FMS_LOADER_CMD_ACTION_TABLE_LOAD\n");
      nlm_device_load_action_table (phy_device, cmd.addr_array, db_base + cmd.addr_shadow);
      break;
    case FMS_LOADER_CMD_ACTION_TABLE_CLEAR:
      DDPRINT ("FMS_LOADER_CMD_ACTION_TABLE_CLEAR\n");
      break;
    case FMS_LOADER_CMD_GID_BMAP_CONFIGURE:
      DDPRINT ("FMS_LOADER_CMD_GID_BMAP_CONFIGURE\n");
      LOCK_GID_BMAP ();
      /* Here the bulk update of the database will happen that would rely on the addr.shadow as on
        identifier. */
        {
          int i, j;
          uint32_t num_blocks_in_this_db;
          uint32_t block_list[TOTAL_FAMOS_BLOCKS];

          nlm_device_create_shadow_block_list_by_db_id (phy_device, /* Warning: Parameter overload */
                                                        phy_device->database_shadow_map[cmd.addr_shadow].db_id,
                                                        &num_blocks_in_this_db, block_list);
          /* cmd.addr_shadow is used as a mean to determine
             the databse id, but it still points to a shadow
             block.
             cmd.addr_array is used to convey number of copies
          */
          for (j = 0; j < phy_device->num_fms_blocks; j++)
            {
              for (i = 0; i < num_blocks_in_this_db; i++)
                {
                  if (block_list[i] == phy_device->array_virt_ids[j]
                      && phy_device->array_block_map_current[j]
                      && phy_device->database_shadow_map[block_list[i]].no_clones_current
                        == cmd.addr_array)
                    {
                      nlm_device_block_scheduler_add (phy_device, block_list[i], j,
                                                      1 << phy_device->array_block_copy_number[j]);
                    }
                }
            }

        }
      UNLOCK_GID_BMAP ();
      break;
    case FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID:
      DDPRINT ("FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID\n");
      BLOCK_SET_VIRT_ID_ASYNC (cmd.addr_array, cmd.addr_shadow);
      break;
    case FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID_WAIT:
      DDPRINT ("FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID_WAIT\n");
      if (HW_REGISTER (FAMOS_CTRL) & 1)
        return NLM_LOADER_IN_PROGRESS;
      else
        phy_device->array_virt_ids[cmd.addr_array] = cmd.addr_shadow;
      break;
    default:
      pretty_assert (0);
    }

  phy_device->cmd_rd_ptr = ++phy_device->cmd_rd_ptr % NLM_FMS_LOADER_CMD_BUFF_SIZE;
  if (phy_device->cmd_rd_ptr == phy_device->cmd_wr_ptr)
    {
      int i;
      DPRINT ("Device state at the completion :\n");
      DPRINT ("Block number      : 0123456789AB\n");
      DPRINT ("Array map target  : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->array_block_map_target[i]);
      DDPRINT ("\n");
      DPRINT ("Array map current : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->array_block_map_current[i]);
      DDPRINT ("\n");
      DPRINT ("Array map copy no : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->array_block_copy_number[i]);
      DDPRINT ("\n");
      DPRINT ("Array map virt id : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->array_virt_ids[i]);
      DDPRINT ("\n");

      DPRINT ("Shadow state at the completion :\n");
      DPRINT ("Block number      : 0123456789AB\n");
      DPRINT ("Shadow blk valid  : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->database_shadow_map[i].valid);
      DDPRINT ("\n");
      DPRINT ("Num clones target : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->database_shadow_map[i].no_clones_target);
      DDPRINT ("\n");
      DPRINT ("Num clones currnt : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->database_shadow_map[i].no_clones_current);
      DDPRINT ("\n");
      DPRINT ("Db_id             : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->database_shadow_map[i].db_id);
      DDPRINT ("\n");
      DPRINT ("Virt block no     : ");
      for (i = 0; i < phy_device->num_fms_blocks; i++)
        DDPRINT ("%01x", phy_device->database_shadow_map[i].virt_blk_no);
      DDPRINT ("\n");

      return NLM_OK;
    }
  else
    return NLM_LOADER_IN_PROGRESS;
}

static void
nlm_device_loader_cmd_add (nlm_phy_device_t *phy_device, struct fms_loader_command cmd)
{
  DPRINT ("rd_ptr: %02d, wr_ptr: %02d, opcode: %02x, addr_array: %01x, addr_shadow: %01x\n",
          phy_device->cmd_rd_ptr, phy_device->cmd_wr_ptr, cmd.opcode, cmd.addr_array, cmd.addr_shadow);

  if (((phy_device->cmd_wr_ptr + 1) % NLM_FMS_LOADER_CMD_BUFF_SIZE) == phy_device->cmd_rd_ptr)
    pretty_assert (0 && "Loader command FIFO overrun");

  phy_device->loader_cmd_mem[phy_device->cmd_wr_ptr] = cmd;
  phy_device->cmd_wr_ptr = ++phy_device->cmd_wr_ptr % NLM_FMS_LOADER_CMD_BUFF_SIZE;
}

static void
nlm_device_loader_clone (nlm_phy_device_t *phy_device, uint32_t addr_shadow)
{
  struct fms_loader_command cmd = {0};
  uint32_t array_block_no;
  /* Create the command list */
  DPRINT ("Clone shadow block %02d to ", addr_shadow);

  /* Find free array block */
  for (array_block_no = 0;
        array_block_no < phy_device->num_fms_blocks && phy_device->array_block_map_target[array_block_no];
        array_block_no++);

  DDPRINT ("array block %02d\n", array_block_no);
  pretty_assert (array_block_no < phy_device->num_fms_blocks);

  phy_device->array_block_map_target[array_block_no] = 1;
  DPRINT ("Number of target clones is %d ",
          phy_device->database_shadow_map[addr_shadow].no_clones_target);
  phy_device->database_shadow_map[addr_shadow].no_clones_target++;
  DDPRINT ("increasing to %d\n",
           phy_device->database_shadow_map[addr_shadow].no_clones_target);

  /* Copy block from shadow to the array */
  cmd.addr_shadow = addr_shadow;
  cmd.addr_array = array_block_no;
  cmd.opcode = FMS_LOADER_CMD_DMA_XFER_START;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Wait for the DMA transfer to complete */
  cmd.opcode = FMS_LOADER_CMD_DMA_XFER_COMPLETE_WAIT;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Load the action table */
  cmd.opcode = FMS_LOADER_CMD_ACTION_TABLE_LOAD;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Assign Virtual Block ID */
  cmd.opcode = FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Wait for Assign Virtual Block ID completion */
  cmd.opcode = FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID_WAIT;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Undelete block */
  cmd.opcode = FMS_LOADER_CMD_ARRAY_ADD;
  nlm_device_loader_cmd_add (phy_device, cmd);

}

static void
nlm_device_loader_declone (nlm_phy_device_t *phy_device, uint32_t addr_array, uint32_t addr_shadow)
{
  struct fms_loader_command cmd = {0};
  /* Create the command list */
  DPRINT ("Declone array block %d, shadow ref %d\n", addr_array, addr_shadow);

  phy_device->array_block_map_target[addr_array] = 0;
  DPRINT ("Number of target clones is %d ",
          phy_device->database_shadow_map[addr_shadow].no_clones_target);
  phy_device->database_shadow_map[addr_shadow].no_clones_target--;
  DDPRINT ("reducing to %d\n",
           phy_device->database_shadow_map[addr_shadow].no_clones_target);

  cmd.addr_array = addr_array;
  cmd.addr_shadow = addr_shadow;

  /* Pause block */
  cmd.opcode = FMS_LOADER_CMD_PAUSE;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Wait for the block to pause */
  cmd.opcode = FMS_LOADER_CMD_PAUSE_WAIT;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Delete block */
  cmd.opcode = FMS_LOADER_CMD_ARRAY_DELETE;
  nlm_device_loader_cmd_add (phy_device, cmd);

  /* Resume block */
  cmd.opcode = FMS_LOADER_CMD_RESUME;
  nlm_device_loader_cmd_add (phy_device, cmd);
}

nlm_status
nlm_device_database_get_num_copies (nlm_device_t *device, uint32_t database_id,
                                    uint32_t *num_copies_current)
{
  uint32_t num_copies = 0;
  int i;
  nlm_phy_device_t *phy_device;
  
  if (NULL == device)
    return NLM_INVALID_ARGUMENT;

  if (0 != device->phy_device->fms_ring_id) /* only master ring (ring #0) can load/unload database */
    return NLM_INVALID_RING_ID;

  if (0 != device->thread_id)
    return NLM_INVALID_THREAD_ID;
  
  if (NLM_MAX_DATABASE_ID < database_id)
    return NLM_INVALID_DATABASE_ID;

  phy_device = device->phy_device;
  
  for (i = 0; i < phy_device->num_fms_blocks; i++)
    if (phy_device->database_shadow_map[i].valid && phy_device->database_shadow_map[i].db_id == database_id)
    {
      DPRINT ("Number of current block clones is %d, number of clones scheduled %d\n",
                phy_device->database_shadow_map[i].no_clones_current,
                phy_device->database_shadow_map[i].no_clones_target);
      if (phy_device->database_shadow_map[i].no_clones_current
          != phy_device->database_shadow_map[i].no_clones_target)
        return NLM_BUSY;
      else
        num_copies = phy_device->database_shadow_map[i].no_clones_current;
    }

  if (num_copies == 0)
    {
      /* Make sure that there are no copies as some functions use
         nlm_device_create_shadow_block_list_by_db_id to find database
         is loaded or not.  */

      uint32_t num_blocks_in_this_db;
      uint32_t block_list[TOTAL_FAMOS_BLOCKS];
      nlm_device_create_shadow_block_list_by_db_id (phy_device, database_id,
                                                    &num_blocks_in_this_db, block_list);
      pretty_assert (num_blocks_in_this_db == 0);
    }

  *num_copies_current = num_copies;
  return (0 == num_copies) ? NLM_DATABASE_UNLOADED : nlm_device_loader_cmd_exec_fms (device);
}

nlm_status
nlm_device_database_set_num_copies (nlm_device_t *device, uint32_t database_id,
                                    uint32_t num_copies)
{
  int32_t i, j, delta = 0, old_delta = 0;
  struct fms_loader_command cmd = {0};
  int32_t array_blocks_required;
  uint32_t num_blocks_in_this_db;
  uint32_t block_list[TOTAL_FAMOS_BLOCKS];
  nlm_phy_device_t *phy_device;
  
  if (NULL == device)
    return NLM_INVALID_ARGUMENT;

  if (0 != device->phy_device->fms_ring_id) /* only master ring (ring #0) can load/unload database */
    return NLM_INVALID_RING_ID;

  if (0 != device->thread_id)
    return NLM_INVALID_THREAD_ID;
  
  if (NLM_MAX_DATABASE_ID < database_id)
    return NLM_INVALID_DATABASE_ID;

  if (num_copies > 4)
    return NLM_INVALID_ARGUMENT;

  phy_device = device->phy_device;
  DPRINT ("Resident size change to %d is requested for database %d\n", num_copies, database_id);

  nlm_device_create_shadow_block_list_by_db_id (phy_device, database_id,
                                                  &num_blocks_in_this_db, block_list);
  if (0 == num_blocks_in_this_db)
    return NLM_INVALID_DATABASE_ID;

  for (array_blocks_required = 0, i = 0; i < num_blocks_in_this_db; i++)
    array_blocks_required
      += num_copies - phy_device->database_shadow_map[block_list[i]].no_clones_target;

  if (array_blocks_required > 0)
    {
      int32_t k, num_free_array_blocks;
      for (k = 0, num_free_array_blocks = 0; k < phy_device->num_fms_blocks; k++)
        num_free_array_blocks += (0 == phy_device->array_block_map_target[k]);

      if (array_blocks_required > num_free_array_blocks)
        {
          DPRINT ("Number of blocks required %d, but there are only %d free\n",
                    array_blocks_required, num_free_array_blocks);
          return NLM_INVALID_DATABASE_SIZE;
        }
    }

  for (i = 0; i < num_blocks_in_this_db; i++)
    {
      pretty_assert (delta == old_delta);
      delta = num_copies - phy_device->database_shadow_map[block_list[i]].no_clones_target;
      if (!i)
        old_delta = delta;
      DPRINT ("Shadow block %d has %d clones\n",
                block_list[i], phy_device->database_shadow_map[block_list[i]].no_clones_current);
      if (phy_device->database_shadow_map[block_list[i]].no_clones_current
          != phy_device->database_shadow_map[block_list[i]].no_clones_target)
        DPRINT ("And has an unprocessed request to change its clone count to %d\n",
                  phy_device->database_shadow_map[block_list[i]].no_clones_target);
        DPRINT ("Count delta is %d\n", delta);
    }

  if (0 < delta)
    { /* Grow */
      for (j = 0; j != delta; j++)
        {
          for (i = 0; i < num_blocks_in_this_db; i++)
            nlm_device_loader_clone (phy_device, block_list[i]);

          /* lock/configure/unlock GID_BMAP */
          cmd.opcode = FMS_LOADER_CMD_GID_BMAP_CONFIGURE;
          cmd.addr_shadow = block_list[0];
          cmd.addr_array = phy_device->database_shadow_map[block_list[0]].no_clones_target;
          nlm_device_loader_cmd_add (phy_device, cmd);
        }
    }
  else
    { /* Shrink */
      LOCK_GID_BMAP ();
      for (i = 0; i < num_blocks_in_this_db; i++) /* Shadow iterator */
        {
          int32_t blk_delta = delta;
          while (blk_delta) /* All the copies may or may not be found in one pass */
            for (j = 0; j < phy_device->num_fms_blocks && blk_delta; j++) /* array iterator */
              {
                DPRINT (
                  "Array block %02d, virtual id %d, target map: %d, current map: %d copy #%d\n",
                    j,
                    phy_device->array_virt_ids[j],
                    phy_device->array_block_map_target[j],
                    phy_device->array_block_map_current[j],
                    phy_device->array_block_copy_number[j]);

                if (block_list[i] == phy_device->array_virt_ids[j]
                    && phy_device->array_block_map_current[j]
                    && phy_device->database_shadow_map[block_list[i]].no_clones_target
                       == phy_device->array_block_copy_number[j] + 1)
                  {
                    pretty_assert (phy_device->database_shadow_map[block_list[i]].no_clones_target);
                    DPRINT ("Deleting array blk %d, shadow ref %d, copy #%d, delta %d, blk %d\n",
                                     j, block_list[i], phy_device->array_block_copy_number[j], delta,
                                      blk_delta);
                    nlm_device_block_scheduler_delete (phy_device, block_list[i], j,
                                                         1 << phy_device->array_block_copy_number[j]);
                    nlm_device_loader_declone (phy_device, j, block_list[i]);
                    blk_delta++;
                  }
              }
        }
      UNLOCK_GID_BMAP ();
    }

  if (NLM_DATABASE_LOAD_POLICY_ASYNC != phy_device->database_load_policy)
    {
      nlm_status status;
      do
        {
          status = nlm_device_loader_cmd_exec_fms (device);
          if (NLM_LOADER_IN_PROGRESS != status)
            return status;
        }
      while (NLM_LOADER_IN_PROGRESS == status);
    }

  return nlm_device_loader_cmd_exec_fms (device);
}

nlm_status
nlm_device_database_balance (nlm_device_t *device)
{
  int i, shadow_blocks_occupied, num_copies;
  uint32_t len;
  uint32_t block_list[TOTAL_FAMOS_BLOCKS];
  nlm_status status = NLM_OK;
  nlm_phy_device_t *phy_device;
  
  if (NULL == device)
    return NLM_INVALID_ARGUMENT;

  if (0 != device->phy_device->fms_ring_id) /* only master ring (ring #0) can load/unload database */
    return NLM_INVALID_RING_ID;
  
  if (0 != device->thread_id)
    return NLM_INVALID_THREAD_ID;

  /*  The current automatic load policy is to use as much of the device space as there is idle;
      if there is insufficient space even for a single copy and there is another database loaded
      then loader will shrink existing database in order to fairly fit the loading one. */
  phy_device = device->phy_device;
  for (shadow_blocks_occupied = 0, i = 0; i < phy_device->num_fms_blocks; i++)
    shadow_blocks_occupied += (0 != phy_device->database_shadow_map[i].valid);

  DPRINT ("Shadow blocks used: %d, shadow blocks free: %d\n", shadow_blocks_occupied,
          phy_device->num_fms_blocks - shadow_blocks_occupied);

  if (0 == shadow_blocks_occupied)
    return NLM_INVALID_DATABASE_ID;

  num_copies = phy_device->num_fms_blocks / shadow_blocks_occupied;
  if (num_copies > 4)
    num_copies = 4;

  DPRINT ("Decided to balance all copies to %d\n", num_copies);
  /* Shrink resident sizes for databases that can */
  for (i = 0; i <= NLM_MAX_DATABASE_ID; i++)
    {
      nlm_device_create_shadow_block_list_by_db_id (phy_device, i, &len, block_list);
      if (!len)
        continue;
      DPRINT ("Current number of database %d resident copies is %d\n", i,
              phy_device->database_shadow_map[block_list[0]].no_clones_current);
      if (phy_device->database_shadow_map[block_list[0]].no_clones_current > num_copies)
        {
          DPRINT ("Shrinking number of database %d resident copies to %d\n", i, num_copies);
          status = nlm_device_database_set_num_copies (device, i, num_copies);
          pretty_assert (NLM_OK == status || NLM_LOADER_IN_PROGRESS == status);
        }
    }

  for (i = 0; i <= NLM_MAX_DATABASE_ID; i++)
    {
      nlm_device_create_shadow_block_list_by_db_id (phy_device, i, &len, block_list);
      if (!len)
        continue;
      if (phy_device->database_shadow_map[block_list[0]].no_clones_current < num_copies)
        {
          DPRINT ("Growing number of database %d resident copies to %d\n", i, num_copies);
          status = nlm_device_database_set_num_copies (device, i, num_copies);
          pretty_assert (NLM_OK == status || NLM_LOADER_IN_PROGRESS == status);
        }
    }

  return status;
}

nlm_status
NLM_PROTO (nlm_device_load_database) (nlm_device_t *device, const void *database,
                                      uint32_t db_size, uint32_t database_id, uint32_t num_copies)
{
  uint32_t num_old_blocks, j;
  uint32_t array_blocks_free;
  uint32_t num_blocks = (db_size - sizeof (struct fms_file_sig)) / sizeof (struct fms_block_hw);
  uint32_t db_shadow_blk_list[TOTAL_FAMOS_BLOCKS];
  nlm_phy_device_t *phy_device;

  if (NULL == device)
    return NLM_INVALID_ARGUMENT;

  if (0 != device->phy_device->fms_ring_id) /* only master ring (ring #0) can load/unload database */
    return NLM_INVALID_RING_ID;

  if (0 != device->thread_id)
    return NLM_INVALID_THREAD_ID;

  if (NLM_MAX_DATABASE_ID < database_id)
    return NLM_INVALID_DATABASE_ID;

  phy_device = device->phy_device;
  if (NLM_LOADER_IN_PROGRESS == nlm_device_loader_cmd_exec_fms (device))
    return NLM_BUSY;

  for (j = 0; j < TOTAL_FAMOS_BLOCKS; j++)
    if ( (BLOCK_DENY_MASK >> j) & 1)
      {
        /* mark it as occupied */
        phy_device->array_block_map_target[j] = 1;
        phy_device->array_block_map_current[j] = 1;
        phy_device->array_virt_ids[j] = 0xf;
        phy_device->array_block_copy_number[j] = 0xf;
      }

  TRY (nlm_loader_fms_check_params_and_head (phy_device, database, db_size))

  nlm_device_create_shadow_block_list_by_db_id (phy_device, database_id,
                                                &num_old_blocks, db_shadow_blk_list);

  if (num_old_blocks)
    return phy_device->database_shadow_map[db_shadow_blk_list[0]].no_clones_target
      ? NLM_INVALID_ARGUMENT : NLM_BUSY;

  if (num_copies > 0)
    {
      int i;
      if (num_copies > 4)
        return NLM_INVALID_ARGUMENT;
      /*  There is a determined number of copies requested; check if the requested number
          of copies fits into the array.
      */
    for (array_blocks_free = 0, i = 0; i < phy_device->num_fms_blocks; i++)
      array_blocks_free += (0 == phy_device->array_block_map_target[i]);

    if (num_blocks * num_copies > array_blocks_free)
      return NLM_INVALID_DATABASE_SIZE;
  }

  TRY (nlm_device_write_database_to_shadow (phy_device, database_id, database, num_blocks))

  if (num_copies)
    TRY (nlm_device_database_set_num_copies (device, database_id, num_copies))
  else
    TRY (nlm_device_database_balance (device))

  if (NLM_DATABASE_LOAD_POLICY_ASYNC != phy_device->database_load_policy)
    {
      nlm_status status;
      do
        {
          status = nlm_device_loader_cmd_exec_fms (device);
          if (NLM_LOADER_IN_PROGRESS != status)
            return status;
        }
      while (NLM_LOADER_IN_PROGRESS == status);
    }

  return nlm_device_loader_cmd_exec_fms (device);
}

nlm_status
NLM_PROTO (nlm_device_unload_database) (nlm_device_t *device, uint32_t database_id)
{
  int i, j;
  uint32_t num_blocks = 0;
  struct fms_loader_command cmd = {0};
  uint32_t db_shadow_blk_list[TOTAL_FAMOS_BLOCKS];
  nlm_phy_device_t *phy_device;
  
  DPRINT ("Unloading database %d\n", database_id);

  if (NULL == device)
    return NLM_INVALID_ARGUMENT;

  if (0 != device->phy_device->fms_ring_id) /* only master ring (ring #0) can load/unload database */
    return NLM_INVALID_RING_ID;

  if (0 != device->thread_id)
    return NLM_INVALID_THREAD_ID;
  
  if (NLM_MAX_DATABASE_ID < database_id)
    {
      DPRINT ("DB ID %d is too big, returning\n", database_id);
      return NLM_INVALID_DATABASE_ID;
    }

  phy_device = device->phy_device;
  if (NLM_LOADER_IN_PROGRESS == nlm_device_loader_cmd_exec_fms (device))
    return NLM_BUSY;

 /* Walk database shadow map, identify blocks to kill and calculate required cmd buff size */
  nlm_device_create_shadow_block_list_by_db_id (phy_device, database_id,
                                                &num_blocks, db_shadow_blk_list);

  if (!num_blocks)
    return NLM_INVALID_DATABASE_ID;

  cmd.opcode = FMS_LOADER_CMD_SHADOW_DELETE;

  LOCK_GID_BMAP ();
  for (i = 0; i < num_blocks; i++)
    {
      cmd.addr_shadow = db_shadow_blk_list[i];
      for (j = 0; j < phy_device->num_fms_blocks; j++)
        {
          DPRINT ("Block %d virt id %d %s\n", j, phy_device->array_virt_ids[j],
                  phy_device->array_block_map_current[j] ? "valid" : "invalid");
          if (phy_device->array_block_map_current[j] && cmd.addr_shadow == phy_device->array_virt_ids[j])
            {
              DPRINT ("Deleting array blk %d, shadow ref %d, copy #%d\n",
                        j, cmd.addr_shadow, phy_device->array_block_copy_number[j]);
              nlm_device_block_scheduler_delete (phy_device, cmd.addr_shadow, j,
                                                  1 << phy_device->array_block_copy_number[j]);
              nlm_device_loader_declone (phy_device, j, cmd.addr_shadow);
            }
        }

        nlm_device_loader_cmd_add (phy_device, cmd);
    }
  UNLOCK_GID_BMAP ();

  if (NLM_DATABASE_LOAD_POLICY_ASYNC != phy_device->database_load_policy)
    {
      nlm_status status;
      do
        {
          status = nlm_device_loader_cmd_exec_fms (device);
          if (NLM_LOADER_IN_PROGRESS != status)
            return status;
        }
      while (NLM_LOADER_IN_PROGRESS == status);
    }

  return nlm_device_loader_cmd_exec_fms (device);
}
