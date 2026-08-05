# Architecture

mipOS is built around a small cooperative kernel plus a narrow board support
package (BSP). The application remains close to the hardware, but it no longer
has to encode every activity as a manual state machine inside one global loop.

![mipOS architecture diagram](https://raw.githubusercontent.com/eantcal/mipos/main/mipos_pictures/miposarc.png)

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
on the board. A port only supplies the primitives needed by the scheduler,
critical sections, the tick path, and the optional console/timer services.

## Source map

The current source tree mirrors this architecture:

| Area | Main files | Role |
|---|---|---|
| Kernel | `mipos_kernel.c`, `mipos_kernel.h`, `mipos_task.c`, `mipos_timer.c` | task table, cooperative dispatcher, waits, signals, software timers |
| Queues and sync | `mipos_queue.c`, `mipos_queue.h`, `mipos_mutex.h` | producer/consumer communication and cooperative exclusion |
| Memory | `mipos_malloc.c`, `mipos_mpool.c` | heap-style allocation and fixed-size block pools |
| Console | `mipos_console.c`, `mipos_console.h` | serial shell, command dispatch, TX queue, optional diagnostics |
| Filesystem bridge | `mipos_tfs.c`, `mipos_diskio.c`, FatFs integration | tiny filesystem and block-device abstraction for FAT storage |
| Networking | `mipos/net/lwip/mipos_lwip_netif.c` | lwIP `netif` adapter, Ethernet frame input/output boundary |
| BSP selection | `mipos_bsp.h`, `mipos/arch/*` | context switch, stack switch, tick, UART, critical sections |
| QEMU firmware | `examples/qemu-arm/*`, `examples/qemu-vexpress-a9/*` | bare-metal examples and board smoke tests |

This mapping is useful when porting: most new-board work should land under a
new `mipos/arch/<target>` directory plus a focused example under `examples/`.
The kernel core should not need board-specific conditionals for every device.

## Kernel responsibilities

The kernel owns scheduling policy and task state. It provides:

- task descriptors and cooperative dispatch;
- blocking waits and explicit scheduling points;
- software timers and sleep handling;
- signal wait/notify plumbing;
- queues for producer/consumer style communication;
- optional memory helpers such as fixed pools and malloc support.

The public task API exposes this model directly. `mipos_t_create` starts a task
using an application-supplied stack; `mipos_schedule` is an explicit yield;
`mipos_tm_wkafter`, `mipos_tm_msleep`, and `mipos_tm_usleep` suspend the current
task until a scheduler or RTC event wakes it.

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

The simulator BSP uses host facilities and small assembly stubs. The QEMU
Cortex-M port uses bare-metal startup, SysTick, and UART MMIO. The QEMU
`vexpress-a9` port now covers the first ARMv7-A step: startup, PL011 UART,
critical sections, and a root task; GIC/timer integration is the next layer.

## Optional services

mipOS keeps optional services separate from the kernel core:

- the console provides a small command shell and command registration API;
- stdio support connects formatted output to the BSP serial path;
- the tiny filesystem demonstrates compact in-memory file handling;
- FatFs support uses `mipos_diskio` as the block-device bridge;
- lwIP integration is optional and currently focused on IPv4/Ethernet tests.

This lets very small targets build only the kernel features they need, while
larger simulator or QEMU configurations can enable richer diagnostics.

## Memory ownership

A central design choice is that the application owns most memory. Task stacks,
queue storage, memory-pool backing arrays, and heap arenas are supplied by the
application rather than allocated implicitly by the kernel. This keeps mipOS
predictable on small devices where every byte matters.

## Runtime flow

A typical firmware follows this sequence:

1. BSP startup initializes CPU state, memory sections, stacks, and serial I/O.
2. The application calls `mipos_start` with a root task and stack.
3. The root task creates other tasks and optional services.
4. Tasks run until they yield, sleep, wait for a signal, or terminate.
5. The BSP tick or epoch hook feeds timer progress back into the kernel.
6. Optional console, filesystem, or network code runs as ordinary cooperative
   tasks or as polled service logic.

That flow is the core portability contract. A new board port is successful when
it can print before the kernel starts, print again from a root task, sleep on a
timer, then run the console without disturbing scheduling.

## Related pages

- [Programming Model](Programming-Model)
- [Supported Targets](Supported-Targets)
- [Porting Guide](Porting-Guide)