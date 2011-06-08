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
#ifndef FMS_FORMATS_H
#define FMS_FORMATS_H

#include "nlm_stdint.h"
#include "nlm_fms_registers.h"
#include "nlm_byteswap.h"
#include "nlm_sha2.h"

#define PQ_ADDR_LOW_BITS 32

#define HW_COPY_PD_INTO_FD(PD, FD) \
  do \
    { \
      FD->packet_byte_count = PD->packet_byte_count; \
      FD->packet_base = PD->packet_base; \
      FD->packet_addr = PD->packet_addr; \
      FD->packet_prd = PD->packet_prd; \
      FD->flow_term = PD->flow_term; \
      FD->sw_disconnect = PD->sw_disconnect; \
    } \
  while (0);

#define HW_SET_INPUT_FIFO_BASE(ADDR) \
  do \
    { \
      SET_HW_REGISTER (FDF_BA_LSB, (uint32_t) (ADDR)); \
      SET_HW_REGISTER (FDF_BA_MSB, (uint32_t) (ADDR>>32)); \
    } \
  while (0);

#define HW_SET_INPUT_FIFO_SIZE(SIZE) \
  do { SET_HW_REGISTER (FDF_SIZE, (uint32_t) (SIZE)); } while (0);

#define HW_SET_OUTPUT_FIFO_BASE(ADDR) \
  do \
    { \
      SET_HW_REGISTER (RDF_BA_LSB, (uint32_t) (ADDR)); \
      SET_HW_REGISTER (RDF_BA_MSB, (uint32_t) (ADDR>>32)); \
    } \
  while (0);

#define HW_SET_OUTPUT_FIFO_SIZE(SIZE) \
  do { SET_HW_REGISTER (RDF_SIZE, (uint32_t) (SIZE)); } while (0);

#define HW_SET_PDF_BASE(ADDR) \
  do \
    { \
      SET_HW_REGISTER (PDF_BASE_LOW, (uint32_t) (ADDR)); \
      SET_HW_REGISTER (PDF_BASE_HI, (uint32_t) (ADDR>>32)); \
    } \
  while (0);

#define HW_SET_PD_BASE(ADDR) \
  do \
    { \
      SET_HW_REGISTER (PD_BASE_LOW, (uint32_t) (ADDR)); \
      SET_HW_REGISTER (PD_BASE_HI, (uint32_t) (ADDR>>32)); \
    } \
  while (0);

/* number of packet base addresses that needs to be configured
   via PQ_BASE_LOW_# and PQ_BASE_HIGH_# registers */
#define HW_PACKET_BASES 16
#define FAMOS_BLOCK_SIZE 0x4000 /**< Total number of rows in a block */
#define FAMOS_NUM_FD_HOST_RINGS 4 /**< Total number of host rings */
#define FAMOS_NUM_FD_XAUI_RINGS 2 /**< Total number of xaui rings */
#define FAMOS_NUM_FD_RINGS 6 /**< Total number of FD rings */
#define TOTAL_FAMOS_BLOCKS 12 /**< Total number of blocks the device has */
#define MAX_ACCEPT_PER_BLOCK 0x800 /**< Maximum number of accept states per block */
#define TOTAL_ACTION_TABLE_ENTRIES (TOTAL_FAMOS_BLOCKS * MAX_ACCEPT_PER_BLOCK)
/**< Total number of entries in the action table */
#define FAMOS_PREAMBLE_NUM_CTXT_BLOCKS 7
#define FAMOS_CTXT_BUFFER_NUM_CTXT_BLOCKS 15
#define FMS_MAX_WAIT_TIMEOUT 0xffff
#define FMS_MAX_DMA_WAIT_TIMEOUT 0xffffffff

