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
#ifndef FMS_REGISTERS_H
#define FMS_REGISTERS_H

#define SYS_SCRATCH_PAD 0x0000  
/**< R/W 32'h1 Dummy Register for Debug  */
#define SYS_CONTROL 0x0004  
/**< R/W 32'h0 "System Control Register
[0] DMA Enable. Enable DMA Engines for normal operation
[31:1] - Reserved"  */
#define SYS_STATUS  0x0008  
/**< RO  32'h20  "[0] - PDF_DMA_BUSY
[1] - FD_DMA_BUSY
[2] - ROB/CONTEXT DMA_BUSY
[4:3] - reserved
[5] - DMA_IDLE
[31:6] - reserved"  */
#define TIMEOUT_ASSERTED  0x000c  
/**< RW1C  32'h0 "[0] - asserted when AI detects a timeout
condition, cleared when SW write one to this register.
[31:1] - reserved
This signal automatically disables the dma enable signal send to DMA block but does not clear the
control[0] bit, enabling software to detect this specific timeout error." */
#define SYS_RESET 0x0010
/**< R/W 32'hffff_ffff "System to sub module reset register. Active Low;
- Bit[0] = App Sys. Module Reset.
- Bit[1] = SW Reset
- Bit[31:2] = Reserved
Even though the document shows different register bits for different modules, the SW during soft
reset can only set all to 0 or all to 1. No partial resets of the blocks are allowed in MARS
design."  */
#define SYS_DEBUG_REG 0X0014  
/**< RO  32'h1234_5678 Debug register with default to 0x12345678. Read
only register.  */
#define SYS_DATE_REG  0x0018  
/**< RO  0x0000_2058 "Revision Register with Date of build information.
[31:16] Unused (zeros)
[15:12] Month (Set to Feb)
[11:4] Day (Set to 5th)
[3:0] Year (Set to 2008)" */
#define SYS_TIME_REG  0x001C  
/**< RO  0x000A_0B0C "(RO) Revision Register with time of build
information.
[31:24] Unused (zeros)
[23:16] Hour (Set to 10 am)
[15:8] Minutes (Set to 11 minutes)
[7:0] Seconds  (Set to 12 seconds)" */
#define DEVICE_ID_REG 0x0020  
/**< R0  See descr.  "TP[9:8] =
2'b11 - 0x8025
2'b10 - 0x8055
2'b01 - 0x0025
2'b00 - 0x0055" */
#define NORESET_SCRATCH 0x0028  
/**< R/W NA  This register is available for software use */
#define CLOCK_COUNTER 0x005C  
/**< RO  32'h0 This is the free running clock counter after the reset
(hard or soft) is removed. At any hard or soft reset, it is cleared.  */
#define SHELL_CONTROL  0x0060 /**< 0 - disable shell, 1 - pcie shell, 2 - xaui1, 4 - xaui2, 8 - ddr */
#define FPGA_CLOCK_SELECT 0x0064  
/**< RO  32'h0 "[0]: SYSCLK = 32 MHz on FPGA.
[31:1] : reserved
This logic selects the FPGA internal SYSCLK speed between 64 MHz (0b) and 32 MHz (1b).
This register is reserved in ASIC version.
At any hard or soft reset, it is cleared."  */
#define PCI_EXPRESS_SETTINGS_REGISTER 0x0068  
/**< RO  32'h0 "This register reports the PCIe test
mode and settings programmed either by I2C or test pins input.
[0]: FAST_INIT mode for simulation
[1]: MASTER LOOPBACK - this bit direct the PCIe controller to master loopback and generate the
compliance pattern (with SKP OS inserted)
[2]: DISABLE SCRAMBLING - When set, this bit requires to disable scrambling on PCIe link
[3]: FPGA MODE - This bit enable to assume maximum lane detected by receiver detect operation as
well as limit compliance state entry.
[4]: REMOTE BOOT - This bit enable to bypass BAR filtering of received request.
[5]: DISABLE COMPLIANCE STATE - This bit disable LTSSM entry in compliance state
[6]: FORCE COMPLIANCE STATE - This bit force LTSSM entry in compliance state
[7]: DISABLE POWER MANAGEMENT - This bit completely disable all power management negotiation
[8]: FORCEX2 - This bit forces PCIe x2 mode initialisation
[9]: FORCEX1 - This bit forces PCIe x1 mode initialisation
[10]: BYPASS RX DETECT - This bit report maximum lane count detected during LTSSM receiver detect
operation
[11]: SLOT CLOCK CONFIGURATION - This bit reports whether the PCIe slot clock has been used or not.
[12]: IMPLEMENT ASPM L1 - This bit reports whether ASPM L1 is implemented or not
[13]: IMPLEMENT ECRC GENERATION - This bit reports whether ECRC generation is implemented or not
[14]: IMPLEMENT ECRC CHECK - This bit reports whether ECRC checking is implemented or not
[15]: IMPLEMENT AER - This bit reports whether Advanced Error Reporting is implemented or not.
[31:16] :  reserved"  */
#define PINS_STATUS_REGISTER  0x006C  
/**< RO  32'h0 "This registers reports the value of the test
pins for debug purpose
[7:0]: PE_CONFIG - PCIe configuration
[17:8]: TP - tests pins
[18]: VCONFIG - Voltage input
[20:19]: MODE1/0 - test pins mode (00b) or I2C slave
[31:21] :  reserved"  */
#define PCI_EXPRESS_STATUS_REGISTER 0x0070  
/**< RO  32'h0 "This registers reports the value of the
PCIe status register which can also be read back by I2C (debug purpose)
[4:0]: LTSSM state
[6:5]: Rx L0s state
[8:7]: Tx L0s state
[9]: Link Up state
[11:10]: DLCMSM state
[12]: Flag FI1
[15:13]: RTRYSM state
[16] :  VC status
[31:17] :  Detected PCIe error" */
#define PCI_EXPRESS_PHY_SETTINGS_REGISTER_0 0x0074  
/**< RO  32'h0 This registers reports the value
of the PHY settings register PHYSETTINGS_REG[31:0] which can be programmed by I2C */
#define PCIE_EXPRESS_PHY_SETTINGS_REGISTER_1  0x0078  
/**< RO  32'h0 "This registers reports the
value of the PHY settings register bit [63:32] which can be programmed by I2C.
[15:0] : PHYSETTINGS_REG[63:32]"  */
#define PCIE_EXPRESS_PHY_SETTINGS_REGISTER_2  0x007C  
/**< RO  32'h0 "This registers reports the
value of the PHY settings register bit [87:64] which can be programmed by I2C.
[23:0] : PHYSETTINGS_REG[87:64]
[31:24] :  reserved"  */
#define INITIAL_POSTED_CREDIT_REGISTER  0x0080  
/**< RO  32'h0 "This register reports the initial
credit advertised by opposite component. This value can be helpful to understand performance number
or loss of certain type of credit.
[7:0] : Posted Header credit advertised by opposite component
[15:8] :  reserved
[27:16] :  Posted Data credit advertised by opposite component
[31:28] : reserved" */
#define INITIAL_NON_POSTED_CREDIT_REGISTER  0x0084  
/**< RO  32'h0 "This register reports the initial
credit advertised by opposite component. This value can be helpful to understand performance number
or loss of certain type of credit.
[7:0] : Non-Posted Header credit advertised by opposite component
[15:8] :  reserved
[27:16] :  Non-Posted Data credit advertised by opposite component
[31:28] : reserved" */
#define INITIAL_COMPLETION_CREDIT_REGISTER  0x0088  
/**< RO  32'h0 "This register reports the initial
credit advertised by opposite component. This value can be helpful to understand performance number
or loss of certain type of credit.
[7:0] : Completion Header credit advertised by opposite component
[15:8] :  reserved
[27:16] :  Completion Data credit advertised by opposite component
[31:28] : reserved" */
#define CURRENT_POSTED_CREDIT_REGISTER  0x008C  
/**< RO  32'h0 "This register reports the current
credit available for posted TLP transmission. This value can be helpful to understand performance
number or loss of certain type of credit.
[7:0] : Posted Header credit advertised by opposite component
[15:8] :  reserved
[27:16] :  Posted Data credit advertised by opposite component
[31:28] : reserved" */
#define CURRENT_NON_POSTED_CREDIT_REGISTER  0x0090  
/**< RO  32'h0 "This register reports the current
credit available for non-posted TLP transmission. This value can be helpful to understand
performance number or loss of certain type of credit.
[7:0] : Non-Posted Header credit advertised by opposite component
[15:8] :  reserved
[27:16] :  Non-Posted Data credit advertised by opposite component
[31:28] : reserved" */
#define CURRENT_COMPLETION_CREDIT_REGISTER  0x0094  
/**< RO  32'h0 "This register reports the current
credit available for posted TLP transmission. This value can be helpful to understand performance
number or loss of certain type of credit.
[7:0] : Completion Header credit advertised by opposite component
[15:8] :  reserved
[27:16] :  Completion Data credit advertised by opposite component
[31:28] : reserved" */
#define LATENCY_REGISTER  0x0098  
/**< RO  32'h0 "This register reports the read latency observed for
first read request generated by the PCIe DMA engine. This value can be helpful to understand the
system latency and observed performance.
[15:0] : LATENCY (in SYSCLK cycles)
[29:16] :  reserved
[30] :  LATENCY RUNNING
[31] : LATENCY DONE"  */
#define FPGA_CAPABILITIES0  0x00A0
/**< RW  32'h0 This register defines the 32 LSB bits of
the local address associated to BAR2/3 of PCIe configuration space  */
#define FPGA_CAPABILITIES1  0x00A4
/**< RW  32'h0 This register defines the 32 MSB bits of
the local address associated to BAR2/3 of PCIe configuration space  */
#define FDF0_BA_LSB 0x1000  
/**< R/W 32'h0 Flow Descriptor FIFO Base Address (LSB 32-bits) */
#define FDF0_BA_MSB 0x1004  
/**< R/W 32'h0 Flow Descriptor FIFO Base Address (MSB 32-bits)   */
#define FDF0_SIZE 0x1008  
/**< R/W 32'h32  Flow Descriptor FIFO Size[31:0] */
#define FDF0_CONFIG 0x1010  
/**< R/W 32'h1 "[0] FDF headers are big endian.
[1] - DISABLE_FDF_RDF_WRT.  When this bit is set to a 1, the writes to the FDF_RDF pointer address
is disabled
[2] - FD_INTERNAL_MEM_MAP
[15:3] reserved
[19:16] - FD_RING_ID[3:0]
[23:20] - PDF/PQ_ID[3:0]
[27:24] - CNXT_RING_ID[3:0]
[31:28] - RSLT_RING_ID[3:0]"  */
#define FDF0_POLL_INTERVAL  0x1014  
/**< R/W 32'h20  "FDF_POLL_INTERNAL[31:0]
This register is used to set the amount of -wait- time that must pass before the FDF DMA engine
updates the external FDF_RD_PTR and RDF_WRT_PTR copies in system memory, but only if at least one of
the external values are -stale-." */
#define FDF0_FIFO_RDPTR 0x101C  
/**< RO  32'h0 "FDF Fifo Read (head) pointer value
This value gets initialize to 0 on a reset.  Software has no ability to set it other than through
reset." */
#define FDF0_FIFO_WRPTR 0x1020  
/**< RW  32'h0 FDF Fifo Write (tail) pointer value */
#define FDF0_FORCE_POLL 0x1024  
/**< W 32'h0 "[0] - Force Poll Cycle
Writing this bit forces the FDF_POLL_INTERVAL wait timer to expire."  */
#define FDF0_RDF_BASE_LOW 0x1030  
/**< R/W 32'h0 [31:0] - FDF_RDF_ADDR[31:0].  These bits are used
for the FDF_RDF_ADDR[31:0]. */
#define FDF0_RDF_BASE_HI  0x1034  
/**< R/W 32'h0 [31:0] - FDF_RDF_ADDR[63:32].  These bits are used
for the FDF_RDF_ADDR[63:32].  */
#define FDF1_BA_LSB 0x1040  
/**< R/W 32'h0 Register for FDF Ring 1 */
#define FDF1_BA_MSB 0x1044  
/**< R/W 32'h0 Register for FDF Ring 1 */
#define FDF1_SIZE 0x1048  
/**< R/W 32'h32  Register for FDF Ring 1 */
#define FDF1_CONFIG 0x1050  
/**< R/W 32'h1 Register for FDF Ring 1 */
#define FDF1_POLL_INTERVAL  0x1054  
/**< R/W 32'h20  Register for FDF Ring 1 */
#define FDF1_FIFO_RDPTR 0x105C  
/**< RO  32'h0 Register for FDF Ring 1 */
#define FDF1_FIFO_WRPTR 0x1060  
/**< RW  32'h0 Register for FDF Ring 1 */
#define FDF1_FORCE_POLL 0x1064  
/**< W 32'h0 Register for FDF Ring 1 */
#define FDF1_RDF_BASE_LOW 0x1070  
/**< R/W 32'h0 Register for FDF Ring 1 */
#define FDF1_RDF_BASE_HI  0x1074  
/**< R/W 32'h0 Register for FDF Ring 1 */
#define FDF2_BA_LSB 0x1080  
/**< R/W 32'h0 Register for FDF Ring 2 */
#define FDF2_BA_MSB 0x1084  
/**< R/W 32'h0 Register for FDF Ring 2 */
#define FDF2_SIZE 0x1088  
/**< R/W 32'h32  Register for FDF Ring 2 */
#define FDF2_CONFIG 0x1090  
/**< R/W 32'h1 Register for FDF Ring 2 */
#define FDF2_POLL_INTERVAL  0x1094  
/**< R/W 32'h20  Register for FDF Ring 2 */
#define FDF2_FIFO_RDPTR 0x109C  
/**< RO  32'h0 Register for FDF Ring 2 */
#define FDF2_FIFO_WRPTR 0x10A0  
/**< RW  32'h0 Register for FDF Ring 2 */
#define FDF2_FORCE_POLL 0x10A4  
/**< W 32'h0 Register for FDF Ring 2 */
#define FDF2_RDF_BASE_LOW 0x10B0  
/**< R/W 32'h0 Register for FDF Ring 2 */
#define FDF2_RDF_BASE_HI  0x10B4  
/**< R/W 32'h0 Register for FDF Ring 2 */
#define FDF3_BA_LSB 0x10C0  
/**< R/W 32'h0 Register for FDF Ring 3 */
#define FDF3_BA_MSB 0x10C4  
/**< R/W 32'h0 Register for FDF Ring 3 */
#define FDF3_SIZE 0x10C8  
/**< R/W 32'h32  Register for FDF Ring 3 */
#define FDF3_CONFIG 0x10D0  
/**< R/W 32'h1 Register for FDF Ring 3 */
#define FDF3_POLL_INTERVAL  0x10D4  
/**< R/W 32'h20  Register for FDF Ring 3 */
#define FDF3_FIFO_RDPTR 0x10DC  
/**< RO  32'h0 Register for FDF Ring 3 */
#define FDF3_FIFO_WRPTR 0x10E0  
/**< RW  32'h0 Register for FDF Ring 3 */
#define FDF3_FORCE_POLL 0x10E4  
/**< W 32'h0 Register for FDF Ring 3 */
#define FDF3_RDF_BASE_LOW 0x10F0  
/**< R/W 32'h0 Register for FDF Ring 3 */
#define FDF3_RDF_BASE_HI  0x10F4  
/**< R/W 32'h0 Register for FDF Ring 3 */
#define FDF4_BA_LSB 0x1100  
/**< R/W 32'h0 Register for FDF Ring 4 */
#define FDF4_BA_MSB 0x1104  
/**< R/W 32'h0 Register for FDF Ring 4 */
#define FDF4_SIZE 0x1108  
/**< R/W 32'h32  Register for FDF Ring 4 */
#define FDF4_CONFIG 0x1110  
/**< R/W 32'h1 Register for FDF Ring 4 */
#define FDF4_POLL_INTERVAL  0x1114  
/**< R/W 32'h20  Register for FDF Ring 4 */
#define FDF4_FIFO_RDPTR 0x111C  
/**< RO  32'h0 Register for FDF Ring 4 */
#define FDF4_FIFO_WRPTR 0x1120  
/**< RW  32'h0 Register for FDF Ring 4 */
#define FDF4_FORCE_POLL 0x1124  
/**< W 32'h0 Register for FDF Ring 4 */
#define FDF4_RDF_BASE_LOW 0x1130  
/**< R/W 32'h0 Register for FDF Ring 4 */
#define FDF4_RDF_BASE_HI  0x1134  
/**< R/W 32'h0 Register for FDF Ring 4 */
#define FDF5_BA_LSB 0x1140  
/**< R/W 32'h0 Register for FDF Ring 5 */
#define FDF5_BA_MSB 0x1144  
/**< R/W 32'h0 Register for FDF Ring 5 */
#define FDF5_SIZE 0x1148  
/**< R/W 32'h32  Register for FDF Ring 5 */
#define FDF5_CONFIG 0x1150  
/**< R/W 32'h1 Register for FDF Ring 5 */
#define FDF5_POLL_INTERVAL  0x1154  
/**< R/W 32'h20  Register for FDF Ring 5 */
#define FDF5_FIFO_RDPTR 0x115C  
/**< RO  32'h0 Register for FDF Ring 5 */
#define FDF5_FIFO_WRPTR 0x1160  
/**< RW  32'h0 Register for FDF Ring 5 */
#define FDF5_FORCE_POLL 0x1164  
/**< W 32'h0 Register for FDF Ring 5 */
#define FDF5_RDF_BASE_LOW 0x1170  
/**< R/W 32'h0 Register for FDF Ring 5 */
#define FDF5_RDF_BASE_HI  0x1174  
/**< R/W 32'h0 Register for FDF Ring 5 */


#define PDF0_POLL_INTERVAL  0x2004  
/**< R/W 32'h0 [11:0] 12 bits of PDF Polling interval. This will
be used when polling for the state busy bit.  */
#define PDF0_CONFIG_DEFAULT 2
#define PDF0_CONFIG 0x2008  
/**< R/W 32'h0 "[0] - ENABLE_INDEX_PD_FORMAT
[3:1] Reserved
[4] Payload data Big Endian. Default is little endian.
[5] PDF Big endian. Default is little endian. This bit is change the endianness of the PDF headers.
[7:6] Reserved
[15:8] Padding Character (default is 00)
[23:16] - reserved
[25:24] - MAX_DATA_BURST_SIZE.  These bits are used to determine the max burst size for data. 0 -
128 byte burst, 1-3 - 256 byte burst
[27:26] - reserved
[29:28] - ADDR_BURST_BOUNDARY. These bits are used to determine the address alignments that a burst
can't cross. 0 - 128 Byte Addr Boundary, 1-3 - 256 Byte Addr Boundary
[31:30] Reserved" */
#define PDF0_CONFIG2  0x2018  
/**< R/W 32'h0 "[0] - DISABLE_Q0
[1] - DISABLE_Q1
[2] - DISABLE_Q2
[3] - DISABLE_Q3
[4] - DISABLE_Q4
[5] - DISABLE_Q5
[6] - DISABLE_Q6
[7] - DISABLE_Q7
[8] - DISABLE_Q8
[9] - DISABLE_Q9
[10] - DISABLE_Q10
[11] - DISABLE_Q11
[31:12] - Reserved" */
#define PDF0_CONFIG4  0x2024  
/**< R/W 32'h0000_ffff "[15:0] - BUSY_BIT_POLL_TIMEOUT[15:0].  This
value is used has the maximum number of times the busy bit can be polled before the flow is failed. 
If this value is 16'h0, this timeout function is disable.
[31:16] - reserved" */
#define PDF0_MEMMAP_CONFIG  0x2028  
/**< R/W 32/h0 "[15:0] -
PQ_INTERNAL_MEM_MAP[15:0]
[16] -
PDF_INTERNAL_MEM_MAP
[17] -
PD_INTERNAL_MEM_MAP
[31:18] - reserved" */
#define PDF0_BASE_LOW 0x2030  /**< R/W 32'h0000_0000 [31:0] - PDF_ADDR[31:0].  */
#define PDF0_BASE_HI  0x2034  /**< R/W 32'h0 [31:0] - PDF_ADDR[63:32].   */

