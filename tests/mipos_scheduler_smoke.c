#include "mipos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Cooperative multitasking smoke test.
 *
 * The root task spawns two worker tasks that each append their own tag to a
 * shared sequence, yielding to the scheduler after every step, and then
 * terminate by returning from their entry point (which exercises the
 * task-startup/teardown path of the dispatcher). The root task verifies that
 * both workers ran to completion and that their execution interleaved.
 */

static char root_stack[64 * 1024];
static char worker_a_stack[64 * 1024];
static char worker_b_stack[64 * 1024];

#define STEPS_PER_WORKER 3

static char sequence[2 * STEPS_PER_WORKER + 1];
static unsigned int seq_len;
static int done_count;

static void append_tag(char tag)
{
    if (seq_len < (unsigned int)(sizeof(sequence) - 1)) {
        sequence[seq_len++] = tag;
    }
}

static int worker_task(task_param_t param)
{
    const char tag = (char)(mipos_reg_t)param;
    int i;

    for (i = 0; i < STEPS_PER_WORKER; ++i) {
        append_tag(tag);
        mipos_schedule();
    }

    ++done_count;

    return tag;
}

static void fail(const char* reason)
{
    fprintf(stderr, "FAILED: %s (sequence='%s')\n", reason, sequence);
    exit(1);
}

static int root_task(task_param_t param)
{
    unsigned int i;
    unsigned int count_a = 0, count_b = 0;
    unsigned int first_b = 0, last_a = 0;

    (void)param;

    mipos_t_create("workerA",
                   worker_task,
                   (task_param_t)(mipos_reg_t)'A',
                   worker_a_stack,
                   sizeof(worker_a_stack),
                   MIPOS_NO_RT);

    mipos_t_create("workerB",
                   worker_task,
                   (task_param_t)(mipos_reg_t)'B',
                   worker_b_stack,
                   sizeof(worker_b_stack),
                   MIPOS_NO_RT);

    while (done_count < 2) {
        mipos_schedule();
    }

    if (seq_len != 2 * STEPS_PER_WORKER) {
        fail("unexpected sequence length");
    }

    for (i = 0; i < seq_len; ++i) {
        switch (sequence[i]) {
            case 'A':
                ++count_a;
                last_a = i;
                break;
            case 'B':
                ++count_b;
                if (count_b == 1) {
                    first_b = i;
                }
                break;
            default:
                fail("unexpected tag in sequence");
        }
    }

    if (count_a != STEPS_PER_WORKER || count_b != STEPS_PER_WORKER) {
        fail("workers did not run the expected number of steps");
    }

    // Both workers yield after every step, so their execution must have
    // interleaved: worker B has to get a slot before worker A is done.
    if (first_b >= last_a) {
        fail("workers did not interleave");
    }

    printf("OK: sequence='%s'\n", sequence);
    exit(0);

    return 0;
}

int main(void)
{
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));
    return 2;
}
