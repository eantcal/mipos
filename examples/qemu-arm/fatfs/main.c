/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * Licensed under the MIT License.
 */

// FatFs demo for QEMU lm3s6965evb.
//
// The block device is a host file opened through QEMU semihosting. The run
// scripts create/copy qemu-fatfs.img in build-qemu-arm and start QEMU from
// that directory with semihosting enabled.

#include "ff.h"
#include "mipos.h"
#include "mipos_bsp.h"
#include "mipos_diskio.h"
#include "mipos_qemu_hostdisk.h"

#include <string.h>

static FATFS g_fat_fs;
static char root_stack[8192];
static char g_curr_dir[128] = "/";

static void make_path(char* out, unsigned int out_size, const char* name)
{
    out[0] = 0;

    if (!name || !name[0]) {
        strncpy(out, g_curr_dir, out_size - 1u);
        out[out_size - 1u] = 0;
        return;
    }

    if (name[0] == '/') {
        strncpy(out, name, out_size - 1u);
        out[out_size - 1u] = 0;
        return;
    }

    strncpy(out, g_curr_dir, out_size - 1u);
    out[out_size - 1u] = 0;

    if (strcmp(out, "/") != 0) {
        strncat(out, "/", out_size - strlen(out) - 1u);
    }

    strncat(out, name, out_size - strlen(out) - 1u);
}

static int exec_cmd_pwd(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    mipos_printf("%s\n", g_curr_dir);
    return 0;
}

static int exec_cmd_ls(int argc, char* argv[])
{
    DIR dir;
    FILINFO info;
    FRESULT res;
    char path[128];

    make_path(path, sizeof(path), argc > 1 ? argv[1] : 0);

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        mipos_printf("ls: cannot open %s (%d)\n", path, (int)res);
        return -1;
    }

    for (;;) {
        res = f_readdir(&dir, &info);
        if (res != FR_OK || info.fname[0] == 0) {
            break;
        }

        mipos_printf("%c %10lu %s\n",
                     (info.fattrib & AM_DIR) ? 'd' : '-',
                     (unsigned long)info.fsize,
                     info.fname);
    }

    f_closedir(&dir);
    return res == FR_OK ? 0 : -1;
}

static int exec_cmd_cd(int argc, char* argv[])
{
    DIR dir;
    char path[128];

    if (argc < 2) {
        strcpy(g_curr_dir, "/");
        return 0;
    }

    make_path(path, sizeof(path), argv[1]);

    if (f_opendir(&dir, path) != FR_OK) {
        mipos_printf("cd: cannot open %s\n", path);
        return -1;
    }

    f_closedir(&dir);
    strncpy(g_curr_dir, path, sizeof(g_curr_dir) - 1u);
    g_curr_dir[sizeof(g_curr_dir) - 1u] = 0;
    return 0;
}

static int exec_cmd_cat(int argc, char* argv[])
{
    FIL file;
    UINT bread;
    char path[128];
    char buffer[65];
    FRESULT res;

    if (argc < 2) {
        mipos_printf("usage: cat <file>\n");
        return -1;
    }

    make_path(path, sizeof(path), argv[1]);
    res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        mipos_printf("cat: cannot open %s (%d)\n", path, (int)res);
        return -1;
    }

    do {
        res = f_read(&file, buffer, sizeof(buffer) - 1u, &bread);
        if (res != FR_OK) {
            f_close(&file);
            return -1;
        }

        buffer[bread] = 0;
        mipos_printf("%s", buffer);
    } while (bread == sizeof(buffer) - 1u);

    mipos_printf("\n");
    f_close(&file);
    return 0;
}

static int exec_cmd_write(int argc, char* argv[])
{
    FIL file;
    UINT written;
    char path[128];
    FRESULT res;

    if (argc < 3) {
        mipos_printf("usage: write <file> <text>\n");
        return -1;
    }

    make_path(path, sizeof(path), argv[1]);
    res = f_open(&file, path, FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) {
        mipos_printf("write: cannot open %s (%d)\n", path, (int)res);
        return -1;
    }

    res = f_write(&file, argv[2], strlen(argv[2]), &written);
    if (res == FR_OK) {
        res = f_write(&file, "\r\n", 2, &written);
    }
    f_close(&file);

    return res == FR_OK ? 0 : -1;
}

static mipos_console_cmd_t user_cmd_list[] = {
    { "ls", exec_cmd_ls, "ls [dir]          - list FAT directory" },
    { "cd", exec_cmd_cd, "cd [dir]          - change FAT directory" },
    { "pwd", exec_cmd_pwd, "pwd              - print FAT directory" },
    { "cat", exec_cmd_cat, "cat <file>       - print FAT file" },
    { "write", exec_cmd_write, "write <file> <s> - append text" },
    CMD_LIST_END
};

static int root_task(task_param_t param)
{
    mipos_console_init_t cinit = { 0 };
    FRESULT mount_result;

    (void)param;

    mipos_add_disk_driver(mipos_qemu_hostdisk_driver());

    cinit.prompt = "\nmipOS-fat>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;

    mipos_console_register_cmd_list(user_cmd_list);
    mipos_console_init(&cinit);

    mount_result = f_mount(&g_fat_fs, "0:", 1);

    mipos_printf("mipOS FatFs QEMU host disk demo\n");
    mipos_printf("image: %s\n", mipos_qemu_hostdisk_image());
    if (mount_result != FR_OK) {
        mipos_printf("f_mount failed: %d\n", (int)mount_result);
        mipos_printf("Run through scripts/run-qemu with semihosting enabled.\n");
    } else {
        mipos_printf("mounted 0:\n");
        exec_cmd_ls(0, 0);
    }

    mipos_t_suspend();
    return 0;
}

int main(void)
{
    mipos_bsp_configure_rs232(115200);
    mipos_start(root_task, 0, root_stack, sizeof(root_stack));

    for (;;) {
    }

    return 0;
}