typedef enum fms_flow_type
{
  FMS_FLOW_STATELESS = 0,  /**< FTYPE=000 is used for Stateless Flows */
  FMS_FLOW_INIT,           /**< FTYPE=001 is used for INIT FDs */
  FMS_FLOW_STATEFUL,       /**< FTYPE=010 is used for Stateful Flows */
  FMS_FLOW_FINISH,         /**< FTYPE=011 is used for an FEOF (End of Flow)
                                marking to terminate stateful flows.  This allows the
                                hardware to recover any linked context state buffers */
  FMS_FLOW_PASS_THROUGH,   /**< FTYPE=100 is a PTHR (pass through) packet.
                                It is not scanned by the engine and only
                                generates an FRD result on the output. */
  FMS_FLOW_REINIT          /**< FTYPE=101 free cntx buffers for the flow and zero pdf */
} fms_flow_type;

typedef enum fms_result_type
{
  FMS_MATCH_RESULT = 0,   /* RTYPE[2:0] = 0 is a Match Result */
  FMS_FRD_RESULT,         /* RTYPE[2:0] = 1 is a FRD Result */
  FMS_PRD_RESULT,         /* RTYPE[2:0] = 2 is a PRD Result */
  FMS_ACTION_SWAP_RESULT, /* RTYPE[2:0] = 3 is an Action Swap Result */
  FMS_ERD_RESULT          /* RTYPE[2:0] = 4 is a ERD Result */
} fms_result_type;

typedef enum fms_context_type
{
  FMS_STATE_CNXT = 0, /* CTYPE[2:0] = 0 contains State Information */
  FMS_COUNTER_CNXT,   /* CTYPE[2:0] = 1 contains Counter Information */
  FMS_GID_CNXT        /* CTYPE[2:0] = 2 contains GID Information */
} fms_context_type;

typedef enum fms_asm_product_id
{
  FMS_ASM_FAMOS_FPGA = 0x46534d46,
  FMS_ASM_FAMOS_ASIC = 0x41534d46
} fms_asm_product_id;

typedef enum fms_asm_input_state
{
  FMS_ASM_EXPECT_VIRT_BLOCK_ID = 0,
  FMS_ASM_EXPECT_ANY,
  FMS_ASM_FILL = '.',
  FMS_ASM_FIRST_I_ROW = 'i',
  FMS_ASM_TCAM_O_ROW = 'o',
  FMS_ASM_CONN_ROW = 'V',
  FMS_ASM_CNTR_O1_ROW = '2',
  FMS_ASM_CNTR_I2_ROW = '3',
  FMS_ASM_CNTR_O2_ROW = '4',
  FMS_ASM_CNTR_I3_ROW = '5',
  FMS_ASM_CNTR_O3_ROW = '6'
} fms_asm_input_state;

typedef enum fms_asm_row_type
{
  FMS_ASM_INVALID_ROW = 0,
  FMS_ASM_TCAM_ROW,
  FMS_ASM_CNTR_ROW
} fms_asm_row_type;

typedef enum fms_asm_hash_type
{
  FMS_ASM_HASH_LSB = 0,
  FMS_ASM_HASH_CRC,
  FMS_ASM_HASH_XOR
} fms_asm_hash_type;

struct hw_capabilities0 /* Hardware capabilities as described in reg 0x00A0 */
{
  __ENDIAN_ORDER14(
  uint32_t num_tdm_slots:4,
  uint32_t num_ctxt_pools:4,
  uint32_t num_rd_rings:4,
  uint32_t num_fd_rings:4,
  uint32_t reserved:7,
  uint32_t no_stats:1,
  uint32_t no_confpin:1,
  uint32_t no_jtag:1,
  uint32_t no_i2c:1,
  uint32_t no_coproc:1,
  uint32_t no_dmatest:1,
  uint32_t no_ddr3:1,
  uint32_t no_xaui2:1,
  uint32_t no_xaui1:1)
};

struct hw_capabilities1 /* Hardware capabilities as described in reg 0x00A0 */
{
  __ENDIAN_ORDER3(
  uint32_t num_rows:16,
  uint32_t num_blocks:8,
  uint32_t num_dma_channels:8)
};

