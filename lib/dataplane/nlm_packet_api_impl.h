/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef __NLM_PACKET_API_IMPL_H
#define __NLM_PACKET_API_IMPL_H

#include <stdarg.h>

#include "nlm_packet_api.h"
#include "nlm_sync.h"
#include "nlm_dp_fifo.h"

#if defined (NLM_HW_MARS2) || defined (NLM_HW_MARS1)
/* mars[12] definitions */
#include "nlm_mars_formats.h"
#ifdef NLM_HW_MARS2
#define NLM_MAX_DATABASE_ID 7
#define NLM_MARS_TLB_SIZE 16
#define DISABLED_TLB_ENTRY 0xffffFFFF
#else
#define NLM_MAX_DATABASE_ID 1
#endif
#define NLM_MAX_GROUP_ID 1022
#define NLM_MAX_RULE_ID 0x1fffff
#define NLM_MARS_BLOCK_SIZE 0x2000

#define NLM_MAX_DATABASE_EXTENSION_AREA (64 * 1024 * 1024)
#define NLM_MAX_CONTEXT_SAVE_RESTORE_AREA (64 * 1024 * 1024)

#elif defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
/* famos definitions */
#include "nlm_fms_formats.h"
#define NLM_MAX_DATABASE_ID 3
#define NLM_MAX_GROUP_ID 1023
#define NLM_MAX_RULE_ID 0xffff
#define NLM_FMS_GROUPS_PER_PACKET 6
#define NLM_FMS_LOADER_CMD_BUFF_SIZE (14 * TOTAL_FAMOS_BLOCKS)
#define NLM_DDR_DMA_AREA_SIZE (32 * 1024)
#define NLM_MIN_FLOW_CNT (16)
/* max size of context save restore area is unlimited */
#elif defined (NLM_HW_MARS3) || defined (NLM_SW_MARS3_MODEL)
/* mars 3 definitions */
#include "nlm_mars3_formats.h"
#define NLM_MAX_DATABASE_ID 3
#define NLM_MAX_GROUP_ID 1023
#define NLM_MAX_RULE_ID 0xffff
#define NLM_FMS_GROUPS_PER_PACKET 6
#define NLM_MIN_FLOW_CNT (16)
#endif

#define ALIGN_UP(addr, align) ((((nlm_uintptr_t) (addr)) + ((align) - 1)) & ~((align) - 1))

#define NLM_MAX_PACKET_SIZE (64 * 1024)
#define NLM_MAX_INPUT_FIFO_SIZE (64 * 1024 * 1024)
#define NLM_MAX_OUTPUT_FIFO_SIZE (64 * 1024 * 1024)
#define NLM_MIN_CONTEXT_SAVE_RESTORE_AREA (8 * 1024)
#define NLM_MIN_MEMORY_POOL_SIZE (32 * 1024)
#define NLM_MIN_SYSMEM_SIZE (32 * 1024)

#if defined (NLM_HW_MARS1) && defined (NLX_LAYER)
#define NLM_PROTO(A) A##_mars1
#elif defined (NLM_HW_MARS2) && defined (NLX_LAYER)
#define NLM_PROTO(A) A##_mars2
#elif defined (NLM_HW_FMS) && defined (NLX_LAYER)
#define NLM_PROTO(A) A##_fms
#elif defined (NLM_SW_FMS_MODEL) && defined (NLX_LAYER)
#define NLM_PROTO(A) A##_fms_model
#elif defined (NLM_XAUI_EX2) && defined (NLX_LAYER)
#define NLM_PROTO(A) A##_xauiex2
#elif defined (NLM_HW_MARS3) && defined (NLX_LAYER)
#define NLM_PROTO(A) A##_mars3
#elif defined (NLM_SW_MARS3_MODEL) && defined (NLX_LAYER)
#define NLM_PROTO(A) A##_mars3_model
#else
#define NLM_PROTO(A) A
#endif

#if !defined (NLM_HW_MARS1) && !defined (NLM_HW_MARS2) && !defined (NLM_SW_FMS_MODEL) \
    && !defined (NLM_HW_FMS) && !defined (NLM_XAUI_EX2) \
    && !defined (NLM_HW_MARS3) && !defined (NLM_SW_MARS3_MODEL)
