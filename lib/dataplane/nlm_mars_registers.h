/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef MARS_REGISTERS_H
#define MARS_REGISTERS_H
/*
  GLOBAL REGISTER MAP

  Table 1: Application Module Register Address Map
  Application Module             Address Range (bits [15:0])
  System Registers (sr)                      0x0FFC - 0x0000
  Data Fetch (df)                            0x1FFC - 0x1000
  Packet Descriptor (pdf)                    0x2FFC - 0x2000
  MARS Engine                                0x5FFC - 0x5000
  ROB Interface (rob)                        0x63FC - 0x6000
  Context Free Pool (cnxt)                   0x67FC - 0x6400
  External Rule Table Extension (ext_rule)   0x6FFC - 0x6800
*/

#define NLM_REGISTER_MAP_SIZE 0x10000

#define SYS_SCRATCH_PAD     0x0000  /**  RW     32'h1   Dummy Register for Debug    */
#define SYS_CONTROL         0x0004  /**  RW     32'h0   FPGA Control Register. Bit Definition */
                                    /**  - Bit[0] FPGA Enable. Enable FPGA for normal operation */
                                    /**  - Bit[1] FE0 Pause. Disable the Flow Engine0 in the FPGA. */
                                    /**  - Bit[2] FE1 Pause. Disable the Flow Engine1 in the FPGA. */
#define SYS_STATUS          0x0008  /**  RO 32'h0   Status Register. Bit Definition */
                                    /**  - Bit[0] = PDF_DMA_BUSY */
#define TIMEOUT_ASSERTED    0x000C  /**  RW  32'h0   asserted when AI detects a timeout condition */
#define SYS_RESET           0x0010  /**  RW     32'hffff_ffff   System to sub module reset register. Active Low; */
                                    /**  - Bit[0] = App Sys Module Reset.  */
                                    /**  - Bit[1] = DF Module Reset. */
                                    /**  - Bit[2] = Packet Queue Module Reset. */
                                    /**  - Bit[3] = FE0 Module Reset */
                                    /**  - Bit[4] = FE1 Module Reset */
                                    /**  - Bit[5] = Reserved */
                                    /**  - Bit[6] = Reserved */
                                    /**  - Bit[7] = SRAM I/F Module Reset */
                                    /**  - Bit[8] = ROB Module Reset */
                                    /**  - Bit[9] = Reserved */
                                    /**  - Bit[10] = FE1 Module Reset */
#define SYS_DEBUG_REG       0X0014  /**  RO      32'h1234_5678   Debug register with default to 0x12345678. Read only register. */
#define SYS_DATE_REG        0x0018  /**  RO      NA      Revision Register with Date of Altera FPGA build information. */
                                    /**  [31:16] Unused (zeros) */
                                    /**  [15:12] Month */
                                    /**  [11:4] Day */
                                    /**  [3:0] Year */
#define SYS_TIME_REG        0x001C  /**  RO      NA      (RO) Revision Register with time of Altera FPGA build information. */
                                    /**  [31:24] Unused (zeros) */
                                    /**  [23:16] Hour */
                                    /**  [15:8] Minutes */
                                    /**  [7:0] Seconds  */
#define DEVICE_ID_REG       0x0020  /**  RO      NA      Device ID register. */
#define CLOCK_COUNTER       0x005C  /**  RO      32'h0   This is the free running clock counter after the reset (hard or soft) is removed. At any hard or soft reset, it is cleared. */

/**  DATA FETCH (DF) BLOCK REGISTER DETAILS */
/**  The default value if the register is not defined in the DF table below for any DF register is 32'h0. */
#define FDF_BA_LSB          0x1000  /**  RW  32'h0   Flow Descriptor FIFO Base Address (LSB 32-bits) */
#define FDF_BA_MSB          0x1004  /**  RW  32'h0   Flow Descriptor FIFO Base Address (MSB 32-bits)  */
#define FDF_SIZE            0x1008  /**  RW  32'h0   Flow Descriptor FIFO Size[31:0] */
#define FDF_CONFIG          0x1010  /**  RW  32'h1   [0] FDF headers are big endian. Default is little endian. */
                                    /**  [1] - DISABLE_FDF_RDF_WRT.  When this bit is set to a 1, the writes to the FDF_RDF pointer address is disabled */
                                    /**  [31:1] reserved */