union hw_fd_ring_config
{
  uint32_t  word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t fdf_headers_are_big_endian:1,
    uint32_t disable_fdf_rdf_wrt:1,
    uint32_t fd_internal_mem_map:1,
    uint32_t rsv:13,
    uint32_t ring_id:4,
    uint32_t pdf_pq_id:4,
    uint32_t cnxt_ring_id:4,
    uint32_t rslt_ring_id:4
    )
  } s;
};

union hw_cnxt_ctrl
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t enable:1,
    uint32_t cnxt_data_internal_memmap:1,
    uint32_t cnxt_ring_internal_memmap:1,
    uint32_t cnxt_ring_big_endian:1,
    uint32_t cnxt_burst_align:2,
    uint32_t free_align:2,
    uint32_t max_wrt_buffer_timer:8,
    uint32_t cnxt_max_state_num:16
    )
  } s;
};

union hw_cnxt_fifo_cnt
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t rd_fifo_level:5,
    uint32_t rsv01:11,
    uint32_t wrt_fifo_level:5,
    uint32_t rsv02:11
    )
  } s;
};

union hw_cnxt_pop_ctrl
{
  uint32_t      word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t  buff_pop:1,
    uint32_t  rsv01:1,
    uint32_t  pop_pass:1,
    uint32_t  rsv02:29
    )
  } s;
};

union hw_indirect_ctrl
{
  uint32_t   word32;
  struct
  {
  __ENDIAN_ORDER8(
  uint32_t  indir_access_byte_count:5,
  uint32_t  rsv01:3,
  uint32_t  is_read:1,
  uint32_t  rsv02:3,
  uint32_t  is_internal:1,
  uint32_t  rsv03:3,
  uint32_t  pcie_param:5,
  uint32_t  rsv04:11
  )
  } s;
};

union hw_dma_ctrl {
  uint32_t   word32;
  struct
  {
  __ENDIAN_ORDER7(
  uint32_t  size:24,
  uint32_t  param_tc:3,
  uint32_t  param_ro:1,
  uint32_t  param_ns:1,
  uint32_t  rsv_ro:1,
  uint32_t  is_read_internal_addr:1,
  uint32_t  is_write_internal_addr:1
  )
  } s;
};

union hw_pdf_memmap_config {
  uint32_t word32;
  struct
  {
  __ENDIAN_ORDER4(
  uint32_t  rsv03:14,
  uint32_t  rsv02:1,
  uint32_t  pdf_internal_mem_map:1,
  uint32_t  rsv01:16
  )
  } s;
};

struct hw_pd_nla /* 4 byte pointer to the next PD: common last 4 bytes of FD and PD descriptors */
{
  __ENDIAN_ORDER2(
  uint32_t rsv_cookie:8,
  uint32_t pd_nla_index:24)
};

struct hw_fd_descriptor /* flow descriptor 32 bytes, v. 2008-09-23 */
{
  __ENDIAN_ORDER5(
  fms_flow_type flow_type:3,
  uint32_t end_anchor:1,
  uint32_t start_anchor:1,
  uint32_t gid_cnt:3,
  uint32_t pdf_index:24)

  __ENDIAN_ORDER4(
  uint32_t rsv11:4,
  uint32_t gid1:12,
  uint32_t rsv12:4,
  uint32_t gid0:12)

  __ENDIAN_ORDER4(
  uint32_t rsv21:4,
  uint32_t gid3:12,
  uint32_t rsv22:4,
  uint32_t gid2:12)

  __ENDIAN_ORDER5(
  uint32_t error:1,
  uint32_t rsv31:3,
  uint32_t gid5:12,
  uint32_t rsv32:4,
  uint32_t gid4:12)

  __ENDIAN_ORDER6(
  uint32_t packet_base:4,
  uint32_t rsv05:9,
  uint32_t sw_disconnect:1,
  uint32_t packet_prd:1,
  uint32_t flow_term:1,
  uint32_t packet_byte_count:16)

  uint32_t packet_addr;

  uint32_t cookie;

  struct hw_pd_nla pd_nla;
};

