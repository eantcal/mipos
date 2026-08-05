# Programming Model

mipOS lets firmware be written as a set of sequential tasks instead of one large
superloop full of manually maintained state machines. The programmer writes the
activity as ordinary C control flow; the kernel preserves the task context when
that activity has to wait.

## From superloop to tasks

In a classic superloop, every activity must remember its own position:

```c
for (;;) {
    poll_uart_state_machine();
    poll_button_state_machine();
    poll_storage_state_machine();
    poll_network_state_machine();
}
```

That works while the product is tiny, but each feature becomes a manually
encoded state machine. In mipOS, the same activities can become independent
sequential tasks:

```c
static int console_task(task_param_t p)
{
    (void)p;

    for (;;) {
        read_command();
        execute_command();
        mipos_schedule();
    }
}
```

The important difference is not performance; it is shape. The call stack holds
where the task is, so the application code can keep reading like the operation
it represents.

## Cooperative tasks

A task is an ordinary C function running on its own application-provided stack.
It keeps local variables across waits because the BSP saves and restores the CPU
context and stack pointer when switching tasks.

A task gives other tasks a chance to run when it:

- calls `mipos_schedule`;
- calls a sleep/wait primitive such as `mipos_tm_wkafter` or `mipos_tm_msleep`;
- waits for a signal, queue item, mutex, or timer;
- returns from its entry point.

There is no preemptive time slicing between tasks. A task that computes for a
long time without yielding will delay the rest of the system, so long-running
work should be split with explicit scheduling points.

## Task creation

The core creation API is `mipos_t_create`. The caller supplies the task name,
entry function, parameter, stack memory, stack size, and flags.

```c
static char worker_stack[1024];

static int worker(task_param_t param)
{
    for (;;) {
        do_one_step(param);
        mipos_tm_msleep(10);
    }
}

mipos_t_create("worker",
               worker,
               user_context,
               worker_stack,
               sizeof(worker_stack),
               MIPOS_NO_RT);
```

The root task is started by `mipos_start`. Most examples follow the same basic
pattern: initialize board or simulator services, start mipOS with a root task,
then let the root task create the rest of the application.

## Yielding and interleaving

The scheduler smoke test in `tests/mipos_scheduler_smoke.c` demonstrates the
basic cooperative contract. The root task creates two workers. Each worker
appends its tag and then calls `mipos_schedule`.

```c
static int worker_task(task_param_t param)
{
    const char tag = (char)(mipos_reg_t)param;

    for (int i = 0; i < 3; ++i) {
        append_tag(tag);
        mipos_schedule();
    }

    return tag;
}
```

The expected sequence is interleaved, for example `ABABAB`, because every worker
yields after each step. If one task never yielded, the system would still be
cooperative, but responsiveness would be poor.

## Sleeping and timers

Time-based waits are expressed as blocking calls:

- `mipos_tm_wkafter(ticks)` yields until a scheduler tick count expires;
- `mipos_tm_msleep(ms)` sleeps for milliseconds;
- `mipos_tm_usleep(us)` sleeps for microseconds when the port supports the
  requested resolution;
- `mipos_get_rtc_counter` reads the RTC counter.

On bare-metal ports, the BSP feeds time into the kernel from a hardware timer,
SysTick, or an epoch hook. On the simulator, time comes from host facilities.

## Queues

Queues are fixed-size FIFO buffers backed by caller-provided storage. They are
useful for producer/consumer handoff without dynamic allocation.

```c
static mipos_queue_t queue;
static mipos_q_item_t queue_pool[8];

mipos_q_init(&queue, queue_pool, 8);
mipos_q_send(&queue, 42);

mipos_q_item_t item;
if (mipos_q_receive(&queue, &item, TRUE) == 0) {
    consume(item);
}
```

`mipos_q_receive` can block. RT-style polled tasks should avoid blocking mode
and use queue count or message-pending checks instead.

## Mutexes

The mutex implementation is intentionally small. A task waiting for a locked
mutex cooperatively yields until the mutex becomes available.

```c
static mipos_mtx_t lock = MU_INIT;

mipos_mu_lock(&lock);
update_shared_state();
mipos_mu_unlock(&lock);
```

Because mipOS is cooperative, task-shared state is stable until the current task
blocks or yields. Mutexes are still useful when an update spans calls that may
schedule, or when the ownership rule is clearer than relying on convention.

## Signals

Signals are the lightest notification mechanism. A task can suspend until a
signal mask is satisfied, while another task resumes or notifies it. The public
API exposes this through helpers such as `mipos_t_waitfor_signal`,
`mipos_t_resume`, and the lower-level signal notification path in the kernel.

Use signals for events, queues for data transfer, and mutexes for ownership.
That separation keeps embedded code easier to inspect.

## Real-time model

mipOS is soft real time. It is predictable when tasks cooperate, but it does not
guarantee hard deadlines if a task holds the CPU too long. Interrupt handlers
should stay short and defer work to tasks where possible.

The concurrency model is deliberately teachable:

- task-shared data changes only at explicit scheduling points;
- interrupt-shared data needs BSP critical sections;
- long computations should yield periodically;
- blocking calls are safe only where the task is allowed to suspend.

## Application-owned resources

The application decides stack sizes and storage pools. This is less automatic
than a large RTOS, but it is a good fit for small devices because resource use
is explicit and easy to audit in code and linker maps.

Typical application-owned resources include:

- root and worker task stacks;
- queue backing arrays;
- fixed-size memory pool arenas;
- malloc heap arena when enabled;
- disk images or block-driver context;
- network adapter context for lwIP experiments.

## Related pages

- [Architecture](Architecture)
- [Build and Test](Build-and-Test)
- [Porting Guide](Porting-Guide)