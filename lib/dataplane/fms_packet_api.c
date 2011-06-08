/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include <string.h>
#ifndef DEBUG
//#define DEBUG
#endif
#include <stdarg.h>
#include "nlm_system.h"
#include "nlm_packet_api_impl.h"

#ifndef api_assert
#define api_assert pretty_assert
#endif

#ifdef __mips
#define NLM_BIG_ENDIAN 1
#else
#define NLM_BIG_ENDIAN 0
#endif

#ifdef DEBUG
#define NLM_DEBUG 1
#else
#define NLM_DEBUG 0
#endif

//#define NLM_PRBS_TEST

#define NLM_WASTE_TIME \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG); \
        HW_REGISTER (SYS_DATE_REG);

/*

  SYSTEM MEMORY LAYOUT:
  =====================

        |                                  | <<== Top of system memory proviced by user
        |                                  |
        |                                  |
        |                                  |
        |             ^                    |
        |             |                    |
        |      Thread_id=n's PDF           |  
        |                                  |      
        |             ^                    |
        |             |                    |
        |      Thread_id=n's PD's          |                 |
        |_________________________________ | <<== DP Thread_id=n start sysmem
        |                                  |
        |                                  |
        |             ^                    |
        |             |                    |
        |      Thread_id=1's PDF           |  
        |                                  |      
        |             ^                    |
        |             |                    |
        |      Thread_id=1's PD's          |
        |                                  |
        |_________________________________ | <<== DP Thread_id=n-1 start sysmem   
        |                                  |
        |                                  |
        |             ^                    |
        |             |                    |
        |      Thread_id=1's PDF           |  
        |                                  |      
        |             ^                    |
        |             |                    |
        |      Thread_id=1's PD's          |
        |_________________________________ | <<== DP Thread_id=0 start sysmem
        |                                  |      Also PDF BASE
        |                                  |
        |                                  |
        |     Software Fifo's              |  
        | (optional when using manager)    |
        |                                  |
        |__________________________________|
        |                                  |
        |     RD fifo (hw input)           |
        |                                  |
        |                                  |
        |__________________________________|
        |                                  |
        |                                  |
        |      FD fifo (hw input)          |
        |                                  |
        |                                  |
        |                                  |
        |__________________________________|
        |                                  |
        |    context_save_restore_area     |
        |                                  |
        |    database_extension_area       |
        |                                  |
        |     output_wrt_ptr               |
        |                                  |
        |     intput_rd_ptr                |    
        |_________________________________ |    
        | nlm_physical_device structure    |
        |_________________________________ | <<== System memory start 
 

   FD fifo (hw input)
   ----------
   fd
   fd  <- input_rd_ptr (changed by hardware)
   fd
   fd  <- input_wrt_ptr (changed by software and written into hw_register)
   fd
   ...
   fd

   RD fifo (hw output)
   ----------
   rd
   rd
   rd  <- output_rd_ptr (changed by software and written into hw_register)
   rd
   rd  <- output_wrt_ptr (changed by hardware)
   ...
   rd

  USER MEMORY LAYOUT:
  ===================


         __________________________________
        |  nlm_job structure allocation    |  << Top of user memory
        |             |                    |
        |             |                    |
        |            \ /                   |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |            / \                   |
        |             |                    |
        |             |                    |
        |  nlm_flow structure allocation   |    
        |_________________________________ |    
        | nlm_device structure (thread n)  |
        |_________________________________ |  << thread_id=n user memory start
        |                                  |     (End user memory for thread_id=0)
        |  nlm_job structure allocation    |
        |             |                    |
        |             |                    |
        |            \ /                   |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |                                  |
        |            / \                   |
        |             |                    |
        |             |                    |
        |  nlm_flow structure allocation   |    
        |_________________________________ |    
        | nlm_device structure (thread 0)  |
        |_________________________________ |  << thread_id=0 user memory start 
*/

static inline void
nlm_debug_dump_regs (nlm_phy_device_t *phy_device, const char *prefix)
{
  uint32_t val;

  val = HW_REGISTER (RDF_TIMESTAMP_LOW);
  DDPRINT ("%stimestamp = 0x%x\n", prefix, val);

  val = *phy_device->output_wrt_ptr;
  DDPRINT ("%sdram output_wrt_ptr = 0x%x\n", prefix, val);

  val = HW_REGISTER (RDF_WRT_PTR);
  DDPRINT ("%sreg output_wrt_ptr = 0x%x\n", prefix, val);
  DDPRINT ("%sreg output_rd_ptr = 0x%x\n", prefix, HW_REGISTER (RDF_RD_PTR));

  val = *phy_device->input_rd_ptr;
  DDPRINT ("%sdram input_rd_ptr = 0x%x\n", prefix, val);

  val = HW_REGISTER (FDF_FIFO_RD_PTR);
  DDPRINT ("%sreg input_rd_ptr = 0x%x\n", prefix, val);
  DDPRINT ("%sreg input_wrt_ptr = 0x%x\n", prefix, HW_REGISTER (FDF_FIFO_WRT_PTR));

  val = HW_REGISTER (CNXT_WRT_PTR);
  DDPRINT ("%sCNXT_WRT_PTR = 0x%x\n", prefix, val);
  val = HW_REGISTER (CNXT_RD_PTR);
  DDPRINT ("%sCNXT_RD_PTR = 0x%x\n", prefix, val);
  val = HW_REGISTER (CNXT_SIZE);
  DDPRINT ("%sCNXT_SIZE = 0x%x\n", prefix, val);
  val = HW_REGISTER (CNXT_CTRL);
  DDPRINT ("%sCNXT_CTRL = 0x%x\n", prefix, val);
  val = HW_REGISTER (CNXT_FIFO_CNT);
  DDPRINT ("%sCNXT_FIFO_CNT = 0x%x\n", prefix, val);
  val = HW_REGISTER (CNXT_WATERMARK);
  DDPRINT ("%sCNXT_WATERMARK = 0x%x\n", prefix, val);
}

static inline void
nlm_debug_dump_pd (nlm_phy_device_t *phy_device, struct hw_pd_descriptor *pd)
{
  api_assert (pd->cookie != pd->pd_nla.pd_nla_index);

  if (1)
    return;
  
  DDPRINT ("PD: cookie byte_count nla_index\n");
  DDPRINT ("    %5d  %6d    %5d    %s %s %s\n",
           pd->cookie, pd->packet_byte_count, pd->pd_nla.pd_nla_index,
           pd->packet_prd ? "" : "NO_PRD",
           pd->flow_term ? "TERM" : "",
           pd->sw_disconnect ? "DIS" : "");

/*  DDPRINT ("pd[0] = 0x%08x\n", ((uint32_t *)pd)[0]);
  DDPRINT ("pd[1] = 0x%08x\n", ((uint32_t *)pd)[1]);
  DDPRINT ("pd[2] = 0x%08x\n", ((uint32_t *)pd)[2]);
  DDPRINT ("pd[3] = 0x%08x\n", ((uint32_t *)pd)[3]);*/

  if (pd->pd_nla.pd_nla_index)
    nlm_debug_dump_pd (phy_device, phy_device->pd_base + pd->pd_nla.pd_nla_index);
}

static void
nlm_debug_dump_pdf (nlm_phy_device_t *phy_device, uint32_t pdf_index)
{
#ifdef NLM_HW_FMS
  if (!phy_device->use_ddr_memory)
    {
      struct hw_pdf_descriptor *pdf;
      pdf = phy_device->u.pdf_base + pdf_index;
      /*DDPRINT ("pdf[0] = 0x%x\n", ((uint32_t *)pdf)[0]);
      DDPRINT ("pdf[1] = 0x%x\n", ((uint32_t *)pdf)[1]);
      DDPRINT ("pdf[2] = 0x%x\n", ((uint32_t *)pdf)[2]);
      DDPRINT ("pdf[3] = 0x%x\n", ((uint32_t *)pdf)[3]);*/
      DDPRINT ("PDF: %s offset = %" NLM_PRId64 "\n", pdf->busy ? "BUSY" : "not busy",
              (((uint64_t)pdf->byte_offset_hi) << 32) | pdf->byte_offset_lo);
    }
  else
    {
      struct hw_pdf_descriptor *pdf = (struct hw_pdf_descriptor *)phy_device->ddr_dma_area;
      nlm_device_copy_from_ddr (phy_device,
                                phy_device->u.pdf_base_in_ddr
                                + pdf_index * sizeof (struct hw_pdf_descriptor),
                                sizeof (struct hw_pdf_descriptor));
      DDPRINT ("pdf[0] = 0x%x\n", ((uint32_t *)pdf)[0]);
      DDPRINT ("pdf[1] = 0x%x\n", ((uint32_t *)pdf)[1]);
      DDPRINT ("pdf[2] = 0x%x\n", ((uint32_t *)pdf)[2]);
      DDPRINT ("pdf[3] = 0x%x\n", ((uint32_t *)pdf)[3]);
      DDPRINT ("PDF: %s offset = %" NLM_PRId64 "\n", pdf->busy ? "BUSY" : "not busy",
               (((uint64_t)pdf->byte_offset_hi) << 32) | pdf->byte_offset_lo);
    }
#endif
}

static inline void
nlm_debug_dump_rd (struct hw_result_descriptor *rd)
{
/*  DDPRINT ("rd[0] = 0x%x\n", ((uint32_t*)rd)[0]);
  DDPRINT ("rd[1] = 0x%x\n", ((uint32_t*)rd)[1]);
  DDPRINT ("rd[2] = 0x%x\n", ((uint32_t*)rd)[2]);
  DDPRINT ("rd[3] = 0x%x\n", ((uint32_t*)rd)[3]);*/

  if (rd->u1.hw.result_type == FMS_FRD_RESULT || rd->u1.hw.result_type == FMS_PRD_RESULT)
    {
      DDPRINT ("%cRD: cookie error timestamp statistic0\n",
               rd->u1.hw.result_type == FMS_FRD_RESULT ? 'F' : 'P');
      DDPRINT ("     %5d  %3d   0x%07" NLM_PRIx64 "  %8x : %s\n",
               rd->cookie,
               rd->u1.hw_frd.error_code,
               (((uint64_t)rd->u4.hw_frd.timestamp_hi) << 24) | rd->u1.hw_frd.timestamp_lo,
               rd->u3.hw_frd_stat0,
               rd->u4.hw.valid ? "valid": "invalid" );
    }
  else if (rd->u1.hw.result_type == FMS_MATCH_RESULT)
    {
      DDPRINT ("RD: cookie offset rule_id match_len\n");
      DDPRINT ("    %5d %6d %5d %5d : %s\n",
               rd->cookie,
               rd->u1.hw.byte_offset_lo,
               rd->u3.hw.rule_id,
               rd->u3.hw.match_length, rd->u4.hw.valid ? "valid": "invalid" );
    }
  else if (rd->u1.hw.result_type == FMS_ERD_RESULT)
    {
      DDPRINT ("erd[0] = 0x%x\n", ((uint32_t*)rd)[0]);
      DDPRINT ("erd[1] = 0x%x\n", ((uint32_t*)rd)[1]);
      DDPRINT ("erd[2] = 0x%x\n", ((uint32_t*)rd)[2]);
      DDPRINT ("erd[3] = 0x%x\n", ((uint32_t*)rd)[3]);
      DDPRINT ("blk = %d vb_offset = %d array_index = %d\n",
               rd->u3.hw_erd.blk_index, rd->u3.hw_erd.vb_oset, rd->u3.hw_erd.array_index);
    }
  else
    api_assert (0);
}

static char*
nlm_debug_flow_type (fms_flow_type flow_type)
{
  switch (flow_type)
    {
    case FMS_FLOW_STATELESS: return "stateless";
    case FMS_FLOW_INIT: return "init";
    case FMS_FLOW_STATEFUL: return "stateful";
    case FMS_FLOW_FINISH: return "fini";
    case FMS_FLOW_PASS_THROUGH: return "pass_thr";
    case FMS_FLOW_REINIT: return "reinit";
    default: return "UNKNOWN";
    }
}

static void
nlm_debug_dump_fd (nlm_phy_device_t *phy_device, struct hw_fd_descriptor *fd)
{
  DDPRINT ("FD:    cookie byte_count nla_index flow_type pdf_index groups\n");
  DDPRINT ("%p    %5d  %6d    %5d      %s    %5d   %d(%d,%d,%d,%d,%d,%d) %s %s %s\n",
           fd, fd->cookie, fd->packet_byte_count, fd->pd_nla.pd_nla_index,
           nlm_debug_flow_type (fd->flow_type),
           fd->pdf_index,
           fd->gid_cnt, fd->gid0, fd->gid1, fd->gid2, fd->gid3, fd->gid4, fd->gid5,
           fd->packet_prd ? "" : "NO_PRD",
           fd->flow_term ? "TERM" : "",
           fd->sw_disconnect ? "DIS" : "");

/*  DDPRINT ("fd[0] = 0x%08x\n", ((uint32_t *)fd)[0]);
  DDPRINT ("fd[1] = 0x%08x\n", ((uint32_t *)fd)[1]);
  DDPRINT ("fd[2] = 0x%08x\n", ((uint32_t *)fd)[2]);
  DDPRINT ("fd[3] = 0x%08x\n", ((uint32_t *)fd)[3]);
  DDPRINT ("fd[4] = 0x%08x\n", ((uint32_t *)fd)[4]);
  DDPRINT ("fd[5] = 0x%08x\n", ((uint32_t *)fd)[5]);
  DDPRINT ("fd[6] = 0x%08x\n", ((uint32_t *)fd)[6]);
  DDPRINT ("fd[7] = 0x%08x\n", ((uint32_t *)fd)[7]);*/

  if (fd->pd_nla.pd_nla_index)
    nlm_debug_dump_pd (phy_device, phy_device->pd_base + fd->pd_nla.pd_nla_index);
}

/* mirror of in/out fifo rd/wrt registers should be initialized with zero */
#define INPUT_RD_PTR_INIT_VALUE 0
#define OUTPUT_WRT_PTR_INIT_VALUE 0
/* msb of 4th word of RD is 'valid' bit, it needs to be cleared by rd_was_processed marker */
#define RD_WAS_PROCESSED 0x7eedbee0
/* rsv01 field of pd is 6 bit wide, make sure PD marker constants fit */
#define PD_CREATED_BUT_NOT_STARTED 0x1e
#define PD_VALID_AND_STARTED 0x1f
/* all real packet bases will have lower 32 bits zeroed,
   any constant with non-zero lower bits will suffice as free marker */
#define FREE_PACKET_BASE ((nlm_phys_addr) 0xf0000000f1eeba3eull)

static nlm_status
reset_device (nlm_phy_device_t *phy_device)
{
  uint32_t regval;

  /* driver should be initialized already and config.register_map_base should point
     to the pci map of HW registers */
  DPRINT ("starting reset...");
  /* do reset */
  SET_HW_REGISTER (SYS_CONTROL, 0); /* disable dma */
  SET_HW_REGISTER (SYS_RESET, 0); /* reset */
  SET_HW_REGISTER (SYS_RESET, 0xffffFFFF); /* set to ff */

  DPRINT ("done\n");

  if (0 != HW_REGISTER (FDF_FIFO_RD_PTR))
    return NLM_DEVICE_FAILURE;

  if (0 != HW_REGISTER (RDF_WRT_PTR))
    return NLM_DEVICE_FAILURE;

  /* Detect ASIC vs. FPGA HW */
  {
    uint32_t sys_date = HW_REGISTER (SYS_DATE_REG);
    if (0x00002058 == sys_date) /* mars1 asic */
      {
        pretty_assert (0 && "don't use fms dataplane with mars1 device");
      }
    else if (0x000051c8 == sys_date) /* mars2 asic */
      {
        pretty_assert (0 && "don't use fms dataplane with mars2 device");
      }
    else if (0x20090301 == sys_date) /* famos ASIC */
      {
        phy_device->hw_type = 4;
        phy_device->num_fms_blocks = 12;
        phy_device->num_rows_per_fms_block = 16 * 1024;

        /* array fuse loading only on asic */
        SET_HW_REGISTER (TEST_REG_ADDR, 0x3);
        SET_HW_REGISTER (TEST_REG_DATA, 0x1);
        SET_HW_REGISTER (TEST_REG_CTRL, 0x1);
        /* wait for write operation to be noticed */
        while (HW_REGISTER (TEST_REG_CTRL) != 0);
        
        /* Wait for array fuse loading to complete */
        do 
          {
            SET_HW_REGISTER (TEST_REG_ADDR, 0x3);
            SET_HW_REGISTER  (TEST_REG_CTRL, 0x2);
            while (HW_REGISTER (TEST_REG_CTRL) != 0);
          }
        while ((HW_REGISTER (TEST_REG_DATA) & 0x4) != 4);
        
        DPRINT ("Array fuse loading complete ...\n");
        
        /* sram fuse loading only on asic */
        SET_HW_REGISTER (TEST_REG_ADDR, 0x3);
        SET_HW_REGISTER (TEST_REG_DATA, 0x2);
        SET_HW_REGISTER (TEST_REG_CTRL, 0x1);
        /* wait for write operation to be noticed */
        while (HW_REGISTER (TEST_REG_CTRL) != 0);
        
        /* Wait for array fuse loading to complete */
        do 
          {
            SET_HW_REGISTER (TEST_REG_ADDR, 0x3);
            SET_HW_REGISTER  (TEST_REG_CTRL, 0x2);
            while (HW_REGISTER (TEST_REG_CTRL) != 0);
          }
        while ((HW_REGISTER (TEST_REG_DATA) & 0x8) != 8);

        DPRINT ("Sram fuse loading complete ...\n");

        /* The following code sets up array registers for manipulating internal clock widths
        *  to make counters work. Woraround for bug #2193
        */
        for (regval = 0x00800006; regval <= 0x0b800006; regval += 0x1000000)
          {
            SET_HW_REGISTER (FAMOS_BLK_ADDR, regval);
            SET_HW_REGISTER (FAMOS_BLK_DATA0, 0x00000040);
            SET_HW_REGISTER (FAMOS_CTRL, 0x00000001);
            while (HW_REGISTER (FAMOS_CTRL) != 0);
          }
      }
    else /* FPGA, lets find the capabilities */
      {
        union
          {
            struct hw_capabilities1 s;
            uint32_t reg;
          } fpga_cap;
        phy_device->hw_type = 6;
        fpga_cap.reg = HW_REGISTER (FPGA_CAPABILITIES1);
        phy_device->num_fms_blocks = fpga_cap.s.num_blocks;
        phy_device->num_rows_per_fms_block = fpga_cap.s.num_rows;
        
        DPRINT ("Device capabilities are:\n");
        DPRINT ("Number of blocks : %d\n", phy_device->num_fms_blocks);
        DPRINT ("Number of rows   : %d\n", phy_device->num_rows_per_fms_block);
#ifndef PERF_TEST
        if (phy_device->num_fms_blocks == 0 || phy_device->num_rows_per_fms_block == 0
            || sys_date == 0)
          /* HW is not alive */
          return NLM_DEVICE_FAILURE;
#endif
      }
  }

  return NLM_OK;
}

