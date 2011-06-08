/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

/** \file nlm_logger.h
  header file for logger of compiler/database API */
#ifndef __NLM_LOGGER_H
#define __NLM_LOGGER_H

#include "nlm_stdint.h"

/** type of the message reported by 'xprint' callback */
typedef enum nlm_xprint_type
{
  NLM_XPRINT_WARNING,
  NLM_XPRINT_ERROR,
  NLM_XPRINT_DEBUG,
  NLM_XPRINT_VERBOSE,
  NLM_XPRINT_FATAL
} nlm_xprint_type;

/** module that called 'xprint' callback */
typedef enum nlm_xprint_module
{
  NLM_MODULE_UNKNOWN,
  NLM_MODULE_RX_PARSER,
  NLM_MODULE_FMS_GRAPH,
  NLM_MODULE_FMS_TAIL,
  NLM_MODULE_ESPRESSO,
  NLM_MODULE_FMS_PUT_PLACER,
  NLM_MODULE_FMS_PLACER,
  NLM_MODULE_DB_MANAGER
} nlm_xprint_module;

#define NLM_LOGGER_MAX_SPRINTF_BUF 128

/** proto of 'xprint' callback */
typedef void (*nlm_callback_type_xprint) (void *cookie, nlm_xprint_type type,
                                          nlm_xprint_module module, const char *str,
                                          unsigned int error_code, void *aux_data);

/** main logger structure.
    nlm_logger structure is used to setup an 'xprint' callback routine that
    may be called during compilation process to report error/warning/debug
    messages to the user */
typedef struct nlm_logger
{
  void *cookie;                                 /**< cookie - 1st argument to xprint callback */
  nlm_callback_type_xprint xprint;              /**< main callback routine */
  char sprintf_buf[NLM_LOGGER_MAX_SPRINTF_BUF]; /**< internal buffer used to construct a string */
} nlm_logger;

/** Generic auxiliary data for 'xprint' callback.
    the last 'aux_data' pointer of the 'xprint' callback can always be casted
    to 'struct nlm_logger_rule_data *' to retrieve rule and group id of the rule
    that triggered callback.
    In some cases 'aux_data' can be casted to 'nlm_log_fms_placer_rule_*'. See below */
struct nlm_logger_rule_data
{
  uint32_t rule_id;
  uint32_t group_id;
};

/* NLM_MODULE_FMS_PLACER error_code values for an nlm_logger callback.

   RULE_FATAL

      An otherwise undistinguished fatal error has occured during placement of
      a particular rule.  The auxiliary data struct (defined below) describes
      the rule.  A fatal error is an internal inconsistency within the
      compilation data structures.

   RULE_NOT_PLACED

      A valid rule element input graph could not be placed.  The auxiliary data
      struct (defined below) describes the rule.

   INPUT_GRAPH_INVALID

      The placer has been passed an input graph that is structurally invalid.
      This is an internal error of the graph builder.  The input graph is that
      of the rule element that is described in the auxiliary data.

   INPUT_GRAPH_TOO_BIG

      The placer has been passed an input graph that is structurally valid, but
      that is too big to be placed. (Either it is too big for the target device
      or it is too big relative to the current capabilities of the placer.) The
      input graph is that of the rule element that is described in the
      auxiliary data.
*/

typedef enum nlm_log_fms_placer
{
  NLM_LOG_FMS_PLACER_RULE_FATAL,
  NLM_LOG_FMS_PLACER_RULE_NOT_PLACED,
  NLM_LOG_FMS_PLACER_RULE_TRACE,
  NLM_LOG_FMS_PLACER_INPUT_GRAPH_INVALID,
  NLM_LOG_FMS_PLACER_INPUT_GRAPH_TOO_BIG
} nlm_log_fms_placer;

/* NLM_LOG_FMS_PLACER_RULE_FATAL auxiliary data.  */

struct nlm_log_fms_placer_rule_fatal
{
  struct nlm_logger_rule_data id;
};

/** Auxiliary data for 'xprint' callback from placer.
   This auxiliary data is supplied for error codes:

      NLM_LOG_FMS_PLACER_RULE_NOT_PLACED
      NLM_LOG_FMS_PLACER_RULE_TRACE
      NLM_LOG_FMS_PLACER_INPUT_GRAPH_INVALID
      NLM_LOG_FMS_PLACER_INPUT_GRAPH_TOO_BIG

   Note that these logs report the status of exactly one compilation
   rule element, as opposed to an entire user rule in the sense of
   nlm_database_add_rule().  In a compiled database, a single user rule
   may be decomposed into a collection of rule elements.  Each of these
   rule elements either does or does not place independently of all the
   other rule elements of the user rule.  */

