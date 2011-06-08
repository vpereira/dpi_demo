/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sched.h>

#include "nlm_system.h"         /* For TRY and DPRINT defs only */
#include "nlm_packet_api.h"
#include "nlm_driver_api.h"
#include "nlm_database_api.h"
#include "nlm_sync.h"
#include "cli.h"

#define MAX_FRAME_SIZE (0xFFFF)
#define STATS_CNT_CONFIG 0x5800
#define STATS_MUX_SEL 0x5804
#define COUNTER0_LOW 0x5840
#define FIFO_SIZE (1 * 1024)

struct nlm_device *master_device;

static uint32_t flag_binding = 1;
static uint32_t flag_group_id = 1;
static uint32_t flag_nthreads = 2;
static uint32_t flag_use_manager = 0;
static uint32_t flag_numflows = 100;
static uint32_t flag_packet_size = 1024;
static uint32_t flag_verbose = 0;
static uint32_t flag_spill_size = 0x180000;
static uint32_t flag_steady_state_iterations = 1000;
static uint32_t flag_nongreedy = 0;
static uint32_t flag_mml = 0;
static int32_t flag_attach_mask = 0;
static int32_t nrings_active;
static uint32_t flag_stateful = 1;
static uint32_t flag_num_copies = 1;
static uint32_t flag_random_traffic = 0;
static uint64_t ddr_size = 0;

static char rule_str[]="abcd";
static char *regex_str = rule_str;

#ifndef NO_COMPILER
/* global vars for demo simplicity */
static const void *db_image;
static uint32_t db_size;
static struct nlm_database *db;
#endif

static atomic_t num_results;
static char *input_data_file;
static char *flag_db_file;

static float *thread_throughput[NLM_LAST_UNUSED_RING_ID];
static float total_throughput;
volatile uint32_t stop_manager_thread[NLM_LAST_UNUSED_RING_ID];
static atomic_t ready_to_run[NLM_LAST_UNUSED_RING_ID];
volatile uint32_t start_work[NLM_LAST_UNUSED_RING_ID];

/* Program wide parameters */
static const struct option options[] =
{
  { "help",       no_argument,        NULL, 'h' },
  { "groupid",    required_argument,  NULL, 'g' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "flows",      required_argument,  NULL, 'f' },
  { "pkt-size",   required_argument,  NULL, 's' },
  { "ext-size",   required_argument,  NULL, 'e' },
  { "num-thr",    required_argument,  NULL, 'n' },
  { "use-mgr",    required_argument,  NULL, 'm' },
  { "input",      required_argument,  NULL, 'i' },
  { "regex",      required_argument,  NULL, 'r' },
  { "database",   required_argument,  NULL, 'd' },
  { "steady-state", required_argument,  NULL, 't' },
  { "random",     no_argument,        NULL, 'R' },
  { "stateless",  no_argument,        NULL, 'S' },
  { "num-copies", required_argument,  NULL, 'c' },
  { "attach-ring", required_argument, NULL, 'a' },
  { "ddr",        required_argument,  NULL, 'D' },
  { 0, 0, 0, 0 }
};

static void
print_usage (char *progname)
{
  printf ("Usage: %s [options] \n"
    "  -h, --help         Print this help, then exit\n"
    "  -g, --groupid      The group ID to be searched (default is 1)\n"
    "  -v, --verbose      Print match information\n"
    "  -f, --flows        Number of flows to use (default is 100)\n"
    "  -s, --pkt-size     The size of the \"packet\" to search (default is 1024)\n"
    "  -e, --ext-size     The size of the database in MB (default is 1M)\n"
    "  -n, --num-thr      The number of threads to use (default is 2)\n"
    "  -m, --use-mgr      Use a separate manager thread (should be 0, 1 or 2, default is 0)\n"
    "  -i, --input        Input file to search\n"
    "  -r, --regex        Specifies regex pattern to search\n"
    "  -d, --database     File that contains the image of the rule database to load\n"
    "  -a, --attach-ring  Attach to the specified PCIE ring on FAMOS (default ring 0). The\n"
    "                     The value specified is a bit mask for enabling/disabling each ring\n"
    "                     For example to enable ring 0 and ring 1 the value specified is 3\n"
    "                     Ring 0 must always be enabled.\n"
    "  -t, --steady-state The number of times the core loop of sending packets to device\n"
    "                     is repeated for each thread (default is 1000)\n"
    "  -R, --random       Fill packet memory with random numbers (default filled with zeros).\n"
    "  -S, --stateless    Use stateless flows (default stateful)\n"
    "  -c, --num-copies   Num of copies of the database to load (default is 1)\n"
    "  -D, --ddr          Use attached ddr. Size of ddr must be specified. This size is divided equally amongs rings\n",
    progname);
  exit (10);
}

