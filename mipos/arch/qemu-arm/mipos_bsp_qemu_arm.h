/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

// BSP for the bare-metal ARM Cortex-M port running under QEMU
// (board: lm3s6965evb, TI Stellaris LM3S6965, Cortex-M3).
//
// The port is cooperative and single-privileged: the kernel, the tasks and
// the exception handlers all run on the main stack (MSP). Context switching
// reuses the newlib setjmp/longjmp pair (which saves/restores r4-r11, sp and
// lr), exactly like the historical IAR Cortex-M3 port did with _ctxm3_setjmp.

#ifndef MIPOS_BSP_QEMU_ARM_H__
#define MIPOS_BSP_QEMU_ARM_H__

#if !defined(__GNUC__) || !defined(__arm__)
#error "The QEMU ARM port requires an arm-none-eabi GCC toolchain"
#endif

#include "mipos_types.h"

#include <setjmp.h>
#include <stdint.h>

/* ------------------------------------------------------------------------- */

/* Platform independent type definitions */
typedef uintptr_t mipos_ptr_t;

/* ------------------------------------------------------------------------- */

/* No watchdog on the emulated board */
#define kick_watchdog()
#define mipos_kick_watchdog()

/* The reset/startup code already configured the stack pointer */
#define mipos_bps_init_sp()

/* Reset configuration is not needed under QEMU */
#define mipos_bsp_setup_reset_and_wd()                                         \
    do {                                                                       \
    } while (0)
#define mipos_bsp_exception_vector()                                           \
    do {                                                                       \
    } while (0)
#define mipos_bsp_check_reset_type() (0)

//! Switch the system clock from the reset bypass oscillator to the PLL
//! (200 MHz / 16 = 12.5 MHz), so the SysTick reload value below is exact.
void mipos_bsp_setup_clk(void);

/* ------------------------------------------------------------------------- */

//! Configure SysTick to generate the 1 ms mipOS RTC quantum
void mipos_bsp_create_hw_rtc_timer(void);

//! Transfer the SysTick ticks accumulated by the ISR into the kernel RTC.
//! Called by the scheduler on every epoch (see
//! mipos_bsp_notify_scheduler_epoch below).
void mipos_bsp_qemu_arm_rtc_epoch(void);

#define mipos_bsp_notify_scheduler_epoch() mipos_bsp_qemu_arm_rtc_epoch()

/* ------------------------------------------------------------------------- */

#define mipos_bsp_enable_irq_mask()                                            \
    __asm__ __volatile__("cpsie i" ::: "memory")

/* ------------------------------------------------------------------------- */

/* Critical sections: save PRIMASK, disable interrupts, restore on leave.
 * Saving/restoring the mask keeps nested critical sections safe. */

static inline unsigned int _mipos_qemu_arm_irq_save(void)
{
    unsigned int primask;
    __asm__ __volatile__("mrs %0, primask\n\t"
                         "cpsid i\n\t"
                         : "=r"(primask)
                         :
                         : "memory");
    return primask;
}

static inline void _mipos_qemu_arm_irq_restore(unsigned int primask)
{
    __asm__ __volatile__("msr primask, %0" : : "r"(primask) : "memory");
}

#define mipos_init_cs() unsigned int _mipos_cs_primask = 0
#define mipos_enter_cs()                                                       \
    do {                                                                       \
        _mipos_cs_primask = _mipos_qemu_arm_irq_save();                        \
    } while (0)
#define mipos_leave_cs()                                                       \
    do {                                                                       \
        _mipos_qemu_arm_irq_restore(_mipos_cs_primask);                        \
    } while (0)

/* ------------------------------------------------------------------------- */

/* Context switching: newlib setjmp/longjmp save and restore the callee-saved
 * registers plus sp and lr, which is all a cooperative switch needs on
 * Cortex-M (no FPU on Cortex-M3). */

#define mipos_save_context(_x) setjmp((void*)(_x))
#define mipos_context_switch_to(_x) longjmp((void*)(_x), 1)

/* ------------------------------------------------------------------------- */

static inline mipos_reg_t mipos_get_sp(void)
{
    mipos_reg_t sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp));
    return sp;
}

/* Store the current sp into *__OLD_SP and switch to __STACK_P (rounded down
 * to the AAPCS-required 8-byte alignment). A single asm block is used so the
 * compiler cannot touch the stack in between. */
#define mipos_replace_sp(__OLD_SP, __STACK_P)                                  \
    do {                                                                       \
        uint32_t __mipos_tmp;                                                  \
        __asm__ __volatile__("mov %0, sp\n\t"                                  \
                             "str %0, [%1]\n\t"                                \
                             "bic %0, %2, #7\n\t"                              \
                             "mov sp, %0\n\t"                                  \
                             : "=&r"(__mipos_tmp)                              \
                             : "r"(__OLD_SP), "r"(__STACK_P)                   \
                             : "memory");                                      \
    } while (0)

/* Reload sp from *__OLD_SP */
#define mipos_set_sp(__OLD_SP)                                                 \
    do {                                                                       \
        uint32_t __mipos_tmp;                                                  \
        __asm__ __volatile__("ldr %0, [%1]\n\t"                                \
                             "mov sp, %0\n\t"                                  \
                             : "=&r"(__mipos_tmp)                              \
                             : "r"(__OLD_SP)                                   \
                             : "memory");                                      \
    } while (0)

/* ------------------------------------------------------------------------- */

#define STACK_UNIT 1

#define MIPOS_LE_U16(x) (x)
#define MIPOS_LE_U32(x) (x)

/* Stacks of the console TX/RX tasks (the console commands use sprintf,
 * which needs some headroom) */
#ifndef MIPOS_CONSOLE_TX_STACK
#define MIPOS_CONSOLE_TX_STACK 4096
#endif
#ifndef MIPOS_CONSOLE_RX_STACK
#define MIPOS_CONSOLE_RX_STACK 4096
#endif

/* ------------------------------------------------------------------------- */

/* Minimal printf used by the kernel panic path: writes the format string
 * verbatim to the UART (no formatting support in this port). */
void mipos_printf(const char* fmt, ...);

/* The full newlib assert would drag in printf/abort and the syscall stubs;
 * evaluate the condition and discard it instead. */
#ifndef assert
#define assert(_C) ((void)(_C))
#endif

/* ------------------------------------------------------------------------- */

#endif // MIPOS_BSP_QEMU_ARM_H__
