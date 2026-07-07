/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

// mipOS console (CLI) for the QEMU lm3s6965evb board.
//
// Run with (interactive; quit with Ctrl-A x):
//   qemu-system-arm -M lm3s6965evb -kernel mipos-arm-qemu-console.elf
//       -nographic -serial mon:stdio
//
// Type 'help' at the mipOS> prompt for the list of commands.

#include "mipos.h"
#include "mipos_bsp.h"

/* ------------------------------------------------------------------------- */

static char root_stack[8192];

/* ------------------------------------------------------------------------- */

static int root_task(task_param_t param)
{
    mipos_console_init_t cinit = { 0 };

    (void)param;

    cinit.prompt = "\nmipOS>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;

    mipos_console_init(&cinit);
    mipos_t_suspend();

    return 0;
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    mipos_bsp_configure_rs232(115200);

    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    /* never reached: mipos_start does not return */
    for (;;) {
    }

    return 0;
}