static void
configure_device (nlm_phy_device_t *phy_device)
{
  int i;

  phy_device->database_load_policy = NLM_DATABASE_LOAD_POLICY_DEFAULT; /* fair load/unload policy */
  phy_device->packet_enqueue_policy = 0; /* smart enqueue */

  /* config input fifo = FDF */
  SET_HW_REGISTER (FDF_CONFIG, (NLM_BIG_ENDIAN & 1) /* fd descriptor endianness */
                               | (0 << 1) /* zero enable fdf_rdf write to sysmem */
                               | (0 << 2) /* use external/PCI-E memory map */
                               | (phy_device->fms_ring_id << 16)
                               | (phy_device->fms_ring_id << 20) /* ignored by HW */
                               | (phy_device->fms_ring_id << 24)
                               | (phy_device->fms_ring_id << 28)
                               );

  /* config PDF area */
  { /* some compilers are too visual to do preprocessor correctly.
       has to workaround bugs by introducing temp variable */
    uint32_t pdf_config = (1 << 1) /* use different PD and PDF bases */
                          | ((0/*big_endian*/ & 1) << 4) /* payload endianness */
                          | ((NLM_BIG_ENDIAN & 1) << 5) /* pdf descriptor endianness */
                          | (0 << 8) /* process 256 PD after FD */
                          | (2 << 24) /* package burst requests in 512 byte chunks from
                                         DMA into AI, AI will break them down into 128 byte
                                         PCI-E transactions, since some motherboard chipsets
                                         don't support larger transactions */
                          ;
    SET_HW_REGISTER (PDF_CONFIG, pdf_config);
  }

  /* the maximum number of times the busy bit can be polled before the flow is failed.
     If this value is 0, this timeout function is disabled */
  SET_HW_REGISTER (PDF_CONFIG4, PDF_BUSY_BIT_POLL_TIMEOUT);

  if (phy_device->use_ddr_memory)
    SET_HW_REGISTER (PDF_MEMMAP_CONFIG, 1 << 16 /* use internal memory map == keep PDFs in DDR */);
  else
    SET_HW_REGISTER (PDF_MEMMAP_CONFIG, 0 /* use external memory map */);

  /* config output fifo = RDF */
  SET_HW_REGISTER (RDF_CONFIG, (NLM_BIG_ENDIAN & 1) /* endianness of RD */
                               | (1 << 1) /* enable timestamp in RD */
                               | (0 << 3) /* use external/PCI-E memory map */
                               | (3 << 8) /* burst 8 RD at a time */
                               );

  SET_HW_REGISTER (RDF_CLAMP0, (MAX_MATCHES_PER_JOB & ((1 << 14) - 1)) /* P0 */
                               | (1 << 14) /* don't fail flow on result overflow */
                               | ((MAX_MATCHES_PER_JOB & ((1 << 14) - 1)) << 16) /* P1 */
                               | (1 << 30) /* don't fail flow on result overflow */
                               );
  SET_HW_REGISTER (RDF_CLAMP1, (MAX_MATCHES_PER_JOB & ((1 << 14) - 1)) /* P2 */
                               | (1 << 14) /* don't fail flow on result overflow */
                               | ((MAX_MATCHES_PER_JOB & ((1 << 14) - 1)) << 16) /* P3 */
                               | (1 << 30) /* don't fail flow on result overflow */
                               );

  /* set the amount of wait time that must pass before the FDF DMA engine updates
     the external FDF_RD_PTR and RDF_WRT_PTR copies in system memory, but only if at
     least one of the external values are stale.
     Making this number large enough will decrease collisions in
     nlm_get_all_search_results(), but will increase request-to-result latency
     at the same time */
  SET_HW_REGISTER (FDF_POLL_INTERVAL, FDF_POLL_INTERVAL_DEFAULT_VALUE);

  /* set PDF polling interval
     [11:5] - 7 bits of PDF Polling interval.
     This will be used when polling for PDF busy bit. It is incremented in units of 32 sclk
     cycles. 0 is equal to 32 clocks, 0x20 - 64 clocks */
  SET_HW_REGISTER (PDF_POLL_INTERVAL, PDF_POLL_INTERVAL_DEFAULT_VALUE);

  /* set output result force timer:
     This timer controls when the internal RDF FIFO gets flushed out to system
     memory regardless of max burst size. */
  SET_HW_REGISTER (RDF_FORCE_TIMER, RDF_FORCE_TIMER_DEFAULT_VALUE);

  /* enable all engines */
  SET_HW_REGISTER (PDF_CONFIG2, 0);

  SET_HW_REGISTER (FDF_FIFO_WRT_PTR, 0);
  SET_HW_REGISTER (RDF_RD_PTR, 0);

  if (phy_device->config.virt_to_phys == NULL)
    { /* there is no callback to convert virt to phys for packet addresses, hence
         init all packet bases around 'packet_base_phys' address */
      nlm_phys_addr phys_addr_base
        = phy_device->config.packet_base_phys & (~0ull << (PQ_ADDR_LOW_BITS + 4));

      api_assert ((1 << 4) == HW_PACKET_BASES);

      for (i = 0; i < HW_PACKET_BASES; i++)
        {
          nlm_phys_addr phys_addr = phys_addr_base + i * (1ull << PQ_ADDR_LOW_BITS);
          api_assert ((phy_device->config.packet_base_phys & (~0ull << (PQ_ADDR_LOW_BITS + 4)))
                      == (phys_addr & (~0ull << (PQ_ADDR_LOW_BITS + 4))));
          phy_device->packet_base[i] = phys_addr;
          SET_HW_REGISTER (PQ_BASE_LOW_0 + i * 8, (uint32_t) phys_addr);
          SET_HW_REGISTER (PQ_BASE_HI_0 + i * 8, (uint32_t) (phys_addr >> 32));
        }
    }
  else
    { /* init packet bases for dynamic allocation */
      for (i = 0; i < HW_PACKET_BASES; i++)
        phy_device->packet_base[i] = FREE_PACKET_BASE;
    }
}

static nlm_status
init_hw_memory (nlm_phy_device_t *phy_device)
{
#ifdef PERF_TEST
  return NLM_OK;
#else

#ifdef NLM_HW_FMS
  {
    /* init group_id bitmap table */
    int db_id, group_id;

    for (db_id = 0; db_id <= NLM_MAX_DATABASE_ID; db_id++)
      {
        for (group_id = 0; group_id <= NLM_MAX_GROUP_ID; group_id++)
          {
            SET_HW_REGISTER (GID_BMAP_DATA0, 0);
            SET_HW_REGISTER (GID_BMAP_DATA1, 0);
            SET_HW_REGISTER (GID_BMAP_CTRL, ((db_id << 26) | (group_id << 16)) | 1);
            while (HW_REGISTER (GID_BMAP_CTRL) & 1);
          }
      }

  }
#endif

  return NLM_OK;
#endif
}

/* init context save/restore area to be used by device to store engine state
   during flow switching */
static nlm_status
init_context_save_restore_area_sysmem (nlm_phy_device_t *phy_device, char **p_free_ptr)
{
  nlm_phys_addr phys_addr;
  char *cntx_buf_base;
  char *cntx_ring_base;
  uint32_t *cntx_ring_ptr;
  uint32_t buf_cnt;
  uint32_t size;
  uint32_t cntx_buf_offset = 0;
  uint32_t last_cntx_buf_offset;

  char *free_ptr = *p_free_ptr;

  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 255) & ~255); /* round up to 256 byte alignment */
  cntx_buf_base = phy_device->context_save_restore_area = free_ptr;
  
  /* FIXME: this changes value in the config structure */
  phy_device->config.size_of_context_save_restore_area &= ~255; /* round down to 256 byte alignment */

  size = phy_device->config.size_of_context_save_restore_area;
  free_ptr += size;
  if (free_ptr > (char *) phy_device->config.sysmem_base_virt + phy_device->config.sysmem_size
                  - sizeof (struct hw_pdf_descriptor) * NLM_MIN_FLOW_CNT /* at least some pdfs */)
    /* system memory is too small */
    return NLM_OUT_OF_DEVICE_MEMORY;

  buf_cnt = size / 260;  /* Every context buffer is 256 bytes long and needs
                            4 bytes of offset to store in the context ring */

  if (buf_cnt == 0 || buf_cnt < NLM_MIN_FLOW_CNT)
    return NLM_INVALID_ARGUMENT;

  phys_addr = CONVERT_TO_PHYSICAL (cntx_buf_base);

  memset (cntx_buf_base, 0, size);

  last_cntx_buf_offset = (buf_cnt - 1) << 8;
  cntx_ring_base = cntx_buf_base + (buf_cnt << 8);
  cntx_ring_ptr = (uint32_t *) cntx_ring_base;

  /* There will be < 260 bytes of sysmem wasted near the end of
     context_save_restore_area */
  while (cntx_buf_offset <= last_cntx_buf_offset)
    {
      *cntx_ring_ptr = cntx_buf_offset;
      cntx_ring_ptr++;
      cntx_buf_offset += 256;
    }

  SET_HW_REGISTER (CNXT_BUFF_BASE_LOW, (uint32_t) phys_addr);
  SET_HW_REGISTER (CNXT_BUFF_BASE_HI, (uint32_t) (phys_addr >> 32));

  phys_addr += buf_cnt << 8;

  SET_HW_REGISTER (CNXT_BASE_BASE_LOW, (uint32_t) phys_addr);
  SET_HW_REGISTER (CNXT_BASE_BASE_HI, (uint32_t) (phys_addr >> 32));

  /* after init and after fini
     CNXT_WRT_PTR - CNXT_RD_PTR + CNXT_FIFO_CNT.rd_fifo_level should equal 'size'
     CNXT_FIFO_CNT.wrt_fifo_level should equal zero */
  SET_HW_REGISTER (CNXT_SIZE, buf_cnt);

  SET_HW_REGISTER (CNXT_CTRL, 0xffff1001 | ((NLM_BIG_ENDIAN & 1) << 3)); /* enable context ring */

  *p_free_ptr = free_ptr;
  return NLM_OK;
}

static nlm_status
init_context_save_restore_area_ddr (nlm_phy_device_t *phy_device)
{
  nlm_phys_addr phys_addr, top_of_ddr;
  uint32_t buf_cnt, buf_index;
  int i;
  uint32_t size;
  
  /* DDR memory allocation:
     0 ->
     cnxt_buf0 cnxt_buf1 ... cnxt_bufN cnxt_ring0 cnxt_ring1 ... cnxt_ringN (padding)  PDF0 PDF1 ... PDFN
     ^                                 ^
     BUFF_BASE                         BASE_BASE
     N == buf_cnt
     */
  
  /* FIXME: changes value in config structure */
  phy_device->config.size_of_context_save_restore_area &= ~255; /* round down to 256 byte alignment */
  size = phy_device->config.size_of_context_save_restore_area;

  /* calculate the number of buffers in context ring,
     each context buffer is 256 bytes */
  buf_cnt = size >> 8;

  /* round down the number of buffers to the nearest multiple of 4,
     since we want the DMA transfer to be the multiple of 16 bytes */
  buf_cnt &= ~3;

  /* there is a DDR, check the size */
  if (size == 0
      || size >= phy_device->config.size_of_ddr_memory
      || buf_cnt == 0
      || size >= phy_device->config.size_of_ddr_memory
            - buf_cnt * sizeof (uint32_t)
            - sizeof (struct hw_pdf_descriptor) * NLM_MIN_FLOW_CNT)
    return NLM_OUT_OF_DDR_MEMORY;

  phys_addr = phy_device->config.base_addr_of_ddr_memory;
  phys_addr |= 1ull << 61; /* use DDR in internal memory map */
  SET_HW_REGISTER (CNXT_BUFF_BASE_LOW, (uint32_t) phys_addr);
  SET_HW_REGISTER (CNXT_BUFF_BASE_HI, (uint32_t) (phys_addr >> 32));

  phys_addr += size;
  SET_HW_REGISTER (CNXT_BASE_BASE_LOW, (uint32_t) phys_addr);
  SET_HW_REGISTER (CNXT_BASE_BASE_HI, (uint32_t) (phys_addr >> 32));

  buf_index = 0;

  do
    {
      for (i = 0;
           i < NLM_DDR_DMA_AREA_SIZE / sizeof (uint32_t) && buf_index < buf_cnt;
           i++, buf_index++)
        ((uint32_t *)phy_device->ddr_dma_area)[i] = buf_index << 8;

      /* DMA it into DDR */
#ifdef NLM_HW_FMS
      nlm_device_copy_into_ddr (phy_device, phys_addr, i * sizeof (uint32_t));
#endif
      phys_addr += i * sizeof (uint32_t);
    }
  while (buf_index < buf_cnt);

  SET_HW_REGISTER (CNXT_SIZE, buf_cnt);

  /* enable context ring and internal memory map */
  SET_HW_REGISTER (CNXT_CTRL, 0xffff1001 | ((NLM_BIG_ENDIAN & 1) << 3) | (1 << 1) | (1 << 2));

  phys_addr = (phys_addr + 127) & ~127; /* round up to 128 alignment */
  HW_SET_PDF_BASE (phys_addr);

  phy_device->u.pdf_base_in_ddr = phys_addr;
  top_of_ddr = (phy_device->config.base_addr_of_ddr_memory + phy_device->config.size_of_ddr_memory) | 1ull << 61;
  top_of_ddr &= ~127; /* round down to 128 alignment */
  phy_device->ddr_chunksize = (top_of_ddr - phys_addr) / phy_device->config.max_threads;
  phy_device->ddr_chunksize &= ~127; /* round down to 128 alignment */

  return NLM_OK;
}

/* check the HW can perform basic operations */
static nlm_status
check_device_sanity (nlm_phy_device_t *phy_device)
{
  uint32_t val = 0;
  int cnt = 1000;
#ifdef PERF_TEST
  return NLM_OK;
#endif
  do
   /* as soon as dma is enabled, HW will prefetch 16 context ring entries into internal fifo.
      This is the check to verify that this is happening */
   val = HW_REGISTER (CNXT_FIFO_CNT);
  while (-- cnt && (val & 0x1f) != 16);

  if (cnt == 0)
    return NLM_DEVICE_DMA_FAILURE;
  else
    return NLM_OK;
}

static void
init_pointer_mirror_area (nlm_phy_device_t *phy_device, char **p_free_ptr)
{
  nlm_phys_addr phys_addr;
  char *free_ptr = *p_free_ptr;

  /* init 32 bit FDF read pointer (sysmem updated by hardware) */
  phy_device->input_rd_ptr = (uint32_t *) free_ptr;
  *phy_device->input_rd_ptr = INPUT_RD_PTR_INIT_VALUE;
  free_ptr += 4;

  /* init 32 bit RDF write pointer (sysmem updated by hardware) */
  phy_device->output_wrt_ptr = (uint32_t *) free_ptr;
  *phy_device->output_wrt_ptr = OUTPUT_WRT_PTR_INIT_VALUE;
  free_ptr += 4;

  phys_addr = CONVERT_TO_PHYSICAL (phy_device->input_rd_ptr);
  SET_HW_REGISTER (FDF_RDF_BASE_LOW, (uint32_t) phys_addr);
  SET_HW_REGISTER (FDF_RDF_BASE_HI, (uint32_t) (phys_addr >> 32));

  /* HW timestamp will be stored in this area */
  free_ptr += 8;

  *p_free_ptr = free_ptr;
}

static void
init_database_extension_area (nlm_phy_device_t *phy_device, char **p_free_ptr)
{
  nlm_phys_addr phys_addr;
  char *free_ptr = *p_free_ptr;
  int i;
  uint32_t size;
  
#ifdef PERF_TEST
  return;
#endif
  /* ignore the size_of_database_extension_area that is specified in the device config */
  api_assert (phy_device->num_fms_blocks >= 1 && phy_device->num_fms_blocks <= 12);
  size = phy_device->num_fms_blocks * sizeof (struct fms_block_hw);
  
  /* FIXME: This overwrites the config structure */
  phy_device->config.size_of_database_extension_area = size;


  /* round up to 128 byte alignment for better DMA performance */
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 127) & ~127);
  phy_device->database_extension_area = (uint8_t *)free_ptr;
  free_ptr += size;

  phys_addr = CONVERT_TO_PHYSICAL (phy_device->database_extension_area);
  SET_HW_REGISTER (BLOCK_BASE_LOW, (uint32_t) (phys_addr & 0xffffFFFF));
  SET_HW_REGISTER (BLOCK_BASE_HI, (uint32_t) (phys_addr >> 32));
  /* SET_HW_REGISTER (BLOCK_BASE_CONFIG, (NLM_BIG_ENDIAN << 1)); */
  /* The current MIPS implementation requires no endiannes shuffling */

  for (i = 0; i < phy_device->num_fms_blocks; i++)
    {
      SET_HW_REGISTER (BLOCK0_CONFIG + i * FMS_BLK_REG_STEP, 0);
      SET_HW_REGISTER (BLOCK0_OFFSET + i * FMS_BLK_REG_STEP, 0);
    }

  *p_free_ptr = free_ptr;
}

static void
init_input_fifo (nlm_phy_device_t *phy_device, char **p_free_ptr)
{
  nlm_phys_addr phys_addr;
  char *free_ptr = *p_free_ptr;

  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 31) & ~31); /* round up to 32 byte alignment */
  phy_device->input_fifo_base = (struct hw_fd_descriptor *) free_ptr;
  free_ptr += phy_device->config.input_fifo_size * sizeof (struct hw_fd_descriptor);
  phys_addr = CONVERT_TO_PHYSICAL (phy_device->input_fifo_base);
  DPRINT ("input_base = 0x%016llx\n", phys_addr);
  HW_SET_INPUT_FIFO_BASE (phys_addr);
  HW_SET_INPUT_FIFO_SIZE (phy_device->config.input_fifo_size);

  *p_free_ptr = free_ptr;
}

static void
init_output_fifo (nlm_phy_device_t *phy_device, char **p_free_ptr)
{
  nlm_phys_addr phys_addr;
  char *free_ptr = *p_free_ptr;

  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 15) & ~15); /* round up to 16 byte alignment */
  phy_device->output_fifo_base = (struct hw_result_descriptor *) free_ptr;
  free_ptr += phy_device->config.output_fifo_size * sizeof (struct hw_result_descriptor);
  phys_addr = CONVERT_TO_PHYSICAL (phy_device->output_fifo_base);
  DPRINT ("output_base = 0x%016llx\n", phys_addr);
  HW_SET_OUTPUT_FIFO_BASE (phys_addr);
  HW_SET_OUTPUT_FIFO_SIZE (phy_device->config.output_fifo_size);

  *p_free_ptr = free_ptr;
}

