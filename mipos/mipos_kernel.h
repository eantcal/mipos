/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* ------------------------------------------------------------------------- */

#ifndef mipos_KERNEL_H__
#define mipos_KERNEL_H__


/* ------------------------------------------------------------------------- */

#define MIPOS_MAJ_VER 2
#define MIPOS_MIN_VER 0

#define MIPOS_STR_VER "mipOS v.2.0 - (c) 2005-2024 antonino.calderone@gmail.com"


/* ------------------------------------------------------------------------- */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ------------------------------------------------------------------------- */

#include "mipos_bsp.h"
#include "mipos_limits.h"
#include "mipos_mutex.h"
#include "mipos_queue.h"
#include "mipos_timer.h"
#include "mipos_types.h"


/* ------------------------------------------------------------------------- */

/** This API must be called periodically in order to
 *  update O/S RTC counter. The field quantum
 *  should be initialized with the interval of time between
 *  current notify and last one
 *  E.g. this update could be made in a execution context
 *  of a periodically timer ISR
 *
 *  Apply to: ISR
 */
void mipos_update_rtc(uint32_t quantum);


/* ------------------------------------------------------------------------- */

/* TASK status */

/** invalid task status */
#define TASK_NOT_RUNNING 1

/** ready to running */
#define TASK_READY 2

/** running state */
#define TASK_RUNNING 4

/** waiting for signal */
#define TASK_SUSPENDED 8

/** terminated but not deleted */
#define TASK_ZOMBIE 16

/** frozen - not more scheduled */
#define TASK_FROZEN 32


/* ------------------------------------------------------------------------- */

// System Signals
#define SYS_RESERVED_SIGNALS (SIGWKP | SIGQUE | SIGALM | SIGTMR)

#define SIGWKP 0x01
#define SIGQUE 0x02
#define SIGALM 0x04
#define SIGTMR 0x08

// User signals
#define SIGUSR1 0x10
#define SIGUSR2 0x20
#define SIGUSR3 0x40

#define SIGANY -1


/* ------------------------------------------------------------------------- */

// Reserved for internal use only
#define printk printf
#define KERNEL__PANIC(_MSG)                                                    \
    do {                                                                       \
        printk(_MSG);                                                          \
        while (1)                                                              \
            ;                                                                  \
    } while (0)


/* ------------------------------------------------------------------------- */

