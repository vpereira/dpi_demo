/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

/** \file nlm_database_api.h
  header file for Controlplane/database API */
#ifndef NLM_DATABASE_API_H
#define NLM_DATABASE_API_H

#include "nlm_common_api.h"

/* error/warning reporting mechanism setup as callback functions from the compiler */
#include "nlm_logger.h"

struct nlm_database;   /* opaque database handle */
struct nlm_rule_group; /* opaque rule_group handle */
struct nlm_rule;       /* opaque rule handle */

/** type of the database */
typedef enum nlm_database_type
{
  NLM_INVALID_DATABASE_TYPE = NLM_FIRST_DATABASE_TYPE, /**< marker for invalid database type */
  NLM_DATABASE_HW_MARS1,     /**< NLS055 */
  NLM_DATABASE_HW_MARS2,     /**< NLS205 */
  NLM_DATABASE_HW_FMS,       /**< NLS2008 */
  NLM_DATABASE_HW_MARS3,     /**< NLS2018 */
  NLM_DATABASE_SW_HORIZON,
  NLM_DATABASE_SW_FMS_MODEL,
  NLM_DATABASE_SW_MARS3_MODEL,
  NLM_LAST_DATABASE_TYPE
} nlm_database_type;

/** returns size of nlm_database (database handle structure) */
uint32_t nlm_database_sizeof (nlm_database_type type);

/** parser flags */
typedef enum nlm_parser_param
{
  NLM_INVALID_PARSER_PARAM = NLM_FIRST_PARSER_PARAM,
  NLM_SYNTAX_PCRE,
  NLM_SYNTAX_PCRE_EXTENDED,
  NLM_SYNTAX_PCRE_IGNORE_CASE,
  NLM_SYNTAX_PCRE_NON_GREEDY,
  NLM_SYNTAX_NETSCREEN,
  NLM_SYNTAX_NETSCREEN_NON_GREEDY,
  NLM_LAST_PARSER_PARAM
} nlm_parser_param;

/** mapper flags */
typedef enum nlm_mapper_param
{
  NLM_INVALID_MAPPER_PARAM = NLM_FIRST_MAPPER_PARAM,
  NLM_DO_AGGRESSIVE_MML,   /**< should mml graphs be constructed even for
                                unsafe rules(e.g. rules with overlapping prefix & suffix) */
  NLM_DO_AGGRESSIVE_MATCH_ONCE, /**< compile certain rules in match once mode instead
                                     of all-matches to improve HW capacity */
  NLM_RULE_SYMBOL_LIMIT,        /**< don't compile rules with symbol count larger than this limit,
                                     (default 0 == compile rules of all sizes) */
  NLM_REMOVE_REDUNDANCY,        /**< remove the prefix and suffix redundant quantifiers based on this
                                     value. (default 3 == remove both) */
  NLM_STITCH_RULES,             /**< enable/disable stitcher based on this value. (default 1 == enabled ) */
  NLM_USE_COUNTER,             /**< enable/disable counter usage based on this value. (default 1 == enabled ) */
  NLM_LAST_MAPPER_PARAM
} nlm_mapper_param;

struct nlm_database_stat
{
  uint32_t parsed_rule_cnt;    /**< number of rules successfully parsed */
  uint32_t graphed_rule_cnt;   /**< number of rules for which graph was successfully generated */
  uint32_t placed_rule_cnt;    /**< number of rules successfully placed */
  uint32_t block_cnt;          /**< number of blocks into which the database has compiled */
};

/** function prototype for callback accessed by nlm_database_set_param (db, NLM_XMALLOC, malloc_callback);
    and nlm_database_set_param (db, NLM_IMAGE_XMALLOC, image_malloc_callback); */
typedef void *(*nlm_callback_type_xmalloc) (void *cookie, uint32_t byte_cnt);
/** proto for NLM_XFREE */
typedef void (*nlm_callback_type_xfree) (void *cookie, void *mem_ptr);

