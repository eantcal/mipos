/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * Licensed under the MIT License.
 *
 * Linker script for QEMU vexpress-a9:
 *   RAM: 1 GiB @ 0x60000000
 */

ENTRY(Reset_Handler)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x60000000, LENGTH = 128M
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
_irq_stack_top = _estack;
_irq_stack_size = 0x1000;
_svc_stack_top = _irq_stack_top - _irq_stack_size;
_svc_stack_size = 0x4000;
_abt_stack_top = _svc_stack_top - _svc_stack_size;
_abt_stack_size = 0x1000;
_und_stack_top = _abt_stack_top - _abt_stack_size;
_und_stack_size = 0x1000;

SECTIONS
{
    .vectors :
    {
        . = ALIGN(32);
        __vectors_start = .;
        KEEP(*(.vectors))
        . = ALIGN(32);
        __vectors_end = .;
    } > RAM

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > RAM

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RAM

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > RAM

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    PROVIDE(end = _ebss);
    PROVIDE(_end = _ebss);

    /DISCARD/ :
    {
        *(.init) *(.fini)
        *(.init_array*) *(.fini_array*)
    }
}
