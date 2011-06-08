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
static uint32_t flag_db_id = 0;

uint32_t flag_verbose = 0;
uint32_t group_id = 1;
char *db_file = NULL;
uint32_t min_packet_size = 64;
uint32_t max_packet_size = 32768;
static char *input_data_file = NULL;
#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2)
static uint32_t num_of_copies = 0;
#else
static uint32_t num_of_copies = 1;
#endif
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

        case 'e':
          flag_spill_size = atoi (optarg) * 1024 * 1024;
          break;

        case 'c':
          num_of_copies = atoi (optarg);
          if (num_of_copies > 4)
            {
              printf ("\033[31mNumber of database copies can be 0, 1, 2, 3, 4.\033[0m\n");
              exit (1);
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


#define ETHERTYPE_FMS   0x4321
#define IP_PROTO_UDP	17
#define IP_PROTO_ICMP	1
#define IP_PROTO_TCP	6
#define DATA_LEN	32

#define PROMISCUOUS 1
#define NONPROMISCUOUS 0
#define PACKETS	32

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
	fms_hdr->h_offset = 2;

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
			TRY (nlm_flow_add_database_and_group (device, flow[i], flag_db_id, flag_group_id));
		}
	}else{
		flag_numflows = 1;
		flow = malloc (sizeof (struct nlm_flow *));
		TRY (nlm_create_flow (device, NLM_FLOW_TYPE_STATELESS, &flow[0]));
		TRY (nlm_flow_add_database_and_group (device, flow[0], flag_db_id, flag_group_id));
	}

	packet_tx(device, flow[0], FMS_FLOW_INIT);

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

	status = nlm_get_device_count (&device_cnt);
	if (status != NLM_OK)
		return status;

	TRY (nlm_prepare_device_config (NLM_MASTER_RING , &config_m));
	TRY (nlm_device_init (&config_m, &master_device));

	if (db_file)
		TRY (load_db_from_file (master_device, db_file, flag_db_id))
	else
		TRY (load_db (master_device, flag_db_id))

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

	packet_process (master_device, &config_a, 0);

	TRY (nlm_device_fini (master_device));
	TRY (nlm_free_device_config (&config_m));

	return 0;
}
