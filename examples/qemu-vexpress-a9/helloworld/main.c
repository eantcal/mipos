/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* mipOS hello-world for QEMU vexpress-a9. */

#include "mipos.h"
#include "mipos_bsp.h"

static char root_stack[4096];

static void uart_puts(const char* s)
{
    while (*s) {
        if (*s == '\n') {
            mipos_bsp_rs232_putc('\r');
        }
        mipos_bsp_rs232_putc((unsigned char)*s++);
    }
}

static int root_task(task_param_t param)
{
    (void)param;

    uart_puts("vexpress-a9 root task running\n");
    uart_puts("timer/GIC are intentionally deferred to the next step\n");

    mipos_t_suspend();
    return 0;
}

int main(void)
{
    mipos_bsp_configure_rs232(115200);
    uart_puts("mipOS QEMU vexpress-a9 port booting...\n");

    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    for (;;) {
    }

    return 0;
}
