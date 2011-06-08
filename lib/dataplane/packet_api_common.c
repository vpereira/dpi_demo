/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

//#define DEBUG
#include <stdarg.h>
#include "nlm_system.h"
#include "nlm_packet_api.h"
#include "nlm_packet_api_impl.h"

#ifndef api_assert
#define api_assert pretty_assert
#endif

/** Estimate netl7 device configuration parameters based on number of flows and jobs
  and fill in different *_size fields of configuration
  \param[in] n_flows number of flows
  \param[in] n_jobs number of jobs
  \param[out] config device configuration
  \return status */
nlm_status
NLM_PROTO (nlm_estimate_device_config) (uint32_t n_flows, uint32_t n_jobs,
                                        struct nlm_device_config *config)
{
  struct nlm_device_config default_config = DEFAULT_NLM_DEVICE_CONFIG;

  if (config == NULL || n_flows == 0 || n_jobs == 0)
    return NLM_INVALID_ARGUMENT;

  memcpy (config, &default_config, sizeof (struct nlm_device_config));

  return NLM_OK;
}

nlm_status
NLM_PROTO (nlm_device_estimate_memory_pool_size) (nlm_device_t *device, uint32_t max_flows,
                                                  uint32_t max_jobs, uint32_t *size_out)
{
  uint64_t size; /* 64 bit type for simple 32 bit overflow detection */
  uint64_t const mask64_high32 = ~(0xffffffff);

  size = sizeof (nlm_device_t);
  size += sizeof (nlm_phy_device_t);
  
  /* The following exactly matches the allocation of the memory
     pool inside nlm_device_init().  */
  size += max_flows * sizeof (struct nlm_flow);
  if ((size & mask64_high32) != 0)
    return NLM_OUT_OF_MEMORY;

  /* 2 extra jobs - one for a job_slot left by DP and another to
     detect cross-over with flow-slots */
  size += (max_jobs + 2 ) * sizeof (struct nlm_job);
  if ((size & mask64_high32) != 0)
    return NLM_OUT_OF_MEMORY;

  size += 512; /* Should accomodate private storage for sw models */

  /* nlm_device_init() trims the size if it is not a multiple
     of eight. We'd align it to 256-byte */
  size = (size + 0xff) & ~(0xff);
  if ((size & mask64_high32) != 0)
    return NLM_OUT_OF_MEMORY;

  /* nlm_device_init() insists on the following minimum.  */

  if (size < NLM_MIN_MEMORY_POOL_SIZE)  /* 32k is packet api minimum memory pool size */
    size = NLM_MIN_MEMORY_POOL_SIZE;

  *size_out = size;

  return NLM_OK;
}