#define PD0_BASE_LOW  0x2038
#define PD0_BASE_HI   0x203C

#define PQ0_BASE_LOW_0  0x2040  /**< R/W 32'h0000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'hf0   */
#define PQ0_BASE_HI_0   0x2044  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hf0 */
#define PQ0_BASE_LOW_1  0x2048  /**< R/W 32'h1000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h1.   */
#define PQ0_BASE_HI_1   0x204C  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h1. */
#define PQ0_BASE_LOW_2  0x2050  /**< R/W 32'h2000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h2.   */
#define PQ0_BASE_HI_2   0x2054  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h2. */
#define PQ0_BASE_LOW_3  0x2058  /**< R/W 32'h3000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h3.   */
#define PQ0_BASE_HI_3   0x205C  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h3. */
#define PQ0_BASE_LOW_4  0x2060  /**< R/W 32'h4000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h4.   */
#define PQ0_BASE_HI_4   0x2064  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h4. */
#define PQ0_BASE_LOW_5  0x2068  /**< R/W 32'h5000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h5.   */
#define PQ0_BASE_HI_5   0x206C  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h5. */
#define PQ0_BASE_LOW_6  0x2070  /**< R/W 32'h6000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h6.   */
#define PQ0_BASE_HI_6   0x2074  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h6. */
#define PQ0_BASE_LOW_7  0x2078  /**< R/W 32'h7000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h7.   */
#define PQ0_BASE_HI_7   0x207C  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h7. */
#define PQ0_BASE_LOW_8  0x2080  /**< R/W 32'h8000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h8.   */
#define PQ0_BASE_HI_8   0x2084  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h8. */
#define PQ0_BASE_LOW_9  0x2088  /**< R/W 32'h9000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'h9.   */
#define PQ0_BASE_HI_9   0x208C  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'h9. */
#define PQ0_BASE_LOW_A  0x2090  /**< R/W 32'ha000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'ha.   */
#define PQ0_BASE_HI_A   0x2094  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'ha. */
#define PQ0_BASE_LOW_B  0x2098  /**< R/W 32'hb000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'hb.   */
#define PQ0_BASE_HI_B   0x209C  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hb. */
#define PQ0_BASE_LOW_C  0x20A0  /**< R/W 32'hc000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'hc.   */
#define PQ0_BASE_HI_C   0x20A4  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hc. */
#define PQ0_BASE_LOW_D  0x20A8  /**< R/W 32'hd000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'hd.   */
#define PQ0_BASE_HI_D   0x20AC  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hd. */
#define PQ0_BASE_LOW_E  0x20B0  /**< R/W 32'he000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'he.   */
#define PQ0_BASE_HI_E   0x20B4  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'he. */
#define PQ0_BASE_LOW_F  0x20B8  /**< R/W 32'hf000_0000 [31:0] - PQ_ADDR[31:0] when PQ_BASE[3:0] = 4'hf.   */
#define PQ0_BASE_HI_F   0x20BC  /**< R/W 32'h0 [31:0] - PQ_ADDR[63:32] when PQ_BASE[3:0] = 4'hf. */

#define PDF1_POLL_INTERVAL  0x2104  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PDF1_CONFIG 0x2108  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PDF1_CONFIG4  0x2124  
/**< R/W 32'h0000_ffff Register for PDF/PQ Ring 1  */
#define PDF1_MEMMAP_CONFIG  0x2128  
/**< R/W 32/h0 Register for PDF/PQ Ring 1  */
#define PDF1_BASE_LOW 0x2130  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 1  */
#define PDF1_BASE_HI  0x2134  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_0  0x2140  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_0 0x2144  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_1  0x2148  
/**< R/W 32'h1000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_1 0x214C  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_2  0x2150  
/**< R/W 32'h2000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_2 0x2154  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_3  0x2158  
/**< R/W 32'h3000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_3 0x215C  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_4  0x2160  
/**< R/W 32'h4000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_4 0x2164  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_5  0x2168  
/**< R/W 32'h5000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_5 0x216C  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_6  0x2170  
/**< R/W 32'h6000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_6 0x2174  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_7  0x2178  
/**< R/W 32'h7000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_7 0x217C  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_8  0x2180  
/**< R/W 32'h8000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_8 0x2184  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_9  0x2188  
/**< R/W 32'h9000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_9 0x218C  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_A  0x2190  
/**< R/W 32'ha000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_A 0x2194  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_B  0x2198  
/**< R/W 32'hb000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_B 0x219C  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_C  0x21A0  
/**< R/W 32'hc000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_C 0x21A4  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_D  0x21A8  
/**< R/W 32'hd000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_D 0x21AC  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_E  0x21B0  
/**< R/W 32'he000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_E 0x21B4  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_LOW_F  0x21B8  
/**< R/W 32'hf000_0000 Register for PDF/PQ Ring 1  */
#define PQ1_BASE_HI_F 0x21BC  
/**< R/W 32'h0 Register for PDF/PQ Ring 1  */
#define PDF2_POLL_INTERVAL  0x2104  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PDF2_CONFIG 0x2208  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PDF2_CONFIG4  0x2224  
/**< R/W 32'h0000_ffff Register for PDF/PQ Ring 2  */
#define PDF2_MEMMAP_CONFIG  0x2228  
/**< R/W 32/h0 Register for PDF/PQ Ring 2  */
#define PDF2_BASE_LOW 0x2230  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 2  */
#define PDF2_BASE_HI  0x2234  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_0  0x2240  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_0 0x2244  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_1  0x2248  
/**< R/W 32'h1000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_1 0x224C  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_2  0x2250  
/**< R/W 32'h2000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_2 0x2254  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_3  0x2258  
/**< R/W 32'h3000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_3 0x225C  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_4  0x2260  
/**< R/W 32'h4000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_4 0x2264  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_5  0x2268  
/**< R/W 32'h5000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_5 0x226C  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_6  0x2270  
/**< R/W 32'h6000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_6 0x2274  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_7  0x2278  
/**< R/W 32'h7000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_7 0x227C  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_8  0x2280  
/**< R/W 32'h8000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_8 0x2284  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_9  0x2288  
/**< R/W 32'h9000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_9 0x228C  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_A  0x2290  
/**< R/W 32'ha000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_A 0x2294  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_B  0x2298  
/**< R/W 32'hb000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_B 0x229C  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_C  0x22A0  
/**< R/W 32'hc000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_C 0x22A4  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_D  0x22A8  
/**< R/W 32'hd000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_D 0x22AC  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_E  0x22B0  
/**< R/W 32'he000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_E 0x22B4  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_LOW_F  0x22B8  
/**< R/W 32'hf000_0000 Register for PDF/PQ Ring 2  */
#define PQ2_BASE_HI_F 0x22BC  
/**< R/W 32'h0 Register for PDF/PQ Ring 2  */
#define PDF3_POLL_INTERVAL  0x2304  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PDF3_CONFIG 0x2308  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PDF3_CONFIG4  0x2324  
/**< R/W 32'h0000_ffff Register for PDF/PQ Ring 3  */
#define PDF3_MEMMAP_CONFIG  0x2328  
/**< R/W 32/h0 Register for PDF/PQ Ring 3  */
#define PDF3_BASE_LOW 0x2330  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 3  */
#define PDF3_BASE_HI  0x2334  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_0  0x2340  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_0 0x2344  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_1  0x2348  
/**< R/W 32'h1000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_1 0x234C  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_2  0x2350  
/**< R/W 32'h2000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_2 0x2354  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_3  0x2358  
/**< R/W 32'h3000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_3 0x235C  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_4  0x2360  
/**< R/W 32'h4000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_4 0x2364  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_5  0x2368  
/**< R/W 32'h5000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_5 0x236C  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_6  0x2370  
/**< R/W 32'h6000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_6 0x2374  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_7  0x2378  
/**< R/W 32'h7000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_7 0x237C  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_8  0x2380  
/**< R/W 32'h8000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_8 0x2384  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_9  0x2388  
/**< R/W 32'h9000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_9 0x238C  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_A  0x2390  
/**< R/W 32'ha000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_A 0x2394  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_B  0x2398  
/**< R/W 32'hb000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_B 0x239C  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_C  0x23A0  
/**< R/W 32'hc000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_C 0x23A4  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_D  0x23A8  
/**< R/W 32'hd000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_D 0x23AC  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_E  0x23B0  
/**< R/W 32'he000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_E 0x23B4  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_LOW_F  0x23B8  
/**< R/W 32'hf000_0000 Register for PDF/PQ Ring 3  */
#define PQ3_BASE_HI_F 0x23BC  
/**< R/W 32'h0 Register for PDF/PQ Ring 3  */
#define PDF4_POLL_INTERVAL  0x2404  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PDF4_CONFIG 0x2408  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PDF4_CONFIG4  0x2424  
/**< R/W 32'h0000_ffff Register for PDF/PQ Ring 4  */
#define PDF4_MEMMAP_CONFIG  0x2428  
/**< R/W 32/h0 Register for PDF/PQ Ring 4  */
#define PDF4_BASE_LOW 0x2430  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 4  */
#define PDF4_BASE_HI  0x2434  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_0  0x2440  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_0 0x2444  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_1  0x2448  
/**< R/W 32'h1000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_1 0x244C  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_2  0x2450  
/**< R/W 32'h2000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_2 0x2454  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_3  0x2458  
/**< R/W 32'h3000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_3 0x245C  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_4  0x2460  
/**< R/W 32'h4000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_4 0x2464  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_5  0x2468  
/**< R/W 32'h5000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_5 0x246C  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_6  0x2470  
/**< R/W 32'h6000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_6 0x2474  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_7  0x2478  
/**< R/W 32'h7000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_7 0x247C  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_8  0x2480  
/**< R/W 32'h8000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_8 0x2484  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_9  0x2488  
/**< R/W 32'h9000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_9 0x248C  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_A  0x2490  
/**< R/W 32'ha000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_A 0x2494  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_B  0x2498  
/**< R/W 32'hb000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_B 0x249C  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_C  0x24A0  
/**< R/W 32'hc000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_C 0x24A4  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_D  0x24A8  
/**< R/W 32'hd000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_D 0x24AC  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_E  0x24B0  
/**< R/W 32'he000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_E 0x24B4  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_LOW_F  0x24B8  
/**< R/W 32'hf000_0000 Register for PDF/PQ Ring 4  */
#define PQ4_BASE_HI_F 0x24BC  
/**< R/W 32'h0 Register for PDF/PQ Ring 4  */
#define PDF5_POLL_INTERVAL  0x2504  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PDF5_CONFIG 0x2508  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PDF5_CONFIG4  0x2524  
/**< R/W 32'h0000_ffff Register for PDF/PQ Ring 5  */
#define PDF5_MEMMAP_CONFIG  0x2528  
/**< R/W 32/h0 Register for PDF/PQ Ring 5  */
#define PDF5_BASE_LOW 0x2530  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 5  */
#define PDF5_BASE_HI  0x2534  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_0  0x2540  
/**< R/W 32'h0000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_0 0x2544  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_1  0x2548  
/**< R/W 32'h1000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_1 0x254C  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_2  0x2550  
/**< R/W 32'h2000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_2 0x2554  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_3  0x2558  
/**< R/W 32'h3000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_3 0x255C  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_4  0x2560  
/**< R/W 32'h4000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_4 0x2564  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_5  0x2568  
/**< R/W 32'h5000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_5 0x256C  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_6  0x2570  
/**< R/W 32'h6000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_6 0x2574  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_7  0x2578  
/**< R/W 32'h7000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_7 0x257C  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_8  0x2580  
/**< R/W 32'h8000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_8 0x2584  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_9  0x2588  
/**< R/W 32'h9000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_9 0x258C  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_A  0x2590  
/**< R/W 32'ha000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_A 0x2594  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_B  0x2598  
/**< R/W 32'hb000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_B 0x259C  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_C  0x25A0  
/**< R/W 32'hc000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_C 0x25A4  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_D  0x25A8  
/**< R/W 32'hd000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_D 0x25AC  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_E  0x25B0  
/**< R/W 32'he000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_E 0x25B4  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_LOW_F  0x25B8  
/**< R/W 32'hf000_0000 Register for PDF/PQ Ring 5  */
#define PQ5_BASE_HI_F 0x25BC  
/**< R/W 32'h0 Register for PDF/PQ Ring 5  */

#define GID_BMAP_CTRL 0x2700
/**< R/W 32'h0
[0] - PORT0_WRT (hardware resets to 0)
[1] - PORT0_RD (hardware resets to 0)
[27:16] - GID_BMAP_MEM_ADDR[11:0]" */
#define GID_ERROR 0x2704 /**< RUM 32'h0
 [0] - single bit error detected
 [1] - multi bit error detected */
#define GID_ERROR_HOLD 0x2708 /**< RO 32'h0 
  [11:0] - failed gid bmap addr
  [29:12] - reserved
  [30] - single bit error
  [31] - multi bit error */
#define GID_BMAP_DATA0  0x2710 /**< R/W 32'h0 [31:0] - GID_BMAP_MEM_DATA[31:0]  */
#define GID_BMAP_DATA1  0x2714 /**< R/W 32'h0 [31:0] - GID_BMAP_MEM_DATA[63:32] */

#define BLOCK_CTRL  0x2780
/**< R/W 32'h0 "[0] - UPDATE GLOBAL BLOCK MAP
[1] - DELETE/ADD (1=Add)
[2] - COMMAND_REJECT (1 is fail)
[5:3] - REJECT_CODE
Add
[3] - BLOCK_ACTIVE.  Trying to add a block at is already active
[5:4] - reserved
Delete
[3] - BLOCK_NOT_ACTIVE. Trying to delete a block that is not active
[4] - CTRL_NOT_IDLE.  Trying to delete an active block that is currently allocated to control TDM
ring
[5] - DATA_NOT_IDLE.  Trying to delete an active block that is currently allocated to data TDM ring.
[31:6] - reserved"  */
#define BLOCK_CTRL_MAP  0x2784  
/**< R/W 32'h0 "[11:0] - BLOCK_MAP for function specified in
BLOCK_CTRL register
[31:12] -reserved"  */
#define BLOCK_IDLE_CHECK 0x2788
/**< R/W 32'h0 "[11:0] - IDLE_CHK_MAP[11:0].
[31:12] - reserved" */
#define BLOCK_IDLE_READY 0x278C
/**< RO  32'h0 [11:0] - IDLE_CHECK[11:0].  When bit is set,
corresponding blocks are idle */
#define STREAM_IDLE_MAP  0x2790
/**< RO  32'hfff [11:0] - STREAM_IDLE[11:0].  When bit is a 1,
corresponding stream is idle.  */
#define ACTIVE_BLOCK_MAP  0x2794  
/**< RO  32'h0 [11:0] - ACTIVE_BLOCK_MAP[11:0].  When a bit is a 1,
this block is active and can be used to process data  */
#define CTRL_STREAM_AVAIL 0x2798  
/**< RO  32'h0 [23:0] - CTRL_STREAM_AVAIL0].  There are 2 streams
for each block.  Each bit marks whether a streams CTRL pipe is currently not in use.  This
information is used for scheduling decisions. */
#define DATA_STREAM_AVAIL 0x279c  
/**< RO  32'h0 [23:0] - DATA_STREAM_AVAIL[23:0].  There are 2
streams for each block.  Each bit marks whether a streams DATA pipe is currently in use.  This
information is used for scheduling decisions. */
#define OTHER_AVAIL 0x27A0  
/**< RO  32'h0 "[7:0] - TDM_CTRL_AVAIL[7:0]
[15:8] - TDM_DATA_AVAIL[7:0]
[19:16] - BYPASS_CTRL_AVAIL[3:0]
[23:20] - BYPASS_DATA_AVAIL[3:0]" */
#define BLOCK_TEST_CONFIG 0x27A4
/**< RW 32'h0 [1] - DISABLE_STREAM1. When this bit is set, the "STREAM1" port to each block is
disabled. */

#define GID_LOCK_ACTIVE 0x27a8
/**< RO */

#define GID_BMAP_LOCK 0x27b0
/**< RW */

#define FAMOS_BLK_ADDR  0x5000
/**< R/W 32'h0 [23:0] - BLOCK_ADDR[23:0]
          [31:24] - BLOCK_INDEX[7:0] */
#define FAMOS_BLK_DATA0 0x5004
/**< R/W 32'h0 [31:0] - FAMOS Block Data[31:0] */
#define FAMOS_BLK_DATA1 0x5008
/**< R/W 32'h0 [31:0] - FAMOS Block Data[63:32] */
#define FAMOS_BLK_DATA2 0x500C
/**< R/W 32'h0 [31:0] - FAMOS Block Data[95:64] */
#define FAMOS_CTRL      0x5010
/**< R/W 32'h0  [0] - PORT0_WRT (hardware resets to 0)
                [1] - PORT0_RD (hardware resets to 0) */

#define TEST_REG_ADDR 0x5020
#define TEST_REG_DATA 0x5024
#define TEST_REG_CTRL 0x5028

#define MINI_PRESENT 0x5040
/**< RO 32'b0 [11:0] 0 = block does not have a mini engine 1 = block has mini engine available */
#define MINI_ON 0x5044
/**< RW 32'b0 [11:0] 0 = use circuit array 1 = use mini engine if available */

