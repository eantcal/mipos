# Porting Guide

A mipOS port is intentionally small. The goal is to provide the kernel with the
few primitives it needs, then bring up optional services one at a time. This page
uses the current BSP implementations as reference material: host simulator,
QEMU Cortex-M, QEMU `vexpress-a9`, and STM8/SDCC.

## Minimal Bring-Up Order

1. Startup code: stack, `.data`, `.bss`, vector table, and a call into `main`.
2. UART `putc`: once text works, debugging becomes much easier.
3. Critical sections: save interrupt state, disable interrupts, restore state.
4. Context switch: `setjmp`/`longjmp` or equivalent save/restore primitive.
5. First task dispatch on an application-provided stack.
6. Tick source or scheduler epoch hook feeding `mipos_update_rtc`.
7. Console RX path, watchdog, reset/clock hooks, and optional device drivers.

Do not bring up storage or networking before the first four items are stable.
Most difficult failures in a new port look like filesystem/network bugs but are
actually stack alignment, critical-section, or context-save bugs.

## BSP Selection

`mipos/mipos_bsp.h` is the selection boundary. New targets should add one BSP
include path and keep board-specific code out of the kernel core.

```c
#if MIPOS_TARGET_QEMU_ARM
#include "mipos_bsp_qemu_arm.h"
#elif MIPOS_TARGET_QEMU_VEXPRESS_A9
#include "mipos_bsp_qemu_vexpress_a9.h"
#elif MIPOS_TARGET_STM32
#include "mipos_bsp_stm32.h"
#elif MIPOS_TARGET_STM8
#include "mipos_bsp_stm8.h"
#else
#include "mipos_bsp_simu.h"
#endif
```

A new port normally needs:

- a `mipos/arch/<target>/mipos_bsp_<target>.h` contract header;
- a `mipos/arch/<target>/mipos_bsp_<target>.c` implementation file;
- startup code if the target is bare metal;
- a linker script if the target is not a host process;
- one minimal example under `examples/<target>/`.

## BSP Contract

A port usually supplies:

- context save/restore macros;
- stack switching helpers;
- critical-section macros;
- serial configure, putc, and non-blocking receive;
- timer creation or scheduler epoch notification;
- optional watchdog, reset, clock, and exception-vector hooks.

The exact implementation depends on the CPU and toolchain. Host simulator ports
can rely on libc and small assembly files. Bare-metal ARM ports need startup
assembly, a linker script, and MMIO access to the board UART/timer. STM8 uses
compiler-specific inline assembly and the local STM8S peripheral headers.

## Critical Sections

Critical sections must preserve the previous interrupt mask, not merely disable
and re-enable interrupts blindly. That matters when kernel code nests critical
sections or when a caller entered the kernel with interrupts already disabled.

The QEMU Cortex-M BSP saves PRIMASK and restores it on leave:

```c
static inline unsigned int _mipos_qemu_arm_irq_save(void)
{
    unsigned int primask;
    __asm__ __volatile__("mrs %0, primask\n\t"
                         "cpsid i\n\t"
                         : "=r"(primask)
                         :
                         : "memory");
    return primask;
}

#define mipos_enter_cs()                                                       \
    do {                                                                       \
        _mipos_cs_primask = _mipos_qemu_arm_irq_save();                        \
    } while (0)
```

The `vexpress-a9` port does the same job with CPSR:

```c
__asm__ __volatile__("mrs %0, cpsr\n\t"
                     "cpsid i\n\t"
                     : "=r"(cpsr)
                     :
                     : "memory");
```

The host simulator has empty critical sections because it is a cooperative host
process in the supported examples. That is valid for simulator tests, but it is
not a model for bare-metal interrupt handling.

```c
#define mipos_init_cs()
#define mipos_enter_cs()
#define mipos_leave_cs()
```

## Context Save And Resume

The scheduler expects two primitives:

- `mipos_save_context(buffer)` returns `0` when saving directly and non-zero
  when resumed;
- `mipos_context_switch_to(buffer)` resumes a previously saved context.

Most ports implement this with `setjmp`/`longjmp` where the C library or runtime
saves the right callee-saved registers for the ABI.

```c
#define mipos_save_context(_x) setjmp((void*)(_x))
#define mipos_context_switch_to(_x) longjmp((void*)(_x), 1)
```

This is only correct if the target runtime's `setjmp` saves the registers and
stack pointer required by the ABI. If that is not true, the port needs a custom
assembly pair, as older Cortex-M support did under `mipos/arch/ctxm3/`.

## Stack Switching

First dispatch is different from resume. The kernel has no saved task context
yet, so it switches to the task's application-provided stack and calls the entry
point. A BSP must provide `mipos_replace_sp(old_sp, new_sp)` and `mipos_set_sp`.

The QEMU Cortex-M implementation stores the current stack pointer, aligns the
new one to 8 bytes, then updates `sp` in one assembly block:

