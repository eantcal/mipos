// main.c : Defines the entry point for the console application.
//

#include "mipos.h"


/* --------------------------------------------------------------------------- */

static char root_stack [16 * 1024] = { 0 };
static char task1_stack[16 * 1024] = { 0 };
static char task2_stack[16 * 1024] = { 0 };

int task1(task_param_t param);
int task2(task_param_t param);


/* --------------------------------------------------------------------------- */

int task1(task_param_t param)
{
    (void)param;

    mipos_printf("\nTask1 started\n");

    mipos_t_create("task2",
        task2,
        (task_param_t)0,
        task2_stack,
        sizeof(task2_stack),
        MIPOS_NO_RT);

    while (1) {
        mipos_puts(".");
        mipos_tm_msleep( 1000 /* ms */ );
    }

    return 0; // this statement is never executed
}

/* --------------------------------------------------------------------------- */


int task2(task_param_t param)
{
    (void)param;

    mipos_printf("Task2 started\n");

    while (1) {
        mipos_puts("x");
        mipos_tm_msleep( 500 /*ms*/ );
    }

    return 0; // this statement is never executed
}

/* --------------------------------------------------------------------------- */

int root_task(task_param_t param)
{
    (void)param;

    mipos_printf("Root task stated, creating task1 and task2\n");

     mipos_t_create("task1",
        task1,
        (task_param_t)0,
        task1_stack,
        sizeof(task1_stack),
        MIPOS_NO_RT);    
         
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
