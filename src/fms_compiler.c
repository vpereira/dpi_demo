/*******************************************************************************
* Legal Notice:
* This product is the confidential property of NetLogic Microsystems Inc.
* ("NetLogic"), is provided under a non-disclosure agreement, and is protected
* under applicable copyright, patent, and trade secret laws. Unauthorized use,
* reproduction, distribution or other dissemination without the prior written
* authorization from NetLogic is strictly prohibited.
*
* NETLOGIC DISCLAIMS ALL WARRANTIES OF ANY NATURE, EXPRESS OR IMPLIED,
* INCLUDING, WITHOUT LIMITATION, THE WARRANTIES OF FITNESS FOR A PARTICULAR
* PURPOSE, MERCHANTABILITY AND/OR NON-INFRINGEMENT OF THIRD PARTY RIGHTS.
* NETLOGIC ASSUMES NO LIABILITY FOR ANY ERROR OR OMISSIONS IN THIS PRODUCT,OR
* FOR THE USE OF THIS PRODUCT. IN NO EVENT SHALL NETLOGIC BE LIABLE TO ANY OTHER
* PARTY FOR ANY SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL DAMAGES, WHETHER
* BASED ON BREACH OF CONTRACT, TORT, PRODUCT LIABILITY, INFRINGEMENT OF
* INTELLECTUAL PROPERTY RIGHTS OR OTHERWISE. NETLOGIC RESERVES THE RIGHT TO MAKE
* CHANGES TO, OR DISCONTINUE, ITS PRODUCTS AT ANY TIME.
*
* Distribution of the product herein does not convey a license or any other
* right in any patent, trademark, or other intellectual property of NetLogic.
*
* Use of the product shall serve as acceptance of these terms and conditions.
* If you do not accept these terms, you should return or destroy the product
* and any other accompanying information immediately.
*
* Copyright, 2008-2009, NetLogic Microsystems, Inc. All rights reserved.
*******************************************************************************/
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include "nlm_database_api.h"
#include "nlm_database_extra.h"
#include "nlm_system.h"

static void
print_usage (char *progname)
{
  printf ("Usage: %s [options] file...\n"
          "  -h, --help                    Print this help, then exit\n"
          "  -v, --version                 Print version number, then exit\n"
          "  -t, --type                    Device Type (fms/mars1/mars2/mars3)\n"
          "  -o FILE, --output=FILE        Compile binary image into FILE\n"
          "  -O NUM, --optimize=NUM        Compiler optimization level (default -O1)\n"
          "  -d NUM, --debug=NUM           Set debug flags to given integer value\n"
          "  -m NUM, --aggr-match-once=NUM Set aggressive match once parameter\n"
          "  -M NUM, --aggr-mml=NUM        Set aggressive mml parameter\n"
          "  -s NUM, --sym-limit=NUM       Set maximum limit of symbols per rule\n"
          "  -V, --verbose                 Increase compiler verbosity\n", progname);
  exit (0);
}

#define FGETS_BUF_SIZE 64*1024
static char buf[FGETS_BUF_SIZE];

static const struct option options[] =
{
  { "help",     no_argument, NULL, 'h' },
  { "version",  no_argument, NULL, 'v' },
  { "verbose",  no_argument, NULL, 'V' },
  { "type",            required_argument, NULL, 't' },
  { "debug",           required_argument, NULL, 'd' },
  { "output",          required_argument, NULL, 'o' },
  { "optimize",        required_argument, NULL, 'O' },
  { "sym-limit",       required_argument, NULL, 's' },
  { "aggr-mml",        required_argument, NULL, 'M' },
  { "aggr-match-once", required_argument, NULL, 'm' },
  { 0, 0, 0, 0 }
};

static int flag_verbose = 0;
static int flag_optimize = 1;
static int flag_debug = 0;
static int flag_aggr_match_once = 1;
static int flag_aggr_mml = 1;
static int flag_symbol_limit = 0;
static char *file_name = "data.bin";
static nlm_database_type database_type = NLM_DATABASE_HW_FMS;