#define FDF_POLL_INTERVAL   0x1014  /**  RW  32'h20  FDF_POLL_INTERNAL[31:0] */
                                    /**  This register is used to set the amount of 'wait' time that must pass before the FDF DMA engine updates the external FDF_RD_PTR and RDF_WRT_PTR copies in system memory, but only if at least one of the external values are 'stale'. */
#define FD_STATUS           0x1018  /**  RO  32'h0   [0] Input DMA engines are idle. */
                                    /**  [31:1] unused */
#define FD_FIFO_RD_PTR      0x101C  /**  RO  32'h0   FDF Fifo Read (head) pointer value */
                                    /**  This value gets initialize to 0 on a reset.  Software has no ability to set it other than through reset. */
#define FD_FIFO_WRT_PTR     0x1020  /**  RW  32'h0   FDF Fifo Write (tail) pointer value */
#define FDF_FORCE_POLL      0x1024  /**  W   32'h0   [0] - Force Poll Cycle */
                                    /**  Writing this bit forces the FDF_POLL_INTERVAL wait timer to expire. */
#define FDF_DEBUG           0x1028  /**  RO  32'h10  [3:0] FDF_CTRL_SM (4'h0 is idle) */
                                    /**  [4] - FDF_TO_PDF_FIFO_EMPTY */
#define FDF_RDF_BASE_LOW    0x1030  /**  RW  32'h0   [31:0] - FDF_RDF_ADDR[31:0].  These bits are used for the FDF_RDF_ADDR[31:0]. */
#define FDF_RDF_BASE_HI     0x1034  /**  RW  32'h0   [31:0] - FDF_RDF_ADDR[63:32].  These bits are used for the FDF_RDF_ADDR[63:32]. */

/**  4.  PACKET QUEUE (PQ) BLOCK REGISTER DETAILS */
/**  The PQ block has 2 AI interface. One is for the PDF module and other for the PDQ module. In the PQ block all the registers are in the PDF module. The PDQ module does not have any target registers accessible from the HT interface. */
/**  The default value if the register is not defined in the PQ table below for any PQ register is 32'h0. */
#define PDF_POLL_INTERVAL   0x2004  /**  RW  32'h0   [11:0] 12 bits of PDF Polling interval. This will be used when polling for the state busy bit. */
#define PDF_CONFIG          0x2008  /**  RW  32'h0   [3:0] Reserved */
                                    /**  [4] Payload data Big Endian. Default is little endian. */
                                    /**  [5] PDF Big endian. Default is little endian. This bit is change the endianness of the PDF headers. */
                                    /**  [7:6] Reserved */
                                    /**  [15:8] Padding Character (default is 00) */
                                    /**   [31:16] Reserved */
#define PDF_CONFIG2         0x2018  /**  RW  32'h0   [0] - DISABLE_Q0 */
                                    /**  [1] - DISABLE_Q1 */
                                    /**  [2] - DISABLE_Q2 */
                                    /**  [3] - DISABLE_Q3 */
                                    /**  [31:4] - Reserved */
#define PDF_CONFIG3         0x201c  /**  RW  32'h0040_0010   [7:0] - DFA_ONLY_PREAMBLE_OFFSET[7:0] (minimum value is 0x10) */
                                    /**  [15:8] - reserved */
                                    /**  [23:16]  - DFA_NFA_PREAMBLE_OFFSET[7:0] (minimum value is 0x40) */
                                    /**  [31:24] - reserved */
#define PDF_CONFIG4         0x2024  /**  RW  32'h00 00_ffff */
                                    /**  [15:0] BUSY_BIT_POLL_TIMEOUT[15:0]. This */
                                    /**  value is used has the maximum number */
                                    /**  of times the busy bit can be polled before */
                                    /**  the flow is failed */
                                    /**  [31:16] - reserved */
#define FID_SID_MASK        0x2020  /**  RW  32'hffff_ffff   [31:0] - FID/SID Mask field.  If a bit is set to a 1, that bit position is defined to be an FID bit to be used by the hardware for load balancing and hardware busy bit interlocking. */
#define PDF_BASE_LOW        0x2030  /**  RW  32'h0000_0000   [31:28] - PDF_ADDR[31:28].  These bits are used for the PDF_ADDR[31:28]. */
                                    /**  [27:0] - reserved */
