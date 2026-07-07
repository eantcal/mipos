/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

// mipOS tiny filesystem demo for the QEMU lm3s6965evb board.
//
// Run with (interactive; quit with Ctrl-A x):
//   qemu-system-arm -M lm3s6965evb -kernel mipos-arm-qemu-filesystem.elf
//       -nographic -serial mon:stdio
//
// Type 'help' at the mipOS> prompt, then try 'ls' and 'cat test1'.

#include "mipos.h"
#include "mipos_bsp.h"

#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------------- */

static char root_stack[8192];
static mipos_tfs_t tiny_fs;
static char ramdisk[mipos_tfs_SIZE + mipos_tfs_OFFSET +
                    mipos_tfs_STORAGE_AREA_SIZE] = { 0 };

/* ------------------------------------------------------------------------- */

static int disk_write(const char* srcbuf, uint32_t bsize, uint32_t doffset)
{
    memcpy(&ramdisk[doffset], srcbuf, bsize);
    return 0;
}

static int disk_read(char* dstbuf, uint32_t bsize, uint32_t soffset)
{
    memcpy(dstbuf, &ramdisk[soffset], bsize);
    return 0;
}

/* ------------------------------------------------------------------------- */

static void create_demo_file(const char* name,
                             const char* chunk0,
                             const char* chunk1,
                             const char* chunk2)
{
    mipos_tfs_file_handle_t handle;
    uint32_t bytes;

    mipos_tfs_delete_file(&tiny_fs, name);

    if (!mipos_tfs_create_file(&tiny_fs, name, &handle)) {
        mipos_printf("failed to create %s\n", name);
        return;
    }

    bytes = (uint32_t)strlen(chunk0);
    mipos_tfs_write_file(&tiny_fs, chunk0, &bytes, handle);
    bytes = (uint32_t)strlen(chunk1);
    mipos_tfs_write_file(&tiny_fs, chunk1, &bytes, handle);
    bytes = (uint32_t)strlen(chunk2);
    mipos_tfs_write_file(&tiny_fs, chunk2, &bytes, handle);

    mipos_tfs_close_file(&tiny_fs, handle);
}

/* ------------------------------------------------------------------------- */

static void print_listing(void)
{
    mipos_tfs_file_handle_t handle = 0;
    mipos_tfs_fd_t fd;
    char label[mipos_tfs_MAX_VOLUME_LENGTH] = { 0 };

    mipos_tfs_get_label(&tiny_fs, label);
    mipos_printf("Volume label %s\n", label);

    if (!mipos_tfs_ffirst(&tiny_fs, "*", &handle)) {
        mipos_printf("No files\n");
        return;
    }

    do {
        mipos_tfs_get_fd(&tiny_fs, handle, &fd);
        mipos_printf("%s %8u bytes\n", fd.filename, (unsigned int)fd.size);
    } while (mipos_tfs_fnext(&tiny_fs, &handle));
}

static int exec_cmd_ls(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    print_listing();

    return 0;
}

/* ------------------------------------------------------------------------- */

static int print_file(const char* name)
{
    mipos_tfs_file_handle_t handle;
    mipos_tfs_fd_t fd;
    char buffer[33];
    uint32_t bytes;

    if (!mipos_tfs_open_file(
          &tiny_fs, name, &handle, mipos_tfs_FILE_OPEN_READ)) {
        mipos_printf("'%s' not found\n", name);
        return -1;
    }

    mipos_tfs_get_fd(&tiny_fs, handle, &fd);
    mipos_printf("%s %8u bytes\n", fd.filename, (unsigned int)fd.size);

    do {
        bytes = sizeof(buffer) - 1;
        if (!mipos_tfs_read_file(&tiny_fs, buffer, &bytes, handle)) {
            break;
        }
        buffer[bytes] = 0;
        mipos_printf("%s", buffer);
    } while (bytes == sizeof(buffer) - 1);

    mipos_printf("\n");
    mipos_tfs_close_file(&tiny_fs, handle);

    return 0;
}

static int exec_cmd_cat(int argc, char* argv[])
{
    if (argc < 2) {
        mipos_printf("usage: cat <filename>\n");
        return -1;
    }

    return print_file(argv[1]);
}

/* ------------------------------------------------------------------------- */

static mipos_console_cmd_t user_cmd_list[] = {
    { "ls", exec_cmd_ls, "ls             - shows list of files" },
    { "cat", exec_cmd_cat, "cat <filename> - show file content" },

    CMD_LIST_END
};

/* ------------------------------------------------------------------------- */

static int root_task(task_param_t param)
{
    mipos_console_init_t cinit = { 0 };

    (void)param;

    tiny_fs.io_dev.io_read = disk_read;
    tiny_fs.io_dev.io_write = disk_write;

    mipos_tfs_setup(&tiny_fs);
    mipos_tfs_mkfs(&tiny_fs, "RAMDISK");

    create_demo_file("test1",
                     "0123456789abcdefghij",
                     "klmnopqrstuvwxyz0123",
                     "ABCDEFGHIJKLMNO\n");
    create_demo_file("test2",
                     "9876543210ABCDEFGHIJ",
                     "KLMNOPQRSTUVWXYZ0123",
                     "abcdefghijklmn\n");

    cinit.prompt = "\nmipOS>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;

    mipos_console_register_cmd_list(user_cmd_list);
    mipos_console_init(&cinit);

    mipos_printf("mipOS tiny filesystem demo\n");
    mipos_printf("Type 'ls' and 'cat test1'.\n");
    print_listing();
    print_file("test1");

    mipos_t_suspend();

    return 0;
}

/* ------------------------------------------------------------------------- */

int main(void)
{
    mipos_bsp_configure_rs232(115200);

    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    for (;;) {
    }

    return 0;
}
