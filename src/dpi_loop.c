/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <time.h>
#include <getopt.h>
#define _XOPEN_SOURCE 500     /* Or: #define _BSD_SOURCE */
#include <unistd.h>
#include <pcap.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>

#include "nlm_packet_api.h"
#include "nlm_packet_api_impl.h"
#include "nlm_driver_api.h"
#include "nlm_database_api.h"
#include "nlm_system.h"
#include "nlm_fms_formats.h"

#define MAX_FLOWS 96

static uint32_t flag_numflows = 32;
static uint32_t flag_group_id = 1;
static uint32_t flag_rule_id = 1;

uint32_t flag_verbose = 0;
uint32_t group_id = 1;
char *db_file = NULL;
uint32_t min_packet_size = 64;
uint32_t max_packet_size = 32768;
static char *input_data_file = NULL;
static uint32_t pd_linking_off = 0;
#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2)
static uint32_t num_of_copies = 0;
#else
static uint32_t num_of_copies = 1;
#endif
static uint32_t num_of_flows = 1;
static uint32_t ddr_size = 0;
static uint32_t flag_nongreedy = 0;
static uint32_t flag_mml = 0;
static uint32_t flag_flow_test = 0;
static double clock_speed = 0;
static double engine_clock_skew = 0;
static double scale_ratio = 0;
static uint32_t flag_spill_size = 0x180000;

/* multi ring related flags */
uint32_t do_attach = 0;
uint32_t attached_ring_id = 0;
uint32_t is_master = 0;
enum RING_MODE {
  RING_MODE_RUN_ON_MASTER,
  RING_MODE_MASTER_ONLY,
  RING_MODE_ATTACH_ONLY,
  RING_MODE_MASTER_AND_ATTACH
};
enum RING_MODE ring_mode;


char rule_str[]="abcd";
unsigned char eval_str[64]="abcd";

static char *regex_str = rule_str;

/* Program wide parameters */
static const struct option options[] =
{
  { "attach",     required_argument,  NULL, 'a' },
  { "database",   required_argument,  NULL, 'd' },
  { "ddr",        required_argument,  NULL, 'D' },
  { "flow-test",  no_argument,        NULL, 'F' },
  { "flows",      required_argument,  NULL, 'f' },
  { "groupid",    required_argument,  NULL, 'g' },
  { "help",       no_argument,        NULL, 'h' },
  { "input",      required_argument,  NULL, 'i' },
  { "linking",    no_argument,        NULL, 'L' },
  { "master",     no_argument,        NULL, 'm' },
  { "mml",        required_argument,  NULL, 'M' },
  { "nongreedy",  no_argument,        NULL, 'N' },
  { "num-copies", required_argument,  NULL, 'c' },
  { "packet-min", required_argument,  NULL, 'p' },
  { "packet-max", required_argument,  NULL, 'P' },
  { "regex",      required_argument,  NULL, 'r' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "ext-size",   required_argument,  NULL, 'e' },
  { 0, 0, 0, 0 }
};

nlm_status
load_db_from_file (struct nlm_device *device, const char *file, uint32_t db_id)
{
  uint32_t dbsize;
  uint8_t *database;
  nlm_status status;
  FILE *img;

  img = fopen (file, "r");
  if (img == NULL)
    {
      printf ("\033[31mSorry, unable to open rule file '%s'\033[0m", file);
      exit (1);
    }

  /* read the database into the memory */
  if (fseek (img, 0, SEEK_END))
    {
      printf ("\033[31mfseek failed\033[0m");
      exit (1);
    }

  dbsize = ftell (img);
  rewind (img);

  if ((database = malloc (dbsize)) == NULL)
    {
      printf ("\033[31mcouldn't allocate memory\033[0m");
      exit (1);
    }

  if (fread (database, sizeof (uint8_t), dbsize, img) != dbsize)
    {
      printf ("\033[31mcan't read database file\033[0m");
      exit (1);
    }

  if (fclose (img))
    {
      printf ("\033[31mcan't close file\033[0m");
      exit (1);
    }

  status = nlm_device_try_load_database (device, database, dbsize, (uint32_t) -1, db_id, num_of_copies);
  if (status != NLM_OK)
    {
      printf ("try_load_database error: %s\n", nlm_get_status_string (status));
      free (database);
      return status;
    }

  /* asynchronous request to start loading the database */
  status = nlm_device_load_database (device, database, dbsize, db_id, num_of_copies);

  /* all other flows can be searched normally at this time,
     we will poll here until our requested database is fully loaded,
     since we want to search it immediately */
  while (status)
    if (status != NLM_LOADER_IN_PROGRESS)
      {
        printf ("load_database error: %s\n", nlm_get_status_string (status));
        exit (status);
      }
    else
      TRY (nlm_device_get_param (device, NLM_DATABASE_LOADER_STATUS, &status));

  /* free it, since HW already stored it inside */
  free (database);
  return NLM_OK;
}

static void
print_usage (char *progname)
{
  printf ("Usage: %s [options]\n"
          "  -a, --attach <id> Attach to ring <id>. <id> must be zero when used with --master.\n"
          "                   When <id> is nonzero, run --master and --attach on different terminals\n"
          "  -d, --database   File that contains the image of the rule database to load\n"
          "  -D, --ddr        Use attached ddr. Size of ddr must be specified\n"
          "  -f, --flows      Number of flows to be run in parallel (default is 1)\n"
          "  -F, --flow-test  Allocate as many flows as possible and run some traffic\n"
          "  -g, --groupid    The group ID to be searched (default is 1)\n"
          "  -h, --help       Print this help, then exit\n"
          "  -i, --input      Input file to search\n"
          "  -L, --linking    Turn off PD-linking\n"
          "  -m, --master     Run on master ring. When used with --attach, ring <id> must be zero\n"
          "  -M, --mml        Apply min-match-length filtering to regex specified with -r\n"
          "  -N, --nongreedy  Compile regex specified with -r with nongreedy flag\n"
          "  -c, --num-copies Num of copies of the database to load (default is 1)\n"
          "  -p, --packet-min Packet size to start measurements from (default is 64)\n"
          "  -P, --packet-max Packet size to end measurements at (default is 32768)\n"
          "  -r, --regex      Specifies regex pattern to search\n"
          "  -v, --verbose    Verbose the output by printing matches\n"
          "  -e, --ext-size   Size of database in MB\n",
          progname);
  exit (1);
}

/* process args, return index to the first non-arg */
static int
process_args (int ac, char **av)
{
  int opt;

  while ((opt = getopt_long (ac, av, "mhve:g:d:p:r:i:P:LM:Nc:f:D:a:F", options, NULL)) != -1)
    {
      switch (opt)
        {
        case 'v':
          flag_verbose++;
          break;

        case 'g':
          flag_group_id = atoi (optarg);
          if (flag_group_id == 0 || flag_group_id > 1023)
            {
              printf ("\033[31mGroup id should be a number.\033[0m\n");
              exit (1);
            }
          break;

        case 'f':
          num_of_flows = atoi (optarg);
          if (num_of_flows == 0 || num_of_flows > MAX_FLOWS)
            {
              printf ("\033[31mInvalid number of flows.\033[0m\n");
              exit (1);
            }
          break;

        case 'e':
          flag_spill_size = atoi (optarg) * 1024 * 1024;
          break;

        case 'F':
          flag_flow_test = 1;
          break;

        case 'c':
          num_of_copies = atoi (optarg);
          if (num_of_copies > 4)
            {
              printf ("\033[31mNumber of database copies can be 0, 1, 2, 3, 4.\033[0m\n");
              exit (1);
            }
          break;

        case 'D':
          ddr_size = atoi (optarg);
          if (ddr_size == 0)
            {
              printf ("\033[31mDDR size specified is invalid\033[0m\n");
              exit(1);
            }
          break;

        case 'p':
          min_packet_size = atoi (optarg);
          if (min_packet_size == 0 || min_packet_size > 65535)
            {
              printf ("\033[31mIncorrect packet size.\033[0m\n");
              exit (1);
            }
          break;

        case 'P':
          max_packet_size = atoi (optarg);
          if (max_packet_size == 0 || max_packet_size > 65535)
            {
              printf ("\033[31mIncorrect maximum packet size.\033[0m\n");
              exit (1);
            }
          break;

        case 'd':
          db_file = optarg;
          break;

        case 'r':
          regex_str = optarg;
          break;

        case 'i':
          input_data_file = optarg;
          break;

        case 'L':
          pd_linking_off = 1;
          break;

        case 'N':
          if (flag_mml)
            {
              printf ("Cannot use both nongreedy and mml flags at the same time\n");
              exit (1);
            }
          flag_nongreedy = 1;
          break;

        case 'M':
          if (flag_nongreedy)
            {
              printf ("Cannot use both nongreedy and mml flags at the same time\n");
              exit (1);
            }
          flag_mml = atoi (optarg);
          if (flag_mml == 0 || flag_mml > 65535)
            {
              printf ("Incorrect min-match-length filtering\n");
              exit (1);
            }
          break;

        case 'm':
          is_master = 1;
          break;

        case 'a':
          do_attach = 1;
          attached_ring_id = atoi (optarg);
          if (attached_ring_id > 3)
            {
              printf ("Attached ring number must be <= 3.\n");
              exit (1);
            }
          break;

        case 'h':
        default:
          print_usage (av[0]);
        }
    }

  if (is_master == 1 && do_attach == 1 && attached_ring_id != 0)
    {
      printf ("--master can only be used to attach to ring 0.\n");
      exit (1);
    }

  if (is_master == 0 && do_attach == 1 && attached_ring_id == 0)
    {
      printf ("Cannot --attach to ring 0 without also specifying --master\n");
      exit (1);
    }

  if (is_master == 0 && do_attach == 0)
    ring_mode = RING_MODE_RUN_ON_MASTER;
  else if (is_master == 0 && do_attach == 1)
    ring_mode = RING_MODE_ATTACH_ONLY;
  else if (is_master == 1 && do_attach == 0)
    ring_mode = RING_MODE_MASTER_ONLY;
  else
    ring_mode = RING_MODE_MASTER_AND_ATTACH;

  return optind;
}