#define RDF0_CONFIG 0x6000
/**< RRW  32'h0[0] Endian-ness of the results. Default is 0 (little endian) 1
means big endian
[1] - TIMESTAMP_ENABLE. This bit is used to enable the timestamp counter for FRD results.
[3] - RD_INTERNAL_MEM_MAP
[9:8] RDF Max Burst Size. This value controls when the internal FIFO is burst out into external
memory (00 - 16 Bytes, 01 - 32 Bytes, 10 - 64 Bytes, 11 - 128 Bytes)
[31:10] - reserved */
#define RDF0_CLAMP0 0x6004
#define RDF0_CLAMP1 0x6030
/**< RW 32'h0[13:0] - PRE_FRD_MAX_RESULT[13:0].  If this field is a 14'h0, there
is no clamp
[15:14] - PRE_FRD_MAX_CTRL.
0 - fail flow if PRE_FRD_MAX_RESULT clamp is exceeded
1 - flag the overflow, but do not fail the flow
2-3 are reserved
[31:16] - reserved */
#define RDF0_BA_LSB 0x6008
/**< R/W 32'h0 Result Descriptor FIFO Base Address Lower 32 bits */
#define RDF0_BA_MSB 0x600C  
/**< R/W 32'h0 Result Descriptor FIFO Base Address upper 32 bits.  */
#define RDF0_WRT_PTR  0x6010  
/**< RO  32'h0 Result Descriptor FIFO Write Pointer. This register
holds the RDF_WRT_PTR.  */
#define RDF0_RD_PTR 0x6014  
/**< R/W 32'h0 Result Descriptor FIFO Read Pointer. This register holds
the RDF Read Pointer value and is updated by software when result buffers are returned to the ring.*/
#define RDF0_SIZE 0x6018  
/**< R/W 32'h32  Result Descriptor FIFO Size. This is the number of RDs
that can fit in the RDF system memory. Each RD is 4 words (32-bits) in size.  */
#define RDF0_FORCE_TIMER  0x6020
/**< R/W 32'h32  This Timer control when the internal RDF FIFO gets
flushed out to system memory regardless of max burst size.  */
#define RDF0_TIMESTAMP_LOW  0x6024  
/**< RO  32'h0 [31:0] - TIMESTAMP[31:0]. These bits have the
current value of the timestamp counter that is used to mark FRDs. */
#define RDF0_TIMESTAMP_HI 0x6028  
/**< RO  32'h0 "[7:0] - TIMESTAMP[39:32]. These bits have the
current value of the timestamp counter that is used to mark FRDs.
[31:8] - reserved"  */

#define RDF1_CONFIG 0x6080
/**< RW  32'h0Register for RDF Ring 1 */
#define RDF1_CLAMP0 0x6084
#define RDF1_CLAMP1 0x60B0
/**< RW  32'h0Register for RDF Ring 1 */
#define RDF1_BA_LSB 0x6088  
/**< R/W 32'h0 Register for RDF Ring 1 */
#define RDF1_BA_MSB 0x608C  
/**< R/W 32'h0 Register for RDF Ring 1 */
#define RDF1_WRT_PTR  0x6090  
/**< RO  32'h0 Register for RDF Ring 1 */
#define RDF1_RD_PTR 0x6094  
/**< R/W 32'h0 Register for RDF Ring 1 */
#define RDF1_SIZE 0x6098  
/**< R/W 32'h32  Register for RDF Ring 1 */
#define RDF1_FORCE_TIMER  0x60A0
/**< R/W 32'h32  Register for RDF Ring 1 */
#define RDF1_TIMESTAMP_LOW  0x60A4  
/**< RO  32'h0 Register for RDF Ring 1 */
#define RDF1_TIMESTAMP_HI 0x60A8  
/**< RO  32'h0 Register for RDF Ring 1 */

#define RDF2_CONFIG 0x6100
/**< RW  32'h0Register for RDF Ring 2 */
#define RDF2_CLAMP0 0x6104
#define RDF2_CLAMP1 0x6130
/**< RW  32'h0Register for RDF Ring 1 */
#define RDF2_BA_LSB 0x6108  
/**< R/W 32'h0 Register for RDF Ring 2 */
#define RDF2_BA_MSB 0x610C
/**< R/W 32'h0 Register for RDF Ring 2 */
#define RDF2_WRT_PTR  0x6110
/**< RO  32'h0 Register for RDF Ring 2 */
#define RDF2_RD_PTR 0x6114
/**< R/W 32'h0 Register for RDF Ring 2 */
#define RDF2_SIZE 0x6118
/**< R/W 32'h32  Register for RDF Ring 2 */
#define RDF2_FORCE_TIMER  0x6120
/**< R/W 32'h32  Register for RDF Ring 2 */
#define RDF2_TIMESTAMP_LOW  0x6124
/**< RO  32'h0 Register for RDF Ring 2 */
#define RDF2_TIMESTAMP_HI 0x6128
/**< RO  32'h0 Register for RDF Ring 2 */

#define RDF3_CONFIG 0x6180
/**< RW  32'h0Register for RDF Ring 2 */
#define RDF3_CLAMP0 0x6184
#define RDF3_CLAMP1 0x61B0
/**< RW  32'h0Register for RDF Ring 1 */
#define RDF3_BA_LSB 0x6188
/**< R/W 32'h0 Register for RDF Ring 3 */
#define RDF3_BA_MSB 0x618C
/**< R/W 32'h0 Register for RDF Ring 3 */
#define RDF3_WRT_PTR  0x6190
/**< RO  32'h0 Register for RDF Ring 3 */
#define RDF3_RD_PTR 0x6194
/**< R/W 32'h0 Register for RDF Ring 3 */
#define RDF3_SIZE 0x6198
/**< R/W 32'h32  Register for RDF Ring 3 */
#define RDF3_FORCE_TIMER  0x61a0
/**< R/W 32'h32  Register for RDF Ring 3 */
#define RDF3_TIMESTAMP_LOW  0x61a4
/**< RO  32'h0 Register for RDF Ring 3 */
#define RDF3_TIMESTAMP_HI 0x61a8  
/**< RO  32'h0 Register for RDF Ring 3 */

#define RDF4_CONFIG 0x6200
/**< RW  32'h0Register for RDF Ring 2 */
#define RDF4_CLAMP0 0x6204
#define RDF4_CLAMP1 0x6230
/**< RW  32'h0Register for RDF Ring 1 */
#define RDF4_BA_LSB 0x6208
/**< R/W 32'h0 Register for RDF Ring 4 */
#define RDF4_BA_MSB 0x620C  
/**< R/W 32'h0 Register for RDF Ring 4 */
#define RDF4_WRT_PTR  0x6210
/**< RO  32'h0 Register for RDF Ring 4 */
#define RDF4_RD_PTR 0x6214
/**< R/W 32'h0 Register for RDF Ring 4 */
#define RDF4_SIZE 0x6218
/**< R/W 32'h32  Register for RDF Ring 4 */
#define RDF4_FORCE_TIMER  0x6220
/**< R/W 32'h32  Register for RDF Ring 4 */
#define RDF4_TIMESTAMP_LOW  0x6224
/**< RO  32'h0 Register for RDF Ring 4 */
#define RDF4_TIMESTAMP_HI 0x6228
/**< RO  32'h0 Register for RDF Ring 4 */

#define RDF5_CONFIG 0x6280
/**< RW  32'h0Register for RDF Ring 2 */
#define RDF5_CLAMP0 0x6284
#define RDF5_CLAMP1 0x62B0
/**< RW  32'h0Register for RDF Ring 1 */
#define RDF5_BA_LSB 0x6288
/**< R/W 32'h0 Register for RDF Ring 5 */
#define RDF5_BA_MSB 0x628C
/**< R/W 32'h0 Register for RDF Ring 5 */
#define RDF5_WRT_PTR  0x6290
/**< RO  32'h0 Register for RDF Ring 5 */
#define RDF5_RD_PTR 0x6294
/**< R/W 32'h0 Register for RDF Ring 5 */
#define RDF5_SIZE 0x6298
/**< R/W 32'h32  Register for RDF Ring 5 */
#define RDF5_FORCE_TIMER  0x62A0
/**< R/W 32'h32  Register for RDF Ring 5 */
#define RDF5_TIMESTAMP_LOW  0x62A4
/**< RO  32'h0 Register for RDF Ring 5 */
#define RDF5_TIMESTAMP_HI 0x62A8
/**< RO  32'h0 Register for RDF Ring 5 */

#define ACTION_DATA0 0x6300
/**< RW  32'h0[31:0] - Action RAM Data[31:0] */
#define ACTION_DATA1 0x6304
/**< RW  32'h0[31:0] - Action RAM Data[63:32] */
#define ACTION_DATA2 0x6308
/**< RW  32'h0[31:0] - Action RAM Data[95:64] */
#define ACTION_DATA3 0x630C
/**< RW  32'h0[31:0] - ACTION RAM Data[127:96] */
#define ACTION_CTRL 0x6310
/**< RW  32'h0
[0] - Action RAM_WRT (hardware resets to 0)
[1] - Action RAM_RD (hardware resets to 0)
[30:16] - Action RAM Addr[14:0] */
#define ACTION_ERROR 0x6318
/**< RO 32'h0  [0] - single bit error detected
               [1]       - multi-bit error detected */

#define CNXT0_BASE_BASE_LO  0x6400  
/**< R/W 32'h0 [31:0] - This register holds bits [31:0] of the
CNXT_BASE_ADDR[63:0]. The CNXT_BASE_ADDR register is a 64 bit register that points to the base of
the CNXT ring.  */
#define CNXT0_BASE_BASE_HI  0x6404  
/**< R/W 32'h0 [31:0] - This register holds bits [63:32] of the
CNXT_BASE_ADDR[63:0]. The CNXT_BASE_ADDR register is a 64 bit register that points to the base of
the CNXT ring.  */
#define CNXT0_BUFF_BASE_LO  0x6408  
/**< R/W 32'h0 "[31:28] - This register holds
CNXT_BUFF_ADDR[31:28]. Bits [63:28] of the CNXT_BUFF_ADDR register are concatenated with lower 28
bits of address that is pulled off the ring.
[27:0] - reserved"  */
#define CNXT0_BUFF_BASE_HI  0x640C  
/**< R/W 32'h0 [31:0] - This register holds bits
CNXT_BUFF_ADDR[63:32]. Bits [63:28] of the CNXT_BUFF_ADDR register are concatenated with lower 28
bits of address that is pulled off the ring.  */
#define CNXT0_WRT_PTR 0x6410  
/**< RO  32'h0 [31:0] - This register contains the CNXT_WRT_PTR[31:0].
This pointer is owned by hardware.  The hardware pushs any returning buffer addresses to the
location pointer to by the CNXT_WRT_PTR.  */
#define CNXT0_RD_PTR  0x6414  
/**< RO  32'h0 [31:0] - This register contains the CNXT_RD_PTR[31:0].
This pointer is owned by hardware.  The hardware pulls available buffer addresses to the location
pointer to by the CNXT_RD_PTR */
#define CNXT0_SIZE  0x6420  
/**< R/W 32'h32  [31:0] - This register contains the CNXT_SIZE[31:0] .
The CNXT_SIZE register defines the size of the context ring.  0x0001 represents 1 buffer. */
#define CNXT0_CTRL  0x6424  
/**< R/W 32'hffff_10_00  "[0] - ENABLE.  When this bit is set to a 1,
hardware starts processing the context ring.
[1] - CNXT_DATA_INTERNAL_MEMMAP
[2] - CNXT_RING_INTERNAL_MEMMAP
[3] - "This function is MARS use to get controlled by the big endian bit in the RSLT ring…so if you
set this bit to the same value you currently set the results ring endian bit…things will continue
to behave as you expect…"
[7:4] - reserved
[15:8] - MAX_WRT_BUFFER_TIMER.  This value is used to determine how long the internal wrt buffer
FIFO waits to gather up returning buffers to form larger write burst.  The counter is 16 system
clock ticks with 0=16 clocks, and xFF=256*16 clocks.
[31:16] - CNXT_MAX_STATE_NUM[15:0]. This register can be used by software to limit the maximum
number of MARS states that can be stored for any flow during a context swap.  It should be noted
that there are internal limits to the number of states that the MARS engine can store in hardware." 
                            */
#define CNXT0_FIFO_CNT  0x6430  
/**< RO  NA  "[4:0] - RD_FIFO_LEVEL.  This value is the number of
internal buffer addresses held in the internal RD_BUFFER fifo.
[15:5] - reserved
[20:16] - WRT_FIFO_LEVEL.  This value is the number of internal buffer addresses held in the
internal WRT_BUFFER fifo.
[31:21] - reserved" */
#define CNXT0_POP_CTRL  0x6440  
/**< R/W 32'h0 "[0] - CNXT_BUFF_POP.  When this bit is set to a 1,
the CPU interface attempts to pop a buffer off the Context Free Pool.  This bit is reset by hardware
when the pop request is completed.
[1] - reserved
[2] - POP_PASS.  This is is loaded when the CNXT_BUFF_POP bit is reset.  When this bit is a 1, the
POP succeeded and a buffer address was loaded into the CNXT_POP_DATA register.  When this bit is a
0, the POP failed because the CNXT_FREE_POOL was empty.
[31:3] - reserved " */
#define CNXT0_POP_DATA  0x6444  
/**< RO  32'h0 [31:0] CNXT_BUFFER_ADDR[31:0].  This register holds
the address that is popped off the CNXT_FREE pool ring.  If the pop failed, this register is reset
to 32'h0. */
#define CNXT0_WATERMARK 0x6448  
/**< R/W 32'hffff_ffff This register is used to mark the lowest point
the context free pool reached during operations.  To -reset-, a 32xffff_ffff should be written to
the register to set up another reading. */
#define CNXT1_BASE_BASE_LO  0x6480  
/**< R/W 32'h0 Register for CNXT Ring 1  */
#define CNXT1_BASE_BASE_HI  0x6484  
/**< R/W 32'h0 Register for CNXT Ring 1  */
#define CNXT1_BUFF_BASE_LO  0x6488  
/**< R/W 32'h0 Register for CNXT Ring 1  */
#define CNXT1_BUFF_BASE_HI  0x648C  
/**< R/W 32'h0 Register for CNXT Ring 1  */
#define CNXT1_WRT_PTR 0x6490  
/**< RO  32'h0 Register for CNXT Ring 1  */
#define CNXT1_RD_PTR  0x6494  
/**< RO  32'h0 Register for CNXT Ring 1  */
#define CNXT1_SIZE  0x64A0  
/**< R/W 32'h32  Register for CNXT Ring 1  */
#define CNXT1_CTRL  0x64A4  
/**< R/W 32'hffff_10_00  Register for CNXT Ring 1  */
#define CNXT1_FIFO_CNT  0x64B0  
/**< RO  NA  Register for CNXT Ring 1  */
#define CNXT1_POP_CTRL  0x64C0  
/**< R/W 32'h0 Register for CNXT Ring 1  */
#define CNXT1_POP_DATA  0x64C4  
/**< RO  32'h0 Register for CNXT Ring 1  */
#define CNXT1_WATERMARK 0x64C8  
/**< R/W 32'hffff_ffff Register for CNXT Ring 1  */
#define CNXT2_BASE_BASE_LO  0x6500  
/**< R/W 32'h0 Register for CNXT Ring 2  */
#define CNXT2_BASE_BASE_HI  0x6504  
/**< R/W 32'h0 Register for CNXT Ring 2  */
#define CNXT2_BUFF_BASE_LO  0x6508  
/**< R/W 32'h0 Register for CNXT Ring 2  */
#define CNXT2_BUFF_BASE_HI  0x650C  
/**< R/W 32'h0 Register for CNXT Ring 2  */
#define CNXT2_WRT_PTR 0x6510  
/**< RO  32'h0 Register for CNXT Ring 2  */
#define CNXT2_RD_PTR  0x6514  
/**< RO  32'h0 Register for CNXT Ring 2  */
#define CNXT2_SIZE  0x6520  
/**< R/W 32'h32  Register for CNXT Ring 2  */
#define CNXT2_CTRL  0x6524  
/**< R/W 32'hffff_10_00  Register for CNXT Ring 2  */
#define CNXT2_FIFO_CNT  0x6530  
/**< RO  NA  Register for CNXT Ring 2  */
#define CNXT2_POP_CTRL  0x6540  
/**< R/W 32'h0 Register for CNXT Ring 2  */
#define CNXT2_POP_DATA  0x6544  
/**< RO  32'h0 Register for CNXT Ring 2  */
#define CNXT2_WATERMARK 0x6548  
/**< R/W 32'hffff_ffff Register for CNXT Ring 2  */
#define CNXT3_BASE_BASE_LO  0x6580  
/**< R/W 32'h0 Register for CNXT Ring 3  */
#define CNXT3_BASE_BASE_HI  0x6584  
/**< R/W 32'h0 Register for CNXT Ring 3  */
#define CNXT3_BUFF_BASE_LO  0x6588  
/**< R/W 32'h0 Register for CNXT Ring 3  */
#define CNXT3_BUFF_BASE_HI  0x658C  
/**< R/W 32'h0 Register for CNXT Ring 3  */
#define CNXT3_WRT_PTR 0x6590  
/**< RO  32'h0 Register for CNXT Ring 3  */
#define CNXT3_RD_PTR  0x6594  
/**< RO  32'h0 Register for CNXT Ring 3  */
#define CNXT3_SIZE  0x65A0  
/**< R/W 32'h32  Register for CNXT Ring 3  */
#define CNXT3_CTRL  0x65A4  
/**< R/W 32'hffff_10_00  Register for CNXT Ring 3  */
#define CNXT3_FIFO_CNT  0x65B0  
/**< RO  NA  Register for CNXT Ring 3  */
#define CNXT3_POP_CTRL  0x65C0  
/**< R/W 32'h0 Register for CNXT Ring 3  */
#define CNXT3_POP_DATA  0x65C4  
/**< RO  32'h0 Register for CNXT Ring 3  */
#define CNXT3_WATERMARK 0x65C8  
/**< R/W 32'hffff_ffff Register for CNXT Ring 3  */
#define CNXT4_BASE_BASE_LO  0x6600  
/**< R/W 32'h0 Register for CNXT Ring 4  */
#define CNXT4_BASE_BASE_HI  0x6604  
/**< R/W 32'h0 Register for CNXT Ring 4  */
#define CNXT4_BUFF_BASE_LO  0x6608  
/**< R/W 32'h0 Register for CNXT Ring 4  */
#define CNXT4_BUFF_BASE_HI  0x660C  
/**< R/W 32'h0 Register for CNXT Ring 4  */
#define CNXT4_WRT_PTR 0x6610  
/**< RO  32'h0 Register for CNXT Ring 4  */
#define CNXT4_RD_PTR  0x6614  
/**< RO  32'h0 Register for CNXT Ring 4  */
#define CNXT4_SIZE  0x6620  
/**< R/W 32'h32  Register for CNXT Ring 4  */
#define CNXT4_CTRL  0x6624  
/**< R/W 32'hffff_10_00  Register for CNXT Ring 4  */
#define CNXT4_FIFO_CNT  0x6630  
/**< RO  NA  Register for CNXT Ring 4  */
#define CNXT4_POP_CTRL  0x6640  
/**< R/W 32'h0 Register for CNXT Ring 4  */
#define CNXT4_POP_DATA  0x6644  
/**< RO  32'h0 Register for CNXT Ring 4  */
#define CNXT4_WATERMARK 0x6648  
/**< R/W 32'hffff_ffff Register for CNXT Ring 4  */
#define CNXT5_BASE_BASE_LO  0x6680  
/**< R/W 32'h0 Register for CNXT Ring 5  */
#define CNXT5_BASE_BASE_HI  0x6684  
/**< R/W 32'h0 Register for CNXT Ring 5  */
#define CNXT5_BUFF_BASE_LO  0x6688  
/**< R/W 32'h0 Register for CNXT Ring 5  */
#define CNXT5_BUFF_BASE_HI  0x668C  
/**< R/W 32'h0 Register for CNXT Ring 5  */
#define CNXT5_WRT_PTR 0x6690  
/**< RO  32'h0 Register for CNXT Ring 5  */
#define CNXT5_RD_PTR  0x6694  
/**< RO  32'h0 Register for CNXT Ring 5  */
#define CNXT5_SIZE  0x66A0  
/**< R/W 32'h32  Register for CNXT Ring 5  */
#define CNXT5_CTRL  0x66A4  
/**< R/W 32'hffff_10_00  Register for CNXT Ring 5  */
#define CNXT5_FIFO_CNT  0x66B0  
/**< RO  NA  Register for CNXT Ring 5  */
#define CNXT5_POP_CTRL  0x66C0  
/**< R/W 32'h0 Register for CNXT Ring 5  */
#define CNXT5_POP_DATA  0x66C4  
/**< RO  32'h0 Register for CNXT Ring 5  */
#define CNXT5_WATERMARK 0x66C8  
/**< R/W 32'hffff_ffff Register for CNXT Ring 5  */