struct hw_pd_descriptor /* packet descriptor 16 bytes */
{
  __ENDIAN_ORDER6(
  uint32_t packet_base:4,
  uint32_t rsv01:9,
  uint32_t sw_disconnect:1,
  uint32_t packet_prd:1,
  uint32_t flow_term:1,
  uint32_t packet_byte_count:16)

  uint32_t packet_addr;

  uint32_t cookie;

  struct hw_pd_nla pd_nla;
};

struct hw_pdf_descriptor /* pdf preamble 128 bytes */
{
#ifdef NLM_SW_FMS_MODEL
  union
    {
      struct fms_search_state *search_state;
      struct fms_engine_state *engine_state;
      struct fms_mars3_engine_state *mars3_engine_state;
    } u;
  uint32_t pdf_dumped:1;
  uint32_t search_state_created:1;
  uint32_t padding[(128 - sizeof (void *) - 4) / 4];
#else
  __ENDIAN_ORDER7(
  uint32_t busy:1,
  uint32_t rsv01:1,
  uint32_t rsv02:1,
  uint32_t error:5,
  uint32_t rsv03:15,
  uint32_t bof:1,
  uint32_t byte_offset_hi:8)

  uint32_t byte_offset_lo;

  __ENDIAN_ORDER2(
  uint32_t rsv04:16,
  uint32_t state_cnt:16)

  __ENDIAN_ORDER2(
  uint32_t rsv05:4,
  uint32_t next_cnxt_addr:28)

  uint32_t cnxt_block[FAMOS_PREAMBLE_NUM_CTXT_BLOCKS][4];
#endif
};

struct hw_cnxt_buffer {
  uint32_t block[FAMOS_CTXT_BUFFER_NUM_CTXT_BLOCKS][4];
  struct {
    __ENDIAN_ORDER2(
    uint32_t rsv01:4,
    uint32_t next_cnxt_addr:28)
  } next;
};

struct hw_result_descriptor /* result descriptor 16 bytes*/
{
  union
    {
      struct
        {
          __ENDIAN_ORDER3(
          fms_result_type result_type:3,
          uint32_t rsv02:5,
          uint32_t byte_offset_lo:24)
        } hw;
      struct
        {
          __ENDIAN_ORDER3(
          fms_result_type result_type:3,
          uint32_t error_code:5,
          uint32_t timestamp_lo:24)
        } hw_frd;
    } u1;

  uint32_t cookie;

  union
    {
      struct
        {
          __ENDIAN_ORDER2(
          uint32_t match_length:16,
          uint32_t rule_id:16)
        } hw;
      uint32_t hw_frd_stat0;
      struct
        {
          __ENDIAN_ORDER4(
          uint32_t rsv05:10,
          uint32_t blk_index:4,
          uint32_t vb_oset:4,
          uint32_t array_index:14)
        } hw_erd;
    } u3;

  union
    {
      struct
        {
          __ENDIAN_ORDER5(
          uint32_t valid:1,
          uint32_t end_anchor:1,
          uint32_t priority:2,
          uint32_t gid:12,
          uint32_t byte_offset_hi:16)
        } hw;
      struct
        {
          __ENDIAN_ORDER6(
          uint32_t valid:1,
          uint32_t ex2_xauierr:1,
          uint32_t ex2_passthrough:1,
          uint32_t ex2_rd_clamp:1,
          uint32_t stat1:12,
          uint32_t timestamp_hi:16)
        } hw_frd;
      struct
        {
          __ENDIAN_ORDER4(
          uint32_t valid:1,
          uint32_t rsv04:7,
          uint32_t replay_flags:8,
          uint32_t byte_offset_hi:16)
        } hw_replay_rd;
      uint32_t valid_sw;
     } u4;
};

struct fms_xfd
{
  __ENDIAN_ORDER4(
  fms_flow_type flow_type:3,
  uint8_t end_anchor:1,
  uint8_t start_anchor:1,
  uint8_t gid_cnt:3)

  uint8_t h_offset;
  uint8_t pad_cnt;
  uint8_t cookie;
  uint8_t pdf_index_hi;
  uint8_t pdf_index_mi;
  uint8_t pdf_index_lo;

