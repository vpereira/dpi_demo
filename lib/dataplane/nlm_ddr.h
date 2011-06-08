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
* Copyright, 2009, NetLogic Microsystems, Inc. All rights reserved.
*******************************************************************************/
#ifndef NLM_DDR_H
#define NLM_DDR_H

#define DDR3_OP_TIMEOUT 0xFFFF
#define DLL_DQS_DELAY_WIDTH 7

#define DLL_CTRL_REG_0_0  DDR3_CTL_71
#define DLL_CTRL_REG_0_1  DDR3_CTL_72
#define DLL_CTRL_REG_0_2  DDR3_CTL_73
#define DLL_CTRL_REG_0_3  DDR3_CTL_74
#define DLL_CTRL_REG_0_4  DDR3_CTL_75
#define DLL_CTRL_REG_0_5  DDR3_CTL_76
#define DLL_CTRL_REG_0_6  DDR3_CTL_77
#define DLL_CTRL_REG_0_7  DDR3_CTL_78
#define DLL_CTRL_REG_0_8  DDR3_CTL_79

#define PHY_CTRL_REG_0_0  DDR3_CTL_81
#define PHY_CTRL_REG_0_1  DDR3_CTL_82
#define PHY_CTRL_REG_0_2  DDR3_CTL_83
#define PHY_CTRL_REG_0_3  DDR3_CTL_84
#define PHY_CTRL_REG_0_4  DDR3_CTL_85
#define PHY_CTRL_REG_0_5  DDR3_CTL_86
#define PHY_CTRL_REG_0_6  DDR3_CTL_87
#define PHY_CTRL_REG_0_7  DDR3_CTL_88
#define PHY_CTRL_REG_0_8  DDR3_CTL_89

#define PHY_CTRL_REG_1_0  DDR3_CTL_90
#define PHY_CTRL_REG_1_1  DDR3_CTL_91
#define PHY_CTRL_REG_1_2  DDR3_CTL_92
#define PHY_CTRL_REG_1_3  DDR3_CTL_93
#define PHY_CTRL_REG_1_4  DDR3_CTL_94
#define PHY_CTRL_REG_1_5  DDR3_CTL_95
#define PHY_CTRL_REG_1_6  DDR3_CTL_96
#define PHY_CTRL_REG_1_7  DDR3_CTL_97
#define PHY_CTRL_REG_1_8  DDR3_CTL_98

union ddr3_ctl_00
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:7,
    uint32_t big_endian_en:1,     // [24] – BIG_ENDIAN_EN(RW)
    uint32_t pad3:7,
    uint32_t auto_refresh_mode:1, // [16] – AUTO_REFRESH_MODE (RW)
    uint32_t pad2:7,
    uint32_t arefresh:1,          // [8] – AREFRESH (WR)
    uint32_t pad1:7,
    uint32_t ap:1                // [0] – AP (RW)
    )
  } s;
};

union ddr3_ctl_06
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:7,
    uint32_t rdlvl_edge:1,            // [24]_-_RDLVL_EDGE_(RW)
    uint32_t pad3:7,
    uint32_t rdlvl_cs:1,              // [16]_-_RDLVL_CS_(RW)
    uint32_t pad2:7,
    uint32_t rdlvl_begin_delay_en:1,  // [8]_-_RDLVL_BEGIN_DELAY_EN_(RW)
    uint32_t pad1:7,
    uint32_t pwrup_srefresh_exit:1   // [0]_-_PWRUP_SREFRESH_EXIT_(RW)  */
    )
  } s;
};

union ddr3_ctl_07
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:7,
    uint32_t rdlvl_gate_req:1,                // [24]_-_RDLVL_GATE_REQ_(RW)
    uint32_t pad3:7,
    uint32_t rdlvl_gate_preamble_check_en:1,  // [16]_-_RDLVL_GATE_PREAMBLE_CHECK_EN_(RW)
    uint32_t pad2:7,
    uint32_t rdlvl_gate_en:1,                 // [8]_-_RDLVL_GATE_EN_(RW)
    uint32_t pad1:7,
    uint32_t rdlvl_en:1                      // [0]_-_RDLVL_EN_(RW)
    )
  } s;
};

union ddr3_ctl_11
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(

    uint32_t pad4:7,
    uint32_t swlvl_load:1,  // [24]_-_SWLVL_LOAD_(WR)
    uint32_t pad3:7,
    uint32_t swlvl_exit:1,  // [16]_-__SWLVL_EXIT_(WR)
    uint32_t pad2:7,
    uint32_t start:1,       // [8]_-_START_(RW)
    uint32_t pad1:7,
    uint32_t srefresh:1    // [0]_-_SREFRESH_(RW)
    )
  } s;
};

union ddr3_ctl_12
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:7,
    uint32_t tref_enable:1,   // [24]_-_TREF_ENABLE_(RW)
    uint32_t pad3:7,
    uint32_t tras_lockout:1,  // [16]_-_TRAS_LOCKOUT_(RW)
    uint32_t pad2:7,
    uint32_t swlvl_start:1,   // [8]_-_SWLVL_START_(WR)
    uint32_t pad1:7,
    uint32_t swlvl_op_done:1 // [0]_-_SWLVL_OP_DONE_(RO)
    )
  } s;
};

