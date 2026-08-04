/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 *
 * Reset handler and exception vectors for QEMU vexpress-a9 (Cortex-A9).
 */

    .syntax unified
    .cpu cortex-a9
    .arm

    .section .vectors, "ax"
    .align 5
    .global __vectors_start
__vectors_start:
    ldr pc, =Reset_Handler
    ldr pc, =Undefined_Handler
    ldr pc, =SVC_Handler
    ldr pc, =PrefetchAbort_Handler
    ldr pc, =DataAbort_Handler
    ldr pc, =Default_Handler
    ldr pc, =IRQ_Handler
    ldr pc, =FIQ_Handler

    .text
    .align 2

    .global Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    cpsid if

    cps #0x12
    ldr sp, =_irq_stack_top

    cps #0x17
    ldr sp, =_abt_stack_top

    cps #0x1b
    ldr sp, =_und_stack_top

    cps #0x13
    ldr sp, =_svc_stack_top

    ldr r0, =__vectors_start
    mcr p15, 0, r0, c12, c0, 0
    dsb
    isb

    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bhs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    mov r3, #0
3:
    cmp r1, r2
    bhs 4f
    str r3, [r1], #4
    b 3b
4:
    bl main
5:
    b 5b

    .size Reset_Handler, . - Reset_Handler

    .global IRQ_Handler
    .type IRQ_Handler, %function
IRQ_Handler:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    bl mipos_bsp_qemu_vexpress_a9_irq
    ldmfd sp!, {r0-r12, pc}^

    .size IRQ_Handler, . - IRQ_Handler

    .global Default_Handler
Default_Handler:
Undefined_Handler:
SVC_Handler:
PrefetchAbort_Handler:
DataAbort_Handler:
FIQ_Handler:
    b .