#error "Please specify HW the dataplane library should be built for"
#endif

/* FD/RD cookie encoded as higher 8 bits is virtual ring ID,
   the remaining 24 bits are job index */
#define NLM_THREAD_ID_MASK   0xFF000000
#define NLM_JOB_INDEX_MASK 0x00FFFFFF

#define NLM_ENCODE_COOKIE(thread_id, jobidx) ((((thread_id) << 24) & NLM_THREAD_ID_MASK) | ((jobidx) & NLM_JOB_INDEX_MASK))
#define NLM_GET_THREAD_ID(cookie) (((cookie) & NLM_THREAD_ID_MASK) >> 24)
#define NLM_GET_JOB_INDEX(cookie) ((cookie) & NLM_JOB_INDEX_MASK)

#define NLM_DEVICE_MAGIC 0xdeadbeef

typedef struct nlm_phy_device
{
  /* Put a bit pattern here. Just a weak way to
     ensure the device has been initialized */
  volatile uint32_t device_magic;

  spinlock_t hw_input_fifo_lock;
  spinlock_t hw_output_fifo_lock;
  spinlock_t hw_packet_base_lock;

#if defined(NLM_HW_MARS2) 
  spinlock_t device_lock;
  uint32_t dma_in_progress;           /* dma request is in progress */
#endif

  /* Array of Software FIFO's for each virtual ring
     allocated in system memory */
  nlm_job_fifo_t *sw_job_fifo;
  nlm_result_fifo_t *sw_result_fifo;

  /* user supplied configuration
     pointers to system memory and register map are part of configuration */
  struct nlm_device_config config;

  /* input and output fifos */
  volatile uint32_t *input_rd_ptr;  /* 32 bit FDF read pointer (sysmem updated by hardware) */
  uint32_t input_wrt_ptr;           /* cached sw copy, sent to hw via register write */
  volatile uint32_t *output_wrt_ptr;/* 32 bit RDF write pointer (sysmem updated by hardware) */
  uint32_t output_rd_ptr;           /* cached sw copy, sent to hw via register write */
  struct hw_fd_descriptor *input_fifo_base;      /* base of the FD fifo */
  struct hw_result_descriptor *output_fifo_base; /* base of the RD fifo */

  /* cached copies of packet bases which are preconfigured via hw registers
     and only packet offset portion is kept in pd/fd descriptors */
  nlm_phys_addr packet_base[HW_PACKET_BASES];

#if defined (NLM_HW_MARS1) || defined (NLM_HW_MARS2) || defined (NLM_SW_MARS3_MODEL) || defined (NLM_HW_MARS3)
  struct hw_pdf_descriptor *pdf_base;       /* base of the PDF area in system memory or DDR */
#else
  union
    {
      struct hw_pdf_descriptor *pdf_base;       /* base of the PDF area in system memory */
      nlm_phys_addr pdf_base_in_ddr;            /* base of PDF in DDR */
    } u;
#endif
  uint32_t chunksize;                       /* per thread system memory chunk from pdf_base */
  void *context_save_restore_area;          /* context save/restore area to be used by device to
                                               store engine state during flow switching*/
  uint8_t *database_extension_area;         /* database spillover area */
  uint8_t database_load_policy;             /* load/unload policy to use */

#if defined(NLM_HW_MARS2)
  uint8_t *database_base[NLM_MAX_DATABASE_ID + 1];
  uint8_t database_block_cnt[NLM_MAX_DATABASE_ID + 1];
  uint8_t database_total_block_cnt[NLM_MAX_DATABASE_ID + 1];
  union __mars_tlb
    {
      struct
        {
          __ENDIAN_ORDER8(
          uint32_t engine_enable:1,
          uint32_t host_enable:1,
          uint32_t rsv03:7,
          uint32_t database_id:3,
          uint32_t rsv02:5,
          uint32_t virt:7,
          uint32_t rsv01:4,
          uint32_t phys:4)
        } s;
      uint32_t word;
    } tlb[NLM_MARS_TLB_SIZE];
  uint8_t dma_database_being_loaded; /* database being loaded during this dma transfer */
  uint8_t dma_expected_block_cnt;    /* number of blocks being transfered */
#endif

#if defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
  spinlock_t ddr_dma_area_lock;
  uint8_t fms_ring_id;
  uint8_t use_ddr_memory;
  uint8_t num_fms_blocks;
  uint8_t packet_enqueue_policy;            /* packet enqueue policy to use */
  uint32_t num_rows_per_fms_block;
  uint32_t wait_timeout;
  uint32_t pause_timeout;                   /* User selectable pause_wait timeout. 0 == forever */
  uint32_t input_wrt_ptr_dirty:1;
  void *ddr_dma_area;
  struct hw_pd_descriptor *pd_base;         /* base of the PD area in system memory */
  uint32_t ddr_chunksize;                   /* per thread ddr memory chunk from pdf_base_in_ddr */
  uint16_t block_group_list[TOTAL_FAMOS_BLOCKS][NLM_MAX_GROUP_ID + 1];
                                                /**< Group list for each hw block */
  struct fms_gid_bitmap_data gid_bmap_shadow[NLM_MAX_DATABASE_ID + 1][NLM_MAX_GROUP_ID + 1];
                                                /**< GID BMAP shadow copy */
  uint8_t cmd_rd_ptr;                           /**< Loader Command instruction pointer */
  uint8_t cmd_wr_ptr;                           /**< Loader Command write pointer */
  struct fms_loader_command loader_cmd_mem[NLM_FMS_LOADER_CMD_BUFF_SIZE];
  /**< Loader Command buffer */
  struct fms_database_shadow_map_entry database_shadow_map[TOTAL_FAMOS_BLOCKS];
  uint8_t array_virt_ids[TOTAL_FAMOS_BLOCKS];
  uint8_t array_block_map_current[TOTAL_FAMOS_BLOCKS];
  uint8_t array_block_map_target[TOTAL_FAMOS_BLOCKS];
  uint8_t array_block_copy_number[TOTAL_FAMOS_BLOCKS];
  /**< Current layout of the extension area */
  struct ddr3_state ddr3;
  /**< DDR3 state storage */
#elif defined (NLM_HW_MARS3) || defined (NLM_SW_MARS3_MODEL)
  uint8_t packet_enqueue_policy;            /* packet enqueue policy to use */
  uint8_t fmn_ring_id;
  uint8_t nfa_cache_size;
  struct hw_pd_descriptor *pd_base;         /* base of the PD area in system memory */
#else
  uint8_t total_sram_blocks;                /* Device SRAM size */
#endif
#if defined (NLM_HW_MARS3)
  struct mars3_db_entry db_info[NLM_MAX_DATABASE_ID];
#endif
  uint8_t hw_type;                      /** HW type: 1 - mars1, 2 - mars2, 0 - unknown, 3 -famos */
#ifndef NLM_HW_MARS1
  uint8_t *first_free_db_block;
#endif
  atomic_t threads_attached;  /**< count of threads attached */
} nlm_phy_device_t;

