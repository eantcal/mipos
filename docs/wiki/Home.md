# mipOS Wiki

mipOS, short for Multum In Parvo Operating System, is a compact cooperative
multitasking kernel for small embedded systems. It was born from the practical
problem of keeping firmware readable when a simple superloop starts growing
into many unrelated state machines.

The kernel is intentionally small. It does not try to be a general-purpose OS,
and it does not hide the hardware behind a large driver framework. Instead, it
provides the minimum reusable infrastructure that embedded applications tend to
need again and again: tasks, scheduling, waits, signals, queues, mutexes,
software timers, memory pools, malloc support, a small console, and optional
filesystem and networking components.

## What mipOS is for

Use mipOS when firmware has several independent activities that are easier to
write as sequential flows than as hand-written state machines. Typical examples
are serial command handlers, protocol tasks, periodic housekeeping, UI polling,
sensor sampling, storage maintenance, and network diagnostics.

mipOS is especially useful when:

- the target has limited RAM and flash;
- cooperative scheduling is acceptable;
- the application can own task stacks and memory pools explicitly;
- portability and readability matter more than adding a large RTOS dependency;
- host simulation and QEMU bring-up are useful during development.

mipOS is not meant to provide memory protection, preemptive time slicing,
process isolation, POSIX sockets, or a complete hardware abstraction layer.
Those omissions are deliberate: they keep the kernel small enough to study,
port, and reason about.

## Architecture at a glance

```text
Application tasks
  |
Optional services: console, stdio, tiny filesystem, FatFs, lwIP
  |
mipOS kernel: scheduler, tasks, timers, signals, mutexes, queues
  |
BSP/port layer: context switch, stacks, critical sections, tick, UART
  |
Host simulator, QEMU board, or physical microcontroller
```

The key design rule is simple: application code owns memory, while the kernel
coordinates execution. Task stacks, queue pools, and memory arenas are supplied
by the application, which keeps RAM usage visible and auditable.

## Current capabilities

- Cooperative task scheduling with explicit yields and blocking waits.
- Task creation on caller-provided stacks.
- Software timers and millisecond sleeps.
- Signals, mutexes, queues, memory pools, and malloc/realloc support.
- Optional console and command shell.
- Optional tiny filesystem and FatFs integration.
- Optional lwIP-based IPv4 networking experiments.
- Host simulator builds for Windows, Linux, and macOS.
- Bare-metal QEMU ARM firmware for Cortex-M and early Cortex-A work.

## Pages

- [Architecture](Architecture.md)
- [Programming Model](Programming-Model.md)
- [Supported Targets](Supported-Targets.md)
- [Build and Test](Build-and-Test.md)
- [QEMU ARM Targets](QEMU-ARM-Targets.md)
- [Networking](Networking.md)
- [Porting Guide](Porting-Guide.md)

## Repository documentation

The canonical in-repository documentation lives in:

- `README.md` for build and usage instructions.
- `docs/mipos-internals.md` for the detailed kernel and BSP internals.
- `NEWS.md` for release-oriented changes.