#define BLOCK_BASE_LOW 0x6800
/**< R/W 32'h0 [31:0] - This register holds bits [31:0]
of the BLOCK_BASE_ADDR[63:0]
that points to the base of the memory
containing the images of the FAMOS
Blocks. */
#define BLOCK_BASE_HI           0x6804
/**< R/W     32'h0 [31:0] - This register holds bits
[63:32] of the
BLOCK_BASE_ADDR[63:0] that
points to the memory containing the
images of the FAMOS Blocks. */
#define BLOCK_BASE_CONFIG       0x6808
/**< R/W     32'h0 [0] - internal memory map
[1] - big endian */
#define BLOCK0_CONFIG           0x6810
/**< R/W     32'h0 [0] - BLOCK0_ENABLE
[31:1] - reserved */
#define BLOCK0_OFFSET           0x6814
/**< R/W     32'h0 [31:0] - This register contains the
address offset from the
BLOCK_BASE_ADDR register when
the start of the image for FAMOS
Block 0 is located. */
#define BLOCK1_CONFIG           0x6818
/**< R/W     32'h0 Same as BLOCK1_CONFIG */
#define BLOCK1_OFFSET           0x681c
/**< R/W     32'h0 Same as BLOCK1_OFFSET */
#define BLOCK2_CONFIG     0x6820
/**< R/W 32'h0 Same as BLOCK2_CONFIG */
#define BLOCK2_OFFSET     0x6824
/**< R/W 32'h0 Same as BLOCK2_OFFSET */
#define BLOCK3_CONFIG     0x6828
/**< R/W 32'h0 Same as BLOCK3_CONFIG */
#define BLOCK3_OFFSET     0x682c
/**< R/W 32'h0 Same as BLOCK3_OFFSET */
#define BLOCK4_CONFIG     0x6830
/**< R/W 32'h0 Same as BLOCK4_CONFIG */
#define BLOCK4_OFFSET     0x6834
/**< R/W 32'h0 Same as BLOCK4_OFFSET */
#define BLOCK5_CONFIG     0x6838
/**< R/W 32'h0 Same as BLOCK5_CONFIG */
#define BLOCK5_OFFSET     0x683c
/**< R/W 32'h0 Same as BLOCK5_OFFSET */
#define BLOCK6_CONFIG     0x6840
/**< R/W 32'h0 Same as BLOCK6_CONFIG */
#define BLOCK6_OFFSET     0x6844
/**< R/W 32'h0 Same as BLOCK6_OFFSET */
#define BLOCK7_CONFIG     0x6848
/**< R/W 32'h0 Same as BLOCK7_CONFIG */
#define BLOCK7_OFFSET     0x684c
/**< R/W 32'h0 Same as BLOCK7_OFFSET */
#define BLOCK8_CONFIG     0x6850
/**< R/W 32'h0 Same as BLOCK8_CONFIG */
#define BLOCK8_OFFSET     0x6854
/**< R/W 32'h0 Same as BLOCK8_OFFSET */
#define BLOCK9_CONFIG     0x6858
/**< R/W 32'h0 Same as BLOCK9_CONFIG */
#define BLOCK9_OFFSET     0x685c
/**< R/W 32'h0 Same as BLOCK9_OFFSET */
#define BLOCK10_CONFIG    0x6860
/**< R/W 32'h0 Same as BLOCK10_CONFIG */
#define BLOCK10_OFFSET    0x6864
/**< R/W 32'h0 Same as BLOCK10_OFFSET */
#define BLOCK11_CONFIG    0x6868
/**< R/W 32'h0 Same as BLOCK11_CONFIG */
#define BLOCK11_OFFSET    0x686c
/**< R/W 32'h0 Same as BLOCK11_OFFSET */

#define BLOCK_DMA_CONFIG0  0x6880  
/**<
[23:0]  - BLOCK_START_OFFSET[23:0]
[27:24] - BLOCK_SEL[3:0]
[31:28] - reserved"  */
#define BLOCK_DMA_CONFIG1  0x6884  
/**<
[23:0] - RULE_TRANSFER_SIZE[23:0]
[31:24] - reserved" */
#define BLOCK_DMA_CONFIG2  0x6888  
/**< R/W 32'h0 "[0] - When this bit is set to a 1, the hardware
starts DMAing the external RULE Table piece into internal memory.  Hardware resets this bit when the
transfer is complete.
[31:1] - reserved"  */
#define BLOCK_DMA_CUR_ADDR 0x688C  
/**< RO  NA  [19:0] -  RULE_DMA_CUR_ADDR[19:0] contains the current
virtual index that the Rule DMA is pointing to.  When the DMA is complete, this value is equal to
the RULE_TABLE_START_OFFSET + RULE_TRANSFER_SIZE  */

#define STATS_CNT_CONFIG  0x5800  
/**< R/W 32'h0 "[9:0] - COUNTER_ENABLE[9:0]
[15:10] - reserved
[25:16] - COUNTER_CLEAR[9:0]
[31:26] - reserved" */
#define STATS_MUX_SEL 0x5804  
/**< R/W 32'h0 [3:0] - STATS_MUX_SEL[3:0]  */
#define PREAMBLE_MATCH  0x5808  
/**< R/W 32'h0 "[27:0] - PDF_PREAMBLE_ADDR
[31:28] - reserved" */
#define COUNTER0_LOW  0x5840  
/**< R/W 32'h0 [31:0] - COUNTER0[31:0] */
#define COUNTER0_HI 0x5844  
/**< R/W 32'h0 [31:0] - COUNTER0[63:32]  */
#define COUNTER1_LOW  0x5848  
/**< R/W 32'h0 [31:0] - COUNTER1[31:0] */
#define COUNTER1_HI 0x584C  
/**< R/W 32'h0 [31:0] - COUNTER1[63:32]  */
#define COUNTER2_LOW  0x5850  
/**< R/W 32'h0 [31:0] - COUNTER2[31:0] */
#define COUNTER2_HI 0x5854  
/**< R/W 32'h0 [31:0] - COUNTER2[63:32]  */
#define COUNTER3_LOW  0x5858  
/**< R/W 32'h0 [31:0] - COUNTER3[31:0] */
#define COUNTER3_HI 0x585c  
/**< R/W 32'h0 [31:0] - COUNTER3[63:32]  */
#define COUNTER4_LOW  0x5860  
/**< R/W 32'h0 [31:0] - COUNTER4[31:0] */
#define COUNTER4_HI 0x5864  
/**< R/W 32'h0 [31:0] - COUNTER4[63:32]  */
#define COUNTER5_LOW  0x5868  
/**< R/W 32'h0 [31:0] - COUNTER5[31:0] */
#define COUNTER5_HI 0x586c  
/**< R/W 32'h0 [31:0] - COUNTER5[63:32]  */
#define COUNTER6_LOW  0x5870  
/**< R/W 32'h0 [31:0] - COUNTER6[31:0] */
#define COUNTER6_HI 0x5874  
/**< R/W 32'h0 [31:0] - COUNTER6[63:32]  */
#define COUNTER7_LOW  0x5878  
/**< R/W 32'h0 [31:0] - COUNTER7[31:0] */
#define COUNTER7_HI 0x587c  
/**< R/W 32'h0 [31:0] - COUNTER7[63:32]  */
#define COUNTER8_LOW  0x5880  
/**< R/W 32'h0 [31:0] - COUNTER8[31:0] */
#define COUNTER8_HI 0x5884  
/**< R/W 32'h0 [31:0] - COUNTER8[63:32]  */
#define COUNTER9_LOW  0x5888  
/**< R/W 32'h0 [31:0] - COUNTER9[31:0] */
#define COUNTER9_HI 0x588c  
/**< R/W 32'h0 [31:0] - COUNTER9[63:32]  */
#define TEST_AND_RESET0 0x5900  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET1 0x5904  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET2 0x5908  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET3 0x590c  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET4 0x5910  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET5 0x5914  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET6 0x5918  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET7 0x591c  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET8 0x5920  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET9 0x5924  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET10  0x5928  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET11  0x592c  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET12  0x5930  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET13  0x5934  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET14  0x5938  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */
#define TEST_AND_RESET15  0x593c  
/**< R/W 32'h1 "[0] - TEST_AND_RESET.
When this register is read, the current value of this bit is fed back to the interface and then the
bit is reset to a 0.  A write of a 1 to this bit causes it to get set back to a 1.  Writes of 0 to
this bit are ignored.
[31:1] - reserved"  */

/* registers to access shell */
#define SHELL_CMD       0x8000
#define SHELL_ADDR_LO   0x8008
#define SHELL_ADDR_HI   0x800C
#define SHELL_ADDR_SIZE 0x8010

/* registers to access DMA engine DDR <-> host memory */
#define DDR_DMA_SRC_ADDR_LO  0x8080
#define DDR_DMA_SRC_ADDR_HI  0x8084
#define DDR_DMA_DST_ADDR_LO  0x8088
#define DDR_DMA_DST_ADDR_HI  0x808C
#define DDR_DMA_CTRL         0x8090
#define DDR_DMA_STATUS       0x8094
#define DDR_DMA_ENABLE       0x8098


#define XAUI_Shell_Control_register 0x8100
/**<  R/W 0x0 XAUI_shell_control_register:
[0]:_configure_XAUI_in_Shell_mode
[1]:_cross-loopback_defining_that_XAUI1_is_looped_back_on_the_board_to_XAUI2_when_set._If_unset,
_each_XAUI_is_assumed_to_be_loopback_through_serial_loopback_or_external_load_board_loopback_or_even
_far-end_loopback.
[2]:_start_XAUI_shell_test_on_XAUI1
[3]:_start_XAUI_shell_test_on_XAUI2
[31:4]:_not_implemented_and_thus_read-only_(RO) */
#define XAUI_Shell_Status_register  0x8104
/**<  RW1C  0x0
[7:0]:_error_counter_for_XAUI1_which_saturate_at_128_(0x80).
_Writing_any_value_to_this_register_will_clear_the_error_counter.
[14:8]:_reserved_and_not_implemented
[15]:_XAUI1_busy_indicating_that_XAUI_shell_fifo_is_expecting_to_receive_additional_packet.
_When_XAUI_start_register_get_clear,_this_register_is_expected_to_get_back_to_0_eventually._
[23:16]:_error_counter_for_XAUI2_which_saturate_at_128_(0x80).
_Writing_any_value_to_this_register_will_clear_the_error_counter.
[30:24]:_reserved_and_not_implemented
[31]:_XAUI2_busy_indicating_that_XAUI_shell_fifo_is_expecting_to_receive_additional_packet.
_When_XAUI_start_register_get_clear,_this_register_is_expected_to_get_back_to_0_eventually.  */
#define IND_ADDR_LSB  0x8140
/**<  R/W 0x0000 0000
  This_register_defines_the_indirect_access_to_memory_32-bit_LSB_address_bits_of_the_64_bit_address
*/
#define IND_ADDR_MSB  0x8144
/**<  R/W 0x0000
0000

This_register_defines_the_indirect_access_to_memory_32-bit_MSB_address_bits_of_the_64_bit_address._
For_internal_address_access,
_the_4_MSB_bits_are_internally_decoded_for_being_able_to_access_XAUI_buffer_memory_or_DDR3_memory:
-_0x0_:_XAUI_buffer_1
-_0x1_:_XAUI_buffer_2
-_0x2_:_DDR3_SDRAM_(local_memory)
For_external_address_access_(through_PCIe_bus),_the_full_64-bit_address_space_is_accessible.  */
#define IND_CTRL  0x8148
/**<  R/W 0x0000
0000

This_register_defines_the_indirect_access_control_register.
_Writing_to_this_address_location_also_immediately_triggers_an_indirect_access:
-_[4:0]_:_Indirect_access_byte_count_(only_4,_8,_12_and_16_bytes_are_permitted)
-_[8]_:_Read_operation_(1'b1)_or_write_operation_(1'b0)
-_[12]:_internal_address_(1'b1)_or_PCIe_address_(1'b0)
-_[20:16]:_PCIe_parameter_(No_Snoop,_Relaxed_Ordering,_Traffic_Class) */
#define IND_STATUS  0x814C
/**<  RO  0x0000
0000
  Busy_status_of_the_indirect_access
-_[0]_:_Busy_status_which_get_asserted_after_writing_in_IND_CTRL_register_and_automatically_de-
asserted_when_transaction_completes.
-_[31:1]_:_reserved */
#define IND_DATA0 0x8150
/**<  R/W 0x0000
0000
  This_register_contains_the_1st_dword_to_write_or_read_through_indirect_access._
For_write_operation,
_the_data_register_need_to_written_prior_to_trigger_the_indirect_access_start_by_writing_in_IND_CTRL
_register
For_Read_operation,_the_value_is_only_valid_after_busy_status_bit_(IND_STATUS_register)_is_cleared.
*/
#define IND_DATA1 0x8154
/**<  R/W 0x0000
0000
  This_register_contains_the_2nd_dword_to_write_or_read_through_indirect_access._
For_write_operation,
_the_data_register_need_to_written_prior_to_trigger_the_indirect_access_start_by_writing_in_IND_CTRL
_register
For_Read_operation,_the_value_is_only_valid_after_busy_status_bit_(IND_STATUS_register)_is_cleared.
*/
#define IND_DATA2  0x8158
/**<  R/W 0x0000
0000
  This_register_contains_the_3rd_dword_to_write_or_read_through_indirect_access._
For_write_operation,
_the_data_register_need_to_written_prior_to_trigger_the_indirect_access_start_by_writing_in_IND_CTRL
_register
For_Read_operation,_the_value_is_only_valid_after_busy_status_bit_(IND_STATUS_register)_is_cleared.
*/
#define IND_DATA3 0x815C
/**<  R/W 0x0000
0000
  This_register_contains_the_4th_dword_to_write_or_read_through_indirect_access._
For_write_operation,
_the_data_register_need_to_written_prior_to_trigger_the_indirect_access_start_by_writing_in_IND_CTRL
_register
For_Read_operation,_the_value_is_only_valid_after_busy_status_bit_(IND_STATUS_register)_is_cleared.
*/

/*  DDR3_DRAM_CONTROLLER */

