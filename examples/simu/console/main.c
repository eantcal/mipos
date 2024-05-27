/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

#include "mipos.h"


/* ---------------------------------------------------------------------------
 */

static char root_stack[128 * 1024];
int root_task(task_param_t param)
{
    mipos_console_init_t cinit = { 0 };
    cinit.prompt = "\nmipOS>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;

    mipos_console_init(&cinit);
    mipos_t_suspend();

    return 0;
}


/* ---------------------------------------------------------------------------
 */

int main()
{
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));
    return 0;
}
