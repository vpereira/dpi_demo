/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "nlm_system.h"

#undef abort

void
pretty_abort (const char * prefix, const char *file, int line, const char *function)
{
  printf ("%s: in %s(), at %s:%d\n", prefix, function, file, line);
  abort ();
}

void
warning (const char *str)
{
  printf ("warning: %s\n", str);
}

void *
default_xmalloc (void *cookie, unsigned int byte_cnt)
{
  return malloc (byte_cnt);
}

void
default_xfree (void *cookie, void *mem_ptr)
{
  free (mem_ptr);
}

nlm_allocator default_allocator = {0, default_xmalloc, default_xfree};

void
nlm_logger_xprintf1 (nlm_logger *logger, const char *fmt, ...)
{
  va_list ap;
  pretty_assert (logger != NULL && fmt != NULL);
  if (logger->xprint == NULL || logger->xprint == nop_xprint)
    return;
  va_start (ap, fmt);
  vsnprintf (logger->sprintf_buf, NLM_LOGGER_MAX_SPRINTF_BUF, fmt, ap);
  va_end (ap);
  logger->xprint (logger->cookie, NLM_XPRINT_DEBUG, NLM_MODULE_UNKNOWN, logger->sprintf_buf, 0, NULL);
}

void
nlm_logger_xprintf2 (nlm_logger *logger, nlm_xprint_type type, const char *fmt, ...)
{
  va_list ap;
  pretty_assert (logger != NULL && fmt != NULL);
  va_start (ap, fmt);
  vsnprintf (logger->sprintf_buf, NLM_LOGGER_MAX_SPRINTF_BUF, fmt, ap);
  va_end (ap);
  logger->xprint (logger->cookie, type, NLM_MODULE_UNKNOWN, logger->sprintf_buf, 0, NULL);
}

void
nlm_logger_xprintf3 (nlm_logger *logger, nlm_xprint_type type, nlm_xprint_module module,
                     const char *fmt, ...)
{
  va_list ap;
  pretty_assert (logger != NULL && fmt != NULL);
  va_start (ap, fmt);
  vsnprintf (logger->sprintf_buf, NLM_LOGGER_MAX_SPRINTF_BUF, fmt, ap);
  va_end (ap);
  logger->xprint (logger->cookie, type, module, logger->sprintf_buf, 0, NULL);
}

void
nlm_logger_xprintf4 (nlm_logger *logger, nlm_xprint_type type, nlm_xprint_module module,
                     unsigned int error_code, const char *fmt, ...)
{
  va_list ap;
  pretty_assert (logger != NULL && fmt != NULL);
  va_start (ap, fmt);
  vsnprintf (logger->sprintf_buf, NLM_LOGGER_MAX_SPRINTF_BUF, fmt, ap);
  va_end (ap);
  logger->xprint (logger->cookie, type, module, logger->sprintf_buf, error_code, NULL);
}

void
nlm_logger_xprintf5 (nlm_logger *logger, nlm_xprint_type type, nlm_xprint_module module,
                     unsigned int error_code, void *aux_data, const char *fmt, ...)
{
  va_list ap;
  pretty_assert (logger != NULL && fmt != NULL);
  va_start (ap, fmt);
  vsnprintf (logger->sprintf_buf, NLM_LOGGER_MAX_SPRINTF_BUF, fmt, ap);
  va_end (ap);
  logger->xprint (logger->cookie, type, module, logger->sprintf_buf, error_code, aux_data);
}

static const char*
module_to_string (nlm_xprint_module module)
{
  switch (module)
    {
    case NLM_MODULE_RX_PARSER: return "parser";
    case NLM_MODULE_FMS_GRAPH: return "graph";
    case NLM_MODULE_ESPRESSO: return "espresso";
    case NLM_MODULE_FMS_PUT_PLACER: return "pplacer";
    case NLM_MODULE_FMS_PLACER: return "placer";
    case NLM_MODULE_DB_MANAGER: return "db_mgr";
    case NLM_MODULE_FMS_TAIL: return "tail";
    case NLM_MODULE_UNKNOWN:
    default: return "unknown";
    }
}

void 
default_xprint (void *cookie, nlm_xprint_type type, nlm_xprint_module module,
                const char *str, unsigned int error_code, void *aux_data)
{
  switch (type)
    {
    case NLM_XPRINT_WARNING:
      if ((module == NLM_MODULE_RX_PARSER
           || module == NLM_MODULE_FMS_GRAPH
           || module == NLM_MODULE_FMS_PUT_PLACER
           || module == NLM_MODULE_FMS_TAIL) && aux_data)
        {
          struct nlm_logger_rule_data *data = (struct nlm_logger_rule_data *) aux_data;
          fprintf (stdout, "Warning(%s,%04x,rule=%d,group=%d): %s\n",
                   module_to_string (module), error_code & 0xffff,
                   data->rule_id, data->group_id, str);
        }
      else
        fprintf (stdout, "Warning(%s,%04x): %s\n", module_to_string (module), error_code & 0xffff, str);
      break;
    case NLM_XPRINT_ERROR:
      if ((module == NLM_MODULE_RX_PARSER
           || module == NLM_MODULE_FMS_GRAPH
           || module == NLM_MODULE_FMS_PUT_PLACER
           || module == NLM_MODULE_FMS_TAIL) && aux_data)
        {
          struct nlm_logger_rule_data *data = (struct nlm_logger_rule_data *) aux_data;
          fprintf (stdout, "Error(%s,%04x,rule=%d,group=%d): %s\n",
                   module_to_string (module), error_code & 0xffff,
                   data->rule_id, data->group_id, str);
        }
      else
        fprintf (stdout, "Error(%s,%04x): %s\n", module_to_string (module), error_code & 0xffff, str);
      break;
    case NLM_XPRINT_VERBOSE:
    case NLM_XPRINT_DEBUG:
      fputs (str, stdout);
      break;
    case NLM_XPRINT_FATAL:
      fputs ("fatal: ", stdout);
      fputs (str, stdout);
      fputs ("\n", stdout);
      break;
    default:
      pretty_assert (0 && "invalid nlm_xprint_type");
    }

    /* Flush so that the output is sent even in the presence of a
       subsequent hang.  */

    fflush (stdout);
}

void 
nop_xprint (void *cookie, nlm_xprint_type type, nlm_xprint_module module,
            const char *str, unsigned int error_code, void *aux_data)
{
  /* doesn't do anything */
}

nlm_logger default_logger = {0, default_xprint, {0}};
nlm_logger nop_logger = {0, nop_xprint, {0}};

