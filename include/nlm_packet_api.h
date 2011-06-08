/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

/** \file nlm_packet_api.h 
 header file for Dataplane/packet API */
#ifndef __NLM_PACKET_API_H
#define __NLM_PACKET_API_H

#include "nlm_common_api.h"

struct nlm_device;              /* prototype of device structure */
struct nlm_flow;                /* prototype of flow structure */
struct nlm_job;                 /* opaque job structure */

typedef enum nlm_ring_id
{
  NLM_MASTER_RING = 0,
  NLM_PCIE_RING_1,
  NLM_PCIE_RING_2,
  NLM_PCIE_RING_3,
  NLM_XAUI_RING_0,
  NLM_XAUI_RING_1,
  NLM_LAST_UNUSED_RING_ID
} nlm_ring_id;

#define NLM_MAX_THREADS        256
#define NLM_MANAGER_THREAD_ID -1

/** structure to pass configuration parameters to data plane library */
struct nlm_device_config
{
  void *register_map_base_virt;                 /**< virt base addr of register map */
  nlm_phys_addr register_map_base_phys;         /**< phys base addr of register map */
  uint32_t register_map_size;                   /**< size of register map in bytes */
  void *sysmem_base_virt;                       /**< virt base addr of system memory */
  nlm_phys_addr sysmem_base_phys;               /**< phys base addr of system memory */
  uint32_t sysmem_size;                         /**< size of system memory in bytes */
  uint32_t input_fifo_size;                     /**< num of elements in input fifo */
  uint32_t output_fifo_size;                    /**< num of elements in output fifo */
  uint32_t size_of_database_extension_area;     /**< database spill over size in bytes. Ignored by NLS2008 */
  uint32_t size_of_context_save_restore_area;   /**< context save/restore area in bytes */
  nlm_phys_addr (*virt_to_phys) (const void *); /**< convert virtual address to physical */
  nlm_phys_addr packet_base_phys;               /**< phys base addr of packet storage area */
  const void *packet_base_virt;                 /**< virt base addr of packet storage area */
  void *memory_pool;                            /**< user supplied memory pool */
  uint32_t memory_pool_size;                    /**< size of memory pool in bytes */
  int (*try_yield_cpu) (void);                  /**< function to try to yield cpu */

                                                /** NLS2008 specific configuration parameters */
  nlm_phys_addr base_addr_of_ddr_memory;        /**< base address of DDR memory (up to 64Gbyte) */
  uint64_t size_of_ddr_memory;                  /**< size of attached DDR memory
                                                     if non zero, it should be bigger
                                                     than size_of_context_save_restore_area */
  nlm_ring_id ring_id;                          /**< 0 - master ring
                                                     1,2,3 - slave pcie rings
                                                     4 - xaui0
                                                     5 - xaui1 (For mars take values in nlm_vring_id */
  void *cookie;                                 /**< handle to be passed to xmalloc, xfree functions.
                                                     Not interpreted by datapalne */
  void *(*xmalloc) (void *cookie, uint32_t);    /**< callback function to allocate virtual memory.
                                                     If specified, memory_pool pointer should be NULL.
                                                     For each thread at most memory_pool_size
                                                     of memory will be allocated using this callback.
                                                     It is assumed this callback function is MT safe */
  void (*xfree) (void *cookie, void *ptr);      /**< callback function to release virtual memory allocated
                                                     by call to xmalloc above. It is assumed this function
                                                     is MT safe. */

  uint32_t max_threads;                         /**< Maximum number of dataplane threads.
                                                     Default 1 */
  uint32_t thread_id;                           /**< Thread ID to which this config belongs */
  uint32_t using_manager_thread;                /**< Will use upto 2 threads for managing the device fifo's */

  int32_t reserved1;                            /**< Reserved for Device driver/Dataplane communication */
};

  
/** macro with default initialization values */
#define DEFAULT_NLM_DEVICE_CONFIG                \
  {                                              \
    0, 0, 0, 0, 0,                               \
    0x200000  /** 2Mbyte of sysmem */,           \
    1024,     /** 1K entries in input fifo */    \
    4096,     /** 4K entries in output fifo */   \
    0x180000, /** 1.5Mbyte of spillover */       \
    0x40000,  /** 256K of context ring */        \
    0, 0, 0, 0,                                  \
    0x40000,  /** 256K of memory pool */         \
    0,                                           \
    0,        /** base of DDR */                 \
    0,        /** size of DDR */                 \
    NLM_MASTER_RING /** ring ID */,              \
    0, 0, 0,  /** cookie, xmalloc, xfree */      \
    1,        /** Max Threads */                 \
    0,        /** Thread ID */                   \
    0,        /** Manager Thread */              \
    0         /** Reserved1 */                   \
  }

