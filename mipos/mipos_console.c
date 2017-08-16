/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#include "mipos_console.h"


/* -------------------------------------------------------------------------- */

#ifdef ENABLE_MIPOS_CONSOLE


/* -------------------------------------------------------------------------- */

static int exec_help_command(int argc, char * argv[]);
static int exec_ver_command(int argc, char * argv[]);
static int exec_ps_command(int argc, char * argv[]);
static int exec_taskdbg_command(int argc, char * argv[]);
static int exec_dump_command(int argc, char* argv[]);
static int exec_patch_command(int argc, char* argv[]);


/* -------------------------------------------------------------------------- */

static
mipos_console_cmd_t _mipos_cmd_list[] = {
  {"help", exec_help_command, "help - shows this list"},
  {"ver", exec_ver_command,   "ver - shows mipOS version"},

  {"dump", exec_dump_command, "dump [<a>,[<c>,[<b>]] - dump a memory area"},
  {"patch",exec_patch_command,"patch <a> <b> - set a byte <b> at address <a>"},

  {"ps", exec_ps_command,     "ps - shows the list of tasks"},
  {"freeze", exec_taskdbg_command,   "freeze <tid> - freezes a task"},
  {"unfreeze", exec_taskdbg_command, "unfreeze <tid> - unfreezes a task"},
  {"delete", exec_taskdbg_command,   "delete <tid> - delete a task"},
  {"signal", exec_taskdbg_command,   "signal <tid> <signum> - set a signal"},

  CMD_LIST_END
};


/* -------------------------------------------------------------------------- */


static mipos_console_cmd_t * mipos_cmd_list = &_mipos_cmd_list[0];
static unsigned int end_line_char = '\n';
static console_flg_t console_flags = 0L;
static const char* console_prompt = 0;
static char sender_task_stack[CONSOLE_SENDER_STACK];
static char recvr_task_stack[CONSOLE_RECEIVER_STACK];
static mipos_queue_t rs232_tx_queue;
static mipos_q_item_t rs232_tx_queue_pool[RS232_TX_QUEUE_LEN];


/* -------------------------------------------------------------------------- */

typedef struct _msg_t
{
    unsigned char  buffer[RS232_TX_BUF_LEN];
    unsigned int len;
} msg_t;


/* -------------------------------------------------------------------------- */

static msg_t rs232_tx_buf_pool[RS232_TX_QUEUE_LEN];
static unsigned int rs232_tx_msg_idx = 0;
static unsigned char rs232_rx_buf[RS232_RX_BUF_LEN];
static unsigned int rs232_rx_buf_idx = 0;
static mipos_console_cmd_t * registered_cmd_list = 0;


/* -------------------------------------------------------------------------- */

void console_register_cmd_list(mipos_console_cmd_t _cmd_list[])
{
    registered_cmd_list = &_cmd_list[0];
}


/* -------------------------------------------------------------------------- */

#ifndef mipos_BSP_SIMU_H__
unsigned char mipos_console_get_char(void)
{
    unsigned char key = 0;

    while (!mipos_bsp_rs232_recv_byte((unsigned char*)&key)) {
        mipos_tm_wkafter(0);
    }

    return key;
}
#endif



/* -------------------------------------------------------------------------- */


void mipos_console_put_char(unsigned char c)
{
    if ((c == end_line_char) &&
        (console_flags & CONSOLE_TX_CRLF))
    {
        mipos_bsp_rs232_putc('\r');
        mipos_bsp_rs232_putc('\n');
    }
    else {
        mipos_bsp_rs232_putc(c);
    }
}


/* -------------------------------------------------------------------------- */

void mipos_puts(const char * buf)
{
    while (*buf) {
        mipos_bsp_rs232_putc(*buf++);
    }
}


/* -------------------------------------------------------------------------- */

int mipos_split_command(char * cmd_line,
    int cmd_line_len,
    char* argv[],
    int max_args)
{
    int arg_count = 0;
    int pos = 0;
    int word_pos = 0;

    for (pos = 0; pos < cmd_line_len; ++pos) {
        if (cmd_line[pos] == ' ' ||
            cmd_line[pos] == '\t' ||
            cmd_line[pos] == '\r' ||
            cmd_line[pos] == '\n' ||
            (pos == (cmd_line_len - 1))
            )
        {
            argv[arg_count++] = &cmd_line[word_pos];
            
            if (arg_count >= max_args || pos == (cmd_line_len - 1)) {
                if (pos == (cmd_line_len - 1) && cmd_line[pos] == ' ')
                    cmd_line[pos] = 0;
                break;
            }
            cmd_line[pos] = 0;
            word_pos = pos + 1;
        }
    }

    return arg_count;
}