#define PDF_BASE_HI         0x2034  /**  RW  32'h0   [31:0] - PDF_ADDR[63:32].  These bits are used for the PDF_ADDR[63:32]. */
#define PQ_BASE_LOW_0       0x2040  /**  RW  32'h0000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h0.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_0        0x2044  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h0. */
#define PQ_BASE_LOW_1       0x2048  /**  RW  32'h1000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h1.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_1        0x204C  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h1. */
#define PQ_BASE_LOW_2       0x2050  /**  RW  32'h2000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h2.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_2        0x2054  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h2. */
#define PQ_BASE_LOW_3       0x2058  /**  RW  32'h3000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h3.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_3        0x205C  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h3. */
#define PQ_BASE_LOW_4       0x2060  /**  RW  32'h4000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h4.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_4        0x2064  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h4. */
#define PQ_BASE_LOW_5       0x2068  /**  RW  32'h5000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h5.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_5        0x206C  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h5. */
#define PQ_BASE_LOW_6       0x2070  /**  RW  32'h6000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h6.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_6        0x2074  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h6. */
#define PQ_BASE_LOW_7       0x2078  /**  RW  32'h7000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h7.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_7        0x207C  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h7. */
#define PQ_BASE_LOW_8       0x2080  /**  RW  32'h8000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h8.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_8        0x2084  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h8. */
#define PQ_BASE_LOW_9       0x2088  /**  RW  32'h9000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'h9.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_9        0x208C  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h9. */
#define PQ_BASE_LOW_A       0x2090  /**  RW  32'ha000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'ha.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_A        0x2094  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'ha. */
#define PQ_BASE_LOW_B       0x2098  /**  RW  32'hb000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'hb.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_B        0x209C  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hb. */
#define PQ_BASE_LOW_C       0x20A0  /**  RW  32'hb000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'hb.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_C        0x20A4  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hb. */
#define PQ_BASE_LOW_D       0x20A8  /**  RW  32'hd000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'hd.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_D        0x20AC  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hd. */
#define PQ_BASE_LOW_E       0x20B0  /**  RW  32'he000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'he.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_E        0x20B4  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'he. */
#define PQ_BASE_LOW_F       0x20B8  /**  RW  32'hf000_0000   [31:28] - PQ_ADDR[31:28].  These bits are used for the PQ_ADDR[31:28] when PQ_BASE[3:0] = 4'hf.  */
                                    /**  [27:0] - reserved */
#define PQ_BASE_HI_F        0x20BC  /**  RW  32'h0   [31:0] - PQ_ADDR[63:32].  These bits are used for the PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hf. */

/**  5.  RESULT OUTPUT BLOCK (ROB) REGISTER DETAILS */
/**  The default value if the register is not defined in the ROB table below for any ROB register is 32'h0000_0000. */
#define RDF_BA_LSB          0x6048  /**  RW  32'h0   Result Descriptor FIFO Base Address Lower 32 bits */
#define RDF_BA_MSB          0x604C  /**  RW  32'h0   Result Descriptor FIFO Base Address upper 32 bits.  */
#define RDF_WRT_PTR         0x6050  /**  RO  32'h0   Result Descriptor FIFO Write Pointer. This register holds the RDF_WRT_PTR. */
#define RDF_RD_PTR          0x6054  /**  RW  32'h0   Result Descriptor FIFO Read Pointer. This register hold the RDF Read Pointer Read and is updated by software when result buffers are returned to the ring. */
#define RDF_SIZE            0x6058  /**  RW  32'h0   Result Descriptor FIFO Size. This is the number of RDs that can fit in the RDF system memory. Each RD is 4 words (32-bits) in size. */
#define ROB_CONFIG          0x60AC  /**  RW  32'h0   [0] Endian-ness of the results. Default is 0 (little endian) 1 means big endian */
                                    /**  [1] - TIMESTAMP_ENABLE. This bit is used to enable the timestamp counter for FRD results. */
                                    /**  [9:8] RDF Max Burst Size. This value controls when the internal FIFO is burst out into external memory (00 - 16 Bytes, 01 - 32 Bytes, 10 - 64 Bytes, 11 - 128 Bytes) */
                                    /**  [15:10] - reserved */
                                    /**  [23:16] - FRD_STAT_SEL[7:0].  This value is used to select the statistics reported in the FRD result */
                                    /**  [31:24] Reserved */
