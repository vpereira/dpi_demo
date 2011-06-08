#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nlm_system.h"
#include "nlm_packet_api_impl.h"

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


void
print_regs(struct nlm_device_config *config)
{
	nlm_phy_device_t *phy_device;
	phy_device = (nlm_phy_device_t *) config->sysmem_base_virt;
	phy_device->device_magic = NLM_DEVICE_MAGIC;
	phy_device->fms_ring_id = config->ring_id;
	memcpy (&phy_device->config, config, sizeof (struct nlm_device_config));

	printf("HW_REGISTER_MAP_BASE : 0x%x\n", HW_REGISTER_MAP_BASE);
	printf("SHELL_CONTROL : 0x%x\n", HW_REGISTER (SHELL_CONTROL));
	printf("X0_CT_TEST : 0x%x\n", HW_REGISTER (X0_CT_TEST));
	printf("X0_CT_CFG0 : 0x%x\n", HW_REGISTER (X0_CT_CFG0));
	printf("X0_CT_CFG1 : 0x%x\n", HW_REGISTER (X0_CT_CFG1));
	printf("X0_CT_CTRL : 0x%x\n", HW_REGISTER (X0_CT_CTRL));
	printf("X0_COMMAND_CONFIG_ : 0x%x\n", HW_REGISTER (X0_COMMAND_CONFIG_));
	printf("X0_TX_IPG_LENGTH_ : 0x%x\n", HW_REGISTER (X0_TX_IPG_LENGTH_));
	printf("X0_aFramesTransmittedOK : %d\n", HW_REGISTER (X0_aFramesTransmittedOK));
	printf("X0_aFramesReceivedOK : %d\n", HW_REGISTER (X0_aFramesReceivedOK_));
	printf("X0_aFrameCheckSequenceErrors_: %d\n", HW_REGISTER (X0_aFrameCheckSequenceErrors_));
	printf("X0_aAlignmentErrors_: %d\n", HW_REGISTER (X0_aAlignmentErrors_));
	printf("X0_aPAUSEMACCtrlFramesTransmitted : %d\n", HW_REGISTER (X0_aPAUSEMACCtrlFramesTransmitted));
	printf("X0_aPAUSEMACCtrlFramesReceived_: %d\n", HW_REGISTER (X0_aPAUSEMACCtrlFramesReceived_));
	printf("X0_etherStatsOctets_: %d\n", HW_REGISTER (X0_etherStatsOctets_));
	printf("X0_etherStatsPkts_: %d\n", HW_REGISTER (X0_etherStatsPkts_));
	printf("X0_MAC_ADDR_0_: 0x%x\n", HW_REGISTER (X0_MAC_ADDR_0_));
	printf("X0_MAC_ADDR_1_: 0x%x\n", HW_REGISTER (X0_MAC_ADDR_1_));
}

void
set_reg(struct nlm_device_config *config, unsigned short reg, unsigned int value)
{
	nlm_phy_device_t *phy_device;
	phy_device = (nlm_phy_device_t *) config->sysmem_base_virt;
	phy_device->device_magic = NLM_DEVICE_MAGIC;
	phy_device->fms_ring_id = config->ring_id;
	memcpy (&phy_device->config, config, sizeof (struct nlm_device_config));

	SET_HW_REGISTER (reg, value);
}

unsigned int
get_reg(struct nlm_device_config *config, unsigned short reg)
{
	nlm_phy_device_t *phy_device;
	phy_device = (nlm_phy_device_t *) config->sysmem_base_virt;
	phy_device->device_magic = NLM_DEVICE_MAGIC;
	phy_device->fms_ring_id = config->ring_id;
	memcpy (&phy_device->config, config, sizeof (struct nlm_device_config));

	return HW_REGISTER(reg);	
}

void
init_regs(struct nlm_device_config *config)
{
	nlm_phy_device_t *phy_device;
	phy_device = (nlm_phy_device_t *) config->sysmem_base_virt;
	phy_device->device_magic = NLM_DEVICE_MAGIC;
	phy_device->fms_ring_id = config->ring_id;
	memcpy (&phy_device->config, config, sizeof (struct nlm_device_config));
	
	SET_HW_REGISTER (X0_CT_TEST, 0x00040000);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CFG0, 0x8888);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CFG1, 0x00000100);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x00100000);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x00100003);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x00100013);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x00100313);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x0010031f);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x0010033f);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x00100f3f);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_CT_CTRL, 0x0011ff3f);
	NLM_WASTE_TIME
//	SET_HW_REGISTER (X0_MAC_ADDR_0_, 0xdf86ee2f);
	SET_HW_REGISTER (X0_MAC_ADDR_0_, 0x86df1ef8);
	NLM_WASTE_TIME
//	SET_HW_REGISTER (X0_MAC_ADDR_1_, 0xf81e);
	SET_HW_REGISTER (X0_MAC_ADDR_1_, 0x2fee);
	NLM_WASTE_TIME
//	SET_HW_REGISTER (X0_COMMAND_CONFIG_, 0x000281b3);
	SET_HW_REGISTER (X0_COMMAND_CONFIG_, 0x000291b3);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_FRM_LENGTH, 0x00002400);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_PAUSE_QUANT_, 0x00000100);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_TX_FIFO_SECTIONS, 0x00000010);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_DMA_CONFIG, 0x00000011);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_DMA_CONFIG, 0x1);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_TTYPE, 0x43414331);
	NLM_WASTE_TIME
	SET_HW_REGISTER (X0_RTYPE, 0x4321);
	NLM_WASTE_TIME
}
