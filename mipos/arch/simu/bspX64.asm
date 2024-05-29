; bspX64.asm
; Adjusted for Microsoft x64 calling convention.

.CODE
; Function to get the current stack pointer (rsp)
mipos_get_sp PROC
    mov rax, rsp  ; Move the value of RSP of caller into RAX
    add rax, 8
    ret           ; Return, RAX will hold the return value
mipos_get_sp ENDP

; Function to replace the stack pointer and store the old one
; Parameters:
; rcx = old_sp (address where old stack pointer will be stored, void**)
; rdx = new_sp (new stack pointer value, void*)
mipos_replace_sp PROC
    mov rax, rsp ; Store current rsp at that address
    add rax, 8   ; Calculate caller rsp
    mov [rcx], rax ; Store caller rsp at that address
    mov rsp, rdx   ; Update rsp to the new value provided in new_sp
     
    ; Instead of 'ret', manually jump to the address stored at the top of the old stack
    mov rax, [rax-8] ; Indirect jump to caller
    jmp rax
mipos_replace_sp ENDP

; Function to set the stack pointer from a provided value
; Parameter:
; rcx = new_sp (new stack pointer value, void*)
mipos_set_sp PROC
    mov rax, rsp
    mov rsp, rcx   ; Update rsp directly from new_sp
    mov rax, [rax]
    jmp rax
mipos_set_sp ENDP

.code
; Define setjmp equivalent in Windows x64 ABI
; RCX points to the buffer
mipos_save_context64 PROC
    mov [rcx], rax
    mov [rcx + 8], rbx
    mov [rcx + 16], rcx
    mov [rcx + 24], rdx
    mov [rcx + 32], rsi
    mov [rcx + 40], rdi
    mov [rcx + 48], rbp
    mov [rcx + 56], rsp
    mov [rcx + 64], r8
    mov [rcx + 72], r9
    mov [rcx + 80], r10
    mov [rcx + 88], r11
    mov [rcx + 96], r12
    mov [rcx + 104], r13
    mov [rcx + 112], r14
    mov [rcx + 120], r15
    mov rax, rsp
    mov rax, [rax]
    mov [rcx + 128], rax
    xor rax, rax    ; Convention: setjmp returns 0 on initial call
    ret
mipos_save_context64 ENDP

; Define longjmp equivalent in Windows x64 ABI
; RCX points to the buffer, RDX is the value to return from setjmp
mipos_context_switch64 PROC
    mov rax, [rcx]        ; Restore all registers
    mov rbx, [rcx + 8]
    mov rcx, [rcx + 16]
    mov rsi, [rcx + 32]
    mov rdi, [rcx + 40]
    mov rbp, [rcx + 48]
    mov rsp, [rcx + 56]
    mov r8, [rcx + 64]
    mov r9, [rcx + 72]
    mov r10, [rcx + 80]
    mov r11, [rcx + 88]
    mov r12, [rcx + 96]
    mov r13, [rcx + 104]
    mov r14, [rcx + 112]
    mov r15, [rcx + 120]
    mov rax, rdx          ; Set return value as provided in RDX
    mov rdx, [rcx + 24]
    mov rcx, [rcx + 128]       ; Jump to stored RIP
    jmp rcx
mipos_context_switch64 ENDP

END
