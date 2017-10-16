;**(C) 2008-2017 antonino.calderone@gmail.com **********
;* File Name          : mipos_kernel_ctxm3.s
;* Author             : Antonino Calderone
;* Date First Issued  : 03/09/2008
;* Description        : Cortex-M3 platform support
;* This file is part of the mipOS-(c) 2008-2017 A. Calderone
;*
;* mipOS (c) 2005-2017 Antonino Calderone
;* All rights reserved.
;* Licensed under the MIT License.
;* See COPYING file in the project root for full license information.
;***********************************************************
; History:
; 03/09/2008: V0.1
; 16/09/2008: V0.2 ; added _ctxm3_setjmp, _ctxm3_longjmp
;***********************************************************
    
  RSEG CODE:CODE(2)

  ; Exported functions
  
  EXPORT __enter_critical_section
  EXPORT __leave_critical_section
  EXPORT _ctxm3_setjmp
  EXPORT _ctxm3_longjmp
 
; ----------------------------------------------------------
; u32  __enter_critical_section(void);
; returns R0
; ----------------------------------------------------------
__enter_critical_section
  ;set prio int mask to mask all (except faults)
  MRS     R0, PRIMASK   
  CPSID   I
  BX      LR

; ----------------------------------------------------------
; void __leave_critical_section(u32  cpu_sr);
; ----------------------------------------------------------
__leave_critical_section 
  MSR     PRIMASK, R0
  BX      LR

; ----------------------------------------------------------
; int _ctxm3_setjmp(jmp_buf env) 
;                            ^- R0
; returns R0
; ----------------------------------------------------------
_ctxm3_setjmp

; Note: Registers are always stored in their numeric order, 
; not as specified. (Example stmia r0, {r1, r3} is 
; the same of stmia r0, {r3, r1} )

; R0 point to env 
; Note2: r0 is incremented by 12*4 bytes,
;        We don't save status register 
;        (should we need to save it ?)
 STMIA R0!, {R1-R12}

 MRS R1, PSP
 MRS R2, MSP
 MOV R3, LR

 STMIA R0!, {R1-R4}

 MOV R0, #0 ; return 0
 BX LR      ; MOV PC, LR 

; ----------------------------------------------------------
; void _ctxm3_longjmp(jmp_buf env, int value)
;                              ^- R0     ^- R1
; The jmp_buf is assumed to contain the following, in order:
; R1-R12, PSP, MSP, LR
; ----------------------------------------------------------
_ctxm3_longjmp
 MOV R5, R0         ; save pointer to env into R5
 MOV R6, R1         ; save value parameter into R6

 ADD R0, R0, #48    ; (12*4) r0->point to xSP, LR regs copy

 LDMIA R0!, {R1-R4} ; load stack pointers and link register

 MSR PSP, R1        ; restore stack pointers
 MSR MSP, R2         
 MOV LR, R3         ; restore link register 

 ISB
 
 PUSH { R6 }        ; save 'value' onto stack

 MOV R0, R5         ; restore R1-R12 registers from env
 LDMIA R0!, {R1-R12} 

 POP { R0 }         ; recover 'value' from the stack
 
 BNE _ret1          ; if 0 return 1, else 'value' 
                    ; Note: register LR set by _ctxm3_setjmp
 MOV R0, #1         ; Execution restart from that address
                    ; and _ctxm3_setjmp returns our R0 value
_ret1
 BX LR              ; return to saved LR
  
 END

