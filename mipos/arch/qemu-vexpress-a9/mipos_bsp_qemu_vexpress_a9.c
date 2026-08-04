/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

#include "mipos_bsp.h"
#include "mipos_kernel.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define VEXPRESS_REG(addr) (*(volatile uint32_t*)(addr))

/* Versatile Express motherboard PL011 UART0. */
#define UART0_BASE 0x10009000u
#define UART0_DR VEXPRESS_REG(UART0_BASE + 0x000)
#define UART0_FR VEXPRESS_REG(UART0_BASE + 0x018)
#define UART0_IBRD VEXPRESS_REG(UART0_BASE + 0x024)
#define UART0_FBRD VEXPRESS_REG(UART0_BASE + 0x028)
#define UART0_LCRH VEXPRESS_REG(UART0_BASE + 0x02C)
#define UART0_CR VEXPRESS_REG(UART0_BASE + 0x030)
#define UART0_IMSC VEXPRESS_REG(UART0_BASE + 0x038)
#define UART0_ICR VEXPRESS_REG(UART0_BASE + 0x044)

#define UART_FR_TXFF 0x20u
#define UART_FR_RXFE 0x10u
#define UART_LCRH_WLEN8 0x60u
#define UART_LCRH_FEN 0x10u
#define UART_CR_UARTEN 0x001u
#define UART_CR_TXE 0x100u
#define UART_CR_RXE 0x200u

void mipos_bsp_setup_clk(void)
{
}

void mipos_bsp_configure_rs232(unsigned int baud_rate)
{
    uint32_t divisor;

    if (baud_rate == 0) {
        baud_rate = 115200;
    }

    UART0_CR = 0;
    UART0_ICR = 0x7ffu;
    UART0_IMSC = 0;

    divisor = (24000000u * 4u) / baud_rate;
    UART0_IBRD = divisor >> 6;
    UART0_FBRD = divisor & 0x3fu;
    UART0_LCRH = UART_LCRH_WLEN8 | UART_LCRH_FEN;
    UART0_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

void mipos_bsp_rs232_putc(unsigned char c)
{
    while (UART0_FR & UART_FR_TXFF) {
    }

    UART0_DR = c;
}

unsigned int mipos_bsp_rs232_recv_byte(unsigned char* key)
{
    unsigned char c;

    if (UART0_FR & UART_FR_RXFE) {
        return 0;
    }

    c = (unsigned char)(UART0_DR & 0xffu);
    if (c == '\n') {
        c = '\r';
    }

    *key = c;
    return 1;
}

void mipos_bsp_create_hw_rtc_timer(void)
{
    /* Step one deliberately runs without an IRQ timer. The next vexpress-a9
     * milestone wires SP804/private timer through the GIC. */
}

void mipos_bsp_qemu_vexpress_a9_epoch(void)
{
    mipos_update_rtc(1);
}

void mipos_bsp_qemu_vexpress_a9_irq(void)
{
}

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