struct nlm_log_fms_placer_rule_stat
{
  struct nlm_logger_rule_data id; /**< rule and group id of the rule that triggered callback */

  unsigned num_cams; /**< # cams in this rule element */
  unsigned num_hubs; /**< # hubs "   "    "      "    */
  unsigned num_ctrs; /**< # ctrs "   "    "           */
  unsigned num_sccs; /**< # SCCs "   "    "           */

  /* The following members give information relating to the cause of
     non-placement.  */

  uint32_t invalid_graph : 1; /**< invalid abstract graph */
  uint32_t too_big       : 1; /**< resources required exceed those of target device */

  /* The following member records the number of placement retries for this
     rule element, which is the number of times that the database placer has
     passed this rule to the block placer after the first passing failed to
     place the element.  The value is zero for an element which either has
     not been passed or has placed in the first passing.  */

  uint32_t retries; /**< number of placement retries  */

  /* The following member is the compiler's internal handle for the rule
     element that is being reported as not placing.  The user does not
     dereference this pointer but uses it in subsequent database probe
     calls which may require the pointer as a handle on the rule element.  */

  void *handle; /**< compiler-internal handle */
};

/* NLM_MODULE_FMS_GRAPH error_code values for an nlm_logger callback.

   NLM_LOG_FMS_GRAPH_INVALID_MML_FOR_REGEX

      For a rule with minimum match length(mml) specification, the mml value is
      invalid. e.g. if regex is abcd and mml=5

   NLM_LOG_FMS_GRAPH_INVALID_REGEX

      An invalid regex like ab^cd

   NLM_LOG_FMS_GRAPH_INVALID_ZERO_LENGTH_REGEX

      The regex can match empty string. e.g. a? a* ^.* but not ^.*$ .*$

   NLM_LOG_FMS_GRAPH_REGEX_HAS_UNSUPPORTED_CONSTRUCT

      The regex has some unsupported construct like a back reference or a word
      boundary(\b) or look-ahead/look-behind etc.

   NLM_LOG_FMS_GRAPH_REGEX_TOO_COMPLEX
      The graph is structurally invalid.  This is an internal error of the graph
      builder. It is the same as NLM_LOG_FMS_PLACER_INPUT_GRAPH_INVALID code of
      the placer.
*/

typedef enum nlm_log_fms_graph
{
  NLM_LOG_FMS_GRAPH_INVALID_MML_FOR_REGEX = 1,
  NLM_LOG_FMS_GRAPH_INVALID_REGEX,
  NLM_LOG_FMS_GRAPH_INVALID_ZERO_LENGTH_REGEX,
  NLM_LOG_FMS_GRAPH_REGEX_HAS_UNSUPPORTED_CONSTRUCT,
  NLM_LOG_FMS_GRAPH_REGEX_TOO_COMPLEX,
  NLM_LOG_FMS_GRAPH_SIZE_EXCEEDS_LIMIT
} nlm_log_fms_graph;

typedef enum nlm_log_fms_tail
{
  NLM_LOG_FMS_TAIL_FANOUT_TOO_HIGH = 1000,
  NLM_LOG_FMS_TAIL_OFFSET_TOO_LARGE,
  NLM_LOG_FMS_TAIL_INVALID_REGEX,
  NLM_LOG_FMS_TAIL_REDUNDANT_PREFIX,
  NLM_LOG_FMS_TAIL_TOO_MANY_START_STATES
} nlm_log_fms_tail;

/** default 'xprint' callback */
extern void default_xprint (void *cookie, nlm_xprint_type type, nlm_xprint_module module,
                            const char *str, unsigned int error_code, void *aux_data);
/** nop callback that ignores all arguments */
extern void nop_xprint (void *cookie, nlm_xprint_type type, nlm_xprint_module module,
                        const char *str, unsigned int error_code, void *aux_data);

/* different types of printf */
extern void nlm_logger_xprintf1 (nlm_logger *logger, const char *fmt, ...);
extern void nlm_logger_xprintf2 (nlm_logger *logger, nlm_xprint_type type, const char *fmt, ...);
extern void nlm_logger_xprintf3 (nlm_logger *logger, nlm_xprint_type type, nlm_xprint_module module,
                                 const char *fmt, ...);
extern void nlm_logger_xprintf4 (nlm_logger *logger, nlm_xprint_type type, nlm_xprint_module module,
                                 unsigned int error_code, const char *fmt, ...);
extern void nlm_logger_xprintf5 (nlm_logger *logger, nlm_xprint_type type, nlm_xprint_module module,
                                 unsigned int error_code, void *aux_data, const char *fmt, ...);

extern nlm_logger default_logger; /**< logger that prints messages to stdout */
extern nlm_logger nop_logger;     /**< logger that suppresses all messages */

#endif /* __NLM_LOGGER_H */