```c
#define mipos_replace_sp(__OLD_SP, __STACK_P)                                  \
    do {                                                                       \
        uint32_t __mipos_tmp;                                                  \
        __asm__ __volatile__("mov %0, sp\n\t"                                  \
                             "str %0, [%1]\n\t"                                \
                             "bic %0, %2, #7\n\t"                              \
                             "mov sp, %0\n\t"                                  \
                             : "=&r"(__mipos_tmp)                              \
                             : "r"(__OLD_SP), "r"(__STACK_P)                   \
                             : "memory");                                      \
    } while (0)
```

STM8/SDCC uses the same contract with STM8 instructions:

```c
#define mipos_replace_sp(__OLD_SP, __N_SP)                                     \
    __asm__("ldw x, sp\n"                                                     \
            "ldw _" #__OLD_SP "+0, x\n"                                      \
            "ldw x, _" #__N_SP "+0\n"                                       \
            "ldw sp, x\n")
```

On 64-bit host simulator builds the repo uses helper functions such as
`mipos_start_task64` or `mipos_start_task_gnu`. The reason is ABI compliance:
x64 stack alignment and callee-saved registers are stricter than the historical
32-bit direct stack switch path.

## Tick And RTC Feed

The kernel only needs monotonically increasing time. The BSP decides whether it
comes from an interrupt, a polled scheduler epoch hook, or host clock readings.

The required kernel call is small:

```c
void mipos_update_rtc(uint32_t quantum)
{
    mipos_kernel_env.rtc_counter += quantum;
    mipos_kick_watchdog();
}
```

The QEMU Cortex-M BSP accumulates SysTick interrupts and drains them from the
scheduler epoch hook, so kernel data structures are touched from thread context:

```c
void SysTick_Handler(void)
{
    ++systick_pending_ticks;
}

void mipos_bsp_qemu_arm_rtc_epoch(void)
{
    uint32_t ticks;

    mipos_enter_cs();
    ticks = systick_pending_ticks;
    systick_pending_ticks = 0;
    mipos_leave_cs();

    if (ticks) {
        mipos_update_rtc(ticks);
    }
}
```

The STM8 BSP feeds RTC directly from the TIM4 interrupt:

```c
INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
{
    mipos_update_rtc(1 /*ms*/);
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}
```

The simulator uses host time in `mipos_bsp_notify_scheduler_epoch`. On Linux it
uses `gettimeofday`, computes elapsed milliseconds, and calls `mipos_update_rtc`.

## UART Contract

The console and debug output need three functions:

```c
void mipos_bsp_configure_rs232(unsigned int baud_rate);
unsigned int mipos_bsp_rs232_recv_byte(unsigned char* key);
void mipos_bsp_rs232_putc(unsigned char c);
```

The QEMU Cortex-M UART implementation is typical for MMIO boards:

```c
void mipos_bsp_rs232_putc(unsigned char c)
{
    while (UART0_FR & UART_FR_TXFF) {
    }

    UART0_DR = c;
}
```

Non-blocking receive returns `0` if no byte is available and `1` when it stores a
byte. Ports should normalize terminal line endings if their console backend can
produce LF-only input:

```c
if (c == '\n') {
    c = '\r';
}

*key = c;
return 1;
```

## Watchdog, Reset, Clock, And Endianness

For early simulator and QEMU targets, reset/watchdog hooks can be no-ops. For a
physical board they should reflect the real hardware. STM8, for example, maps
watchdog kicking to the independent watchdog reload call:

```c
#define mipos_kick_watchdog() IWDG_ReloadCounter()
```

Each BSP also defines endian helpers used by filesystem and protocol code:

```c
#define MIPOS_LE_U16(x) (x)
#define MIPOS_LE_U32(x) (x)
```

Those identity mappings are correct on the little-endian targets currently in
the tree. A big-endian target must implement explicit byte swapping.

## Recommended Validation

Start with a firmware that prints a banner before the kernel starts. Then start
a root task and print again from task context. After that, add a timer and a
sleeping task. Only add storage or networking once scheduling and time are
reliable.

Good early checks:

- banner before `mipos_start`;
- root task banner after scheduler startup;
- repeated timer ticks;
- two tasks yielding in a predictable sequence;
- console RX/TX;
- filesystem or network smoke test.

The host-side scheduler smoke test is the model for the two-task check. It
creates two workers, each worker yields, and the root task verifies that the
sequence interleaves before exiting.

## QEMU-First Strategy

QEMU targets make porting safer because startup, stack switching, and interrupt
handling can be tested without physical hardware. In this repository:

- `lm3s6965evb` is the mature Cortex-M3 teaching/workhorse target;
- `vexpress-a9` is the ARMv7-A didactic board bring-up path;
- `virt` is the future CI target for virtio block and network work.

Once a QEMU target is stable, the same BSP checklist can be applied to a real
evaluation board with similar CPU and peripherals.

## Related Pages

- [Architecture](Architecture)
- [Programming Model](Programming-Model)
- [Supported Targets](Supported-Targets)
- [QEMU ARM Targets](QEMU-ARM-Targets)