#ifndef NO_COMPILER
/* global vars for demo simplicity */
static const void *db_image;
static uint32_t db_size;
static struct nlm_database *db;
#endif

static nlm_status
compile_db (void)
{
#ifdef NO_COMPILER
  return NLM_OK;
#else
  struct nlm_rule_group *group;
  struct nlm_rule *rule;

  db = malloc (nlm_database_sizeof (NLM_DATABASE_HW_FMS));
  if (db == NULL)
    {
      printf ("out of memory\n");
      exit (3);
    }

  TRY (nlm_database_interface_init (NLM_DATABASE_HW_FMS, db));
  TRY (nlm_database_set_param (db, NLM_OPTIMIZATION_LEVEL, 2));
  TRY (nlm_database_open (db));
  TRY (nlm_database_add_group (db, flag_group_id, &group));

  if (flag_nongreedy)
    nlm_database_set_param (db, NLM_PARSER_PARAM, NLM_SYNTAX_PCRE_NON_GREEDY);
  else
    nlm_database_set_param (db, NLM_PARSER_PARAM, NLM_SYNTAX_PCRE);

  TRY (nlm_database_add_rule (db, group, flag_rule_id/*rule_id*/, regex_str, &rule));

  if (flag_mml)
    TRY (nlm_database_set_rule_action (db, rule, NLM_MIN_MATCH_LENGTH, flag_mml))

  TRY (nlm_database_compile (db, &db_image, &db_size));
  return NLM_OK;
#endif
}

static nlm_status
free_db (void)
{
#ifdef NO_COMPILER
  return NLM_OK;
#else
  TRY (nlm_database_close (db));
  TRY (nlm_database_interface_fini (db));
  free (db);
  return NLM_OK;
#endif
}

static nlm_status
load_db (struct nlm_device *device, uint32_t db_id)
{
#ifdef NO_COMPILER
  DDPRINT ("This demo is built without builtin compiler. Please specify -d to load the database\n");
  return NLM_UNSUPPORTED;
#else
  nlm_status status;
  TRY (nlm_device_try_load_database (device, db_image, db_size, (uint32_t) -1, db_id, num_of_copies));

  /* asynchronous request to start loading the database */
  status = nlm_device_load_database (device, db_image, db_size, db_id, num_of_copies);

  /* all other flows can be searched normally at this time,
     we will poll here until our requested database is fully loaded,
     since we want to search it immediately */
  while (status)
    if (status != NLM_LOADER_IN_PROGRESS)
      {
        printf ("load_database error: %s\n", nlm_get_status_string (status));
        exit (status);
      }
    else
      TRY (nlm_device_get_param (device, NLM_DATABASE_LOADER_STATUS, &status));

  return NLM_OK;
#endif
}

static nlm_status
init_packet_memory (char *packet_memory, uint32_t mem_size)
{
  int i;

  memset (packet_memory, 0, mem_size);

  if (input_data_file)
    {
      int size;
      FILE *fp;

      if (input_data_file[0] == '-' && input_data_file[1] == 0)
        fp = stdin;
      else
        {
          fp = fopen (input_data_file, "r");
          if (fp == NULL)
            {
              printf ("Couldn't open file %s\n", input_data_file);
              return NLM_INVALID_ARGUMENT;
            }
        }

      size = fread (packet_memory, 1, mem_size, fp);

      if (fp != stdin)
        fclose (fp);

      if (flag_verbose)
        {
          if (size < mem_size)
            printf ("All bytes of file '%s' was read into packet memory\n", input_data_file);
          else
            printf ("First '%d' bytes of file '%s' was read into packet memory\n", mem_size,
                    input_data_file);
        }
    }
  else if (db_file == NULL)
    for (i = 0; i < mem_size - sizeof (eval_str); i += sizeof (eval_str))
      {
        memcpy (packet_memory + i, eval_str, sizeof (eval_str));
      }
  else
	  memset (packet_memory, 0xaa, mem_size);

  return NLM_OK;
}

#define REG_WRITE(ADDR, DATA)  TRY (nlm_device_set_param (device, NLM_HW_REGISTER, ADDR, DATA ))
#define REG_READ(ADDR, RESULT) TRY (nlm_device_get_param (device, NLM_HW_REGISTER, ADDR, &RESULT))

#define CLEAR_AND_START_COUNTERS  REG_WRITE (0x5804, 2); \
                                  REG_WRITE (0x5800, 0xffff0000); \
                                  REG_WRITE (0x5800, 0xffff)

#define STOP_COUNTERS REG_WRITE (0x5800, 0x0)

#define BLOCK_REG_WRITE(ADDR, DATA) \
  { uint32_t __reg; \
    REG_WRITE (0x5000 /* fms_block_addr */, ADDR); \
    REG_WRITE (0x5004 /* fms_block_data0 */, DATA); \
    REG_WRITE (0x5010 /* fms_block_ctrl */, 1 /* write */); \
    do \
      __reg = 0; /*REG_READ (0x5010, __reg)*/ \
    while (__reg & 1); \
  }

#define BLOCK_REG_READ(ADDR, RESULT) \
  { uint32_t __reg; \
    REG_WRITE (0x5000 /* fms_block_addr */, ADDR); \
    REG_WRITE (0x5010 /* fms_block_ctrl */, 2 /* read */); \
    do \
      REG_READ (0x5010, __reg) \
    while (__reg & 2); \
    REG_READ (0x5004 /* fms_block_data0 */, RESULT); \
  }

static uint16_t
compute_next_lfsr (uint16_t value)
{
  union __lfsr
    {
      uint16_t lfsr;
      struct bits
        {
           uint16_t bit0:1;
           uint16_t bit1:1;
           uint16_t bit2:1;
           uint16_t bit3:1;
           uint16_t bit4:1;
           uint16_t bit5:1;
           uint16_t bit6:1;
           uint16_t bit7:1;
           uint16_t bit8:1;
           uint16_t bit9:1;
           uint16_t bit10:1;
           uint16_t bit11:1;
           uint16_t bit12:1;
           uint16_t bit13:1;
           uint16_t bit14:1;
           uint16_t bit15:1;
        } s;
    } in, out;
  in.lfsr = value;

  out.s.bit0 = in.s.bit0 ^ in.s.bit1 ^ in.s.bit2 ^ in.s.bit3 ^ in.s.bit4 ^ in.s.bit5 ^ in.s.bit6
               ^ in.s.bit7 ^ in.s.bit8 ^ in.s.bit9 ^ in.s.bit10 ^ in.s.bit11 ^ in.s.bit12
               ^ in.s.bit13 ^ in.s.bit15;
  out.s.bit1 = in.s.bit1 ^ in.s.bit2 ^ in.s.bit3 ^ in.s.bit4 ^ in.s.bit5 ^ in.s.bit6 ^ in.s.bit7
               ^ in.s.bit8 ^ in.s.bit9 ^ in.s.bit10 ^ in.s.bit11 ^ in.s.bit12 ^ in.s.bit13 ^ in.s.bit14;
  out.s.bit2 = in.s.bit0 ^ in.s.bit1 ^ in.s.bit14;
  out.s.bit3 = in.s.bit1 ^ in.s.bit2 ^ in.s.bit15;
  out.s.bit4 = in.s.bit2 ^ in.s.bit3;
  out.s.bit5 = in.s.bit3 ^ in.s.bit4;
  out.s.bit6 = in.s.bit4 ^ in.s.bit5;
  out.s.bit7 = in.s.bit5 ^ in.s.bit6;
  out.s.bit8 = in.s.bit6 ^ in.s.bit7;
  out.s.bit9 = in.s.bit7 ^ in.s.bit8;
  out.s.bit10 = in.s.bit8 ^ in.s.bit9;
  out.s.bit11 = in.s.bit9 ^ in.s.bit10;
  out.s.bit12 = in.s.bit10 ^ in.s.bit11;
  out.s.bit13 = in.s.bit11 ^ in.s.bit12;
  out.s.bit14 = in.s.bit12 ^ in.s.bit13;
  out.s.bit15 = in.s.bit0 ^ in.s.bit1 ^ in.s.bit2 ^ in.s.bit3 ^ in.s.bit4 ^ in.s.bit5
                ^ in.s.bit6 ^ in.s.bit7 ^ in.s.bit8 ^ in.s.bit9 ^ in.s.bit10 ^ in.s.bit11
                ^ in.s.bit12 ^ in.s.bit14 ^ in.s.bit15;
  return out.lfsr;
}

static nlm_status
detect_clock_skew (struct nlm_device *device)
{
  uint32_t lfsr_start = 0, lfsr_end = 0;
  int i;
  uint16_t lfsr;
  uint64_t start_sclock = 0, end_sclock = 0;

  /* start lfsr */
  BLOCK_REG_WRITE (0x800060 /* lfsr */, 1 /* start */);

  TRY (nlm_device_get_param (device, NLM_TIMESTAMP, &start_sclock));

  lfsr_start = 1;
  /* BLOCK_REG_READ (0x800060, lfsr_start); */

  usleep (300);

  TRY (nlm_device_get_param (device, NLM_TIMESTAMP, &end_sclock));
  BLOCK_REG_READ (0x800060, lfsr_end);

  BLOCK_REG_WRITE (0x800060 /* lfsr */, 0 /* stop */);

  for (i = 0, lfsr = lfsr_start; lfsr != lfsr_end; i++, lfsr = compute_next_lfsr (lfsr));

  engine_clock_skew = i * 1. / (end_sclock - start_sclock);

  if (flag_verbose)
    DDPRINT ("Engine clock skew is %1.3f (over %d iterations)\n", engine_clock_skew, i);

  return NLM_OK;
}