#define RDF_FORCE_TIMER     0x60C0    /**  RW  32'h0   This Timer control when the internal RDF FIFO gets flushed out to system memory regardless of max burst size. */
#define RDF_TIMESTAMP       0x60C4    /** timestamp low 32-bit */
#define RDF_TIMESTAMP_HI    0x60C8    /** timestamp upper 8-bit */
#define RDF_TIMESTAMP_LOAD  0x60CC    /** R/W 32/h0 Writing to this register causes the RDF_TIMESTAMP_HOLD registers to get loaded with the current value of the TIMESTAMP counter */
#define RDF_TIMESTAMP_HOLD  0x60D0    /** RO  32'h0 [31:0] This holds a snapshot of the TIMERSTAMP[31:0] counter. */
#define RDF_TIMESTAMP_HOLD_HI 0x60D4  /** RO  32'h0 [7:0]  This holds a snapshot of the TIMESTAMP[39:32] counter. */
                                      /** [31:8] - reserved */

 /** 6.  CONTEXT FREE POOL RING REGISTERS */
#define CNXT_BASE_BASE_LOW  0x6400  /**  RW  32'h0   [31:0] - This register holds bits [31:0] of the CNXT_BASE_ADDR[63:0]. The CNXT_BASE_ADDR register is a 64 bit register that points to the base of the CNXT ring. */
#define CNXT_BASE_BASE_HI   0x6404  /**  RW  32'h0   [31:0] - This register holds bits [63:32] of the CNXT_BASE_ADDR[63:0]. The CNXT_BASE_ADDR register is a 64 bit register that points to the base of the CNXT ring. */
#define CNXT_BUFF_BASE_LOW  0x6408  /**  RW  32'h0   [31:0] - This register holds bits [31:0] of the offset that is added to all context values pulled from the CNXT ring  */
#define CNXT_BUFF_BASE_HI   0x640C  /**  RW  32'h0   [31:0] - This register holds bits [63:32] of the offset that is added to all context values pulled from the CNXT ring */
#define CNXT_WRT_PTR        0x6410  /**  RW  32'h0   [31:0] - This register contains the CNXT_WRT_PTR[31:0]. This pointer is owned by hardware.  The hardware pushs any returning buffer addresses to the location pointer to by the CNXT_WRT_PTR. */
#define CNXT_RD_PTR         0x6414  /**  RW  32'h0   [31:0] - This register contains the CNXT_RD_PTR[31:0]. This pointer is owned by hardware.  The hardware pulls available buffer addresses to the location pointer to by the CNXT_RD_PTR */
#define CNXT_SIZE           0x6420  /**  RW  32'h0   [31:0] - This register contains the CNXT_SIZE[31:0] . The CNXT_SIZE register defines the size of the context ring.  0x0001 represents 1 buffer. */
#define CNXT_CTRL           0x6424  /**  RW  32'hffff_10_00  [0] - ENABLE.  When this bit is set to a 1, hardware starts processing the context ring. */
                                    /**  [7:1] - reserved */
                                    /**  [15:8] - MAX_WRT_BUFFER_TIMER.  This value is used to determine how long the internal wrt buffer FIFO waits to gather up returning buffers to form larger write burst.  The counter is 16 system clock ticks with 0=16 clocks, and xFF=256*16 clocks. */
                                    /**  [31:16] - CNXT_MAX_STATE_NUM[15:0]. This register can be used by software to limit the maximum number of MARS states that can be stored for any flow during a context swap.  It should be noted that there are internal limits to the number of states that the MARS engine can store in hardware. */
#define CNXT_FIFO_CNT       0x6430  /**  RW  NA      [4:0] - RD_FIFO_LEVEL.  This value is the number of internal buffer addresses held in the internal RD_BUFFER fifo. */
                                    /**  [15:5] - reserved */
                                    /**  [20:16] - WRT_FIFO_LEVEL.  This value is the number of internal buffer addresses held in the internal WRT_BUFFER fifo. */
                                    /**  [31:21] - reserved */