/** configuration parameters that can be set and get by corresponding functions */
typedef enum nlm_database_param
{
  NLM_INVALID_DATABASE_PARAM = NLM_FIRST_DATABASE_PARAM, /**< marker for invalid parameter */
  NLM_OPTIMIZATION_LEVEL,  /**< optimization level: 0 - no optimization */
  NLM_VERBOSITY_LEVEL,     /**< verbosity level: 0 - no extra messages */
  NLM_PARSER_PARAM,        /**< parser flags (one of nlm_parser_param) */
  NLM_MAPPER_PARAM,        /**< mapper flags */
  NLM_DEBUG_FLAGS,         /**< bitmap to control dumping of internal structures */
  NLM_CALLBACK_COOKIE,     /**< 'void *cookie' to be sent to xmalloc, xfree, xdump callbacks */
  NLM_IMAGE_XMALLOC,       /**< void * (*xmalloc) (void *cookie, uint32_t byte_cnt); callback */
  NLM_XMALLOC,             /**< void * (*xmalloc) (void *cookie, uint32_t byte_cnt); callback */
  NLM_XFREE,               /**< void (*xfree) (void *cookie, void *mem_ptr); callback */
  NLM_LOGGER,              /**< pointer to the nlm_logger */
  NLM_DATABASE_SW_MODEL_TYPE, /**< sub-type of software model */
  NLM_DATABASE_STAT,       /**< probe function to inquire the status of a compiled database.
                                It is valid to call the function after any non-fatal return from
                                nlm_database_compile() but before the nlm_database_close() */
  NLM_LAST_DATABASE_PARAM
} nlm_database_param;

/** init interfaces of the database compiler
  \param[in] type of database to be initialized
  \param[in] database pointer to database handle
  \return status */
nlm_status nlm_database_interface_init (nlm_database_type type, struct nlm_database *database);

/** shutdown interfaces of the database compiler
  \param[in] database pointer to database handle
  \return status */
nlm_status nlm_database_interface_fini (struct nlm_database *database);

/** Get specified database parameter
  \param[in] database pointer to database handle
  \param[in] param database parameter
  \return status */
nlm_status nlm_database_get_param (struct nlm_database *database, nlm_database_param param, ...);

/** Set specified database parameter
  \param[in] database pointer to database handle
  \param[in] param database parameter
  \return status */
nlm_status nlm_database_set_param (struct nlm_database *database, nlm_database_param param, ...);

/** Open and initialize database compiler
  \param[in] database pointer to database handle
  \return status */
nlm_status nlm_database_open (struct nlm_database *database);

/** Close and free database of rules
  \param[in] database pointer
  \return status */
nlm_status nlm_database_close (struct nlm_database *database);

/** Create and add rule group to the \a database
  \param[in] database pointer
  \param[in] group_id id of the rule group
  \param[out] p_group rule group handle
  \return status */
nlm_status nlm_database_add_group (struct nlm_database *database,
                                   uint32_t group_id,
                                   struct nlm_rule_group **p_group);

/** Create and add \a rule to the rule \a group
  \param[in] database pointer
  \param[in] group rule group handle
  \param[in] rule_id id of the rule
  \param[in] regex rule to be parsed and compiled
  \param[out] p_rule rule handle
  \return status */
nlm_status nlm_database_add_rule (struct nlm_database *database, struct nlm_rule_group *group,
                                  uint32_t rule_id, const char *regex,
                                  struct nlm_rule **p_rule);


/** rule actions to be set */
typedef enum nlm_rule_action
{
  NLM_INVALID_RULE_ACTION = NLM_FIRST_RULE_ACTION, /**< marker for invalid action */
  NLM_MIN_MATCH_LENGTH,   /**< match must meet the minimum match length criterion
                            to be considered a match */
  NLM_MIN_OFFSET,         /**< offset for the end of the match must meet the
                            specified offset criterion */
  NLM_EXACT_OFFSET,       /**< offset for the end of the match must meet the
                            specified offset criterion */
  NLM_MAX_OFFSET,         /**< offset for the end of the match must meet the
                            specified offset criterion */
  NLM_RESULT_PRIORITY,    /**< specifies priority of the rule to be clamped
                            differently by dataplane */
  NLM_NON_GREEDY,         /**< cause rule to match nongreedy (weak nongreedy) */
  NLM_MATCH_ONCE,         /**< cause rule to match just once (strong nongreedy)
                            (only first match reported) */
  NLM_TRIGGER_GROUP,         /**< cause rule to activate the linked group without output */
  NLM_MATCH_AND_TRIGGER_GROUP, /**< cause rule to activate the linked group with output */

  NLM_LAST_RULE_ACTION
} nlm_rule_action;

/** Set rule action
  \param[in] database pointer
  \param[in] rule handle
  \param[in] action rule action type
  \return status */
nlm_status nlm_database_set_rule_action (struct nlm_database *database,
                                         struct nlm_rule *rule,
                                         nlm_rule_action action, ...);

/** Compile the whole database
  \param[in] database pointer
  \param[out] p_database pointer to the compiled image
  \param[out] p_size compiled image size
  \return status */
nlm_status nlm_database_compile (struct nlm_database *database,
                                 const void **p_database,
                                 uint32_t *p_size);

#define NLM_MIN_RESULT_PRIORITY 0
#define NLM_MAX_RESULT_PRIORITY 3

#endif /* NLM_DATABASE_API_H */

