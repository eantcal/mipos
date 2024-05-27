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

// Simple RAM Disk driver

mipos_tfs_t mipos_tiny_fs;
static char ramdisk_ptr[mipos_tfs_SIZE + mipos_tfs_OFFSET +
                        mipos_tfs_STORAGE_AREA_SIZE] = { 0 };

static int disk_write(const char* srcbuf, uint32_t bsize, uint32_t doffset)
{
    memcpy(&ramdisk_ptr[doffset], srcbuf, bsize);
    return 0;
}

static int disk_read(char* dstbuf, uint32_t bsize, uint32_t soffset)
{
    memcpy(dstbuf, &ramdisk_ptr[soffset], bsize);
    return 0;
}

/* ------------------------------------------------------------------------- */

static int exec_command(int argc, char* argv[])
{
    mipos_tfs_file_handle_t file_handle = 0;
    mipos_tfs_fd_t fd;
    char volume_dsc[mipos_tfs_MAX_VOLUME_LENGTH] = { 0 };

    mipos_tfs_get_label(&mipos_tiny_fs, volume_dsc);

    // list all files in the RAMDISK
    if (0 == strcmp(argv[0], "ls")) {

        mipos_printf("Volume label %s\n", volume_dsc);

        if (mipos_tfs_ffirst(&mipos_tiny_fs, "*", &file_handle)) {

            do {
                mipos_tfs_get_fd(&mipos_tiny_fs, file_handle, &fd);
                mipos_printf("%s %8i bytes\n", fd.filename, fd.size);
                if (!mipos_tfs_fnext(&mipos_tiny_fs, &file_handle))
                    break;
            } while (1);
        }
    }

    // Show all files and related content
    if (0 == strcmp(argv[0], "cat")) {
        mipos_printf("Volume label %s\n", volume_dsc);

        if (mipos_tfs_ffirst(&mipos_tiny_fs, "*", &file_handle)) {

            do {
                mipos_tfs_FILE* f = 0;
                uint32_t i = 0;

                mipos_tfs_get_fd(&mipos_tiny_fs, file_handle, &fd);
                f = mipos_tfs_fopen(fd.filename, "r");
                if (f) {
                    mipos_printf("%s %8i bytes\n", fd.filename, fd.size);
                    for (i = 0; i < fd.size; ++i) {
                        mipos_printf("%c", mipos_tfs_fgetc(f));
                    }
                    mipos_printf("\n");

                    mipos_tfs_fclose(f);
                }

                if (!mipos_tfs_fnext(&mipos_tiny_fs, &file_handle))
                    break;
            } while (1);
        }
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

static mipos_console_cmd_t user_cmd_list[] = {
    { "ls", exec_command, "ls - shows list of files" },
    { "cat", exec_command, "cat - shows list and content of files" },

    CMD_LIST_END
};

/* ------------------------------------------------------------------------- */

static char root_stack[4 * 1024];
int root_task(task_param_t param)
{
    char volume_dsc[mipos_tfs_MAX_VOLUME_LENGTH] = { 0 };
    mipos_tfs_FILE* f = 0;

    mipos_console_init_t cinit = { 0 };
    cinit.prompt = "\nmipOS>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;
    cinit.recv_cbk = 0;

    mipos_console_register_cmd_list(user_cmd_list);

    mipos_console_init(&cinit);

    mipos_tiny_fs.io_dev.io_read = disk_read;
    mipos_tiny_fs.io_dev.io_write = disk_write;

    mipos_tfs_setup(&mipos_tiny_fs);

    if (mipos_tfs_get_label(&mipos_tiny_fs, volume_dsc)) {
        if (strcmp(volume_dsc, "RAMDISK")) {
            mipos_tfs_mkfs(&mipos_tiny_fs, "RAMDISK");
        }
    }

    mipos_init_ramdisk_stdio(&mipos_tiny_fs);

    // Create two files, test1 and test2
    f = mipos_tfs_fopen("test1", "w");
    if (f) {
        mipos_tfs_fwrite("0123456789", 10, 1, f);
        mipos_tfs_fwrite("abcdefghij", 10, 1, f);
        mipos_tfs_fwrite("klmnopqrst", 10, 1, f);
        mipos_tfs_fwrite("uvwxyz0123", 10, 1, f);
        mipos_tfs_fwrite("ABCDEFGHIJ", 10, 1, f);
        mipos_tfs_fwrite("KLMNOPQRST", 10, 1, f);
        mipos_tfs_fclose(f);
    }

    f = mipos_tfs_fopen("test2", "w");
    if (f) {
        mipos_tfs_fwrite("9876543210", 10, 1, f);
        mipos_tfs_fwrite("ABCDEFGHIJ", 10, 1, f);
        mipos_tfs_fwrite("KLMNOPQRST", 10, 1, f);
        mipos_tfs_fwrite("UVWXYZ0123", 10, 1, f);
        mipos_tfs_fwrite("abcdefghij", 10, 1, f);
        mipos_tfs_fwrite("klmnopqrst", 10, 1, f);
        mipos_tfs_fclose(f);
    }

    mipos_t_suspend();

    return 0;
}

/* ------------------------------------------------------------------------- */

int main()
{
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));
    return 0;
}
