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
#ifdef SDCC
INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
#elif defined( _COSMIC_ )
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
}


/* -------------------------------------------------------------------------- */

#ifdef SDCC
void * mipos_get_sp()
{
   __asm
   LDW X,SP
   ADW X, #2
   __endasm ;
}
#endif
