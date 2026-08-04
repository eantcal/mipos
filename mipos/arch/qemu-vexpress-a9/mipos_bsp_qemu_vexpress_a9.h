/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

#ifndef MIPOS_BSP_QEMU_VEXPRESS_A9_H__
#define MIPOS_BSP_QEMU_VEXPRESS_A9_H__

#if !defined(__GNUC__) || !defined(__arm__)
#error "The QEMU vexpress-a9 port requires an arm-none-eabi GCC toolchain"
#endif

#include "mipos_types.h"

#include <setjmp.h>
#include <stdint.h>

/* ------------------------------------------------------------------------- */

typedef uintptr_t mipos_ptr_t;

#define kick_watchdog()
#define mipos_kick_watchdog()

#define mipos_bps_init_sp()
#define mipos_bsp_setup_reset_and_wd()                                         \
    do {                                                                       \
    } while (0)
#define mipos_bsp_exception_vector()                                           \
    do {                                                                       \
    } while (0)
#define mipos_bsp_check_reset_type() (0)

void mipos_bsp_setup_clk(void);
void mipos_bsp_create_hw_rtc_timer(void);
void mipos_bsp_qemu_vexpress_a9_epoch(void);
void mipos_bsp_qemu_vexpress_a9_irq(void);

#define mipos_bsp_notify_scheduler_epoch() mipos_bsp_qemu_vexpress_a9_epoch()

static inline unsigned int _mipos_qemu_vexpress_a9_irq_save(void)
{
    unsigned int cpsr;
    __asm__ __volatile__("mrs %0, cpsr\n\t"
                         "cpsid i\n\t"
                         : "=r"(cpsr)
                         :
                         : "memory");
    return cpsr;
}

static inline void _mipos_qemu_vexpress_a9_irq_restore(unsigned int cpsr)
{
    __asm__ __volatile__("msr cpsr_c, %0" : : "r"(cpsr) : "memory");
}

#define mipos_bsp_enable_irq_mask()                                            \
    __asm__ __volatile__("cpsie i" ::: "memory")

#define mipos_init_cs() unsigned int _mipos_cs_cpsr = 0
#define mipos_enter_cs()                                                       \
    do {                                                                       \
        _mipos_cs_cpsr = _mipos_qemu_vexpress_a9_irq_save();                   \
    } while (0)
#define mipos_leave_cs()                                                       \
    do {                                                                       \
        _mipos_qemu_vexpress_a9_irq_restore(_mipos_cs_cpsr);                   \
    } while (0)

#define mipos_save_context(_x) setjmp((void*)(_x))
#define mipos_context_switch_to(_x) longjmp((void*)(_x), 1)

static inline mipos_reg_t mipos_get_sp(void)
{
    mipos_reg_t sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp));
    return sp;
}

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

#define mipos_set_sp(__OLD_SP)                                                 \
    do {                                                                       \
        uint32_t __mipos_tmp;                                                  \
        __asm__ __volatile__("ldr %0, [%1]\n\t"                                \
                             "mov sp, %0\n\t"                                  \
                             : "=&r"(__mipos_tmp)                              \
                             : "r"(__OLD_SP)                                   \
                             : "memory");                                      \
    } while (0)

#define STACK_UNIT 1
#define MIPOS_LE_U16(x) (x)
#define MIPOS_LE_U32(x) (x)

#ifndef assert
#define assert(_C) ((void)(_C))
#endif

void mipos_printf(const char* fmt, ...);

#endif /* MIPOS_BSP_QEMU_VEXPRESS_A9_H__ */
