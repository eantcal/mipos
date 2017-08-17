/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#include "mipos_kernel.h"
#include "stm8s_clk.h"


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

    /* Select HSI as system clock source */
    CLK_SYSCLKSourceSwitchCmd(ENABLE);
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);


#ifndef SLOWMODE
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
#else
    /* Divisore di frequenza per il clock */
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_4);
#endif		
    while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI) {
    }
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
    //IWDG_Enable();

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
@far @interrupt void TIM4_UPD_OVF_IRQHandler(void)
{
    //TIM4_ClearFlag(TIM4_FLAG_Update);
    mipos_update_rtc(1 /*ms*/);

    TIM4_ClearITPendingBit(TIM4_IT_Update);
}


/* -------------------------------------------------------------------------- */

void mipos_bsp_create_hw_rtc_timer(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

    /* TimerTick = 1 ms
       Warning: fcpu must be equal to 16MHz
       fck_cnt = fck_psc/presc = fcpu/128 = 125kHz --> 1 tick every 8 µs
       ==> 1 ms / 8 µs = 125 ticks
     */

#ifndef SLOWMODE
    TIM4_TimeBaseInit(TIM4_Prescaler_128, 125);
#else
    TIM4_TimeBaseInit(TIM4_Prescaler_32, 125);
#endif		

    /* Master Mode selection: Update event */
    TIM4_SelectOutputTrigger(TIM4_TRGOSource_Update);

    TIM4_UpdateRequestConfig(TIM4_UpdateSource_Global);

    TIM4_ITConfig(TIM4_IT_Update, ENABLE);

    TIM4_Cmd(ENABLE);
}


/* -------------------------------------------------------------------------- */

/** mipos_bsp_configure_rs232 configures serial port used for the
 *  console
 *
 * @param none
 * @return none
 */
void mipos_bsp_configure_rs232(unsigned int baud_rate)
{

    /* Enable USART clock */
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOA, GPIO_Pin_5, ENABLE);

    /* Configure USART Rx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(GPIOA, GPIO_Pin_4, ENABLE);

    /* USART configuration */
    USART_Init(USART1, baud_rate,
        USART_WordLength_8b,
        USART_StopBits_1,
        USART_Parity_No,
        USART_Mode_Tx | USART_Mode_Rx);
}


/* -------------------------------------------------------------------------- */

unsigned int mipos_bsp_rs232_recv_byte(unsigned char *key)
{
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
        *key = USART_ReceiveData8(USART1);
        return 1;
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

void mipos_bsp_rs232_putc(unsigned char c)
{
    USART_SendData8(USART1, c);

    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
        // empty
    }
}


/* -------------------------------------------------------------------------- */

int setjmpex(jmp_buf env)
{
    #asm
        SUBW SP, #$0x6
        PUSHW X
        PUSHW Y
        PUSH A
        PUSH CC

        POP CC
        POP A
        POPW Y
        POPW X
        ADDW SP, #$0x6


        LD    A, (0x01, SP)
        LD(0x02, X), A

        LDW   Y, (0x02, SP)  //pc
        LDW(0x03, X), Y

        LDW   Y, SP         //sp
        LDW(X), Y

        CLRW  X
        RETF
        #endasm
}


/* -------------------------------------------------------------------------- */

void longjmpex(jmp_buf env, int val)
{
#asm
        LDW   Y, (0x04, SP)
        LDW   0x04, Y
        LDW   Y, X
        LDW   Y, (Y)
        LDW   SP, Y

        LD    A, (0x02, X)
        LD(0x01, SP), A

        LDW   X, (0x03, X)
        LDW(0x02, SP), X

        LDW   X, 0x04

        JRNE  skip_inc_x;
        INCW X;

skip_inc_x:
        LDW  Y, SP
        LDW  SP, Y
        SUBW SP, #$0xc; 'sp=sp-12

        ADD SP, #0x1;  //  POP CC
        ADD SP, #0x1;  //  POP A
        POPW Y
        ADD SP, #0x2;  //  POPW X

        TNZW X

        JRNE  skip_inc_x2;
        INCW X;
skip_inc_x2:

        RETF
#endasm
}
