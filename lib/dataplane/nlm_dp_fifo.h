/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#ifndef __NLM_DP_FIFO_H
#define __NLM_DP_FIFO_H

#if defined (NLM_HW_MARS2) || defined (NLM_HW_MARS1)
#include "nlm_mars_formats.h"
#elif defined (NLM_HW_MARS3) || defined (NLM_SW_MARS3_MODEL)
#include "nlm_mars3_formats.h"
#else
#include "nlm_fms_formats.h"
#endif

/* A simple dumb implementation of FIFO that is lockless
   for single reader/single writer only. If you have multiple
   readers or multiple writers, this is not for you. 
   
   The fifo in and out pointers are simple upward counters that will
   eventually wrap around. This is ok, as we account for the
   wrap around.

   The data type size is specialized for the FD and RD structures,
   So it is not generally suitable for pointer FIFO types */

typedef struct nlm_job_fifo
{
  uint32_t size;
  volatile uint32_t in;
  volatile uint32_t out;
  struct hw_fd_descriptor *buf;
} nlm_job_fifo_t;

typedef struct nlm_result_fifo
{
  uint32_t size;
  uint32_t skip;
  volatile uint32_t in;
  volatile uint32_t out;
  struct hw_result_descriptor *buf;
} nlm_result_fifo_t;

/* Init should be done in sequential part of code */
static __inline__ 
void job_fifo_init (nlm_job_fifo_t *fifo, uint32_t size, struct hw_fd_descriptor *buf)
{
  fifo->size = size;
  fifo->in = 0;
  fifo->out = 0;
  fifo->buf = buf;
}

/* Return number of slots free in the FIFO at this
   instant */
static __inline__ 
uint32_t job_fifo_free (nlm_job_fifo_t *fifo)
{
  return (fifo->size - fifo->in + fifo->out);
}

/* Since the fifo is an array of FD's and RD's
   we will essentially, first peek at the input
   side to get the pointer to the fifo buffer,
   once we populate it, we will actually push into
   the fifo, which is an update of the fifo index */
static __inline__ 
struct hw_fd_descriptor *peek_job_fifo_input (nlm_job_fifo_t *fifo)
{
  if (fifo->size - fifo->in + fifo->out == 0) /* fifo is full */
    return NULL;
  
  return &fifo->buf[fifo->in & (fifo->size - 1)];
}

/* Push value into FIFO. Assumes we have already
   peeked at the input, so no safety checking is done */
static __inline__ 
void push_job_fifo (nlm_job_fifo_t *fifo)
{
  fifo->in++;
}

/* Peek at the output to get the pointer to the fifo buffer,
   once we grab everything from it, we will actually pop
   the fifo, which is an update of the fifo index */
static __inline__ 
struct hw_fd_descriptor *peek_job_fifo_output (nlm_job_fifo_t *fifo)
{
  if (fifo->in == fifo->out) /* No results to peek at */
    return NULL;
  
  return &fifo->buf[fifo->out & (fifo->size - 1)];
}

/* Pop value from FIFO. Assumes we have peeked at the output,
   so no safety checking is done here */
static __inline__ 
void pop_job_fifo (nlm_job_fifo_t *fifo)
{
  fifo->out++;
}

static __inline__ 
uint32_t job_fifo_empty (nlm_job_fifo_t *fifo)
{
  return (fifo->in == fifo->out) ? 1 : 0;
}

/* Init should be done in sequential part of code */
static __inline__ 
void result_fifo_init (nlm_result_fifo_t *fifo, uint32_t size, struct hw_result_descriptor *buf)
{
  fifo->size = size;
  fifo->in = 0;
  fifo->out = 0;
  fifo->buf = buf;
}

/* Since the fifo is an array of RD's and RD's
   we will essentially, first peek at the input
   side to get the pointer to the fifo buffer,
   once we populate it, we will actually push into
   the fifo, which is an update of the fifo index */
static __inline__ 
struct hw_result_descriptor *peek_result_fifo_input (nlm_result_fifo_t *fifo)
{
  if (fifo->size - fifo->in + fifo->out == 0) /* fifo is full */
    return NULL;
  
  return &fifo->buf[fifo->in & (fifo->size - 1)];
}

/* Push value into FIFO. Assumes we have already
   peeked at the input, so no safety checking is done */
static __inline__ 
void push_result_fifo (nlm_result_fifo_t *fifo)
{
  fifo->in++;
}

/* Peek at the output to get the pointer to the fifo buffer,
   once we grab everything from it, we will actually pop
   the fifo, which is an update of the fifo index */
static __inline__ 
struct hw_result_descriptor *peek_result_fifo_output (nlm_result_fifo_t *fifo)
{
  if (fifo->in == fifo->out) /* No results to peek at */
    return NULL;
  
  return &fifo->buf[fifo->out & (fifo->size - 1)];
}

/* Pop value from FIFO. Assumes we have peeked at the output,
   so no safety checking is done here */
static __inline__ 
void pop_result_fifo (nlm_result_fifo_t *fifo)
{
  fifo->out++;
}

#endif /* __NLM_DP_FIFO_H */





