; bspX64.asm
; Adjusted for Microsoft x64 calling convention.

.CODE
; Function to get the current stack pointer (rsp)
mipos_get_sp PROC
    mov rax, rsp  ; Move the value of RSP of caller into RAX
    add rax, 8
    ret           ; Return, RAX will hold the return value
mipos_get_sp ENDP

; Calls a task entry point using a newly supplied stack.
; Parameters:
; rcx = old_sp (address where the scheduler stack pointer will be stored)
; rdx = new_sp (top of the task stack)
; r8  = task entry point
; r9  = task parameter
mipos_start_task64 PROC
    push rbx
    mov rbx, rcx
    mov [rbx], rsp

    mov rsp, rdx
    and rsp, -16

    ; Windows x64 requires the caller to reserve 32 bytes of shadow space.
    sub rsp, 32
    mov rcx, r9
    call r8

    mov rsp, [rbx]
    pop rbx
    ret
mipos_start_task64 ENDP

; Define setjmp equivalent in Windows x64 ABI.
; RCX points to the buffer (a 256-byte jmp_buf).
; Buffer layout:
;   [0..56]   rbx, rbp, rsi, rdi, r12, r13, r14, r15
;   [64]      caller rsp
;   [72]      return address (rip)
;   [80]      mxcsr
;   [96..240] xmm6..xmm15 (callee-saved in the Windows x64 ABI)
mipos_save_context64 PROC
    mov [rcx], rbx
    mov [rcx + 8], rbp
    mov [rcx + 16], rsi
    mov [rcx + 24], rdi
    mov [rcx + 32], r12
    mov [rcx + 40], r13
    mov [rcx + 48], r14
    mov [rcx + 56], r15
    lea rax, [rsp + 8]
    mov [rcx + 64], rax
    mov rax, [rsp]
    mov [rcx + 72], rax
    stmxcsr dword ptr [rcx + 80]
    movdqu xmmword ptr [rcx + 96], xmm6
    movdqu xmmword ptr [rcx + 112], xmm7
    movdqu xmmword ptr [rcx + 128], xmm8
    movdqu xmmword ptr [rcx + 144], xmm9
    movdqu xmmword ptr [rcx + 160], xmm10
    movdqu xmmword ptr [rcx + 176], xmm11
    movdqu xmmword ptr [rcx + 192], xmm12
    movdqu xmmword ptr [rcx + 208], xmm13
    movdqu xmmword ptr [rcx + 224], xmm14
    movdqu xmmword ptr [rcx + 240], xmm15
    xor rax, rax
    ret
mipos_save_context64 ENDP

; Define longjmp equivalent in Windows x64 ABI.
; RCX points to the buffer, RDX is the value to return from setjmp
mipos_context_switch64 PROC
    mov r10, rcx
    mov rbx, [r10]
    mov rbp, [r10 + 8]
    mov rsi, [r10 + 16]
    mov rdi, [r10 + 24]
    mov r12, [r10 + 32]
    mov r13, [r10 + 40]
    mov r14, [r10 + 48]
    mov r15, [r10 + 56]
    mov rsp, [r10 + 64]
    mov r11, [r10 + 72]
    ldmxcsr dword ptr [r10 + 80]
    movdqu xmm6, xmmword ptr [r10 + 96]
    movdqu xmm7, xmmword ptr [r10 + 112]
    movdqu xmm8, xmmword ptr [r10 + 128]
    movdqu xmm9, xmmword ptr [r10 + 144]
    movdqu xmm10, xmmword ptr [r10 + 160]
    movdqu xmm11, xmmword ptr [r10 + 176]
    movdqu xmm12, xmmword ptr [r10 + 192]
    movdqu xmm13, xmmword ptr [r10 + 208]
    movdqu xmm14, xmmword ptr [r10 + 224]
    movdqu xmm15, xmmword ptr [r10 + 240]
    mov rax, rdx
    test rax, rax
    jne mipos_context_switch64_jump
    mov rax, 1
mipos_context_switch64_jump:
    jmp r11
mipos_context_switch64 ENDP

END