/* Information maintained in the physical device */
typedef struct NLM_PROTO(nlm_device)              /* definition of device structure */
{
  /* Pointer to the physical device */
  nlm_phy_device_t *phy_device;
  /* ID of this thread */
  uint32_t thread_id;
  
  /* flow memory */
  struct nlm_flow *first_free_flow;         /* pointer to the first never used free flow */
#if defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
  uint32_t fms_ring_id;
  struct nlm_flow *stateful_flow_list_head; /* head of the list of free stateful flows */
  struct nlm_flow *stateless_flow_list_head;/* head of the list of free stateless flows */
  struct nlm_flow *pending_flow_list_head;  /* the list of stateful flows to be started */
  struct nlm_flow *pending_flow_list_tail;  /* tail of the list of stateful flows to be started */
#else
  struct nlm_flow *flow_list_head;          /* pointer to the head of the list of free flows */
#endif

  /* job memory */
  struct nlm_job *first_free_job;         /* pointer to the first never used free job */
  struct nlm_job *job_list_head;          /* pointer to the head of the list of free jobs */
  struct nlm_job *job_base;               /* base of job area in memory pool */

  struct hw_pdf_descriptor *first_free_pdf;
#if defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
  struct hw_pd_descriptor *pd_list_head;
  struct hw_pd_descriptor *first_free_pd;
  uint32_t max_pdf_index;
  uint32_t current_pdf_index;
#else /* Mars 1, 2, 3*/
  struct hw_pdf_descriptor *pdf_list_head;  /* pointer to the head of the list of free pdfs */
  struct hw_pdf_descriptor *pdf_base;       /* base of the PDF area in system memory or DDR */
  char *top_of_heap;
#endif

  struct nlm_job_fifo *sw_job_fifo;
  struct nlm_result_fifo *sw_result_fifo;

  /* Based off user supplied configuration */
  void *memory_pool;         /* Start of user memory of this thread */
  uint32_t memory_pool_size; /* Size of memory above */
  void *cookie;
} nlm_device_t;