#define DDR3_CTL_00 0x9000
/**<  R/W 32'h0 [24]_-_BIG_ENDIAN_EN(RW)
[16]_-_AUTO_REFRESH_MODE_(RW)
[8]_-_AREFRESH_(WR)
[0]_-_AP_(RW) */
#define DDR3_CTL_01 0x9004
/**<  R/W 32'h0 [24]_-_CONCURRENTAP_(RW)
[16]_-_BIST_GO_(WR)
[8]_-_BIST_DATA_CHECK_(RW)
[0]_-_BIST_ADDR_CHECK_(RW)  */
#define DDR3_CTL_02 0x9008
/**<  R/W 32'h0 [24]_-_DRIVE_DQ_DQS_(RW)
[16]_-_DQS_N_EN_(RW)
[8]_-_DLL_BYPASS_MODE_(RW)
[0]_-_DLLLOCKREG_(RO) */
#define DDR3_CTL_03 0x900C
/**<  R/W 32'h0 [24]_-_EN_WR_LEVELING_(RW)
[16]_-_ENABLE_QUICK_SREFRESH_(RW)
[8]_-_EIGHT_BANK_MODE_(RW)
[0]_-_ECC_DISABLE_W_UC_ERR_(RW) */
#define DDR3_CTL_04 0x9010
/**<  R/W 32'h0 [24]_-_INTRPTWRITEA_(RW)
[16]_-_INTRPTREADA_(RW)
[8]_-_INTRPTAPBURST_(RW)
[0]_-_FWC_(WR)  */
#define DDR3_CTL_05 0x9014
/**<  R/W 32'h0 [24]_-_POWER_DOWN_(RW)
[16]_-_ODT_ALT_EN_(RW)
[8]_-_ODT_ADD_TURN_CLK_EN_(RW)
[0]_-_NO_CMD_INIT_(RW)  */
#define DDR3_CTL_06 0x9018
/**<  R/W 32'h0 [24]_-_RDLVL_EDGE_(RW)
[16]_-_RDLVL_CS_(RW)
[8]_-_RDLVL_BEGIN_DELAY_EN_(RW)
[0]_-_PWRUP_SREFRESH_EXIT_(RW)  */
#define DDR3_CTL_07 0x901C
/**<  R/W 32'h0 [24]_-_RDLVL_GATE_REQ_(RW)
[16]_-_RDLVL_GATE_PREAMBLE_CHECK_EN_(RW)
[8]_-_RDLVL_GATE_EN_(RW)
[0]_-_RDLVL_EN_(RW) */
#define DDR3_CTL_08 0x9020
/**<  R/W 32'h0 [24]_-_RDLVL_OFFSET_DIR3_(RW)
[16]_-_RDLVL_OFFSET_DIR2_(RW)
[8]_-_RDLVL_OFFSET_DIR1_(RW)
[0]_-_RDLVL_OFFSET_DIR0_(RW)  */
#define DDR3_CTL_09 0x9024
/**<  R/W 32'h0 [24]_-_RDLVL_OFFSET_DIR7_(RW)
[16]_-_RDLVL_OFFSET_DIR6_(RW)
[8]_-_RDLVL_OFFSET_DIR5_(RW)
[0]_-_RDLVL_OFFSET_DIR4_(RW)  */
#define DDR3_CTL_10 0x9028
/**<  R/W 32'h0 [24]_-_REG_DIMM_ENABLE_(RW)
[16]_-__REDUC_(RW)
[8]_-_RDLVL_REQ_(WR)
[0]_-_RDLVL_OFFSET_DIR8_(RW)  */
#define DDR3_CTL_11 0x902C
/**<  R/W 32'h0 [24]_-_SWLVL_LOAD_(WR)
[16]_-__SWLVL_EXIT_(WR)
[8]_-_START_(RW)
[0]_-_SREFRESH_(RW) */
#define DDR3_CTL_12 0x9030
/**<  R/W 32'h0 [24]_-_TREF_ENABLE_(RW)
[16]_-__TRAS_LOCKOUT_(RW)
[8]_-_SWLVL_START_(WR)
[0]_-_SWLVL_OP_DONE_(RO)  */
#define DDR3_CTL_13 0x9034
/**<  R/W 32'h0 [24]_-_WRLVL_INTERVAL_CT_EN_(RW)
[16]_-__WRLVL_CS_(RW)
[8]_-_WRITE_MODEREG_(WR)
[0]_-_WRITEINTERP_(RW)  */
#define DDR3_CTL_14 0x9038
/**<  R/W 32'h0 [25:24]_-_CS_MAP_(RW)
[17:16]_-__BIST_RESULT_(RO)
[9:8]_-_ADDRESS_MIRRORING_(RW)
[0]_-_WRLVL_REQ_(WR)  */
#define DDR3_CTL_15 0x903C
/**<  R/W 32'h00020000  [25:24]_-_ODT_RD_MAP_CS0_(RW)
[17:16]_-__MAX_CS_REG_(RO)
[9:8]_-_DRAM_CLK_DISABLE_(RW)
[1:0]_-_CTRL_RAW_(RW) */
#define DDR3_CTL_16 0x9040
/**<  R/W 32'h0 [25:24]_-_Q_FULLNESS_(RW)
[17:16]_-__ODT_WR_MAP_CS1(RW)
[9:8]_-_ODT_WR_MAP_CS0_(RW)
[1:0]_-_ODT_RD_MAP_CS1_(RW) */
#define DDR3_CTL_17 0x9044
/**<  R/W 32'h0 [25:24]_-_ZQ_ON_SREF_EXIT_(RW)
[17:16]_-__ZQCS_CHIP_(RW)
[9:8]_-_SW_LEVELING_MODE_(RW)
[1:0]_-_RTT_0_(RW)  */
#define DDR3_CTL_18 0x9048
/**<  R/W 32'h0 [26:24]_-_CKE_DELAY_(RW)
[18:16]_-__BSTLEN_(RW)
[10:8]_-_ADDR_PINS_(RW)
[1:0]_-_ZQ_REQ_(RW) */
#define DDR3_CTL_19 0x904C
/**<  R/W 32'h0 [26:24]_-_TCKE_(RW)
[18:16]_-__OUT_OF_RANGE_TYPE_(RO)
[10:8]_-_LVL_STATUS_(RO)
[2:0]_-_COLUMN_SIZE_(RW)  */
#define DDR3_CTL_20 0x9050
/**<  R/W 32'h0 [26:24]_-_W2R_SAMECS_DLY_(RW)
[18:16]_-__W2R_DIFFCS_DLY_(RW)
[10:8]_-_TRTP_(RW)
[2:0]_-_TRRD_(RW) */
#define DDR3_CTL_21 0x9054
/**<  R/W 32'h0 [27:24]_-_DRAM_CLASS_(RW)
[19:16]_-__CASLAT_(RW)
[11:8]_-_BURST_ON_FLY_BIT_(RW)
[3:0]_-_APREBIT_(RW)  */
#define DDR3_CTL_22 0x9058
/**<  R/W 32'hD00 [27:24]_-_RDLVL_GATE_DQ_ZERO_COUNT_(RW)
[19:16]_-__RDLVL_ZERO_COUNT_(RW)
[11:8]_-_MAX_COL_REG_(RO)
[3:0]_-_INITAREF_(RW) */
#define DDR3_CTL_23 0x905C
/**<  R/W 32'h0 [27:24]_-_TDFI_RDLVL_EN_(RW)
[19:16]_-__TDFI_WRLAT_BASE_(RW)
[11:8]_-_TDFI_PHY_WRLAT_(RO)
[3:0]_-_TDFI_CTRLUPD_MIN_(RO) */
#define DDR3_CTL_24 0x9060
/**<  R/W 32'h0 [27:24]_-_WRLAT_(RW)
[19:16]_-__TWTR_(RW)
[11:8]_-_TRP_(25)
[3:0]_-_TDFI_WRLVL_EN_(RW)  */
#define DDR3_CTL_25 0x9064
/**<  R/W 32'h10000000  [28:24]_-_MAX_ROW_REG_(RO)
[20:16]_-__CASLAT_LIN_GATE_(RW)
[12:8]_-_CASLAT_LIN_(RW)
[3:0]_-_WRLAT_ADJ(RW) */
#define DDR3_CTL_26 0x9068
/**<  R/W 32'h0 [28:24]_-_TDAL_(RW)
[20:16]_-__RDLAT_ADJ_(RW)
[12:8]_-_OCD_ADJUST_PUP_CS_0_(RW)
[4:0]_-_OCD_ADJUST_PDN_CS_0_(RW)  */
#define DDR3_CTL_27 0x906C
/**<  R/W 32'h0 [28:24]_-_TFAW_(RW)
[20:16]_-__TDFI_RDDATA_EN_BASE_(RW)
[12:8]_-_TDFI_RDDATA_EN_(RO)
[0]_-_TDFI_PHY_RDLAT_(RW) */
#define DDR3_CTL_28 0x9070
/**<  R/W 32'h0 [29:24]_-_ADDR_SPACE_(RW)
[20:16]_-__TWR_INT_(RW)
[12:8]_-_TRC_(RW)
[4:0]_-_TMRD_(RW) */
#define DDR3_CTL_29 0x9074
/**<  R/W 32'h0 [30:24]_-_OUT_OF_RANGE_LENGTH_(RO)
[21:16]_-__WLMRD_(RW)
[13:8]_-_WLDQSEN_(RW)
[5:0]_-_DLL_LOCK_(RO) */
#define DDR3_CTL_30 0x9078
/**<  R/W 32'h0 [31:24]_-_RDLVL_BEGIN_DELAY_0_(RW)
[23:16]_-__ECC_U_SYND_(RO)
[15:8]_-_ECC_C_SYND_(RO)
[7:0]_-_DLL_RST_ADJ_DLY_(RW)  */
#define DDR3_CTL_31 0x907C
/**<  R/W 32'h0 [31:24]_-_RDLVL_BEGIN_DELAY_4_(RW)
[23:16]_-__RDLVL_BEGIN_DELAY_3__(RW)
[16:8]_-_RDLVL_BEGIN_DELAY_2_(RW)
[7:0]_-_RDLVL_BEGIN_DELAY_1_(RW)  */
#define DDR3_CTL_32 0x9080
/**<  R/W 32'h0 [31:24]_-_RDLVL_BEGIN_DELAY_8_(RW)
[23:16]_-__RDLVL_BEGIN_DELAY_7__(RW)
[16:8]_-_RDLVL_BEGIN_DELAY_6_(RW)
[7:0]_-_RDLVL_BEGIN_DELAY_5_(RW)  */
#define DDR3_CTL_33 0x9084
/**<  R/W 32'h0 [31:24]_-_RDLVL_END_DELAY_3_(RW)
[23:16]_-__RDLVL_END_DELAY_2__(RW)
[16:8]_-_RDLVL_END_DELAY_1_(RW)
[7:0]_-_RDLVL_END_DELAY_0_(RW)  */
#define DDR3_CTL_34 0x9088
/**<  R/W 32'h0 [31:24]_-_RDLVL_END_DELAY_7_(RW)
[23:16]_-__RDLVL_END_DELAY_6__(RW)
[16:8]_-_RDLVL_END_DELAY_5_(RW)
[7:0]_-_RDLVL_END_DELAY_4_(RW)  */
#define DDR3_CTL_35 0x908C
/**<  R/W 32'h0 [31:24]_-_RDLVL_GATE_CLK_ADJUST_2_(RW)
[23:16]_-__RDLVL_GATE_CLK_ADJUST_1__(RW)
[16:8]_-_RDLVL_GATE_CLK_ADJUST_0_(RW)
[7:0]_-_RDLVL_BEGIN_DELAY_8_(RW)  */
#define DDR3_CTL_36 0x9090
/**<  R/W 32'h0 [31:24]_-_RDLVL_GATE_CLK_ADJUST_6_(RW)
[23:16]_-__RDLVL_GATE_CLK_ADJUST_5__(RW)
[16:8]_-_RDLVL_GATE_CLK_ADJUST_4_(RW)
[7:0]_-_RDLVL_CLK_ADJUST_3_(RW) */
#define DDR3_CTL_37 0x9094
/**<  R/W 32'h0 [31:24]_-_RDLVL_GATE_DELAY_1_(RW)
[23:16]_-__RDLVL_GATE_DELAY_0__(RW)
[16:8]_-_RDLVL_GATE_CLK_ADJUST_8_(RW)
[7:0]_-_RDLVL_CLK_ADJUST_7_(RW) */
#define DDR3_CTL_38 0x9098
/**<  R/W 32'h0 [31:24]_-_RDLVL_GATE_DELAY_5_(RW)
[23:16]_-__RDLVL_GATE_DELAY_4__(RW)
[16:8]_-_RDLVL_GATE_DELAY_3_(RW)
[7:0]_-_RDLVL_GATE_DELAY_2_(RW) */
#define DDR3_CTL_39 0x909C
/**<  R/W 32'h0 [31:24]_-_RDLVL_GATE_MAX_DELAY_(RW)
[23:16]_-__RDLVL_GATE_DELAY_8__(RW)
[16:8]_-_RDLVL_GATE_DELAY_7_(RW)
[7:0]_-_RDLVL_GATE_DELAY_6_(RW) */
#define DDR3_CTL_40 0x90A0
/**<  R/W 32'h0 [31:24]_-_RDLVL_MIDPOINT_DELAY_2_(RW)
[23:16]_-__RDLVL_MIDPOINT_DELAY_1__(RW)
[16:8]_-_RDLVL_MIDPOINT_DELAY_0_(RW)
[7:0]_-_RDLVL_MAX_DELAY_(RW)  */
#define DDR3_CTL_41 0x90A4
/**<  R/W 32'h0 [31:24]_-_RDLVL_MIDPOINT_DELAY_6_(RW)
[23:16]_-__RDLVL_MIDPOINT_DELAY_5__(RW)
[16:8]_-_RDLVL_MIDPOINT_DELAY_4_(RW)
[7:0]_-_RDLVL_MIDPOINT_DELAY_3(RW)  */
#define DDR3_CTL_42 0x90A8
/**<  R/W 32'h0 [31:24]_-_RDLVL_OFFSET_DELAY_1_(RW)
[23:16]_-__RDLVL_OFFSET_DELAY_0__(RW)
[16:8]_-_RDLVL_MIDPOINT_DELAY_8_(RW)
[7:0]_-_RDLVL_MIDPOINT_DELAY_7_(RW) */
#define DDR3_CTL_43 0x90AC
/**<  R/W 32'h0 [31:24]_-_RDLVL_OFFSET_DELAY_5_(RW)
[23:16]_-__RDLVL_OFFSET_DELAY_4__(RW)
[16:8]_-_RDLVL_OFFSET_DELAY_3_(RW)
[7:0]_-_RDLVL_OFFSET_DELAY_2_(RW) */
#define DDR3_CTL_44 0x90B0
/**<  R/W 32'h0 [31:24]_-_REFRESH_PER_ZQ_(RW)
[23:16]_-__RDLVL_OFFSET_DELAY_8__(RW)
[16:8]_-_RDLVL_OFFSET_DELAY_7_(RW)
[7:0]_-_RDLVL_OFFSET_DELAY_6_(RW) */
#define DDR3_CTL_45 0x90B4
/**<  R/W 32'h0 [31:24]_-_SWLVL_RESP_3_(RW)
[23:16]_-__SWLVL_RESP_2_(RW)
[15:8]_-_SWLVL_RESP_1_(RW)
[7:0]_-_SWLVL_RESP_0_(RW) */
#define DDR3_CTL_46 0x90B8
/**<  R/W 32'h0 [31:24]_-_SWLVL_RESP_7_(RW)
[23:16]_-__SWLVL_RESP_6_(RW)
[15:8]_-_SWLVL_RESP_5_(RW)
[7:0]_-_SWLVL_RESP_4_(RW) */
#define DDR3_CTL_47 0x90BC
/**<  R/W 32'h0 [31:24]_-_TDFI_RDLVL_RESPLAT_(RW)
[23:16]_-__TDFI_RDLVL_RESP_(RW)
[15:8]_-_TDFI_RDLVL_DLL_(RW)
[7:0]_-_SWLVL_RESP_8_(RW) */
#define DDR3_CTL_48 0x90C0
/**<  R/W 32'h0 [31:24]_-_TDFI_WRLVLV_RESPLAT_(RW)
[23:16]_-__TDFI_WRLVL_RESP_(RW)
[15:8]_-_TDFI_WRLVL_DLL_(RW)
[7:0]_-_TDFI_RDLVL_RR_(RW)  */
#define DDR3_CTL_49 0x90C4
/**<  R/W 32'h0 [31:24]_-_TRCD_INT_(RW)
[23:16]_-__TRAS_MIN_(RW)
[15:8]_-_TMOD_(WR)
[7:0]_-_TDFI_WRLVL_WW_(RW)  */
#define DDR3_CTL_50 0x90C8
/**<  R/W 32'h0 [19:8]_-_ZQCS_(RW)
[7:0]_-_TRFC_(RW) */
#define DDR3_CTL_51 0x90CC
/**<  R/W 32'h0 [29:16]_-_TDFI_CTRLUPD_MAX_(RO)
[11:0]_-__ZQINI_(RW)  */
#define DDR3_CTL_52 0x90D0
/**<  R/W 32'h0 [29:16]_-__TDFI_PHYUPD_TYPE0_(RO)
_[7:0]_-_TDFI_PHYUPD_RESP_(RO)  */
#define DDR3_CTL_53 0x90D4
/**<  R/W 32'h0 [31:16]_-_INT_ACK_(WR)
[13:0]_-_TREF_(RW)  */
#define DDR3_CTL_54 0x90D8
/**<  R/W 32'h0 [31:16]_-_EMRS1_DATA_0_(RW)
[15:0]_-_DLL_RST_DELAY_(RW) */
#define DDR3_CTL_55 0x90DC
/**<  R/W 32'h0 [31:16]_-_EMRS2_DATA_0_(RW)
[15:0]_-_EMRS1_DATA_1_(RW)  */
#define DDR3_CTL_56 0x90E0
/**<  R/W 32'h0 [31:16]_-_EMRS3_DATA_0_(RW)
[15:0]_-_EMRS2_DATA_1_(RW)  */
#define DDR3_CTL_57 0x90E4
/**<  R/W 32'h0 _[15:0]_-_EMRS3_DATA_1_(RW) */
#define DDR3_CTL_58 0x90E8
/**<  R/W 32'h0 [31:16]_-_MRS_DATA_0_(RW)
  */
