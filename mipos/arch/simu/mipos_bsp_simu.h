/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifndef mipos_BSP_SIMU_H__
#define mipos_BSP_SIMU_H__

#include "mipos_types.h"

/* ------------------------------------------------------------------------- */

#include <intrin.h>
#include <setjmp.h>
#include <stdint.h>

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
typedef uintptr_t mipos_ptr_t; // Universal pointer type compatible with the
// target architecture

/* ------------------------------------------------------------------------- */

/* Used only for debugging purpose on Windows and Linux platforms */
#if (defined(_MSC_VER) || defined(__BORLANDC__))
#if (defined(_WIN32))
#include <windows.h>
#define KERNEL_DBG_DELAY Sleep(1)
#endif
#pragma check_stack(off)
#pragma optimize("agpswy", on)

#else // LINUX

#include <unistd.h>
#define KERNEL_DBG_DELAY usleep(100)
#endif

/* ------------------------------------------------------------------------- */

// Section for INTEL x86/x86-64 supporting Borland C++,
// MS-Visual Studio C++ (Windows),
// GCC (Windows/Linux) compilers
#if (defined(_MSC_VER) || defined(__BORLANDC__))

#pragma warning(disable : 4312)
#pragma warning(disable : 4313)

#if defined(_M_X64) || defined(__x86_64__)
extern void mipos_replace_sp(void** old_sp, void* new_sp);
extern void mipos_set_sp(void* new_sp);
extern mipos_reg_t mipos_get_sp();
extern int set_context(void* buffer);
extern void jump_context(void* buffer, int value);
#define mipos_save_context(_x) set_context((unsigned int*)_x)
#define mipos_context_switch_to(_x) jump_context((unsigned int*)_x, 1)
#else
#define mipos_replace_sp(__OLD_SP, __STACK_P)                                  \
    __asm { mov ebx, [__OLD_SP] }                                              \
    __asm                                                                      \
    {                                                                          \
        mov eax, esp                                                           \
    }                                                                          \
    __asm { mov [ebx], eax }                                                   \
    __asm                                                                      \
    {                                                                          \
        mov eax, __STACK_P                                                     \
    }                                                                          \
    __asm { mov esp, eax }

#define mipos_set_sp(__OLD_SP)                                                 \
    __asm { mov eax, __OLD_SP }                                                \
    __asm                                                                      \
    {                                                                          \
        mov esp, [eax]                                                         \
    }

extern mipos_reg_t mipos_get_sp();
#define mipos_save_context(_x) setjmp((unsigned int*)_x)
#define mipos_context_switch_to(_x) longjmp((unsigned int*)_x, 1)
#endif

#elif defined(__GNUC__)
#if defined(__x86_64__)
#define mipos_replace_sp(__OLD_SP, __STACK_P)                                  \
    __asm__ __volatile__("movq " #__OLD_SP ", %rbx\n\t"                        \
                         "movq %rsp, %rax\n\t"                                 \
                         "movq %rax, (%rbx)\n\t"                               \
                         "movq " #__STACK_P ", %rax\n\t"                       \
                         "movq %rax, %rsp\n\t")

#define mipos_set_sp(__OLD_SP)                                                 \
    __asm__ __volatile__("movq " #__OLD_SP ", %rax\n\t"                        \
                         "movq %rax, %rsp\n\t")
#else
#define mipos_replace_sp(__OLD_SP, __STACK_P)                                  \
    __asm__ __volatile__("movl " #__OLD_SP ", %ebx\n\t"                        \
                         "movl %esp, %eax\n\t"                                 \
                         "movl %eax, (%ebx)\n\t"                               \
                         "movl " #__STACK_P ", %eax\n\t"                       \
                         "movl %eax, %esp\n\t")

#define mipos_set_sp(__OLD_SP)                                                 \
    __asm__ __volatile__("movl " #__OLD_SP ", %eax\n\t"                        \
                         "movl %eax, %esp\n\t")
#endif

extern unsigned int mipos_get_sp();

#else
#error "Platform or compiler not supported"
#endif

#define inline

#define mipos_init_cs()
#define mipos_enter_cs()
#define mipos_leave_cs()

#define assert(_C)                                                             \
    if (_C)                                                                    \
        ;

#define STACK_UNIT 4096

#if (defined(_WIN32))
#define mipos_bsp_notify_scheduler_epoch()                                     \
    do {                                                                       \
        static unsigned long long old_tc = 0;                                  \
        unsigned long long tc = GetTickCount64();                              \
        if (old_tc)                                                            \
            mipos_update_rtc((uint32_t)(tc - old_tc));                         \
        old_tc = tc;                                                           \
                                                                               \
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

#define CONSOLE_SENDER_STACK (1024 * 1024)
#define CONSOLE_RECEIVER_STACK (1024 * 1024)

#define MIPOS_LE_U16(x) (x)
#define MIPOS_LE_U32(x) (x)

#define mipos_printf printf

#endif // mipos_BSP_SIMU_H__
