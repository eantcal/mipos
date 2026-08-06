# Programming Model

mipOS lets firmware be written as a set of sequential tasks instead of one large
superloop full of manually maintained state machines. The programmer writes the
activity as ordinary C control flow; the kernel preserves the task context when
that activity has to wait.

This page is grounded in the current implementation under `mipos/` and the
scheduler smoke test under `tests/`. Function names and snippets intentionally
use the source-level API rather than pseudo-RTOS terminology.

## From Superloop To Tasks

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

## Cooperative Tasks

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

## Task Creation

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

The implementation in `mipos/mipos_task.c` shows the intended ownership model:
the kernel finds a free descriptor, records the entry point, then stores the
caller-provided stack pointer and size.

```c
if (!(p_task->entry_point)) {
    _mipos_t_create(name, p_task, entry_point, param, flags);

    p_task->task_stack = task_stack;
    p_task->stack_size = stack_size;

    mipos_leave_cs();
    return id;
}
```

The root task is started by `mipos_start`. Most examples follow the same basic
pattern: initialize board or simulator services, start mipOS with a root task,
then let the root task create the rest of the application.

```c
static char root_stack[64 * 1024];

int main(void)
{
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));
    return 2;
}
```

## Scheduler Lifecycle

`mipos_start` is the kernel entry point. Its startup sequence is visible in
`mipos/mipos_kernel.c`:

```c
scheduler_init();
mipos_bsp_setup_reset_and_wd();
mipos_bsp_setup_clk();
mipos_bsp_exception_vector();
mipos_bsp_create_hw_rtc_timer();
mipos_bsp_enable_irq_mask();

mipos_t_create(
  "root", root_task, root_task_param, task_stack, stack_size, MIPOS_NO_RT);
```

After that, execution stays inside the scheduler loop. The scheduler repeatedly
walks the fixed task table, skips invalid or non-runnable descriptors, services
software timers, and dispatches the next ready non-real-time task.

The task table is static and bounded. That is deliberate: small targets should
make task capacity visible at compile time instead of discovering memory pressure
through dynamic allocation.

## First Dispatch And Resume

The dispatcher has two paths:

- first run: switch to the application-provided task stack and call the entry
  point;
- resume: restore the saved task register state after a previous wait/yield.

The first-dispatch path in `mipos/mipos_kernel.c` computes the top of the stack
from the caller's buffer, adjusts alignment on 64-bit simulator builds, then
uses the BSP stack-switch primitive:

```c
top_of_the_stack = (void*)(&p_task->task_stack[p_task->stack_size]);
top_of_the_stack =
  (void*)((mipos_reg_t)top_of_the_stack - sizeof(void*));

mipos_replace_sp(saved_stack_pointer, top_of_the_stack);

p_task->task_ret_value = p_task->entry_point(p_task->param);
```

On 64-bit simulator builds this is routed through a dedicated helper instead of
using the older direct stack macro. That split exists because x64 ABIs have more
strict stack alignment and callee-saved register rules than small 8/32-bit MCU
ports.

## Yielding And Interleaving

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

The test makes that behavior explicit:

```c
if (first_b >= last_a) {
    fail("workers did not interleave");
}
```

This is the useful mental model for application code: a task owns the CPU until
it reaches a kernel scheduling point.

## Suspension And Context Save

Most blocking operations eventually use the same internal pattern. The macro
`_mipos_t_suspend` in `mipos/mipos_kernel.h` saves the current task context,
marks the task as waiting for a signal, stores the process stack pointer, and
jumps back to the scheduler context.

```c
if (mipos_save_context(mipos_kernel_env.task_context_ptr->reg_state)) {
    _mipos_t_reset_signal_and_run(_SIGNUM);
} else {
    mipos_kernel_env.task_context_ptr->process_stack_pointer =
      (mipos_reg_t)mipos_get_sp();
    _mipos_t_set_wait_for_signal(_SIGNUM);
    mipos_context_switch_to(mipos_kernel_env.scheduler_registers_state);
}
```