static void
init_ddr_memory (nlm_phy_device_t *phy_device, char **p_free_ptr)
{
  char *free_ptr = *p_free_ptr;

#ifdef NLM_HW_FMS
  if (phy_device->fms_ring_id == NLM_MASTER_RING) /* master ring should train DDR */
    {
      uint64_t size_of_ddr = 0;
	  printf("[%s] ingress\n", __FUNCTION__);

      SET_HW_REGISTER (DDR_DMA_ENABLE, 1); /* enable dma0 */

      nlm_device_init_ddr_memory (phy_device, &size_of_ddr);
      DPRINT ("DDR size = %"PRId64"\n", size_of_ddr);
      printf("DDR size = %d\n", size_of_ddr);
    }
#endif

#ifdef NLM_HW_FMS
  if (phy_device->config.size_of_ddr_memory == 0)
#else
  if (1) /* do not use DDR in any of the software models */
#endif
    {
      /* user requested not to use DDR memory or there is no DDR memory */
      phy_device->use_ddr_memory = 0;
      return;
    }

  phy_device->use_ddr_memory = 1;

  /* round up to 16 byte alignment to DMA transfer DDR from/to host memory */
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 15) & ~15);
  phy_device->ddr_dma_area = free_ptr;
  free_ptr += NLM_DDR_DMA_AREA_SIZE;

  *p_free_ptr = free_ptr;
}

static nlm_status
init_manager_thread_fifo (nlm_phy_device_t *phy_device,
                          char **free_ptr_p, char *top_of_heap)
{
  char *free_ptr;
  int32_t i;
  
  if (!phy_device->config.using_manager_thread)
    return NLM_OK;
  
  free_ptr = *free_ptr_p;
  phy_device->sw_job_fifo = (nlm_job_fifo_t *) free_ptr;
  free_ptr = free_ptr + phy_device->config.max_threads * sizeof (nlm_job_fifo_t);
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 7) & ~7);
  for (i = 0; i < phy_device->config.max_threads; i++)
    {
      job_fifo_init (&phy_device->sw_job_fifo[i], phy_device->config.input_fifo_size, (struct hw_fd_descriptor *)free_ptr);
      free_ptr += sizeof (struct hw_fd_descriptor) * phy_device->config.input_fifo_size;
      if (free_ptr >= top_of_heap)
        /* system memory is too small */
        return NLM_OUT_OF_DEVICE_MEMORY;
    }
      
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 7) & ~7);
  if (free_ptr >= top_of_heap)
    /* system memory is too small */
    return NLM_OUT_OF_DEVICE_MEMORY;
  
  phy_device->sw_result_fifo = (nlm_result_fifo_t *) free_ptr;
  free_ptr = free_ptr + phy_device->config.max_threads * sizeof (nlm_result_fifo_t);
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 7) & ~7);
  for (i = 0; i < phy_device->config.max_threads; i++)
    {
      result_fifo_init (&phy_device->sw_result_fifo[i], phy_device->config.output_fifo_size, (struct hw_result_descriptor *)free_ptr);
      free_ptr += sizeof (struct hw_result_descriptor) * phy_device->config.output_fifo_size;
      if (free_ptr >= top_of_heap)
        /* system memory is too small */
        return NLM_OUT_OF_DEVICE_MEMORY;
    }

  *free_ptr_p = free_ptr;
  return NLM_OK;
}

static nlm_status
check_mem_overlap (nlm_phy_device_t *phy_device)
{
  int i, j;
  nlm_phys_addr sysmem_lo[NLM_LAST_UNUSED_RING_ID];
  nlm_phys_addr sysmem_hi[NLM_LAST_UNUSED_RING_ID];
  nlm_phys_addr ddr_lo[NLM_LAST_UNUSED_RING_ID];
  nlm_phys_addr ddr_hi[NLM_LAST_UNUSED_RING_ID];

  for (i = 0; i < NLM_LAST_UNUSED_RING_ID; i++)
    {
      if (HW_REGISTER (PDF0_CONFIG + i * PDF_RING_OFFSET) == 0)
        {
          /* this ring is not available in fpga */
          sysmem_lo[i] = 0;
          ddr_lo[i] = 0;
        }
      else if (HW_REGISTER (PDF0_CONFIG + i * PDF_RING_OFFSET) != PDF0_CONFIG_DEFAULT)
        {
          /* this ring was set up */

          sysmem_lo[i] = (((uint64_t)HW_REGISTER (FDF0_RDF_BASE_HI + i * FDF_RING_OFFSET)) << 32)
                         | HW_REGISTER (FDF0_RDF_BASE_LOW + i * FDF_RING_OFFSET);

          if (HW_REGISTER (PDF0_MEMMAP_CONFIG + i * PDF_RING_OFFSET) != 0)
            {
              /* DDR is used to keep context save/restore area and PDFs */
              ddr_lo[i] = (((uint64_t)HW_REGISTER (CNXT0_BUFF_BASE_HI + i * CNXT_RING_OFFSET)) << 32)
                             | HW_REGISTER (CNXT0_BUFF_BASE_LO + i * CNXT_RING_OFFSET);
            }
          else
            {
              /* sysmem is used to keep PDFs */
            }
        }
      else
        {
          sysmem_lo[i] = 0;
          ddr_lo[i] = 0;
        }
    }

  /* Calculate upper boundaries of sysmem and DDR mem for all rings. */
  for (i = 0; i < NLM_LAST_UNUSED_RING_ID; i++)
    {
      if (sysmem_lo[i])
        sysmem_hi[i] = sysmem_lo[i] + HW_REGISTER (SCRATCH0 + (2 * i) * 4) - 1;
      if (ddr_lo[i])
        ddr_hi[i] = ddr_lo[i] + HW_REGISTER (SCRATCH0 + (2 * i + 1) * 4) - 1;
    }

  for (i = 0; i < NLM_LAST_UNUSED_RING_ID; i++)
    {
      if (sysmem_lo[i])
        {
          for (j = 0; j < NLM_LAST_UNUSED_RING_ID; j++)
            if (i != j && sysmem_lo[j]
                && !(sysmem_hi[i] < sysmem_lo[j] || sysmem_hi[j] < sysmem_lo[i]))
              return NLM_INVALID_DEVICE_CONFIG;
        }
      if (ddr_lo[i])
        {
          for (j = 0; j < NLM_LAST_UNUSED_RING_ID; j++)
            if (i != j && ddr_lo[j]
                && !(ddr_hi[i] < ddr_lo[j] || ddr_hi[j] < ddr_lo[i]))
              return NLM_INVALID_DEVICE_CONFIG;
        }
    }

  return NLM_OK;
}

static nlm_status
check_xaui_buffers (nlm_phy_device_t *phy_device)
{
  int32_t i, j, ind = 0;
  uint32_t buffer_addr[511];
  uint32_t ctrl_reg_val, addr, test_register;
  
  /* There should be 511 Xaui buffers */
  if (phy_device->fms_ring_id == NLM_XAUI_RING_0)
    {
      if (HW_REGISTER (X0_FREE_LEVEL) != 511)
        {
          DPRINT ("Warning X0_FREE_LEVEL is not expected value of 511 instead %d\n", 
                  HW_REGISTER (X0_FREE_LEVEL));
          return NLM_DEVICE_XAUI_BUFFER_INCONSISTENT;
        }
      test_register = X0_TEST;
    }
  else if (phy_device->fms_ring_id == NLM_XAUI_RING_1)
    {
      if (HW_REGISTER (X1_FREE_LEVEL) != 511)
        {
          DPRINT ("Warning X1_FREE_LEVEL is not expected value of 511 instead %d\n", 
                  HW_REGISTER (X0_FREE_LEVEL));
          return NLM_DEVICE_XAUI_BUFFER_INCONSISTENT;
        }
      test_register = X1_TEST;
    }
  else
    return NLM_OK; /* not XAUI */

  /* We pop each of the XAUI buffer address and the goal here
     is to find no zero address & no repeated address */
  for (i = 0; i < 511; i++)
    {
      /* Request to pop one buffer */
      SET_HW_REGISTER (test_register, 0x1);

      /* wait until pop request is satisfied */
      ctrl_reg_val = 0x1;
      while ((ctrl_reg_val & 0x1) != 0)
        ctrl_reg_val = HW_REGISTER (test_register);

      /* address is 10 bit value in control register */
      addr = (ctrl_reg_val >> 16) & 0x3FF;
      if (addr == 0)
        return NLM_DEVICE_XAUI_BUFFER_INCONSISTENT;
      
      /* check if we have seen this address before
         A slow scan to check this */
      for (j = 0; j < ind; j++)
        {
          if (buffer_addr[j] == addr)
            return NLM_DEVICE_XAUI_BUFFER_INCONSISTENT;
        }
      buffer_addr[ind] = addr;
      ind++;
    }
  
  return NLM_OK;
}

static nlm_status
check_context_ring (nlm_phy_device_t *phy_device)
{
  uint32_t cnxt_buf_cnt = HW_REGISTER (CNXT_SIZE);
  uint64_t offsets_sum = 0, ref_offsets_sum = 0;
  uint32_t offsets_chksum = 0, ref_offsets_chksum = 0;
  uint32_t shifted_offset;
  uint32_t cnxt_pop_ctrl_reg_val, cnxt_pop_data_reg_val;
  uint32_t i;
  nlm_status status = NLM_OK;

  api_assert (cnxt_buf_cnt != 0);

  /* Try to pop all context buffers from hardware. One context buffer is always wasted
     (one element of the ring is always unused) Hence we will get only CNXT_SIZE - 1
     buffers back */
  for (i = 0; i < cnxt_buf_cnt - 1; i++)
    {
      cnxt_pop_ctrl_reg_val = 0x1;
      SET_HW_REGISTER (CNXT_POP_CTRL, 0x1);
      while ((cnxt_pop_ctrl_reg_val & 0x1) != 0)
        cnxt_pop_ctrl_reg_val = HW_REGISTER (CNXT_POP_CTRL);
      if ((cnxt_pop_ctrl_reg_val & 0x4) == 0x4)
        {
          cnxt_pop_data_reg_val = HW_REGISTER (CNXT_POP_DATA);
          /* Context buffer address offset must be 256-byte aligned */
          if ((cnxt_pop_data_reg_val & 0xff) != 0)
            status = NLM_DEVICE_CONTEXT_FAIL;
          else
            {
              shifted_offset = cnxt_pop_data_reg_val >> 8;
              ref_offsets_chksum ^= i;
              offsets_sum += shifted_offset;
              offsets_chksum ^= shifted_offset;
            }
        }
      else
        /* We must get back cnxt_size - 1 number of context buffers, if not this is
           an error condition */
        status = NLM_DEVICE_CONTEXT_FAIL;
    }

  /* Try popping a context buffer one last time, this must fail. */
  cnxt_pop_ctrl_reg_val = 0x1;
  SET_HW_REGISTER (CNXT_POP_CTRL, 0x1);
  while ((cnxt_pop_ctrl_reg_val & 0x1) != 0)
    cnxt_pop_ctrl_reg_val = HW_REGISTER (CNXT_POP_CTRL);
  if ((cnxt_pop_ctrl_reg_val & 0x4) == 0x4)
    status = NLM_DEVICE_CONTEXT_FAIL;

  /* Compute ref_offsets_sum. During init we stored context buffer offsets
     0, 256, 512, 768, 1024, ... in the context ring.
     Last context buffer is never returned by hardware and is wasted. Above we
     compute offsets_sum as sum of offsets divided by 256 (right shifted by 8 bits)
     So to calculate ref_offsets_sum we find sum of shifted offsets of first
     (cnxt_buf_cnt - 1) number of buffers.
     i.e. 0 + 1 + 2 + 3 + ...((cnxt_buf_cnt - 1) terms)
     = (cnxt_buf_cnt - 1) * (cnxt_buf_cnt - 2) / 2 */

  ref_offsets_sum = ((uint64_t) cnxt_buf_cnt - 1) * (cnxt_buf_cnt - 2) / 2;

  /* Compare the sum of offsets and checksum of offsets */
  if (offsets_sum != ref_offsets_sum || offsets_chksum != ref_offsets_chksum)
    status = NLM_DEVICE_CONTEXT_FAIL;

  return status;
}

static nlm_status
device_attach (struct nlm_device_config *config, nlm_device_t **p_device,
               nlm_phy_device_t *phy_device, char *memory_pool,
               uint32_t memory_pool_size)
{
  char *free_ptr, *top_of_heap, *pd_start, *pd_end;
  nlm_device_t *device;
  uint32_t attach_tid;

  printf("[%s] in, ring_id:%d \n", __FUNCTION__, config->ring_id);
  
  /* Ensure that more threads than the requested number have not attached */
  attach_tid = NLM_ATOMIC_ADD32_RETURN (&phy_device->threads_attached, 1);
  
  if (attach_tid > config->max_threads)
    return NLM_EXCESS_DEVICE_ATTACH;

  /* allocate structures in memory pool */
  free_ptr = memory_pool;
  memset (free_ptr, 0, memory_pool_size);
  top_of_heap = free_ptr + memory_pool_size;

  /* allocate nlm_device at base of user supplied memory pool */
  device = (nlm_device_t *) free_ptr;
  free_ptr += sizeof (nlm_device_t);
  device->phy_device = phy_device;

  /* Set relevant parametres from the config into
     our device structure */
  device->thread_id = config->thread_id;
  device->fms_ring_id = config->ring_id;
  device->memory_pool = memory_pool;
  device->memory_pool_size = memory_pool_size;
  device->cookie = config->cookie;

  /* Remaining DP threads, need to initialize specific fields */
  /* Figure out our FIFO locations, and initiallize them */
  if (config->using_manager_thread)
    {
      device->sw_job_fifo = &phy_device->sw_job_fifo[config->thread_id];
      device->sw_result_fifo = &phy_device->sw_result_fifo[config->thread_id];
    }

  /* init flow free list at the lowest free space in user supplied memory pool */
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 7) & ~7); /* round up to 8 byte alignment */
  device->first_free_flow = (struct nlm_flow *) free_ptr;
  device->stateful_flow_list_head = NULL;
  device->stateless_flow_list_head = NULL;

  /* init job free list at the highest end of user supplied memory pool.
     note that one nlm_job space is here reserved for nlm_job allocation
     (see flow and job allocation routines).  */
  free_ptr = top_of_heap;
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr) & ~7); /* round down to 8 byte alignment */
  /* we are going to calculate sequence id for HW by job_base - job
     and we want this number to be always greater than zero,
     so we can detect memory corruption if sequence id is zero */
  device->job_base = (struct nlm_job *) free_ptr;
  device->first_free_job = device->job_base - 1;
  device->job_list_head = NULL;

  /* init PD free list pointers in system memory */
  device->pd_list_head = NULL;
  pd_start = (char *) phy_device->pd_base + phy_device->chunksize * config->thread_id;
  pd_end = pd_start + phy_device->chunksize;
  pd_start = (char *) (((nlm_uintptr_t) pd_start + 127) & ~127); /* round up to 128 alignment */
  if (pd_start == (char *) phy_device->pd_base)
    pd_start += sizeof (struct hw_pd_descriptor);

  device->pd_list_head = NULL;
  device->first_free_pd = (struct hw_pd_descriptor *) pd_start;
  
  /* PDF's grow from top down in system memory. If PDF's are in DDR
     they will get allocated differently, and these pointers are
     used as reference by the PD allocation routines */
  device->first_free_pdf = (struct hw_pdf_descriptor *) (((nlm_uintptr_t) pd_end) & ~ 127);
  device->first_free_pdf--;

  /* If PDF's are in ddr convert complete system memory is
     reserved for PD's */
  if (phy_device->use_ddr_memory)
    {
      /* create the PDF as indexes in DDR memory */
      nlm_phys_addr pdf_start, pdf_end;
      
      pdf_start = phy_device->u.pdf_base_in_ddr + phy_device->ddr_chunksize * config->thread_id;
      pdf_end = pdf_start + phy_device->ddr_chunksize;

      device->current_pdf_index = (pdf_start - phy_device->u.pdf_base_in_ddr) / sizeof (struct hw_pdf_descriptor);
      /* PDF index should not be zero */
      if (pdf_start == phy_device->u.pdf_base_in_ddr)
        device->current_pdf_index++;
      device->max_pdf_index = (pdf_end - phy_device->u.pdf_base_in_ddr) / sizeof (struct hw_pdf_descriptor);
      device->max_pdf_index--;
    }

  *p_device = device;

  printf("[%s] out, ring_id:%d \n", __FUNCTION__, config->ring_id);
  return NLM_OK;
}

