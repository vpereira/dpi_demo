/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include "nlm_driver_api.h"

/** init all devices */
nlm_status
nlm_init_all_devices (struct nlm_device * devices[NLM_MAX_DEVICE_COUNT],
                      uint32_t *n_devices)
{
  nlm_status status;
  int i;
  uint32_t device_cnt = 0;
  
  status = nlm_get_device_count (&device_cnt);
  if (status != NLM_OK)
    return status;

  if (device_cnt == 0)
    return NLM_DRIVER_FAILURE;

  *n_devices = device_cnt;

  for (i = 0; i < device_cnt; i++)
    {
      struct nlm_device_config config;

      /* estimate netl7 device configuration parameters based on number of flows and jobs */
      status = nlm_estimate_device_config (1000 /* flows */, 1000 /* jobs */, &config);
      if (status != NLM_OK)
        return status;

      status = nlm_prepare_device_config (i, &config);
      if (status != NLM_OK)
        return status;

      /* configure device and get device handle */
      status = nlm_device_init (&config, &devices[i]);
      if (status != NLM_OK)
        return status;
    }

  return NLM_OK;
}

nlm_status 
nlm_fini_all_devices (struct nlm_device * devices[NLM_MAX_DEVICE_COUNT],
                      uint32_t n_devices)
{
  nlm_status status;
  int i;
  
  for (i = 0; i < n_devices; i++)
    {
      status = nlm_device_fini (devices[i]);
      if (status != NLM_OK)
        return status;
    }

  return NLM_OK;
}