The exact mechanics behind `mipos_save_context`, `mipos_get_sp`, and
`mipos_context_switch_to` belong to the BSP. For example, the simulator may use
`setjmp`/`longjmp` plus small assembly helpers, while STM8 and QEMU ARM provide
architecture-specific stack and interrupt primitives.

## Sleeping And Timers

Time-based waits are expressed as blocking calls:

- `mipos_tm_wkafter(ticks)` yields until a scheduler tick count expires;
- `mipos_tm_msleep(ms)` sleeps for milliseconds;
- `mipos_tm_usleep(us)` sleeps for microseconds when the port supports the
  requested resolution;
- `mipos_get_rtc_counter` reads the RTC counter.

The RTC path is intentionally small. The BSP periodically calls
`mipos_update_rtc`, and the scheduler wakes tasks whose `rtc_timeout` has
expired.

```c
void mipos_update_rtc(uint32_t quantum)
{
    mipos_kernel_env.rtc_counter += quantum;
    mipos_kick_watchdog();
}
```

Bare-metal ports feed this from a hardware timer, SysTick, or an epoch hook. The
STM8 port calls it from the TIM4 interrupt; the QEMU ARM ports call it from the
emulated timer path. On the simulator, time comes from host facilities.

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

`mipos/mipos_queue.c` shows the storage model directly:

```c
queue->head = 0;
queue->tail = 0;
queue->item = queue_pool;
queue->queue_limit = queue_pool_length;
queue->queue_count = 0;
```

A blocking receive suspends the current task on `MIPOS_SIGQUE` when the queue is
empty. A send wakes the suspended task by notifying the same signal.

```c
if (queue->queue_count == 0 && queue->suspended_tid == 0) {
    queue->suspended_tid = mipos_kernel_env.current_task;

    if (bl) {
        _mipos_t_suspend(MIPOS_SIGQUE);
    }
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

The implementation in `mipos/mipos_mutex.h` is a small macro rather than a
separate scheduler object:

```c
while (1) {
    if ((_MTX)->state == MUTEX_UNLOCKED) {
        break;
    }
    mipos_tm_wkafter(0);
}
(_MTX)->state |= MUTEX_LOCKED;
```

Because mipOS is cooperative, task-shared state is stable until the current task
blocks or yields. Mutexes are still useful when an update spans calls that may
schedule, or when the ownership rule is clearer than relying on convention.

## Signals

Signals are the lightest notification mechanism. A task can suspend until a
signal mask is satisfied, while another task resumes or notifies it. The public
API exposes this through helpers such as `mipos_t_waitfor_signal`,
`mipos_t_resume`, and the lower-level signal notification path in the kernel.

Signals are also used internally:

- `MIPOS_SIGWKP` resumes a suspended task;
- `MIPOS_SIGQUE` wakes queue receivers;
- `MIPOS_SIGALM` supports scheduler-tick waits;
- `MIPOS_SIGTMR` supports RTC-based sleeps.

Use signals for events, queues for data transfer, and mutexes for ownership.
That separation keeps embedded code easier to inspect.

## Real-Time Model

mipOS is soft real time. It is predictable when tasks cooperate, but it does not
guarantee hard deadlines if a task holds the CPU too long. Interrupt handlers
should stay short and defer work to tasks where possible.

The concurrency model is deliberately simple:

- task-shared data changes only at explicit scheduling points;
- interrupt-shared data needs BSP critical sections;
- long computations should yield periodically;
- blocking calls are safe only where the task is allowed to suspend.

The kernel has a small real-time task path, but the current examples and tests
primarily exercise cooperative non-real-time tasks. Treat the non-real-time path
as the documented default unless a port or application explicitly uses
`MIPOS_RT`.

## Application-Owned Resources

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

A useful review rule is: if an object can grow, the owning application or
example should make its storage visible in source. That is why examples declare
root stacks, queue pools, filesystem buffers, and network contexts explicitly.

## Related Pages

- [Architecture](Architecture)
- [Build and Test](Build-and-Test)
- [Porting Guide](Porting-Guide)