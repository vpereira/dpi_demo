/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include <string.h>
#include <stdlib.h>
#include "nlm_system.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <driver/netl7driver.h>
#include <unistd.h>
#include <stdio.h>
#include "nlm_driver_api.h"

#ifndef api_assert
#define api_assert pretty_assert
#endif
  
nlm_status
nlm_prepare_device_config (uint32_t device_id, 
                           struct nlm_device_config *config)
{
  nlm_sys_cfg_t sys_cfg;
  char buf[1024];
  int dev_handle;
  void *virt_addr;

  printf("[%s] in, ring_id:%d\n", __FUNCTION__, config->ring_id);
  
  if (config == NULL)
   {
     DPRINT ("device config pointer is null\n");
     return NLM_INVALID_ARGUMENT;
   }

  if (device_id >= NLM_MAX_DEVICE_COUNT)
   {
     DPRINT ("Device id is too large\n");
     return NLM_INVALID_ARGUMENT;
   }
  
  if (config->memory_pool_size == 0 || config->input_fifo_size == 0
      || config->output_fifo_size == 0 || config->sysmem_size == 0)
    return NLM_INVALID_ARGUMENT;
  
  /* allocate memory pool if it is still NULL, and no xmalloc pointer
     is specified for dynamic allocation through the dataplane */
  if (config->memory_pool == NULL
      && config->xmalloc == NULL)
    {
      config->memory_pool = malloc (config->memory_pool_size);
      if (config->memory_pool == NULL)
        return NLM_OUT_OF_MEMORY;
    }

  /* open the device */
#if defined (NLM_HW_FMS)
  if (snprintf (buf, sizeof (buf), NLM_FMS_PROC_PATH, device_id, config->ring_id) < 0)
    return NLM_DRIVER_FAILURE;
#elif defined (NLM_HW_MARS2)
  if (snprintf (buf, sizeof (buf), NLM_MARS2_PROC_PATH, device_id) < 0)
    return NLM_DRIVER_FAILURE;
#elif defined (NLM_HW_MARS1)
  if (snprintf (buf, sizeof (buf), NLM_MARS1_PROC_PATH, device_id) < 0)
    return NLM_DRIVER_FAILURE;
#else
  if (snprintf (buf, sizeof (buf), NLM_MARS3_PROC_PATH, device_id) < 0)
    return NLM_DRIVER_FAILURE;
#endif

  dev_handle = config->reserved1 = open (buf, O_RDWR | O_SYNC, 0);
  if (dev_handle < 0)
    {
      DPRINT ("open of %s failed\n", buf);
      return NLM_DRIVER_FAILURE;
    }

  /* get size and address of register map */
  if (ioctl (dev_handle, _IOR (NLM_IOCTL_GET_REGMEM, 0, nlm_sys_cfg_t), &sys_cfg) < 0)
    return NLM_DRIVER_FAILURE;

  virt_addr = mmap (0, sys_cfg.mem_size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, dev_handle, sys_cfg.physical_addr);
  if (virt_addr == MAP_FAILED)
    return NLM_DRIVER_FAILURE;
  
  DPRINT ("register map base = 0x%llx\n", sys_cfg.physical_addr);
  DPRINT ("register map size = 0x%x\n", sys_cfg.mem_size);
  DPRINT ("register map virt = 0x%p\n", virt_addr);
  
  config->register_map_size = sys_cfg.mem_size;
  config->register_map_base_virt = virt_addr;
  config->register_map_base_phys = sys_cfg.physical_addr;

  sys_cfg.mem_size = config->sysmem_size;
  if (ioctl (dev_handle, _IOR (NLM_IOCTL_GET_SYSMEM, 0, nlm_sys_cfg_t), &sys_cfg) < 0)
    return NLM_DRIVER_FAILURE;
  
  if (sys_cfg.mem_size < config->sysmem_size)
    return NLM_OUT_OF_DEVICE_MEMORY;

  virt_addr = mmap (0, sys_cfg.mem_size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, dev_handle, sys_cfg.physical_addr);
  if (virt_addr == MAP_FAILED)
    return NLM_DRIVER_FAILURE;

  DPRINT ("system memory virt = 0x%p\n", virt_addr);
  DPRINT ("Driver installed with fixed memory\n");
  DPRINT ("system memory base = 0x%llx\n", sys_cfg.physical_addr);
  DPRINT ("system memory size = 0x%x\n", sys_cfg.mem_size);
  
  /* store phys addr of system memory */
#if defined (_MIPS_ARCH_OCTEON) && defined (NLM_HW_MARS1)
  config->sysmem_base_phys = sys_cfg.physical_addr | (1ull<<39);
#else
  config->sysmem_base_phys = sys_cfg.physical_addr;
#endif
  config->sysmem_base_virt = virt_addr;
  config->sysmem_size = sys_cfg.mem_size;

  /* get the DMA-able packet memory */
  if (ioctl (dev_handle, _IOR (NLM_IOCTL_GET_DMAMEM, 0, nlm_sys_cfg_t), &sys_cfg) < 0)
    return NLM_DRIVER_FAILURE;
  
  if (sys_cfg.mem_size == 0)
    return NLM_OUT_OF_DEVICE_MEMORY;

  virt_addr = mmap (0, sys_cfg.mem_size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, dev_handle, sys_cfg.physical_addr);

  if (virt_addr == MAP_FAILED)
    return NLM_DRIVER_FAILURE;
  
#if defined (_MIPS_ARCH_OCTEON) && defined (NLM_HW_MARS1)
  config->packet_base_phys = sys_cfg.physical_addr | (1ull<<39);
#else
  config->packet_base_phys = sys_cfg.physical_addr;
#endif
  
  config->packet_base_virt = virt_addr;
  /* device handle should be open, if not memory will be freed */
  printf("[%s] out \n", __FUNCTION__);

  return NLM_OK;
}

nlm_status
nlm_free_device_config (struct nlm_device_config *config)
{
  if (config == NULL)
    return NLM_INVALID_ARGUMENT;
  
  if (config->memory_pool && config->xfree == NULL)
    free (config->memory_pool);

  if (config->reserved1 > 0)
    {
      if (config->sysmem_base_virt)
        munmap (config->sysmem_base_virt, config->sysmem_size);
      /* Dont have size of packet memory
         if (config->packet_base_virt)
         munmap (config->packet_base_virt, size); */
      if (config->register_map_base_virt)
        munmap (config->register_map_base_virt, config->register_map_size);

      close (config->reserved1);
    }
  
  return NLM_OK;
}

#if defined (NLM_HW_MARS1)
#define DEVICE_ID "184e0008"
#elif defined (NLM_HW_MARS2)
#define DEVICE_ID "184e0009"
#elif defined (NLM_HW_FMS)
#define DEVICE_ID "184e000a"
#elif defined (NLM_HW_MARS3)
#define DEVICE_ID "184e000c"
#else
#error "Unsupported device, please compile with correct -DNLM_HW_* flag"
#endif

nlm_status 
nlm_get_device_count (uint32_t *n_devices)
{
  FILE *fdev;
  uint32_t dev_cnt = 0;
  char page[256];

  fdev = fopen ("/proc/bus/pci/devices", "r");
  if (NULL == fdev)
    {
      DPRINT ("Can't open /proc/bus/pci/devices\n");
      return NLM_DRIVER_FAILURE;
    }

  while (fgets (page, sizeof (page), fdev))
    dev_cnt += (NULL != strstr (page, DEVICE_ID));

  *n_devices = dev_cnt;
  return NLM_OK;
}