#define CNXT_WATERMARK      0x6448  /** This register is used to mark the lowest point the context free pool reached during operations */

/**  7.  EXTERNAL RULE TABLE EXTENSION */
#define EXT_A_BASE_LOW      0x6800  /**  RW  32'h0   [31:0] - This register holds bits [31:0] of the EXT_A_BASE_ADDR[63:0]. The EXT_A_BASE_ADDR register is a 64 bit register that points to the base of the Extension Rules Table for Database A. */
#define EXT_A_BASE_HI       0x6804  /**  RW  32'h0   [31:0] - This register holds bits [63:32] of the EXT_A_BASE_ADDR[63:0]. The EXT_A_BASE_ADDR register is a 64 bit register that points to the base of the Extension Rules Table for Database A. */
#define EXT_A_CONFIG        0x6808  /**  RW  32'h0   [20:0] EXT_A_TABLE_MAX_SIZE[20:0].  This value is used to define the size (in 96 bit rules) of the External Rules Table for Database A. */
                                    /**  [23:21] - reserved */
                                    /**  [24] - EXT_A_TABLE_ENABLE */
                                    /**  [31:25] - reserved */
#define EXT_B_BASE_LOW      0x6810  /**  RW  32'h0   [31:0] - This register holds bits [31:0] of the EXT_B_BASE_ADDR[63:0]. The EXT_B_BASE_ADDR register is a 64 bit register that points to the base of the Extension Rules Table for Database B. */
#define EXT_B_BASE_HI       0x6814  /**  RW  32'h0   [31:0] - This register holds bits [63:32] of the EXT_B_BASE_ADDR[63:0]. The EXT_B_BASE_ADDR register is a 64 bit register that points to the base of the Extension Rules Table for Database B. */
#define EXT_B_CONFIG        0x6818  /**  RW  32'h0   [20:0] EXT_B_TABLE_MAX_SIZE[20:0].  This value is used to define the size (in 96 bit rules) of the External Rules Table for Database B. */
                                    /**  [23:21] - reserved */
                                    /**  [24] - EXT_B_TABLE_ENABLE */
                                    /**  [31:25] - reserved */

/**  8.  MARS ENGINE (ENG) REGISTER DETAILS */
#define ENTRY0_RAM_ADDR     0x5000  /**  RW  32'h0   [16:0] - PORT0 ENTRY_RAM Index[16:0] */
#define ENTRY0_RAM_DATA_A   0x5004  /**  RW  32'h0   [31:0] - PORT0 ENTRY_RAM Data[31:0] */
#define ENTRY0_RAM_DATA_B   0x5008  /**  RW  32'h0   [31:0] - PORT0 ENTRY_RAM Data[63:32] */
#define ENTRY0_RAM_CTRL     0x500C  /**  RW  32'h0   [0] - PORT0_WRT (hardware resets to 0) */
                                    /**  [1] - PORT0_RD (hardware resets to 0) */
#define ENTRY1_RAM_ADDR     0x5010  /**  RW  32'h0   [16:0] - PORT1 ENTRY_RAM Index[16:0] */
#define ENTRY1_RAM_DATA_A   0x5014  /**  RW  32'h0   [31:0] - PORT1 ENTRY_RAM Data[31:0] */
#define ENTRY1_RAM_DATA_B   0x5018  /**  RW  32'h0   [31:0] - PORT1 ENTRY_RAM Data[63:32] */
#define ENTRY1_RAM_CTRL     0x501c  /**  RW  32'h0   [0] - PORT1_WRT (hardware resets to 0) */
                                    /**  [1] - PORT1_RD (hardware resets to 0) */
#define ENTRY0_RAM_DATA_C   0x5048  /**  RW  32'h0   [31:0] - PORT0 ENTRY_RAM Data[95:64] */
#define ENTRY1_RAM_DATA_C   0x504c  /**  RW  32'h0   [31:0] - PORT1 ENTRY_RAM Data[95:64] */