static nlm_status
physical_device_init (struct nlm_device_config *config)
{
  char *free_ptr, *top_of_heap;
  nlm_phy_device_t *phy_device;
  nlm_phys_addr phys_addr;
  nlm_status status;

  printf("[%s] ingress \n", __FUNCTION__);
  
  /* allocate structures in system memory */
  free_ptr = (char *) config->sysmem_base_virt;

  /* clear system memory */
  memset (free_ptr, 0, config->sysmem_size);

  phy_device = (nlm_phy_device_t *) config->sysmem_base_virt;
  phy_device->device_magic = NLM_DEVICE_MAGIC;
  phy_device->fms_ring_id = config->ring_id;
  memcpy (&phy_device->config, config, sizeof (struct nlm_device_config));

  top_of_heap = (char *) (((nlm_uintptr_t) free_ptr + config->sysmem_size) & ~ 63);
  free_ptr += sizeof (nlm_phy_device_t);
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 7) & ~7); /* round up to 8 byte alignment */
  
  if (free_ptr >= top_of_heap)
    return NLM_OUT_OF_DEVICE_MEMORY;

  printf("[%s] ring_id : 0x%x\n", __FUNCTION__, config->ring_id);

  if (config->ring_id == NLM_MASTER_RING)
    {
      /* master ring should reset the device */
      status = reset_device (phy_device);
      if (status != NLM_OK)
        return status;
    }
  else if (config->ring_id == NLM_XAUI_RING_0)
    {
      if (HW_REGISTER (PDF0_CONFIG) == PDF0_CONFIG_DEFAULT || HW_REGISTER (PDF0_CONFIG) == 0)
        /* master ring was not setup */
        return NLM_INCORRECT_ACCESS;

	  printf("[%s] xaui init\n", __FUNCTION__);
	  printf("HW_REGISTER_MAP_BASE : 0x%x\n", HW_REGISTER_MAP_BASE);
	  printf("reg_map_base_vir : 0x%x\n", phy_device->config.register_map_base_virt);
	  printf("reg_map_base_phy : 0x%x\n", phy_device->config.register_map_base_phys);

      /* Bring the xaui link up */
      SET_HW_REGISTER (X0_CT_TEST, 0x00040000);
	  printf("X0_CT_TEST : 0x%x\n", HW_REGISTER (X0_CT_TEST));
      NLM_WASTE_TIME

#if 0
      SET_HW_REGISTER (X0_CT_TEST, 0x00044000);
	  printf("X0_CT_TEST : 0x%x\n", HW_REGISTER (X0_CT_TEST));
      NLM_WASTE_TIME
#endif

	// DeQA/B/C/D
      SET_HW_REGISTER (X0_CT_CFG0, 0x8888);
	  printf("X0_CT_CFG0 : 0x%x\n", HW_REGISTER (X0_CT_CFG0));
      NLM_WASTE_TIME

	// PCS jumbo_en
      SET_HW_REGISTER (X0_CT_CFG1, 0x00000100);
	  printf("X0_CT_CFG1 : 0x%x\n", HW_REGISTER (X0_CT_CFG1));
      NLM_WASTE_TIME

	// XAUI i/f is powered down by default 
      SET_HW_REGISTER (X0_CT_CTRL, 0x00100000);
	  printf("X0_CT_CTRL : 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_CT_CTRL, 0x00100003);
	  printf("X0_CT_CTRL : 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_CT_CTRL, 0x00100013);
	  printf("X0_CT_CTRL: 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_CT_CTRL, 0x00100313);
	  printf("X0_CT_CTRL: 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_CT_CTRL, 0x0010031f);
	  printf("X0_CT_CTRL: 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_CT_CTRL, 0x0010033f);
	  printf("X0_CT_CTRL: 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_CT_CTRL, 0x00100f3f);
	  printf("X0_CT_CTRL: 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_CT_CTRL, 0x0011ff3f);
	  printf("X0_CT_CTRL: 0x%x\n", HW_REGISTER (X0_CT_CTRL));
      NLM_WASTE_TIME

	// MAC Address	0xf81edf86ee2f
      SET_HW_REGISTER (X0_MAC_ADDR_0_, 0x86df1ef8);
	  printf("X0_MAC_ADDR_0_ : 0x%x\n", HW_REGISTER (X0_MAC_ADDR_0_));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_MAC_ADDR_1_, 0x2fee);
	  printf("X0_MAC_ADDR_1_ : 0x%x\n", HW_REGISTER (X0_MAC_ADDR_1_));
      NLM_WASTE_TIME

	// TX/RX_ENA, LAN Mode, 
     // SET_HW_REGISTER (X0_COMMAND_CONFIG_, 0x000281b3);
      SET_HW_REGISTER (X0_COMMAND_CONFIG_, 0x000283b3);
	  printf("X0_COMMAND_CONFIG_ : 0x%x\n", HW_REGISTER (X0_COMMAND_CONFIG_));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_FRM_LENGTH, 0x00002400);
	  printf("X0_FRM_LENGTH : 0x%x\n", HW_REGISTER (X0_FRM_LENGTH));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_PAUSE_QUANT_, 0x00000100);
	  printf("X0_PAUSE_QUANT_ : 0x%x\n", HW_REGISTER (X0_PAUSE_QUANT_));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_TX_FIFO_SECTIONS, 0x00000010);
	  printf("X0_TX_FIFO_SECTIONS : 0x%x\n", HW_REGISTER (X0_TX_FIFO_SECTIONS));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_DMA_CONFIG, 0x00000011); 
	  printf("X0_DMA_CONFIG : 0x%x\n", HW_REGISTER (X0_DMA_CONFIG));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_DMA_CONFIG, 0x1);
	  printf("X0_DMA_CONFIG : 0x%x\n", HW_REGISTER (X0_DMA_CONFIG));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_TTYPE, 0x43414331);
	  printf("X0_TTYPE : 0x%x\n", HW_REGISTER (X0_TTYPE));
      NLM_WASTE_TIME

      SET_HW_REGISTER (X0_RTYPE, 0x4321);
	  printf("X0_RTYPE : 0x%x\n", HW_REGISTER (X0_RTYPE));
      NLM_WASTE_TIME


#ifdef NLM_PRBS_TEST
      SET_HW_REGISTER (X0_CT_TEST, 0x0004000F);
	  printf("X0_CT_TEST : 0x%x\n", HW_REGISTER (X0_CT_TEST));
      NLM_WASTE_TIME
 
      SET_HW_REGISTER (X0_CT_TEST, 0x000400FF);
	  printf("X0_CT_TEST : 0x%x\n", HW_REGISTER (X0_CT_TEST));
      NLM_WASTE_TIME
#endif

    }
  else if (config->ring_id == NLM_XAUI_RING_1)
    {
      if (HW_REGISTER (PDF0_CONFIG) == PDF0_CONFIG_DEFAULT || HW_REGISTER (PDF0_CONFIG) == 0)
        /* master ring was not setup */
        return NLM_INCORRECT_ACCESS;

      /* Bring the xaui link up */
      SET_HW_REGISTER (X1_CT_TEST, 0x00040000);
      NLM_WASTE_TIME
        
      SET_HW_REGISTER (X1_CT_CFG0, 0x8888);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CFG1, 0x00000100);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x00100000);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x00100003);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x00100013);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x00100313);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x0010031f);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x0010033f);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x00100f3f);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_CT_CTRL, 0x0011ff3f);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_COMMAND_CONFIG_, 0x000281b3);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_FRM_LENGTH, 0x00002400);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_PAUSE_QUANT_, 0x00000100);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_TX_FIFO_SECTIONS, 0x00000010);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_DMA_CONFIG, 0x00000011);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_DMA_CONFIG, 0x1);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_TTYPE, 0x43414331);
      NLM_WASTE_TIME

      SET_HW_REGISTER (X1_RTYPE, 0x4321);
      NLM_WASTE_TIME

#ifdef NLM_PRBS_TEST
      SET_HW_REGISTER (X1_CT_TEST, 0x0004000F);
      NLM_WASTE_TIME
 
      SET_HW_REGISTER (X1_CT_TEST, 0x000400FF);
      NLM_WASTE_TIME
#endif

    }
  else /* for any non-master ring, check that master ring is up */
    {
      if (HW_REGISTER (PDF0_CONFIG) == PDF0_CONFIG_DEFAULT || HW_REGISTER (PDF0_CONFIG) == 0)
        /* master ring was not setup */
        return NLM_INCORRECT_ACCESS;

#if 0
      /* FIXME: Not sure about the intention of this test
         Does not seem to hold true on the FPGA's. */
      if (HW_REGISTER (PDF_CONFIG) != PDF0_CONFIG_DEFAULT)
        /* this ring was already setup once or unavailable on fpga, we cannot use it */
        return NLM_INCORRECT_ACCESS;
#endif
    }

  init_pointer_mirror_area (phy_device, &free_ptr);

  if (free_ptr >= top_of_heap)
    return NLM_OUT_OF_DEVICE_MEMORY;

  init_ddr_memory (phy_device, &free_ptr);

  if (free_ptr >= top_of_heap)
    return NLM_OUT_OF_DEVICE_MEMORY;

  if (phy_device->hw_type != 5) /* Not for Xaui ex2 */
    {
      /* Use SCRATCH0 and SCRATCH1 registers for storing sysmem size and DDR mem
         size respectively for ring 0
         Use SCRATCH2 and SCRATCH3 registers for storing sysmem size and DDR mem
         size respectively for ring 1 and so on */
      SET_HW_REGISTER (SCRATCH0 + (2 * config->ring_id) * 4, config->sysmem_size);
      if (config->size_of_ddr_memory >> 32)
        SET_HW_REGISTER (SCRATCH0 + (2 * config->ring_id + 1) * 4, 0);
      else
        SET_HW_REGISTER (SCRATCH0 + (2 * config->ring_id + 1) * 4, config->size_of_ddr_memory);
    }
  
  if (phy_device->hw_type != 5) /* Dont do it on Xaui ex2 */
    {
      
      if (config->ring_id == NLM_MASTER_RING) /* only master ring can load/unload database */
        {
          /* init database spill over area */
          init_database_extension_area (phy_device, &free_ptr);
        }

      /* init context save/restore area */
      if (phy_device->use_ddr_memory)
        {
          status = init_context_save_restore_area_ddr (phy_device);
          if (status != NLM_OK)
            return status;
        }
      else
        {
          status = init_context_save_restore_area_sysmem (phy_device, &free_ptr);
          if (status != NLM_OK)
            return status;
        }
    }

  /* init FD fifo */
  init_input_fifo (phy_device, &free_ptr);
  
  if (free_ptr >= top_of_heap)
    return NLM_OUT_OF_DEVICE_MEMORY;

  /* init RD fifo */
  init_output_fifo (phy_device, &free_ptr);
  
  if (free_ptr >= top_of_heap)
    return NLM_OUT_OF_DEVICE_MEMORY;

  status = init_manager_thread_fifo (phy_device, &free_ptr, top_of_heap);

  if (status != NLM_OK)
    return status;
  
  /* init PD */
  free_ptr = (char *) (((nlm_uintptr_t) free_ptr + 127) & ~127); /* round up to 128 alignment */
  phy_device->pd_base = (struct hw_pd_descriptor *) free_ptr;
  phys_addr = CONVERT_TO_PHYSICAL (phy_device->pd_base);
  DPRINT ("pd_base = 0x%016llx, %p\n", phys_addr, phy_device->pd_base);
  HW_SET_PD_BASE (phys_addr);

  if (!phy_device->use_ddr_memory)
    {
      phy_device->u.pdf_base = (struct hw_pdf_descriptor *) phy_device->pd_base;
      HW_SET_PDF_BASE (phys_addr);
      if (((struct hw_pdf_descriptor *) top_of_heap - phy_device->u.pdf_base) >> 24 != 0)
        top_of_heap = (char *) (phy_device->u.pdf_base + (1 << 24) - 1);
    }

  phy_device->chunksize = (top_of_heap - free_ptr) / config->max_threads;
  if (phy_device->chunksize < (32 * sizeof (struct hw_pd_descriptor)))
    /* there should be room for at least some PDs in the system memory */
    return NLM_OUT_OF_DEVICE_MEMORY;

  /* configure device registers */
  configure_device (phy_device);

  if (config->ring_id != NLM_MASTER_RING)
    {
      /* slave rings should not touch gid or enable fpga or context ring */

      /* check that memory ranges do not overlap */
      status = check_mem_overlap (phy_device);
      if (status != NLM_OK)
        {
          SET_HW_REGISTER (PDF_CONFIG, PDF0_CONFIG_DEFAULT); /* mark the ring as unused */
          return status;
        }

      return NLM_OK;
    }

  status = init_hw_memory (phy_device);
  if (status != NLM_OK)
    return status;

  SET_HW_REGISTER (SYS_CONTROL, 0x1); /* enable fpga/DMA */
  
  if (phy_device->hw_type != 5)
    return check_device_sanity (phy_device);

  return NLM_OK;
}

/* initialize device with 'config' and return pointer to it */
static nlm_status
nlm_device_init_1 (struct nlm_device_config *config, nlm_device_t **p_device)
{
  nlm_status status;
  char *memory_pool;
  uint32_t memory_pool_size;

  printf("[%s] ingress\n", __FUNCTION__);
  
  api_assert (sizeof (void*) == sizeof (nlm_uintptr_t)
              && sizeof (uint32_t) == 4
              && sizeof (nlm_phys_addr) == 8);
  api_assert (sizeof (struct hw_pdf_descriptor) == 128);
  api_assert (sizeof (struct hw_result_descriptor) == 16);
  api_assert (sizeof (struct hw_pd_descriptor) == 16);
  api_assert (sizeof (struct hw_fd_descriptor) == 32);

  *p_device = NULL;

  if (/* ignore the size of database spill over are */
      config->register_map_base_virt == NULL
      || config->register_map_base_phys == 0
      || config->sysmem_base_virt == NULL
      || config->sysmem_size < NLM_MIN_SYSMEM_SIZE
      /* context save/restore area shouldn't be less than 8k */
      || config->size_of_context_save_restore_area < NLM_MIN_CONTEXT_SAVE_RESTORE_AREA
      || (config->memory_pool == NULL && config->xmalloc == NULL)
      /* size of memory pool in bytes cannot be less than 32k when
         specified. When xmalloc/xfree pair are used, they can be less
         than the min 32K for each thread */
      || (config->memory_pool != NULL
          && config->memory_pool_size < NLM_MIN_MEMORY_POOL_SIZE)
      /* input and output fifos cannot be of zero size or larger than 64M entries */
      || config->input_fifo_size == 0
      || config->input_fifo_size > NLM_MAX_INPUT_FIFO_SIZE
      || config->output_fifo_size == 0
      || config->output_fifo_size > NLM_MAX_OUTPUT_FIFO_SIZE
      || config->max_threads == 0
      || config->max_threads > NLM_MAX_THREADS)
    return NLM_INVALID_DEVICE_CONFIG;

  if (config->thread_id != 0)
    return NLM_INVALID_THREAD_ID;

  /* check alignment of system memory */
  if ((nlm_uintptr_t) config->sysmem_base_virt & 7)
    return NLM_INVALID_DEVICE_CONFIG;

  /* catch ugly bugs early */
  if (config->memory_pool == config->sysmem_base_virt
      || config->register_map_base_virt == config->memory_pool
      || config->register_map_base_virt == config->sysmem_base_virt
      || config->register_map_base_phys == config->sysmem_base_phys)
    return NLM_INVALID_DEVICE_CONFIG;

  /* input fifo size needs to be power of 2 */
  if (config->input_fifo_size & (config->input_fifo_size - 1))
    return NLM_INVALID_DEVICE_CONFIG;

  /* output fifo size needs to be power of 2 */
  if (config->output_fifo_size & (config->output_fifo_size - 1))
    return NLM_INVALID_DEVICE_CONFIG;

  if (config->xmalloc)
    {
      memory_pool = config->xmalloc (config->cookie, config->memory_pool_size);
      memory_pool_size = config->memory_pool_size;
      if (memory_pool == NULL
          || ((nlm_uintptr_t) memory_pool & 7))
        return NLM_INVALID_DEVICE_CONFIG;
    }
  else
    {
      memory_pool = config->memory_pool;
      memory_pool_size = config->memory_pool_size;
      
      if (memory_pool == NULL
          || ((nlm_uintptr_t) memory_pool & 7))
        return NLM_INVALID_DEVICE_CONFIG;
      
      /* Lets partition the memory pool among all the threads */
      if (config->using_manager_thread)
        {
          /* Reserve space at the bottom of the user memory pool
             for the manager thread device handle. */
          memory_pool += sizeof (nlm_device_t);
          /* round up to 8 byte alignment */
          memory_pool = (char *) (((nlm_uintptr_t) memory_pool + 7) & ~7); 
          memory_pool_size -= (memory_pool - (char *) config->memory_pool);
        }
      memory_pool_size /= config->max_threads;
      memory_pool_size &= ~7;
    }

  /* if caller didn't setup device_config properly we will segfault here */
  *(uint32_t *) config->sysmem_base_virt = 0;
  *(uint32_t *) memory_pool = 0;

  status = physical_device_init (config);
  if (status != NLM_OK)
    return status;
  
  return device_attach (config, p_device, (nlm_phy_device_t *) config->sysmem_base_virt, 
                        memory_pool, memory_pool_size);
}

nlm_status
NLM_PROTO (nlm_device_init) (struct nlm_device_config *config, nlm_device_t **p_device)
{
  if (config == NULL || p_device == NULL)
    return NLM_INVALID_ARGUMENT;

  if (config->ring_id != NLM_MASTER_RING)
    /* only master ring (ring_id = 0) can do 'nlm_device_init' */
    return NLM_INVALID_RING_ID;

  return nlm_device_init_1 (config, p_device);
}

static nlm_status
compare_configs (struct nlm_device_config *master_config, struct nlm_device_config *config)
{
  if (master_config->register_map_base_virt != config->register_map_base_virt
      || master_config->register_map_base_phys != config->register_map_base_phys
      || master_config->register_map_size != config->register_map_size
      || master_config->sysmem_base_virt != config->sysmem_base_virt
      || master_config->sysmem_base_phys != config->sysmem_base_phys
      || master_config->sysmem_size != config->sysmem_size
      || master_config->input_fifo_size != config->input_fifo_size
      || master_config->output_fifo_size != config->output_fifo_size
      /*|| master_config->size_of_database_extension_area != config->size_of_database_extension_area
        || master_config->size_of_context_save_restore_area != config->size_of_context_save_restore_area*/
      || master_config->virt_to_phys != config->virt_to_phys
      || master_config->packet_base_phys != config->packet_base_phys
      || master_config->packet_base_virt != config->packet_base_virt
      || master_config->base_addr_of_ddr_memory != config->base_addr_of_ddr_memory
      || master_config->size_of_ddr_memory != config->size_of_ddr_memory)
    return NLM_INVALID_DEVICE_CONFIG;
  
  if (master_config->ring_id != config->ring_id
      || master_config->max_threads != config->max_threads
      || master_config->using_manager_thread != config->using_manager_thread
      || master_config->xmalloc != config->xmalloc
      || master_config->xfree != config->xfree
      /* With xmalloc and xfree, memory pool size can be different for
         for each of the threads */
      || (master_config->memory_pool != NULL
          && master_config->memory_pool_size != config->memory_pool_size))
    return NLM_INVALID_DEVICE_CONFIG;

  if (master_config->xmalloc == NULL)
    {
      /* The memory pool pointers must be the same if xmalloc
         has not been provided */
      if (master_config->memory_pool != config->memory_pool)
        return NLM_INVALID_DEVICE_CONFIG;
    }
  
  if (config->using_manager_thread
      && config->thread_id == NLM_MANAGER_THREAD_ID)
    return NLM_OK;
  
  if (config->thread_id >= master_config->max_threads)
    return NLM_INVALID_THREAD_ID;

  return NLM_OK;
}