// Reserved for internal use only
#define _mipos_t_create(_NAME, _PTASK, _ENTRY_POINT, _PARAM, _FLAGS)           \
    do {                                                                       \
        (_PTASK)->name = _NAME;                                                \
        (_PTASK)->entry_point = _ENTRY_POINT;                                  \
        (_PTASK)->signal_mask = (mipos_task_sigmask_t)(SIGANY);                \
        (_PTASK)->signal_pending = 0;                                          \
        (_PTASK)->signal_waiting = 0;                                          \
        (_PTASK)->timer_tick_count = 0;                                        \
        (_PTASK)->rtc_timeout = 0;                                             \
        (_PTASK)->status = TASK_READY;                                         \
        (_PTASK)->param = _PARAM;                                              \
        (_PTASK)->u_flags.i_flags = 0;                                         \
        (_PTASK)->u_flags.flags.rt_task =                                      \
          ((_FLAGS)&MIPOS_RT) == MIPOS_RT ? 1 : 0;                             \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function creates and starts a new task
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *  @param  name        - const char ptr to task name (it must be
 *                        valid for whole task life)
 *  @param  entry_point - startup routine
 *  @param  param       - user's context
 *  @param  prio        - task priority
 *  @param  task_stack  - pointer to a free memory region
 *                        that task will be use for
 *                        the stack allocation
 *  @param  stack_size  - size of stack (in bytes)
 *  @param  flags       - creation flags
 *
 *  @return the task identifier
 */
mipos_task_id_t mipos_t_create(mipos_task_name_t name,
                               task_entry_point_t entry_point,
                               task_param_t param,
                               char* task_stack,
                               mipos_task_stack_size_t stack_size,
                               t_create_flags_t flags);


/* ------------------------------------------------------------------------- */

/** This function suspends a task until it receives
 *  a specified signal
 *
 *  Apply to: NON-RT TASK
 *
 *  @param _SIGNUM - signal mask
 *  @return the task identifier
 */
#define mipos_t_waitfor_signal(_SIGNUM)                                        \
    _mipos_t_suspend((_SIGNUM) & (~SYS_RESERVED_SIGNALS))


/* ------------------------------------------------------------------------- */

/** This function deletes a specified task
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *  @param id - the task identifier
 */
void mipos_t_delete(mipos_task_id_t id);


/* ------------------------------------------------------------------------- */

/** This function deletes a specified task
 *
 *  Apply to: NON-RT TASK
 *
 *  @param p_task - the task descriptor pointer
 *
 */
void _mipos_t_delete(mipos_task_t* p_task);


/* ------------------------------------------------------------------------- */

/** This function returns the status of specified task
 * ( running, suspended ...)
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *  @param  id - the task identifier
 *  @return task status or -1 if the id is invalid
 */
task_status_t mipos_t_get_status(mipos_task_id_t id);


/* ------------------------------------------------------------------------- */

/** This function returns a pointer to the task descriptor
 *
 *  Apply to: NON-RT TASK
 *
 *  @param  id - the task identifier (0 means current task)
 *  @return task descriptor or NULL if the id is invalid
 */
mipos_task_t* _mipos_get_p_task(mipos_task_id_t id);


/* ------------------------------------------------------------------------- */

/** This function resumes a specified task
 *   @param id - the task identifier
 *
 *  Apply to: NON-RT TASK, RT-TASK
 */
#define mipos_t_resume(_TID) mipos_t_notify_signal(_TID, SIGWKP)


/* ------------------------------------------------------------------------- */

/** This function suspends the calling task
 *
 *  Apply to: NON-RT TASK
 */
#define mipos_t_suspend() _mipos_t_suspend(SIGWKP)


/* ------------------------------------------------------------------------- */

/** This function restarts a ZOMBIE task.
 *  Task is ZOMBIE if it has completed execution
 *  but isn't deleted (via mipos_t_delete() syscall)
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 * @param _TID: tid of restarting task
 */
#define mipos_t_restart(_TID)                                                  \
    do {                                                                       \
        mipos_init_cs();                                                       \
        mipos_enter_cs();                                                      \
        mipos_task_t* p_task = _mipos_get_p_task(_TID);                        \
        if (p_task && p_task->entry_point && p_task->status == TASK_ZOMBIE) {  \
            p_task->status = TASK_READY;                                       \
        }                                                                      \
        mipos_leave_cs();                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/**
 * This function restarts a ZOMBIE tasks
 * (similar to mipos_t_restart),
 * but replace old task routine parameter with
 * new one provided with second argument
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 * @param _TID: tid of restarting task
 * @param _NEW_PARAM: new task parameter
 */
#define mipos_t_restart_with_param(_TID, _NEW_PARAM)                           \
    do {                                                                       \
        mipos_init_cs();                                                       \
        mipos_enter_cs();                                                      \
        mipos_task_t* p_task = _mipos_get_p_task(_TID);                        \
        if (p_task && p_task->entry_point && p_task->status == TASK_ZOMBIE) {  \
            p_task->status = TASK_READY;                                       \
            p_task->param = _NEW_PARAM;                                        \
        }                                                                      \
        mipos_leave_cs();                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function suspends the current task for _COUNT ticks
 *
 *  Apply to: NON-RT TASK
 *
 *   @param _COUNT : the number of kernel ticks
 */
#define mipos_tm_wkafter(_COUNT) _mipos_tm_wkafter(SIGALM, _COUNT)


/* ------------------------------------------------------------------------- */

/** This function call mipOS scheduler
 *
 *  Apply to: NON-RT TASK
 *
 */
#define mipos_schedule() mipos_tm_wkafter(0)


/* ------------------------------------------------------------------------- */

/** This function will cause the calling task to be
 *  suspended from execution until the number of real-time
 *  microseconds specified by the argument
 *  _COUNT has elapsed.
 *  The suspension time may be longer than requested due to
 *  the scheduling of other activity by the system and
 *  hw timer precision.
 *
 *  Apply to: NON-RT TASK
 *
 *  @param _COUNT : the number of microseconds, must be less
 *                  than 2^32.
 *                  If its value is 0, then the call has no
 *                  effect.
 */
#ifndef mipos_tm_usleep
#define mipos_tm_usleep(_COUNT) _mipos_tm_rtc_quantum_sleep(SIGTMR, _COUNT)
#endif


/* ------------------------------------------------------------------------- */

/** This function will cause the calling task to be
 *  suspended from execution until the number of real-time
 *  milliseconds specified by the argument _COUNT has
 *  elapsed.
 *  The suspension time may be longer than requested due to
 *  the scheduling of other activity by the system and hw
 *  timer precision.
 *
 *  Apply to: NON-RT TASK
 *
 *
 *  @param _COUNT : the number of milliseconds, must be less
 *                  than 4294967.
 *                  If its value is 0, then the call has no
 *                  effect.
 */
#ifndef mipos_tm_msleep
#define mipos_tm_msleep(_COUNT) _mipos_tm_rtc_quantum_sleep(SIGTMR, _COUNT)
#endif


/* ------------------------------------------------------------------------- */

/** Get the RTC counter in microseconds
 *
 *  Apply to: NON-RT TASK, RT-TASK
 */
void mipos_get_rtc_counter(mipos_rtc_cntr_t* cnt);


/* ------------------------------------------------------------------------- */

// Reserved for internal use only
#define _mipos_t_reset_signal_and_run(_SIGNUM)                                 \
    do {                                                                       \
        mipos_kernel_env.task_context_ptr->status = TASK_RUNNING;              \
        mipos_kernel_env.task_context_ptr->signal_waiting &=                   \
          (mipos_task_sigmask_t) ~(_SIGNUM);                                   \
        mipos_kernel_env.task_context_ptr->signal_pending &=                   \
          (mipos_task_sigmask_t) ~(_SIGNUM);                                   \
    } while (0)


/* ------------------------------------------------------------------------- */

// Reserved for internal use only
#define _mipos_t_set_wait_for_signal(_SIGNUM)                                  \
    do {                                                                       \
        mipos_kernel_env.task_context_ptr->signal_waiting |= (_SIGNUM);        \
        if (mipos_kernel_env.task_context_ptr->status &                        \
            (TASK_READY | TASK_RUNNING | TASK_SUSPENDED)) {                    \
            mipos_kernel_env.task_context_ptr->status = TASK_SUSPENDED;        \
        }                                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

// Reserved for internal use only
#define _mipos_t_suspend(_SIGNUM)                                              \
    do {                                                                       \
        if (mipos_save_context(                                                \
              mipos_kernel_env.task_context_ptr->reg_state)) {                 \
            _mipos_t_reset_signal_and_run(_SIGNUM);                            \
        } else {                                                               \
            mipos_kernel_env.task_context_ptr->process_stack_pointer =         \
              (mipos_reg_t)mipos_get_sp();                                     \
            _mipos_t_set_wait_for_signal(_SIGNUM);                             \
            mipos_context_switch_to(                                           \
              mipos_kernel_env.scheduler_registers_state);                     \
        }                                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

#define _mipos_tm_wkafter(_SIGNUM, _COUNT)                                     \
    do {                                                                       \
        if (mipos_save_context(                                                \
              mipos_kernel_env.task_context_ptr->reg_state)) {                 \
            _mipos_t_reset_signal_and_run(_SIGNUM);                            \
        } else {                                                               \
            mipos_kernel_env.task_context_ptr->process_stack_pointer =         \
              (mipos_reg_t)mipos_get_sp();                                     \
            _mipos_t_set_wait_for_signal(_SIGNUM);                             \
            mipos_kernel_env.task_context_ptr->timer_tick_count =              \
              (uint32_t)(_COUNT);                                              \
            mipos_context_switch_to(                                           \
              mipos_kernel_env.scheduler_registers_state);                     \
        }                                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

// Reserved for internal use only
#define _mipos_tm_rtc_quantum_sleep(_SIGNUM, _COUNT)                           \
    do {                                                                       \
        if (mipos_save_context(                                                \
              mipos_kernel_env.task_context_ptr->reg_state)) {                 \
            _mipos_t_reset_signal_and_run(_SIGNUM);                            \
        } else {                                                               \
            mipos_init_cs();                                                   \
            _mipos_t_set_wait_for_signal(_SIGNUM);                             \
            mipos_enter_cs();                                                  \
            mipos_kernel_env.task_context_ptr->rtc_timeout =                   \
              mipos_kernel_env.rtc_counter + (uint32_t)(_COUNT);               \
            mipos_leave_cs();                                                  \
            mipos_kernel_env.task_context_ptr->process_stack_pointer =         \
              (mipos_reg_t)mipos_get_sp();                                     \
            mipos_context_switch_to(                                           \
              mipos_kernel_env.scheduler_registers_state);                     \
        }                                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function notifies a signal to a task.
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *    @param id - the task identifier
 *    @param signum - the signal(s) to notify
 *
 */
#define mipos_t_notify_signal(_TID, _SIGNUM)                                   \
    do {                                                                       \
        mipos_task_t* p_task;                                                  \
        mipos_init_cs();                                                       \
        mipos_enter_cs();                                                      \
        p_task = _mipos_get_p_task(_TID);                                      \
        if (p_task) {                                                          \
            p_task->signal_pending |= (_SIGNUM)&p_task->signal_mask;           \
        }                                                                      \
        mipos_leave_cs();                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function freezes a task
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *  @param id - the task identifier
 */
#define mipos_t_freeze(_TID)                                                   \
    do {                                                                       \
        mipos_task_t* p_task;                                                  \
        mipos_init_cs();                                                       \
        mipos_enter_cs();                                                      \
        p_task = _mipos_get_p_task(_TID);                                      \
        if (p_task) {                                                          \
            p_task->status |= TASK_FROZEN;                                     \
        }                                                                      \
        mipos_leave_cs();                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function unfreezes a task
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *   @param id - the task identifier
 */
#define mipos_t_unfreeze(_TID)                                                 \
    do {                                                                       \
        mipos_task_t* p_task;                                                  \
        mipos_init_cs();                                                       \
        mipos_enter_cs();                                                      \
        p_task = _mipos_get_p_task(_TID);                                      \
        if (p_task) {                                                          \
            p_task->status &= ~TASK_FROZEN;                                    \
        }                                                                      \
        mipos_leave_cs();                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function disables a signal for a task
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *  @param id - the task identifier
 *  @param signum - the signal(s) to disable
 */
#define mipos_t_sig_ignore(_TID, _SIGNUM)                                      \
    do {                                                                       \
        mipos_init_cs();                                                       \
        mipos_enter_cs();                                                      \
        mipos_task_t* p_task = _mipos_get_p_task(_TID);                        \
        if (p_task) {                                                          \
            p_task->signal_mask &= ~((_SIGNUM) & (~SYS_RESERVED_SIGNALS));     \
        }                                                                      \
        mipos_leave_cs();                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function enables a signal for a task
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *  @param id - the task identifier
 *  @param signum - the signal(s) to enable
 */
#define mipos_t_sig_allow(_TID, _SIGNUM)                                       \
    do {                                                                       \
        mipos_init_cs();                                                       \
        mipos_enter_cs();                                                      \
        mipos_task_t* p_task = _mipos_get_p_task(_TID);                        \
        if (p_task) {                                                          \
            p_task->signal_mask |= (_SIGNUM) & (~SYS_RESERVED_SIGNALS);        \
        }                                                                      \
        mipos_leave_cs();                                                      \
    } while (0)


/* ------------------------------------------------------------------------- */

/** This function register a callback function
 *  called if mipOS goes in idle state.
 *  Note that the idle state means there
 *  are no tasks to process:
 *  all tasks have completed their own execution
 *
 *  Apply to: NON-RT TASK, RT-TASK
 *
 *  @param idle_state_notify_cbk: user callback function
 *  @param param: parameter passed to the callback function
 */
void mipos_set_idle_cbk(idle_state_notify_cbk_t idle_state_notify_cbk,
                        task_param_t param);


/* ------------------------------------------------------------------------- */

// Kernel state is saved there
extern mipos_kernel_t mipos_kernel_env;


/* ------------------------------------------------------------------------- */

/** Starts the mipOS kernel
 * @param root_task: entry point of root task
 * @param root_task_param: context param for root task
 * @param root_task_prio: priority of root task
 * @param task_stack: pointer to a memory area used by root task as stack
 * @param task_size: size of root task stack
 * @param isr_stack: pointer to a memory area used by ISR as stack
 * @param isr_task_size: size of ISR task stack
 */
void mipos_start(task_entry_point_t root_task,
                 task_param_t root_task_param,
                 mipos_task_stack_ptr_t task_stack,
                 mipos_task_stack_size_t stack_size);


/* ------------------------------------------------------------------------- */

#endif //!__KERNEL_H__


/* ------------------------------------------------------------------------- */
