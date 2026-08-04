# Architecture

mipOS is built around a small cooperative kernel plus a narrow board support
package (BSP). The application remains close to the hardware, but it no longer
has to encode every activity as a manual state machine inside one global loop.

## Layering

```text
+-------------------------------------------------------------+
| Application tasks                                            |
| Domain logic: protocols, UI, sensors, storage, diagnostics    |
+-------------------------------------------------------------+
| Optional services                                            |
| Console, stdio glue, tiny filesystem, FatFs, lwIP             |
+-------------------------------------------------------------+
| mipOS kernel                                                 |
| Tasks, scheduler, timers, signals, mutexes, queues, memory    |
+-------------------------------------------------------------+
| BSP / architecture port                                      |
| Context save/restore, stack switch, critical sections, tick,  |
| UART and minimal platform services                           |
+-------------------------------------------------------------+
| Host process, QEMU machine, or physical MCU                   |
+-------------------------------------------------------------+
```

The layers are intentionally thin. The kernel does not know about every device
on the board. A port only has to supply the few primitives needed by the kernel
and by the optional console/timer services.

## Kernel responsibilities

The kernel owns scheduling policy and task state. It provides:

- task descriptors and cooperative dispatch;
- blocking waits and explicit scheduling points;
- software timers and sleep handling;
- synchronization primitives such as signals and mutexes;
- queues for producer/consumer style communication;
- optional memory helpers such as fixed pools and malloc support.

Because scheduling is cooperative, task-to-task concurrency is simple: a task
runs until it yields, sleeps, waits, or exits. Shared state between tasks does
not change unexpectedly in the middle of ordinary code. Interrupt-shared state
still needs BSP critical sections.

## BSP responsibilities

A BSP adapts mipOS to one CPU/toolchain/board combination. It supplies:

- `mipos_save_context` and `mipos_context_switch_to`;
- first-dispatch stack switching;
- critical-section enter/leave primitives;
- a system tick source or scheduler epoch hook;
- serial input/output for the console;
- minimal reset, clock, watchdog, and exception-vector hooks where needed.

This is the main portability boundary. The simulator ports use host facilities
and small assembly stubs; Cortex-M and Cortex-A QEMU ports use bare-metal
startup code and MMIO UARTs.

## Optional services

mipOS keeps optional services separate from the kernel core:

- the console provides a small command shell;
- stdio support connects formatted output to the BSP serial path;
- the tiny filesystem demonstrates compact in-memory file handling;
- FatFs support provides a more realistic FAT filesystem layer;
- lwIP integration is optional and currently focused on IPv4/Ethernet tests.

This lets very small targets build only the kernel features they need, while
larger simulator or QEMU configurations can enable richer diagnostics.

## Memory ownership

A central design choice is that the application owns most memory. Task stacks,
queue storage, memory-pool backing arrays, and heap arenas are supplied by the
application rather than allocated implicitly by the kernel. This keeps mipOS
predictable on small devices where every byte matters.