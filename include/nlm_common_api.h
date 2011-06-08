/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

/** \file nlm_common_api.h
 header file for shared structures in Dataplane/packet API and Controlplane/database API */
#ifndef __NLM_COMMON_API_H
#define __NLM_COMMON_API_H

#include "nlm_stdint.h"

/** status and error codes returned by all API functions */
typedef enum nlm_status
{
#define NLM_INC_SEL(name,string) name,
#include "nlm_error_tbl.def"
#undef NLM_INC_SEL
  NLM_STATUS_LAST_UNUSED
} nlm_status;

/** function to convert status code to string 
  \param[in] status code to be converted
  \return string that describes the code */
const char *nlm_get_status_string (nlm_status status);

/** defines physical address for data plane library */
typedef unsigned long long nlm_phys_addr;

/* defines for database API */
#define NLM_FIRST_DATABASE_TYPE 1000
#define NLM_FIRST_DATABASE_PARAM 1100
#define NLM_FIRST_RULE_ACTION 1200
#define NLM_FIRST_PARSER_PARAM 1300
#define NLM_FIRST_MAPPER_PARAM 1400
#define NLM_FIRST_PLACER_PARAM 1400

/* defines for device API */
#define NLM_FIRST_DEVICE_TYPE 2000
#define NLM_FIRST_DEVICE_PARAM 2100
#define NLM_FIRST_DEVICE_CONFIG_PARAM 2200
#define NLM_FIRST_FLOW_TYPE 2300

#endif /* __NLM_COMMON_API_H */