nlm_status
NLM_PROTO (nlm_device_attach) (struct nlm_device_config *config, nlm_device_t **p_device)
{
  nlm_phy_device_t *phy_device;
  char *memory_pool;
  uint32_t memory_pool_size;

  printf("[%s] in, ring_id:%d, thread_id:%d\n", __FUNCTION__, config->ring_id, config->thread_id);
  
  if (config == NULL || p_device == NULL)
    return NLM_INVALID_ARGUMENT;

  if (config->ring_id == NLM_MASTER_RING
      && config->thread_id == 0)
    /* 'nlm_device_attach' should not be used for master ring */
    return NLM_INVALID_RING_ID;

  /* Master thread should not attach in Mars,
     and thread ID cannot be more than max threads */
  if (config->thread_id > (NLM_MAX_THREADS - 1)
      && config->thread_id != NLM_MANAGER_THREAD_ID)
    return NLM_INVALID_THREAD_ID;

  if (config->ring_id >= NLM_LAST_UNUSED_RING_ID) /* unsupported ring id */
    return NLM_INVALID_RING_ID;

  if (config->thread_id == 0)
    /* This is the first time initialization of the
       ring. Treat it similar to a regular device init.
       Divide up the memory etc. */
    return nlm_device_init_1 (config, p_device);

  /* This is a DP thread attaching to an existing ring.
     We should compare our specified configuration
     against the device_init configuration */

  phy_device = (nlm_phy_device_t *) config->sysmem_base_virt;
  if (phy_device == NULL)
    return NLM_INVALID_DEVICE_CONFIG;
  
  if (phy_device->device_magic != NLM_DEVICE_MAGIC)
    return NLM_DEVICE_UNINITIALIZED;
  
  if (compare_configs (&phy_device->config, config) != NLM_OK)
    return NLM_INVALID_DEVICE_CONFIG;

  if (config->thread_id == NLM_MANAGER_THREAD_ID)
    {
      nlm_device_t *device;
      /* Manager threads, do not require a lot of memory,
         we just allocate enough to to hold a device
         handle */
      if (phy_device->config.using_manager_thread == 0)
        return NLM_INVALID_DEVICE_CONFIG;

      if (config->xmalloc != NULL)
        {
          memory_pool = config->xmalloc (config->cookie, sizeof (nlm_device_t));
          if (memory_pool == NULL)
            return NLM_INVALID_DEVICE_CONFIG;
          memset (config->memory_pool, 0, sizeof (nlm_device_t));
          device = (nlm_device_t *) memory_pool;
        }
      else
        {
          /* This is a shared memory pool, we will
             have a device memory allocated at the
             base of the memory pool. */
          device = (nlm_device_t *) config->memory_pool;
        }
      
      device->phy_device = phy_device;
      device->thread_id = config->thread_id;
      device->fms_ring_id = config->ring_id;
      device->memory_pool = config->memory_pool;
      device->cookie = config->cookie;
      device->memory_pool_size = sizeof (nlm_device_t);

      *p_device = device;
      return NLM_OK;
    }

  /* Regular DP threads. We will wither carve memory off
     the global memory pool, or we will malloc it using
     the callback function */
  memory_pool = NULL;
  if (config->memory_pool)
    {
      if ((nlm_uintptr_t) config->memory_pool & 7)
        /* check memory pool alignment */
        return NLM_INVALID_DEVICE_CONFIG;
      
      /* If the user specifies the memory pool, then we will divide
         it equally among all the DP threads */
      memory_pool = config->memory_pool;
      memory_pool_size = config->memory_pool_size;

      if (config->using_manager_thread)
        {
          /* Reserve space at the bottom of the user memory pool
           * for the manager thread device handle. */
          memory_pool += sizeof (nlm_device_t);
          /* round up to 8 byte alignment */
          memory_pool = (char *) (((nlm_uintptr_t) memory_pool + 7) & ~7);
          memory_pool_size -= (memory_pool - (char *) config->memory_pool);
        }

      memory_pool_size /= config->max_threads;
      memory_pool_size &= ~7;
      memory_pool = memory_pool + memory_pool_size * config->thread_id;
    }
  else if (config->xmalloc)
    {
      /* With xmalloc, we will exclusively allocate our
         user memory */
      memory_pool = config->xmalloc (config->cookie, config->memory_pool_size);
      if ((nlm_uintptr_t) config->memory_pool & 7)
        /* check memory pool alignment */
        return NLM_INVALID_DEVICE_CONFIG;
      memory_pool_size = config->memory_pool_size & ~7;
    }

  if (memory_pool == NULL)
    return NLM_INVALID_DEVICE_CONFIG;
  
  printf("[%s] out\n", __FUNCTION__);

  return device_attach (config, p_device, phy_device, memory_pool, memory_pool_size);
}

/* shutdown device */
nlm_status
NLM_PROTO (nlm_device_fini) (nlm_device_t *device)
{
  nlm_status status = NLM_OK;
  uint32_t i;
  nlm_phy_device_t *phy_device;
  
  /* user supplied memory pool cannot be empty */
  api_assert (device->memory_pool != NULL);
  phy_device = device->phy_device;
  
#if !defined (NLM_HW_FMS) || defined (PERF_TEST)
  if (0)
#endif
  status = check_context_ring (phy_device);

#ifdef DEBUG_DUMP_PDFS
  {
    char *ptr = device->memory_pool;
    struct nlm_flow *flow;
    ptr += sizeof (nlm_device_t);

    ptr = (char *) (((nlm_uintptr_t) ptr + 7) & ~7); /* round up to 8 byte alignment */
    flow = (struct nlm_flow *) ptr;
    for (;flow < device->first_free_flow; flow++)
      {
        if (!flow->is_stateless)
          {
            if (flow->type == NLM_FLOW_TYPE_INVALID)
              DDPRINT ("invalid");
            else if (flow->type == NLM_FLOW_TYPE_STATEFUL)
              DDPRINT ("stateful");
            else
              continue;
            DDPRINT (" pdf_index = %d\n", flow->u.stateful.pdf_index);
            nlm_debug_dump_pdf (device, flow->u.stateful.pdf_index);
          }
      }
  }
#endif

  /* DPRINT ("active_block_map = 0x%x\n", HW_REGISTER (ACTIVE_BLOCK_MAP)); */
  SET_HW_REGISTER (SYS_CONTROL, 0); /* disable fpga/DMA */

  SET_HW_REGISTER (DDR_DMA_ENABLE, 0); /* disable dma0 */

  /* stop all blocks */
  for (i = 0; i < 12; i++)
    {
      SET_HW_REGISTER (BLOCK_CTRL_MAP, 1 << i);
      SET_HW_REGISTER (BLOCK_CTRL, 1); /* stop block */
      /* DPRINT ("block_ctrl = 0x%x\n", HW_REGISTER (BLOCK_CTRL)); */
    }

  /* DPRINT ("active_block_map = 0x%x\n", HW_REGISTER (ACTIVE_BLOCK_MAP)); */

  {
    uint32_t wait_cnt = 1 << 12;
    while (HW_REGISTER (SYS_STATUS) != 0x700 && wait_cnt)
      wait_cnt --;
  }

  SET_HW_REGISTER (SYS_RESET, 0); /* reset */
  SET_HW_REGISTER (SYS_RESET, 0xffffFFFF); /* set to ff */
  
  return status;
}

/* shutdown slave ring */
nlm_status
NLM_PROTO (nlm_device_detach) (nlm_device_t *device)
{
  nlm_phy_device_t *phy_device;
  uint32_t detach_tid;
  
  /* SET_HW_REGISER (PDF_CONFIG, PDF0_CONFIG_DEFAULT); */ /* mark the ring as unused */
  /* really need to have a sw reset per ring here */
  if (device == NULL)
    return NLM_INVALID_ARGUMENT;

  phy_device = device->phy_device;
  if (phy_device->device_magic != NLM_DEVICE_MAGIC)
    return NLM_DEVICE_UNINITIALIZED;

  if (device->fms_ring_id == 0
      && device->thread_id == 0)
    {
      /* Ring 0 / thread 0 should not detach */
      return NLM_INVALID_THREAD_ID;
    }
  
  /* If we have an xfree pointer, then the memory
     pool was allocated using xmalloc, simeply
     free out memory_pool pointer */
  if (phy_device->config.xfree)
    phy_device->config.xfree (device->cookie, device->memory_pool);

  /* Decrement the number of threads attached. We want to find the last
     thread operating on the ring. For Ring0, we will never hit the
     last thread here */
  detach_tid = NLM_ATOMIC_ADD32_RETURN (&phy_device->threads_attached, -1);
  if (detach_tid == 0)
    {
      /* last thread to detach will perform validation
         of the context ring. */
      nlm_status status = check_context_ring (phy_device);
      if (status != NLM_OK)
        return status;
    }

#if !defined (NLM_HW_FMS) || defined (PERF_TEST)
  if (0)
#endif
    return check_xaui_buffers (phy_device);

  return NLM_OK;
}

nlm_status
NLM_PROTO (nlm_device_get_param_va) (nlm_device_t *device, nlm_device_param param, va_list ap)
{
  nlm_device_param curpar = param;
  nlm_status status = NLM_OK;
  uint32_t *curval;
  nlm_phy_device_t *phy_device;
  
  if (device == NULL)
    return NLM_INVALID_ARGUMENT;
  
  if (device->thread_id != 0)
    return NLM_INVALID_THREAD_ID;
  
  phy_device = device->phy_device;
  
  switch (curpar)
    {
    case NLM_MAX_MATCHES_PER_JOB:
      /* RDF_CLAMP [13:0] */
      curval = va_arg (ap, uint32_t *);
      *curval = HW_REGISTER (RDF_CLAMP0) & ((1 << 14) - 1);
      break;
    case NLM_MAX_STATES_PER_BYTE:
      status = NLM_UNSUPPORTED;
      break;
    case NLM_OUTPUT_FIFO_TIMER:
      /* RDF_FORCE_TIMER 32-bit */
      curval = va_arg (ap, uint32_t *);
      *curval = HW_REGISTER (RDF_FORCE_TIMER);
      break;
    case NLM_INPUT_FIFO_TIMER:
      /* FDF_POLL_INTERVAL 32-bit */
      curval = va_arg (ap, uint32_t *);
      *curval = HW_REGISTER (FDF_POLL_INTERVAL);
      break;
    case NLM_TIMESTAMP:
      {
        uint64_t *p_timestamp = va_arg (ap, uint64_t *);
        uint64_t timestamp;
        /* get the timestamp of HW
           The total time (latency) it takes for the packet to travel through HW
           can be determined by frd->timestamp - timestamp@start_job */
        SET_HW_REGISTER (RDF_TIMESTAMP_LOW, 0); /* write zero into timestamp to take a snapshot */
        timestamp = HW_REGISTER (RDF_TIMESTAMP_LOW);
        timestamp |= ((uint64_t) HW_REGISTER (RDF_TIMESTAMP_HI) & 0xff) << 32;
        *p_timestamp = timestamp;
        break;
      }
    case NLM_HW_REGISTER:
      {
        uint32_t reg_addr = va_arg (ap, uint32_t);
        uint32_t *var_addr = va_arg (ap, uint32_t *);
        *var_addr = HW_REGISTER (reg_addr);
        break;
      }
    case NLM_TOTAL_BLOCK_CNT:
      {
        uint32_t *p_blocks = va_arg (ap, uint32_t *);
        *p_blocks = device->phy_device->num_fms_blocks;
        break;
      }
    case NLM_DATABASE_LOAD_POLICY:
      {
        uint32_t *p_policy = va_arg (ap, uint32_t *);
        *p_policy = device->phy_device->database_load_policy;
        break;
      }
    case NLM_DATABASE_COPY_CNT:
      {
        uint32_t database_id = va_arg (ap, uint32_t);
        uint32_t *num_copies_current = va_arg (ap, uint32_t *);
        nlm_status *db_status = va_arg (ap, nlm_status *);
        *db_status = nlm_device_database_get_num_copies (device, database_id, num_copies_current);
        break;
      }
    case NLM_DATABASE_TOTAL_BLOCK_CNT:
      {
        /* uint32_t database_id = va_arg (ap, uint32_t); */
        /* uint32_t *p_blocks = va_arg (ap, uint32_t *); */
        /* status = nlm_device_get_database_total_block_cnt_fms (device, database_id, p_blocks); */
        break;
      }
    case NLM_FLOW_OFFSET:
      {
        struct nlm_flow *flow = va_arg (ap, struct nlm_flow *);
        uint64_t *offset = va_arg (ap, uint64_t *);
        if (flow == NULL || flow->type != NLM_FLOW_TYPE_STATEFUL)
          return NLM_INVALID_FLOW;
        if (offset == NULL)
          return NLM_INVALID_ARGUMENT;
        *offset = flow->u.stateful.flow_offset;
        break;
      }
    case NLM_PACKET_ENQUEUE_POLICY:
      {
        uint32_t *p_policy = va_arg (ap, uint32_t *);
        *p_policy = device->phy_device->packet_enqueue_policy;
        break;
      }
    case NLM_DATABASE_LOADER_STATUS:
      {
        nlm_status *loader_status = va_arg (ap, nlm_status *);
        *loader_status = nlm_device_loader_cmd_exec_fms (device);
        break;
      }
    default:
      status = NLM_INVALID_ARGUMENT;
      break;
    }

  return status;
}

/* configure 'device' parameters */
nlm_status
NLM_PROTO (nlm_device_get_param) (nlm_device_t *device, nlm_device_param param, ...)
{
  va_list ap;
  nlm_status status;

  va_start (ap, param);

  status = NLM_PROTO (nlm_device_get_param_va) (device, param, ap);

  va_end (ap);

  return status;
}

nlm_status
NLM_PROTO (nlm_device_set_param_va) (nlm_device_t *device, nlm_device_param param, va_list ap)
{
  nlm_device_param curpar = param;
  nlm_status status = NLM_OK;
  uint32_t curval, regval;
  nlm_phy_device_t *phy_device;
  
  if (device == NULL)
    return NLM_INVALID_ARGUMENT;

  if (device->thread_id != 0)
    return NLM_INVALID_THREAD_ID;
  
  phy_device = device->phy_device;
  switch (curpar)
    {
    case NLM_MAX_MATCHES_PER_JOB:
      /* RDF_CLAMP [13:0] */
      curval = va_arg (ap, uint32_t);
      regval = HW_REGISTER (RDF_CLAMP0) & (~0 << 14);
      regval |= curval & ((1 << 14) - 1);
      SET_HW_REGISTER (RDF_CLAMP0, regval);
      break;
    case NLM_MAX_STATES_PER_BYTE:
      status = NLM_UNSUPPORTED;
      break;
    case NLM_OUTPUT_FIFO_TIMER:
      /* RDF_FORCE_TIMER 32-bit */
      curval = va_arg (ap, uint32_t);
      SET_HW_REGISTER (RDF_FORCE_TIMER, curval);
      break;
    case NLM_INPUT_FIFO_TIMER:
      /* FDF_POLL_INTERVAL 32-bit */
      curval = va_arg (ap, uint32_t);
      SET_HW_REGISTER (FDF_POLL_INTERVAL, curval);
      break;
    case NLM_TOTAL_BLOCK_CNT:
    case NLM_DATABASE_TOTAL_BLOCK_CNT:
    case NLM_TIMESTAMP:
      status = NLM_INCORRECT_ACCESS;
      break;
    case NLM_HW_REGISTER:
      {
        uint32_t reg_addr = va_arg (ap, uint32_t);
        uint32_t value = va_arg (ap, uint32_t);
        /* Hack to make mini-engine/asic/fpga testing easy
           If mini engine is being enabled, make the number
           of rows 64 */
        if (reg_addr == MINI_ON && value > 0)
          phy_device->num_rows_per_fms_block = 64;
        SET_HW_REGISTER (reg_addr, value);
	//	printf("[%s] reg:%x, val:%x\n", __FUNCTION__, reg_addr, value);
        break;
      }
    case NLM_DATABASE_LOAD_POLICY:
      {
        uint32_t policy = va_arg (ap, uint32_t);
        device->phy_device->database_load_policy = policy;
        break;
      }
    case NLM_DATABASE_COPY_CNT:
      {
        uint32_t database_id = va_arg (ap, uint32_t);
        uint32_t num_copies = va_arg (ap, uint32_t);
        nlm_status *db_status = va_arg (ap, nlm_status *);
        if (NLM_MAX_DATABASE_ID < database_id)
          {
            *db_status = NLM_INVALID_DATABASE_ID;
            return NLM_OK;
          }
        if (1 > num_copies || 4 < num_copies)
          {
            *db_status = NLM_INVALID_ARGUMENT;
            return NLM_OK;
          }
        if (NLM_LOADER_IN_PROGRESS == nlm_device_loader_cmd_exec_fms (device))
          return NLM_BUSY;
        else
          *db_status = nlm_device_database_set_num_copies (device, database_id, num_copies);
      }
      break;
    case NLM_DATABASE_BALANCE:
      {
        nlm_status *db_status = va_arg (ap, nlm_status *);
        *db_status = nlm_device_database_balance (device);
        break;
      }
    case NLM_FLOW_OFFSET:
      {
        struct nlm_flow *flow = va_arg (ap, struct nlm_flow *);
        uint64_t offset = va_arg (ap, uint64_t);
        if (flow == NULL || flow->type != NLM_FLOW_TYPE_STATEFUL)
          return NLM_INVALID_FLOW;
        flow->u.stateful.flow_offset = offset;
        break;
      }
    case NLM_PACKET_ENQUEUE_POLICY:
      {
        uint32_t policy = va_arg (ap, uint32_t);
        device->phy_device->packet_enqueue_policy = policy;
        break;
      }
    case NLM_FLOW_FORCE_ONE_FD:
      {
        struct nlm_flow *flow = va_arg (ap, struct nlm_flow*);
        /* manual packet enqueue:
           the next start_jobs() will create new FD with possible PD chain,
           in other words PDs of the next start_jobs() will not be try-linked into
           the end of PD chain of alive flow */
        flow->last_pd = NULL;
        break;
      }
    default:
      status = NLM_INVALID_ARGUMENT;
      break;
    }
  return status;
}

nlm_status
NLM_PROTO (nlm_device_set_param) (nlm_device_t *device, nlm_device_param param, ...)
{
  va_list ap;
  nlm_status status;

  va_start (ap, param);

  status = NLM_PROTO (nlm_device_set_param_va) (device, param, ap);

  va_end (ap);

  return status;
}

/* allocate one PDF */
static inline void
sysmem_alloc_pdf (nlm_device_t *device, struct nlm_flow *flow)
{
  nlm_phy_device_t *phy_device = device->phy_device;
  if (phy_device->use_ddr_memory)
    {
      if (device->current_pdf_index < device->max_pdf_index)
        {
          flow->u.stateful.pdf_index = device->current_pdf_index++;
          /* zero PDF in DDR */
          ((uint64_t *)phy_device->ddr_dma_area)[0] = 0;
          ((uint64_t *)phy_device->ddr_dma_area)[1] = 0;
          nlm_device_copy_into_ddr (phy_device, 
                                    phy_device->u.pdf_base_in_ddr 
                                    + flow->u.stateful.pdf_index * sizeof (struct hw_pdf_descriptor), 
                                    16 /* bytes */);
        }
      else
        flow->u.stateful.pdf_index = 0; /* out of DDR memory */
    }
  else
    {
      struct hw_pdf_descriptor *pdf;

      /* PDs are allocated in lower addresses of system memory and going up,
         PDFs are allocated in higher addresses and going down */
      pdf = device->first_free_pdf;
      if ((void *) (pdf - 1) <= (void *) device->first_free_pd)
        {
          flow->u.stateful.pdf_index = 0; /* NLM_OUT_OF_DEVICE_MEMORY */
          return;
        }
      
      /* zero PDF in system memory */
      memset (pdf, 0, sizeof (struct hw_pdf_descriptor));
      /* calculate PDF index */
      flow->u.stateful.pdf_index = pdf - phy_device->u.pdf_base;
      /* move the first_free_pdf pointer == allocate one pdf */
      device->first_free_pdf--;
    }
}

/* allocate one PD in 'flow' */
static inline struct hw_pd_descriptor *
sysmem_alloc_pd (nlm_device_t *device, struct nlm_flow *flow)
{
  struct hw_pd_descriptor *pd;
  if (device->pd_list_head)
    {
      pd = device->pd_list_head;
      device->pd_list_head = *(struct hw_pd_descriptor **) pd;
      return pd;
    }

  pd = device->first_free_pd;
  if ((struct hw_pdf_descriptor *) (pd + 1) > device->first_free_pdf)
    return NULL;
  
  /* allocate one pd */
  device->first_free_pd++;

  /* we can only have 24-bits of pd-linked jobs */
  if ((nlm_uintptr_t) (pd - device->phy_device->pd_base) >> 24)
    return NULL;
  
  return pd;
}

