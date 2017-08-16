/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


 /* ------------------------------------------------------------------------- */

#include "mipos_kernel.h"


/* -------------------------------------------------------------------------- */

void mipos_q_init(mipos_queue_t * queue,
    mipos_q_item_t * queue_pool,
    mipos_q_size_t queue_pool_length)
{
    mipos_init_cs();

    assert(queue && queue_pool && queue_pool_length > 0);

    mipos_enter_cs();

    queue->suspended_tid = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->item = queue_pool;
    queue->queue_limit = queue_pool_length;
    queue->queue_count = 0;

    mipos_leave_cs();
}


/* -------------------------------------------------------------------------- */

mipos_q_size_t mipos_q_send(mipos_queue_t * queue, mipos_q_item_t item)
{
    mipos_q_size_t ret_count = 0;
    mipos_init_cs();

    assert(queue);

    mipos_enter_cs();

    if (queue->queue_count < queue->queue_limit) {
        ++queue->queue_count;
        queue->item[queue->tail] = item;
        queue->tail = (queue->tail + 1) % queue->queue_limit;

        if (queue->suspended_tid) {
            mipos_t_notify_signal(queue->suspended_tid, SIGQUE);
        }

        ret_count = queue->queue_count;

        mipos_leave_cs();

        return ret_count;
    }
    else {
        if (queue->suspended_tid) {
            mipos_t_notify_signal(queue->suspended_tid, SIGQUE);
        }
    }
    mipos_leave_cs();

    return (mipos_q_size_t)-1;
}


/* -------------------------------------------------------------------------- */

mipos_q_size_t mipos_q_receive(mipos_queue_t * queue, mipos_q_item_t * p_item, bool_t bl)
{
    if (!p_item) {
        return (mipos_q_size_t)-1;
    }

    if (queue->queue_count == 0 && queue->suspended_tid == 0)  {
        queue->suspended_tid = KERNEL_ENV.current_task;

        if (bl) {
            _mipos_t_suspend(SIGQUE);
        }

        queue->suspended_tid = 0;
    }

    if (queue->queue_count > 0) {
        *p_item = queue->item[queue->head];

        queue->head = (queue->head + 1) % queue->queue_limit;

        --queue->queue_count;

        return 1;
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

mipos_q_size_t mipos_q_count(mipos_queue_t * queue)
{
    mipos_q_size_t ret_count = 0;
    mipos_init_cs();

    assert(queue);

    mipos_enter_cs();
    ret_count = queue->queue_count;
    mipos_leave_cs();

    return ret_count;
}


/* -------------------------------------------------------------------------- */

int mipos_q_message_pending(mipos_queue_t * queue)
{
    int ret = 0;
    mipos_init_cs();

    assert(queue);

    mipos_enter_cs();
    ret = !(queue->queue_count == 0 && queue->suspended_tid == 0);
    mipos_leave_cs();

    return ret;
}


/* -------------------------------------------------------------------------- */
