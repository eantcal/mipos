/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* --------------------------------------------------------------------------- */

#include "mipos.h"


/* --------------------------------------------------------------------------- */

/* Reserve 16 KB for root task */
static char root_stack [16 * 1024] = { 0 };


/* --------------------------------------------------------------------------- */

void timer_handler(mipos_timer_ctx_t ctx)
{
   mipos_puts("timer_handler\r\n");
}


/* --------------------------------------------------------------------------- */

int root_task(task_param_t param)
{
    (void)param;
    timer_id_t timer;

    mipos_printf("Root task stated, creating task1 and task2\n");

    timer = mipos_timer_start( 
        timer_handler, 
        0 /*no cancel callback*/ , 
        1000 /* 1s */, 
        0 /*ctx*/, 
        1 /*periodic*/ );

    mipos_tm_msleep( 10 * 1000 /*10 seconds*/ );
    mipos_timer_cancel( timer );

    
    mipos_printf("Root cancelled timer\n");

    // root task has completed its job so it can terminate itself
    mipos_t_delete(0); 

    return 0; // this statement is never executed
}


/* --------------------------------------------------------------------------- */

int main()
{
    // Start mipOS root task
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    return 0; // this statement is never executed
}
