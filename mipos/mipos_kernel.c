/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#include "mipos_kernel.h"
#include "mipos_bsp.h"

#ifndef KERNEL_DBG_DELAY
#  define KERNEL_DBG_DELAY
#endif


/* -------------------------------------------------------------------------- */

#define KERNEL__IDLE_TASK_ID 0


/* -------------------------------------------------------------------------- */

kernel_t KERNEL_ENV = { 0 };


/* -------------------------------------------------------------------------- */

static int idle_task(task_param_t param)
{
    static idle_state_notify_cbk_t notify = 0;

    mipos_init_cs();

    mipos_enter_cs();

    //Keeps trace of the idle state
    ++KERNEL_ENV.idle_counter;
    notify = KERNEL_ENV.idle_state_notify_cbk;

    mipos_leave_cs();

    KERNEL_DBG_DELAY;

    //Notify to the user that we are in idle state
    if (notify) {
        notify(param);
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

void mipos_update_rtc(u32 quantum)
{
    KERNEL_ENV.rtc_counter += quantum;
    mipos_kick_watchdog();
}


/* -------------------------------------------------------------------------- */

static void scheduler_init(void)
{
    mipos_task_id_t id;

    //Initialize kernel environment
    KERNEL_ENV.task_count = 1; //IDLE
    KERNEL_ENV.current_task = KERNEL__IDLE_TASK_ID;
    KERNEL_ENV.tick_counter = 0L;
    KERNEL_ENV.rtc_counter = 0L;
    KERNEL_ENV.task_context_ptr =
        &(KERNEL_ENV.task_table[KERNEL__IDLE_TASK_ID]);

    _mipos_t_create("idle",
        KERNEL_ENV.task_context_ptr,
        idle_task,
        0,
        MIPOS_NO_RT);

    KERNEL_ENV.task_context_ptr->status = TASK_READY;
    KERNEL_ENV.task_context_ptr->signal_mask = 0;

    KERNEL_ENV.idle_state_notify_cbk = 0;
    KERNEL_ENV.idle_param = 0;

    // Initialize the task descriptor table
    for (id = KERNEL__IDLE_TASK_ID + 1 /* skip idle*/;
        id < MIPOS_TASKS_TBL_ENTRIES_CNT;
        ++id)
    {
        mipos_task_t * p_task = &(KERNEL_ENV.task_table[id]);
        _mipos_t_delete(p_task);
    }

    KERNEL_ENV.timer_bitmap = 0;
    KERNEL_ENV.next_rtc_timeout = 0;

    for (id = 0; id < MIPOS_MAX_TIMERS_NO; ++id) {
        mipos_timer_t * p_timer = &(KERNEL_ENV.timer_table[id]);

        p_timer->handler_on_expire_cbk = 0;
        p_timer->handler_on_cancel_cbk = 0;

        p_timer->context = 0;
        p_timer->rtc_timeout = 0;
        p_timer->flgs = 0;
    }

}


/* -------------------------------------------------------------------------- */

static void * saved_stack_pointer;
static void * top_of_the_stack;


/* -------------------------------------------------------------------------- */

/* Kernel entry point */
void mipos_start(task_entry_point_t root_task,
    task_param_t root_task_param,
    mipos_task_stack_ptr_t task_stack,
    mipos_task_stack_size_t stack_size)
{
    static int task_count;
    static int task_search_cond = 0;
    static mipos_task_t * p_task;

#if defined( __ICCARM__ )  
    static char isr_stack[MIPOS_ISR_STACK_SIZE];
#define isr_stack_size sizeof(isr_stack)

    /* Cortex M3 processor family */
    __SETUP_PROCESS_SP();
    __SETUP_EXCEPTION_SP(&isr_stack[isr_stack_size - 4]);
#else
    mipos_bps_init_sp();
#endif

    // Initialize the scheduler 
    scheduler_init();

    // Init devices and start watch dog
    mipos_bsp_setup_reset_and_wd();

    mipos_kick_watchdog();
    mipos_bsp_setup_clk();

    mipos_kick_watchdog();
    mipos_bsp_exception_vector();

    // Set up the HW timer for mipOS to generate mipOS tick
    mipos_kick_watchdog();

    // Create RTC time counter
    mipos_bsp_create_hw_rtc_timer();

    // Allow IRQ
    mipos_bsp_enable_irq_mask();

    // Start the root task 
    mipos_t_create("root",
        root_task,
        root_task_param,
        task_stack,
        stack_size,
        MIPOS_NO_RT);

    //---------
scheduler:
    //---------
    mipos_kick_watchdog();

    mipos_bsp_notify_scheduler_epoch();

    //---------------
    //  Soft-timers
    //---------------

    //If there are active timers
    if (KERNEL_ENV.timer_bitmap &&
        //rtc_next_timetout is undefined, or...
        ((!KERNEL_ENV.next_rtc_timeout) ||
            //... a timer is expired
            KERNEL_ENV.rtc_counter >= KERNEL_ENV.next_rtc_timeout))
    {
        int id = 0;

        //search for active timers
        for (; id < MIPOS_MAX_TIMERS_NO; ++id)
        {
            mipos_timer_bmp_t tmask = 1 << id;

            if (KERNEL_ENV.timer_bitmap & tmask) {
                mipos_timer_t* p_timer = &(KERNEL_ENV.timer_table[id]);

                // Timer has been cancelled, execute on-cancel 
                // handler (if it had been defined)
                // and remove the timer from active timer collection
                if (p_timer->flgs & TIMER_FLG_CANCELLED)
                {
                    KERNEL_ENV.timer_bitmap &= ~tmask;

                    if (p_timer->handler_on_cancel_cbk) {
                        p_timer->handler_on_cancel_cbk(p_timer->context);
                    }

                    //check for next timer
                    continue;
                }

                //if current timer count is expired...
                if (p_timer->rtc_timeout <= KERNEL_ENV.rtc_counter) {
                    
                    //invalidate next_rtc_timeout
                    KERNEL_ENV.next_rtc_timeout = 0;

                    p_timer->flgs |= TIMER_FLG_EXPIRED;

                    //finally execute the on-expire handler
                    p_timer->handler_on_expire_cbk(p_timer->context);

                    //if no periodic, de-active it
                    if (!(p_timer->flgs & TIMER_FLG_PERIODIC)) {
                        KERNEL_ENV.timer_bitmap &= ~tmask;
                    }
                    else {
                        p_timer->flgs &= (mipos_reg_t)(~TIMER_FLG_EXPIRED);

                        //Reset the timer count
                        p_timer->rtc_timeout =
                            KERNEL_ENV.rtc_counter + p_timer->interval;
                    }

                    //check for next timer
                    continue;
                }

            }
        }

        // Select next expiring timer
        for (id = 0; id < MIPOS_MAX_TIMERS_NO; ++id)
        {
            mipos_timer_t * p_timer = &(KERNEL_ENV.timer_table[id]);
            mipos_timer_bmp_t tmask = 1 << id;

            if (KERNEL_ENV.timer_bitmap & tmask) {
                if (!KERNEL_ENV.next_rtc_timeout ||
                    KERNEL_ENV.next_rtc_timeout > p_timer->rtc_timeout)
                {
                    KERNEL_ENV.next_rtc_timeout = p_timer->rtc_timeout;
                }
            }
        }
    }

    // Search for NORT task descriptor... 
    task_count = 0;
    do {
        mipos_init_cs();
        mipos_enter_cs();

        ++task_count;

        KERNEL_ENV.current_task =
            (KERNEL_ENV.current_task + 1) %
            MIPOS_TASKS_TBL_ENTRIES_CNT;

        p_task =
            &(KERNEL_ENV.task_table[KERNEL_ENV.current_task]);

        //RT task handling
        if (p_task->u_flags.flags.rt_task) {

            p_task->task_ret_value = p_task->entry_point(p_task->param);

            //Request for scheduling DPC 
            if (p_task->task_ret_value) {

                //re-schedule DPC
                KERNEL_ENV.current_task = (mipos_task_id_t)p_task->task_ret_value;

                p_task =
                    &(KERNEL_ENV.
                        task_table[KERNEL_ENV.current_task]);

                p_task->u_flags.flags.wkup = 1;
                p_task->status = TASK_READY;
            }
        }

        task_search_cond =
            // a) cannot find entry point or
            p_task->entry_point == 0 ||

            // b) the task is real-time or
            p_task->u_flags.flags.rt_task ||

            // c) the task is invalid or  
            (p_task->status &
            (TASK_NOT_RUNNING | TASK_ZOMBIE | TASK_FROZEN)) ||

            // d) the list of task descriptors has not been 
            //    completely visited
                ((task_count <= MIPOS_TASKS_TBL_ENTRIES_CNT &&
                    KERNEL_ENV.current_task == KERNEL__IDLE_TASK_ID));

        mipos_leave_cs();
    } while (task_search_cond);

    do {
        mipos_init_cs();

        mipos_enter_cs();
        task_search_cond =
            KERNEL_ENV.current_task == KERNEL__IDLE_TASK_ID;
        mipos_leave_cs();
    } while (0);


    // If selected task is IDLE, execute related entry point callback, then
    // restart scheduling
    if (task_search_cond) {

        p_task->entry_point(KERNEL_ENV.idle_param);
        goto scheduler;
    }

    KERNEL_DBG_DELAY;

    do
    {
        // tick counter is incremented for each TSM iteration
        ++KERNEL_ENV.tick_counter;

        // Move task_context_ptr to scheduled task
        KERNEL_ENV.task_context_ptr = p_task;

        // Task State Machine (TSM)
        switch (p_task->status)
        {
            //  Task state is "running", but its startup 
            //  routine terminated and the task hasn't ever been
            //  deleted, so it remains a zombie until it will be deleted
            case TASK_RUNNING:
            {
                mipos_init_cs();
                mipos_enter_cs();
                p_task->status = TASK_ZOMBIE;
                mipos_leave_cs();
            }
            goto scheduler;

            //  If Task state is "suspended", then manage wakeup event
            case TASK_SUSPENDED:
            {
                mipos_init_cs();
                mipos_enter_cs();

                if (p_task->signal_waiting & SIGALM) {
                    if (p_task->timer_tick_count > 0) {
                        --p_task->timer_tick_count;
                    }
                    else {
                        p_task->signal_pending |= SIGALM;
                    }
                }

                if (p_task->signal_waiting & SIGTMR)
                {
                    if (p_task->rtc_timeout &&
                        KERNEL_ENV.rtc_counter >= p_task->rtc_timeout)
                    {
                        p_task->signal_pending |= SIGTMR;
                        p_task->rtc_timeout = 0;
                    }
                }
                mipos_leave_cs();

                if (p_task->signal_pending & p_task->signal_waiting) {
                    mipos_init_cs();
                    mipos_enter_cs();
                    p_task->u_flags.flags.wkup = 1;
                    p_task->status = TASK_READY;

                    p_task->signal_pending &= ~(p_task->signal_pending &
                        p_task->signal_waiting);

                    mipos_leave_cs();
                    goto scheduler;
                }
            }
            break;

            // task is ready
            case TASK_READY: {
                mipos_init_cs();
                mipos_enter_cs();
                p_task->status = TASK_RUNNING;
                mipos_leave_cs();
            }

            goto dispatcher;

            // unexpected task status (BUG ?!): panic !!!
            case TASK_NOT_RUNNING:
            case TASK_ZOMBIE:
            default:
                KERNEL__PANIC("invalid process status: "
                    "system halted !");
        }
    } while (0);

    goto scheduler;

    //---------
dispatcher:
    //---------

    // Before a task execution, save current scheduler state
    if (mipos_save_context((u8*)KERNEL_ENV.scheduler_registers_state)) {
        //Resuming kernel context, go to the scheduler
        goto scheduler;
    }

    // The task has been suspended
    if (p_task->u_flags.flags.wkup) {
        // Reset wkup flag
        p_task->u_flags.flags.wkup = 0;

        // Resume a suspended task
        mipos_context_switch_to((u8*)p_task->reg_state);
    }
    else {
        mipos_init_cs();

        mipos_enter_cs();

        // First execution: set up the task stack frame
        saved_stack_pointer = &p_task->kernel_stack_pointer;

        top_of_the_stack =
            &p_task->task_stack[p_task->stack_size - sizeof(void*)];

        mipos_replace_sp(saved_stack_pointer,
            top_of_the_stack);

        mipos_leave_cs();

        // Run the task (first time)
        p_task->task_ret_value =
            p_task->entry_point(p_task->param);

        mipos_enter_cs();

        // Running task terminated,
        // restore the kernel stack pointer and go to the 
        // scheduler
        saved_stack_pointer = &p_task->kernel_stack_pointer;
        mipos_set_sp(saved_stack_pointer);

        mipos_leave_cs();
    }

    goto scheduler;
}


/* -------------------------------------------------------------------------- */

void mipos_get_rtc_counter(mipos_rtc_cntr_t * cnt)
{
    mipos_init_cs();
    mipos_enter_cs();

    *cnt = KERNEL_ENV.rtc_counter;

    mipos_leave_cs();
}


/* -------------------------------------------------------------------------- */

void mipos_set_idle_cbk(
    idle_state_notify_cbk_t idle_state_notify_cbk,
    task_param_t param
)
{
    mipos_init_cs();

    mipos_enter_cs();

    KERNEL_ENV.idle_state_notify_cbk = idle_state_notify_cbk;
    KERNEL_ENV.idle_param = param;

    mipos_leave_cs();
}


/* -------------------------------------------------------------------------- */