#define DDR3_CTL_59 0x90EC
/**<  R/W 32'h0 [31:16]_-_REFRESH_PER_RDLVL_(RW)
[15:0]_-_MRS_DATA_1_(RW)  */
#define DDR3_CTL_60 0x90F0
/**<  R/W 32'h0 [31:16]_-_TCPD_(RW)
[15:0]_-_REFRESH_PER_RDLVL_GATE_(RW)  */
#define DDR3_CTL_61 0x90F4
/**<  R/W 32'h0 [31:16]_-_TDFI_WRLVL_MAX_(RW)
[15:0]_-_TDFI_RDLVL_MAX_(RW)  */
#define DDR3_CTL_62 0x90F8
/**<  R/W 32'h0 [31:16]_-_TPDEX_(RW)
[15:0]_-_TDLL_(RW)  */
#define DDR3_CTL_63 0x90FC
/**<  R/W 32'h0 [31:16]_-_TXPDLL_(RW)
[15:0]_-_TRAS_MAX_(RW)  */
#define DDR3_CTL_64 0x9100
/**<  R/W 32'h0 [31:16]_-_TXSR_(RW)
[15:0]_-_TXSNR_(RW) */
#define DDR3_CTL_65 0x9104
/**<  R/W 32'h0 [31:16]_-_WRLVL_INTERVAL_(RW)
[15:0]_-_VERSION_(RO) */
#define DDR3_CTL_66 0x9108  /**<  R/W 32'h0 [17:0]_-_WRLVL_STATUS_(RO)  */
#define DDR3_CTL_67 0x910C  /**<  R/W 32'h0 [23:0]_-_TINIT_(RW) */
#define DDR3_CTL_68 0x9110  /**<  R/W 32'h0 [31:0]_-_CKE_INACTIVE_(RW)  */
#define DDR3_CTL_69 0x9114  /**<  R/W 32'h0 [7:0]_-_DFI_WRLVL_MAX_DELAY_(RW)  */
#define DDR3_CTL_70 0x9118  /**<  R/W 32'h0 [7:0]_-_DFT_CTRL_REG_(RW) */
#define DDR3_CTL_71 0x911C  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_0_(RW)  */
#define DDR3_CTL_72 0x9120  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_1_(RW)  */
#define DDR3_CTL_73 0x9124  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_2_(RW)  */
#define DDR3_CTL_74 0x9128  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_3_(RW)  */
#define DDR3_CTL_75 0x912C  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_4_(RW)  */
#define DDR3_CTL_76 0x9130  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_5_(RW)  */
#define DDR3_CTL_77 0x9134  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_6_(RW)  */
#define DDR3_CTL_78 0x9138  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_7_(RW)  */
#define DDR3_CTL_79 0x913C  /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_0_8_(RW)  */
#define DDR3_CTL_80 0x9140  /**<  R/W 32'h0 [31:0]_-_PAD_CTRL_REG_0_(RW)  */
#define DDR3_CTL_81 0x9144  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_0_(RW)  */
#define DDR3_CTL_82 0x9148  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_1_(RW)  */
#define DDR3_CTL_83 0x914C  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_2_(RW)  */
#define DDR3_CTL_84 0x9150  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_3_(RW)  */
#define DDR3_CTL_85 0x9154  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_4_(RW)  */
#define DDR3_CTL_86 0x9158  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_5_(RW)  */
#define DDR3_CTL_87 0x915C  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_6_(RW)  */
#define DDR3_CTL_88 0x9160  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_7_(RW)  */
#define DDR3_CTL_89 0x9164  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_0_8_(RW)  */
#define DDR3_CTL_90 0x9168  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_0_(RW)  */
#define DDR3_CTL_91 0x916C  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_1_(RW)  */
#define DDR3_CTL_92 0x9170  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_2_(RW)  */
#define DDR3_CTL_93 0x9174  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_3_(RW)  */
#define DDR3_CTL_94 0x9178  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_4_(RW)  */
#define DDR3_CTL_95 0x917C  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_5_(RW)  */
#define DDR3_CTL_96 0x9180  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_6_(RW)  */
#define DDR3_CTL_97 0x9184  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_7_(RW)  */
#define DDR3_CTL_98 0x9188  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_1_8_(RW)  */
#define DDR3_CTL_99 0x918C  /**<  R/W 32'h0 [31:0]_-_PHY_CTRL_REG_2_(RW)  */
#define DDR3_CTL_100  0x9190  /**<  R/W 32'h0 [31:0]_-_TRST_PWRON_(RW)  */
#define DDR3_CTL_101  0x9194  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_0_(RW)  */
#define DDR3_CTL_102  0x9198  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_1_(RW)  */
#define DDR3_CTL_103  0x919C  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_2_(RW)  */
#define DDR3_CTL_104  0x91A0  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_3_(RW)  */
#define DDR3_CTL_105  0x91A4  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_4_(RW)  */
#define DDR3_CTL_106  0x91A8  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_5_(RW)  */
#define DDR3_CTL_107  0x91AC  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_6_(RW)  */
#define DDR3_CTL_108  0x91B0  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_7_(RW)  */
#define DDR3_CTL_109  0x91B4  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_0_(RO) */
#define DDR3_CTL_110  0x91B8  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_1_(RO) */
#define DDR3_CTL_111  0x91BC  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_2_(RO) */
#define DDR3_CTL_112  0x91C0  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_3_(RO) */
#define DDR3_CTL_113  0x91C4  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_4_(RO) */
#define DDR3_CTL_114  0x91C8  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_5_(RO) */
#define DDR3_CTL_115  0x91CC  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_6_(RO) */
#define DDR3_CTL_116  0x91D0  /**<  R/W 32'h0 [31:0]_-_USER_DEF_REG_RO_7_(RO) */
#define DDR3_CTL_117  0x91D4  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_0_(RW)  */
#define DDR3_CTL_118  0x91D8  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_1_(RW)  */
#define DDR3_CTL_119  0x91DC  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_2_(RW)  */
#define DDR3_CTL_120  0x91E0  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_3_(RW)  */
#define DDR3_CTL_121  0x91E4  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_4_(RW)  */
#define DDR3_CTL_122  0x91E8  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_5_(RW)  */
#define DDR3_CTL_123  0x91EC  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_6_(RW)  */
#define DDR3_CTL_124  0x91F0  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_7_(RW)  */
#define DDR3_CTL_125  0x91F4  /**<  R/W 32'h0 [7:0]_-_WRLVL_DELAY_8_(RW)  */
#define DDR3_CTL_126  0x91F8  /**<  R/W 32'h0 [31:0]_-_XOR_CHECK_BITS_(RW)  */
#define DDR3_CTL_127  0x91FC  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_ADDR[31:0]_(RO)  */
#define DDR3_CTL_128  0x9200  /**<  RO  32'h0 [3:0]_-_BIST_FAIL_ADDR[35:32]_(RO)  */
#define DDR3_CTL_129  0x9204  /**<  R/W 32'h0 [31:0]_-_BIST_START_ADDR[31:0]_(RW) */
#define DDR3_CTL_130  0x9208  /**<  R/W 32'h0 [3:0]_-_BIST_START_ADDR[35:32]_(RW) */
#define DDR3_CTL_131  0x920C  /**<  RO  32'h0 [31:0]_-_ECC_C_ADDR[31:0]_(RO)  */
#define DDR3_CTL_132  0x9210  /**<  RO  32'h0 [3:0]_-_ECC_C_ADDR[35:32]_(RO)  */
#define DDR3_CTL_133  0x9214  /**<  RO  32'h0 [31:0]_-_ECC_U_ADDR[31:0]_(RO)  */
#define DDR3_CTL_134  0x9218  /**<  RO  32'h0 [3:0]_-_ECC_U_ADDR[35:32]_(RO)  */
#define DDR3_CTL_135  0x921C  /**<  RO  32'h0 [31:0]_-_OUT_OF_RANGE_ADDR[31:0]_(RO) */
#define DDR3_CTL_136  0x9220  /**<  RO  32'h0 [3:0]_-_OUT_OF_RANGE_ADDR[35:32]_(RO) */
#define DDR3_CTL_137  0x9224  /**<  RO  32'h0 [31:0]_-_RDLVL_ERROR_STATUS[31:0]_(RO)  */
#define DDR3_CTL_138  0x9228  /**<  RO  32'h0 [5:0]_-_RDLVL_ERROR_STATUS[37:32]_(RO)  */
#define DDR3_CTL_139  0x922C  /**<  RO  32'h0 [31:0]_-_ECC_C_DATA[31:0]_(RO)  */
#define DDR3_CTL_140  0x9230  /**<  RO  32'h0 [31:0]_-_ECC_C_DATA[63:32]_(RO) */
#define DDR3_CTL_141  0x9234  /**<  RO  32'h0 [31:0]_-_ECC_U_DATA[31:0]_(RO)  */
#define DDR3_CTL_142  0x9238  /**<  RO  32'h0 [31:0]_-_ECC_U_DATA[63:32]_(RO) */
#define DDR3_CTL_143  0x923C  /**<  RW  32'h0 [31:0]_-_RDLVL_GATE_RESP_MASK[31:0]_(RW)  */
#define DDR3_CTL_144  0x9240  /**<  RW  32'h0 [31:0]_-_RDLVL_GATE_RESP_MASK[63:32]_(RW) */
#define DDR3_CTL_145  0x9244  /**<  RW  32'h0 [7:0]_-_RDLVL_GATE_RESP_MASK[71:64]_(RW)  */
#define DDR3_CTL_146  0x9248  /**<  RW  32'h0 [31:0]_-_RDLVL_RESP_MASK[31:0]_(RW) */
#define DDR3_CTL_147  0x924C  /**<  RW  32'h0 [31:0]_-_RDLVL_RESP_MASK[63:32]_(RW)  */
#define DDR3_CTL_148  0x9250  /**<  RW  32'h0 [7:0]_-_RDLVL_RESP_MASK[71:64]_(RW) */
#define DDR3_CTL_149  0x9254  /**<  RW  32'h0 [31:0]_-_BIST_DATA_MASK[31:0]_(RW)  */
#define DDR3_CTL_150  0x9258  /**<  RW  32'h0 [31:0]_-_BIST_DATA_MASK[63:32]_(RW) */
#define DDR3_CTL_151  0x925C  /**<  RW  32'h0 [31:0]_-_BIST_DATA_MASK_[95:64]_(RW)  */
#define DDR3_CTL_152  0x9260  /**<  RW  32'h0 [31:0]_-_BIST_DATA_MASK_[127:96]_(RW) */
#define DDR3_CTL_153  0x9264  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA[31:0]_(RO) */
#define DDR3_CTL_154  0x9268  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA[63:32]_(RO)  */
#define DDR3_CTL_155  0x926C  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA_[95:64]_(RO) */
#define DDR3_CTL_156  0x9270  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA_[127:96]_(RO)  */
#define DDR3_CTL_157  0x9274  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA[159:128]_(RO)  */
#define DDR3_CTL_158  0x9278  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA[191:160]_(RO)  */
#define DDR3_CTL_159  0x927C  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA_[223:192]_(RO) */
#define DDR3_CTL_160  0x9280  /**<  RO  32'h0 [31:0]_-_BIST_EXP_DATA_[255:224]_(RO) */
#define DDR3_CTL_161  0x9284  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA[31:0]_(RO)  */
#define DDR3_CTL_162  0x9288  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA[63:32]_(RO) */
#define DDR3_CTL_163  0x928C  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA_[95:64]_(RO)  */
#define DDR3_CTL_164  0x9290  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA_[127:96]_(RO) */
#define DDR3_CTL_165  0x9294  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA[159:128]_(RO) */
#define DDR3_CTL_166  0x9298  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA[191:160]_(RO) */
#define DDR3_CTL_167  0x929C  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA_[223:192]_(RO)  */
#define DDR3_CTL_168  0x92A0  /**<  RO  32'h0 [31:0]_-_BIST_FAIL_DATA_[255:224]_(RO)  */
#define DDR3_CTL_169  0x92A4
/**<  RW  32'h0 [25:24]_-_DLL_OBS_REG0_3_(RW)
[17:16]_-_DLL_OBS_REG0_2_(RW)_
[9:8]_-_DLL_OBS_REG0_1_(RW)
[1:0]_-_DLL_OBS_REG0_0_(RW) */
#define DDR3_CTL_170  0x92A8
/**<  RW  32'h0 [25:24]_-_DLL_OBS_REG0_7_(RW)
[17:16]_-_DLL_OBS_REG0_6_(RW)_
[9:8]_-_DLL_OBS_REG0_5_(RW)
[1:0]_-_DLL_OBS_REG0_4_(RW) */
#define DDR3_CTL_171  0x92AC
/**<  RW  32'h0 [14:8]_-_PHY_OBS_REG_2_(RW)
[1:0]_-_DLL_OBS_REG0_8_(RW) */
#define DDR3_CTL_172  0x92B0  /**<  RW  32'h0 [16:0]_-_INT_MASK_(RW)  */
#define DDR3_CTL_173  0x92B4  /**<  RO  32'h0 [16:0]_-_INT_STATUS_(RO)  */
#define DDR3_CTL_174  0x92B8  /**<  RW  32'h0 [23:0]_-_PAD_CTRL_REG_1_(RW)  */
#define DDR3_CTL_175  0x92BC  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_0_(RW)  */
#define DDR3_CTL_176  0x92C0  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_1_(RW)  */
#define DDR3_CTL_177  0x92C4  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_2_(RW)  */
#define DDR3_CTL_178  0x92C8  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_3_(RW)  */
#define DDR3_CTL_179  0x92CC  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_4_(RW)  */
#define DDR3_CTL_180  0x92D0  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_5_(RW)  */
#define DDR3_CTL_181  0x92D4  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_6_(RW)  */
#define DDR3_CTL_182  0x92D8  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_7_(RW)  */
#define DDR3_CTL_183  0x92DC  /**<  RW  32'h0 [31:0]_-_DLL_CTRL_REG_1_8_(RW)  */
#define DDR3_CTL_184  0x92E0  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_0_(RW) */
#define DDR3_CTL_185  0x92E4  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_1_(RW) */
#define DDR3_CTL_186  0x92E8  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_2_(RW) */
#define DDR3_CTL_187  0x92EC  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_3_(RW) */
#define DDR3_CTL_188  0x92D0  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_4_(RW) */
#define DDR3_CTL_189  0x92D4  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_5_(RW) */
#define DDR3_CTL_190  0x92D8  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_6_(RW) */
#define DDR3_CTL_191  0x92DC  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_7_(RW) */
#define DDR3_CTL_192  0x92E0  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_0_8_(RW) */
#define DDR3_CTL_193  0x92E4  /**<  RW  32'h0 [31:0]_-_PHY_OBS_REG_1_(RW) */
#define DDR3_CTL_194  0x9308 /**<  R/W 32'h0 [31:0]_-_DLL_CTRL_REG_2[31:0] */
#define DDR3_CTL_195  0x930C /**<  R/W 32'h0 [0]_-_DLL_CTRL_REG_2[32]  */
#define DDR3_CTL_213  0x9354
#define DDR3_CTL_236  0x93B0


/*  XAUI_DMA_REGISTER_DETAILS           */

#define X0_DMA_CONFIG 0xA400
/**<  R/W 32'h0 [0]_-_ENABLE_XAUI_DMA
[1]_-_ETYPE_COMPARE_DISABLE
[2]_-_PAUSE_PROCESS_DISABLE
[31:3]_-_reserved */
#define X0_RTYPE  0xA404
/**<  R/W 32'hfffc
[15:0]_-_RTYPE[15:0].__This_value_is_compared_with_the_Ethernet_type_of_the_incoming_packet.
__If_there_is_a_match,_and_the_frame_is_valid,_the_packet_gets_forwarded_to_the_engine_for_scan.  */
#define X0_TTYPE  0xA408
/**<  R/W 32'hfffe_fffd
[15:0]_-_TTYPE[15:0].__This_is_the_Ethernet_Type_field_used_for_packets_that_have_been_scanned.
[31:16]_-_ETYPE.
__This_is_the_Ethernet_Type_field_use_for_incoming_packets_that_were_marked_with_errors_from_the_MAC
 */
#define X0_MEM_CORR0  0xA420
/**<  R/W 32'h0
[31:0]_-_XORed_with_write_data_[31:0]_to_insert_ECC_corruption  */
#define X0_MEM_CORR1  0xA424
/**<  R/W 32'h0
[6:0]_-_XORed_with_ecc_and_parity_bits_to_insert_ECC_corruption
[30:7]_-_reserved
[31]_-_DISABLE_ECC_CHECK0 */
#define X0_MEM_CORR2  0xA428
/**<  R/W 32'h0
[31:0]_-_XORed_with_write_data_[63:32]_to_insert_ECC_corruption */
#define X0_MEM_CORR3  0xA42c
/**<  R/W 32'h0
[6:0]_-_XORed_with_ecc_and_parity_bits_to_insert_ECC_corruption
[30:7]_-_reserved
[31]_-_DISABLE_ECC_CHECK1 */
#define X0_MEM_CORR4  0xA430
/**<  R/W 32'h0
[31:0]_-_XORed_with_write_data_[95:64]_to_insert_ECC_corruption */
#define X0_MEM_CORR5  0xA434
/**<  R/W 32'h0
[6:0]_-_XORed_with_ecc_and_parity_bits_to_insert_ECC_corruption
[30:7]_-_reserved
[31]_-_DISABLE_ECC_CHECK2 */
#define X0_MEM_CORR6  0xA438
/**<  R/W 32'h0
[31:0]_-_XORed_with_write_data_[127:96]_to_insert_ECC_corruption  */
#define X0_MEM_CORR7  0xA43c
/**<  R/W 32'h0
[6:0]_-_XORed_with_ecc_and_parity_bits_to_insert_ECC_corruption
[30:7]_-_reserved
[31]_-_DISABLE_ECC_CHECK3 */
#define X0_MEM_ERROR  0xA440
/**<  RUM 32'h0 [0]_-_SINGLE_BIT_ERR_DETECTED
[1]_-_MULTI_BIT_ERROR_DETECTED
[31:2]_-_reserved */
#define X0_MEM_ERROR_HOLD 0xA444
/**<  RO  32'h0 [12:0]_-_FAILED_ADDR
[29:13]_-_reserved
[30]_-_SINGLE_BIT_ERR
[31]_-_MULTI_BIT_ERR  */
#define X0_BYPASS_TEST0 0xA460  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[31:0]  */
#define X0_BYPASS_TEST1 0xA464  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[63:32] */
#define X0_BYPASS_TEST2 0xA468  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[95:64] */
#define X0_BYPASS_TEST3 0xA46c  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[127:96]  */
#define X0_BYPASS_TEST4 0xA470  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[159:128] */
#define X0_BYPASS_TEST5 0xA474
/**<  R/W 32'h0 [1:0]_-_BYPASS_TESTDATA[161:160]
[31:2]_-_reserved */
#define X0_BYPASS_CONFIG  0xA478
/**<  R/W 32'h0 [0]_-_BYPASS_TEST_REQ
[1]_-_BYPASS_TEST_WRT_(1-write_0-read)
[30:2]_-_reserved
[31]_-_BYPASS_TEST_ENABLE */
#define X0_BYPASS_STATUS  0xA47c
/**<  RO  NA  [0]_-_BYPASS_WRT_FIFO_FULL
[1]_-_BYPASS_RD_FIFO_EMPTY
[31:2]_-_reserved */
#define X0_FREE_CONFIG  0xA500
/**<  R/W 32'h1
[0]_-_ENABLE.__When_this_bit_is_a_0,_XAUI_Free_pool_is_held_in_reset._Default_value_is_1.
[31:1]_-_reserved */
#define X0_FREE_THRESH  0xA504
/**<  R/W 32'h30
[8:0]_-_RD_RESERVED_BUFF.__This_is_the_number_of_buffers_reserved_for_the_result_ring.
__This_is_to_avoid_any_resource_deadlock.__Default_value_is_0x30_(48_buffers).
[31:9]_-_reserved */
#define X0_TEST 0xA508
/**<  R/W 32'h0 [0]_-_FREE_POOL_REQ
[1]_-_PUSH_(1_-_push_to_pool,_0_-_pop_from_pool
[15:2]_-_reserved
[24:16]_-_FREE_POOL_ADDR[9:0]
[31:25]_-_reserved  */
#define X0_FREE_LEVEL 0xA50C
/**<  RO  32'h1ff
[8:0]_-_FREE_BUFFER_LEVEL[8:0]._Max_is_511_buffers_(0x1ff)
[31:9]_-_reserved */
#define X0_PAUSE_DA0  0xA600  /**<  R/W 32'hc2000001  [31:0]_-_PAUSE_DA[31:0] */
#define X0_PAUSE_DA1  0xA604
/**<  R/W 32'h0180  [15:0]_-_PAUSE_DA[47:32]
[31:16]_-_reserved  */
#define X0_PAUSE_SA0  0xA608  /**<  R/W 32'h81828300  [31:0]_-_PAUSE_SA[31:0] */
#define X0_PAUSE_SA1  0xA60C
/**<  R/W 32'h0080  [15:0]_-_PAUSE_SA[47:32]
[31:16]_-_reserved  */
#define X0_PAUSE_QUANTA 0xA610
/**<  R/W 32'h0008_0040 [15:0]_-_PAUSE_QUANTA.
[31:16]_-_XOFF_GEN_EARLY_RELEASE  */
#define X0_PAUSE_THRESH 0xA614
/**<  R/W 32'h00b400aa
[8:0]_-_XOFF_THRESHHOLD[8:0].__If_the_FREE_BUFFER_LEVEL_is_less_than_or_equal_to_this_value,
_an_XOFF_is_issued.
[15:9]_-_reserved
[24:16]_-_XON_THRESHHOLD[8:0].__If_the_FREE_BUFFER_LEVEL_is_more_than_this_value,
_and_a_previous_XOFF_has_not_fully_expired,_an_XON_is_issued.
[31:25]_-_reserved  */
#define X0_PAUSE_CONFIG 0xA618
/**<  R/W 32'h0100
[0]_-_CPU_INSERT_XOFF_(hardware_resets_when_issued)
[1]_-_CPU_INSERT_XON_(hardware_resets_when_issued)
[7:2]_-_reserved
[8]_-_PAUSE_ENABLE._(1_-_allows_PAUSE_engine_to_function)
[9]_-_DISABLE_HARDWARE_XOFF
[10]_-_DISABLE_HARDWARE_XON
[11]_-_DISABLE_MAX_RX_PAUSE
[12]_-_MAC_XOFF_GEN_(rising_edge)
[13]_-_MAC_XON_GEN_(rising_edge)
[31:14]_-_reserved  */

#define X1_MEM_ERROR  0xB440
/**<  RUM 32'h0 [0]_-_SINGLE_BIT_ERR_DETECTED
[1]_-_MULTI_BIT_ERROR_DETECTED
[31:2]_-_reserved */
#define X1_MEM_ERROR_HOLD 0xB444
/**<  RO  32'h0 [12:0]_-_FAILED_ADDR
[29:13]_-_reserved
[30]_-_SINGLE_BIT_ERR
[31]_-_MULTI_BIT_ERR  */