  __ENDIAN_ORDER2(
  uint8_t gid1_hi:4,
  uint8_t gid0_hi:4)
  uint8_t gid0_lo;
  uint8_t gid1_lo;

  __ENDIAN_ORDER2(
  uint8_t gid3_hi:4,
  uint8_t gid2_hi:4)
  uint8_t gid2_lo;
  uint8_t gid3_lo;

  __ENDIAN_ORDER2(
  uint8_t gid5_hi:4,
  uint8_t gid4_hi:4)
  uint8_t gid4_lo;
  uint8_t gid5_lo;
};

struct xaui_pckt
{
  struct fms_xfd xfd;
  uint8_t payload[16384];
};

struct fms_xaui_result_header
{
  __ENDIAN_ORDER2(
  fms_flow_type flow_type:3,
  uint8_t error:5)

  uint8_t rd_cnt;
  uint8_t cookie;

  __ENDIAN_ORDER3(
  uint8_t rsv01:3,
  uint8_t priority_clamp:4,
  uint8_t result_clamp:1)
};

struct fms_xaui_result
{
  __ENDIAN_ORDER3(
  uint8_t rsv01:1,
  uint8_t ngrd:1,
  uint8_t byte_offset_hi:6)

  uint8_t byte_offset_lo;

  uint16_t match_id;

  __ENDIAN_ORDER4(
  uint8_t packet_erd:1,
  uint8_t end_anchor:1,
  uint8_t priority:2,
  uint8_t gid_hi:4)

  uint8_t gid_lo;
};

struct fms_state_cnxt_block
{
  __ENDIAN_ORDER7(
  fms_context_type ctype:3,
  uint32_t comp:1,
  uint32_t cnt:4,
  uint32_t vb_offset:4,
  uint32_t rsv01:4,
  uint32_t s0:14,
  uint32_t s1_lo:2)

  __ENDIAN_ORDER3(
  uint32_t s1_hi:12,
  uint32_t s2:14,
  uint32_t s3_lo:6)

  __ENDIAN_ORDER3(
  uint32_t s3_hi:8,
  uint32_t s4:14,
  uint32_t s5_lo:10)

  __ENDIAN_ORDER3(
  uint32_t s5_hi:4,
  uint32_t s6:14,
  uint32_t s7_lo:14)
};

struct fms_counter_cnxt_block
{
  __ENDIAN_ORDER6(
  fms_context_type ctype:3,
  uint32_t comp:1,
  uint32_t cnt:4,
  uint32_t vb_offset:4,
  uint32_t rsv01:1,
  uint32_t c0_lo:19)

  __ENDIAN_ORDER3(
  uint32_t c0_hi:6,
  uint32_t c1:25,
  uint32_t c2_lo:1)

  __ENDIAN_ORDER2(
  uint32_t c2_hi:24,
  uint32_t c3_lo:8)

  __ENDIAN_ORDER2(
  uint32_t c3_hi:17,
  uint32_t rsv02:15)
};

struct fms_gid_cnxt_block
{
  __ENDIAN_ORDER7(
  fms_context_type ctype:3,
  uint32_t comp:1,
  uint32_t cnt:4,
  uint32_t rsv01:10,
  uint32_t do_end_anc0:1,
  uint32_t do_start_anc0:1,
  uint32_t gid0_value:12)

  uint32_t rsv02;

  __ENDIAN_ORDER4(
  uint32_t rsv03:18,
  uint32_t do_end_anc1:1,
  uint32_t do_start_anc1:1,
  uint32_t gid1_value:12)

  uint32_t rsv04;
};

struct fms_action_result_entry
{
  union
    {
      struct
        {
          __ENDIAN_ORDER7(
          uint32_t valid:1,
          uint32_t offset_ctrl:2,
          uint32_t dup_supr:1,
          uint32_t rsv01:1,
          uint32_t end_anchor:1,
          uint32_t gid:12,
          uint32_t array_index_comp:14)
        } s;
      uint32_t reg;
    } w0;

