/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifndef mipos_TYPES_H__
#define mipos_TYPES_H__

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define MIPOS64
#else
#define MIPOS32
#endif
#endif

/* ------------------------------------------------------------------------- */

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define MIPOS64
#else
#define MIPOS32
#endif
#endif

/* ------------------------------------------------------------------------- */

#include <stdint.h>

#include "mipos_limits.h"

/* ------------------------------------------------------------------------- */

#ifdef CTXM3_SETJMP
#undef jmp_buf
#define jmp_buf _ctxm3_jmp_buf
#else
#include <setjmp.h>
#endif

/* ------------------------------------------------------------------------- */

#ifndef __STM8L10X_STDPERIPH_VERSION_MAIN

//! Boolean definitions
typedef char bool_t;

#if !defined(__ICCARM__)
#define FALSE 0
#define TRUE 1
#endif

#endif //__STM8L10X_STDPERIPH_VERSION_MAIN

/* ------------------------------------------------------------------------- */

#define bool_t_cast(_X) ((_X) ? TRUE : FALSE)

/* ------------------------------------------------------------------------- */

/** Registers state */
typedef jmp_buf registers_state_t;

/* ------------------------------------------------------------------------- */

/** TASK status */
typedef uint16_t task_status_t;

/* ------------------------------------------------------------------------- */

/** TASK parameter */
typedef void* task_param_t;

/* ------------------------------------------------------------------------- */

/** TASK flag mask */
typedef unsigned char task_flag_mask_t;

/* ------------------------------------------------------------------------- */

/** TASK identifier */
typedef unsigned int mipos_task_id_t;

/* ------------------------------------------------------------------------- */

/** TASK flag */
typedef struct __task_flag
{
    task_flag_mask_t wkup : 1;    /* used to wkup a task */
    task_flag_mask_t rt_task : 1; /* set for RT-task */
    task_flag_mask_t mbz : 6;     /* reserved, must be zero */
} task_flag_t;

/* ------------------------------------------------------------------------- */

/** TASK creation flags */
typedef enum _t_create_flags_t
{
    MIPOS_NO_RT = 0,
    MIPOS_RT = 1
} t_create_flags_t;

/* ------------------------------------------------------------------------- */

/** TASK descriptor field types */

#ifdef MIPOS64
typedef unsigned long long mipos_reg_t;
#else
typedef unsigned int mipos_reg_t;
#endif
typedef const char* mipos_task_name_t;
typedef mipos_reg_t mipos_task_stack_size_t;
typedef char* mipos_task_stack_ptr_t;

typedef unsigned char mipos_task_sigmask_t;
typedef unsigned char mipos_task_sig_t;
typedef int mipos_task_tickcntr_t;

/* ------------------------------------------------------------------------- */

#ifdef __RCSTM8__
#define MIPOS_LONG_LONG_NOT_SUPPORTED
typedef unsigned int u16;
#endif

/* ------------------------------------------------------------------------- */

#ifdef MIPOS_LONG_LONG_NOT_SUPPORTED
typedef unsigned long mipos_rtc_cntr_t;
#else
typedef unsigned long long mipos_rtc_cntr_t;
#endif

/* ------------------------------------------------------------------------- */

typedef int (*task_entry_point_t)(task_param_t);
typedef void (*idle_state_notify_cbk_t)(task_param_t);

/* ------------------------------------------------------------------------- */

typedef struct __task_t
{
    mipos_task_name_t name;         //!< task name
    task_entry_point_t entry_point; //!< task entry point
    int task_ret_value;             //!< retval for terminating task
    task_param_t param;             //!< user's context cookie

    task_status_t status; //!< task status: READY, RUNNING, ...

    union
    { //!< special purpose flags
        task_flag_t flags;
        task_flag_mask_t i_flags;
    } u_flags;

    /** used by mipos_tm_wkafter syscall */
    mipos_task_tickcntr_t timer_tick_count;

    /** used by mipos_tm_usleep syscall */
    mipos_rtc_cntr_t rtc_timeout;

    /** bit oriented signal mask */
    mipos_task_sigmask_t signal_mask;

    /** pending signals */
    mipos_task_sigmask_t signal_pending;

    /** waiting signals */
    mipos_task_sigmask_t signal_waiting;

    /** cpu regs status */
    registers_state_t reg_state;
    mipos_reg_t kernel_stack_pointer;
    mipos_reg_t process_stack_pointer;

    char* task_stack;
    mipos_task_stack_size_t stack_size;
} mipos_task_t;

/* ------------------------------------------------------------------------- */

#define TIMER_FLG_PERIODIC 1
#define TIMER_FLG_EXPIRED 2
#define TIMER_FLG_CANCELLED 4

/* ------------------------------------------------------------------------- */

typedef void* mipos_timer_ctx_t;
typedef void (*mipos_timer_handler_t)(mipos_timer_ctx_t);
typedef int timer_id_t;
#define TIMER_INVALID_ID (-1)
typedef unsigned long mipos_timer_intvl_t;

/* ------------------------------------------------------------------------- */

typedef struct __timer_t
{
    mipos_timer_handler_t handler_on_expire_cbk; //!< on-expire callback
    mipos_timer_handler_t handler_on_cancel_cbk; //!< on-cancel callback

    /** user's context cookie */
    mipos_timer_ctx_t context;

    /** timeout */
    mipos_rtc_cntr_t rtc_timeout;

    /** timer flags */
    mipos_reg_t flgs;

    /** timeout interval */
    mipos_timer_intvl_t interval;
} mipos_timer_t;

/* ------------------------------------------------------------------------- */

typedef unsigned int mipos_timer_bmp_t;

/* ------------------------------------------------------------------------- */

typedef struct __kernel_t
{
    //! Tick counter
    unsigned long tick_counter;

    //! Idle state reference
    idle_state_notify_cbk_t idle_state_notify_cbk;
    task_param_t idle_param;

    //! RTC counter
    mipos_rtc_cntr_t rtc_counter;

    //! references to the current task
    mipos_task_id_t current_task;
    mipos_task_t* task_context_ptr;

    int task_count;

    registers_state_t scheduler_registers_state;

    //! Task descriptors table
    mipos_task_t task_table[MIPOS_TASKS_TBL_ENTRIES_CNT];

    //! Timers table
    mipos_rtc_cntr_t next_rtc_timeout;
    mipos_timer_bmp_t timer_bitmap;
    mipos_timer_t timer_table[MIPOS_MAX_TIMERS_NO];

} mipos_kernel_t;

/* ------------------------------------------------------------------------- */

#endif // mipos_TYPES_H__

/* ------------------------------------------------------------------------- */