union ddr3_ctl_13
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:7,
    uint32_t wrlvl_interval_ct_en:1,  // [24]_-_WRLVL_INTERVAL_CT_EN_(RW)
    uint32_t pad3:7,
    uint32_t wrlvl_cs:1,              // [16]_-_WRLVL_CS_(RW)
    uint32_t pad2:7,
    uint32_t write_modereg:1,         // [8]_-_WRITE_MODEREG_(WR)
    uint32_t pad1:7,
    uint32_t writeinterp:1           // [0]_-_WRITEINTERP_(RW)
    )
  } s;
};

union ddr3_ctl_17
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:6,
    uint32_t zq_on_sref_exit:2,   // [25:24]_-_ZQ_ON_SREF_EXIT_(RW)
    uint32_t pad3:6,
    uint32_t zwcs_chip:2,         // [17:16]_-__ZQCS_CHIP_(RW)
    uint32_t pad2:6,
    uint32_t sw_leveling_mode:2,  // [9:8]_-_SW_LEVELING_MODE_(RW)
    uint32_t pad1:6,
    uint32_t rtt_0:2             // [1:0]_-_RTT_0_(RW)
    )
  } s;
};

union ddr3_ctl_24
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:4,
    uint32_t wrlat:4,         // [27:24]_-_WRLAT_(RW)
    uint32_t pad3:4,
    uint32_t twtr:4,          // [19:16]_-__TWTR_(RW)
    uint32_t pad2:4,
    uint32_t trp:4,           // [11:8]_-_TRP_(25)
    uint32_t pad1:4,
    uint32_t tdfi_wrlvl_en:4 // [3:0]_-_TDFI_WRLVL_EN_(RW)
    )
  } s;
};

union ddr3_ctl_25
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:3,
    uint32_t max_row_reg:5,     // R/W 32'h10000000  [28:24]_-_MAX_ROW_REG_(RO)
    uint32_t pad3:3,
    uint32_t caslat_lin_gate:5, // [20:16]_-__CASLAT_LIN_GATE_(RW)
    uint32_t pad2:3,
    uint32_t caslat_lin:5,      // [12:8]_-_CASLAT_LIN_(RW)
    uint32_t pad1:4,
    uint32_t wrlat_adj:4       // [3:0]_-_WRLAT_ADJ(RW) */
    )
  } s;
};

union ddr3_ctl_26
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:3,
    uint32_t tdal:5,                // [28:24] - TDAL (RW)
    uint32_t pad3:3,
    uint32_t rdlat_adj:5,           // [20:16] - RDLAT_ADJ (RW)
    uint32_t pad2:3,
    uint32_t ocd_adjust_pup_cs_0:5, // [12:8] - OCD_ADJUST_PUP_CS_0 (RW)
    uint32_t pad1:3,
    uint32_t ocd_adjust_pdn_cs_0:5 // [4:0] - OCD_ADJUST_PDN_CS_0 (RW)
    )
  } s;
};

union ddr3_ctl_27
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER8(
    uint32_t pad4:3,
    uint32_t tfaw:5,                // [28:24] - TFAW_(RW)
    uint32_t pad3:3,
    uint32_t tdfi_rddata_en_base:5, // [20:16] - TDFI_RDDATA_EN_BASE_(RW)
    uint32_t pad2:3,
    uint32_t tdfi_rddata_en:5,      // [12:8]_-_TDFI_RDDATA_EN_(RO)
    uint32_t pad1:7,
    uint32_t tdfi_phy_rdlat:1      // [0]_-_TDFI_PHY_RDLAT_(RW)
    )
  } s;
};

union ddr3_ctl_37
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t rdlvl_gate_delay_1:8,       // [31:24]_-_RDLVL_GATE_DELAY_1_(RW)
    uint32_t rdlvl_gate_delay_0:8,      // [23:16]_-_RDLVL_GATE_DELAY_0__(RW)
    uint32_t rdlvl_gate_clk_adjust_8:8, // [16:8]_-_RDLVL_GATE_CLK_ADJUST_8_(RW)
    uint32_t rdlvl_clk_adjust_7:8      // [7:0]_-_RDLVL_CLK_ADJUST_7_(RW)
    )
  } s;
};

union ddr3_ctl_38
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t rdlvl_gate_delay_5:8,       // [31:24]_-_RDLVL_GATE_DELAY_5_(RW)
    uint32_t rdlvl_gate_delay_4:8,      // [23:16]_-_RDLVL_GATE_DELAY_4__(RW)
    uint32_t rdlvl_gate_delay_3:8,      // [31:24]_-_RDLVL_GATE_DELAY_3_(RW)
    uint32_t rdlvl_gate_delay_2:8      // [23:16]_-_RDLVL_GATE_DELAY_2__(RW)
    )
  } s;
};

