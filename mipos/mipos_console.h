/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#ifndef mipos_CONSOLE_H_
#define mipos_CONSOLE_H_


/* -------------------------------------------------------------------------- */

#include "mipos_bsp.h"
#include "mipos_kernel.h"


/* -------------------------------------------------------------------------- */

typedef void(*console_recv_cbk_t)(char *, int);

#ifndef RS232_TX_QUEUE_LEN
#define RS232_TX_QUEUE_LEN 8
#endif

#ifndef RS232_TX_BUF_LEN
#define RS232_TX_BUF_LEN  80
#endif

#ifndef RS232_RX_BUF_LEN
#define RS232_RX_BUF_LEN  32
#endif

typedef unsigned int console_flg_t;
#define CONSOLE_FLG_ECHO 1
#define CONSOLE_TX_CRLF  2

#define CONSOLE_MAX_ARGS_COUNT 8


/* -------------------------------------------------------------------------- */

typedef struct _mipos_console_init_t {
    console_recv_cbk_t recv_cbk;
    unsigned int baud_rate;
    console_flg_t flags;
    const char * prompt;
    unsigned char end_line_char;
}
mipos_console_init_t;


/* -------------------------------------------------------------------------- */

/** Configures the console and starts communication tasks
 *  @param config: pointer to the configuration structure
 */
void mipos_console_init(const mipos_console_init_t * config);


/* -------------------------------------------------------------------------- */

/** Puts a character on the console
 *  @param c: the ASCII code of the character
 */
void mipos_console_put_char(unsigned char c);


/* -------------------------------------------------------------------------- */

/** Puts a string on the console
 *  @param str: the string pointer
 */
void mipos_puts(const char* str);


/* -------------------------------------------------------------------------- */

/** Gets a character from the console
 *  @return the read character or NULL if no any
 */
unsigned char mipos_console_get_char(void);


/* -------------------------------------------------------------------------- */

/** Sends a "length limited" string to the console.
 *  @param buf: the buffer containing string to send
 *  @param len: length of the string
 *  @return the number of bytes sent
 */
unsigned int mipos_console_send(const unsigned char * buf, unsigned int len);


/* -------------------------------------------------------------------------- */

/** Sends a "null terminated" string to the console.
 *  @param buf: the buffer containing null-terminated-string to send
 *  @return the number of bytes sent
 */
unsigned int mipos_console_send_null_str(const char * buf);


/* -------------------------------------------------------------------------- */

/** Prototype for call-back function assignable to a command
 */
typedef int(*mipos_console_cbk_t)(int argc, char* argv[]);


/* -------------------------------------------------------------------------- */

/** mipos_console_cmd_t is a struct that represents a single registered command
 *  cmd_str is the command string, cmd_exec is a pointer the call-back
 *  function called if command is issued to the console terminal
 */
typedef struct _console_cmd_t {
    const char * cmd_str;
    mipos_console_cbk_t cmd_exec;
    const char * cmd_help;
} mipos_console_cmd_t;


/* -------------------------------------------------------------------------- */

#define CMD_LIST_END \
    { (const char *) 0, (mipos_console_cbk_t) 0 }


/* -------------------------------------------------------------------------- */

/** console_register_cmd_list registers a list of command
 *  Each command is referred to a call-back function.
 *  Parameters typed after command string on the terminal, separated by space
 *  or tab character are passed within an array of char* elements.
 *  The number of arguments is passed to the callback function as first
 *  parameter of the function.
 */
void console_register_cmd_list(mipos_console_cmd_t cmd_list[]);


/* -------------------------------------------------------------------------- */

/** Parse a command line and split it in arguments */
int mipos_split_command(
    char * cmd_line,
    int cmd_line_len,
    char* argv[],
    int max_args);


/* -------------------------------------------------------------------------- */

#endif // !mipos_CONSOLE_H_


/* -------------------------------------------------------------------------- */
