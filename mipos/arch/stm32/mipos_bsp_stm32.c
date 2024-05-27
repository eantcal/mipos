/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* -------------------------------------------------------------------------- */

#include "mipos_bsp_stm32.h"
#include "mipos_kernel.h"


/* -------------------------------------------------------------------------- */

void mipos_bsp_setup_reset_and_wd(void)
{
    RCC_DeInit();
    NVIC_SETPRIMASK();

#ifdef ENABLE_WATCHDOG
    /* Enable LSI */
    RCC_LSICmd(ENABLE);
    /* Wait till LSI is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
    }

    /* Select LSI as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    /* IWDG timeout equal to 350ms (the timeout may varies due to LSI frequency
       dispersion)
       -------------------------------------------------------------*/
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
    IWDG_SetPrescaler(IWDG_Prescaler_32);

    /* Set counter reload value to 349 */
    IWDG_SetReload(349);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
#endif // ENABLE_WATCHDOG
}


/* -------------------------------------------------------------------------- */

void mipos_bsp_exception_vector(void)
{
    // NVIC init
#ifndef EMB_FLASH
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}


/* -------------------------------------------------------------------------- */

#ifndef USE_EXTERNAL_OSCILLATOR
void mipos_bsp_setup_clk(void)
{
    // 1. Clocking the controller from internal HSI RC (8 MHz)
    // wait until the HSI is ready
    RCC_HSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
        ;

    // Use internal hoscillator
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    RCC_HSEConfig(RCC_HSE_OFF);

#ifdef notyet
    RCC_MCOConfig(RCC_MCO_HSI);
#define HSI_CALIBRATION_VALUE 0x1c
    RCC_AdjustHSICalibrationValue(HSI_CALIBRATION_VALUE);
#endif

    // 3. Init PLL
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16); // 64MHz
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        ;

    // 4. Set system clock dividers
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

#ifdef EMB_FLASH
    // 5. Init Embedded Flash
    // Zero wait state, if 0 < HCLK 24 MHz
    // One wait state, if 24 MHz < HCLK 56 MHz
    // Two wait states, if 56 MHz < HCLK 72 MHz
    // Flash wait state
    FLASH_SetLatency(FLASH_Latency_2);
    // Half cycle access
    FLASH_HalfCycleAccessCmd(FLASH_HalfCycleAccess_Disable);
    // Prefetch buffer
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
#endif // EMB_FLASH

    // 5. Clock system from PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
}

#else // USE_EXTERNAL_OSCILLATOR defined

void mipos_bsp_setup_clk(void)
{
    // 1. Clocking the controller from internal HSI RC (8 MHz)
    RCC_HSICmd(ENABLE);

    // wait until the HSI is ready
    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
        ;
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

    // 2. Enable ext. high frequency OSC
    RCC_HSEConfig(RCC_HSE_ON);

    // wait until the HSE is ready
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
        ;

    // 3. Init PLL
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // 72MHz
    RCC_PLLCmd(ENABLE);

    // wait until the PLL is ready
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        ;

    // 4. Set system clock dividers
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

#ifdef EMB_FLASH
    // 5. Init Embedded Flash
    // Zero wait state, if 0 < HCLK 24 MHz
    // One wait state, if 24 MHz < HCLK 56 MHz
    // Two wait states, if 56 MHz < HCLK 72 MHz
    // Flash wait state
    FLASH_SetLatency(FLASH_Latency_2);
    // Half cycle access
    FLASH_HalfCycleAccessCmd(FLASH_HalfCycleAccess_Disable);
    // Prefetch buffer
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
#endif // EMB_FLASH

    // 5. Clock system from PLL
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
}
#endif //! USE_EXTERNAL_OSCILLATOR


/* -------------------------------------------------------------------------- */

void mipos_bsp_configure_rs232(unsigned int baud_rate)
{
    GPIO_InitTypeDef gpio_init = { 0 };
    USART_InitTypeDef usart_init = { 0 };

    /* USART2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    usart_init.USART_BaudRate = baud_rate;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart_init.USART_Clock = USART_Clock_Disable;
    usart_init.USART_CPOL = USART_CPOL_Low;
    usart_init.USART_CPHA = USART_CPHA_2Edge;
    usart_init.USART_LastBit = USART_LastBit_Disable;

    /* Configure the USART2 */
    USART_Init(USART2, &usart_init);

    /* Enable the USART Receive interrupt */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    gpio_init.GPIO_Pin = GPIO_Pin_2;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_3;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);

    /* Enable the USART2 */
    USART_Cmd(USART2, ENABLE);
}


/* -------------------------------------------------------------------------- */

unsigned int mipos_bsp_rs232_recv_byte(unsigned char* key)
{
    if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
        *key = (unsigned char)USART2->DR;
        return 1;
    }

    return 0;
}


/* -------------------------------------------------------------------------- */


void mipos_bsp_rs232_putc(unsigned char c)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {
        mipos_tm_wkafter(0);
    }

    USART_SendData(USART2, c);
}
