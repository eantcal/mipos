/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

// BSP for STM8 & Cosmic C compiler && Raisonance C Compiler

#ifndef mipos_BSP_STM8_H__
#define mipos_BSP_STM8_H__

#include "stm8s.h"
#include "setjmp.h"


/* -------------------------------------------------------------------------- */

//True for the first 3 seconds of system live
char system_booting( void );

//1ms O/S tick generated by hw timer
void mipos_bsp_create_hw_rtc_timer( void );

//Configure reset and watch-dog registers
void mipos_bsp_setup_reset_and_wd( void );

//Configure exception table
//void mipos_bsp_exception_vector( void );
#define mipos_bsp_exception_vector()

//Configure CLKs
//void mipos_bsp_setup_clk( void );
#define mipos_bsp_setup_clk()

//Return reset type
int mipos_bsp_check_reset_type( void );


/* -------------------------------------------------------------------------- */

#ifdef __RCSTM8__

#define mipos_set_sp(__N_SP) _setSP_((u16)__N_SP)
#define mipos_replace_sp(__OLD_SP, __N_SP) \
  do { __OLD_SP = (u8*) _getSP_();  \
       mipos_set_sp(__N_SP); } while (0)

#define mipos_get_sp() _getSP_()
#define mipos_init_cs()
#define mipos_enter_cs() _sim_()
#define mipos_leave_cs() _rim_()


/* -------------------------------------------------------------------------- */

#else //Cosmic

#define mipos_set_sp(__N_SP) _asm("LDW SP,X\n", __N_SP)
#define mipos_replace_sp(__OLD_SP, __N_SP) do {\
  __OLD_SP = (void*) _asm("LDW X,SP\n");\
	mipos_set_sp(__N_SP);\
} while (0)
#define mipos_get_sp() _asm("LDW X,SP")

#define mipos_init_cs()
#define mipos_enter_cs() _asm("SIM")
#define mipos_leave_cs() _asm("RIM")


/* -------------------------------------------------------------------------- */

#endif

#define assert( _C ) do { if (_C); } while(0)
#define STACK_UNIT 1

#define mipos_bsp_notify_scheduler_epoch()  /*UNUSED*/
#define mipos_bsp_dbg_device_setup()        /*UNUSED*/
#define mipos_bps_init_sp()                 /*UNUSED*/

#define mipos_bsp_enable_irq_mask()	enableInterrupts()
#define mipos_kick_watchdog() IWDG_ReloadCounter()

//Required for simulation code compatibility
#define inline                              /*EMPTY*/
#define MIPOS_IGNORE_TASK_PRIORITY
#define MIPOS_RTC_QUANTUM_IN_MS
#define MIPOS_LONG_LONG_NOT_SUPPORTED
#define ENABLE_WATCHDOG

/* -------------------------------------------------------------------------- */

#ifdef BUILT_IN_SETJMP

typedef unsigned char jmp_bufex[ 16 ];
int  setjmpex(jmp_bufex env);
void longjmpex(jmp_bufex env);
#define mipos_save_context(_x)      setjmpex((unsigned char*)_x)
#define mipos_context_switch_to(_x) longjmpex((unsigned char*)_x)

#else

typedef jmp_buf jmp_bufex;

#define mipos_save_context(_x)      setjmp((unsigned char*)_x)
#define mipos_context_switch_to(_x) longjmp((unsigned char*)_x,1)

#endif

/* -------------------------------------------------------------------------- */

#ifndef CONSOLE_SENDER_STACK
#define CONSOLE_SENDER_STACK (512)
#endif

#ifndef CONSOLE_RECEIVER_STACK
#define CONSOLE_RECEIVER_STACK (512)
#endif

#define MIPOS_LE_U16(x) (x)
#define MIPOS_LE_U32(x) (x)


#endif // ... mipos_BSP_STM8_H__
