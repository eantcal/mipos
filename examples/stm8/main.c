/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/

/* --------------------------------------------------------------------------- */

#include "mipos.h"


/* -------------------------------------------------------------------------- */

/* TODO: implement for dubugging purposes */
void assert_failed(unsigned char* file, unsigned long line)
{ 
    (void) file;
    (void) line;

    while (1);
}


/* -------------------------------------------------------------------------- */

/* TODO: implement putchar required by vprintf */ 
void putchar(char c) {
    (void) c;
}


/* --------------------------------------------------------------------------- */

static char root_stack[16 * 1024] = { 0 };


/* --------------------------------------------------------------------------- */

int root_task(task_param_t param)
{
    (void)param;

    printf("Hello World !\n");
    mipos_t_suspend();

    return 0; // this statement is never executed
}


/* --------------------------------------------------------------------------- */

int main()
{
    // Start mipOS root task
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    return 0; // this statement is never executed
}


