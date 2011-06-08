/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include "nlm_system.h"
#include "nlm_common_api.h"

const char *
nlm_get_status_string (nlm_status status)
{
  static const char *status_strings[]={
#define NLM_INC_SEL(name,string) string,
#include "nlm_error_tbl.def"
#undef NLM_INC_DEF
    "should never be seen" /* NLM_STATUS_LAST_UNUSED */
  };

  if ((unsigned int) status > NLM_STATUS_LAST_UNUSED)
    return "invalid status code";

  pretty_assert (sizeof (status_strings) / sizeof (char *) == NLM_STATUS_LAST_UNUSED + 1);

  return status_strings[status];
}