/* process args, return index to the first non-arg */
static int
process_args (int argc, char **argv)
{
  int opt;

  while ((opt = getopt_long (argc, argv, "d:hO:o:vVm:M:s:t:", options, NULL)) != -1)
    {
      switch (opt)
        {
        case 'v':
          printf ("%s version 2.5.1\n", argv[0]);
          exit (0);

        case 'V':
          flag_verbose++;
          break;

        case 'O':
          flag_optimize = atoi (optarg);
          break;

        case 'd':
          flag_debug = atoi (optarg);
          break;

        case 's':
          flag_symbol_limit = atoi (optarg);
          break;

        case 'm':
          flag_aggr_match_once = atoi (optarg);
          break;

        case 'M':
          flag_aggr_mml = atoi (optarg);
          break;

        case 'o':
          file_name = optarg;
          break;

        case 't':
          if ((strcmp (optarg, "mars1")) == 0 )
            database_type = NLM_DATABASE_HW_MARS1;
          else if ((strcmp (optarg, "mars2")) == 0)
            database_type = NLM_DATABASE_HW_MARS2;
          else if ((strcmp (optarg, "fms")) == 0)
            database_type = NLM_DATABASE_HW_FMS;
          else if ((strcmp (optarg, "mars3")) == 0)
            database_type = NLM_DATABASE_HW_MARS3;
          else
            print_usage (argv[0]);
          break;

        case 'h':
        default:
          print_usage (argv[0]);
        }
    }
  return optind;
}

int
main (int argc, char **argv)
{
  FILE *fd;
  struct nlm_database *db;
  uint32_t line_num = 0;
  const void *db_image;
  uint32_t db_size;
  int index;
  nlm_status status;
  struct nlm_database_stat *database_stat;

  index = process_args (argc, argv);

  if (index == argc)
    print_usage (argv[0]);

  fd = fopen (argv[index], "r");
  if (fd == NULL)
    {
      printf ("%s: %s\n", argv[index], strerror (errno));
      exit (2);
    }

  db = malloc (nlm_database_sizeof (database_type));

  if (db == NULL)
    {
      printf ("out of memory\n");
      exit (3);
    }

  TRY (nlm_database_interface_init (database_type, db));

  if (flag_verbose)
    TRY (nlm_database_set_param (db, NLM_VERBOSITY_LEVEL, flag_verbose));

  TRY (nlm_database_set_param (db, NLM_OPTIMIZATION_LEVEL, flag_optimize));

  if (database_type == NLM_DATABASE_HW_FMS || database_type == NLM_DATABASE_HW_MARS3)
    {

      TRY (nlm_database_set_param (db, NLM_MAPPER_PARAM, NLM_DO_AGGRESSIVE_MML,
                                   flag_aggr_mml));
      TRY (nlm_database_set_param (db, NLM_MAPPER_PARAM, NLM_DO_AGGRESSIVE_MATCH_ONCE,
                                   flag_aggr_match_once));
      TRY (nlm_database_set_param (db, NLM_MAPPER_PARAM, NLM_RULE_SYMBOL_LIMIT,
                                   flag_symbol_limit));
      if (flag_debug)
        TRY (nlm_database_set_param (db, NLM_DEBUG_FLAGS, flag_debug));
    }

  TRY (nlm_database_open (db));

  while (!feof (fd))
    {
      char *str = fgets (buf, sizeof (buf), fd);
      line_num ++;
      if (str == 0)
        break;

      if (nlm_db_parse_line (db, &default_logger, line_num, str, str + strlen (str)) != NLM_OK)
        break;
    }

  /* compile it */
  status = nlm_database_compile (db, &db_image, &db_size);

  if (status != NLM_OK && status != NLM_PARTIAL_COMPILE)
    return status;

  if (status == NLM_PARTIAL_COMPILE)
    printf ("Partial compilation. Some rules were not compiled.\n");
  if (database_type == NLM_DATABASE_HW_FMS)
    {
      TRY (nlm_database_get_param (db, NLM_DATABASE_STAT, &database_stat));

      if (flag_verbose)
        printf ("Placed %d out of %d graphs (expanded from original %d rules) into %d blocks\n",
                database_stat->placed_rule_cnt, database_stat->graphed_rule_cnt,
                database_stat->parsed_rule_cnt, database_stat->block_cnt);
    }

  do
    {
      FILE *fp = fopen (file_name, "w");
      if (fp == NULL)
        {
          printf ("Couldn't open the file %s\n", file_name);
          break;
        }

      if (fwrite (db_image, 1, db_size, fp) != db_size)
        {
          printf ("Couldn't write into the file %s\n", file_name);
        }
      fclose (fp);
    }
  while (0);

  TRY (nlm_database_close (db));
  TRY (nlm_database_interface_fini (db));
  free (db);

  return 0;
}
