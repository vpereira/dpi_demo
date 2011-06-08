/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

/** \file nlm_driver_api.h
  header file for linux specific initialization phase of Data plane API */
#ifndef __NLM_DRIVER_API_H
#define __NLM_DRIVER_API_H

#include "nlm_packet_api.h"

#define NLM_MAX_DEVICE_COUNT 8

/** Get number of netl7 devices
  \param[out] n_devices number of the netl7 devices on the system
  \return status */
nlm_status nlm_get_device_count (uint32_t *n_devices);

/** Initialize netl7 device configuration
  Fill in platform specific fields of the configuration like system memory,
  register map, packet memory addresses.
  \param[in] device_id id of the netl7 device on the system
  \param[in] config device configuration
  \param[out] config device configuration
  \return status */
nlm_status nlm_prepare_device_config (uint32_t device_id, 
                                      struct nlm_device_config *config);

/** Unmap virtual memory mapped by nlm_prepare_device_config() for system memory, packet memory
    and register memory. Free memory pool */
nlm_status nlm_free_device_config (struct nlm_device_config *config);

/** Initialize all netl7 devices
  \param[out] devices array of initialized device handles
  \param[out] n_devices number of initialized devices
  \return status */
nlm_status nlm_init_all_devices (struct nlm_device * devices[NLM_MAX_DEVICE_COUNT],
                                 uint32_t *n_devices);

/** Shutdown all netl7 devices
  \param[in] devices array of device handles to be shutdown
  \param[in] n_devices number of devices
  \return status */
nlm_status nlm_fini_all_devices (struct nlm_device * devices[NLM_MAX_DEVICE_COUNT],
                                 uint32_t n_devices);

#endif /* __NLM_DRIVER_API_H */