/** Estimate netl7 device configuration parameters based on number of flows and jobs
    and fill in different *_size fields of configuration.
  \param[in] n_flows number of flows
  \param[in] n_jobs number of jobs
  \param[out] config device configuration
  \return status */
nlm_status nlm_estimate_device_config (uint32_t n_flows, uint32_t n_jobs,
                                       struct nlm_device_config *config);


/** Initialize device with 'config' and return pointer to it
  \param[in] config device configuration parameters as specified in nlm_device_config structure
  \param[out] p_device returned device handle. This pointer should not be zero
  \return status 
  \warning Should be called only by thread_id = 0 */
nlm_status nlm_device_init (struct nlm_device_config *config, struct nlm_device **p_device);

/** Shutdown device
  \param[in] device device handle
  \return status 
  \warning should be called only by thread_id = 0 after all remaining DP threads
           have called nlm_device_detach */
nlm_status nlm_device_fini (struct nlm_device *device);

/** Attach to initialized device with 'config' and return pointer to it
  \param[in] config device configuration parameters as specified in nlm_device_config structure
  \param[out] p_device returned device handle. This pointer should not be zero
  \return status */
nlm_status nlm_device_attach (struct nlm_device_config *config, struct nlm_device **p_device);

/** Detach from the live device without shutting it down
  \param[in] device device handle
  \return status */
nlm_status nlm_device_detach (struct nlm_device *device);

/** Try loading the database
  \param[in] device device handle
  \param[in] database pointer to compiled rule image
  \param[in] db_size database buffer size 
  \param[in] replacing_db_id database id the new database is about to replace
  \param[in] new_db_id database id of the new database
  \param[in] num_blocks number of physical HW blocks to be occupied by database
  \return NLM_OK if the database can be successfully loaded or error code otherwise 
  \warning should be called only by thread_id = 0 */
nlm_status nlm_device_try_load_database (struct nlm_device *device, const void *database,
                                         uint32_t db_size, uint32_t replacing_db_id,
                                         uint32_t new_db_id, uint32_t num_blocks);

/** Load database onto the device.
  this function will call yield_cpu() callback from time to time
  to yield cpu to accommodate OSes with cooperative multi-tasking
  \param[in] device device handle
  \param[in] database pointer to compiled rule image
  \param[in] db_size database buffer size 
  \param[in] database_id id to be assigned to this database that further will be used 
             in nlm_device_unload_database() and nlm_create_flow() functions
  \param[in] num_blocks number of physical HW blocks to be occupied by database
  \return NLM_OK if database is fully loaded and error code otherwise 
  \warning should be called only by thread_id = 0 */
nlm_status nlm_device_load_database (struct nlm_device *device, const void *database,
                                     uint32_t db_size, uint32_t database_id, uint32_t num_blocks);

/** Unload database and free associated memory
  \param[in] device device handle
  \param[in] database_id id of the database to be unloaded
  \return status 
  \warning should be called only by thread_id = 0 */
nlm_status nlm_device_unload_database (struct nlm_device *device, uint32_t database_id);

