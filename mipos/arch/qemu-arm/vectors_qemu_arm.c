/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

// Vector table for the QEMU lm3s6965evb board (Cortex-M3).

#include <stdint.h>

/* ------------------------------------------------------------------------- */

extern uint32_t _estack; /* provided by the linker script */

void Reset_Handler(void);   /* startup_qemu_arm.s */
void SysTick_Handler(void); /* mipos_bsp_qemu_arm.c */

/* ------------------------------------------------------------------------- */

void Default_Handler(void)
{
    for (;;) {
        /* unexpected exception/interrupt: halt here */
    }
}

#define WEAK_DEFAULT __attribute__((weak, alias("Default_Handler")))

void NMI_Handler(void) WEAK_DEFAULT;
void HardFault_Handler(void) WEAK_DEFAULT;
void MemManage_Handler(void) WEAK_DEFAULT;
void BusFault_Handler(void) WEAK_DEFAULT;
void UsageFault_Handler(void) WEAK_DEFAULT;
void SVC_Handler(void) WEAK_DEFAULT;
void DebugMon_Handler(void) WEAK_DEFAULT;
void PendSV_Handler(void) WEAK_DEFAULT;
void UART0_IRQHandler(void) WEAK_DEFAULT; /* IRQ 5, unused: RX is polled */

/* ------------------------------------------------------------------------- */

typedef void (*vector_t)(void);

/* Cortex-M3 core exceptions plus the first 48 LM3S6965 interrupt lines
 * (all routed to Default_Handler except UART0). */
__attribute__((section(".isr_vector"), used))
const vector_t g_vector_table[16 + 48] = {
    (vector_t)(uintptr_t)&_estack, /* initial stack pointer     */
    Reset_Handler,                 /* reset                     */
    NMI_Handler,                   /* NMI                       */
    HardFault_Handler,             /* hard fault                */
    MemManage_Handler,             /* MPU fault                 */
    BusFault_Handler,              /* bus fault                 */
    UsageFault_Handler,            /* usage fault               */
    0, 0, 0, 0,                    /* reserved                  */
    SVC_Handler,                   /* SVCall                    */
    DebugMon_Handler,              /* debug monitor             */
    0,                             /* reserved                  */
    PendSV_Handler,                /* PendSV                    */
    SysTick_Handler,               /* SysTick: mipOS 1 ms tick  */

    /* External interrupts (LM3S6965) */
    Default_Handler,  /*  0: GPIO A     */
    Default_Handler,  /*  1: GPIO B     */
    Default_Handler,  /*  2: GPIO C     */
    Default_Handler,  /*  3: GPIO D     */
    Default_Handler,  /*  4: GPIO E     */
    UART0_IRQHandler, /*  5: UART0      */
    Default_Handler,  /*  6: UART1      */
    Default_Handler,  /*  7: SSI0       */
    Default_Handler,  /*  8: I2C0       */
    Default_Handler,  /*  9: PWM fault  */
    Default_Handler,  /* 10: PWM gen 0  */
    Default_Handler,  /* 11: PWM gen 1  */
    Default_Handler,  /* 12: PWM gen 2  */
    Default_Handler,  /* 13: QEI0       */
    Default_Handler,  /* 14: ADC seq 0  */
    Default_Handler,  /* 15: ADC seq 1  */
    Default_Handler,  /* 16: ADC seq 2  */
    Default_Handler,  /* 17: ADC seq 3  */
    Default_Handler,  /* 18: watchdog   */
    Default_Handler,  /* 19: timer 0A   */
    Default_Handler,  /* 20: timer 0B   */
    Default_Handler,  /* 21: timer 1A   */
    Default_Handler,  /* 22: timer 1B   */
    Default_Handler,  /* 23: timer 2A   */
    Default_Handler,  /* 24: timer 2B   */
    Default_Handler,  /* 25: comp 0     */
    Default_Handler,  /* 26: comp 1     */
    Default_Handler,  /* 27: reserved   */
    Default_Handler,  /* 28: sysctl     */
    Default_Handler,  /* 29: flash      */
    Default_Handler,  /* 30: GPIO F     */
    Default_Handler,  /* 31: GPIO G     */
    Default_Handler,  /* 32: reserved   */
    Default_Handler,  /* 33: UART2      */
    Default_Handler,  /* 34: reserved   */
    Default_Handler,  /* 35: timer 3A   */
    Default_Handler,  /* 36: timer 3B   */
    Default_Handler,  /* 37: I2C1       */
    Default_Handler,  /* 38: QEI1       */
    Default_Handler,  /* 39: reserved   */
    Default_Handler,  /* 40: reserved   */
    Default_Handler,  /* 41: reserved   */
    Default_Handler,  /* 42: ethernet   */
    Default_Handler,  /* 43: hibernate  */
    Default_Handler,  /* 44: reserved   */
    Default_Handler,  /* 45: reserved   */
    Default_Handler,  /* 46: reserved   */
    Default_Handler,  /* 47: reserved   */
};
