# Programming Model

mipOS lets firmware be written as a set of sequential tasks instead of one
large superloop full of manually maintained state machines.

## Cooperative tasks

A task is an ordinary C function running on its own stack. It keeps local
variables across waits because the kernel saves and restores the CPU context
and stack pointer when switching tasks.

A task gives other tasks a chance to run when it:

- calls a sleep or wait primitive;
- waits for a signal, mutex, queue item, or timer;
- explicitly schedules/yields;
- exits.

There is no preemptive time slicing between tasks. A task that computes for a
long time without yielding will delay the rest of the system, so long-running
work should be split with explicit scheduling points.

## Why this helps

Without a kernel, firmware often becomes a superloop where every feature has to
remember its own progress in explicit state variables. With mipOS, the natural
control flow stays visible:

```text
read command
wait for input
parse command
perform action
wait for next command
```

The call stack becomes the state machine. This usually makes protocol handlers,
console commands, periodic tasks, and storage workflows easier to read and test.

## Synchronization

mipOS provides small primitives that match common embedded patterns:

- mutexes for cooperative mutual exclusion;
- queues for producer/consumer transfer;
- signals for event notification;
- timers and sleeps for time-based waits;
- memory pools for fixed-size allocation without heap fragmentation.

Since tasks are cooperative, ordinary task-shared updates are stable until the
current task blocks or yields. Data shared with interrupt handlers is different:
it must be protected with the BSP critical-section macros or designed as an
interrupt-safe handoff.

## Real-time model

mipOS is soft real time. It is predictable when tasks cooperate, but it does not
guarantee hard deadlines if a task holds the CPU too long. Interrupt handlers
should stay short and defer work to tasks where possible.

## Application-owned resources

The application decides stack sizes and storage pools. This is less automatic
than a large RTOS, but it is a good fit for small devices because resource use
is explicit and easy to audit in code and linker maps.