#define X1_BYPASS_TEST0 0xB460  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[31:0]  */
#define X1_BYPASS_TEST1 0xB464  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[63:32] */
#define X1_BYPASS_TEST2 0xB468  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[95:64] */
#define X1_BYPASS_TEST3 0xB46c  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[127:96]  */
#define X1_BYPASS_TEST4 0xB470  /**<  R/W 32'h0 [31:0]_-_BYPASS_TESTDATA[159:128] */
#define X1_BYPASS_TEST5 0xB474
/**<  R/W 32'h0 [1:0]_-_BYPASS_TESTDATA[161:160]
[31:2]_-_reserved */
#define X1_BYPASS_CONFIG  0xB478
/**<  R/W 32'h0 [0]_-_BYPASS_TEST_REQ
[1]_-_BYPASS_TEST_WRT_(1-write_0-read)
[30:2]_-_reserved
[31]_-_BYPASS_TEST_ENABLE */
#define X1_BYPASS_STATUS  0xB47c
/**<  RO  NA  [0]_-_BYPASS_WRT_FIFO_FULL
[1]_-_BYPASS_RD_FIFO_EMPTY
[31:2]_-_reserved */

/*  XAUI_MAC_REGISTER_DETAILS           */

#define X0_REVISION 0xA000
/**<  RO  32'h1003b 7:0:_Core_Revision._
15:8:_Core_Version._
31:16:_Programmable_Customer_Revision */
#define X0_SCRATCH  0xA004
/**<  R/W 32'h0
The_Scratch_Register_provides_a_memory_location_to_test_the_register_access */
#define X0_COMMAND_CONFIG_ 0xA008
/**<  R/W 32'h40  Control/Configuration_of_the_core._
[0]_:_TX_ENA_:_MAC_Transmit_Path_Enable._Should_be_set_to_'1'_to_enable_the_MAC_transmit_path,
_should_be_set_to_'0'_(Reset_value)_to_disable_the_MAC_transmit_path.
[1]_:_RX_ENA_:_MAC_Receive_Path_Enable._Should_be_set_to_'1'_to_enable_the_MAC_receive_path,
_should_be_set_to_'0'_(Reset_value)_to_disable_the_MAC_receive_path.
[3]_:_WAN_MODE_:_WAN_Mode_Enable._Sets_WAN_mode_(1)_or_LAN_mode_(0,_default)_of_operation._
Note:_When_changing_the_mode,_verify_correct_setting_of_the_transmit_Inter-Packet-Gap_(IPG)
_in_register_TX_IPG_LENGTH._

[4]_:_PROMIS_EN:_Enables_MAC_promiscuous_operation._When_set_to_'1',
_all_frames_are_received_without_any_MAC_address_filtering,_when_set_to_'0'_(Reset_value)
_Unicast_Frames_with_a_destination_address_not_matching_the_Core_MAC_Address_(
Programmed_in_Registers_MAC_ADDR_0_and_MAC_ADDR_1)_are_rejected.

[5]_:_PAD_EN:_Enable_Frame_Padding_Removal_in_receive_path._If_set_to_'1',
_padding_is_removed_before_the_frame_is_conveyed_to_the_MAC_Client_application._If_set_to_'0'_(
Reset_value)_no_padding_is_removed_on_receive_by_the_MAC._
[6]_:_CRC_FWD:_Terminate_/_Forward_Received_CRC.
_If_set_to_'1'_the_CRC_field_of_received_frames_is_forwarded_with_the_frame_to_the_user_application.
_If_set_to_'1'_(Reset_value)_the_CRC_field_is_stripped_from_the_frame._
Note:_If_padding_(Bit_PAD_EN_set_to_'1')_is_enabled,_CRC_FWD_is_ignored._
[7]_:_PAUSE_FWD:_Terminate_/_Forward_Pause_Frames.
_If_set_to_'1'_pause_frames_are_forwarded_to_the_user_application._When_set_to_'0'_(Reset_value)
_pause_frames_are_terminated_and_discarded_within_the_MAC.

[8]_:_PAUSE_IGNORE:_Ignore_Pause_Frame_Quanta.
_If_set_to_'1'_received_pause_frames_are_ignored_by_the_MAC._When_set_to_'0'_(Reset_value)
_the_transmit_process_is_stopped_for_the_amount_of_time_specified_in_the_pause_quanta_received_withi
n_a_pause_frame.

[9]_:_TX_ADDR_INS:_Set_Source_MAC_Address_on_Transmit.
_If_set_to_'1'_the_MAC_overwrites_the_source_MAC_address_received_from_the_Client_Interface_with_the
_MAC_address_(Programmed_in_Registers_MAC_ADDR_0_and_MAC_ADDR_1)_._When_set_to_'0'_(Reset_value),
_the_source_MAC_address_from_the_Client_Interface_is_transmitted_unmodified_to_the_line.

[10]_:_LOOPBACK_EN:_PHY_Interface_Loopback._When_set_to_'1',_the_signal_loop_ena_is_set_to_'1',
_when_set_to_'0'_(Reset_value)_the_signal_loop_ena_is_set_to_'0'.

[12]_:_SW_RESET:_Self-Clearing_Software_Reset._When_with_'1'_all_Statistic_Counters_are_reset_to_0.

[13]_:_CMD_FRAME_ENA:_Enables_reception_of_all_command_frames.
_When_set_to_'1'_all_Command_Frames_are_accepted,_when_set_to_'0'_(Reset_Value)
_only_Pause_Frames_are_accepted_and_all_other_Command_Frames_are_rejected.

[14]_:_RX_ERR_DISC:_Receive_Errored_Frame_Discard_Enable._When_set_to_'1',
_any_frame_received_with_an_error_is_discarded_in_the_Core_and_not_forwarded_to_the_Client_interface
._When_set_to_'0'_(Reset_value),
_errored_Frames_are_forwarded_to_the_Client_interface_with_ff_rx_err_asserted._
Note:
_It_is_recommended_to_set_RX_ERR_DISC_to_'1'_only_when_Store_and_Forward_operation_is_enabled_on_the
_Core_Receive_FIFO_(Register_RX_SECTION_FULL_set_to_0).

[15]_:_PHY_TXENA:_PHY_Transmit_Enable._When_set_to_'1',_the_signal_phy_txena_is_set_to_'1',
_when_set_to_'0'_(Reset_value)_the_signal_phy_txena_is_set_to_'0'.

[16]_:_SEND_IDLE:_Force_Idle_Generation._When_set_to_'1',
_the_MAC_permanently_sends_XGMII_Idle_sequences_even_when_faults_are_received.

[17]_:_NO_LGTH_CHECK:_Payload_Length_Check_Disable._When_set_to_'0'_(Reset_value),
_the_Core_checks_the_frame's_payload_length_with_the_Frame_Length/Type_field,_when_set_to_'1',
_the_payload_length_check_is_disabled.
  */
#define X0_MAC_ADDR_0_  0xA00C
/**<  R/W 32'h0
The_lower_32-bit_of_the_48-Bit_MAC_Address._Bit_0_is_LSB._  */
#define X0_MAC_ADDR_1_  0xA010
/**<  R/W 32'h0
The_upper_16-bit_of_the_48-Bit_MAC_Address._Bit_0_is_Bit_32_of_MAC_address._
Bits_15_to_32_are_unused_and_always_set_to_'0'. */
#define X0_FRM_LENGTH 0xA014
/**<  R/W 32'h600 Maximum_supported_frame_length._
The_MAC_supports_any_frame_size_up_to_16352_bytes_(0x3fe0)._Typical_settings_are_1518_for_standard.
_Set_to_1536_after_Reset._
Bits_15_to_32_are_unused_and_always_set_to_'0'._  */
#define X0_PAUSE_QUANT_ 0xA018
/**<  R/W 32'h0
Pause_Quanta_value_used_with_transmitted_pause_frames._Each_Quanta_specifies_a_512_bit-time._Bits_31
:16_are_unused_and_always_set_to_'0'._  */
#define X0_RX_FIFO_SECTIONS 0xA01C
/**<  R/W 32'h10_ 15:_0:_RX_section_available_threshold,_reset_value_is_0x10._
31:16:_RX_section_empty_threshold,_reset_value_is_0x00._
All_threshold_values_are_in_steps_of_FIFO_words_(8_byte)._
See_12_page_48. */
#define X0_TX_FIFO_SECTIONS  0xA020
/**<  R/W _32'h10 15:_0:_TX_section_available_threshold,_reset_value_is_0x10._
31:16:_TX_section_empty_threshold,_reset_value_is_0x0._
All_threshold_values_are_in_steps_of_FIFO_words_(8_byte)._
See_12_page_48.
  */
#define X0_RX_FIFO_ALMOST_F_E 0xA024
/**<  RO  _32'h80008  15:0:_RX_FIFO_almost_empty_threshold_
31:16:_RX_FIFO_almost_full_threshold._
Read_Only_Fixed_thresholds_set_with_a_Core_configuration_parameter_(
See_User_Guide_document_for_details).__ */
#define X0_TX_FIFO_ALMOST_F_E_  0xA028
/**<  RO  32'h80008 15:0:_TX_FIFO_almost_empty_threshold_
31:16:_TX_FIFO_almost_full_threshold_
Read_Only_Fixed_thresholds_set_with_a_Core_configuration_parameter_(
See_User_Guide_document_for_details). */
#define X0_HASHTABLE_LOAD_  0xA02C
/**<  WO  32'h0 Hash_table_programming._Write_only_register._
Bits_5:0_specify_the_hash_table_address_(code)_
Bit_8_enables_(1)_or_disables_(0)_multicast_frame_reception_for_the_entry._
See_Table_8_page_38.  */
#define X0_MDIO_STATUS_ 0xA030
/**<  RO  32'h0 MDIO_Status_
[0]:_MDIO_busy._If_set_a_MDIO_transaction_is_currently_ongoing.
_If_cleared_the_application_can_access_the_other_registers._
[1]:_MDIO_read_error._If_set,
_the_last_read_transaction_had_no_response_from_a_PHY_and_the_data_read_could_be_invalid.
_This_can_happen,_if_the_PHY_address_does_not_match_any_PHY_that_is_available_on_the_MDIO_bus.  */
#define X0_MDIO_COMMAND_  0xA034
/**<  R/W 32'h0 MDIO_Command_(Phy_and_Port_Address)_
[4:0]:_Device_address_
[9:5]:_Port_Address_
[14]:_If_written_with_1,_a_read_with_address_post-increment_will_be_performed._Post-
Increment_will_be_performed_in_the_PHY_internal_address_register._
[15]:_If_written_with_1,_a_normal_read_transaction_is_initiated.  */
#define X0_MDIO_DATA_ 0xA038
/**<  R/W 32'h0 MDIO_Data_to_write_and_last_Data_read_
[15:0]_16_bit_data_word_
When_written:_Initiates_a_write_transaction_to_the_PHY.
_The_COMMAND_register_must_have_been_initialized.
_The_busy_status_bit_will_be_set_immediately_and_cleared_when_the_write_transaction_has_finished._
When_read:_Returns_the_data_read_from_the_PHY_register_after_a_read_transaction_has_been_completed_(
initiated_by_writing_a_1_to_Bit_15_or_Bit_14_of_the_COMMAND_register)._
Bit_31_of_the_register_is_the_busy_bit_as_found_in_MDIO_STATUS.0.
_The_value_read_is_invalid_as_long_as_Bit_31_is_set. */
#define X0_MDIO_REGADDR_  0xA03C
/**<  WO  32'h0 MDIO_Register_Address_
Address_of_register_within_the_PHY_device_to_read_from_or_write_to.
_After_writing_this_register_an_address-
write_transaction_will_be_initiated_to_set_the_PHYs_internal_address_register_to_the_value_given._
The_COMMAND_register_must_have_been_initialized_before_the_first_write_to_this_register */
#define X0_STATUS_  0xA040
/**<  ROR
General_Purpose_Status._Use_to_monitor_the_MAC_and_PHY_interface._
[0]_RX_LOC_FAULT:_Latch-High_Local_Fault_Status.
_Set_to_'1'_when_the_MAC_detects_RX_Local_Fault_Sequences_on_the_XGMII_receive_interface.
_Reset_to_'0_after_read_and_after_reset.
[1]_RX_REM_FAULT:_Latch-High_Remote_Fault_Status.
_Set_to_'1'_when_the_MAC_detects_RX_Remote_Fault_Sequences_on_the_XGMII_receive_interface.
_Reset_to_'0_after_read_and_after_reset.
[2]_PHY_STAT_0:_Latch_Low_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(0)
_transitions_to_'1'_after_reset_or_after_read._Set_to_'0'_when_the_when_the_signal_phy_status(0)
_transitions_to_'0._Used,_when_the_XAUI_interface_reference_design_is_implemented,
_to_monitor_the_status_of_the_XGXS_PLL._

[3]_PHY_STAT_1:_Latch_Low_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(1)
_transitions_to_'1'_after_reset_or_after_read._Set_to_'0'_when_the_when_the_signal_phy_status(1)
_transitions_to_'0._Used,_when_the_XAUI_interface_reference_design_is_implemented,
_to_monitor_the_status_of_the_XGXS_PLL.

[4]_PHY_STAT_2:_Latch-High_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(2)
_transitions_to_'1'._Reset_to_'0_after_read_and_after_reset._Used,
_when_the_XAUI_interface_reference_design_is_implemented,
_to_detect_disparity_errors_from_the_Receive_8b/10b_decoder.

[5]_PHY_STAT_3:_Latch-High_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(3)
_transitions_to_'1'._Reset_to_'0_after_read_and_after_reset._Used,
_when_the_XAUI_interface_reference_design_is_implemented,_to_detect_K28.5_patterns_from_the_line.

[6]_PHY_STAT_4:_Latch-High_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(4)
_transitions_to_'1'._Reset_to_'0_after_read_and_after_reset._Used,
_when_the_XAUI_interface_reference_design_is_implemented,_to_10b_character_errors._

[7]_PHY_STAT_5:_Latch_Low_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(5)
_transitions_to_'1'_after_reset_or_after_read._Set_to_'0'_when_the_when_the_signal_phy_status(5)
_transitions_to_'0._Used,_when_the_XAUI_interface_reference_design_is_implemented,
_to_monitor_the_XGXS_transmit_and_receive_PLL_lock_status.

[8]_PHY_STAT_6:_Latch_Low_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(6)
_transitions_to_'1'_after_reset_or_after_read._Set_to_'0'_when_the_when_the_signal_phy_status(6)
_transitions_to_'0._Used,_when_the_XAUI_interface_reference_design_is_implemented,
_to_monitor_the_XGXS_receive_lock_on_incoming_data_status.

[9]_PHY_STAT_7:_Latch_Low_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(7)
_transitions_to_'1'_after_reset_or_after_read._Set_to_'0'_when_the_when_the_signal_phy_status(7)
_transitions_to_'0._Used,_when_the_XAUI_interface_reference_design_is_implemented,
_to_monitor_the_XAUI_Lanes_alignment_status.

[10]_PHY_STAT_8:_Latch_Low_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(8)
_transitions_to_'1'_after_reset_or_after_read._Set_to_'0'_when_the_when_the_signal_phy_status(8)
_transitions_to_'0._Used,_when_the_XAUI_interface_reference_design_is_implemented,
_to_monitor_the_XAUI_Lanes_synchronization_status.

[11]_PHY_STAT_9:_Latch_Low_PHY_Status_Register._Set_to_'1'_when_the_signal_phy_status(9)
_transitions_to_'1'_after_reset_or_after_read._Set_to_'0'_when_the_when_the_signal_phy_status(9)
_transitions_to_'0._Used,_when_the_XAUI_interface_reference_design_is_implemented,
_to_monitor_the_signal_detection_status.

[12]_PHY_LOS:_PHY_indicates_loss-of-signal._Represents_value_of_pin_"phy_los".
  */
#define X0_TX_IPG_LENGTH_ 0xA044
/**<  R/W 32'hC Transmit_Inter-Packet-Gap_(IPG)_value._
A_6-bit_value:_Depending_on_LAN_or_WAN_mode_of_operation_the_value_has_the_following_meaning:_

-_LAN_Mode:_Number_of_octets_in_steps_of_4._Valid_values_are_8,_12,_16,_..._100.
_DIC_is_fully_supported_(see_10.6.1_page_43)_for_any_setting._A_default_of_12_(reset_value)
_must_be_set_to_conform_to_IEEE802.3ae._Warning:_When_set_to_8,
_PCS_layers_may_not_be_able_to_perform_clock_rate_compensation._

WAN_Mode:_Stretch_factor._Valid_values_are_4..15._The_stretch_factor_is_calculated_as_(value+1)*8.
_A_default_of_12_(reset_value)_must_be_set_to_conform_to_IEEE_802.3ae_(i.e._13*8=104).
_A_larger_value_shrinks_the_IPG_(increasing_bandwidth).
The_reset_value_12_leads_to_IEEE802.3ae_conformant_behavior_in_both_modes.  */
#define X0_aFramesTransmittedOK 0xA080
  /**<  RO  32'h0 For_each_Frame_transmitted_without_error_(Including_Pause_Frames).  */
#define X0_aFramesReceivedOK_ 0xA088
/**<  RO  32'h0
Frame_received_without_error_(Including_Pause_Frames).
  */
#define X0_aFrameCheckSequenceErrors_ 0xA090
/**<  RO  32'h0
CRC-32_Error_is_detected_but_the_frame_is_otherwise_of_correct_length.  */
#define X0_aAlignmentErrors_  0xA098
/**<  RO  32'h0
For_each_Frame_received_with_and_alignment_error. */
#define X0_aPAUSEMACCtrlFramesTransmitted 0xA0A0 /**<  RO  32'h0 Valid_pause_frames_transmitted_ */
#define X0_aPAUSEMACCtrlFramesReceived_ 0xA0A8 /**<  RO  32'h0 Valid_pause_frames_received.  */
#define X0_aFrameTooLongErrors  0xA0B0
/**<  RO  32'h0
Frame_received_exceeded_the_maximum_length_programmed_with_register_FRM_LGTH.
  */
#define X0_aInRangeLengthErrors 0xA0B8
/**<  RO  32'h0
A_count_of_frames_with_a_length/type_field_value_between_46_(VLAN:_42)_and_less_than_0x0600,
_that_does_not_match_the_number_of_payload_data_octets_received._
  */
