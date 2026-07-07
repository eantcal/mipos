/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifndef MIPOS_BSP_SIMU_H__
#define MIPOS_BSP_SIMU_H__

#include "mipos_types.h"

#include <ctype.h>
#include <setjmp.h>
#include <stdint.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

/* ------------------------------------------------------------------------- */

#define kick_watchdog()
#define mipos_bps_init_sp()
#define mipos_bsp_enable_irq_mask()

/* ------------------------------------------------------------------------- */

#define mipos_bsp_setup_clk()                                                  \
    do {                                                                       \
    } while (0)
#define mipos_bsp_setup_reset_and_wd()                                         \
    do {                                                                       \
    } while (0)
#define mipos_bsp_exception_vector()                                           \
    do {                                                                       \
    } while (0)
#define mipos_bsp_create_hw_onems_timer()                                      \
    do {                                                                       \
    } while (0)
#define mipos_bsp_check_reset_type() (0)

/* ------------------------------------------------------------------------- */

/* Platform independent type definitions */
typedef uintptr_t
  mipos_ptr_t; // Universal pointer type compatible with the target architecture

/* ------------------------------------------------------------------------- */

/* Used only for debugging purpose on Windows and Linux platforms */
#if defined(_MSC_VER) || defined(__BORLANDC__)
#if defined(_WIN32)
#include <windows.h>
#define KERNEL_DBG_DELAY Sleep(1)
#endif
#pragma check_stack(off)
#pragma optimize("agpswy", on)

#else // LINUX or macOS

#define KERNEL_DBG_DELAY usleep(100)

#endif

/* ------------------------------------------------------------------------- */

// Section for INTEL x86/x86-64 supporting Borland C++,
// MS-Visual Studio C++ (Windows),
// GCC (Windows/Linux/macOS) compilers
#if defined(_MSC_VER) || defined(__BORLANDC__)

#pragma warning(disable : 4312)
#pragma warning(disable : 4313)

#if defined(_M_X64) || defined(__x86_64__)
extern mipos_reg_t mipos_get_sp();
extern int mipos_save_context64(void* buffer);
extern void mipos_context_switch64(void* buffer, int value);
extern int mipos_start_task64(void** old_sp,
                              void* new_sp,
                              task_entry_point_t entry_point,
                              task_param_t param);
#define mipos_save_context(_x) mipos_save_context64((unsigned int*)_x)
#define mipos_context_switch_to(_x) mipos_context_switch64((unsigned int*)_x, 1)
#define mipos_start_task_on_stack(_OLD_SP, _STACK_P, _ENTRY, _PARAM)           \
    mipos_start_task64((_OLD_SP), (_STACK_P), (_ENTRY), (_PARAM))
#else
#define mipos_replace_sp(__OLD_SP, __STACK_P)                                  \
    __asm { mov ebx, [__OLD_SP] }                                                 \
    __asm                                                                      \
    {                                                                          \
        mov eax, esp                                                           \
    }                                                                          \
    __asm { mov [ebx], eax }                                                      \
    __asm                                                                      \
    {                                                                          \
        mov eax, __STACK_P                                                     \
    }                                                                          \
    __asm { mov esp, eax }

#define mipos_set_sp(__OLD_SP)                                                 \
    __asm { mov eax, __OLD_SP }                                                   \
    __asm                                                                      \
    {                                                                          \
        mov esp, [eax]                                                         \
    }

extern mipos_reg_t mipos_get_sp();
#define mipos_save_context(_x) setjmp((unsigned int*)_x)
#define mipos_context_switch_to(_x) longjmp((unsigned int*)_x, 1)
#endif

#elif defined(__GNUC__)

#if defined(__x86_64__) || defined(__aarch64__)
extern mipos_reg_t mipos_get_sp();
extern int mipos_start_task_gnu(void** old_sp,
                                void* new_sp,
                                task_entry_point_t entry_point,
                                task_param_t param);
#define mipos_save_context(_x) setjmp(_x)
#define mipos_context_switch_to(_x) longjmp((_x), 1)
#define mipos_start_task_on_stack(_OLD_SP, _STACK_P, _ENTRY, _PARAM)           \
    mipos_start_task_gnu((_OLD_SP), (_STACK_P), (_ENTRY), (_PARAM))

#elif defined(__i386__) || defined(__i386)
extern mipos_reg_t mipos_get_sp();
#define mipos_replace_sp(__OLD_SP, __STACK_P)                                  \
    do {                                                                       \
        void** __old_sp_ptr = (void**)(__OLD_SP);                              \
        void* __new_sp = (__STACK_P);                                          \
        __asm__ __volatile__("movl %%esp, (%0)\n\t"                            \
                             "movl %1, %%esp\n\t"                             \
                             :                                                 \
                             : "r"(__old_sp_ptr), "r"(__new_sp)               \
                             : "memory");                                      \
    } while (0)

#define mipos_set_sp(__OLD_SP)                                                 \
    do {                                                                       \
        void* __old_sp = *(void**)(__OLD_SP);                                  \
        __asm__ __volatile__("movl %0, %%esp\n\t"                              \
                             :                                                 \
                             : "r"(__old_sp)                                  \
                             : "memory");                                      \
    } while (0)

#define mipos_save_context(_x) setjmp(_x)
#define mipos_context_switch_to(_x) longjmp((_x), 1)
#else
#error "GNU simulator target supports only x86, x64 and AArch64"
#endif

#else
#error "Platform or compiler not supported"
#endif

#define mipos_init_cs()
#define mipos_enter_cs()
#define mipos_leave_cs()

#define assert(_C)                                                             \
    if (_C)                                                                    \
        ;

/* ------------------------------------------------------------------------- */

#define STACK_UNIT 4096

#if defined(_WIN32)
#define mipos_bsp_notify_scheduler_epoch()                                     \
    do {                                                                       \
        static unsigned long long old_tc = 0;                                  \
        unsigned long long tc = GetTickCount64();                              \
        if (old_tc)                                                            \
            mipos_update_rtc((uint32_t)(tc - old_tc));                         \
        old_tc = tc;                                                           \
    } while (0)

#elif defined(__APPLE__)
#define mipos_bsp_notify_scheduler_epoch()                                     \
    do {                                                                       \
        static uint64_t old_tc = 0;                                            \
        uint64_t tc = mach_absolute_time();                                    \
        mach_timebase_info_data_t info;                                        \
        mach_timebase_info(&info);                                             \
        tc *= info.numer;                                                      \
        tc /= info.denom;                                                      \
        if (old_tc)                                                            \
            mipos_update_rtc((uint32_t)((tc - old_tc) / 1000000));             \
        old_tc = tc;                                                           \
    } while (0)

#else
#include <sys/time.h>
#define mipos_bsp_notify_scheduler_epoch()                                     \
    do {                                                                       \
        struct timeval tv;                                                     \
        static unsigned long long old_tc = 0;                                  \
        gettimeofday(&tv, 0);                                                  \
        unsigned long long tc = ((unsigned long long)((tv.tv_sec * 1000ul) +   \
                                                      (tv.tv_usec / 1000ul))); \
        if (old_tc)                                                            \
            mipos_update_rtc((uint32_t)(tc - old_tc));                         \
        old_tc = tc;                                                           \
    } while (0)
#endif

#define mipos_bsp_create_hw_rtc_timer()
#define mipos_kick_watchdog()

#define MIPOS_CONSOLE_TX_STACK (1024 * 1024)
#define MIPOS_CONSOLE_RX_STACK (1024 * 1024)

#define MIPOS_LE_U16(x) (x)
#define MIPOS_LE_U32(x) (x)

#define mipos_printf printf

#endif