/* base address of register map */
#if defined (__linux__) || defined (__CYGWIN__) || defined (_WIN32)
#define HW_REGISTER_MAP_BASE phy_device->config.register_map_base_virt
#elif defined (__OCTEON__)
#define HW_REGISTER_MAP_BASE phy_device->config.register_map_base_phys
#else
#error "OS is not supported"
#endif

/* convert virtual address to physical */
#define CONVERT_TO_PHYSICAL(PTR)                                        \
  ((phy_device)->config.sysmem_base_phys                                \
   + ((const char *) (PTR) - (const char *) (phy_device)->config.sysmem_base_virt))

struct nlm_flow                /* definition of flow structure */
{
  nlm_flow_type type;

#if defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2) 
  uint16_t db_group_id[NLM_FMS_GROUPS_PER_PACKET]; /**< current set of (database_id, group_id) pairs */
  uint8_t db_group_cnt;
  uint8_t errored;
  uint32_t db_group_combination;
  uint32_t last_db_group_combination; /**< last db_group_combination used in nlm_flow_add_job_to_list() */
  uint32_t is_stateless:1;
  struct hw_pd_descriptor *last_pd;
  struct hw_pd_descriptor *pending_pd_head;
  struct nlm_flow *next; /**< pointer to the next flow in the list of pending flows */
#else /* mars family of devices */
  uint16_t database_id;
  uint16_t group_id;
  uint16_t cache_db_group_id; /**< store the old database_id and group_id, used for stateful flows */
#endif

  union
    {
      struct __stateful
        {
#if defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
          uint32_t pdf_index;
#else
          struct hw_pdf_descriptor *pdf;
#endif
          uint64_t flow_offset;
        } stateful;
#if defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
      struct __stateless
        {
          /** number of in-flight jobs in the stateless flow */
          uint32_t job_seq_id;
        } stateless;
#endif
    } u;
  
#if defined (NLM_XAUI_EX2)
  uint8_t passthrough;
#endif
};

struct nlm_job
{
  uint64_t flow_offset;
  struct nlm_flow *flow;
  void *user_cookie;

#if defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
  struct hw_pd_descriptor *pd;
  uint32_t db_group_combination;
  /** set of (database_id, group_id) pairs chosen for the given packet */
  uint16_t hw_db_group_id[NLM_FMS_GROUPS_PER_PACKET];
  uint8_t hw_db_group_cnt;
#else
  uint16_t hw_database_id;
  uint16_t hw_group_id;
#endif

#if defined (NLM_XAUI_EX2)
  uint8_t passthrough;
#endif

  struct hw_fd_descriptor fd;
};

/*  one RAM entry */
typedef struct ram_entry
{
  uint32_t  data_a;
  uint32_t  data_b;
  uint32_t  data_c;
} ram_entry;

/* the following macros need to be called from a function with valid 'device' pointer */

#if defined (NLM_HW_MARS2)
nlm_status nlm_device_get_database_block_cnt_mars2 (nlm_device_t *device, uint32_t database_id,
                                                    uint32_t *p_num_blocks);
nlm_status nlm_device_get_database_total_block_cnt_mars2 (nlm_device_t *device,
                                                          uint32_t database_id,
                                                          uint32_t *p_num_blocks);
void nlm_device_check_dma_completion_mars2 (nlm_phy_device_t *device);
nlm_status nlm_device_set_database_block_cnt_mars2 (nlm_device_t *device, uint32_t database_id,
                                                    uint32_t num_blocks);
#elif defined (NLM_HW_MARS1)
nlm_status nlm_device_get_database_block_cnt_mars1 (nlm_device_t *device, uint32_t database_id,
                                                    uint32_t *p_num_blocks);