/* free PD */
static inline void
sysmem_free_pd (nlm_device_t *device, struct nlm_flow *flow, struct hw_pd_descriptor *pd)
{
  /* free this PD by adding it to the free pd list */
  *(struct hw_pd_descriptor **) pd = device->pd_list_head;
  device->pd_list_head = pd;
  pd->cookie = 0; /* set cookie field to invalid cookie (sequence id) value */
}

/* allocate one FD in input fifo */
static inline struct hw_fd_descriptor *
sysmem_alloc_fd (nlm_phy_device_t *phy_device)
{
  uint32_t input_wrt_ptr = phy_device->input_wrt_ptr;
  struct hw_fd_descriptor *fd = phy_device->input_fifo_base + input_wrt_ptr;

  /* input_fifo_size is power of 2 */
  if (((input_wrt_ptr + 1) & (phy_device->config.input_fifo_size - 1)) == *phy_device->input_rd_ptr)
    return NULL;

  phy_device->input_wrt_ptr = (input_wrt_ptr + 1) & (phy_device->config.input_fifo_size - 1);
  phy_device->input_wrt_ptr_dirty = 1;
  memset (fd, 0, sizeof (struct hw_fd_descriptor)); /* TODO remove it */
  return fd;
}

/* allocate nlm_flow in user supplied memory pool */
static inline struct nlm_flow *
memory_pool_alloc_flow (nlm_device_t *device, nlm_flow_type flow_type)
{
  struct nlm_flow *flow;
  if (flow_type == NLM_FLOW_TYPE_STATELESS)
    {
      if (device->stateless_flow_list_head)
        {
          flow = device->stateless_flow_list_head;
          device->stateless_flow_list_head = *(struct nlm_flow **) flow;
          /** zero flow structure including db_group_cnt and db_group_id[] */
          memset (flow, 0, sizeof (struct nlm_flow));
          return flow;
        }

      /* free space size is not necessarily a multiple of nlm_flow size */

      flow = device->first_free_flow;
      if ((void *) (flow + 1) >= (void *) device->first_free_job)
        return NULL;

      /* allocate one nlm_flow */
      device->first_free_flow = flow + 1;
      /** zero flow structure including db_group_cnt and db_group_id[] */
      memset (flow, 0, sizeof (struct nlm_flow));
      return flow;
    }
  else /* allocate stateful flow */
    {
      if (device->stateful_flow_list_head)
        {
          uint32_t pdf_index;

          flow = device->stateful_flow_list_head;
          device->stateful_flow_list_head = *(struct nlm_flow **) flow;

          /* reuse pdf_index that we used for some other flow which was freed */
          pdf_index = flow->u.stateful.pdf_index;
          api_assert (pdf_index != 0);
          /** zero flow structure including db_group_cnt and db_group_id[] */
          memset (flow, 0, sizeof (struct nlm_flow));
          flow->u.stateful.pdf_index = pdf_index;

          return flow;
        }

      flow = device->first_free_flow;
      if ((void *) (flow + 1) >= (void *) device->first_free_job)
        return NULL;

      /* allocate one nlm_flow */
      device->first_free_flow = flow + 1;
      /** zero flow structure including db_group_cnt and db_group_id[] */
      memset (flow, 0, sizeof (struct nlm_flow));

      /* allocate PDF for the first time */
      sysmem_alloc_pdf (device, flow);

      return flow;
    }
}

/* free nlm_flow in user supplied memory pool */
static inline void
memory_pool_free_flow (nlm_device_t *device, struct nlm_flow *flow)
{
  if (flow->is_stateless)
    {
      /* add flow to the free flow list */
      *(struct nlm_flow **) flow = device->stateless_flow_list_head;
      device->stateless_flow_list_head = flow;
    }
  else
    {
      /* add flow to the free flow list */
      *(struct nlm_flow **) flow = device->stateful_flow_list_head;
      device->stateful_flow_list_head = flow;
    }
}

/* allocate nlm_job in user supplied memory pool */
static inline struct nlm_job *
memory_pool_alloc_job (nlm_device_t *device)
{
  struct nlm_job *job;
  if (device->job_list_head)
    {
      job = device->job_list_head;
      device->job_list_head = *(struct nlm_job **) job;
      return job;
    }

  /* free space size is not necessarily a multiple of nlm_job size */

  job = device->first_free_job;
  if ((void *) (job - 1) <= (void *) device->first_free_flow)
    return NULL;

  /* allocate one nlm_job */
  device->first_free_job = job - 1;
  return job;
}

/* free nlm_job in user supplied memory pool */
static inline void
memory_pool_free_job (nlm_device_t *device, struct nlm_job *job)
{
  /* add job to the free job list */
  *(struct nlm_job **) job = device->job_list_head;
  device->job_list_head = job;
}

nlm_status
NLM_PROTO (nlm_create_flow) (nlm_device_t *device, nlm_flow_type flow_type, struct nlm_flow **p_flow)
{
  struct nlm_flow *flow;
  if (device == NULL || p_flow == NULL)
    return NLM_INVALID_ARGUMENT;

  *p_flow = NULL;

  /* user supplied memory pool cannot be empty */
  api_assert (device->memory_pool != NULL);

  if (flow_type != NLM_FLOW_TYPE_STATEFUL && flow_type != NLM_FLOW_TYPE_STATELESS)
    return NLM_INVALID_FLOW;

  flow = memory_pool_alloc_flow (device, flow_type);

  if (flow == NULL)
    return NLM_OUT_OF_MEMORY;

  flow->type = flow_type;

  if (flow_type == NLM_FLOW_TYPE_STATEFUL)
    {
      if (flow->u.stateful.pdf_index == 0)
        {
          if (device->phy_device->use_ddr_memory)
            return NLM_OUT_OF_DDR_MEMORY;
          else
            return NLM_OUT_OF_DEVICE_MEMORY;
        }

      flow->u.stateful.flow_offset = 0; /* byte match offset starts from 0 */
    }
  else if (flow_type == NLM_FLOW_TYPE_STATELESS)
    {
      flow->is_stateless = 1;
    }

  *p_flow = flow;
  return NLM_OK;
}

#define MUX_DB_GID(DATABASE_ID, GROUP_ID) ((DATABASE_ID & NLM_MAX_DATABASE_ID) << 10) | (GROUP_ID & NLM_MAX_GROUP_ID)

/** add (database, rule_group) pair to the set of (database, group) pairs searched in
    the subsequent packets of this flow */
nlm_status
NLM_PROTO (nlm_flow_add_database_and_group) (nlm_device_t *device, struct nlm_flow *flow,
                                             uint32_t database_id, uint32_t group_id)
{
  uint32_t db_group;
  int i;

  if (device == NULL || flow == NULL)
    return NLM_INVALID_ARGUMENT;
  if (database_id > NLM_MAX_DATABASE_ID)
    return NLM_INVALID_DATABASE_ID;
  if (group_id > NLM_MAX_GROUP_ID || group_id == 0)
    return NLM_INVALID_GROUPID;

  db_group = MUX_DB_GID (database_id, group_id);
  api_assert (db_group != 0);

  if (flow->db_group_cnt >= NLM_FMS_GROUPS_PER_PACKET)
    return NLM_INCORRECT_ACCESS; /* user tried to add 7th (database,group) to search */

  for (i = 0; i < flow->db_group_cnt; i++)
   if (flow->db_group_id[i] == db_group) /* (database, group) was already added for this flow */
     return NLM_INCORRECT_ACCESS;

  flow->db_group_id[flow->db_group_cnt++] = db_group;
  flow->db_group_combination++;
  return NLM_OK;
}

/** remove (database, rule_group) pair from the set of (database, group) pairs searched in
    the following packets of this flow */
nlm_status
NLM_PROTO (nlm_flow_remove_database_and_group) (nlm_device_t *device, struct nlm_flow *flow,
                                                uint32_t database_id, uint32_t group_id)
{
  uint32_t db_group;
  int i;
  if (device == NULL || flow == NULL)
    return NLM_INVALID_ARGUMENT;
  if (database_id > NLM_MAX_DATABASE_ID)
    return NLM_INVALID_DATABASE_ID;
  if (group_id > NLM_MAX_GROUP_ID || group_id == 0)
    return NLM_INVALID_GROUPID;

  db_group = MUX_DB_GID (database_id, group_id);
  api_assert (db_group != 0);

  for (i = 0; i < flow->db_group_cnt; i++)
   if (flow->db_group_id[i] == db_group) /* (database, group) was found in the set of this flow */
     {
       int j;
       /* shift the array */
       for (j = i; j < NLM_FMS_GROUPS_PER_PACKET - 1; j++)
         flow->db_group_id[j] = flow->db_group_id[j + 1];

       flow->db_group_id[NLM_FMS_GROUPS_PER_PACKET - 1] = 0; /* zero is invalid group_id */
       flow->db_group_cnt--;
       flow->db_group_combination++;
       return NLM_OK;
     }

  return NLM_INCORRECT_ACCESS; /* not found */
}

#ifdef PERF_TEST
/* this function is used in dataplane performance test.
   It reads FDs with chains of PDs from input fifo and generates corresponding
   PRDs and FRDs into output fifo.
   Main assumption: output fifo must not overflow */
static void
bypass_hw (nlm_phy_device_t *phy_device)
{
  uint32_t input_rd_ptr = *phy_device->input_rd_ptr;

  while (phy_device->input_wrt_ptr != input_rd_ptr)
    {
      struct hw_fd_descriptor *fd = phy_device->input_fifo_base;
      uint32_t cookie;
      uint32_t pd_nla_index;

      fd += input_rd_ptr;

      cookie = fd->cookie;

      pd_nla_index = fd->pd_nla.pd_nla_index;

      /* let dataplane know that FD was processed */
      input_rd_ptr++;
      input_rd_ptr &= phy_device->config.input_fifo_size - 1;

      process_next_packet:

      { /* now add FRD into the output fifo */
        struct hw_result_descriptor *rd = phy_device->output_fifo_base;

        rd += *phy_device->output_wrt_ptr;

        /* fill in RD/FRD fields */
        rd->u1.hw_frd.result_type = pd_nla_index ? FMS_PRD_RESULT : FMS_FRD_RESULT;
        rd->u1.hw_frd.error_code = 0;
        rd->u1.hw_frd.timestamp_lo = 0;
        rd->u4.valid_sw = 0x80000000;
        rd->cookie = cookie;

        /* commit FRD into the fifo */
        (*phy_device->output_wrt_ptr) ++;
        (*phy_device->output_wrt_ptr) &= phy_device->config.output_fifo_size - 1;
      }
      if (pd_nla_index)
        {
          struct hw_pd_descriptor *pd = phy_device->pd_base;
          pd += pd_nla_index;

          pd_nla_index = pd->pd_nla.pd_nla_index;
          cookie = pd->cookie;
          goto process_next_packet;
        }
    }
  *phy_device->input_rd_ptr = input_rd_ptr;
}
#endif

static inline void
nlm_device_update_input_wrt_ptr (nlm_phy_device_t *phy_device)
{
  /* make sure to flush CPU caches into memory, because HW will dma the FD entry
     right after we update the FD write pointer and some cpus don't flush caches
     on dma request by itself (like x86 does) */
  MEMORY_BARRIER ();
  
  /* now we can send all these FDs to HW.
     update FDF write pointer, so the device can start processing it */
  SET_HW_REGISTER (FDF_FIFO_WRT_PTR, phy_device->input_wrt_ptr);
  phy_device->input_wrt_ptr_dirty = 0;
#ifdef PERF_TEST
  bypass_hw (phy_device);
#endif
}

static inline nlm_status
store_phys_addr_into_fd (nlm_phy_device_t *phy_device, struct hw_fd_descriptor *fd,
                         nlm_phys_addr phys_addr)
{
  int i;
  int free_base = -1;

  /* mars2 = 32 + 4 bits of packet address, mars1 = 28 + 4 bits of packet address */
  nlm_phys_addr phys_addr_base = phys_addr & (~0ull << PQ_ADDR_LOW_BITS);
  fd->packet_addr = phys_addr & ((1ull << PQ_ADDR_LOW_BITS) - 1);

  if (phy_device->config.virt_to_phys == NULL)
    /* there is no callback to convert virt to phys, hence
       all packet bases were initialized at the init time around 'packet_base_phys' */
    {
       if ((phy_device->config.packet_base_phys & (~0ull << (PQ_ADDR_LOW_BITS + 4)))
           != (phys_addr_base & (~0ull << (PQ_ADDR_LOW_BITS + 4))))
         return NLM_OUT_OF_PACKET_MEMORY;

       fd->packet_base = (phys_addr_base >> PQ_ADDR_LOW_BITS) & 0xf;
       return NLM_OK;
    }

  /* allocate packet base dynamically */
  NLM_SPIN_LOCK (&phy_device->hw_packet_base_lock);
  for (i = 0; i < HW_PACKET_BASES; i++)
    if (phy_device->packet_base[i] == phys_addr_base)
      {
        /* 'i' packet base was already configured. Use it for this pd/packet */
        fd->packet_base = i;
        NLM_SPIN_UNLOCK (&phy_device->hw_packet_base_lock);
        return NLM_OK;
      }
    else if (phy_device->packet_base[i] == FREE_PACKET_BASE && free_base == -1)
      {
        free_base = i;
        break;
      }

  if (free_base == -1)
    {
      NLM_SPIN_UNLOCK (&phy_device->hw_packet_base_lock);
      return NLM_OUT_OF_PACKET_MEMORY;
    }

  phy_device->packet_base[free_base] = phys_addr_base;
  SET_HW_REGISTER (PQ_BASE_LOW_0 + free_base * 8, (uint32_t) phys_addr_base);
  SET_HW_REGISTER (PQ_BASE_HI_0 + free_base * 8, (uint32_t) (phys_addr_base >> 32));
  fd->packet_base = free_base;
  NLM_SPIN_UNLOCK (&phy_device->hw_packet_base_lock);
  return NLM_OK;
}

static inline nlm_status
store_phys_addr_into_pd (nlm_phy_device_t *phy_device, struct hw_pd_descriptor *pd,
                         nlm_phys_addr phys_addr)
{
  int i;
  int free_base = -1;

  /* mars2 = 32 + 4 bits of packet address, mars1 = 28 + 4 bits of packet address */
  nlm_phys_addr phys_addr_base = phys_addr & (~0ull << PQ_ADDR_LOW_BITS);
  pd->packet_addr = phys_addr & ((1ull << PQ_ADDR_LOW_BITS) - 1);

  if (phy_device->config.virt_to_phys == NULL)
    /* there is no callback to convert virt to phys, hence
       all packet bases were initialized at the init time around 'packet_base_phys' */
    {
       if ((phy_device->config.packet_base_phys & (~0ull << (PQ_ADDR_LOW_BITS + 4)))
           != (phys_addr_base & (~0ull << (PQ_ADDR_LOW_BITS + 4))))
         return NLM_OUT_OF_PACKET_MEMORY;

       pd->packet_base = (phys_addr_base >> PQ_ADDR_LOW_BITS) & 0xf;
       return NLM_OK;
    }

  /* allocate packet base dynamically */
  for (i = 0; i < HW_PACKET_BASES; i++)
    if (phy_device->packet_base[i] == phys_addr_base)
      {
        /* 'i' packet base was already configured. Use it for this pd/packet */
        pd->packet_base = i;
        return NLM_OK;
      }
    else if (phy_device->packet_base[i] == FREE_PACKET_BASE && free_base == -1)
      {
        free_base = i;
        break;
      }

  if (free_base == -1)
    return NLM_OUT_OF_PACKET_MEMORY;

  phy_device->packet_base[free_base] = phys_addr_base;
  SET_HW_REGISTER (PQ_BASE_LOW_0 + free_base * 8, (uint32_t) phys_addr_base);
  SET_HW_REGISTER (PQ_BASE_HI_0 + free_base * 8, (uint32_t) (phys_addr_base >> 32));
  pd->packet_base = free_base;
  return NLM_OK;
}

/** helper function for enqueue payload for the search from 'start' to 'end' in 'flow'
  and 'cookie' which is going to be accepted as-is and returned by
  nlm_get_all_search_results() */
static inline nlm_status
nlm_flow_enqueue_search_1 (nlm_device_t *device, struct nlm_flow *flow,
                           const void *start, const void *end,
                           void *cookie, struct nlm_job **p_job)
{
  struct hw_pd_descriptor *pd;
  struct hw_fd_descriptor *fd;
  nlm_uintptr_t packet_byte_count;
  nlm_phys_addr phys_addr;
  struct nlm_job *job;
  nlm_phy_device_t *phy_device;
  
  phy_device = device->phy_device;
  job = memory_pool_alloc_job (device);
  if (job == NULL)
    return NLM_OUT_OF_MEMORY;

  job->flow = flow;
  job->user_cookie = cookie;
  job->db_group_combination = flow->db_group_combination;

  fd = &job->fd;
  fd->cookie = NLM_ENCODE_COOKIE (device->thread_id, (device->job_base - job));
  fd->gid0 = flow->db_group_id[0];
  fd->gid1 = flow->db_group_id[1];
  fd->gid2 = flow->db_group_id[2];
  fd->gid3 = flow->db_group_id[3];
  fd->gid4 = flow->db_group_id[4];
  fd->gid5 = flow->db_group_id[5];
  fd->gid_cnt = flow->db_group_cnt;

  if (start == NULL && end == NULL) /* enqueue zero-length packet */
    {
      packet_byte_count = 0;
      phys_addr = phy_device->config.packet_base_phys;
    }
  else /* normal packet */
    {
      packet_byte_count = (const char *) end - (const char *) start;
      if (packet_byte_count >= NLM_MAX_PACKET_SIZE || packet_byte_count == 0)
        return NLM_INVALID_PACKET_SIZE;

      if (phy_device->config.virt_to_phys)
        phys_addr = phy_device->config.virt_to_phys (start);
      else
        phys_addr = phy_device->config.packet_base_phys
                    + (nlm_phys_addr) (nlm_uintptr_t)
                         ((const char *) start
                          - (const char *) phy_device->config.packet_base_virt);
    }

  if (flow->type == NLM_FLOW_TYPE_STATELESS)
    {
      /* Directly store all the information in the FD part
         of the job structure. No PD is required for stateless
         flows */
      job->pd = NULL;

      if (store_phys_addr_into_fd (phy_device, fd, phys_addr) != NLM_OK)
        return NLM_OUT_OF_PACKET_MEMORY;

      fd->flow_type = FMS_FLOW_STATELESS;
      fd->end_anchor = 1;
      fd->start_anchor = 1;
      fd->pdf_index = 0;

      fd->sw_disconnect = 0;
      fd->packet_prd = 1;
      fd->flow_term = 0;
      fd->packet_byte_count = packet_byte_count;

      fd->pd_nla.pd_nla_index = 0;
      fd->pd_nla.rsv_cookie = 0;
      
      *p_job = job;
      return NLM_OK;
    }
  
  /* Stateful flow */
  pd = sysmem_alloc_pd (device, flow);

  if (pd == NULL)
    {
      memory_pool_free_job (device, job);
      return NLM_OUT_OF_DEVICE_MEMORY;
    }

  if (store_phys_addr_into_pd (phy_device, pd, phys_addr) != NLM_OK)
    {
      sysmem_free_pd (device, flow, pd);
      return NLM_OUT_OF_PACKET_MEMORY;
    }


  /* calculate the job index that we're going to use to retrieve the info
     about flow, pd, job during get_all_search_results() */
  /* PD's FD's RD's cookies are 32-bit. TBD add a check in 64-bit mode */
  pd->cookie = fd->cookie;
  pd->packet_byte_count = packet_byte_count;
  pd->rsv01 = PD_CREATED_BUT_NOT_STARTED;
  pd->packet_prd = 1;
  pd->flow_term = 0;
  pd->sw_disconnect = 0;
  pd->pd_nla.pd_nla_index = 0;
  pd->pd_nla.rsv_cookie = 0;

  job->pd = pd;
  *p_job = job;

  return NLM_OK;
}

