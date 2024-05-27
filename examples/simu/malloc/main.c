/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#include "mipos.h"

/* ------------------------------------------------------------------------- */

#ifdef MIPOS64
#error 64 bit platform not supported yet
#endif

mipos_mm_t mm;
char malloc_arena[1024 * 1024] = { 0 };

static int exec_free_command(int argc, char* argv[])
{
#ifdef MIPOS64
    uintptr_t addr = 0;
#else
    int addr = 0;
#endif

    if (argc > 1) {
#if defined(MIPOS64) || defined(MIPOS32)
        sscanf(argv[1], "%zx", &addr);
#else
        sscanf(argv[1], "%x", &addr);
#endif
    }

#if defined(MIPOS_TARGET_SIMU)
    if (addr == 0) {
        return 1;
    }
#endif
    mipos_mm_free(&mm, (void*)addr);

    return 0;
}


/* ------------------------------------------------------------------------- */

static int exec_command(int argc, char* argv[])
{

    // list all files in the RAMDISK
    if (0 == strcmp(argv[0], "mem")) {
        mipos_mm_dbg_print(&mm);
    }

    // Show all files and related content
    else if (0 == strcmp(argv[0], "alloc")) {
        int size = rand() % 1024;
        void* ptr = mipos_mm_alloc(&mm, size);
        mipos_printf("Allocating %i bytes at %p\n", size, ptr);
    }

    else if (0 == strcmp(argv[0], "free")) {
        exec_free_command(argc, argv);
        mipos_mm_dbg_print(&mm);
    }

    else {
        mipos_printf("Unknown command\n");
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

static mipos_console_cmd_t user_cmd_list[] = {
    { "mem", exec_command, "mem - shows malloc info" },
    { "alloc", exec_command, "alloc - a dynamic block of memory" },
    { "free", exec_command, "free - ptr to free" },

    CMD_LIST_END
};

/* ------------------------------------------------------------------------- */

static char root_stack[4 * 1024];
int root_task(task_param_t param)
{
    mipos_console_init_t cinit = { 0 };
    cinit.prompt = "\nmipOS>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;
    cinit.recv_cbk = 0;

    mipos_console_register_cmd_list(user_cmd_list);

    mipos_console_init(&cinit);

    mipos_mm_init(&mm, malloc_arena, sizeof(malloc_arena));

    mipos_t_suspend();

    return 0;
}

/* ------------------------------------------------------------------------- */

int main()
{
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));
    return 0;
}
