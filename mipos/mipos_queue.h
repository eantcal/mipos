/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifndef mipos_QUEUE_H__
#define mipos_QUEUE_H__

/* ------------------------------------------------------------------------- */

#include "mipos_types.h"

/* ------------------------------------------------------------------------- */

typedef unsigned int mipos_q_item_t;
typedef unsigned short mipos_q_size_t;

/* ------------------------------------------------------------------------- */

typedef struct __queue_t
{
    mipos_task_id_t suspended_tid;
    mipos_q_size_t head, tail;
    mipos_q_item_t* item;
    mipos_q_size_t queue_limit;
    mipos_q_size_t queue_count;
} mipos_queue_t;

/* ------------------------------------------------------------------------- */

/** Initialize a message queue (FIFO)
 * @param queue: pointer to a mipos_queue_t instance
 * @param queue_pool: pointer to a pool used for the queue
 * @param queue_pool_length: length of the queue pool
 */
void mipos_q_init(mipos_queue_t* queue,
                  mipos_q_item_t* queue_pool,
                  mipos_q_size_t queue_pool_length);

/* ------------------------------------------------------------------------- */

/** This function sends a message to a specified ordinary
 *  message queue If a task is already waiting at the queue,
 *  the message is passed to that task, which is then
 *  unblocked and made ready to run.
 *  If no task is waiting, the input message is copied into
 *  a message buffer of the pool
 *
 *  Callable From: NON-RT TASK, RT-TASK
 *
 * @param queue: pointer to a mipos_queue_t instance
 * @param queue_pool: item to enqueue
 * @return number of enqueued items
 */
mipos_q_size_t mipos_q_send(mipos_queue_t* queue, mipos_q_item_t item);

/* ------------------------------------------------------------------------- */

/** Return current queue length
 *
 *  Callable From: NON-RT TASK, RT-TASK
 *
 * @param queue: pointer to a mipos_queue_t instance
 * @return number of enqueued items
 */
mipos_q_size_t mipos_q_count(const mipos_queue_t* queue);

/* ------------------------------------------------------------------------- */

/** Dequeue an item from the queue.
 *  Calling task is suspended until the queue is empty
 *
 *  Callable From: NON-RT TASK and RT_TASK
 *
 * @param queue: pointer to a mipos_queue_t instance
 * @param p_item: pointer to dequeued item
 * @param bl    : blocking mode (use FALSE within RT_TASK)
 * @return in case of success 0, -1 otherwise
 */
mipos_q_size_t mipos_q_receive(mipos_queue_t* queue,
                               mipos_q_item_t* p_item,
                               bool_t bl);

/* ------------------------------------------------------------------------- */

/** Returns non zero value if there are messages
 *  to be processed onto the queue
 *
 *  Callable From: NON-RT TASK, RT-TASK
 *
 *  @param queue: pointer to a mipos_queue_t instance
 *  @return 1 if there aren't messages to be processed
 */
int mipos_q_message_pending(const mipos_queue_t* queue);

/* ------------------------------------------------------------------------- */

#endif // mipos_QUEUE_H_
