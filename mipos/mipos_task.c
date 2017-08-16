/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#include "mipos_kernel.h"


/* -------------------------------------------------------------------------- */

mipos_task_id_t mipos_t_create(mipos_task_name_t name,
    task_entry_point_t entry_point,
    task_param_t param,
    mipos_task_stack_ptr_t task_stack,
    mipos_task_stack_size_t stack_size,
    t_create_flags_t flags)
{
    mipos_task_id_t id = 1;
    mipos_init_cs();

    mipos_enter_cs();
    for (; id < MIPOS_TASKS_TBL_ENTRIES_CNT; ++id) {
        mipos_task_t * p_task = &(KERNEL_ENV.task_table[id]);

        if (!(p_task->entry_point)) {
            _mipos_t_create(name,
                p_task,
                entry_point,
                param,
                flags);

            p_task->task_stack = task_stack;
            p_task->stack_size = stack_size;

            mipos_leave_cs();
            return id;
        }
    }
    mipos_leave_cs();

    return (mipos_task_id_t)-1;
}


/* -------------------------------------------------------------------------- */

mipos_task_t * _mipos_get_p_task(mipos_task_id_t id)
{
    mipos_task_t * p_task = 0;

    if (id == 0) {
        // current task
        p_task = KERNEL_ENV.task_context_ptr;
    }
    else if (id > 0 && id < MIPOS_TASKS_TBL_ENTRIES_CNT) {
        p_task = &(KERNEL_ENV.task_table[id]);
    }

    return p_task;
}


/* -------------------------------------------------------------------------- */

void _mipos_t_delete(mipos_task_t * p_task)
{
    p_task->entry_point = 0;
    p_task->param = 0;

    p_task->signal_mask = 0;
    p_task->signal_pending = 0;
    p_task->signal_waiting = 0;

    p_task->status = TASK_NOT_RUNNING;
    p_task->u_flags.i_flags = 0;
    p_task->timer_tick_count = 0;
}


/* -------------------------------------------------------------------------- */

void mipos_t_delete(mipos_task_id_t id)
{
    mipos_task_t * p_task;

    mipos_init_cs();

    mipos_enter_cs();

    p_task = _mipos_get_p_task(id);

    if (p_task) {
        _mipos_t_delete(p_task);
    }

    mipos_leave_cs();
}


/* -------------------------------------------------------------------------- */

task_status_t mipos_t_get_status(mipos_task_id_t id)
{
    task_status_t status = (task_status_t)-1;
    mipos_task_t * p_task;

    mipos_init_cs();

    mipos_enter_cs();

    p_task = _mipos_get_p_task(id);

    if (p_task) {
        status = p_task->status;
    }

    mipos_leave_cs();

    return status;
}


/* -------------------------------------------------------------------------- */


