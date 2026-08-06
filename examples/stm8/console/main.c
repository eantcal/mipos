/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

#include "mipos.h"


/* ------------------------------------------------------------------------- */

void assert_failed(unsigned char* file, unsigned long line)
{
    (void)file;
    (void)line;

    while (1) {
        ;
    }
}


/* ------------------------------------------------------------------------- */

#if defined(__SDCC) && __SDCC_REVISION < 9624
void putchar(char c)
{
    mipos_bsp_rs232_putc((unsigned char)c);
}
#else
int putchar(int c)
{
    mipos_bsp_rs232_putc((unsigned char)c);
    return c;
}
#endif


/* ------------------------------------------------------------------------- */

static char root_stack[1024] = { 0 };


/* ------------------------------------------------------------------------- */

int root_task(task_param_t param)
{
    mipos_console_init_t cinit = { 0 };

    (void)param;

    cinit.prompt = "\rmipOS-stm8>";
    cinit.end_line_char = '\r';
    cinit.baud_rate = 9600;
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;

    mipos_console_init(&cinit);
    mipos_t_suspend();

    return 0;
}


/* ------------------------------------------------------------------------- */

int main(void)
{
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));
    return 0;
}
