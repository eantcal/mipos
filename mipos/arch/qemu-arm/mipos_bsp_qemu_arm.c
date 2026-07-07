/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

// BSP implementation for the QEMU lm3s6965evb board (TI Stellaris LM3S6965,
// Cortex-M3): UART0 console and SysTick-driven 1 ms RTC quantum.

#include "mipos_bsp.h"
#include "mipos_kernel.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */

#define QEMU_ARM_REG(addr) (*(volatile uint32_t*)(addr))

/* System clock: QEMU emulates the LM3S6965 with the reset RCC configuration,
 * which yields 200 MHz / 16 = 12.5 MHz. Only the SysTick reload value depends
 * on it; the emulated UART ignores the programmed baud rate. */
#define QEMU_ARM_SYSCLK_HZ 12500000u

/* --- UART0 (Stellaris UART, PL011-like register layout) ------------------ */

#define UART0_BASE 0x4000C000u
#define UART0_DR QEMU_ARM_REG(UART0_BASE + 0x000)
#define UART0_FR QEMU_ARM_REG(UART0_BASE + 0x018)
#define UART0_IBRD QEMU_ARM_REG(UART0_BASE + 0x024)
#define UART0_FBRD QEMU_ARM_REG(UART0_BASE + 0x028)
#define UART0_LCRH QEMU_ARM_REG(UART0_BASE + 0x02C)
#define UART0_CTL QEMU_ARM_REG(UART0_BASE + 0x030)

#define UART_FR_TXFF 0x20u /* TX FIFO full */
#define UART_FR_RXFE 0x10u /* RX FIFO empty */

#define UART_LCRH_WLEN8 0x60u
#define UART_LCRH_FEN 0x10u

#define UART_CTL_UARTEN 0x001u
#define UART_CTL_TXE 0x100u
#define UART_CTL_RXE 0x200u

/* System control: enable the UART0 clock (QEMU does not gate peripheral
 * clocks, but keep the sequence faithful to the real hardware). */
#define SYSCTL_RCGC1 QEMU_ARM_REG(0x400FE104u)
#define SYSCTL_RCGC1_UART0 0x0001u

/* Run-mode clock configuration */
#define SYSCTL_RCC QEMU_ARM_REG(0x400FE060u)
#define SYSCTL_RCC_SYSDIV_16 (0xFu << 23) /* 200 MHz / 16 = 12.5 MHz */
#define SYSCTL_RCC_USESYSDIV (1u << 22)
#define SYSCTL_RCC_XTAL_8MHZ (0xBu << 6)

/* --- SysTick (ARMv7-M) ---------------------------------------------------- */

#define SYST_CSR QEMU_ARM_REG(0xE000E010u)
#define SYST_RVR QEMU_ARM_REG(0xE000E014u)
#define SYST_CVR QEMU_ARM_REG(0xE000E018u)

#define SYST_CSR_ENABLE 0x1u
#define SYST_CSR_TICKINT 0x2u
#define SYST_CSR_CLKSOURCE 0x4u

/* ------------------------------------------------------------------------- */

void mipos_bsp_setup_clk(void)
{
    /* Out of reset the Stellaris runs from the bypass oscillator. Select the
     * PLL with SYSDIV = 16 so the system clock is 200 MHz / 16 = 12.5 MHz
     * (QEMU applies the change immediately; on real hardware this sequence
     * would also need to wait for the PLL lock interrupt). */
    SYSCTL_RCC =
      SYSCTL_RCC_SYSDIV_16 | SYSCTL_RCC_USESYSDIV | SYSCTL_RCC_XTAL_8MHZ;
}

/* ------------------------------------------------------------------------- */

void mipos_bsp_configure_rs232(unsigned int baud_rate)
{
    uint32_t divisor;

    if (baud_rate == 0) {
        baud_rate = 115200;
    }

    SYSCTL_RCGC1 |= SYSCTL_RCGC1_UART0;

    /* Disable the UART while it is being configured */
    UART0_CTL = 0;

    /* Integer and fractional baud-rate divisors (divisor = clk / (16*baud),
     * fractional part in 1/64 units). QEMU ignores them, but program the
     * values a real LM3S6965 would need. */
    divisor = (QEMU_ARM_SYSCLK_HZ * 4u) / baud_rate; /* divisor * 64 */
    UART0_IBRD = divisor >> 6;
    UART0_FBRD = divisor & 0x3Fu;

    /* 8-bit words, FIFO enabled */
    UART0_LCRH = UART_LCRH_WLEN8 | UART_LCRH_FEN;

    UART0_CTL = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
}

/* ------------------------------------------------------------------------- */

void mipos_bsp_rs232_putc(unsigned char c)
{
    while (UART0_FR & UART_FR_TXFF) {
        /* wait for room in the TX FIFO */
    }

    UART0_DR = c;
}

/* ------------------------------------------------------------------------- */

unsigned int mipos_bsp_rs232_recv_byte(unsigned char* key)
{
    unsigned char c;

    if (UART0_FR & UART_FR_RXFE) {
        return 0;
    }

    c = (unsigned char)(UART0_DR & 0xFFu);

    /* Normalize LF to CR so the console end-of-line handling works both
     * with interactive terminals (Enter sends CR) and with piped input
     * (the QEMU stdio backend may deliver LF only), mirroring what the
     * host-simulator BSP does. */
    if (c == '\n') {
        c = '\r';
    }

    *key = c;
    return 1;
}

/* ------------------------------------------------------------------------- */

/* Number of 1 ms SysTick interrupts not yet accounted in the kernel RTC.
 * Written by the SysTick ISR, drained by the scheduler epoch hook, so the
 * kernel data structures are only ever touched from thread context. */
static volatile uint32_t systick_pending_ticks;

void SysTick_Handler(void)
{
    ++systick_pending_ticks;
}

/* ------------------------------------------------------------------------- */

void mipos_bsp_create_hw_rtc_timer(void)
{
    SYST_CSR = 0;
    SYST_RVR = (QEMU_ARM_SYSCLK_HZ / 1000u) - 1u; /* 1 ms tick */
    SYST_CVR = 0;
    SYST_CSR = SYST_CSR_ENABLE | SYST_CSR_TICKINT | SYST_CSR_CLKSOURCE;
}

/* ------------------------------------------------------------------------- */

void mipos_bsp_qemu_arm_rtc_epoch(void)
{
    uint32_t ticks;

    mipos_init_cs();

    mipos_enter_cs();
    ticks = systick_pending_ticks;
    systick_pending_ticks = 0;
    mipos_leave_cs();

    if (ticks) {
        mipos_update_rtc(ticks);
    }
}

/* ------------------------------------------------------------------------- */

void mipos_printf(const char* fmt, ...)
{
    char buffer[256];
    const char* out = fmt;
    va_list args;

    va_start(args, fmt);
    if (vsnprintf(buffer, sizeof(buffer), fmt, args) >= 0) {
        out = buffer;
    }
    va_end(args);

    while (*out) {
        if (*out == '\n') {
            mipos_bsp_rs232_putc('\r');
        }
        mipos_bsp_rs232_putc((unsigned char)*out++);
    }
}
