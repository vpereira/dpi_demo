/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef NLM_NETL7DRIVER_H
#define NLM_NETL7DRIVER_H

/* ioctl */

#define NLM_IOCTL_INVALID     0x0
#define NLM_IOCTL_GET_REGMEM  0x1
#define NLM_IOCTL_GET_SYSMEM  0x2
#define NLM_IOCTL_GET_DMAMEM  0x3

typedef struct
{
  unsigned int mem_size;
  unsigned long long physical_addr;
} nlm_sys_cfg_t;

/* The possible devices exported through /proc interface.
   The path can be constructed by adding the device ID
   and ring ID (on Famos only) */
#define NLM_MARS1_PROC_PATH        "/proc/NetL7/mars1.%01d"
#define NLM_MARS2_PROC_PATH        "/proc/NetL7/mars2.%01d"
#define NLM_FMS_PROC_PATH          "/proc/NetL7/fms.%01d.r%01d"
#define NLM_MARS3_PROC_PATH        "/proc/NetL7/mars3.%01d"

#endif /* NLM_NETL7DRIVER_H */