static void
perror_exit (const char *format, ...)
{
  if (format != 0)
    {
      va_list ap;
      va_start (ap, format);
      vfprintf (stderr, format, ap);
      fputs (" : ", stderr);
    }
  perror (0);
  exit (1);
}

/* process args, return index to the first non-arg */
static int
process_args (int ac, char **av)
{
  int opt;

  while ((opt = getopt_long (ac, av, "hvRSc:a:m:g:r:s:t:f:d:e:i:n:D:", options, NULL)) != -1)
    {
      switch (opt)
        {
        case 'v':
          flag_verbose = 1;
          break;
          
        case 'i':
          input_data_file = optarg;
          break;

        case 'm':
          flag_use_manager = atoi (optarg);
          if (flag_use_manager < 0 || flag_use_manager > 2)
            {
              printf ("\033[31mOnly a maximum of 2 manager threads can be used\033[0m\n");
              exit (1);
            }
          break;
          
        case 'n':
          flag_nthreads = atoi (optarg);
          if (flag_nthreads < 1)
            {
              printf ("\033[31mNum threads specified should be greater than 0.\033[0m\n");
              exit (1);
            }
          break;
          
        case 'g':
          if (!isdigit (optarg[0]))
            {
              printf ("\033[31mGroup id should be a number.\033[0m\n");
              exit (1);
            }
          flag_group_id = atoi (optarg);
          if (0 >= flag_group_id)
            printf ("\033[33mWarning: the group id is 0. "
                    "Did you specify it in decimal format?\033[0m\n");
          break;

        case 's':
          flag_packet_size = atoi (optarg);
          if (MAX_FRAME_SIZE < flag_packet_size)
            {
              printf ("\033[31mFrame size exceeds the maximum of %d\033[0m\n", MAX_FRAME_SIZE);
              exit (1);
            }
          break;

        case 't':
          flag_steady_state_iterations = atoi (optarg);
          if (flag_steady_state_iterations < 1)
            {
              printf ("\033[31mSteady state iterations must be at least 1\033[0m\n");
              exit (1);
            }
          break;
          
        case 'e':
          flag_spill_size = atoi (optarg) * 1024 * 1024;
          break;

        case 'f':
          flag_numflows = atoi (optarg);
          if (flag_numflows < 1)
            {
              printf ("\033[31mNumber of flows should be greater than 0\033[0m\n");
              exit (2);
            }
          break;
          
        case 'r':
          regex_str = optarg;
          break;
          
        case 'd':
          flag_db_file = optarg;
          break;

        case 'R':
          flag_random_traffic = 1;
          break;
          
        case 'S':
          flag_stateful = 0;
          break;
          
        case 'c':
          flag_num_copies = atoi (optarg);
          if (flag_num_copies > 4)
            {
              printf ("\033[31mNumber of database copies can be 0, 1, 2, 3, 4.\033[0m\n");
              exit (1);
            }
          break;

        case 'a':
          flag_attach_mask = atoi (optarg);
          if (flag_attach_mask == 0
              || flag_attach_mask > 0xF)
            {
              printf ("\033[31mRing ID must be a value between 1 and 3\033[0m\n");
              exit (1);
            }
          if ((flag_attach_mask & 0x1) != 1)
            {
              printf ("\033[31mRing 0 must be enabled\033[0m\n");
              exit (1);
            }

          nrings_active = __builtin_popcount (flag_attach_mask);
          if (nrings_active == 0 || nrings_active > 4)
            {
              printf ("\033[31mRing ID must be a value between 1 and 3\033[0m\n");
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

        case 'h':
        default:
          print_usage (av[0]);
        }
    }
  return optind;
}

void
report_counters (uint32_t ring_id)
{
  /* Accumulate throughput */
  int i;
  for (i = 0; i < flag_nthreads; i++)
    {
      printf ("Thread[%d] speed (Ring %d)\t: %1.3f Mbps\n",
              i, ring_id, thread_throughput[ring_id][i]);
      total_throughput += thread_throughput[ring_id][i];
    }
      
  if (ring_id == 0)
    {
      printf ("\t\t\t\t --------------\n");
      printf ("Approximate Throughput\t\t: %1.3f Mbps\n", total_throughput);
      printf ("\nNumber of Thread(s) per ring\t: %d\n", flag_nthreads);
      printf ("Manager Thread(s) per ring\t: %d\n", flag_use_manager);
      if (flag_stateful)
        printf ("Stateful Flows/Thread\t\t: %d\n", flag_numflows);
      else
        printf ("Stateless Flows/Thread\t\t: 1\n");
      printf ("Packet Size\t\t\t: %d\n", flag_packet_size);
      printf ("Number of Results\t\t: %d\n", num_results.counter);
      if (input_data_file)
        printf ("Traffic type\t\t\t: %s\n", input_data_file);
      else if (flag_random_traffic)
        printf ("Traffic type\t\t\t: Random\n");
      else
        printf ("Traffic type\t\t\t: All zeros\n");
      printf ("Steady state iterations\t\t: %d\n\n", flag_steady_state_iterations);
    }
}

typedef struct thread_info
{
  pthread_t tid;
  struct nlm_device_config *config;
} thread_info_t;

/* Manager thread that arbitrates the hardware resources. This
   thread does it both for input and output fifo's */
static nlm_status
input_output_manager_thread (thread_info_t *info)
{
  struct nlm_device_config config;
  struct nlm_device *device;
  uint32_t tid = info->tid;
  struct nlm_device_config *master_config = info->config;
  
  /* Lets bind the thread to its cpu numbered by its thread ID */
  if (flag_binding)
    {
      cpu_set_t cpu_mask;
      uint32_t cpu_id;
      CPU_ZERO (&cpu_mask);
      cpu_id = (master_config->ring_id * (flag_nthreads + flag_use_manager)) + tid;
      CPU_SET (cpu_id, &cpu_mask);
      pthread_setaffinity_np (pthread_self(), sizeof (cpu_mask), &cpu_mask);
    }

  /* Our config parametres are identical to the master
     thread except our thread ID */
  config = *master_config;
  config.thread_id = NLM_MANAGER_THREAD_ID;
      
  TRY (nlm_device_attach (&config, &device));
  
  while(!stop_manager_thread[master_config->ring_id])
    {
      TRY (nlm_manage_input (device));
      TRY (nlm_manage_output (device));
    }

  /* Detach and cleanup our resources */
  TRY (nlm_device_detach (device));
  
  return NLM_OK;
}

/* Manager thread that arbitrates the hardware resources. This
   thread does only the input fifo */
static nlm_status
input_only_manager_thread (thread_info_t *info)
{
  struct nlm_device_config config;
  struct nlm_device *device;
  uint32_t tid = info->tid;
  struct nlm_device_config *master_config = info->config;

  /* Lets bind the thread to its cpu numbered by its thread ID */
  if (flag_binding)
    {
      cpu_set_t cpu_mask;
      uint32_t cpu_id;
      CPU_ZERO (&cpu_mask);
      cpu_id = (master_config->ring_id * (flag_nthreads + flag_use_manager) + tid);
      CPU_SET (cpu_id, &cpu_mask);
      pthread_setaffinity_np (pthread_self(), sizeof (cpu_mask), &cpu_mask);
    }

  /* Our config parametres are identical to the master
     thread except our thread ID */
  config = *master_config;
  config.thread_id = NLM_MANAGER_THREAD_ID;
      
  TRY (nlm_device_attach (&config, &device));
  
  while(!stop_manager_thread[master_config->ring_id])
    {
      TRY (nlm_manage_input (device));
    }

  /* Detach and cleanup our resources */
  TRY (nlm_device_detach (device));
  
  return NLM_OK;
}

/* Manager thread that arbitrates the hardware resources. This
   thread manager only output FIFO */
static nlm_status
output_only_manager_thread (thread_info_t *info)
{
  struct nlm_device_config config;
  struct nlm_device *device;
  uint32_t tid = info->tid;
  struct nlm_device_config *master_config = info->config;

  /* Lets bind the thread to its cpu numbered by its thread ID */
  if (flag_binding)
    {
      cpu_set_t cpu_mask;
      uint32_t cpu_id;
      CPU_ZERO (&cpu_mask);
      cpu_id = (master_config->ring_id * (flag_nthreads + flag_use_manager) + tid);
      CPU_SET (cpu_id, &cpu_mask);
      pthread_setaffinity_np (pthread_self(), sizeof (cpu_mask), &cpu_mask);
    }

  /* Our config parametres are identical to the master
     thread except our thread ID */
  config = *master_config;
  config.thread_id = NLM_MANAGER_THREAD_ID;
      
  TRY (nlm_device_attach (&config, &device));
  
  while(!stop_manager_thread[master_config->ring_id])
    {
      TRY (nlm_manage_output (device));
    }

  /* Detach and cleanup our resources */
  TRY (nlm_device_detach (device));
  
  return NLM_OK;
}

static nlm_status
collect_results (struct nlm_device *device, nlm_uintptr_t tid, uint32_t n_jobs)
{
  uint32_t jobs_completed = 0;
  while (jobs_completed != n_jobs)
    {
      struct nlm_result matches[FIFO_SIZE];
      uint32_t n_matches;
      uint32_t i;
      
      /* get all results from output fifo */
      TRY (nlm_get_all_search_results (device, FIFO_SIZE, matches, &n_matches));
      for (i = 0; i < n_matches; i++)
        {
          if (matches[i].cookie != (void *) tid)
            {
              printf ("\033[31mIncorrect cookie received in result buffer\n\033[0m");
              return NLM_INVALID_JOB;
            }
          
          switch (matches[i].status)
            {
            case NLM_OK:
            case NLM_END_ANCHORED:
              if (flag_verbose)
                printf ("rule = %d byte offset = %d flow offset = %"PRId64" status = '%s'\n",
                        matches[i].u.match.rule_id, matches[i].u.match.byte_offset,
                        matches[i].u.match.flow_offset,
                        nlm_get_status_string (matches[i].status));
              (void) NLM_ATOMIC_ADD32 (&num_results, 1);
              break;
            
            case NLM_END_OF_JOB:
              jobs_completed++;
              break;
              
            default:
              printf ("Search result #%d error: status %d %s\n",
                      i, matches[i].status,  nlm_get_status_string (matches[i].status));
              break;
            }
        }
    }

  return NLM_OK;
}

/* Main process where the dataplane threads can send packets
   to the device and obtain results */
static nlm_status
packet_process (struct nlm_device *device, uint32_t tid,
                struct nlm_device_config *config)
{
#define PACKETS 32
  int32_t i, j, val;
  struct nlm_job *job[PACKETS];
  char *packet_start, *packet_end;
  struct nlm_flow **flow;
  uint32_t pkt_mem_size;
  uint64_t total_bytes = 0;
  struct timeval tv; /* as clock() is not available on some systems */
  uint32_t start_time, end_time;
  uint32_t flow_id;

  return 0;

  printf("[%s] ring_id:%d, thread_id:%d\n", __FUNCTION__, config->ring_id ,config->thread_id);
  
  /* Divide the packet memory equally among all the threads */
  pkt_mem_size = (2 * 1024 * 1024) / flag_nthreads;
  packet_start = (char *) config->packet_base_virt + pkt_mem_size * tid;
  packet_end = packet_start + pkt_mem_size;
  
  /* Create the required number of flows for ourselves */
  if (flag_stateful)
    {
      flow = malloc (flag_numflows * sizeof (struct nlm_flow *));
      for (i = 0; i < flag_numflows; i++)
        {
          /* create stateful flow to search against database 0 */
          TRY (nlm_create_flow (device, NLM_FLOW_TYPE_STATEFUL, &flow[i]));
          TRY (nlm_flow_add_database_and_group (device, flow[i], 0, flag_group_id));
        }
    }
  else
    {
      flag_numflows = 1;
      flow = malloc (sizeof (struct nlm_flow *));
      TRY (nlm_create_flow (device, NLM_FLOW_TYPE_STATELESS, &flow[0]));
      TRY (nlm_flow_add_database_and_group (device, flow[0], 0, flag_group_id));
    }

  /* Wait for all the dataplane threads to be ready */
  val = NLM_ATOMIC_ADD32_RETURN (&ready_to_run[config->ring_id], 1);
  
  printf("[%s] val:%d\n", __FUNCTION__, val);

#if 0
  if (val == flag_nthreads)
    start_work[config->ring_id] = 1;
  else
    while (start_work[config->ring_id] == 0);
#endif
  
  /* has to use gettimeofday(), as clock() is not available on some systems */
  gettimeofday (&tv, 0);
  start_time = tv.tv_sec * 1000000 + tv.tv_usec;

  /* Rip through the packet memory, enqueue one packet, wait for
     results. During this process round robin through the flows.
     We do this number of times so that all threads reach steady state
     through the device */

 // if(config->ring_id >= 4)
//	  flag_steady_state_iterations = 0;

	flag_steady_state_iterations = 1;

  for (j = 0; j < flag_steady_state_iterations; j++)
    {
      flow_id = 0;
      packet_start = (char *) config->packet_base_virt + pkt_mem_size * tid;

	  printf("[%s] pkt_start:0x%x, pkt_end:0x%x\n", __FUNCTION__, packet_start, packet_end);

      while (packet_start + flag_packet_size < packet_end)
        {
          for (i = 0; (packet_start + flag_packet_size) < packet_end && i < PACKETS; i++)
            {
              TRY (nlm_flow_enqueue_search (device, flow[flow_id], packet_start, 
                                            packet_start + flag_packet_size,
                                            (void *) (long) tid, &job[i]));
              packet_start += flag_packet_size;
              total_bytes += flag_packet_size;
              flow_id++;
              if (flow_id == flag_numflows)
                flow_id = 0;
            }
          
          //DPRINT ("Starting jobs, numflows: %d, job: %p\n", numflows, job);
          printf("Starting jobs, numflows: %d, job: %p\n", flag_numflows, job);
          TRY (nlm_start_jobs (device, i, job));
		  if(config->ring_id < 4)
			  TRY (collect_results (device, tid, i));
        }
    }

  printf("[%s] end job and collect result\n", __FUNCTION__);

  gettimeofday (&tv, 0);
  end_time = tv.tv_sec * 1000000 + tv.tv_usec;
  thread_throughput[config->ring_id][tid] = total_bytes * 8. / (end_time - start_time);

  /* Wrap up the search. */
  if (flag_stateful)
    {
      for (i = 0; i < flag_numflows; i++)
        {
          int status;
          TRY (nlm_flow_remove_database_and_group (device, flow[i], 0, flag_group_id));
          status = nlm_destroy_stateful_flow (device, flow[i], (void *) (long) tid);
          if (status == NLM_OK)
            {
				if(config->ring_id < 4)
				  TRY (collect_results (device, tid, 1));
            }
          else if (status == NLM_END_OF_JOB)
            continue;
          else
            {
              printf ("Destroy stateful flow failed\n");
              exit (1);
            }
        }
    }
  else
    {
      TRY (nlm_destroy_stateless_flow (device, flow[0]));
	 // printf("[%s] destroy stateless flow\n", __FUNCTION__);
    }

  printf("[%s] free flow\n", __FUNCTION__);

  free (flow);

  return NLM_OK;
}

static nlm_status
dataplane_thread (thread_info_t *info)
{
  struct nlm_device *device;
  struct nlm_device_config config;
  uint32_t tid = info->tid;
  struct nlm_device_config *master_config = info->config;

  printf("[%s]\n", __FUNCTION__);

  /* Lets bind the thread to its cpu numbered by its thread ID */
  if (flag_binding)
    {
      cpu_set_t cpu_mask;
      uint32_t cpu_id;
      CPU_ZERO (&cpu_mask);
      cpu_id = (master_config->ring_id * (flag_nthreads + flag_use_manager) + tid);
      CPU_SET (cpu_id, &cpu_mask);
      pthread_setaffinity_np (pthread_self (), sizeof (cpu_mask), &cpu_mask);
    }
  
  /* Our config parametres are identical to the master
     thread except our thread ID */
  config = *master_config;
  config.thread_id = tid;
      
#if 1
  TRY (nlm_device_attach (&config, &device));
  
  /* Start sending work to the device */
  packet_process (device, tid, &config);

  /* Detach and cleanup our resources */
  TRY (nlm_device_detach (device));
#endif

  return NLM_OK;
}

static nlm_status
init_packet_memory (char *packet_memory, uint32_t mem_size)
{
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
  else if (flag_random_traffic)
    {
      int i, max = mem_size / sizeof (long int);
      long int *mem = (long int *) packet_memory;
      struct timeval tv;

      gettimeofday(&tv, 0);
      srandom(tv.tv_sec * 1000000 + tv.tv_usec); 

      for (i = 0 ; i < max; i++)
        mem[i] = random ();
    }
  else
    memset (packet_memory, 0, mem_size);

  return NLM_OK;
}

static nlm_status
attach_ring (void *arg)
{
  struct nlm_device *device;
  struct nlm_device_config config = DEFAULT_NLM_DEVICE_CONFIG;
  pthread_t *threadids;
  uint32_t tid = 0;
  thread_info_t *tinfo;
  uint32_t ring_id = (int) (long) arg;
  int i;

  printf("[%s] ring_id:%d\n", __FUNCTION__, ring_id);
  
  /* Lets bind the thread to its cpu numbered by its thread ID */
  if (flag_binding)
    {
      cpu_set_t cpu_mask;
      uint32_t cpu_id;
      CPU_ZERO (&cpu_mask);
      cpu_id = (ring_id * (flag_nthreads + flag_use_manager) + tid);
      CPU_SET (cpu_id, &cpu_mask);
      pthread_setaffinity_np (pthread_self (), sizeof (cpu_mask), &cpu_mask);
    }
  
  thread_throughput[ring_id] = calloc (flag_nthreads, sizeof (float));
  
  /* Override config defaults with specific values */
  config.size_of_database_extension_area = flag_spill_size;
  config.memory_pool_size = 32 * 1024 * 1024;
  config.max_threads = flag_nthreads;
  config.using_manager_thread = (flag_use_manager > 0) ? 1 : 0;

  config.input_fifo_size = FIFO_SIZE;
  config.output_fifo_size = 4 * config.input_fifo_size;
  config.ring_id = ring_id;

  TRY (nlm_prepare_device_config (0 /* use this HW device */, &config));

  TRY (init_packet_memory ((char *) config.packet_base_virt, 2 * 1024 * 1024));
  TRY (nlm_device_attach (&config, &device));

  if (ddr_size)
    {
      config.size_of_ddr_memory = ddr_size / nrings_active;
      config.base_addr_of_ddr_memory = ring_id * config.size_of_ddr_memory;
    }

  threadids = malloc ((flag_nthreads + flag_use_manager) * sizeof (pthread_t));
  tinfo = malloc ((flag_nthreads + flag_use_manager) * sizeof (thread_info_t));
  
  /* Create all the dataplane threads that will process the packets */
  for (i = 1; i < flag_nthreads; i++)
    {
      tinfo[i].tid = i;
      tinfo[i].config = &config;
      if (pthread_create (&threadids[i], NULL, (void *(*)(void *))dataplane_thread, &tinfo[i]) != 0)
        perror_exit ("\033[31mpthread_create() failed\033[0m");
    }

  if (flag_use_manager)
    {
      if (flag_use_manager == 1)
        {
          tinfo[i].tid = i;
          tinfo[i].config = &config;
          if (pthread_create(&threadids[flag_nthreads], NULL, 
                             (void *(*)(void *))input_output_manager_thread, &tinfo[i]) != 0)
            perror_exit ("\033[31mpthread_create() failed\033[0m");
          i++;
        }
      else
        {
          tinfo[i].tid = i;
          tinfo[i].config = &config;
          if (pthread_create(&threadids[flag_nthreads], NULL, 
                             (void *(*)(void *))input_only_manager_thread, &tinfo[i]) != 0)
            perror_exit ("\033[31mpthread_create() failed\033[0m");
          i++;
          tinfo[i].tid = i;
          tinfo[i].config = &config;
          if (pthread_create(&threadids[flag_nthreads + 1], NULL, 
                             (void *(*)(void *))output_only_manager_thread, &tinfo[i]) != 0)
            perror_exit ("\033[31mpthread_create() failed\033[0m");
          i++;
        }
    }
  
  /* At this point the master device can also participate
     in the packet processing */
  packet_process (device, 0, &config);
  
  /* Wait for all the data plane threads to complete */
  for (i = 1; i < flag_nthreads; i++)
    pthread_join (threadids[i], NULL);
  
  if (flag_use_manager)
    {
      stop_manager_thread[ring_id] = 1;
      pthread_join (threadids[flag_nthreads], NULL);
      if (flag_use_manager == 2)
        pthread_join (threadids[flag_nthreads + 1], NULL);
    }

  /* Detach and cleanup our resources */
  printf("[%s] before detach\n", __FUNCTION__);
  TRY (nlm_device_detach (device));
  printf("[%s] after detach\n", __FUNCTION__);

  TRY (nlm_free_device_config (&config));

  free (threadids);
  free (tinfo);
  
  return NLM_OK;
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

  TRY (nlm_device_try_load_database (device, db_image, db_size, (uint32_t) -1, db_id, flag_num_copies /* num of copies */));
  /* asynchronous request to start loading the database */
  status = nlm_device_load_database (device, db_image, db_size, db_id, flag_num_copies);

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
load_database (struct nlm_device *device, const char *db_file_name)
{
  uint8_t *database = NULL;
  uint32_t dbsize = 0;
  uint32_t start_time, end_time;
  struct timeval tv; /* as clock() is not available on some systems */
  FILE *dbfile;
  nlm_status status;
  
  dbfile = fopen (db_file_name, "r");
  if (!dbfile)
    {
      printf ("\033[31mSorry, unable to open rule file %s\033[0m", db_file_name);
      return NLM_INVALID_ARGUMENT;
    }
  
  /* read the database into the memory */
  if (fseek (dbfile, 0, SEEK_END))
    {
      printf ("\033[31mGuessing the db size failed\033[0m");
      return NLM_INVALID_ARGUMENT;
    }

  dbsize = ftell (dbfile);
  rewind (dbfile);
  
  if ((database = malloc (dbsize)) == NULL)
    {
      printf ("\033[31mcan't get memory\033[0m");
      return NLM_INVALID_ARGUMENT;
    }

  if (fread (database, sizeof(uint8_t), dbsize, dbfile) != dbsize)
    {
      printf ("\033[31mcan't read database file\033[0m");
      return NLM_INVALID_ARGUMENT;
    }

  if (fclose (dbfile))
    {
      printf ("\033[31mcan't close file\033[0m");
      return NLM_INVALID_ARGUMENT;
    }

  status = nlm_device_try_load_database (device, database, dbsize, (uint32_t) -1, 0, flag_num_copies);
  if (status != NLM_OK)
    {
      free (database);
      return status;
    }
  
  /* has to use gettimeofday(), as clock() is not available on some systems */
  gettimeofday (&tv, 0);
  start_time = tv.tv_sec * 1000000 + tv.tv_usec;
  
  status = nlm_device_load_database (device, database, dbsize, 0, flag_num_copies);

  /* all other flows can be searched normally at this time,
     we will poll here until our requested database is fully loaded,
     since we want to search it immediately */
  while (status)
    {
      if (status != NLM_LOADER_IN_PROGRESS)
        {
          printf ("load_database error: %s\n", nlm_get_status_string (status));
          exit (status);
        }
      else
        TRY (nlm_device_get_param (device, NLM_DATABASE_LOADER_STATUS, &status));
    }
  
  gettimeofday (&tv, 0);
  end_time = tv.tv_sec * 1000000 + tv.tv_usec;
  printf ("Database load time = %1.6f seconds\n",
          (double) (end_time - start_time) / 1000000);

  /* free it, since HW already stored it inside */
  free (database);
  return NLM_OK;
}

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

  TRY (nlm_database_add_rule (db, group, 5/*rule_id*/, regex_str, &rule));

  if (flag_mml)
    TRY (nlm_database_set_rule_action (db, rule, NLM_MIN_MATCH_LENGTH, flag_mml))

  TRY (nlm_database_compile (db, &db_image, &db_size));
  return NLM_OK;
#endif
}

#define MAXMESG 100

void *
cli_run(void *arg)
{
	RC_CLI_t rc;
	cli_init_data_t init_data;
	char input[MAXMESG];

	strcpy(init_data.prefix, "root");
	cli_init(&init_data);

	cli_engine("");

	for(;;) {
		fgets(input, MAXMESG, stdin);
		cli_engine(input);
	}

	return NULL;
}

int
main (int ac, char*av[])
{
  pthread_t ringids[NLM_LAST_UNUSED_RING_ID];
  pthread_t *threadids;
  uint32_t avindex;
  uint32_t cpucount, n_devices;
  long i;
  int rings, ring_mask;
  struct nlm_device_config config = DEFAULT_NLM_DEVICE_CONFIG;
  thread_info_t *tinfo;
  enum nlm_ring_id ring_id;
  pthread_t thr_cli;
  int status;
  
  avindex = process_args (ac, av);

  cpucount = sysconf (_SC_NPROCESSORS_ONLN);
  printf ("Detected %d CPU's on the system\n", cpucount);

  if (((flag_nthreads + flag_use_manager) * nrings_active) > cpucount)
    {
      printf ("\033[31mWarning: Requested number of threads greater than cpucount\n\033[0m");
      flag_binding = 0;
    }

  if (flag_verbose)
    {
      if (flag_db_file)
        printf ("Using database '%s' to search\n", flag_db_file);
      else
        printf ("Searching regex '%s'\n", regex_str);
    }

  TRY (compile_db ());

  /* Override config defaults wit specific values */
  config.size_of_database_extension_area = flag_spill_size;
  config.memory_pool_size = 32 * 1024 * 1024;
  config.max_threads = flag_nthreads;
  config.using_manager_thread = (flag_use_manager > 0) ? 1 : 0;

  config.input_fifo_size = FIFO_SIZE;
  config.output_fifo_size = 4 * config.input_fifo_size;
  
  if (ddr_size)
    {
      if (nrings_active)
        config.size_of_ddr_memory = ddr_size / nrings_active;
      else
        config.size_of_ddr_memory = ddr_size;
      config.base_addr_of_ddr_memory = 0; /* ring 0 */
    }
  
  TRY (nlm_get_device_count (&n_devices));
  
  if (n_devices == 0)
    perror_exit ("\033[31mNo NetL7 devices detected\033[0m");

  /* Get rest of the system memory/packet memory info from the driver */
  TRY (nlm_prepare_device_config (0 /* use this HW device */, &config));
  
  /* Init the device */
  TRY (nlm_device_init (&config, &master_device));

  /* Load the database */
  if (flag_db_file)
    {
      TRY (load_database (master_device, flag_db_file));
    }
  else
    {
      TRY (load_db (master_device, 0));
    }

#if 0
  if (pthread_create (&thr_cli, NULL, (void *(*)(void *))cli_run, &config) != 0){
	  printf("error pthread_create \n");
  }
#endif

  /* Lets spawn threads for each of the rings. This main thread
     will participate in the work of master ring 0. */
  rings = nrings_active;
  ring_mask = 8;
 // ring_id = NLM_PCIE_RING_3;
  ring_id = NLM_XAUI_RING_0;
  memset (ringids, 0, sizeof (ringids));

  while (rings > 1)
    {
      if (flag_attach_mask & ring_mask)
        {
          if (pthread_create (&ringids[ring_id], NULL, (void *(*)(void *)) attach_ring, (void *) ring_id) != 0)
            perror_exit ("\033[31mpthread_create() failed\033[0m");
          rings--;
        }
      ring_id--;
      ring_mask >>= 1;
    }
  
  /* The last remaining ring, which is master ring, we will
     participate 

     HW needs to know translation of virt->phys address for the packets,
     so we use packet_memory that driver allocated for us for demo purposes.
     Production system will know packet_base_virt and packet_base_phys beforehand */
  TRY (init_packet_memory ((char *) config.packet_base_virt, 2 * 1024 * 1024));
  
  threadids = malloc ((flag_nthreads + flag_use_manager) * sizeof (pthread_t));
  tinfo = malloc ((flag_nthreads + flag_use_manager) * sizeof (thread_info_t));
  thread_throughput[0] = calloc (flag_nthreads, sizeof (float));

  /* Create all the dataplane threads that will process the packets */
#if 1
  for (i = 1; i < flag_nthreads; i++)
    {
      tinfo[i].tid = i;
      tinfo[i].config = &config;
      if (pthread_create (&threadids[i], NULL, (void *(*)(void *))dataplane_thread, &tinfo[i]) != 0)
        perror_exit ("\033[31mpthread_create() failed\033[0m");
    }
#endif

  if (flag_use_manager)
    {
      if (flag_use_manager == 1)
        {
          tinfo[i].tid = i;
          tinfo[i].config = &config;
          if (pthread_create(&threadids[flag_nthreads], NULL, 
                             (void *(*)(void *))input_output_manager_thread, &tinfo[i]) != 0)
            perror_exit ("\033[31mpthread_create() failed\033[0m");
          i++;
        }
      else
        {
          tinfo[i].tid = i;
          tinfo[i].config = &config;
          if (pthread_create(&threadids[flag_nthreads], NULL, 
                             (void *(*)(void *))input_only_manager_thread, &tinfo[i]) != 0)
            perror_exit ("\033[31mpthread_create() failed\033[0m");
          i++;
          tinfo[i].tid = i;
          tinfo[i].config = &config;
          if (pthread_create(&threadids[flag_nthreads + 1], NULL, 
                             (void *(*)(void *))output_only_manager_thread, &tinfo[i]) != 0)
            perror_exit ("\033[31mpthread_create() failed\033[0m");
          i++;
        }
    }
  
  /* At this point the master device can also participate
     in the packet processing */
 packet_process (master_device, 0, &config);

  /* Wait for all the data plane threads to complete */
  for (i = 1; i < flag_nthreads; i++)
    pthread_join (threadids[i], NULL);
  
  if (flag_use_manager)
    {
      stop_manager_thread[config.ring_id] = 1;
      pthread_join (threadids[flag_nthreads], NULL);
      if (flag_use_manager == 2)
        pthread_join (threadids[flag_nthreads + 1], NULL);
    }
  
  /* wait for all the rings to complete work */
  for (i = 1; i < NLM_LAST_UNUSED_RING_ID; i++)
    {
      if (ringids[i] != 0)
        {
          pthread_join (ringids[i], NULL);
          report_counters (i);
          free (thread_throughput[i]);
        }
    }
  
 // report_counters (0);

  /* Clean up the device */
  TRY (nlm_device_fini (master_device));
  TRY (nlm_free_device_config (&config));
  
  free (threadids);
  free (tinfo);
  free (thread_throughput[0]);
  
  TRY (free_db ());

  printf("[%s] free resources\n", __FUNCTION__);

 // pthread_join(thr_cli, (void *)&status);
  return 0;
}