static nlm_status
detect_clock_speed (struct nlm_device *device)
{
#if defined (NLM_HW_MARS1)
  /* mars1 doesn't have counters */
  clock_speed = 133.33;
#else
  struct timeval tv;
  uint32_t start_time, end_time;
  uint64_t start_sclock = 0, end_sclock = 0;

  if (clock_speed)
    return NLM_OK;

  /* use HW counters to measure system clock speed */

  gettimeofday (&tv, 0);
  start_time = tv.tv_sec * 1000000 + tv.tv_usec;
  TRY (nlm_device_get_param (device, NLM_TIMESTAMP, &start_sclock));

  usleep (300000); /* 0.3 of a second */

  gettimeofday (&tv, 0);
  end_time = tv.tv_sec * 1000000 + tv.tv_usec;
  TRY (nlm_device_get_param (device, NLM_TIMESTAMP, &end_sclock));

  clock_speed = (end_sclock - start_sclock) * 1. / (end_time - start_time);

  if (flag_verbose)
    DDPRINT ("System clock freq is %1.3f MHz\n", clock_speed);

#endif

#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2) || 1
  if (0)
#endif
  detect_clock_skew (device);

  engine_clock_skew = 1;
  scale_ratio = 1;/*312.5 / clock_speed;*/

  return NLM_OK;
}

void
read_device_register (struct nlm_device *dev, uint32_t reg)
{
  nlm_status s;
  uint32_t val;

  s = nlm_device_get_param (dev, NLM_HW_REGISTER, reg, &val);
  if (s != NLM_OK)
    {
      printf ("register read failed.\n");
      return;
    }
  printf ("Register 0x%08x = 0x%08x\n", (unsigned int)reg, (unsigned int)val);
  return;
}

void
dump_words (FILE *fp, uint32_t *base, size_t n_words)
{
  int i;

  for (i = 0; i < n_words; i++)
    fprintf (fp, "0x%" NLM_PRIxPTR " : 0x%08" NLM_PRIx32 "\n", (nlm_uintptr_t)base + i, base[i]);
  return;
}

void
dump_registers_fp (FILE *fp, struct nlm_device *dev)
{
  uint32_t r, v;
  nlm_status s;

  for (r = 0; r <= 0x68FF; r += 4)
    {
      s = nlm_device_get_param (dev, NLM_HW_REGISTER, r, &v);
      if (s != NLM_OK)
        {
          printf ("failure while reading a register.\n");
          return;
        }
      fprintf (fp, "0x%" NLM_PRIx32 " : 0x%" NLM_PRIx32 "\n", r, v);
    }

  return;
}

void
dump_registers_fname (struct nlm_device *dev, const char *fname)
{
  FILE *fp;

  fp = fopen (fname, "r");
  dump_registers_fp (fp, dev);
  fclose (fp);
}