/** Enqueue payload for the search from 'start' to 'end' in 'flow'
  and 'cookie' which is going to be accepted as-is and returned by
  nlm_get_all_search_results()
  Use the set of (database, rule_group) pairs created by nlm_flow_(add|remove)_database_and_group()
  up till this point for scanning the payload. */
nlm_status
NLM_PROTO (nlm_flow_enqueue_search) (nlm_device_t *device, struct nlm_flow *flow,
                                     const void *start, const void *end,
                                     void *cookie, struct nlm_job **p_job)
{
  if (device == NULL || flow == NULL || p_job == NULL)
    return NLM_INVALID_ARGUMENT;

  *p_job = NULL;

  if (start == NULL || end == NULL)
    {
      if (start == NULL && end == NULL && (device->phy_device->packet_enqueue_policy & 1))
        /* enqueue zero-length packet */;
      else
        return NLM_INVALID_ARGUMENT;
    }

  if (flow->type != NLM_FLOW_TYPE_STATEFUL && flow->type != NLM_FLOW_TYPE_STATELESS)
    return NLM_INVALID_FLOW;

  if (flow->db_group_cnt == 0) /* no (database, group_id) pairs were added */
    return NLM_INCORRECT_ACCESS;

  if (device->phy_device->packet_enqueue_policy & 2)
    {
      /* increment combination every time, so every job is considered to be in the different group
         and started as a separate FD */
      flow->db_group_combination++;
    }

  TRY (nlm_flow_enqueue_search_1 (device, flow, start, end, cookie, p_job));
  
  return NLM_OK;
}

/* cancel 'job' that wasn't sent to the 'device' */
nlm_status
NLM_PROTO (nlm_cancel_job) (nlm_device_t *device, struct nlm_job *job)
{
  struct hw_pd_descriptor *pd;
  struct nlm_flow *flow;

  if (device == NULL || job == NULL)
    return NLM_INVALID_ARGUMENT;

  pd = job->pd;
  if (pd == NULL || pd->rsv01 != PD_CREATED_BUT_NOT_STARTED)
    /* this job was already sent to HW */
    return NLM_INVALID_JOB;

  flow = job->flow;
  if (flow == NULL)
    return NLM_INVALID_FLOW;

  if (flow->type == NLM_FLOW_TYPE_STATEFUL
      || flow->type == NLM_FLOW_TYPE_STATELESS)
    {
      /* free this PD */
      api_assert (pd->cookie); /* In valid PD cookie must be non zero */
      sysmem_free_pd (device, flow, pd);
    }
  else
    return NLM_INVALID_FLOW;

  memory_pool_free_job (device, job);

  return NLM_OK;
}

static void
nlm_device_add_flow_to_pending (nlm_device_t *device, struct nlm_flow *flow)
{
  if (device->pending_flow_list_head == NULL)
    {
      api_assert (device->pending_flow_list_tail == NULL);
      device->pending_flow_list_head = flow;
      device->pending_flow_list_tail = flow;
    }
  else
    {
      api_assert (device->pending_flow_list_tail);
      device->pending_flow_list_tail->next = flow;
      device->pending_flow_list_tail = flow;
      api_assert (flow->next == NULL);
    }
}
static void
nlm_device_remove_flow_from_pending (nlm_device_t *device, struct nlm_flow *flow)
{
  api_assert (flow == device->pending_flow_list_head);

  /* remove processed flow from the list */
  device->pending_flow_list_head = flow->next;
  flow->next = NULL;

  flow->pending_pd_head = NULL;

  if (device->pending_flow_list_head == NULL)
    {
      api_assert (flow == device->pending_flow_list_tail);
      device->pending_flow_list_tail = NULL;
    }
}

static inline void
nlm_flow_add_job_to_list (nlm_device_t *device, struct nlm_flow *flow, struct nlm_job *job,
                          struct hw_pd_descriptor *pd)
{
  if (flow->last_pd)
    {
      if (job->db_group_combination != flow->last_db_group_combination)
        /* job has different set of (database, group_id) pairs that last enqueued */
        flow->last_pd->sw_disconnect = 1;

      /* try to append PD to the list of PDs and hope that HW didn't fetch the 'last_pd' yet */
      flow->last_pd->pd_nla.pd_nla_index = pd - device->phy_device->pd_base;
    }
  else if (flow->pending_pd_head == NULL)
    {
      flow->pending_pd_head = pd;
      nlm_device_add_flow_to_pending (device, flow);
    }

  flow->last_pd = pd;
  flow->last_db_group_combination = job->db_group_combination;
  if (NLM_DEBUG)
    {
      /* DPRINT ("pdf_index = %d\n", flow->u.stateful.pdf_index); */
      nlm_debug_dump_pd (device->phy_device, pd);
    }
}

static nlm_status
nlm_flow_start_pending_pd_chain (nlm_device_t *device, struct nlm_flow *flow)
{
  struct hw_pd_descriptor *pd = flow->pending_pd_head;
  struct nlm_job *job = device->job_base - NLM_GET_JOB_INDEX (pd->cookie);
  struct hw_fd_descriptor *fd;
  nlm_phy_device_t *phy_device = device->phy_device;
  nlm_job_fifo_t *sw_job_fifo = device->sw_job_fifo;
  
  if (sw_job_fifo)
    fd = peek_job_fifo_input (sw_job_fifo);
  else
    {
      fd = sysmem_alloc_fd (phy_device);
      if (fd == NULL)
        {
          /* input fifo is full, cannot start any more FDs at this time */
          return NLM_INPUT_FIFO_FULL;
        }
    }

  nlm_device_remove_flow_from_pending (device, flow);

  if (/* job->hw_db_group_cnt == 0 -> there are no active groups in this flow
         and zero-length packet is not needed -> enqueue single flow_reinit FD */
      job->fd.gid_cnt == 0 && pd->flow_term)
    {
      fd->flow_type = FMS_FLOW_REINIT;
    }
  else if (/* 'flow_term' bit set -> end-of-flow job enqueued by destroy_stateful_flow() */
           pd->flow_term /* and group_cnt != 0*/)
    {
      fd->flow_type = FMS_FLOW_STATEFUL;
      HW_COPY_PD_INTO_FD (pd, fd);

      /* use this FD to send zero-length packet */
      api_assert (fd->packet_byte_count == 0);

      /* this FD does not terminate the flow, but the chained PD will */
      fd->flow_term = 0;

      /* do not generate PRD for this FD */
      fd->packet_prd = 0;

      /* do end anchor check for the last zero-length packet of the flow */
      fd->end_anchor = 1;

      /* last packet doesn't need start anchor check */
      fd->start_anchor = 0;

      fd->pd_nla.rsv_cookie = 0;

      /* link current PD with flow_term bit into this FD */
      fd->pd_nla.pd_nla_index = pd - device->phy_device->pd_base;
    }
  else
    {
      fd->flow_type = FMS_FLOW_STATEFUL;
      HW_COPY_PD_INTO_FD (pd, fd);

      fd->end_anchor = 0; /* no end anchor check for normal packets */
      fd->pd_nla.rsv_cookie = 0;
      /* link next PD in the chain into this FD */
      fd->pd_nla.pd_nla_index = pd->pd_nla.pd_nla_index;

      if (job->flow_offset == 0) /* first packet of the flow */
        fd->start_anchor = 1;
      else
        fd->start_anchor = 0;
    }

  /* copy the cookie field into FD */
  fd->cookie = pd->cookie;

  fd->gid_cnt = job->fd.gid_cnt;
  fd->gid0 = job->fd.gid0;
  fd->gid1 = job->fd.gid1;
  fd->gid2 = job->fd.gid2;
  fd->gid3 = job->fd.gid3;
  fd->gid4 = job->fd.gid4;
  fd->gid5 = job->fd.gid5;

  fd->pdf_index = flow->u.stateful.pdf_index;
  api_assert (flow->u.stateful.pdf_index >> 24 == 0);

  if (NLM_DEBUG)
    nlm_debug_dump_fd (device->phy_device, fd);

  if (sw_job_fifo)
    {
#if defined(_MIPS_ARCH_OCTEON)
      MEMORY_BARRIER();
#endif
      push_job_fifo (sw_job_fifo);
    }
  
  return NLM_OK;
}

static void
nlm_try_start_stateful_flows (nlm_device_t *device)
{
  nlm_phy_device_t *phy_device = device->phy_device;
  
  if (!device->sw_job_fifo)
    NLM_SPIN_LOCK (&phy_device->hw_input_fifo_lock);

  while (device->pending_flow_list_head) /* there is a flow with pending to start jobs */
    {
      if (nlm_flow_start_pending_pd_chain (device, device->pending_flow_list_head) != NLM_OK)
        /* don't try all pending flows if the input fifo is full */
        break;
    }

  if (!device->sw_job_fifo)
    {
      if (phy_device->input_wrt_ptr_dirty)
        nlm_device_update_input_wrt_ptr (phy_device);
      
      NLM_SPIN_UNLOCK (&phy_device->hw_input_fifo_lock);
    }
}

static void
nlm_flow_notice_job_completion (nlm_device_t *device, struct nlm_flow *flow,
                                struct hw_pd_descriptor *pd)
{
  struct hw_pd_descriptor *next_pd;

  if (flow->last_pd == pd)
    {
      /* we received FRD for the PD we wanted to use for try-linking,
         zero 'last_pd', so the next PD for this flow will be sent as FD */
      flow->last_pd = NULL;
      api_assert (pd->pd_nla.pd_nla_index == 0);
      return;
    }

  if (pd->pd_nla.pd_nla_index == 0)
    /* last job of the flow nothing to do */
    return;

  api_assert (flow->pending_pd_head == NULL);

  next_pd = device->phy_device->pd_base + pd->pd_nla.pd_nla_index;

  flow->pending_pd_head = next_pd;
  nlm_device_add_flow_to_pending (device, flow);
}

/* start searching 'jobs' on the 'device' */
nlm_status
NLM_PROTO (nlm_start_jobs) (nlm_device_t *device, uint32_t n_jobs, struct nlm_job *jobs[])
{
  int i, n_stateless, n_stateful;
  nlm_phy_device_t *phy_device;
  
  if (device == NULL || n_jobs == 0 || jobs == NULL)
    return NLM_INVALID_ARGUMENT;

  phy_device = device->phy_device;
  n_stateless = 0;
  n_stateful = 0;
  
  /* Pre-check all jobs structures to be sure we can start the
     process of submitting jobs */
  for (i = 0; i < n_jobs; i++)
    {
      struct nlm_flow *flow;
      if (jobs[i] == NULL)
        return NLM_INVALID_JOB;

      if ((flow = jobs[i]->flow) == NULL)
        return NLM_INVALID_JOB;

      if (flow->type == NLM_FLOW_TYPE_STATELESS)
        n_stateless++;
      else if (flow->type == NLM_FLOW_TYPE_STATEFUL)
        n_stateful++;
      else
        return NLM_INVALID_FLOW;
    }
  
  if (n_stateless)
    {
      /* For stateless jobs, we go through the usual
         process of allocating system FD's and copying
         our software FD into it. When manager thread is
         used, simply push the FD into the software FIFO */
      if (device->sw_job_fifo)
        {
          nlm_job_fifo_t *fifo = device->sw_job_fifo;
          if (job_fifo_free (fifo) < n_stateless)
            return NLM_INPUT_FIFO_FULL;
          
          for (i = 0; i < n_jobs; i++)
            {
              if (jobs[i]->flow->type == NLM_FLOW_TYPE_STATELESS)
                {
                  struct hw_fd_descriptor *fd;
                  fd = peek_job_fifo_input (fifo);
                  api_assert (fd != NULL);
                  
                  *fd = jobs[i]->fd;
#if defined(_MIPS_ARCH_OCTEON)
                  MEMORY_BARRIER();
#endif
                  push_job_fifo (fifo);
                }
            }
        }
      else
        {
          uint32_t saved_input_wrt_ptr;

          NLM_SPIN_LOCK (&phy_device->hw_input_fifo_lock);
          saved_input_wrt_ptr = phy_device->input_wrt_ptr;

          /* send FDs for all stateless jobs,
             if something is wrong, clear all created FDs and return an error,
             so either all jobs are started or none */
          for (i = 0; i < n_jobs; i++)
            {
              struct nlm_flow *flow = jobs[i]->flow;
              if (flow->type == NLM_FLOW_TYPE_STATELESS)
                {
                  struct hw_fd_descriptor *fd;
                  /* create new fd for this job */
                  fd = sysmem_alloc_fd (phy_device);
                  if (fd == NULL)
                    {
                      /* automatically free all FDs allocated by sysmem_alloc_fd()
                         in previous iterations of this loop */
                      phy_device->input_wrt_ptr = saved_input_wrt_ptr;
                      NLM_SPIN_UNLOCK (&phy_device->hw_input_fifo_lock);
                      return NLM_INPUT_FIFO_FULL;
                    }

                  *fd = jobs[i]->fd;
                  flow->u.stateless.job_seq_id++; /* increment the number of in-flight jobs in this stateless flow */
                  jobs[i]->flow_offset = 0; /* stateless flows don't keep track of offsets */
                  /* we cannot use fd->pd_nla.pd_nla_index to the next PD of stateless flow */
                  nlm_debug_dump_fd (phy_device, fd);
                }
            }

          if (n_stateful == 0)
            /* let the HW know of our jobs */
            nlm_device_update_input_wrt_ptr (device->phy_device);

          NLM_SPIN_UNLOCK (&phy_device->hw_input_fifo_lock);
        }
    }

  if (n_stateful == 0)
    return NLM_OK;
  
  /* process stateful jobs */
  for (i = 0; i < n_jobs; i++)
    {
      struct nlm_flow *flow;

      flow = jobs[i]->flow;
      if (flow->type == NLM_FLOW_TYPE_STATEFUL)
        {
          struct hw_pd_descriptor *pd;
          pd = jobs[i]->pd;

          api_assert (flow->u.stateful.pdf_index);

          /* remember the current flow_offset into job structure, so
            get_search_results can add this value to byte_offset within packet and
            have full 64-bit flow offset */
          jobs[i]->flow_offset = flow->u.stateful.flow_offset;
          /* increase flow_offset for given flow pretending that this pd(packet)
             will be scanned without errors */
          flow->u.stateful.flow_offset += pd->packet_byte_count;
          api_assert (pd->rsv01 == PD_CREATED_BUT_NOT_STARTED);

          nlm_flow_add_job_to_list (device, flow, jobs[i], pd);

          pd->rsv01 = PD_VALID_AND_STARTED;
        }
    }

  nlm_try_start_stateful_flows (device);

  return NLM_OK;
}

/* destroy 'flow' on the 'device'
   cookie should not be zero */
nlm_status
NLM_PROTO (nlm_destroy_stateful_flow) (nlm_device_t *device, struct nlm_flow *flow, void *cookie)
{
  if (device == NULL || flow == NULL)
    return NLM_INVALID_ARGUMENT;

  /* user supplied memory pool cannot be empty */
  api_assert (device->memory_pool != NULL);

  if (flow->type == NLM_FLOW_TYPE_STATEFUL)
    {
      struct nlm_job *job = NULL;

      /* set flow type to invalid to make sure no packets can be enqueued to this flow */
      flow->type = NLM_FLOW_TYPE_INVALID;

      if (flow->u.stateful.flow_offset == 0
          || flow->db_group_combination == 0)
        {
          /* No job was ever sent on this flow, or
             no database or group ID were added, so
             dont send anything to the HW, simply
             release resources */
          memory_pool_free_flow (device, flow);
          return NLM_END_OF_JOB;
        }
      
      /* enqueue zero-length packet */
      /* make sure that it goes into new FD,
         since we want end-anchor check with zero-length packet */
      flow->db_group_combination++;
      TRY (nlm_flow_enqueue_search_1 (device, flow, NULL, NULL, cookie, &job));
      job->pd->flow_term = 1;
      api_assert (job->pd->rsv01 == PD_CREATED_BUT_NOT_STARTED);
      job->pd->rsv01 = PD_VALID_AND_STARTED;
      nlm_flow_add_job_to_list (device, flow, job, job->pd);
      job->flow_offset = flow->u.stateful.flow_offset;
      nlm_try_start_stateful_flows (device);
      return NLM_OK;
    }
  else if (flow->type == NLM_FLOW_TYPE_STATELESS)
    /* cannot nlm_destroy_stateful_flow() stateless flow, because 
       it's destroyed immediately */
    return NLM_INVALID_ARGUMENT;
  else
    return NLM_INVALID_FLOW;
}

/* destroy 'flow' on the 'device'
   It sends hidden destroy/finish flow request to HW */
nlm_status
NLM_PROTO (nlm_destroy_stateless_flow) (nlm_device_t *device, struct nlm_flow *flow)
{
  if (device == NULL || flow == NULL)
    return NLM_INVALID_ARGUMENT;

  /* user supplied memory pool cannot be empty */
  api_assert (device->memory_pool != NULL);

  if (flow->type == NLM_FLOW_TYPE_STATEFUL)
    /* cannot nlm_destroy_stateless_flow() stateful flow, because it's destroyed via enqueueing special job
       that needs to be received back */
    return NLM_INVALID_ARGUMENT;
  else if (flow->type == NLM_FLOW_TYPE_STATELESS)
    {
      /* just mark it as invalid, so no more enqueue_packet() or start_jobs() for this flow can happen */
      flow->type = NLM_FLOW_TYPE_INVALID;

      if (flow->u.stateless.job_seq_id == 0) /* no pending jobs for this stateless flow */
        memory_pool_free_flow (device, flow);
      /* else get_all_results() will actually free the nlm_flow structure when all jobs for this flow
         finish */
      return NLM_OK;
    }
  else
    return NLM_INVALID_FLOW;
}


