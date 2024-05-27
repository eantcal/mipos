/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifndef mipos_BSP_H__
#define mipos_BSP_H__

#define RESET_TYPE_HWRS 0
#define RESET_TYPE_IWDG 1

/* ------------------------------------------------------------------------- */

/** configures serial port used for the console
 *
 * @param baud_rate speed rate
 * @return none
 */
void mipos_bsp_configure_rs232(unsigned int baud_rate);

/* ------------------------------------------------------------------------- */

/** receive a byte from serial interface
 * this function is not blocking
 * @param key returned byte
 * @return 1 if a data is read, 0 otherwise
 */
unsigned int mipos_bsp_rs232_recv_byte(unsigned char* key);

/* ------------------------------------------------------------------------- */

/** send a byte to serial interface
 * this function is not blocking
 */
void mipos_bsp_rs232_putc(unsigned char c);

/* ------------------------------------------------------------------------- */

#if MIPOS_TARGET_STM32 // defined(__ICCARM__)
#include "mipos_bsp_stm32.h"
#elif MIPOS_TARGET_STM8 // defined(__CSMC__) || defined(__RCSTM8__)
#include "mipos_bsp_stm8.h"
#else // MIPOS_TARGET_SIMU
#ifndef MIPOS_TARGET_SIMU
#define MIPOS_TARGET_SIMU
#endif
#include "mipos_bsp_simu.h"
#endif

#endif // ... mipos_BSP_H__

/* ------------------------------------------------------------------------- */
