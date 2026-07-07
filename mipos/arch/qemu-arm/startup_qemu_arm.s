/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 *
 * Reset handler for the QEMU ARM Cortex-M port: set up the stack pointer,
 * initialize .data and .bss, then jump to main().
 */

    .syntax unified
    .cpu cortex-m3
    .thumb

    .text

    .globl Reset_Handler
    .type Reset_Handler, %function
    .thumb_func
Reset_Handler:
    /* The core loads sp from the vector table, but reload it explicitly so
     * the handler also works when entered through a debugger reset. */
    ldr r0, =_estack
    mov sp, r0

    /* Copy .data from flash to RAM */
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
    /* Zero .bss */
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bhs 4f
    str r3, [r1], #4
    b 3b
4:
    bl main

    /* main() should never return */
5:
    b 5b

    .size Reset_Handler, . - Reset_Handler
