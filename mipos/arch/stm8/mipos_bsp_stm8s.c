/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* -------------------------------------------------------------------------- */

#ifdef __RCSTM8__
#pragma SRC("mipos_bst_stm8s.s")
#endif

#include "mipos_kernel.h"


/* -------------------------------------------------------------------------- */

int mipos_bsp_check_reset_type(void)
{

    /* Check if the system has resumed from IWDG reset ------*/
    if (RST_GetFlagStatus(RST_FLAG_IWDGF) != RESET) {

        /* Clear reset flags */
        RST_ClearFlag(RST_FLAG_IWDGF);

        return RESET_TYPE_IWDG;
    }

    return RESET_TYPE_HWRS;
}


/* -------------------------------------------------------------------------- */

void mipos_bsp_setup_reset_and_wd(void)
{
    CLK_DeInit();

    /* Configure the HSI prescaler to the optimal value */
    CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);

#ifdef ENABLE_WATCHDOG

    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI / 256 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);

    /* Set counter reload value */
    IWDG_SetReload(0xFF /*255 ~ 2 sec */);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();

#endif // ENABLE_WATCHDOG
}


/* -------------------------------------------------------------------------- */

/**
 * @brief Timer4 Update/Overflow Interrupt routine.
 * @par Parameters:
 * None
 * @retval
 * None
 */
#ifdef __SDCC
INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
#elif defined(_COSMIC_)
@far @interrupt void TIM4_UPD_OVF_IRQHandler(void)
#else /* _RAISONANCE_ */
void TIM4_UPD_OVF_IRQHandler(void) interrupt 23
#endif
{
    mipos_update_rtc(1 /*ms*/);
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}


/* -------------------------------------------------------------------------- */

void mipos_bsp_create_hw_rtc_timer(void)
{
#if MIPOS_STM8_UCSIM
    return;
#else
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);

    /* TimerTick = 1 ms
       Warning: fcpu must be equal to 16MHz
       fck_cnt = fck_psc/presc = fcpu/128 = 125kHz --> 1 tick every 8 µs
       ==> 1 ms / 8 µs = 125 ticks
     */

#ifndef SLOWMODE
    TIM4_TimeBaseInit(TIM4_PRESCALER_128, 125);
#else
    TIM4_TimeBaseInit(TIM4_PRESCALER_32, 125);
#endif

    TIM4_UpdateRequestConfig(TIM4_UPDATESOURCE_GLOBAL);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
    TIM4_Cmd(ENABLE);
#endif
}


/* -------------------------------------------------------------------------- */

#if MIPOS_STM8_UCSIM
#define MIPOS_STM8_UCSIM_SIMIF (*(volatile unsigned char*)0x5000)
#ifdef MIPOS_STM8_UCSIM_SCRIPTED_CONSOLE
static const char* mipos_stm8_ucsim_input = "help\rps\rdump 0 16\r";
static unsigned int mipos_stm8_ucsim_input_pos = 0;
#endif
#endif

void mipos_bsp_configure_rs232(unsigned int baud_rate)
{
    if (!baud_rate) {
        baud_rate = 115200;
    }

    CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1, ENABLE);
    UART1_DeInit();
    UART1_Init((uint32_t)baud_rate,
               UART1_WORDLENGTH_8D,
               UART1_STOPBITS_1,
               UART1_PARITY_NO,
               UART1_SYNCMODE_CLOCK_DISABLE,
               UART1_MODE_TXRX_ENABLE);
    UART1_Cmd(ENABLE);
}


/* -------------------------------------------------------------------------- */

unsigned int mipos_bsp_rs232_recv_byte(unsigned char* key)
{
    if (!key) {
        return 0;
    }

#if MIPOS_STM8_UCSIM && defined(MIPOS_STM8_UCSIM_SCRIPTED_CONSOLE)
    if (mipos_stm8_ucsim_input[mipos_stm8_ucsim_input_pos]) {
        *key = (unsigned char)mipos_stm8_ucsim_input[mipos_stm8_ucsim_input_pos++];
        return 1;
    }
#endif

    if (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET) {
        return 0;
    }

    *key = UART1_ReceiveData8();
    return 1;
}


/* -------------------------------------------------------------------------- */

void mipos_bsp_rs232_putc(unsigned char c)
{
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET) {
        ;
    }

    UART1_SendData8(c);
}


/* -------------------------------------------------------------------------- */

#ifdef __SDCC
void* mipos_bsp_call_task_context;

int mipos_bsp_call1(void* entry_point, void* param) __naked
{
    (void)entry_point;
    (void)param;

    __asm
        exgw x, y
        ldw x, (0x03, sp)
        push #<00001$
        push #>00001$
        ld a, yl
        push a
        ld a, yh
        push a
        ret
00001$:
        ldw y, (0x01, sp)
        addw sp, #4
        ld a, yl
        push a
        ld a, yh
        push a
        ret
    __endasm;
}


/* -------------------------------------------------------------------------- */

void mipos_bsp_run_task_from_tcb(void* task_context) __naked
{
    (void)task_context;

    __asm
        ldw _mipos_bsp_call_task_context+0, x
        ldw y, x
        ldw y, (0x02, y)
        ldw x, (0x06, x)
        push #<00003$
        push #>00003$
        ld a, yl
        push a
        ld a, yh
        push a
        ret
00003$:
        ldw y, _mipos_bsp_call_task_context+0
        ldw (0x04, y), x
        ret
    __endasm;
}


/* -------------------------------------------------------------------------- */

int mipos_bsp_call2(void* entry_point, int arg0, void* arg1) __naked
{
    (void)entry_point;
    (void)arg0;
    (void)arg1;

    __asm
        exgw x, y
        ldw x, (0x05, sp)
        ld a, xl
        push a
        ld a, xh
        push a
        ldw x, (0x05, sp)
        push #<00002$
        push #>00002$
        ld a, yl
        push a
        ld a, yh
        push a
        ret
00002$:
        ldw y, (0x01, sp)
        addw sp, #6
        ld a, yl
        push a
        ld a, yh
        push a
        ret
    __endasm;
}


/* -------------------------------------------------------------------------- */

void* mipos_get_sp(void)
{
    __asm__("ldw x, sp\n" "addw x, #2\n");
}
#endif