nlm_status
run_abcd_on_device1 (uint32_t device_id, uint32_t packet_size, struct nlm_device_config *config,
                    int is_stateful)
{
  struct nlm_flow *flow;
  struct nlm_device *device;
  char *packet_memory;
  void *my_cookie;
  nlm_status status;
  int iter = 0;
  char *packet_start;
  uint32_t start_time, end_time;
  uint32_t mem_size = 2 * 1024 * 1024;
  uint64_t last_timestamp = 0;
  struct timeval tv; /* as clock() is not available on some systems */
  uint64_t engine_cycles = 0;
  uint32_t packet_cnt = 0;

  if (flag_verbose)
    DDPRINT ("\nStarting run...\n");

  /* initialize HW device based on configuration and get device handle */
  TRY (nlm_device_init (config, &device));

  detect_clock_speed (device);

  /* HW needs to know translation of virt->phys address for the packets,
     so we use packet_memory that driver allocated for us for demo purposes.
     Production system will know packet_base_virt and packet_base_phys beforehand.

     HW can access any physical memory present on the system,
     so packet doesn't need to be copied into specialized location.
     Dataplane converts virtual adress into physical using the following formula:
     packet_start_phys = nlm_device_config->packet_base_phys
                         + start - nlm_device_config->packet_base_virt

     In production system packet will get DMAed into some kernel buffer by NIC
     and dataplane would only need to know the virt and phys address of such
     kernel buffer to process the packet, effectively doing zero-copy access */
  packet_memory = (char *)config->packet_base_virt;

  TRY (init_packet_memory (packet_memory, mem_size));

  if (db_file)
    TRY (load_db_from_file (device, db_file, 0))
  else
    TRY (load_db (device, 0))

  /* create stateful flow to search regexes from database 0 */
  if (is_stateful)
    TRY (nlm_create_flow (device, NLM_FLOW_TYPE_STATEFUL, &flow))
  else
    TRY (nlm_create_flow (device, NLM_FLOW_TYPE_STATELESS, &flow))

  TRY (nlm_flow_add_database_and_group (device, flow, 0, group_id));

  /* has to use gettimeofday(), as clock() is not available on some systems */
  gettimeofday (&tv, 0);
  start_time = tv.tv_sec * 1000000 + tv.tv_usec;

  if (pd_linking_off)
    /* experimental disable of PD-linking  */
    TRY (nlm_device_set_param (device, NLM_PACKET_ENQUEUE_POLICY, 2))

  /* enable only one dma queue, so only one processing engine will be busy */
  TRY (nlm_device_set_param (device, NLM_HW_REGISTER, 0x2018 /*PDF_CONFIG2*/, ~1));

  packet_start = packet_memory;
  while (packet_start + packet_size < packet_memory + mem_size)
    {
      static struct nlm_job *job[512 * 1024];
      int job_cnt;

      /* some user specified cookie */
      my_cookie = (void *) ((char*) 0xff0000 + iter);
      /* User can submit only up to 'input_fifo_size - 1' jobs at a time. But below we
      submit 'input_fifo_size - 2' jobs at a time since we need one job slot for
      nlm_destroy_stateful_flow (it internally uses one job) */
      for (job_cnt = 0;
           packet_start + packet_size < packet_memory + mem_size && job_cnt < config->input_fifo_size - 2;
           job_cnt++, iter++)
        {
          /* enqueue search for the packet */
          TRY (nlm_flow_enqueue_search (device, flow, packet_start, packet_start + packet_size,
                                        my_cookie, &job[job_cnt]));
          packet_start += packet_size;
        }

      /* TRY (nlm_device_get_param (device, NLM_TIMESTAMP, &last_timestamp)); */
      last_timestamp = 0;
      TRY (nlm_start_jobs (device, job_cnt, job));

      if (packet_start + packet_size >= packet_memory + mem_size && is_stateful)
        {
          /* destroy flow once last packet is sent */
          TRY (nlm_destroy_stateful_flow (device, flow, NULL));
          job_cnt++;
        }

      while (job_cnt) /* get results for all jobs */
        {
          int cnt = 1000000;
          do
            {
              uint32_t i;
              struct nlm_result matches[1024];
              uint32_t n_matches;

              /* get 16 or less results out of the output fifo into 'matches' buffer */
              status = nlm_get_all_search_results (device, 1024, matches, &n_matches);

              if (status != NLM_OK)
                DDPRINT ("get_all_search_results error: %s\n", nlm_get_status_string (status));
              else
                {
                  if (status == NLM_OK && n_matches == 0)
                    usleep (10);/* don't sleep or throughput will suffer */
                  else if (n_matches != 0)
                    {
                      for (i = 0; i < n_matches; i++)
                        if (matches[i].status == NLM_OK)
                          {
                            if (flag_verbose)
                              DDPRINT ("rule = %d flow offset = %"PRId64"\n",
                                       matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                          }
                        else if (matches[i].status == NLM_END_ANCHORED && flag_verbose)
                          {
                            if (flag_verbose)
                              DDPRINT ("rule = %d flow offset = %"PRId64" end_anchored\n",
                                       matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                          }
                        else if (matches[i].status == NLM_END_OF_JOB)
                          {
                            status = NLM_END_OF_JOB; /* to exit out of the loop */
                            job_cnt--;
                            /* difference between timestamps will give us true performance
                               of the single engine on the single packet */
                            if (last_timestamp
                                   /* filter out end_of_flow notification */
                                && matches[i].cookie != NULL)
                              {
                                if (flag_verbose == 2)
                                  DDPRINT ("engine speed = %1.3f bits per cycle\n",
                                           packet_size * 8.
                                           / (matches[i].u.stats.timestamp - last_timestamp));
                                if (matches[i].u.stats.timestamp - last_timestamp > 0xff0000)
                                  /* mars2 timestamp is only 24-bit */;
                                else
                                  {
                                    engine_cycles += matches[i].u.stats.timestamp - last_timestamp;
                                    packet_cnt++;
                                  }
                              }
                            last_timestamp = matches[i].u.stats.timestamp;
                          }
                        else
                          {
                            DDPRINT ("get_all_search_results result[%d]: %s\n",
                                     i, nlm_get_status_string (matches[i].status));
                            status = NLM_END_OF_JOB; /* to exit out of the loop */
                            job_cnt --;
                          }
                    }
                }
              cnt--;
            }
          while (status == NLM_OK && cnt);
        }
    }

  gettimeofday (&tv, 0);
  end_time = tv.tv_sec * 1000000 + tv.tv_usec;

  /* DDPRINT ("execution time = %1.3f seconds\n", (double) (end_time - start_time) / 1000000); */

  if (flag_verbose)
    {
#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2)
      DDPRINT ("Single %s flow speed with packet_size %5d is %1.3f Mbps (system clock)"
               " and %1.3f Mbps (internal HW counter)\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size, iter * packet_size * 8. / (end_time - start_time),
               packet_size * 8. * clock_speed * packet_cnt / engine_cycles);
#else
      DDPRINT ("Single %s flow speed with packet_size %5d is:\n"
               "%1.3f Mbps (system clock), %1.3f Mbps (internal HW counter), latency per packet = %1.3f us\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size,
               iter * packet_size * 8. / (end_time - start_time) * scale_ratio / engine_clock_skew,
               packet_size * 8. * clock_speed * scale_ratio / engine_clock_skew * packet_cnt / engine_cycles,
               engine_cycles / (packet_cnt * clock_speed * scale_ratio / engine_clock_skew));
#endif
    }
  else
    {
#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2)
      DDPRINT ("Single %s flow speed with packet_size %5d is %1.3f Mbps\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size, packet_size * 8. * clock_speed * packet_cnt / engine_cycles);
#else
      DDPRINT ("Single %s flow speed with packet_size %5d is %1.3f Mbps\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size, packet_size * 8. * clock_speed * scale_ratio
               / engine_clock_skew * packet_cnt / engine_cycles);
#endif
    }

  /* shutdown device */
  TRY (nlm_device_fini (device));

  return NLM_OK;
}

nlm_status
run_abcd_on_famos (uint32_t packet_size, char *packet_memory, uint32_t input_fifo_size,
                   struct nlm_device *search_device, int is_stateful)
{
  struct nlm_flow *flow;
  void *my_cookie;
  nlm_status status;
  int iter = 0;
  char *packet_start;
  uint32_t start_time, end_time;
  uint32_t mem_size = 2 * 1024 * 1024;
  uint64_t last_timestamp = 0;
  struct timeval tv; /* as clock() is not available on some systems */
  uint64_t engine_cycles = 0;
  uint32_t packet_cnt = 0;

  if (flag_verbose)
    DDPRINT ("\nStarting run...\n");

  TRY (init_packet_memory (packet_memory, mem_size));

  if (ring_mode == RING_MODE_MASTER_ONLY)
    {
      DDPRINT ("Master ring database loaded. Now sleeping forever.\n");
      while (1)
        usleep (100000);
    }

  /* create stateful flow to search regexes from database 0 */
  if (is_stateful)
    TRY (nlm_create_flow (search_device, NLM_FLOW_TYPE_STATEFUL, &flow))
  else
    TRY (nlm_create_flow (search_device, NLM_FLOW_TYPE_STATELESS, &flow))

  TRY (nlm_flow_add_database_and_group (search_device, flow, 0, group_id));

  /* has to use gettimeofday(), as clock() is not available on some systems */
  gettimeofday (&tv, 0);
  start_time = tv.tv_sec * 1000000 + tv.tv_usec;

  if (pd_linking_off)
    /* experimental disable of PD-linking  */
    TRY (nlm_device_set_param (search_device, NLM_PACKET_ENQUEUE_POLICY, 2))

  /* enable only one dma queue, so only one processing engine will be busy */
  TRY (nlm_device_set_param (search_device, NLM_HW_REGISTER, 0x2018 /*PDF_CONFIG2*/, ~1));

  packet_start = packet_memory;
  while (packet_start + packet_size < packet_memory + mem_size)
    {
      static struct nlm_job *job[512 * 1024];
      int job_cnt;

      /* some user specified cookie */
      my_cookie = (void *) ((char*) 0xff0000 + iter);
      /* User can submit only up to 'input_fifo_size - 1' jobs at a time. But below we
      submit 'input_fifo_size - 2' jobs at a time since we need one job slot for
      nlm_destroy_stateful_flow (it internally uses one job) */
      for (job_cnt = 0;
           packet_start + packet_size < packet_memory + mem_size && job_cnt < input_fifo_size - 2;
           job_cnt++, iter++)
        {
          /* enqueue search for the packet */
          TRY (nlm_flow_enqueue_search (search_device, flow, packet_start,
                                        packet_start + packet_size,
                                        my_cookie, &job[job_cnt]));
          packet_start += packet_size;
        }

      /* TRY (nlm_device_get_param (device, NLM_TIMESTAMP, &last_timestamp)); */
      last_timestamp = 0;
      TRY (nlm_start_jobs (search_device, job_cnt, job));

      if (packet_start + packet_size >= packet_memory + mem_size && is_stateful)
        {
          /* destroy flow once last packet is sent */
          TRY (nlm_destroy_stateful_flow (search_device, flow, NULL));
          job_cnt++;
        }

      while (job_cnt) /* get results for all jobs */
        {
          int cnt = 1000000;
          do
            {
              uint32_t i;
              struct nlm_result matches[1024];
              uint32_t n_matches;

              /* get 16 or less results out of the output fifo into 'matches' buffer */
              status = nlm_get_all_search_results (search_device, 1024, matches, &n_matches);

              if (status != NLM_OK)
                DDPRINT ("get_all_search_results error: %s\n", nlm_get_status_string (status));
              else
                {
                  if (status == NLM_OK && n_matches == 0)
                    usleep (10);/* don't sleep or throughput will suffer */
                  else if (n_matches != 0)
                    {
                      for (i = 0; i < n_matches; i++)
                        if (matches[i].status == NLM_OK)
                          {
                            if (flag_verbose)
                              DDPRINT ("rule = %d flow offset = %"PRId64"\n",
                                       matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                          }
                        else if (matches[i].status == NLM_END_ANCHORED && flag_verbose)
                          {
                            if (flag_verbose)
                              DDPRINT ("rule = %d flow offset = %"PRId64" end_anchored\n",
                                       matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                          }
                        else if (matches[i].status == NLM_END_OF_JOB)
                          {
                            status = NLM_END_OF_JOB; /* to exit out of the loop */
                            job_cnt--;
                            /* difference between timestamps will give us true performance
                               of the single engine on the single packet */
                            if (last_timestamp
                                   /* filter out end_of_flow notification */
                                && matches[i].cookie != NULL)
                              {
                                if (flag_verbose == 2)
                                  DDPRINT ("engine speed = %1.3f bits per cycle\n",
                                           packet_size * 8.
                                           / (matches[i].u.stats.timestamp - last_timestamp));
                                if (matches[i].u.stats.timestamp - last_timestamp > 0xff0000)
                                  /* mars2 timestamp is only 24-bit */;
                                else
                                  {
                                    engine_cycles += matches[i].u.stats.timestamp - last_timestamp;
                                    packet_cnt++;
                                  }
                              }
                            last_timestamp = matches[i].u.stats.timestamp;
                          }
                        else
                          {
                            DDPRINT ("get_all_search_results result[%d]: %s\n",
                                     i, nlm_get_status_string (matches[i].status));
                            status = NLM_END_OF_JOB; /* to exit out of the loop */
                            job_cnt --;
                          }
                    }
                }
              cnt--;
            }
          while (status == NLM_OK && cnt);
        }
    }

  gettimeofday (&tv, 0);
  end_time = tv.tv_sec * 1000000 + tv.tv_usec;

  /* DDPRINT ("execution time = %1.3f seconds\n", (double) (end_time - start_time) / 1000000); */

  if (flag_verbose)
    {
#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2)
      DDPRINT ("Single %s flow speed with packet_size %5d is %1.3f Mbps (system clock)"
               " and %1.3f Mbps (internal HW counter)\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size, iter * packet_size * 8. / (end_time - start_time),
               packet_size * 8. * clock_speed * packet_cnt / engine_cycles);
#else
      DDPRINT ("Single %s flow speed with packet_size %5d is:\n"
               "%1.3f Mbps (system clock), %1.3f Mbps (internal HW counter), latency per packet = %1.3f us\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size,
               iter * packet_size * 8. / (end_time - start_time) * scale_ratio / engine_clock_skew,
               packet_size * 8. * clock_speed * scale_ratio / engine_clock_skew * packet_cnt / engine_cycles,
               engine_cycles / (packet_cnt * clock_speed * scale_ratio / engine_clock_skew));
#endif
    }
  else
    {
#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2)
      DDPRINT ("Single %s flow speed with packet_size %5d is %1.3f Mbps\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size, packet_size * 8. * clock_speed * packet_cnt / engine_cycles);
#else
      DDPRINT ("Single %s flow speed with packet_size %5d is %1.3f Mbps\n",
               (is_stateful ? "stateful" : "stateless"),
               packet_size, packet_size * 8. * clock_speed * scale_ratio
               / engine_clock_skew * packet_cnt / engine_cycles);
#endif
    }

  return NLM_OK;
}

nlm_status
run_abcd_on_all (uint32_t device_cnt, uint32_t packet_size)
{
#define MAX_DEVICES 8
#define PACKET_MEM_SIZE (2 * 1024 * 1024) /* 2Mbyte of packet memory */

#define MAX_JOB_CNT (256 * 1024 - 1)
#define FIFO_SIZE (256 * 1024)
#define MAX_RESULTS (1024)

  struct nlm_flow *flow[MAX_FLOWS * MAX_DEVICES];
  struct nlm_device *device[MAX_DEVICES];
  char *packet_memory[MAX_DEVICES];
  void *my_cookie;
  nlm_status status;
  int iter = 0;
  char *packet_start;
  uint32_t start_time[MAX_DEVICES], end_time[MAX_DEVICES];
  uint32_t mem_size = PACKET_MEM_SIZE;
  uint64_t last_timestamp = 0;
  struct timeval tv; /* as clock() is not available on some systems */
  int i, j;
  int job_cnt_per_device = 0;
  static struct nlm_job *job[MAX_DEVICES][MAX_JOB_CNT];

  /* use default values for platform independent fields of device_config */
  struct nlm_device_config default_config = DEFAULT_NLM_DEVICE_CONFIG;
  struct nlm_device_config config[MAX_DEVICES];

  pretty_assert (device_cnt < MAX_DEVICES);

  pretty_assert (MAX_JOB_CNT < FIFO_SIZE);

  if (db_file)
    DDPRINT ("Using database '%s' to search in %d stateful flows with packet size %d\n",
             db_file, num_of_flows, packet_size);
  else
    DDPRINT ("Searching regex '%s' in %d stateful flows with packet size %d\n",
             regex_str, num_of_flows, packet_size);

  for (i = 0; i < device_cnt; i++)
    {
      /* fill in device_config with platform specific values for
         register map, system memory and memory pool */
      memcpy (&config[i], &default_config, sizeof (struct nlm_device_config));
      config[i].input_fifo_size = FIFO_SIZE;
      config[i].output_fifo_size = FIFO_SIZE;
      config[i].memory_pool_size = 32 * 1024 * 1024; /* 32M of memory pool */
      config[i].size_of_database_extension_area = flag_spill_size;
      TRY (nlm_prepare_device_config (i /* use this HW device */, &config[i]));

      /* initialize HW device based on configuration and get device handle */
      TRY (nlm_device_init (&config[i], &device[i]));

      detect_clock_speed (device[i]);

      /* enable all queues */
      TRY (nlm_device_set_param (device[i], NLM_HW_REGISTER, 0x2018 /*PDF_CONFIG2*/, 0));

      if (db_file)
        TRY (load_db_from_file (device[i], db_file, 0))
      else
        TRY (load_db (device[i], 0))

      /* HW needs to know translation of virt->phys address for the packets,
        so we use packet_memory that driver allocated for us for demo purposes.
        Production system will know packet_base_virt and packet_base_phys beforehand */
      packet_memory[i] = (char *)config[i].packet_base_virt;

      TRY (init_packet_memory (packet_memory[i], mem_size));
    }

  /* create stateful device_cnt * num_of_flows flows to search regexes from database 0
     on all devices */
  for (i = 0; i < device_cnt; i++)
    for (j = 0; j < num_of_flows; j++)
      {
        TRY (nlm_create_flow (device[i], NLM_FLOW_TYPE_STATEFUL, &flow[i * MAX_FLOWS + j]));
        TRY (nlm_flow_add_database_and_group (device[i], flow[i * MAX_FLOWS + j], 0, group_id));
      }

  for (i = 0; i < device_cnt; i++)
    {
      int job_cnt;

      /* some user specified cookie */
      my_cookie =  (void *) ((char*) 0xff0000 + iter);

      packet_start = packet_memory[i];

      for (job_cnt = 0;
           packet_start + packet_size <= packet_memory[i] + mem_size
           && job_cnt < MAX_JOB_CNT;)
        {
          /* enqueue search for the packet */
          for (j = 0; j < num_of_flows && job_cnt < MAX_JOB_CNT; j++, iter++)
            {
              TRY (nlm_flow_enqueue_search (device[i], flow[i * MAX_FLOWS + j], packet_start,
                                            packet_start + packet_size,
                                            my_cookie, &job[i][job_cnt++]));
            }
          packet_start += packet_size;
        }

      if (job_cnt_per_device == 0)
        job_cnt_per_device = job_cnt;
      else
        pretty_assert (job_cnt_per_device == job_cnt);
    }

  for (i = 0; i < device_cnt; i++)
    {
      gettimeofday (&tv, 0);
      start_time[i] = tv.tv_sec * 1000000 + tv.tv_usec; /* start time of device 'i' */
      TRY (nlm_start_jobs (device[i], job_cnt_per_device, job[i]));
    }

  {
    int all_job_cnt = job_cnt_per_device * device_cnt;
    int job_cnt[MAX_DEVICES];
    for (i = 0 ; i < device_cnt; i++)
      job_cnt[i] = job_cnt_per_device;

    do
      {
        int d;
        for (d = 0; d < device_cnt; d++)
          {
            static struct nlm_result matches[MAX_RESULTS];
            uint32_t n_matches;

            /* get 16 or less results out of the output fifo into 'matches' buffer */
            status = nlm_get_all_search_results (device[d], MAX_RESULTS, matches, &n_matches);

            if (status != NLM_OK)
              DDPRINT ("get_all_search_results error: %s\n", nlm_get_status_string (status));
            else
              {
                if (status == NLM_OK && n_matches == 0)
                  ;/* don't sleep or throughput will suffer */
                else if (n_matches != 0)
                  {
                    uint32_t i;
                    for (i = 0; i < n_matches; i++)
                      if (matches[i].status == NLM_OK)
                        {
                          if (flag_verbose)
                            DDPRINT ("device = %d rule = %d flow offset = %"PRId64"\n", d,
                                    matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                        }
                      else if (matches[i].status == NLM_END_ANCHORED)
                        {
                          if (flag_verbose)
                            DDPRINT ("rule = %d flow offset = %"PRId64" end_anchored\n",
                                    matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                        }
                      else if (matches[i].status == NLM_END_OF_JOB)
                        {
                          job_cnt[d]--;
                          all_job_cnt--;
                          if (job_cnt[d] == 0)
                            {
                              /* device 'd' is done processing jobs */
                              gettimeofday (&tv, 0);
                              end_time[d] = tv.tv_sec * 1000000 + tv.tv_usec;
                            }
                          /* difference between timestamps will give us true performance
                             of the single engine on the single packet */
                          /*DDPRINT ("engine speed = %1.3f bits per cycle\n",
                                     packet_size * 8.
                                  / (matches[i].u.stats.timestamp - last_timestamp));*/
                          last_timestamp = matches[i].u.stats.timestamp;
                        }
                      else
                        DDPRINT ("get_all_search_results result[%d]: %s\n",
                                i, nlm_get_status_string (matches[i].status));
                  }
              }
          }
      }
    while (all_job_cnt);
  }

  {
    /* Destroy the flows created */
    for (i = 0; i < device_cnt; i++)
      {
        for (j = 0; j < num_of_flows; j++)
          {
            status = nlm_destroy_stateful_flow (device[i], flow[i * MAX_FLOWS + j], (void *) 1);
            if (status == NLM_END_OF_JOB)
              continue;
            else if (status == NLM_OK)
              {
                /* Need to wait for end of job acknowledgement */
                static struct nlm_result matches[MAX_RESULTS];
                uint32_t n_matches;
                do 
                  {
                    status = nlm_get_all_search_results (device[i], MAX_RESULTS, matches, &n_matches);
                    
                    if (status != NLM_OK)
                      DDPRINT ("get_all_search_results error: %s\n", nlm_get_status_string (status));
                    else if (n_matches == 0)
                      continue;
                    else
                      {
                        pretty_assert (n_matches == 1);
                        if (matches[0].status == NLM_END_OF_JOB)
                          break;
                      }
                  }
                while (1);
              }
          }
      }
  }
            
  {
    double total_speed = 0.0;
    for (i = 0; i < device_cnt; i++)
      {
        double single_device_speed = iter * packet_size * 8.
                                     / (end_time[i] - start_time[i]) / device_cnt;
#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2)
#else
        single_device_speed = single_device_speed * scale_ratio / engine_clock_skew;
#endif
        if (device_cnt > 1)
          DDPRINT ("device %d ", i);
        DDPRINT ("speed = %1.3f MBits per second\n", single_device_speed);
        total_speed += single_device_speed;
      }

    if (device_cnt > 1)
      DDPRINT ("Total system speed = %1.3f MBits per second\n", total_speed);
  }

  /* shutdown devices */
  for (i = 0; i < device_cnt; i++)
    {
      TRY (nlm_device_fini (device[i]));
      TRY (nlm_free_device_config (&config[i]));
    }

  return NLM_OK;
}

static nlm_status
test_max_flow (void)
{
  struct nlm_device_config config = DEFAULT_NLM_DEVICE_CONFIG;
  uint32_t max_memory_pool = 2ull * 1024 * 1024 * 1024;
  void *pool;
  struct nlm_flow *flow;
  struct nlm_device *device;
  char *packet_memory;
  uint32_t mem_size = 2 * 1024 * 1024;
  uint32_t num_flows = 0;
  nlm_status status;
  uint32_t jobs_completed = 0;

  for (;;)
    {
      pool = malloc (max_memory_pool);
      if (pool)
        break;
      else
        max_memory_pool = max_memory_pool - max_memory_pool / 4;
    }

  DDPRINT ("Allocated %d Mbytes of memory pool\n", max_memory_pool >> 20);

  config.memory_pool_size = max_memory_pool;
  config.memory_pool = pool;

  if (ddr_size)
    {
      DDPRINT ("Got %d Mbytes of DDR\n", ddr_size >> 20);
      config.size_of_ddr_memory = ddr_size;
    }

  config.size_of_database_extension_area = flag_spill_size;
  TRY (nlm_prepare_device_config (0 /* use this HW device */, &config));
  DDPRINT ("Got %d Mbytes of system memory\n", config.sysmem_size >> 20);

  TRY (nlm_device_init (&config, &device));

  /* HW needs to know translation of virt->phys address for the packets,
     so we use packet_memory that driver allocated for us for demo purposes.
     Production system will know packet_base_virt and packet_base_phys beforehand */
  packet_memory = (char *)config.packet_base_virt;

  TRY (init_packet_memory (packet_memory, mem_size));

  if (db_file)
    TRY (load_db_from_file (device, db_file, 0))
  else
    TRY (load_db (device, 0))

  /* create stateful flow to search regexes from database 0 */
  DDPRINT ("while (true) { create flow, enqueue search, start job, get results } ...\n");
  for (;;)
    {
      status = nlm_create_flow (device, NLM_FLOW_TYPE_STATEFUL, &flow);
      if (status == NLM_OK)
        {
          struct nlm_job *job;
          struct nlm_result matches[1024];
          uint32_t n_matches, n_tries = 0;

          num_flows++;

          status = nlm_flow_add_database_and_group (device, flow, 0, group_id);
          if (status != NLM_OK)
            {
              DDPRINT ("nlm_flow_add_database_and_group error: %s\n", nlm_get_status_string (status));
              break;
            }

          status = nlm_flow_enqueue_search (device, flow, packet_memory, packet_memory + 128,
                                            0, &job);
          if (status != NLM_OK)
            {
              DDPRINT ("nlm_flow_enqueue_search error: %s\n", nlm_get_status_string (status));
              break;
            }

          while ((status = nlm_start_jobs (device, 1, &job)) != NLM_OK)
            {
              n_tries++;
              if (status != NLM_OK && n_tries == 100)
                {
                  DDPRINT ("nlm_start_jobs error: %s\n", nlm_get_status_string (status));
                  num_flows--;
                  break; /* break out of while loop */
                }
            }

          if (status != NLM_OK)
            break; /* If above start jobs failed repeatedly break out of core loop */

          status = nlm_get_all_search_results (device, 1024, matches, &n_matches);
          if (status != NLM_OK)
            {
              DDPRINT ("nlm_get_all_search_results error: %s\n", nlm_get_status_string (status));
              break;
            }
          else if (n_matches)
            {
              int i;
              for (i = 0; i < n_matches; i++)
                if (matches[i].status == NLM_OK)
                  {
                    if (flag_verbose)
                      DDPRINT ("rule = %d flow offset = %"PRId64"\n",
                               matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                  }
                else if (matches[i].status == NLM_END_ANCHORED && flag_verbose)
                  {
                    if (flag_verbose)
                      DDPRINT ("rule = %d flow offset = %"PRId64" end_anchored\n",
                               matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
                  }
                else if (matches[i].status == NLM_END_OF_JOB)
                  {
                    jobs_completed++;
                  }
                else
                  {
                    DDPRINT ("get_all_search_results result[%d]: %s\n",
                             i, nlm_get_status_string (matches[i].status));
                  }
            }
        }
      else
        break;
    }

  DDPRINT ("Created %d flows\n", num_flows);
  DDPRINT ("%d search jobs completed while we were creating flows\n", jobs_completed);
  DDPRINT ("Last flow creation failed: %s\n", nlm_get_status_string (status));
  DDPRINT ("Flow creation failure expected, as system memory has been exhausted\n");
  DDPRINT ("Waiting for the rest of search jobs to finish...\n");

  while (jobs_completed != num_flows)
    {
      struct nlm_result matches[1024];
      uint32_t n_matches;

      TRY (nlm_get_all_search_results (device, 1024, matches, &n_matches));
      if (n_matches)
        {
          int i;
          for (i = 0; i < n_matches; i++)
            if (matches[i].status == NLM_OK)
              {
                if (flag_verbose)
                  DDPRINT ("rule = %d flow offset = %"PRId64"\n",
                           matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
              }
            else if (matches[i].status == NLM_END_ANCHORED && flag_verbose)
              {
                if (flag_verbose)
                  DDPRINT ("rule = %d flow offset = %"PRId64" end_anchored\n",
                           matches[i].u.match.rule_id, matches[i].u.match.flow_offset);
              }
            else if (matches[i].status == NLM_END_OF_JOB)
              {
                jobs_completed++;
              }
            else
              {
                DDPRINT ("get_all_search_results result[%d]: %s\n",
                         i, nlm_get_status_string (matches[i].status));
              }
        }
    }

  DDPRINT ("All %d jobs completed\n", jobs_completed);

  return NLM_OK;
}

nlm_status
setup_configs_and_devices (struct nlm_device_config *config_m, struct nlm_device_config *config_a,
                           struct nlm_device **db_device, struct nlm_device **search_device)
{
  config_m->memory_pool_size = 32 * 1024 * 1024;
  config_a->memory_pool_size = 32 * 1024 * 1024;
  config_m->size_of_ddr_memory = ddr_size;
  config_a->size_of_ddr_memory = ddr_size;
  config_m->size_of_database_extension_area = flag_spill_size;
  config_a->size_of_database_extension_area = flag_spill_size;

  switch (ring_mode)
    {
    case RING_MODE_RUN_ON_MASTER:
    case RING_MODE_MASTER_ONLY:
      TRY (nlm_prepare_device_config (NLM_MASTER_RING, config_m));
      config_m->ring_id = NLM_MASTER_RING;
      TRY (nlm_device_init (config_m, db_device));
      *search_device = *db_device;
      break;
    case RING_MODE_MASTER_AND_ATTACH:
      TRY (nlm_prepare_device_config (NLM_MASTER_RING , config_m));
      config_m->ring_id = NLM_MASTER_RING;
      TRY (nlm_device_init (config_m, db_device));
      *search_device = *db_device;
      break;
    case RING_MODE_ATTACH_ONLY:
      config_a->ring_id = attached_ring_id;
      TRY (nlm_prepare_device_config (NLM_MASTER_RING, config_a));
      TRY (nlm_device_attach (config_a, search_device));
      break;
    }

  return NLM_OK;
}

nlm_status
do_one_device_run ()
{
  struct nlm_device_config config = DEFAULT_NLM_DEVICE_CONFIG;
  uint32_t psize, dev_num;
  int do_stateful, do_stateless;

  config.memory_pool_size = 32 * 1024 * 1024; /* 32M of memory pool */
  config.size_of_ddr_memory = ddr_size;
  config.size_of_database_extension_area = flag_spill_size;
  TRY (nlm_prepare_device_config (0 /* use this HW device */, &config));

  do_stateful = 1;
  do_stateless = 0;
  dev_num = 0;
  for (psize = min_packet_size; psize <= max_packet_size; psize *= 2)
    TRY (run_abcd_on_device1 (dev_num, psize, &config, do_stateful));
  for (psize = min_packet_size; psize <= max_packet_size; psize *= 2)
    TRY (run_abcd_on_device1 (dev_num, psize, &config, do_stateless));

  return NLM_OK;
}

nlm_status
do_one_device_run_famos ()
{
  struct nlm_device_config config_m = DEFAULT_NLM_DEVICE_CONFIG;
  struct nlm_device_config config_a = DEFAULT_NLM_DEVICE_CONFIG;
  struct nlm_device *db_device, *search_device;
  char *packet_memory = NULL;
  uint32_t input_fifo_size = 0;
  uint32_t psize, do_stateful, do_stateless;

  db_device = NULL;
  search_device = NULL;
  TRY (setup_configs_and_devices (&config_m, &config_a, &db_device, &search_device));

  /* HW needs to know translation of virt->phys address for the packets,
     so we use packet_memory that driver allocated for us for demo purposes.
     Production system will know packet_base_virt and packet_base_phys beforehand */
  switch (ring_mode)
    {
    case RING_MODE_RUN_ON_MASTER:
      packet_memory = (char *)config_m.packet_base_virt;
      input_fifo_size = config_m.input_fifo_size;
      detect_clock_speed (db_device);
      break;
    case RING_MODE_MASTER_ONLY:
      packet_memory = (char *)config_m.packet_base_virt;
      input_fifo_size = config_m.input_fifo_size;
      detect_clock_speed (db_device);
      break;
    case RING_MODE_MASTER_AND_ATTACH:
      packet_memory = (char *)config_m.packet_base_virt;
      input_fifo_size = config_m.input_fifo_size;
      detect_clock_speed (db_device);
      break;
    case RING_MODE_ATTACH_ONLY:
      packet_memory = (char *)config_a.packet_base_virt;
      input_fifo_size = config_a.input_fifo_size;
      detect_clock_speed (search_device);
      break;
    }

  do_stateful = 1;
  do_stateless = 0;

  if (db_device)
    {
      if (db_file)
        {
          TRY (load_db_from_file (db_device, db_file, 0));
        }
      else
        {
          TRY (load_db (db_device, 0));
        }
    }

  do
    {
      for (psize = min_packet_size; psize <= max_packet_size; psize *= 2)
        TRY (run_abcd_on_famos (psize, packet_memory, input_fifo_size, search_device,
                                do_stateful));
      for (psize = min_packet_size; psize <= max_packet_size; psize *= 2)
        TRY (run_abcd_on_famos (psize, packet_memory, input_fifo_size, search_device,
                                do_stateless));
    } while (ring_mode == RING_MODE_MASTER_AND_ATTACH || ring_mode == RING_MODE_ATTACH_ONLY);

  /* free the virtual memory mapping created by the nlm_prepare_device_config() */
  switch (ring_mode)
    {
    case RING_MODE_RUN_ON_MASTER:
    case RING_MODE_MASTER_ONLY:
    case RING_MODE_MASTER_AND_ATTACH:
      TRY (nlm_device_fini (search_device));
      TRY (nlm_free_device_config (&config_m));
      break;
    case RING_MODE_ATTACH_ONLY:
      TRY (nlm_device_detach (search_device));
      TRY (nlm_free_device_config (&config_a));
      break;
    }

  return NLM_OK;
}

nlm_status
setup_configs_and_devices2 (struct nlm_device_config *config_m, struct nlm_device_config *config_a,
		                           struct nlm_device **db_device, struct nlm_device **search_device)
{
	config_m->memory_pool_size = 32 * 1024 * 1024;
	config_a->memory_pool_size = 32 * 1024 * 1024;
	config_m->size_of_ddr_memory = ddr_size;
	config_a->size_of_ddr_memory = ddr_size;
	config_m->size_of_database_extension_area = flag_spill_size;
	config_a->size_of_database_extension_area = flag_spill_size;

	TRY (nlm_prepare_device_config (NLM_MASTER_RING , config_m));
	config_m->ring_id = NLM_MASTER_RING;
	TRY (nlm_device_init (config_m, db_device));
	*search_device = *db_device;

	return NLM_OK;
}

nlm_status
do_one_device(void)
{
	struct nlm_device_config config_m = DEFAULT_NLM_DEVICE_CONFIG;
	struct nlm_device_config config_a = DEFAULT_NLM_DEVICE_CONFIG;
	struct nlm_device *db_device, *search_device;
	char *packet_memory = NULL;
	uint32_t input_fifo_size = 0;
	uint32_t psize, do_stateful, do_stateless;

	db_device = NULL;
	search_device = NULL;
	TRY (setup_configs_and_devices (&config_m, &config_a, &db_device, &search_device));

	packet_memory = (char *)config_m.packet_base_virt;
	input_fifo_size = config_m.input_fifo_size;
	detect_clock_speed (db_device);

	do_stateful = 1;
	do_stateless = 0;

	if(db_device){
		if(db_file){
			TRY (load_db_from_file (db_device, db_file, 0));
		}
		else{
			TRY (load_db (db_device, 0));
		}
	}

	TRY (nlm_device_fini (search_device));
	TRY (nlm_free_device_config (&config_m));

	return NLM_OK;
}

#define ETHERTYPE_FMS   0x4321
#define IP_PROTO_UDP	17
#define IP_PROTO_ICMP	1
#define IP_PROTO_TCP	6
#define DATA_LEN	32

#define PROMISCUOUS 1
#define NONPROMISCUOUS 0

#define PACKETS	32

struct fms_rst_hdr{

};

void
callback(u_char *useless, const struct pcap_pkthdr *pkthdr,
		                        const u_char *packet)
{
	struct ip *iph;
//	struct fms_rst_hdr *fms_rst;
	struct fms_xaui_result_header *fms_rst;
	static int count = 1;
	struct ether_header *ep;
	unsigned short ether_type;
	int chcnt =0;
	int length = pkthdr->len;

	ep = (struct ether_header *)packet;
	packet += sizeof(struct ether_header);
	ether_type = ntohs(ep->ether_type);

	if(ether_type == ETHERTYPE_IP){
		char src_ip[32];
		char dst_ip[32];

		memset(src_ip, 0, 32);
		memset(dst_ip, 0, 32);

		iph = (struct ip *)packet;

		strcpy(src_ip, inet_ntoa(iph->ip_src));
		strcpy(dst_ip, inet_ntoa(iph->ip_dst));

		printf("src:%s, dst:%s\n", src_ip, dst_ip) ;
	}else if(ether_type == 0x4331){
		fms_rst = (struct fms_xaui_result_header *)packet;

		printf("rst:0x%x, err:0x%x, rd_cnt:%d, cookie:%d\n", fms_rst->flow_type, fms_rst->error, fms_rst->rd_cnt, fms_rst->cookie);

		if(fms_rst->rd_cnt == 0)
			return;

	}else if(ether_type == ETHERTYPE_FMS){

	}else{
		printf("Unknown Ethernet Type\n");
	}
}



void *
packet_rx(void *arg)
{
	pcap_t *pcd = NULL;
	struct bpf_program fp;
	bpf_u_int32 netp;
	bpf_u_int32 maskp;
	char *devname = "eth6_0";
	char errbuf[PCAP_ERRBUF_SIZE];
	int ret ;
	//0xf81edf86ee2f
	char *opt = "ether host f8:1e:df:86:ee:2f";
//	opt = NULL;
	

	ret = pcap_lookupnet(devname, &netp, &maskp, errbuf);
	if (ret == -1){
		printf("%s\n", errbuf);
		exit(1);
	}

	pcd = pcap_open_live(devname, BUFSIZ,  NONPROMISCUOUS, -1, errbuf);
	if (pcd == NULL){
		printf("%s\n", errbuf);
		exit(1);
	}

	if (pcap_compile(pcd, &fp, opt, 0, netp) == -1){
		printf("compile error\n");
		exit(1);
	}

	if (pcap_setfilter(pcd, &fp) == -1){
		printf("setfilter error\n");
		exit(0);
	}

	pcap_loop(pcd, PACKETS, callback, NULL);

	return (void *)ret;
}

void
ip_chksum(struct iphdr* _iph)
{
	int i;
	unsigned short* iph =  (unsigned short*)_iph;
	unsigned short len = 20;
	unsigned int chksum = 0;
	unsigned short final_chk;

	len >>= 1;
	_iph->check = 0;

	for(i=0;i<len;i++)
		chksum += *iph++;

	chksum = (chksum >> 16) + (chksum & 0xffff);
	chksum += (chksum >> 16);

	final_chk = (~chksum & 0xffff);

	_iph->check = final_chk;
}

static nlm_status
packet_tx (struct nlm_device *device, struct nlm_flow *flow, fms_flow_type flow_type)
{
	pcap_t *pcap_fp = NULL;
	char *devname = "eth6_0";
	char errbuf[PCAP_ERRBUF_SIZE];
	unsigned char packet[1500];
	int packet_length = 0;
	struct ether_header *eth;
	struct fms_xfd *fms_hdr;
	struct iphdr *iph;
	struct udphdr *udph;
	unsigned short ether_type;
	int i;
	int packets = PACKETS;

	if ((pcap_fp = pcap_open_live(devname,
									65536,
									1,
									1000,
									errbuf 
									)) == NULL){
		printf("The specified device: %s cannot be opened by pcap.\n", devname);
		return 1;
	}

	memset(packet, 0, sizeof(packet));

	//Ethernet Header
	eth = (struct ether_header *)packet;
	eth->ether_dhost[0] = 0x00;
	eth->ether_dhost[1] = 0x01;
	eth->ether_dhost[2] = 0x02;
	eth->ether_dhost[3] = 0x03;
	eth->ether_dhost[4] = 0x04;
	eth->ether_dhost[5] = 0x05;

	eth->ether_shost[0] = 0x10;
	eth->ether_shost[1] = 0x20;
	eth->ether_shost[2] = 0x30;
	eth->ether_shost[3] = 0x40;
	eth->ether_shost[4] = 0x50;
	eth->ether_shost[5] = 0x60;

	eth->ether_type = htons(ETHERTYPE_FMS);
//	eth->ether_type = htons(ETHERTYPE_IP);
	packet_length += sizeof(struct ether_header);

#if 1
	// DPI XAUI Cammand Header
	fms_hdr = (struct fms_xfd *)(&packet[packet_length]);

	memset(fms_hdr, 0, sizeof(struct fms_xfd));

	switch(flow_type){
		case FMS_FLOW_INIT:
			fms_hdr->flow_type = FMS_FLOW_INIT;
			fms_hdr->gid_cnt = 0;
			packets = 1;
			break;
		case FMS_FLOW_STATELESS:
			fms_hdr->flow_type = FMS_FLOW_STATELESS;
			fms_hdr->end_anchor = 1;
			fms_hdr->start_anchor = 1;

			fms_hdr->pdf_index_hi = 0;
			fms_hdr->pdf_index_mi = 0;
			fms_hdr->pdf_index_lo = 0;

			fms_hdr->gid_cnt = flow->db_group_cnt;

//			fms_hdr->gid_cnt = 2;
			break;
		case FMS_FLOW_STATEFUL:
			fms_hdr->flow_type = FMS_FLOW_STATEFUL;
			fms_hdr->end_anchor = 0;
			fms_hdr->start_anchor = 0;

			fms_hdr->pdf_index_hi = (flow->u.stateful.pdf_index >> 16) & 0xff;
			fms_hdr->pdf_index_mi = (flow->u.stateful.pdf_index >> 8) & 0xff;
			fms_hdr->pdf_index_lo = (flow->u.stateful.pdf_index & 0xff);

			fms_hdr->gid_cnt = flow->db_group_cnt;
			break;
	}

//	fms_hdr->h_offset = sizeof(struct fms_xfd) ;
	fms_hdr->h_offset = sizeof(struct iphdr) + sizeof(struct udphdr) + 2;
	fms_hdr->h_offset = 0;

	fms_hdr->pad_cnt = 0;
	fms_hdr->cookie = 0;

	switch(fms_hdr->gid_cnt){
		case 0:
			packet_length += 7;
			break;
		case 1:
			fms_hdr->gid0_hi = (flow->db_group_id[0] >> 8) & 0xf;
			fms_hdr->gid0_lo = flow->db_group_id[0] & 0xff;

			packet_length += 9;
			break;
		case 2:
			fms_hdr->gid0_hi = (flow->db_group_id[0] >> 8) & 0xf;
			fms_hdr->gid0_lo = flow->db_group_id[0] & 0xff;

			fms_hdr->gid1_hi = (flow->db_group_id[1] >> 8) & 0xf;
			fms_hdr->gid1_lo = flow->db_group_id[1] & 0xff;

			packet_length += 10;
			break;
		case 3:
			fms_hdr->gid0_hi = (flow->db_group_id[0] >> 8) & 0xf;
			fms_hdr->gid0_lo = flow->db_group_id[0] & 0xff;

			fms_hdr->gid1_hi = (flow->db_group_id[1] >> 8) & 0xf;
			fms_hdr->gid1_lo = flow->db_group_id[1] & 0xff;

			fms_hdr->gid2_hi = (flow->db_group_id[2] >> 8) & 0xf;
			fms_hdr->gid2_lo = flow->db_group_id[2] & 0xff;

			packet_length += 12;
			break;
		case 4:
			fms_hdr->gid0_hi = (flow->db_group_id[0] >> 8) & 0xf;
			fms_hdr->gid0_lo = flow->db_group_id[0] & 0xff;

			fms_hdr->gid1_hi = (flow->db_group_id[1] >> 8) & 0xf;
			fms_hdr->gid1_lo = flow->db_group_id[1] & 0xff;

			fms_hdr->gid2_hi = (flow->db_group_id[2] >> 8) & 0xf;
			fms_hdr->gid2_lo = flow->db_group_id[2] & 0xff;

			fms_hdr->gid3_hi = (flow->db_group_id[3] >> 8) & 0xf;
			fms_hdr->gid3_lo = flow->db_group_id[3] & 0xff;

			packet_length += 13;
			break;
		case 5:
			fms_hdr->gid0_hi = (flow->db_group_id[0] >> 8) & 0xf;
			fms_hdr->gid0_lo = flow->db_group_id[0] & 0xff;

			fms_hdr->gid1_hi = (flow->db_group_id[1] >> 8) & 0xf;
			fms_hdr->gid1_lo = flow->db_group_id[1] & 0xff;

			fms_hdr->gid2_hi = (flow->db_group_id[2] >> 8) & 0xf;
			fms_hdr->gid2_lo = flow->db_group_id[2] & 0xff;

			fms_hdr->gid3_hi = (flow->db_group_id[3] >> 8) & 0xf;
			fms_hdr->gid3_lo = flow->db_group_id[3] & 0xff;

			fms_hdr->gid4_hi = (flow->db_group_id[4] >> 8) & 0xf;
			fms_hdr->gid4_lo = flow->db_group_id[4] & 0xff;

			packet_length += 15;
			break;
		case 6:
			fms_hdr->gid0_hi = (flow->db_group_id[0] >> 8) & 0xf;
			fms_hdr->gid0_lo = flow->db_group_id[0] & 0xff;

			fms_hdr->gid1_hi = (flow->db_group_id[1] >> 8) & 0xf;
			fms_hdr->gid1_lo = flow->db_group_id[1] & 0xff;

			fms_hdr->gid2_hi = (flow->db_group_id[2] >> 8) & 0xf;
			fms_hdr->gid2_lo = flow->db_group_id[2] & 0xff;

			fms_hdr->gid3_hi = (flow->db_group_id[3] >> 8) & 0xf;
			fms_hdr->gid3_lo = flow->db_group_id[3] & 0xff;

			fms_hdr->gid4_hi = (flow->db_group_id[4] >> 8) & 0xf;
			fms_hdr->gid4_lo = flow->db_group_id[4] & 0xff;

			fms_hdr->gid5_hi = (flow->db_group_id[5] >> 8) & 0xf;
			fms_hdr->gid5_lo = flow->db_group_id[5] & 0xff;

			packet_length += 16;
			break;
		default:
			break;
	}

	printf("gid_cnt:%d, ", fms_hdr->gid_cnt);
	for(i=0;i<6;i++)
		printf("gid%d:%d, ", i, flow->db_group_id[i]);
	printf("pdf_index:%d\n", flow->u.stateful.pdf_index);

	ether_type = htons(ETHERTYPE_IP);
	memcpy(packet+packet_length, &ether_type, sizeof(ether_type));
	packet_length += sizeof(ether_type);
#endif

	// IP Header
	iph = (struct iphdr *)(&packet[packet_length]);
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->id = 1;
	iph->frag_off = 0;
	iph->ttl = 128;
	iph->saddr = htonl(0xc0a82115);
	iph->daddr = htonl(0xc0a83869);
	iph->protocol = IP_PROTO_UDP;
	iph->tot_len = htons( sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(eval_str));
	ip_chksum(iph);

	packet_length += sizeof(struct iphdr);


	// UDP Header
	udph = (struct udphdr *)(&packet[packet_length]);
	udph->source = htons(300);
	udph->dest = htons(80);
	udph->len = htons(sizeof(struct udphdr)+DATA_LEN);
	udph->check = 0;

	packet_length += sizeof(struct udphdr);

	memcpy(packet+packet_length, eval_str, sizeof(eval_str));
	packet_length += sizeof(eval_str);

#if 0
	for(i=0;i<10;i++){
		memcpy(packet+packet_length, eval_str, strlen(eval_str));
		packet_length += strlen(eval_str);
	}
#endif

	for(i=0;i<packets;i++){
		fms_hdr->cookie = i;
		if(pcap_sendpacket(pcap_fp, packet, packet_length) != 0){
			printf("Error sending the packet:%s\n", pcap_geterr(pcap_fp));
		}
		usleep(1000000);
	}

	pcap_close(pcap_fp);

	return NLM_OK;
}

static nlm_status
packet_process (struct nlm_device *device, struct nlm_device_config *config, int flag_stateful)
{
	char *packet_start, *packet_end;
	struct nlm_flow **flow;
	uint32_t pkt_mem_size;
	int32_t i, j, val;
	pthread_t p_thread;
	int thr_id;
	int status;

	pkt_mem_size = (2 * 1024 * 1024);
	packet_start = (char *) config->packet_base_virt;
	packet_end = packet_start + pkt_mem_size;

	if (flag_stateful){
		flow = malloc (flag_numflows * sizeof (struct nlm_flow *));
		for (i = 0; i < flag_numflows; i++){
			TRY (nlm_create_flow (device, NLM_FLOW_TYPE_STATEFUL, &flow[i]));
			TRY (nlm_flow_add_database_and_group (device, flow[i], 0, flag_group_id));
		}
	}else{
		flag_numflows = 1;
		flow = malloc (sizeof (struct nlm_flow *));
		TRY (nlm_create_flow (device, NLM_FLOW_TYPE_STATELESS, &flow[0]));
		TRY (nlm_flow_add_database_and_group (device, flow[0], 0, flag_group_id));
	}

//	packet_tx(device, flow[0], FMS_FLOW_INIT);

	thr_id = pthread_create(&p_thread, NULL, packet_rx, (void *)NULL);
	if (thr_id < 0){
		perror("thread create error : ");
		return -1;
	}

	packet_tx(device, flow[0], FMS_FLOW_STATELESS);

	if (flag_stateful){
		for (i = 0; i < flag_numflows; i++){
			TRY (nlm_flow_remove_database_and_group (device, flow[i], 0, flag_group_id));
		}
	}else{
		TRY (nlm_flow_remove_database_and_group (device, flow[0], 0, flag_group_id));
	}

	pthread_join(p_thread, (void *)&status);

	free (flow);
	return NLM_OK;
}

int
main (int ac, char **av)
{
	struct nlm_device *master_device;
	uint32_t device_cnt = 0, psize, avindex, dnum;
	nlm_status status;
	struct nlm_device_config config_m = DEFAULT_NLM_DEVICE_CONFIG;
	struct nlm_device_config config_a = DEFAULT_NLM_DEVICE_CONFIG;
	int i;

	avindex = process_args (ac, av);
	if (flag_verbose)
	{
		if (db_file)
			DDPRINT ("Using database '%s' to search\n", db_file);
	 	else
			DDPRINT ("Searching regex '%s'\n", regex_str);
	}

//	TRY (compile_db ());

	status = nlm_get_device_count (&device_cnt);
	if (status != NLM_OK)
		return status;

	TRY (nlm_prepare_device_config (NLM_MASTER_RING , &config_m));
	TRY (nlm_device_init (&config_m, &master_device));

	if (db_file)
		TRY (load_db_from_file (master_device, db_file, 0))
	else
		TRY (load_db (master_device, 0))

	config_a.size_of_database_extension_area = flag_spill_size;
	config_a.memory_pool_size = 32 * 1024 * 1024;
	config_a.max_threads = 1;
	config_a.using_manager_thread = 0;

	config_a.input_fifo_size = 256 * 1024;
	config_a.output_fifo_size = 4 * config_a.input_fifo_size;
	config_a.ring_id = NLM_XAUI_RING_0;
	config_a.thread_id = 0;

	TRY (nlm_prepare_device_config (NLM_MASTER_RING , &config_a));

	TRY (init_packet_memory ((char *) config_a.packet_base_virt, 2 * 1024 * 1024));
	TRY (nlm_device_attach (&config_a, &master_device));

	while(1);
//	packet_process (master_device, &config_a, 0);

	TRY (nlm_device_fini (master_device));
	TRY (nlm_free_device_config (&config_m));

	TRY (free_db ());

	return 0;
}

int
main2(int ac, char **av)
{
  uint32_t device_cnt = 0, psize, avindex, dnum;

  avindex = process_args (ac, av);
  if (flag_verbose)
    {
      if (db_file)
        DDPRINT ("Using database '%s' to search\n", db_file);
      else
        DDPRINT ("Searching regex '%s'\n", regex_str);
    }

  TRY (compile_db ());

  TRY (nlm_get_device_count (&device_cnt));
  if (device_cnt == 0)
    {
      DDPRINT ("No NetL7 devices detected\n");
      return 1;
    }

  if (flag_flow_test)
    {
      TRY (test_max_flow ());
      num_of_flows = 0;
    }

  if (num_of_flows == 1)
    {
      if (device_cnt == 1)
        {
#if defined(NLM_HW_FMS)
          TRY (do_one_device_run_famos ());
#else
          TRY (do_one_device_run ());
#endif
        }
      else
        {
          for (dnum = 0; dnum < device_cnt; dnum++)
            {
              /* use default values for platform independent fields of device_config */
              struct nlm_device_config config = DEFAULT_NLM_DEVICE_CONFIG;
              config.memory_pool_size = 32 * 1024 * 1024; /* 32M of memory pool */
              /* config.size_of_context_save_restore_area = 8192; */

              config.size_of_ddr_memory = ddr_size;
              config.size_of_database_extension_area = flag_spill_size;
              /* fill in device_config with platform specific values for
                 register map, system memory and memory pool */
              TRY (nlm_prepare_device_config (dnum /* use this HW device */, &config));
              DDPRINT ("Starting runs on device #%d\n", dnum);
              for (psize = min_packet_size; psize <= max_packet_size; psize *= 2)
                TRY (run_abcd_on_device1 (dnum, psize, &config, 1 /* stateful */)); /* try various packet sizes */

              for (psize = min_packet_size; psize <= max_packet_size; psize *= 2)
                TRY (run_abcd_on_device1 (dnum, psize, &config, 0 /* stateless */)); /* try various packet sizes */
              TRY (nlm_free_device_config (&config));
            }
        }
    }

  if (num_of_flows > 1)
    {
      for (psize = min_packet_size; psize <= max_packet_size; psize *= 2)
        TRY (run_abcd_on_all (1, psize))
    }

  if (device_cnt > 1 && num_of_flows > 1)
    {
      DDPRINT ("Now searching all devices in parallel...\n");
      TRY (run_abcd_on_all (device_cnt, 32768));
    }

  TRY (free_db ());
  return 0;
}