  union
    {
      struct
        {
          __ENDIAN_ORDER2(
          uint32_t match_len:16,
          uint32_t match_id:16)
        } s;
      uint32_t reg;
    } w1;

  union
    {
      struct
        {
          __ENDIAN_ORDER3(
          uint32_t priority:2,
          uint32_t dup_supr_indx:14,
          uint32_t offset_comp:16)
        } s;
      uint32_t reg;
    } w2;

  union
    {
      struct
        {
          __ENDIAN_ORDER4(
          uint32_t replay_flags:4,
          uint32_t replay_block_nla:12,
          uint32_t rsv31:4,
          uint32_t action_block_nla:12)
        } s;
      uint32_t reg;
    } w3;
};

struct fms_action_replay_entry
{
  __ENDIAN_ORDER3(
  uint32_t valid:1,
  uint32_t replay_op:8,
  uint32_t rsv01:23)

  __ENDIAN_ORDER4(
  uint32_t rsv11:18,
  uint32_t end_anchor:1,
  uint32_t start_anchor:1,
  uint32_t gid_value:12)

  uint32_t rsv20;

  __ENDIAN_ORDER3(
  uint32_t rsv31:4,
  uint32_t replay_block_nla:12,
  uint32_t rsv32:16)
};

struct fms_gid_bitmap_data
{
  union
    {
      struct
        {
          __ENDIAN_ORDER6(
          uint32_t valid0:1,
          uint32_t rsv00:3,
          uint32_t fms_block_bitmap0:12,
          uint32_t valid1:1,
          uint32_t rsv01:3,
          uint32_t fms_block_bitmap1:12)
        } s;
      uint32_t reg;
    } w0;

  union
    {
      struct
        {
          __ENDIAN_ORDER6(
          uint32_t valid2:1,
          uint32_t rsv02:3,
          uint32_t fms_block_bitmap2:12,
          uint32_t valid3:1,
          uint32_t rsv03:3,
          uint32_t fms_block_bitmap3:12)
        } s;
      uint32_t reg;
    } w1;
};

struct fms_row_accept_flags
{
  uint8_t accept;
  uint16_t priority;
  uint16_t rule_id;
  uint16_t min_offset;
  uint16_t exact_offset;
  uint16_t max_offset;
  uint16_t known_match_length;
  uint8_t end_anchor:1;
  uint16_t gid:12;
};

struct fms_cntr_row_hw
{
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t  rsvd00:12,
    uint32_t  mml:1,
    uint32_t  rsvd01:3,
    uint32_t  val:16)
  } w0;
  struct
  {
    __ENDIAN_ORDER11(
    uint32_t  rsvd14:2,
    uint32_t  accept:1,
    uint32_t  rsvd13:6,
    uint32_t  k3:2,
    uint32_t  c3:5,
    uint32_t  rsvd12:1,
    uint32_t  k2:2,
    uint32_t  c2:5,
    uint32_t  rsvd11:1,
    uint32_t  k1:2,
    uint32_t  c1:5)
  } w1i;
  struct
  {
    __ENDIAN_ORDER10(
    uint32_t  parity:1,
    uint32_t  rsvd13:8,
    uint32_t  k3:2,
    uint32_t  c3:5,
    uint32_t  rsvd12:1,
    uint32_t  k2:2,
    uint32_t  c2:5,
    uint32_t  rsvd11:1,
    uint32_t  k1:2,
    uint32_t  c1:5)
  } w2o;
};

struct fms_cntr_row
{
  uint16_t complete;
  uint16_t addr;

  uint16_t value;

  uint8_t parity; /**< 1-bit */
  uint8_t min_match_length; /**< 1-bit */

  uint16_t gid;

  struct fms_row_accept_flags accept_flags;

  struct
  {
    uint8_t c;
    uint8_t k;
  } i;
  struct
  {
    uint8_t c;
    uint8_t k;
  } o;
};

