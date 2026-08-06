# mipOS Wiki

![mipOS logo](https://raw.githubusercontent.com/eantcal/mipos/main/mipos_pictures/mipos.png)

mipOS, short for Multum In Parvo Operating System, is a compact cooperative
multitasking kernel for small embedded systems. It was created to solve a very
practical firmware problem: once a simple superloop grows into many unrelated
state machines, the application becomes harder to read than the hardware it is
controlling.

mipOS keeps the kernel deliberately small. It does not try to be a general
purpose operating system, and it does not hide the board behind a large driver
framework. Instead, it provides the reusable mechanisms that small embedded
applications need again and again: tasks, cooperative scheduling, blocking
waits, signals, queues, mutexes, software timers, memory pools, malloc support,
a serial console, and optional filesystem and networking components.

![mipOS architecture diagram](https://raw.githubusercontent.com/eantcal/mipos/main/mipos_pictures/miposarc.png)

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
- host simulation and QEMU bring-up are useful during development;
- the same code should be exercised on a desktop before it reaches a board.

It is not meant to provide memory protection, preemptive time slicing, process
isolation, POSIX sockets, or a complete hardware abstraction layer. Those
omissions are deliberate. The result is a kernel small enough to study, port,
and reason about without losing sight of the application.

## How the pieces fit

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

The public include `mipos/mipos.h` reflects this shape directly. It always
includes the kernel API, then conditionally exposes optional modules such as
`mipos_console`, `mipos_tfs`, `mipos_stdio`, `mipos_malloc`, and `mipos_mpool`
when the corresponding `ENABLE_MIPOS_*` switch is active.

The key design rule is simple: application code owns memory, while the kernel
coordinates execution. Task stacks, queue pools, memory pools, and heap arenas
are supplied by the application. That makes RAM use visible in source code and
linker maps, which matters on small devices.

## Current capabilities

- Cooperative task scheduling with explicit yields and blocking waits.
- Task creation on caller-provided stacks through `mipos_t_create`.
- Software timers, `mipos_tm_wkafter`, `mipos_tm_msleep`, and RTC accounting.
- Signals, mutexes, queues, fixed memory pools, and malloc/realloc support.
- Optional serial console and command registration.
- Optional tiny filesystem and FatFs disk-driver bridge.
- Optional lwIP-based IPv4 networking experiments.
- Host simulator builds for Windows, Linux, and macOS.
- Bare-metal QEMU ARM firmware for Cortex-M and early Cortex-A work.
- Optional STM8 firmware builds with SDCC.

![mipOS running on an evaluation board](https://raw.githubusercontent.com/eantcal/mipos/main/mipos_pictures/mipos2.jpg)

## Development style

mipOS is intentionally friendly to iterative bring-up:

1. Run logic in the host simulator.
2. Validate kernel primitives with unit tests and scheduler smoke tests.
3. Move the same kernel contracts to QEMU firmware.
4. Bring up board-specific BSP pieces one at a time.
5. Add storage, networking, and console services only after the scheduler and
   serial path are boringly reliable.

This is why the repository now keeps simulator builds, QEMU `lm3s6965evb`, QEMU
`vexpress-a9`, and a planned QEMU `virt` roadmap side by side.

## Pages

- [Architecture](Architecture)
- [Programming Model](Programming-Model)
- [Supported Targets](Supported-Targets)
- [Build and Test](Build-and-Test)
- [QEMU ARM Targets](QEMU-ARM-Targets)
- [STM8 SDCC](STM8-SDCC)
- [Networking](Networking)
- [Porting Guide](Porting-Guide)

## Repository documentation

The canonical in-repository documentation lives in:

- `README.md` for build and usage instructions.
- `docs/mipos-internals.md` for the detailed kernel and BSP internals.
- `NEWS.md` for release-oriented changes.