union ddr3_ctl_39
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t rdlvl_gate_max_delay:8,     // [31:24]_-_RDLVL_GATE_MAX_DELAY_(RW)
    uint32_t rdlvl_gate_delay_8:8,      // [23:16]_-_RDLVL_GATE_DELAY_8__(RW)
    uint32_t rdlvl_gate_delay_7:8,      // [31:24]_-_RDLVL_GATE_DELAY_7_(RW)
    uint32_t rdlvl_gate_delay_6:8      // [23:16]_-_RDLVL_GATE_DELAY_6__(RW)
    )
  } s;
};

union ddr3_ctl_45
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t swlvl_resp_3:8,     // [31:24]_-_SWLVL_RESP_3_(RW)
    uint32_t swlvl_resp_2:8,      // [23:16]_-__SWLVL_RESP_2_(RW)
    uint32_t swlvl_resp_1:8,      // [15:8]_-_SWLVL_RESP_1_(RW)
    uint32_t swlvl_resp_0:8      // [7:0]_-_SWLVL_RESP_0_(RW)
    )
  } s;
};

union ddr3_ctl_46
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t swlvl_resp_7:8,     // [31:24]_-_SWLVL_RESP_7_(RW)
    uint32_t swlvl_resp_6:8,      // [23:16]_-__SWLVL_RESP_6_(RW)
    uint32_t swlvl_resp_5:8,      // [15:8]_-_SWLVL_RESP_5_(RW)
    uint32_t swlvl_resp_4:8      // [7:0]_-_SWLVL_RESP_4_(RW)
    )
  } s;
};

union ddr3_ctl_47
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t tdfi_rdlvl_resplat:8, // [31:24]_-_TDFI_RDLVL_RESPLAT_(RW)
    uint32_t tdfi_rdlvl_resp:8,   // [23:16]_-_TDFI_RDLVL_RESP_(RW)
    uint32_t tdfi_rdlvl_dll:8,    // [15:8]_-_TDFI_RDLVL_DLL_(RW)
    uint32_t swlvl_resp_8:8      // [7:0]_-_SWLVL_RESP_8_(RW)
    )
  } s;
};

union ddr3_ctl_48
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER4(
    uint32_t tdfi_wrlvl_resplat:8, // [31:24]_-_TDFI_WRLVLV_RESPLAT_(RW)
    uint32_t tdfi_wrlvl_resp:8,   // [23:16]_-_TDFI_WRLVL_RESP_(RW)
    uint32_t tdfi_wrlvl_dll:8,    // [15:8]_-_TDFI_WRLVL_DLL_(RW)
    uint32_t tdfi_rdlvl_rr:8      // [7:0]_-_TDFI_RDLVL_RR_(RW)
    )
  } s;
};

union ddr3_ctl_213
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t rdlvl_delay_1:16,
    uint32_t rdlvl_delay_0:16
    )
  } s;
};

union ddr3_ctl_214
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t rdlvl_delay_3:16,
    uint32_t rdlvl_delay_2:16
    )
  } s;
};

union ddr3_ctl_215
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t rdlvl_delay_5:16,
    uint32_t rdlvl_delay_4:16
    )
  } s;
};

union ddr3_ctl_216
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t rdlvl_delay_7:16,
    uint32_t rdlvl_delay_6:16
    )
  } s;
};

union ddr3_ctl_217
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t rdlvl_end_delay_0:16,
    uint32_t rdlvl_delay_8:16
    )
  } s;
};

union ddr3_ctl_236
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t wrlvl_delay_0:16,
    uint32_t rdlvl_offset_delay_8:16
    )
  } s;
};

union ddr3_ctl_237
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t wrlvl_delay_2:16,
    uint32_t wrlvl_delay_1:16
    )
  } s;
};

union ddr3_ctl_238
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t wrlvl_delay_4:16,
    uint32_t wrlvl_delay_3:16
    )
  } s;
};

union ddr3_ctl_239
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t wrlvl_delay_6:16,
    uint32_t wrlvl_delay_5:16
    )
  } s;
};

union ddr3_ctl_240
{
  uint32_t     word32;
  struct
  {
    __ENDIAN_ORDER2(
    uint32_t wrlvl_delay_8:16,
    uint32_t wrlvl_delay_7:16
    )
  } s;
};

static nlm_status nlm_device_ddr_write_ind_32 (nlm_phy_device_t *phy_device,
                                               uint64_t addr, uint32_t data);
static nlm_status nlm_device_ddr_write_ind_64 (nlm_phy_device_t *phy_device,
                                               uint64_t addr, uint64_t data);
static void nlm_device_ddr3_wait_op_done (nlm_phy_device_t *phy_device);
static void nlm_device_ddr3_sw_lvl_load (nlm_phy_device_t *phy_device);
static void nlm_device_ddr3_sw_lvl_exit (nlm_phy_device_t *phy_device);
static void nlm_device_ddr3_sw_lvl_start (nlm_phy_device_t *phy_device);

#endif
