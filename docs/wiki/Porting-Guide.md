# Porting Guide

A mipOS port is intentionally small. The goal is to provide the kernel with the
few primitives it needs, then bring up optional services one at a time.

## Minimal bring-up order

1. Startup code: stack, `.data`, `.bss`, vector table, and a call into `main`.
2. UART `putc`: once text works, debugging becomes much easier.
3. Critical sections: save interrupt state, disable interrupts, restore state.
4. Context switch: `setjmp`/`longjmp` or equivalent save/restore primitive.
5. First task dispatch on an application-provided stack.
6. Tick source or scheduler epoch hook feeding `mipos_update_rtc`.
7. Console RX path, watchdog, reset/clock hooks, and optional device drivers.

## BSP contract

A port usually supplies:

- context save/restore macros;
- stack switching helpers;
- critical-section macros;
- serial configure, putc, and non-blocking receive;
- timer creation or scheduler epoch notification;
- optional watchdog, reset, clock, and exception-vector hooks.

The exact implementation depends on the CPU and toolchain. Host simulator ports
can rely on libc and small assembly files. Bare-metal ARM ports need startup
assembly, a linker script, and MMIO access to the board UART/timer.

## Recommended validation

Start with a firmware that prints a banner before the kernel starts. Then start
a root task and print again from task context. After that, add a timer and a
sleeping task. Only add storage or networking once scheduling and time are
boringly reliable.

Good early checks:

- banner before `mipos_start`;
- root task banner after scheduler startup;
- repeated timer ticks;
- two tasks yielding in a predictable sequence;
- console RX/TX;
- filesystem or network smoke test.

## QEMU-first strategy

QEMU targets make porting safer because startup, stack switching, and interrupt
handling can be tested without physical hardware. In this repository:

- `lm3s6965evb` is the mature Cortex-M3 teaching/workhorse target;
- `vexpress-a9` is the ARMv7-A didactic board bring-up path;
- `virt` is the future CI target for virtio block and network work.

Once a QEMU target is stable, the same BSP checklist can be applied to a real
evaluation board with similar CPU and peripherals.