/* -------------------------------------------------------------------------- */

static void show_command_list(mipos_console_cmd_t* cmd_list)
{
    while (cmd_list && cmd_list->cmd_str && cmd_list->cmd_exec) {
        mipos_puts("\t");
        mipos_puts(cmd_list->cmd_help);
        mipos_puts("\r\n");
        ++cmd_list;
    }
}


/* -------------------------------------------------------------------------- */

static int try_exec_cmd(char * cmd_str,
    unsigned int cmd_str_len,
    mipos_console_cmd_t* cmd_list)
{
    int index_of_cmd = 0;
    char* argv[CONSOLE_MAX_ARGS_COUNT] = { 0 };

    int argc = mipos_split_command(cmd_str,
        cmd_str_len,
        argv,
        sizeof(argv) / sizeof(argv[0]));

    while (cmd_list->cmd_str && cmd_list->cmd_exec)
    {
        if (argc > 0 && strcmp(argv[0], cmd_list->cmd_str) == 0)
        {
            cmd_list->cmd_exec(argc, argv);
            return index_of_cmd;
        }
        ++cmd_list;
        ++index_of_cmd;
    }

    return -1L;
}


/* -------------------------------------------------------------------------- */

/** recvr_task task entry point
 *
 * @param context: pointer to the task context parameter area
 */