nlm_status nlm_device_get_database_total_block_cnt_mars1 (nlm_device_t *device,
                                                          uint32_t database_id,
                                                          uint32_t *p_num_blocks);
#elif defined (NLM_SW_FMS_MODEL) || defined (NLM_HW_FMS) || defined (NLM_XAUI_EX2)
nlm_status nlm_device_loader_cmd_exec_fms (nlm_device_t *device);
nlm_status nlm_device_database_set_num_copies (nlm_device_t *device, uint32_t database_id,
                                                uint32_t num_copies);
nlm_status nlm_device_database_get_num_copies (nlm_device_t *device, uint32_t database_id,
                                                uint32_t *num_copies_current);
nlm_status nlm_device_database_balance (nlm_device_t *device);
nlm_status nlm_device_init_ddr_memory (nlm_phy_device_t *device, uint64_t *size);
void nlm_device_copy_into_ddr (nlm_phy_device_t *device, nlm_phys_addr ddr_addr, uint32_t byte_cnt);
void nlm_device_copy_from_ddr (nlm_phy_device_t *device, nlm_phys_addr ddr_addr, uint32_t byte_cnt);
#endif

nlm_status NLM_PROTO(nlm_device_get_param_va) (nlm_device_t *device, nlm_device_param param, va_list
ap);
nlm_status NLM_PROTO(nlm_device_set_param_va) (nlm_device_t *device, nlm_device_param param, va_list ap);
nlm_status NLM_PROTO(nlm_device_estimate_memory_pool_size) (nlm_device_t *device, uint32_t max_flows,
                                          uint32_t max_jobs, uint32_t *size_out);

/* strict prototypes:
nlm_status NLM_PROTO(nlm_device_init) (struct nlm_device_config *config, struct nlm_device **p_device);
nlm_status NLM_PROTO(nlm_device_fini) (struct nlm_device *device);
nlm_status NLM_PROTO(nlm_device_try_load_database) (struct nlm_device *device, const void *database,
                                                    uint32_t db_size, uint32_t replacing_db_id,
                                                    uint32_t new_db_id, uint32_t num_blocks);
nlm_status NLM_PROTO(nlm_device_load_database) (struct nlm_device *device, const void *database,
                                                uint32_t db_size, uint32_t database_id,
                                                uint32_t num_blocks);
nlm_status NLM_PROTO(nlm_device_unload_database) (struct nlm_device *device, uint32_t database_id);
nlm_status NLM_PROTO(nlm_device_get_param) (struct nlm_device *device, nlm_device_param param, ...);
nlm_status NLM_PROTO(nlm_device_set_param) (struct nlm_device *device, nlm_device_param param, ...);
nlm_status NLM_PROTO(nlm_create_flow) (struct nlm_device *device, nlm_flow_type flow_type,
                                       struct nlm_flow **p_flow);
nlm_status NLM_PROTO (nlm_flow_add_database_and_group) (nlm_device_t *device, struct nlm_flow *flow,
                                                        uint32_t database_id, uint32_t group_id)
nlm_status NLM_PROTO (nlm_flow_remove_database_and_group) (nlm_device_t *device, struct nlm_flow *flow,
                                                           uint32_t database_id, uint32_t group_id)
nlm_status NLM_PROTO(nlm_destroy_stateful_flow) (struct nlm_device *device, struct nlm_flow *flow,
                                                 void *cookie);
nlm_status NLM_PROTO(nlm_destroy_stateless_flow) (struct nlm_device *device, struct nlm_flow *flow);
nlm_status NLM_PROTO(nlm_flow_enqueue_search) (struct nlm_device *device, struct nlm_flow *flow,
                                               const void *start, const void *end,
                                               void *cookie, struct nlm_job **p_job);
nlm_status NLM_PROTO(nlm_cancel_job) (struct nlm_device *device, struct nlm_job *job);
nlm_status NLM_PROTO(nlm_start_jobs) (struct nlm_device *device, uint32_t n_jobs,
                                      struct nlm_job *jobs[]);
nlm_status NLM_PROTO(nlm_get_all_search_results) (struct nlm_device *device, uint32_t n_buf_entries,
                                                  struct nlm_result *buf, uint32_t *n_results);
*/
#endif /* __NLM_PACKET_API_IMPL_H */