/** configuration parameters that can be set and get by corresponding functions */
typedef enum nlm_device_param
{
  NLM_INVALID_PARAM
    = NLM_FIRST_DEVICE_PARAM,   /**< marker for invalid parameter */
  NLM_FLOW_OFFSET,              /**< offset of the last byte processed in the flow */
  NLM_HW_MEMORY,                /**< HW memory */
  NLM_HW_REGISTER,              /**< HW register */
  NLM_MAX_MATCHES_PER_JOB,      /**< maximum number of matches per job */
  NLM_MAX_STATES_PER_BYTE,      /**< maximum number of states per byte */
  NLM_OUTPUT_FIFO_TIMER,        /**< HW timer controls when the internal result fifo gets 
                                 flushed out to system memory */
  NLM_INPUT_FIFO_TIMER,         /**< HW timer that must pass before the DMA engine updates 
                                 the external input fifo read and output fifo write pointers 
                                 in system memory */
  NLM_TIMESTAMP,                /**< HW timestamp counter */
  NLM_DATABASE_COPY_CNT,        /**< number of database copies in the device */
  NLM_DATABASE_BLOCK_CNT,       /**< number of blocks loaded into HW memory for given database */
  NLM_DATABASE_TOTAL_BLOCK_CNT, /**< total number of blocks for given database
                                  total == number of blocks in HW memory + blocks in spillover */
  NLM_DATABASE_BALANCE,         /**< Balance loaded databases, set only parameter */
  NLM_DATABASE_LOAD_POLICY,     /**< database load/unload policy */
  NLM_DATABASE_LOADER_STATUS,   /**< info on the current loader state */
  NLM_TOTAL_BLOCK_CNT,          /**< total number of database blocks supported by HW */
  NLM_PACKET_ENQUEUE_POLICY,    /**< packet enqueue policy */
  NLM_FLOW_FORCE_ONE_FD,        /**< force FD to be generated for the next start_jobs() */
  NLM_XAUI_PASSTHROUGH_FLOW,
  NLM_LAST_DEVICE_PARAM
} nlm_device_param;

/** Get specified device parameter */
nlm_status nlm_device_get_param (struct nlm_device *device, nlm_device_param param, ...);
/** Set specified device parameter */
nlm_status nlm_device_set_param (struct nlm_device *device, nlm_device_param param, ...);

/** supported flow types */
typedef enum nlm_flow_type
{
  NLM_FLOW_TYPE_INVALID = NLM_FIRST_FLOW_TYPE, /**< marker for invalid flow */
  NLM_FLOW_TYPE_STATEFUL,   /**< stateful flow */
  NLM_FLOW_TYPE_STATELESS,  /**< stateless flow */
  NLM_LAST_FLOW_TYPE
} nlm_flow_type;

/** supported database load policies */
typedef enum nlm_database_load_policy
{
  NLM_DATABASE_LOAD_POLICY_DEFAULT = 0, /**< default load policy for this device */
  NLM_DATABASE_LOAD_POLICY_MANUAL,      /**< user specified load */
  NLM_DATABASE_LOAD_POLICY_ASYNC,       /**< FMS asynchronous load model */
  NLM_DATABASE_LOAD_POLICY_LAST_UNUSED
} nlm_database_load_policy;

/** Create flow of the type 'flow_type' on the 'device' for subsequent search
  in 'database_id'
  \param[in] device device handle
  \param[in] flow_type type of flow to be created
  \param[out] p_flow pointer to the created flow 
  \return status */
nlm_status nlm_create_flow (struct nlm_device *device, nlm_flow_type flow_type,
                            struct nlm_flow **p_flow);

/** add (database, rule_group) pair to the set of (database, group) pairs searched in 
    the following packets of this flow.
  \param[in] device device handle
  \param[in] flow flow handle
  \param[in] database_id database id to be used during the search
  \param[in] group_id scan packet with this group id
  \return status */
nlm_status nlm_flow_add_database_and_group (struct nlm_device *device, struct nlm_flow *flow,
                                            uint32_t database_id, uint32_t group_id);

/** remove (database, rule_group) pair to the set of (database, group) pairs searched in 
    the following packets of this flow.
  \param[in] device device handle
  \param[in] flow flow handle
  \param[in] database_id database id used during the search
  \param[in] group_id remove this group id
  \return status */
nlm_status nlm_flow_remove_database_and_group (struct nlm_device *device, struct nlm_flow *flow,
                                               uint32_t database_id, uint32_t group_id);

/** Destroy 'flow' on the 'device'.
  Sends visible destroy/finish flow request with given \a cookie
  \param[in] device device handle
  \param[in] flow flow handle
  \param[in] cookie is going to be accepted as-is and returned by nlm_get_all_search_results()
  \return status */
nlm_status nlm_destroy_stateful_flow (struct nlm_device *device, struct nlm_flow *flow,
                                      void *cookie);
/** Destroy 'flow' on the 'device'.
  Sends hidden destroy/finish flow request to HW
  \param[in] device device handle
  \param[in] flow flow handle
  \return status */
