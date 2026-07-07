/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

// mipOS hello-world for the QEMU lm3s6965evb board.
//
// Build (see README.md) and run with:
//   qemu-system-arm -M lm3s6965evb -kernel mipos-arm-qemu.elf -nographic

#include "mipos.h"
#include "mipos_bsp.h"

/* ------------------------------------------------------------------------- */

static char root_stack[4096];

/* ------------------------------------------------------------------------- */

static void uart_puts(const char* s)
{
    while (*s) {
        if (*s == '\n') {
            mipos_bsp_rs232_putc('\r');
        }
        mipos_bsp_rs232_putc((unsigned char)*s++);
    }
}

static void uart_put_u32(uint32_t value)
{
    char buf[11];
    char* p = &buf[sizeof(buf) - 1];

    *p = '\0';
    do {
        *--p = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value);

    uart_puts(p);
}

/* ------------------------------------------------------------------------- */

static int root_task(task_param_t param)
{
    uint32_t tick = 0;

    (void)param;

    uart_puts("root task running\n");

    for (;;) {
        mipos_tm_msleep(1000);
        uart_puts("tick: ");
        uart_put_u32(++tick);
        uart_puts("\n");
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    mipos_bsp_configure_rs232(115200);

    uart_puts("mipOS ARM/QEMU port booting...\n");

    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    /* never reached: mipos_start does not return */
    for (;;) {
    }

    return 0;
}
