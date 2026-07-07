#if defined(__APPLE__)
#define MIPOS_SYM(name) _##name
#else
#define MIPOS_SYM(name) name
#endif

#if defined(__x86_64__)

    .text

    .globl MIPOS_SYM(mipos_get_sp)
MIPOS_SYM(mipos_get_sp):
    leaq 8(%rsp), %rax
    ret

    .globl MIPOS_SYM(mipos_start_task_gnu)
MIPOS_SYM(mipos_start_task_gnu):
    pushq %rbx
    movq %rdi, %rbx
    movq %rsp, (%rbx)

    movq %rsi, %rsp
    andq $-16, %rsp

    movq %rcx, %rdi
    call *%rdx

    movq (%rbx), %rsp
    popq %rbx
    ret

#elif defined(__aarch64__)

// AAPCS64: x0 = old_sp (out), x1 = new_sp, x2 = entry point, x3 = parameter.
// x19 is callee-saved and survives the entry-point call; the task stack
// pointer must stay 16-byte aligned at all times.

    .text
    .p2align 2

    .globl MIPOS_SYM(mipos_get_sp)
MIPOS_SYM(mipos_get_sp):
    mov x0, sp
    ret

    .globl MIPOS_SYM(mipos_start_task_gnu)
MIPOS_SYM(mipos_start_task_gnu):
    stp x29, x30, [sp, #-32]!
    str x19, [sp, #16]
    mov x29, sp

    mov x19, x0
    mov x9, sp
    str x9, [x19]

    and x9, x1, #-16
    mov sp, x9

    mov x0, x3
    blr x2

    ldr x9, [x19]
    mov sp, x9

    ldr x19, [sp, #16]
    ldp x29, x30, [sp], #32
    ret

#endif