#define X0_VLANTransmittedOK_ 0xA0C0 /**<  RO  32'h0 Valid_VLAN_tagged_frames_transmitted_ */
#define X0_VLANReceivedOK_  0xA0C8 /**<  RO  32'h0 Valid_VLAN_tagged_frames_received_  */
#define X0_ifOutOctets_ 0xA0D0-_0xA0D4
/**<  RO  32'h0
All_octets_transmitted_except_preamble_(i.e._Header,_Payload,_Pad_and_FCS)
_for_all_valid_frames_and_valid_pause_frames_
Note:_The_IEEE_object_'aOctetsTransmittedOk'_equals:_ifOutOctets_-_(18*aFramesTransmittedOk)-(4*
VLANTransmittedOK)_ */
#define X0_ifInOctets_  0xA0D8
/**<  RO  32'h0
0xA0DC
All_octets_received_except_preamble_(i.e._Header,_Payload,_Pad_and_FCS)
_for_all_valid_frames_and_valid_pause_frames_received._
Note:_The_IEEE_object_'aOctetsReceivedOk'_equals:_ifInOctets_-_(18*aFramesReceivedOk)-(4*
VLANReceivedOK)_ */
#define X0_ifInUcastPkts_ 0xA0E0
/**<  RO  32'h0
Incremented_with_each_valid_frame_received_on_the_receive_FIFO_interface_and_bit_0_of_the_destinatio
n_address_was_'0'.
  */
#define X0_ifInMulticastPkts_ 0xA0E8
/**<  RO  32'h0
Incremented_with_each_valid_frame_received_on_the_receive_FIFO_interface_and_bit_0_of_the_destinatio
n_address_was_'1'_but_not_the_broadcast_address_(all_bits_set_'1'_)._Pause_frames_are_not_counted.
*/
#define X0_ifInBroadcastPkts_ 0xA0F0
/**<  RO  32'h0
Incremented_with_each_valid_frame_received_on_the_receive_FIFO_interface_(FIFO)
_and_all_bits_of_the_destination_address_were_set_'1'. */
#define X0_ifOutErrors  0xA0F8
/**<  RO  32'h0 Number_of_frames_transmitted_with_error:_
-_FIFO_Overflow_Error_
-_FIFO_Underflow_Error_
-_User_application_defined_error_(ff_tx_err_asserted_together_with_ff_tx_eop)_
  */
#define X0_ifOutUcastPkts_  0xA108
/**<  RO  32'h0
Incremented_with_each_frame_written_to_the_FIFO_interface_and_bit_0_of_the_destination_address_set_t
o_'0'._
  */
#define X0_ifOutMulticastPkts 0xA110
/**<  RO  32'h0
Incremented_with_each_frame_written_to_the_FIFO_interface_and_bit_0_of_the_destination_address_set_t
o_'1'_but_not_the_broadcast_address_(all_bits_'1')._  */
#define X0_ifOutBroadcastPkts 0xA118
/**<  RO  32'h0
Incremented_with_each_frame_written_to_the_FIFO_interface_and_all_bits_of_the_destination_address_se
t_to_'1'. */
#define X0_etherStatsDropEvents_  0xA120
/**<  RO  32'h0
Counts_the_number_of_dropped_packets_due_to_internal_errors_of_the_MAC_Client.
_Occurs_when_a_Receive_FIFO_overflow_condition_persists.
  */
#define X0_etherStatsOctets_  0xA128
/**<  RO  32'h0
Total_number_of_octets_received._Good_and_bad_packets.  */
#define X0_etherStatsPkts_  0xA130
/**<  RO  32'h0
Total_number_of_packets_received._Good_and_bad_packets._"._
  */
#define X0_etherStatsUndersizePkts_ 0xA138
/**<  RO  32'h0
Total_number_of_packets_that_were_less_than_64_octets_long_with_a_good_CRC._
Note:_Undersize_packets_are_not_delivered_to_the_FIFO_interface.
  */
#define X0_etherStatsPkts64Octets 0xA140
/**<  RO  32'h0
Incremented_when_a_packet_of_64_octets_length_is_received_(good_and_bad_frames_are_counted).
  */
#define X0_etherStatsPkts65to127Octets  0xA148
/**<  RO  32'h0
Frames_(Good_and_bad)_with_65_to_127_octets */
#define X0_etherStatsPkts128to255Octets_  0xA150
/**<  RO  32'h0
Frames_(Good_and_bad)_with_128_to_255_octets  */
#define X0_etherStatsPkts256to511Octets 0xA158
/**<  RO  32'h0
Frames_(Good_and_bad)_with_256_to_511_octets  */
#define X0_etherStatsPkts512to1023Octets_ 0xA160
/**<  RO  32'h0
Frames_(Good_and_bad)_with_512_to_1023_octets */
#define X0_etherStatsPkts1024to1518Octets 0xA168
/**<  RO  32'h0
Frames_(Good_and_bad)_with_1024_to_1518_octets  */
#define X0_etherStatsPkts1519toX  0xA170
/**<  RO  32'h0
Proprietary_RMON_extension_counter_that_counts_the_number_of_frames_with_1519_bytes_to_the_maximum_l
ength_programmed_in_the_register_FRAME_LENGTH._
  */
#define X0_etherStatsOversizePkts 0xA178
/**<  RO  32'h0
Total_number_of_packets_longer_than_the_valid_maximum_length_(Programmed_in_register_FRM_LENGTH)_(
Excluding_framing_bits,_but_including_FCS_octets),_and_with_a_good_Frame_Check_Sequence._

  */
#define X0_etherStatsJabbers  0xA180
/**<  RO  32'h0
Total_number_of_packets_longer_than_valid_maximum_length_octets_(Programmed_in_register_FRM_LENGTH)_
(Excluding_framing_bits,_but_including_FCS_octets),_and_with_a_bad_Frame_Check_Sequence._ */
#define X0_etherStatsFragments  0xA188
/**<  RO  32'h0
Total_number_of_packets_that_were_less_than_64_octets_long_with_a_wrong_CRC._
Note:_Fragments_are_not_delivered_to_the_FIFO_interface._
  */
#define X0_ifInErrors 0xA190
/**<  RO  32'h0 Number_of_frames_received_with_error:_
-_FIFO_Overflow_Error_
-_CRC_Error_
-_Frame_Too_Long_Error_
-_Alignment_Error_
-_The_dedicated_Error_Code_(0xfe,_not_a_code_error)_was_received_
  */


/*  XAUI_Config_&_TEST_Registers            */


#define X0_CT_CTRL  0xa800
/**<  R/W 32'h0 XAUI_i/f_is_powered_down_by_default.
[0]_~sw_pwrdna
[1]_~sw_pwrdnb
[2]_~sw_pwrdnc
[3]_~sw_pwrdnd
[4]_~sw_resetpllab
[5]_~sw_resetpllcd_
[8]_~sw_reseta
[9]_~sw_resetb
[10]_~sw_resetc
[11]_~sw_resetd
[12]_sw_rstn_sd0rxclk
[13]_sw_rstn_sd1rxclk
[14]_sw_rstn_sd2rxclk
[15]_sw_rstn_sd3rxclk
[16]_sw_rstn_txclk
[20]_~SREFBUF_PWRDN */
#define X0_CT_CFG0  0xa804
/**<  R/W 32'h0 [3:0]_DeqA[3:0]
[7:4]_DeqB[3:0]
[11:8]_DeqC[3:0]
[15:12]_DeqD[3:0]
[19:16]_DtxA[3:0]
[23:20]_DtxB[3:0]
[27:24]_DtxC[3:0]
[31:28]_DtxD[3:0] */
#define X0_CT_CFG1  0xa808
/**<  R/W 32'h0 [0]_HiDrvA
[1]_LoDrvA
[2]_HiDrvB
[3]_LoDrvB
[4]_HiDrvC
[5]_LoDrvC
[6]_HiDrvD
[7]_LoDrvD
[8]_PCS_jumbo_en
[10:9]_SREFBUF_TERMADJ[1:0] */
#define X0_CT_TEST  0xa80c
/**<  R/W 32'h0 [3:0]_-_PRBS_channel_select
[7:4]_-_PRBS_check
[11:8]_-_LPBK[D/C/B/A]
[12]_-_CONTEN
[13]_-_CONTIN
[14]_-_FAR-END_LOOPBACK_(XGMII_RX_loopbacked_to_TX) */
#define X0_CT_STAT  0xa810
/**<  RO  32'h0 [1:0]_-_PRBS_Error_Count0[1:0]
[3:2]_-_PRBS_Error_Count1[1:0]
[5:4]_-_PRBS_Error_Count2[1:0]
[7:6]_-_PRBS_Error_Count3[1:0]
[8]_-_CONTOUT */

/*  Table_:_XAUI1_Config_Registers            */

#define X1_CT_CTRL  0xb800
/**<  R/W 32'h0 XAUI_i/f_is_powered_down_by_default.
[0]_~sw_pwrdna
[1]_~sw_pwrdnb
[2]_~sw_pwrdnc
[3]_~sw_pwrdnd
[4]_~sw_resetpllab
[5]_~sw_resetpllcd_
[8]_~sw_reseta
[9]_~sw_resetb
[10]_~sw_resetc
[11]_~sw_resetd
[12]_sw_rstn_sd0rxclk
[13]_sw_rstn_sd1rxclk
[14]_sw_rstn_sd2rxclk
[15]_sw_rstn_sd3rxclk
[16]_sw_rstn_txclk
[20]_~SREFBUF_PWRDN */
#define X1_CT_CFG0  0xb804
/**<  R/W 32'h0 [3:0]_DeqA[3:0]
[7:4]_DeqB[3:0]
[11:8]_DeqC[3:0]
[15:12]_DeqD[3:0]
[19:16]_DtxA[3:0]
[23:20]_DtxB[3:0]
[27:24]_DtxC[3:0]
[31:28]_DtxD[3:0] */
#define X1_CT_CFG1  0xb808
/**<  R/W 32'h0 [0]_HiDrvA
[1]_LoDrvA
[2]_HiDrvB
[3]_LoDrvB
[4]_HiDrvC
[5]_LoDrvC
[6]_HiDrvD
[7]_LoDrvD
[8]_PCS_jumbo_en
[10:9]_SREFBUF_TERMADJ[1:0] */
#define X1_CT_TEST  0xb80c
/**<  R/W 32'h0 [3:0]_-_PRBS_channel_select
[7:4]_-_PRBS_check
[11:8]_-_LPBK[D/C/B/A]
_[12]_-_CONTEN
[13]_-_CONTIN */
#define X1_CT_STAT  0xb810
/**<  RO  32'h0 [1:0]_-_PRBS_Error_Count0[1:0]
[3:2]_-_PRBS_Error_Count1[1:0]
[5:4]_-_PRBS_Error_Count2[1:0]
[7:6]_-_PRBS_Error_Count3[1:0]
[8]_-_CONTOUT */

#define X1_COMMAND_CONFIG_ 0xB008
#define X1_FRM_LENGTH 0xB014
#define X1_PAUSE_QUANT_ 0xB018
#define X1_TX_FIFO_SECTIONS  0xB020
#define X1_DMA_CONFIG 0xB400
#define X1_RTYPE  0xB404
#define X1_TTYPE  0xB408
#define X1_FREE_LEVEL  0xB50C
#define X1_TEST 0xB508

/** support for multiple rings of fms */
#define FMS_RING_SELECT (phy_device->fms_ring_id)
#define FDF_RING_OFFSET (FDF1_BA_LSB - FDF0_BA_LSB)
#define PDF_RING_OFFSET (PDF1_POLL_INTERVAL - PDF0_POLL_INTERVAL)
#define RDF_RING_OFFSET (RDF1_CONFIG - RDF0_CONFIG)
#define CNXT_RING_OFFSET (CNXT1_BASE_BASE_LO - CNXT0_BASE_BASE_LO)

/* FDF rings */
#define FDF_BA_LSB (FDF0_BA_LSB + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_BA_MSB (FDF0_BA_MSB + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_SIZE (FDF0_SIZE + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_CONFIG (FDF0_CONFIG + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_POLL_INTERVAL (FDF0_POLL_INTERVAL + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_FIFO_RD_PTR (FDF0_FIFO_RDPTR + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_FIFO_WRT_PTR (FDF0_FIFO_WRPTR + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_FORCE_POLL (FDF0_FORCE_POLL + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_RDF_BASE_LOW (FDF0_RDF_BASE_LOW + FMS_RING_SELECT * FDF_RING_OFFSET)
#define FDF_RDF_BASE_HI (FDF0_RDF_BASE_HI + FMS_RING_SELECT * FDF_RING_OFFSET)

/* PDF rings */
#define PDF_POLL_INTERVAL (PDF0_POLL_INTERVAL + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PDF_CONFIG (PDF0_CONFIG + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PDF_CONFIG2 (PDF0_CONFIG2 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PDF_CONFIG4 (PDF0_CONFIG4 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PDF_MEMMAP_CONFIG (PDF0_MEMMAP_CONFIG + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PDF_BASE_LOW (PDF0_BASE_LOW + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PDF_BASE_HI (PDF0_BASE_HI + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PD_BASE_LOW (PD0_BASE_LOW + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PD_BASE_HI (PD0_BASE_HI + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_0 (PQ0_BASE_LOW_0 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_0 (PQ0_BASE_HI_0 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_1 (PQ0_BASE_LOW_1 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_1 (PQ0_BASE_HI_1 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_2 (PQ0_BASE_LOW_2 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_2 (PQ0_BASE_HI_2 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_3 (PQ0_BASE_LOW_3 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_3 (PQ0_BASE_HI_3 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_4 (PQ0_BASE_LOW_4 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_4 (PQ0_BASE_HI_4 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_5 (PQ0_BASE_LOW_5 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_5 (PQ0_BASE_HI_5 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_6 (PQ0_BASE_LOW_6 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_6 (PQ0_BASE_HI_6 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_7 (PQ0_BASE_LOW_7 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_7 (PQ0_BASE_HI_7 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_8 (PQ0_BASE_LOW_8 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_8 (PQ0_BASE_HI_8 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_9 (PQ0_BASE_LOW_9 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_9 (PQ0_BASE_HI_9 + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_A (PQ0_BASE_LOW_A + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_A (PQ0_BASE_HI_A + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_B (PQ0_BASE_LOW_B + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_B (PQ0_BASE_HI_B + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_C (PQ0_BASE_LOW_C + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_C (PQ0_BASE_HI_C + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_D (PQ0_BASE_LOW_D + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_D (PQ0_BASE_HI_D + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_E (PQ0_BASE_LOW_E + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_E (PQ0_BASE_HI_E + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_LOW_F (PQ0_BASE_LOW_F + FMS_RING_SELECT * PDF_RING_OFFSET)
#define PQ_BASE_HI_F (PQ0_BASE_HI_F + FMS_RING_SELECT * PDF_RING_OFFSET)

/* RDF rings */
#define RDF_BA_LSB (RDF0_BA_LSB + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_BA_MSB (RDF0_BA_MSB + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_WRT_PTR (RDF0_WRT_PTR + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_RD_PTR (RDF0_RD_PTR + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_SIZE (RDF0_SIZE + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_CONFIG (RDF0_CONFIG + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_CLAMP0 (RDF0_CLAMP0 + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_CLAMP1 (RDF0_CLAMP1 + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_FORCE_TIMER (RDF0_FORCE_TIMER + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_TIMESTAMP_LOW (RDF0_TIMESTAMP_LOW + FMS_RING_SELECT * RDF_RING_OFFSET)
#define RDF_TIMESTAMP_HI (RDF0_TIMESTAMP_HI + FMS_RING_SELECT * RDF_RING_OFFSET)

/* cnxt rings */
#define CNXT_BASE_BASE_LOW (CNXT0_BASE_BASE_LO + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_BASE_BASE_HI (CNXT0_BASE_BASE_HI + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_BUFF_BASE_LOW (CNXT0_BUFF_BASE_LO + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_BUFF_BASE_HI (CNXT0_BUFF_BASE_HI + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_WRT_PTR (CNXT0_WRT_PTR + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_RD_PTR (CNXT0_RD_PTR + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_SIZE (CNXT0_SIZE + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_CTRL (CNXT0_CTRL + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_FIFO_CNT (CNXT0_FIFO_CNT + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_POP_CTRL (CNXT0_POP_CTRL + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_POP_DATA (CNXT0_POP_DATA + FMS_RING_SELECT * CNXT_RING_OFFSET)
#define CNXT_WATERMARK (CNXT0_WATERMARK + FMS_RING_SELECT * CNXT_RING_OFFSET)

/* Scratch registers */
#define SCRATCH0   0x5940
#define SCRATCH1   0x5944
#define SCRATCH2   0x5948
#define SCRATCH3   0x594C
#define SCRATCH4   0x5950
#define SCRATCH5   0x5954
#define SCRATCH6   0x5958
#define SCRATCH7   0x595C
#define SCRATCH8   0x5960
#define SCRATCH9   0x5964
#define SCRATCH10  0x5968
#define SCRATCH11  0x596C
#define SCRATCH12  0x5970
#define SCRATCH13  0x5974
#define SCRATCH14  0x5978
#define SCRATCH15  0x597C

#if defined (__linux__) || defined (__CYGWIN__) || defined (_WIN32)
#define HW_REGISTER(REG_NAME)    \
    SWAP32(((volatile uint32_t *)(HW_REGISTER_MAP_BASE))[((unsigned short)(REG_NAME))>>2])

#define SET_HW_REGISTER(REG_NAME, VAL) \
    (((volatile uint32_t *)(HW_REGISTER_MAP_BASE))[((unsigned short)(REG_NAME))>>2]) = SWAP32(VAL)
#else
#error "OS is not supported"
#endif

/* amount of wait time that must pass before the FDF DMA engine updates
   the external FDF_RD_PTR and RDF_WRT_PTR copies in system memory,
   but only if at least one of the external values are stale */
#define FDF_POLL_INTERVAL_DEFAULT_VALUE 64

/* used when polling for PDF busy bit. It is incremented in units of 32 sclk
   cycles. 0 is equal to 32 clocks, 0x20 - 64 clocks */
#define PDF_POLL_INTERVAL_DEFAULT_VALUE 0x20

/* the maximum number of times the busy bit can be polled before the flow is failed.
   If this value is 0, this timeout function is disabled */
#define PDF_BUSY_BIT_POLL_TIMEOUT 0xffff

/* This timer control when the internal RDF FIFO gets flushed out to system
   memory regardless of max burst size */
#define RDF_FORCE_TIMER_DEFAULT_VALUE 0x32

/* maximum number of results per job */
#define MAX_MATCHES_PER_JOB 0x3fff

#define FMS_BLK_REG_STEP (BLOCK1_CONFIG - BLOCK0_CONFIG)

/* Maximum number of DDR3 slices */
#define NLM_DDR3_MAX_SLICE 32
#define NLM_DDR3_MAX_SLICE_SUB 4
#define NLM_DDR3_MAX_CS 2
#define NLM_DDR3_MAX_DLL_DELAY 256
#define NLM_DDR3_MEM_DQS_WIDTH 8
#define NLM_DDR3_MEM_ECC_DQS_WIDTH 1
#define NLM_DDR3_MAX_CS 2
#define NLM_DDR3_CS_MAP 3
#define NLM_DDR3_OUT_PAD_DELAY 100
#define NLM_DDR3_IN_PAD_DELAY 100
#define NLM_DDR3_FLIGHT_DELAY 50
#define NLM_DDR3_TDQSCK_MAX 255
#define NLM_DDR3_DLL_DQS_DELAY_WIDTH 7
#define NLM_DDR3_TDQSQ 125

#endif /* FMS_REGISTERS_H */