nlm_status nlm_destroy_stateless_flow (struct nlm_device *device, struct nlm_flow *flow);

/** Enqueue payload for the search from 'start' to 'end' in 'flow' with 'group_id'
  and 'cookie' which is going to be accepted as-is and returned by 
  nlm_get_all_search_results()
  Function returns 'job' pointer for this search request. 
  \param[in] device device handle
  \param[in] flow flow handle
  \param[in] start pointer to the first byte to be scanned
  \param[in] end pointer to the byte after the last byte to be scanned
  \param[in] cookie is going to be accepted as-is and returned by nlm_get_all_search_results()
  \param[out] p_job returns job pointer that tracks this search request. 
                    p_job pointer should not be zero
  \return status */
nlm_status nlm_flow_enqueue_search (struct nlm_device *device, struct nlm_flow *flow,
                                    const void *start, const void *end,
                                    void *cookie, struct nlm_job **p_job);

/** Cancel 'job' that wasn't sent to the 'device'
  \param[in] device device handle
  \param[in] job job handle to be cancelled 
  \return status */
nlm_status nlm_cancel_job (struct nlm_device *device, struct nlm_job *job);

/** Start searching 'jobs' on the 'device'
  \param[in] device device handle
  \param[in] n_jobs number of jobs in the array
  \param[in] jobs array of jobs to be started
  \return status */
nlm_status nlm_start_jobs (struct nlm_device *device, uint32_t n_jobs, struct nlm_job *jobs[]);

/** Information about one result found by HW 
    filled in by nlm_get_all_search_results() function */
struct nlm_result
{
  nlm_status status;      
  /**< status of this result.
    \li \c NLM_OK u.match.* fields contain info about match 
    \li \c NLM_END_ANCHORED u.match.* fields contain info about match that needs
       to be post-processed, since it was triggered by end-anchored rule
    \li \c NLM_END_OF_JOB u.stats.* fields contain statistics for completed job
    \li \c NLM_* u.* is undefined and status contains the error code
                     for abnormally completed job */
  void *cookie;                   /**< cookie for this result */
  union
    {
      struct
        {
          uint32_t rule_id;       /**< rule ID that triggered the match */
          uint32_t group_id;      /**< group ID that triggered the match */
          uint32_t database_id;   /**< database ID that triggered the match */
          uint32_t byte_offset;   /**< match offset in bytes from the beginning of the job 
                                       pointing to the last byte of the match */
          uint64_t flow_offset;   /**< match offset in bytes from the beginning of the flow
                                       pointing to the last byte of the match */
          uint32_t match_length;  /**< length of the match in bytes */
        } match;
      struct
        {
          uint64_t timestamp;      /**< HW timestamp when end_of_job result goes into ouput fifo */
          uint32_t total_state_cnt;/**< total number of states executed by HW engine */
          uint8_t peak_state_cnt;  /**< peak number of states seen by HW engine */
          uint8_t final_state_cnt; /**< final number of states seen by HW engine */
        } stats;
    } u;
};

/** Return all results found so far by \a device into \a buf buffer,
   but no more than \a n_buf_entries at a time.
  \param[in] device device handle
  \param[in] n_buf_entries requested number of entries in the result buffer
  \param[in] buf result buffer to store results
  \param[out] n_results actual number of results returned
  \return NLM_OK, if \a n_results were found and copied into \a buf,
                  \a n_results can be zero, which means that no new results
                  were found and \a buf is not changed */
nlm_status nlm_get_all_search_results (struct nlm_device *device, uint32_t n_buf_entries,
                                       struct nlm_result *buf, uint32_t *n_results);


/** Executed by the manager thread. Dispatches accumulated
    jobs from DP threads to the physical device.
    \param[in] device device handle
    \return NLM_OK, if no errors were encountered during submission
            of jobs to hardware */
nlm_status nlm_manage_input (struct nlm_device *device);
                              
/** Executed by the manager thread. Return results produced by 
    the physical device to the DP thread buffers for later 
    retreival by the DP threads
    \param[in] device device handle
    \return NLM_OK, if no errors were encountered during the
            the retreival of results from hardware */
nlm_status nlm_manage_output (struct nlm_device *device);

#endif /* __NLM_PACKET_API_H */