struct fms_tcam_row_hw
{
  struct
  {
    __ENDIAN_ORDER5(
    uint32_t  gid:12,
    uint32_t  rsvd02:1,
    uint32_t  Y:9,
    uint32_t  rsvd01:1,
    uint32_t  X:9)
  } w0;
  struct
  {
    __ENDIAN_ORDER13(
    uint32_t  rsvd13:1,
    uint32_t  negate:1,
    uint32_t  accept:1,
    uint32_t  start:1,
    uint32_t  rsvd12:5,
    uint32_t  s5:1,
    uint32_t  rsvd11:3,
    uint32_t  s2:3,
    uint32_t  s1:4,
    uint32_t  k:2,
    uint32_t  c:5,
    uint32_t  s8:4,
    uint32_t  v:1)
  } w1i;
  struct
  {
    __ENDIAN_ORDER10(
    uint32_t  parity:1,
    uint32_t  rsvd23:8,
    uint32_t  s5:1,
    uint32_t  rsvd22:3,
    uint32_t  s2:3,
    uint32_t  s1:4,
    uint32_t  k:2,
    uint32_t  c:5,
    uint32_t  s8:4,
    uint32_t  rsvd21:1)
  } w2o;
};

struct fms_tcam_row
{
  uint8_t complete;
  uint8_t parity;
  uint16_t addr;

  uint16_t mask;
  uint16_t data;

  uint8_t start;
  uint8_t negate;

  uint16_t v;

  uint16_t gid;

  struct fms_row_accept_flags accept_flags;

  struct
  {
    uint8_t s8;
    uint8_t s1;
    uint8_t s2;
    uint8_t s5;
    uint8_t c;
    uint8_t k;
  } i;
  struct
  {
    uint8_t s8;
    uint8_t s1;
    uint8_t s2;
    uint8_t s5;
    uint8_t c;
    uint8_t k;
  } o;
};

struct fms_file_sig
{
  uint32_t product_id;
  uint32_t major_version;
  uint32_t minor_version;
};

union fms_row_hw
{
  struct fms_tcam_row_hw tcam;
  struct fms_cntr_row_hw cntr;
  uint32_t data[3];
};

struct fms_block_hw
{
  union fms_row_hw row[FAMOS_BLOCK_SIZE];
  struct fms_action_result_entry action_table[MAX_ACCEPT_PER_BLOCK];
  struct nlm_chksum checksum;
};

struct fms_database_shadow_map_entry
{
  uint8_t valid:1;
  uint8_t no_clones_current;
  uint8_t no_clones_target;
  uint8_t db_id;
  uint8_t virt_blk_no;
};

typedef enum fms_loader_command_opcode
{
  FMS_LOADER_CMD_NOP = 0,
  FMS_LOADER_CMD_ARRAY_ADD,
  FMS_LOADER_CMD_ARRAY_DELETE,
  FMS_LOADER_CMD_SHADOW_DELETE,
  FMS_LOADER_CMD_PAUSE,
  FMS_LOADER_CMD_PAUSE_WAIT,
  FMS_LOADER_CMD_RESUME,
  FMS_LOADER_CMD_RESUME_WAIT,
  FMS_LOADER_CMD_DMA_XFER_START,
  FMS_LOADER_CMD_DMA_XFER_COMPLETE_WAIT,
  FMS_LOADER_CMD_ACTION_TABLE_LOAD,
  FMS_LOADER_CMD_ACTION_TABLE_CLEAR,
  FMS_LOADER_CMD_GID_BMAP_CONFIGURE,
  FMS_LOADER_CMD_GID_BMAP_DELETE,
  FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID,
  FMS_LOADER_CMD_ASSIGN_VIRT_BLK_ID_WAIT,
  FMS_LOADER_CMD_LAST
} fms_loader_command_opcode;

struct fms_loader_command /* */
{
  __ENDIAN_ORDER3(
  uint16_t opcode:8,
  uint16_t addr_array:4,
  uint16_t addr_shadow:4)
};

typedef enum ddr3_leveling_mode
{
  NONE = 0,
  WRITE,
  READ,
  GATE
} ddr3_leveling_mode;

