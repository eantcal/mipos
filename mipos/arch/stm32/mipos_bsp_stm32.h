/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* -------------------------------------------------------------------------- */

// BSP for STM32 IAR Compiler

#ifndef mipos_BSP_STM32_H__
#define mipos_BSP_STM32_H__


/* -------------------------------------------------------------------------- */

#define CONSOLE_UART2


/* -------------------------------------------------------------------------- */

#include "arm_comm.h"
#include "mipos_ctxm3.h"
#include "setjmp.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_lib.h"
#include <intrinsics.h>


/* -------------------------------------------------------------------------- */

/* returns 1-watchdog, 0-power up */
int mipos_bsp_check_reset_type(void);


/* -------------------------------------------------------------------------- */

#define mipos_bsp_dbg_device_setup()                                           \
    do {                                                                       \
        *((u32*)(0xE0042004)) = 0x4500;                                        \
    } while (0)

#define mipos_tm_msleep(_COUNT) _mipos_tm_rtc_quantum_sleep(MIPOS_SIGTMR, _COUNT)


/* -------------------------------------------------------------------------- */

#ifdef CONSOLE_UART2
#define _USART2
#define UART USART2
#else
#define _USART1
#define UART USART1
#endif
#include "includes.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"


/* -------------------------------------------------------------------------- */

#ifdef CTXM3_SETJMP
#undef setjmp
#undef longjmp
#define setjmp _ctxm3_setjmp
#define longjmp _ctxm3_longjmp
#else
#include <setjmp.h>
#endif //...CTXM3_SETJMP


/* -------------------------------------------------------------------------- */

//! 1ms O/S tick generated by hw timer
void mipos_bsp_create_hw_rtc_timer(void);

//! Configure reset and watch-dog registers
void mipos_bsp_setup_reset_and_wd(void);

//! Configure exception table
void mipos_bsp_exception_vector(void);

//! Configure CLKs
void mipos_bsp_setup_clk(void);

//! Return reset type
int mipos_bsp_check_reset_type(void);


/* -------------------------------------------------------------------------- */

#define STACK_UNIT 1
#define mipos_bsp_notify_scheduler_epoch() /*UNUSED*/
#define mipos_bsp_enable_irq_mask() NVIC_RESETPRIMASK()
#define mipos_kick_watchdog() IWDG_ReloadCounter()
#define MIPOS_IGNORE_TASK_PRIORITY
#define MIPOS_LONG_LONG_NOT_SUPPORTED
#define mipos_save_context(_x) setjmp((u32*)_x)
#define mipos_context_switch_to(_x) longjmp((u32*)_x, 1)
#define mipos_bsp_create_hw_rtc_timer()


/* -------------------------------------------------------------------------- */

#ifdef ENABLE_WATCHDOG
#define kick_watchdog()                                                        \
    do {                                                                       \
        vrcs_blink_wd_led();                                                   \
    } while (0)
#else
#define kick_watchdog()
#endif


/* -------------------------------------------------------------------------- */

#ifndef MIPOS_ISR_STACK_SIZE
#define MIPOS_ISR_STACK_SIZE 384
#endif


#define MIPOS_LE_U16(x) (x)
#define MIPOS_LE_U32(x) (x)


/* -------------------------------------------------------------------------- */

#endif // ... mipos_BSP_STM8_H__