#define ENTRY2_RAM_ADDR     0x5180  /**  RW [16:0] - PORT ENTRY_RAM Index[16:0] */
#define ENTRY2_RAM_DATA_A   0x5184  /**  RW [31:0] - PORT ENTRY_RAM Data[31:0] */
#define ENTRY2_RAM_DATA_B   0x5188  /**  RW [31:0] - PORT ENTRY_RAM Data[63:32] */
#define ENTRY2_RAM_DATA_C   0x518C  /**  RW [31:0] - PORT ENTRY_RAM Data[95:64] */
#define ENTRY2_RAM_CTRL     0x5190  /**  RW [0] - PORT0_WRT (hardware resets to 0) */

#define ENTRY3_RAM_ADDR     0x5194
#define ENTRY3_RAM_DATA_A   0x5198
#define ENTRY3_RAM_DATA_B   0x519C
#define ENTRY3_RAM_DATA_C   0x51A0
#define ENTRY3_RAM_CTRL     0x51A4

#define ENTRY4_RAM_ADDR     0x51A8
#define ENTRY4_RAM_DATA_A   0x51AC
#define ENTRY4_RAM_DATA_B   0x51B0
#define ENTRY4_RAM_DATA_C   0x51B4
#define ENTRY4_RAM_CTRL     0x51B8

#define ENTRY5_RAM_ADDR     0x51BC
#define ENTRY5_RAM_DATA_A   0x51C0
#define ENTRY5_RAM_DATA_B   0x51C4
#define ENTRY5_RAM_DATA_C   0x51C8
#define ENTRY5_RAM_CTRL     0x51CC

#define ENTRY6_RAM_ADDR     0x51D0
#define ENTRY6_RAM_DATA_A   0x51D4
#define ENTRY6_RAM_DATA_B   0x51D8
#define ENTRY6_RAM_DATA_C   0x51DC
#define ENTRY6_RAM_CTRL     0x51E0

#define ENTRY7_RAM_ADDR     0x51E4
#define ENTRY7_RAM_DATA_A   0x51E8
#define ENTRY7_RAM_DATA_B   0x51EC
#define ENTRY7_RAM_DATA_C   0x51F0
#define ENTRY7_RAM_CTRL     0x51F4

#define ENTRY_RAM_CONFIG    0x5060  /**  RW  32'h10  [4:0] - A_SIZE[4:0].  These bits are used map internal memories to the A Database (0 - No Memories, 1 - 1 Entry RAM,�, 16-All 16 Entry RAMS).  */
                                    /**  [7:5] - reserved */
                                    /**  [12:8] - reserved */
                                    /**  NOTE - the sum of A_SIZE and B_SIZE should be 16 or less. */
                                    /**  [15:13] - reserved */
                                    /**  [16] - PARITY_CHK_ENABLE */
                                    /**  [31:17] - reserved */
#define ENTRY_RAM_CONFIG2   0x50C0  /**  RW  32'h10  [4:0] - B_SIZE[4:0].  These bits are used map internal memories to the A Database (0 - No Memories, 1 - 1 Entry RAM,�, 16-All 16 Entry RAMS).  */
                                    /**  [31:5] - reserved */
                                    /**  NOTE - the sum of A_SIZE and B_SIZE should be 16 or less. */
#define ENTRY_RAM_STATUS    0x5064  /**  R/RUM       32'h0   [7:0] - BANK_A_OVERFLOW[7:0] */
                                    /**  [15:0] - BANK_B_OVERFLOW[7:0]  */
                                    /**  These bits are used to flag that the engine attempted to access a 'rule' that was beyond the end of the allocated database. */
                                    /**  Note, these bits are 'Reset Under Mask'.  A 1' is written to the bit position to reset */

#define ENTRY_RAM_ERR       0x5068  /** This register holds the first error detected during an access to the */
                                    /** Entry memory. The value is held if any of bit [0] is set in the */
                                    /** ENTRY_RAM_STATUS register. */
#define BITMAP_RAM_DATA     0x5070  /**  RW  32'h0   [7:0] - BITMAP_RAM Data[7:0] */
#define BITMAP_RAM_INDEX    0x5074  /**  RW  32'h0   [12:0] - ENTRY_RAM Index[12:0] */
#define BITMAP_RAM_CTRL     0x5078  /**  RW  32'h0   [0] - BM_WRT (hardware resets to 0) */
                                    /**  [1] - BM_RD (hardware resets to 0) */