typedef enum ddr3_leveling_request
{
  TERMREQ = 0,
  SETREQ,
  GETREQ,
  SETGETREQ
} ddr3_leveling_request;

typedef enum ddr3_leveling_transition
{
  S0X = 0,
  S1X,
  SX0,
  SX1
} ddr3_leveling_transition;

typedef enum ddr3_leveling_tristate
{
  VAL_0 = 0,
  VAL_1,
  VAL_Z,
  VAL_X
} ddr3_leveling_tristate;

struct ddr3_state /* DDR3 state */
{
  uint32_t m_valid;
  uint32_t m_save_responses;        /* Init to 1 */
  uint32_t m_cs_rank;               /* Init to 0 */
  uint32_t m_in_preamble;           /* Init to 1 */
  uint32_t m_step_size;             /* coarse delay increment */
  uint32_t m_retry_count;           /* number of times to reread a response. Init to 1 */
  uint32_t m_jitter_range;          /* range required for consistent response Init to 1 */
  int32_t m_wrlvl_swhw;             /* Init to -1; */
  int32_t m_rdlvl_swhw;             /* Init to -1; */
  int32_t m_gtlvl_swhw;             /* Init to -1; */
  uint32_t sim_ps_period;           /* Init to 1; */
  uint32_t m_is_gatesim;            /* Init to 0 */
  uint32_t m_fast;                  /* Init to 0 */
  uint32_t m_wrlvl_is_leveled;      /* initial write leveling complete. Init to 0 */
  uint32_t m_rdlvl_is_leveled;      /* initial read leveling complete. Init to 0 */
  uint32_t m_gtlvl_is_leveled;      /* initial gate leveling complete. Init to 0 */
  uint32_t dll_is_macro;
  uint32_t m_min_delay;
  uint32_t m_max_delay;
  uint32_t m_max_slice;                 /* Init to 0 */
  uint32_t tdqsq_delay;
  uint32_t shift;
  ddr3_leveling_mode leveling_type_status;
  ddr3_leveling_transition edges[4];
  ddr3_leveling_request m_request[NLM_DDR3_MAX_SLICE];   // request response I/O
  uint32_t m_resp[NLM_DDR3_MAX_SLICE];          // latest response value
  uint32_t m_response[NLM_DDR3_MAX_SLICE][NLM_DDR3_MAX_DLL_DELAY];       // response values
  uint32_t m_delay[NLM_DDR3_MAX_SLICE];         // delay value
  uint32_t m_dll_ctrl_0[NLM_DDR3_MAX_SLICE];    // dll control register 0 values
  uint32_t m_phy_ctrl_0[NLM_DDR3_MAX_SLICE];    // phy control register 1 values
  uint32_t m_phy_ctrl_1[NLM_DDR3_MAX_SLICE];    // phy control register 1 values
  uint32_t m_data_delay[NLM_DDR3_MAX_CS][NLM_DDR3_MAX_SLICE]; // memory data path delays per [cs][slice]
  uint32_t m_ctrl_delay[NLM_DDR3_MAX_CS][NLM_DDR3_MAX_SLICE]; // memory control path delays per [cs][slice]
  uint32_t m_wrlvl_delay[NLM_DDR3_MAX_SLICE][NLM_DDR3_MAX_SLICE_SUB];    // write leveling delays
  uint32_t m_gtlvl_delay[NLM_DDR3_MAX_SLICE][NLM_DDR3_MAX_SLICE_SUB];    // gate leveling delays
  uint32_t m_rdlvl_delay[2][NLM_DDR3_MAX_SLICE][NLM_DDR3_MAX_SLICE_SUB]; // read leveling delays
  uint32_t m_wrlvl_startVal[NLM_DDR3_MAX_SLICE];        // write leveling pattern startVal
  uint32_t m_gtlvl_startVal[NLM_DDR3_MAX_SLICE];        // gate leveling pattern startVal
  uint32_t m_rdlvl_startVal[2][NLM_DDR3_MAX_SLICE];     // read leveling pattern startVal
};

#endif /* #define FMS_FORMATS_H */

