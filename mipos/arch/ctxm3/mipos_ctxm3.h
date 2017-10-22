/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#ifndef mipos_CTXM3_H_
#define mipos_CTXM3_H_


/* -------------------------------------------------------------------------- */

#if defined( __ICCARM__ )
#  define IGNORE_TASK_PRIORITY
#  include "includes.h"


/* -------------------------------------------------------------------------- */

/** disable interrupt preserving priority mask */
u32  __enter_critical_section(void);


/* -------------------------------------------------------------------------- */

/** enable interrupt restoring saved priority mask */
void __leave_critical_section(u32  cpu_sr);


/* -------------------------------------------------------------------------- */

#  define CTXM3_SETJMP 1


/* -------------------------------------------------------------------------- */

typedef u32 _ctxm3_jmp_buf[16]; /* R1-R12, PSP, MSP, LR */


/* -------------------------------------------------------------------------- */

/**
 * Sets up the local _ctxm3_jmp_buf buffer and initializes it for the jump.
 * This routine saves the program's calling environment in the environment
 * buffer specified by the env argument for later use by _ctxm3_longjmp.
 *  If the return is from a direct invocation, _ctxm3_setjmp returns 0.
 *  If the return is from a call to longjmp, _ctxm3_setjmp returns
 *  a nonzero value.
*/
int _ctxm3_setjmp(_ctxm3_jmp_buf env);


/* -------------------------------------------------------------------------- */

/**
 * Restores the context of the environment buffer env that was saved
 * by invocation of the _ctxm3_setjmp routine in the same invocation
 * of the program.
 * The value specified by value is passed from _ctxm3_longjmp to _ctxm3_setjmp
 * After _ctxm3_longjmp is completed, program execution continues as if
 * the corresponding invocation of _ctxm3_setjmp had just returned.
 * If the value passed to _ctxm3_longjmp is 0, _ctxm3_setjmp will behave as
 * if it had returned 1; otherwise, it will behave as if it had
 * returned value.
 */
void _ctxm3_longjmp(_ctxm3_jmp_buf env, int value);


/* -------------------------------------------------------------------------- */

/** Initialize a critical section
 */
#  define mipos_init_cs() static u32 _kernel_cs


 /* -------------------------------------------------------------------------- */

/** Disable irq preserving old mask status
 */
#  define mipos_enter_cs() \
  do { _kernel_cs = __enter_critical_section(); } while(0)


/* -------------------------------------------------------------------------- */

/** Enable irq setting saved mask status
*/
#  define mipos_leave_cs() \
  do { __leave_critical_section(_kernel_cs); } while (0)


/* -------------------------------------------------------------------------- */

#  ifdef CORTEXM3__DONT_USE_PROCESS_STACK
#    define mipos_get_sp() ( __MRS_MSP() )
#  else
#    define mipos_get_sp() ( __MRS_PSP() )
#  endif
#endif


/* -------------------------------------------------------------------------- */

#if defined( __ICCARM__ )


/* -------------------------------------------------------------------------- */

#include "cortexm3_macro.h"


/* -------------------------------------------------------------------------- */

#ifdef CORTEXM3__DONT_USE_PROCESS_STACK
#  define __SET_STACK_POINTER(_SP_VAL_) __MSR_MSP((u32) (_SP_VAL_))
#  define __GET_STACK_POINTER() ( __MRS_MSP() )
#  define __SETUP_PROCESS_SP()
#else
#  define __SET_STACK_POINTER(_SP_VAL_) __MSR_PSP((u32) (_SP_VAL_))
#  define __GET_STACK_POINTER() ( __MRS_PSP() )
#  define __SETUP_PROCESS_SP()\
  asm ("mrs r0, CONTROL");\
  asm ("mov r0, #2");\
  asm ("msr CONTROL, r0");\
  asm ("mrs r0, MSP");\
  asm ("msr PSP, r0");\
  asm ("isb");
#endif


/* -------------------------------------------------------------------------- */

#define IRQ_SAVE_REGISTERS()\
  asm("mrs R0, MSP");\
  asm("sub R0, R0, #0x20");\
  asm("stm R0, {R4-R11}");


/* -------------------------------------------------------------------------- */

#define IRQ_RESTORE_REGISTERS()\
  asm("mrs R0, MSP");\
  asm("sub R0, R0, #0x20");\
  asm("ldm R0, {R4-R11}");


/* -------------------------------------------------------------------------- */

#define __SETUP_EXCEPTION_SP(_SP_) __MSR_MSP((u32) (_SP_))


/* -------------------------------------------------------------------------- */

#define __SETUP_KERNEL_SP()

/* -------------------------------------------------------------------------- */

#  define mipos_replace_sp(__OLD_SP, __STACK_P) do {\
    mipos_init_cs();\
    mipos_enter_cs();\
      *((u32*)__OLD_SP) = __GET_STACK_POINTER();\
      __SET_STACK_POINTER(__STACK_P);\
      __DSB();\
      __ISB();\
    mipos_leave_cs();\
} while(0)


/* -------------------------------------------------------------------------- */

#  define mipos_set_sp(__OLD_SP) \
  do {\
    mipos_init_cs();\
    mipos_enter_cs();\
      __SET_STACK_POINTER(*((u32*)__OLD_SP));\
      __DSB();\
      __ISB();\
    mipos_leave_cs();\
} while(0)


/* -------------------------------------------------------------------------- */

#else
#  error platform or compiler not supported
#endif


/* -------------------------------------------------------------------------- */

#endif //! mipos_CTXM3_H_

/* -------------------------------------------------------------------------- */