#define BITMAP_RAM_CTRL2    0x507C
#define MLXE0_BASE_ADDR     0x5080  /**  RW  32'h0   [19:0] - MLXE0_BASE_ADDR[19:0] */
#define MLXE1_BASE_ADDR     0x5084  /**  RW  32'h0   [19:0] - MLXE1_BASE_ADDR[19:0] */
#define MLXE2_BASE_ADDR     0x50D0
#define MLXE3_BASE_ADDR     0x50D4
#define MLXE4_BASE_ADDR     0x50D8
#define MLXE5_BASE_ADDR     0x50DC
#define MLXE6_BASE_ADDR     0x50E0
#define MLXE7_BASE_ADDR     0x50E4

#define ENGINE_CONFIG       0x5090  /**  RW  32'hffff_0080   [7:0] - MAX_STATE_CNT[7:0].  This register can be used to lower the maximum number states allowed per byte.  The maximum value allowed is 128. */
                                    /**  [15:8] - reserved */
                                    /**  [31:16] - MAX_RESULTS_CNT[15:0].  This register can be used to limit the maximum number of states per flow */
#define DFA_NFA_TIMEOUT     0x50c4  /** DFA_NFA_TIMEOUT is used to terminate any byte that is stuck in a rule loop */

#define MEM3_CONFIG         0x50FC
#define TLB_ENTRY_BLOCK0    0x5100
#define RULE_DMA_CONFIG_START   0x6880
#define RULE_DMA_CONFIG_SIZE    0x6884 /** number of entries to be loaded */
#define RULE_DMA_CONFIG_STATUS  0x6888

/**  Macro to read/write a register. The BASE must be defined before using this macro. The addr has 32-bit increment   */
#if defined (__linux__) || defined (__CYGWIN__) || defined (_WIN32)
#define HW_REGISTER(REG_NAME)    \
    SWAP32(((volatile uint32_t *)(HW_REGISTER_MAP_BASE))[((unsigned short)(REG_NAME))>>2])

#define SET_HW_REGISTER(REG_NAME, VAL) \
    (((volatile uint32_t *)(HW_REGISTER_MAP_BASE))[((unsigned short)(REG_NAME))>>2]) = SWAP32(VAL)
#elif defined (__OCTEON__)
#include "octeon/octeon_pcmap_regs.h"
#define HW_REGISTER(REG_NAME) SWAP32(oct_read32(HW_REGISTER_MAP_BASE + REG_NAME))
#define SET_HW_REGISTER(REG_NAME, VAL) oct_write32(HW_REGISTER_MAP_BASE + REG_NAME, SWAP32(VAL))
#else
#error "OS is not supported"
#endif

#define FDF_POLL_INTERVAL_DEFAULT_VALUE 0x3f
#define PDF_POLL_INTERVAL_DEFAULT_VALUE 0x3f
#define PDF_POLL_INTERVAL_MAX_VALUE 0xfff
#define PDF_BUSY_BIT_MAX_POLL_VALUE 0xffff
#define RDF_FORCE_TIMER_DEFAULT_VALUE 0x32
#define MAX_MATCHES_PER_JOB_DEFAULT_VALUE 0xffff
#define MAX_STATES_PER_BYTE_DEFAULT_VALUE 0x7d

typedef union entry_ram_config
{
  uint32_t reg;
#ifdef __mips
  struct
  {
    uint32_t rsv2:15;
    uint32_t parchk:1;
    uint32_t rsv1:11;
    uint32_t db_size:5;
  } fields;
#else
  struct
  {
    uint32_t db_size:5;
    uint32_t rsv1:11;
    uint32_t parchk:1;
    uint32_t rsv2:15;
  } fields;
#endif
} entry_ram_config;

typedef union ext_area_config
{
  uint32_t reg;
#ifdef __mips
  struct
  {
    uint32_t rsv2:7;
    uint32_t enable:1;
    uint32_t rsv1:3;
    uint32_t table_max_size:21;
  } fields;
#else
  struct
  {
    uint32_t table_max_size:21;
    uint32_t rsv1:3;
    uint32_t enable:1;
    uint32_t rsv2:7;
  } fields;
#endif
} ext_area_config;

#endif /* MARS_REGISTERS_H */