static
int recvr_task(task_param_t context) {
    unsigned char c = 0;
    console_recv_cbk_t recv_cbk = (console_recv_cbk_t)context;
    rs232_rx_buf_idx = 0;

    while (1)
    {
        c = mipos_console_get_char();

        if (c) {

            if (console_flags & CONSOLE_FLG_ECHO) {
                if (!(rs232_rx_buf_idx < 1 && c == '\b')) {
                    mipos_console_put_char(c);
                }
            }

            if ((c == (unsigned char)end_line_char)) {

                char* cmd_str = (char*)rs232_rx_buf;
                unsigned int cmd_len = rs232_rx_buf_idx;

                if (mipos_cmd_list &&
                    try_exec_cmd(cmd_str, cmd_len,
                        mipos_cmd_list) >= 0) {
                }
                else if (registered_cmd_list &&
                    try_exec_cmd(cmd_str, cmd_len,
                        registered_cmd_list) >= 0) {
                }
                else {
                    recv_cbk((unsigned char*)cmd_str, cmd_len);
                }

                if (console_prompt) {
                    mipos_console_send_null_str(console_prompt);
                }

                memset(rs232_rx_buf, 0, sizeof(rs232_rx_buf));
                rs232_rx_buf_idx = 0;
            }


            if (c > 31 && rs232_rx_buf_idx < RS232_RX_BUF_LEN) {
                rs232_rx_buf[rs232_rx_buf_idx++] = c;
            }
            else if (c == '\b'  && rs232_rx_buf_idx > 0) {
                mipos_console_put_char(' ');
                mipos_console_put_char('\b');
                rs232_rx_buf[--rs232_rx_buf_idx] = 0;
            }
        }
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

/** sender_task task entry point
 *
 * @param context: pointer to the task context parameter area
 * @return none
 */
static
int sender_task(task_param_t context) {
    mipos_q_item_t item;
    mipos_puts("\r\n");
    mipos_puts(MIPOS_STR_VER);

    mipos_puts(" console\r\n");
    mipos_puts("Compile date/time: " __DATE__ " " __TIME__ " - opt. mods:\r\n");
    mipos_puts("  mipos_console\r\n");

#ifdef ENABLE_MIPOS_FS
    mipos_puts("  mipos_fs\r\n");
#endif

#ifdef ENABLE_MIPOS_STDIO
    mipos_puts("  mipos_stdio\r\n");
#endif

#if defined(ENABLE_MIPOS_MM) || defined(ENABLE_MIPOS_MALLOC)
    mipos_puts("  mipos_mm\r\n");
#endif

#ifdef ENABLE_MIPOS_MALLOC
    mipos_puts("  mipos_malloc\r\n");
#endif

#ifdef ENABLE_MIPOS_MPOOL
    mipos_puts("  mipos_mpool\r\n");
#endif

    if (console_prompt) {
        mipos_puts(console_prompt);
    }

    while (1) {

        static unsigned char* buffer;
        unsigned int len;

        bool_t bl = mipos_q_count(&rs232_tx_queue) <= 0;

        if (mipos_q_receive(&rs232_tx_queue, &item, bl) <= 0) {
            mipos_tm_msleep(100);
            continue;
        }

        len = rs232_tx_buf_pool[item].len;
        buffer = rs232_tx_buf_pool[item].buffer;

        while (len--) {
            if (!(*buffer)) {
                break;
            }

            mipos_console_put_char(*buffer);

            ++buffer;
        }
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

unsigned int mipos_console_send(const unsigned char * buf, unsigned int len)
{
    if (mipos_q_count(&rs232_tx_queue) >= RS232_TX_QUEUE_LEN) {
        return 0;
    }

    if (len > RS232_TX_BUF_LEN) {
        len = RS232_TX_BUF_LEN;
    }

    rs232_tx_buf_pool[rs232_tx_msg_idx].len = len;
    memcpy(rs232_tx_buf_pool[rs232_tx_msg_idx].buffer, buf, len);

    mipos_q_send(&rs232_tx_queue, rs232_tx_msg_idx);

    rs232_tx_msg_idx = (rs232_tx_msg_idx + 1) % RS232_TX_QUEUE_LEN;

    return len;
}


/* -------------------------------------------------------------------------- */

unsigned int mipos_console_send_null_str(const char * buf)
{
    int len = strlen(buf);
    return mipos_console_send((const unsigned char*)buf, len);
}


/* -------------------------------------------------------------------------- */

void mipos_console_init(const mipos_console_init_t * config)
{
    end_line_char = config->end_line_char;
    console_flags = config->flags;
    console_prompt = config->prompt;

    mipos_q_init(&rs232_tx_queue,
        rs232_tx_queue_pool,
        (sizeof(rs232_tx_queue_pool) / sizeof(mipos_q_item_t)));

    mipos_bsp_configure_rs232(config->baud_rate);

    mipos_t_create("cnsltx",
        sender_task,
        (task_param_t)0,
        sender_task_stack,
        sizeof(sender_task_stack),
        MIPOS_NO_RT);

    mipos_t_create("cnslrx",
        recvr_task,
        (task_param_t)config->recv_cbk,
        recvr_task_stack,
        sizeof(recvr_task_stack),
        MIPOS_NO_RT);

}


/* -------------------------------------------------------------------------- */

inline
const char* mipos_dbg_describe_state(int st) {

    if (st & TASK_FROZEN) {
        return "frozen";
    }
    switch (st) {
    case TASK_NOT_RUNNING:
        return "not run";
    case TASK_READY:
        return "ready";
    case TASK_RUNNING:
        return "running";
    case TASK_SUSPENDED:
        return "suspend";
    case TASK_ZOMBIE:
        return "zombie";
    }

    return "unknown";
}


/* -------------------------------------------------------------------------- */

extern void mipos_puts(const char * buf);


/* -------------------------------------------------------------------------- */

void mem_dump(const int b[],
    unsigned int buf_len,
    int mode)
{
    int row;
    int buf_index = 0;
    if (buf_len == 0) return;

    int tot_rows = (buf_len >> 4) + ((buf_len & 0xf) ? 1 : 0);

    char local_buffer[32] = { 0 };

    for (row = 0; row < tot_rows; row++) {
        int col;

        buf_index = row * 16;

        sprintf(local_buffer, "0x%08X ", (unsigned int) ((const int*)& b[buf_index]));
        mipos_puts(local_buffer);

        for (col = 0; col < 4; col++) {
            unsigned int i = buf_index + col;

            if (i >= buf_len)
                mipos_puts("            ");
            else {
                char c[4];
                c[mode ? 0 : 3] = (b[i] >> 24);
                c[mode ? 1 : 2] = (b[i] >> 16);
                c[mode ? 2 : 1] = (b[i] >> 8);
                c[mode ? 3 : 0] = (b[i]);

                sprintf(local_buffer, "%02X %02X %02X %02X ",
                    c[0] & 0xff, c[1] & 0xff, c[2] & 0xff, c[3] & 0xff
                );

                mipos_puts(local_buffer);
            }
        }

        for (col = 0; col < 4; col++) {
            unsigned int i = buf_index + col;
            if (i >= buf_len) break;
            char c[4];
            c[mode ? 0 : 3] = (b[i] >> 24);
            c[mode ? 1 : 2] = (b[i] >> 16);
            c[mode ? 2 : 1] = (b[i] >> 8);
            c[mode ? 3 : 0] = (b[i]);

            sprintf(local_buffer, "%c%c%c%c",
                c[0] > 31 ? c[0] : '.',
                c[1] > 31 ? c[1] : '.',
                c[2] > 31 ? c[2] : '.',
                c[3] > 31 ? c[3] : '.'
            );
            mipos_puts(local_buffer);
        }

        if ((tot_rows - 1) != row) {
            mipos_puts("\r\n");
        }
    }

    mipos_puts("\r\n");
}


/* -------------------------------------------------------------------------- */

static
int exec_help_command(int argc, char* argv[])
{
    mipos_puts("mipOS command list:\r\n");
    show_command_list(mipos_cmd_list);

    mipos_puts("\r\nUser-defined command list:\r\n");
    show_command_list(registered_cmd_list);
    return 0;
}


/* -------------------------------------------------------------------------- */

static
int exec_dump_command(int argc, char* argv[])
{
    int page = 64;
    int addr = 0;
    int mode = 0;

    if (argc > 3) {
        mode = 1;
    }
    if (argc > 2) {
        page = atoi(argv[2]);
    }
    if (argc > 1) {
        sscanf(argv[1], "%x", &addr);
    }

    mem_dump((int*)addr, page, mode);

    return 0;
}


/* -------------------------------------------------------------------------- */

static
int exec_patch_command(int argc, char* argv[])
{
    int addr = 0;
    int value = 0;

    if (argc != 3) {
        mipos_puts("Wrong number of parameters\r\n");
        return -1L;
    }

    sscanf(argv[1], "%x", &addr);
    sscanf(argv[2], "%x", &value);

    *((char*)(addr)) = (char)(value & 0xff);

    return 0;
}


/* -------------------------------------------------------------------------- */

static
int exec_taskdbg_command(int argc, char* argv[])
{
    int tid = 0;

    if (argc > 1) {
        tid = atoi(argv[1]);
    }

    if (tid > 0 && tid < MIPOS_MAX_TASKS_COUNT) {
        switch (argv[0][0]) {
            case 'u':mipos_t_unfreeze(tid); break;
            case 'f':mipos_t_freeze(tid);   break;
            case 'd':mipos_t_delete(tid);   break;
            case 's': 
                if (argc > 2) {
                    int signum = atoi(argv[2]);
                    mipos_t_notify_signal(tid, signum);
                }
                else {
                    mipos_puts("\rMissing second parameter\r");
                }
        }

    }
    else {
        mipos_puts("\rMissing first parameter\r");
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

static
int exec_ver_command(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    mipos_puts(MIPOS_STR_VER);
    mipos_puts("\r\n");
    return 0;
}


/* -------------------------------------------------------------------------- */

static
int exec_ps_command(int argc, char * argv[])
{
    char tx_buf[32];
    int id;

    (void)argc;
    (void)argv;

    mipos_puts("  name   id entryptr pend_sig wait_sig st status   "
        "topstack stackptr stack/tot\r\n");

    for (id = 1 /* skip idle*/;
        id < MIPOS_MAX_TASKS_COUNT;
        ++id)
    {
        mipos_task_t * p_task = &(KERNEL_ENV.task_table[id]);

        if (p_task->entry_point) {
            void * top_of_stack =
                &p_task->task_stack[p_task->stack_size - sizeof(mipos_reg_t)];

            sprintf(tx_buf, "%8s ", p_task->name);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%02x ", id);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%08x ", (u32) p_task->entry_point);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%08x ", p_task->signal_pending);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%08x ", p_task->signal_waiting);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%2i %8s ",
                p_task->status,
                mipos_dbg_describe_state(p_task->status));
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%08x ", (unsigned int) top_of_stack);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%08x ", p_task->process_stack_pointer);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%4i/",
                (char*)top_of_stack - (char*)p_task->process_stack_pointer);
            mipos_puts(tx_buf);

            sprintf(tx_buf, "%4i\r\n", p_task->stack_size);
            mipos_puts(tx_buf);

            mipos_tm_msleep(1);
        }
    }

    return 0;
}


/* -------------------------------------------------------------------------- */


#else
#warning macro ENABLE_MIPOS_CONSOLE not defined, console will be not compiled
#endif // !ENABLE_MIPOS_CONSOLE