static nlm_status
convert_hw_error_code_to_status (uint32_t error_code)
{
  switch (error_code)
    {
    case 0: return NLM_OK;
    /* mars codes */
    case 1: case 4: return NLM_DEVICE_ILLEGAL_INSTRUCTION;
    case 2: case 5: return NLM_DEVICE_SRAM_ADDR;
    case 3: case 6: return NLM_DEVICE_PARITY_ERROR;
    case 7: return NLM_DEVICE_NFA_STATE_LIMIT;
    case 8: /* max matches per job limit reached */ return NLM_END_OF_JOB;
    case 9: return NLM_DEVICE_NFA_TIMEOUT;
    case 10: return NLM_DEVICE_DFA_TIMEOUT;
    case 13: return NLM_DEVICE_BUSY_POLL_TIMEOUT;
    case 15: return NLM_DEVICE_CONTEXT_FAIL;
    /* famos codes */
    case 0x15: return NLM_DEVICE_RESULT_CLAMP;
    case 0x16: return NLM_DEVICE_CONTEXT_CLAMP;
    case 0x17: return NLM_DEVICE_CONTEXT_RING_EMPTY;
    case 0x1C: return NLM_INVALID_GROUPID;
    /* case 0x1f: */
    default: return NLM_DEVICE_FAILURE;
    }
}

static inline int 
populate_results (nlm_device_t *device, struct hw_result_descriptor *rd, 
                  struct nlm_result *buf, int32_t *bindex)
{
  int32_t j;
  struct nlm_flow *flow;
  struct nlm_job *job;
  struct hw_pd_descriptor *pd;

    /* The RD is for us, copy the RD information into the user buf. */
  if (NLM_DEBUG)
    nlm_debug_dump_rd (rd);

  j = *bindex;
  api_assert (rd->cookie != 0);
  api_assert (rd->u4.hw.valid);
  /* figure out the flow and PD this RD belongs to */
  job = device->job_base - NLM_GET_JOB_INDEX (rd->cookie);
  api_assert (job->fd.cookie == rd->cookie);
  
  flow = job->flow;
  pd = job->pd;

  if (flow->is_stateless)
    api_assert (pd == NULL);
  else
    api_assert (pd != NULL);
  
  buf[j].cookie = job->user_cookie;

  if (rd->u1.hw.result_type == FMS_MATCH_RESULT)
    {
      if (rd->u4.hw.end_anchor)
        {
          /* Errate. The HW may push out end anchored results
             even though the flow may have errored out earlier
             on. Workaround, simply ignore the result */
          if (flow->errored)
            return 0;
          
          buf[j].status = NLM_END_ANCHORED;
        }
      else
        buf[j].status = NLM_OK;

      api_assert (flow->errored == 0);
      buf[j].u.match.flow_offset = rd->u1.hw.byte_offset_lo
        | (((uint64_t)rd->u4.hw.byte_offset_hi) << 24);
      /* byte_offset in RD is actually a flow offset in fms,
         to calculate the offset within packet we need to : */
      buf[j].u.match.byte_offset = buf[j].u.match.flow_offset - job->flow_offset;
      buf[j].u.match.rule_id = rd->u3.hw.rule_id;
      buf[j].u.match.group_id = rd->u4.hw.gid & 0x3ff;
      buf[j].u.match.database_id = rd->u4.hw.gid >> 10;
      buf[j].u.match.match_length = rd->u3.hw.match_length;
      /* invalidate rd to make sure we don't scan it again */
      rd->u4.valid_sw = RD_WAS_PROCESSED;
      *bindex = ++j;
      return 0;
    }
  else if (rd->u1.hw.result_type == FMS_FRD_RESULT
           || rd->u1.hw.result_type == FMS_PRD_RESULT)
    { 
      /* frd result == processing of this PD is complete */
      buf[j].u.stats.timestamp
        = ((((uint64_t)(rd->u4.hw_frd.timestamp_hi)) << 24)
           | rd->u1.hw_frd.timestamp_lo);
      buf[j].u.stats.total_state_cnt = 0;
      buf[j].u.stats.final_state_cnt = 0;
      buf[j].u.stats.peak_state_cnt = 0;
      if (rd->u1.hw_frd.error_code == 0)
        buf[j].status = NLM_END_OF_JOB;
      else
        {
          buf[j].status = convert_hw_error_code_to_status (rd->u1.hw_frd.error_code);
          /* Remember the flow has errored, as we may need to discard results later */
          flow->errored = rd->u1.hw_frd.error_code;
          if (NLM_DEBUG)
            nlm_debug_dump_regs (device->phy_device, "get_all_results ");
        }
      /* now we can free this PD */
      if (pd)
        {
          api_assert (pd->rsv01 == PD_VALID_AND_STARTED);
          if (pd->flow_term)
            { 
              /* destroy_flow job */
              api_assert (flow->type == NLM_FLOW_TYPE_INVALID);
              api_assert (rd->u1.hw.result_type == FMS_FRD_RESULT);
              /* free PDF and nlm_flow associated with this flow */
              memory_pool_free_flow (device, flow);
            }
          else if ((flow->type == NLM_FLOW_TYPE_STATEFUL
                    || (flow->type == NLM_FLOW_TYPE_INVALID && !flow->is_stateless))
                   && rd->u1.hw.result_type == FMS_FRD_RESULT)
            nlm_flow_notice_job_completion (device, flow, pd);
          /* free PD */
          sysmem_free_pd (device, flow, pd);
        }
      
      memory_pool_free_job (device, job);
      if (NLM_DEBUG)
        {
          if (!flow->is_stateless
              && rd->u1.hw.result_type == FMS_FRD_RESULT)
            {
              nlm_debug_dump_pdf (device->phy_device, flow->u.stateful.pdf_index);
            }
        }
      if (flow->is_stateless)
        {
          api_assert (rd->u1.hw.result_type == FMS_FRD_RESULT);
          flow->u.stateless.job_seq_id--; /* one job is done, decrement in-flight job count */
          if (flow->type == NLM_FLOW_TYPE_INVALID /* destroy_flow() was initiated */
              && flow->u.stateless.job_seq_id == 0) /* now we can free nlm_flow structure */
            memory_pool_free_flow (device, flow);
        }
      rd->u4.valid_sw = RD_WAS_PROCESSED;
      *bindex = ++j;
      return 0;
    }
  else if (rd->u1.hw.result_type == FMS_ERD_RESULT)
    {
      /* should probably assert here... */
      if (NLM_DEBUG)
        {
          DDPRINT ("ERD at block = %d vb_offset = %d array_index = %d\n",
                   rd->u3.hw_erd.blk_index, rd->u3.hw_erd.vb_oset, rd->u3.hw_erd.array_index);
        }
      rd->u4.valid_sw = RD_WAS_PROCESSED;
      return 0;
    }
  else
    api_assert (0 && "unsupported result type");

  return 0; /* We are good */
}

static int
consume_rd_for_thread (nlm_device_t *device, struct hw_result_descriptor *rd, 
                       struct nlm_result *buf, int32_t *bindex)
{
  int32_t thread_id;

  if (rd->u4.valid_sw == RD_WAS_PROCESSED)
    return 0; /* Already processed we are good */

  /* check if the thread ID matches us */
  api_assert (rd->cookie != 0);
  thread_id = NLM_GET_THREAD_ID (rd->cookie);
  if (thread_id != device->thread_id)
    return 1; /* Result is not for us */

  return populate_results (device, rd, buf, bindex);
}

static nlm_status
get_search_results_from_fifo (nlm_device_t *device, uint32_t n_buf_entries,
                              struct nlm_result *buf, uint32_t *n_results)
{
  int32_t j;
  nlm_result_fifo_t *fifo;
  struct hw_result_descriptor *rd;
  
  fifo = device->sw_result_fifo;
  j = 0;
  
  while ((rd = peek_result_fifo_output (fifo)) != 0)
    {
      api_assert (populate_results (device, rd, buf, &j) == 0);
      pop_result_fifo (fifo);
      if (j == n_buf_entries)
        break;
    }
  
  /* we copied 'j' results to user's 'buf' */
  *n_results = j;
  
  return NLM_OK;
}

/** Return all results found so far by \a device into \a buf buffer,
   but no more than \a n_buf_entries at a time.
  \param[in] device device handle
  \param[in] n_buf_entries requested number of entries in the result buffer
  \param[in] buf result buffer to store results
  \param[out] n_results actual number of results returned
  \return NLM_OK, if \a n_results were found and copied into \a buf,
                  \a n_results can be zero, which means that no new results
                  were found and \a buf is not changed */
nlm_status
NLM_PROTO (nlm_get_all_search_results) (nlm_device_t *device, uint32_t n_buf_entries,
                                        struct nlm_result *buf, uint32_t *n_results)
{
  int32_t no_update, j = 0;
  uint32_t output_wrt_ptr;
  uint32_t output_rd_ptr;
  nlm_phy_device_t *phy_device;
  
  if (device == NULL || n_buf_entries == 0 || buf == NULL || n_results == NULL)
    return NLM_INVALID_ARGUMENT;

  /* If we are using the FIFO thread, our results are in our software
     FIFO, get it from there. */
  if (device->sw_result_fifo)
    {
      nlm_status status = get_search_results_from_fifo (device, n_buf_entries, buf, n_results);
      if (device->pending_flow_list_head) /* there are flows with pending jobs */
        nlm_try_start_stateful_flows (device); /* try to start them */
      return status;
    } 

  /* We need to get our search results directly from the hardware,
     for this we perform the sollowing steps.
     1. Lock the HW fifo
     2. Scan the results and find the RD's that belong to us,
        and copy results into the user buff.
     3. If all RD in output fifo were successfully popped then
        update the HW fifo pointer, if not skip this step
        some other ring will come in and take care of it */
  phy_device = device->phy_device;
  NLM_SPIN_LOCK (&phy_device->hw_output_fifo_lock);

  output_rd_ptr = phy_device->output_rd_ptr;
  output_wrt_ptr = *phy_device->output_wrt_ptr;

  api_assert (output_rd_ptr < phy_device->config.output_fifo_size
              && output_wrt_ptr < phy_device->config.output_fifo_size);

  if (output_rd_ptr == output_wrt_ptr)
    {
      /* result fifo is empty */
      *n_results = 0;
      NLM_SPIN_UNLOCK (&phy_device->hw_output_fifo_lock);
      if (device->pending_flow_list_head) /* there are flows with pending jobs */
        nlm_try_start_stateful_flows (device); /* try to start them */
      return NLM_OK;
    }

  DPRINT ("output_rd_ptr %d, output_wrt_ptr %d\n", output_rd_ptr, output_wrt_ptr);

  no_update = 0;
  /* scan output fifo for results that correspond to the given 'job' */
  if (output_rd_ptr < output_wrt_ptr)
    {
      struct hw_result_descriptor *rd = phy_device->output_fifo_base + output_rd_ptr;
      for (; output_rd_ptr < output_wrt_ptr; rd++, output_rd_ptr++)
        {
          no_update |= consume_rd_for_thread (device, rd, buf, &j); 
          if (j == n_buf_entries)
            break;
        }
    }
  else
    {
      struct hw_result_descriptor *rd = phy_device->output_fifo_base + output_rd_ptr;
      for (; output_rd_ptr < phy_device->config.output_fifo_size; output_rd_ptr++, rd++)
        {
          no_update |= consume_rd_for_thread (device, rd, buf, &j);
          if (j == n_buf_entries)
            break;
        }
      
      if (j < n_buf_entries)
        {
          rd = phy_device->output_fifo_base;
          for (output_rd_ptr = 0; output_rd_ptr < output_wrt_ptr; output_rd_ptr++, rd++)
            {
              no_update |= consume_rd_for_thread (device, rd, buf, &j);
              if (j == n_buf_entries)
                break;
            }
        }
    }

  /* copied so many results into the user buf */
  *n_results = j;

  /* wrap rd_ptr */
  output_rd_ptr &= phy_device->config.output_fifo_size - 1;
  if (!no_update && output_rd_ptr != phy_device->output_rd_ptr)
    {
      phy_device->output_rd_ptr = output_rd_ptr;
      /* make sure to flush CPU caches into memory, though we not suppose to */
      MEMORY_BARRIER ();
      SET_HW_REGISTER (RDF_RD_PTR, output_rd_ptr);
    }
  
  NLM_SPIN_UNLOCK (&phy_device->hw_output_fifo_lock);

  if (device->pending_flow_list_head) /* there are flows with pending jobs */
    nlm_try_start_stateful_flows (device); /* try to start them */

  return NLM_OK;
}

/* Executed by the FIFO thread. Dispatches all accumulated
   jobs from the virtual ring FIFO's to the hardware FIFO */
nlm_status
NLM_PROTO(nlm_manage_input) (nlm_device_t *device)
{
  int32_t njobs, thr_with_no_jobs, i, max_threads;
  nlm_phy_device_t *phy_device;
  
  if (device == NULL)
    return NLM_INVALID_ARGUMENT;

  if (device->thread_id != NLM_MANAGER_THREAD_ID)
    return NLM_INVALID_THREAD_ID;
  
  /* The device handle is really a physical device
     handle */
  phy_device = device->phy_device;

  /* Should be called only from the FIFO ring, make sure we
     were configured for this */
  if (phy_device->config.using_manager_thread == 0)
    return NLM_INVALID_ARGUMENT;

#ifdef NLM_HW_MARS2
  if (phy_device->dma_in_progress)
    nlm_device_check_dma_completion_mars2 (phy_device);
#endif

  NLM_SPIN_LOCK (&phy_device->hw_input_fifo_lock);
  /* We will attempt to dispatch as many jobs as possible
     to the device. We do this in a round robin manner
     to avoide head of line blocking */
  max_threads = phy_device->config.max_threads;
  njobs = 0;
  thr_with_no_jobs = 0;

  while (thr_with_no_jobs != max_threads)
    {
      thr_with_no_jobs = 0;
      for (i = 0; i < max_threads; i++)
        {
          struct hw_fd_descriptor *fd;
          struct nlm_job_fifo *fifo = &phy_device->sw_job_fifo[i];
          
          if (job_fifo_empty (fifo))
            {
              thr_with_no_jobs++;
              continue;
            }
          
          fd = sysmem_alloc_fd (phy_device);
          if (fd == NULL)
            {
              /* HW Fifo full, just quit the whole loop */
              thr_with_no_jobs = max_threads;
              break;
            }
          
          *fd = *peek_job_fifo_output (fifo);
          pop_job_fifo (fifo);
          
          /* copy the FD from the job, once we run out of FD's let the
             hardware loose, and push the remaining jobs back into the
             FIFO */
          njobs++;
        }
    }
  
  /* Nothing to be done */
  if (njobs == 0)
    {
      NLM_SPIN_UNLOCK (&phy_device->hw_input_fifo_lock);
      return NLM_OK;
    }
  
  nlm_device_update_input_wrt_ptr (phy_device);
  
#ifdef PERF_TEST
  bypass_hw (phy_device);
#endif
  NLM_SPIN_UNLOCK (&phy_device->hw_input_fifo_lock);
  return NLM_OK;
}

/* Given a result descriptor, figure out which thread
   it belongs to and push the RD into the threads's SW
   FIFO. If successful return 0, else return non zero
   value on failure */
static inline int
consume_rd (nlm_phy_device_t *phy_device, struct hw_result_descriptor *rd)
{
  struct hw_result_descriptor *drd;
  uint32_t thread_id;
  nlm_result_fifo_t *fifo;
  
  if (rd->u4.valid_sw == RD_WAS_PROCESSED)
    /* nlm_get_search_results () already read this RD */
    return 0;
  
  api_assert (rd->cookie != 0);
  thread_id = NLM_GET_THREAD_ID (rd->cookie);
  api_assert (thread_id < phy_device->config.max_threads);
  fifo = &phy_device->sw_result_fifo[thread_id];

  if (fifo->skip)
    {
      /* Do not push results, as an earlier iteration may
         have failed to push results. All RD's must be
         delivered in order. */
      return 1;
    }
  
  drd = peek_result_fifo_input (fifo);
  if (drd == NULL)
    {
      /* The FIFO for this thread is full, set the skip
         bit on the fifo, so that we accidentially will
         not push any more results into this fifo, as we
         keep examining the remaining RD's. All RD's must
         be delivered in order */
      fifo->skip = 1;
      return 1;
    }
  
  *drd = *rd;
#if defined(_MIPS_ARCH_OCTEON)
  MEMORY_BARRIER();
#endif
  push_result_fifo (fifo);

  /* We consumed the result, mark it as done */
  rd->u4.valid_sw = RD_WAS_PROCESSED;
  return 0;
}

/** Return all results found so far by \a device into \a the
    thread specific buffer pools. */
nlm_status
NLM_PROTO(nlm_manage_output) (nlm_device_t *device)
{
  int no_update, i;
  uint32_t output_wrt_ptr;
  uint32_t output_rd_ptr;
  nlm_phy_device_t *phy_device;

  if (device == NULL)
    return NLM_INVALID_ARGUMENT;

  if (device->thread_id != NLM_MANAGER_THREAD_ID)
    return NLM_INVALID_THREAD_ID;

  /* The device handle is really a physical device
     handle */
  phy_device = device->phy_device;
  
#ifdef NLM_HW_MARS2
  if (phy_device->dma_in_progress)
    nlm_device_check_dma_completion_mars2 (phy_device);
#endif

  NLM_SPIN_LOCK (&phy_device->hw_output_fifo_lock);
  output_rd_ptr = phy_device->output_rd_ptr;
  output_wrt_ptr = *phy_device->output_wrt_ptr;

  api_assert (output_rd_ptr < phy_device->config.output_fifo_size
              && output_wrt_ptr < phy_device->config.output_fifo_size);

  if (output_rd_ptr == output_wrt_ptr)
    {
      NLM_SPIN_UNLOCK (&phy_device->hw_output_fifo_lock);
      /* result fifo is empty */
      return NLM_OK;
    }
  
  /* During the last pass over the RD's we might have skipped
     populating some of the threads RD fifo's as they were
     full. Reset this information */
  for (i = 0; i < phy_device->config.max_threads; i++)
    phy_device->sw_result_fifo[i].skip = 0;
  
  no_update = 0; /* Assume we can update the output FIFO pointer */
  if (output_rd_ptr < output_wrt_ptr)
    {
      struct hw_result_descriptor *rd = phy_device->output_fifo_base + output_rd_ptr;
      for (; output_rd_ptr < output_wrt_ptr; rd++, output_rd_ptr++)
        no_update |= consume_rd (phy_device, rd);      
    }
  else
    {
      struct hw_result_descriptor *rd = phy_device->output_fifo_base + output_rd_ptr;
      for (; output_rd_ptr < phy_device->config.output_fifo_size; output_rd_ptr++, rd++)
        no_update |= consume_rd (phy_device, rd);  
      
      rd = phy_device->output_fifo_base;
      for (output_rd_ptr = 0; output_rd_ptr < output_wrt_ptr; output_rd_ptr++, rd++)
        no_update |= consume_rd (phy_device, rd);  
    }

  output_rd_ptr &= phy_device->config.output_fifo_size - 1; /* wrap rd_ptr */
  if (!no_update && output_rd_ptr != phy_device->output_rd_ptr)
    {
      phy_device->output_rd_ptr = output_rd_ptr;
      /* make sure to flush CPU caches into memory, though we not suppose to */
      MEMORY_BARRIER ();
      SET_HW_REGISTER (RDF_RD_PTR, output_rd_ptr);
    }
  
  NLM_SPIN_UNLOCK (&phy_device->hw_output_fifo_lock);
  return NLM_OK;
}

