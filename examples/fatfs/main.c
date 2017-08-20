/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* --------------------------------------------------------------------------- */

#include "mipos.h"
#include "mipos_diskio.h"
#include "diskio.h"
#include "ff.h"


/* --------------------------------------------------------------------------- */


static char * ramdisk_ptr = 0;
static size_t ramdisk_size = 0;
#define SECTOR_SIZE 512

// target-simulation ramdisk image data
static char ramdisk_fname[1024] = "disk.img" /* default name */;

/* --------------------------------------------------------------------------- */

static FATFS g_fat_fs;
static char root_stack[4 * 1024];
static char g_curr_dir[1024] = "/";
static volatile int _disk_status = MIPOS_DISK_STS_NOINIT;


/* -------------------------------------------------------------------------- */

/** Get drive status
*
* @param pdrv Physical drive nmuber to identify the drive
* @return 1: drive not initialized, 2: no medium in the drive, 3: write protected, 0: Ok
*/
static
int ramdisk_status(mipos_pdrv_t pdrv) {
    if (pdrv != 0) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    return _disk_status;
}


/* -------------------------------------------------------------------------- */

/** Inidialize a drive
*
* @param pdrv Physical drive nmuber to identify the drive
*
* @return 1: drive not initialized, 2: no medium in the drive, 3: write protected, 0: Ok
*/
static
int ramdisk_init(mipos_pdrv_t pdrv) {
    size_t img_size = 0;

    if (pdrv != 0) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    // read from host (linux/window) filesystem a ramdisk image
    FILE * f = fopen(ramdisk_fname, "rb");

    if (!f) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    fseek(f, 0, SEEK_END);
    img_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if ((int)img_size < 1) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    ramdisk_size = img_size;

    ramdisk_ptr = malloc(img_size);

    if (!ramdisk_ptr) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    if (fread(ramdisk_ptr, 1, img_size, f)<1) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    fclose(f);
    
    _disk_status = 0;

    return _disk_status;
}


/* -------------------------------------------------------------------------- */

/** Read sectors
*
* @param pdrv Physical drive nmuber to identify the drive
* @param char *buff Data buffer to store read data
* @param sector Start sector in LBA
* @param uint32_t count  Number of sectors to read
*
* @return  0: Successful, 1: R/W Error, 2: Write Protected, 3: Not Ready, 4 Invalid Parameter
*/
static
int ramdisk_read(mipos_pdrv_t pdrv, char *buf, mipos_sec_t sector, mipos_sec_t count)
{
    /* If not initialized */
    if (pdrv != 0 || _disk_status & MIPOS_DISK_STS_NOINIT) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    /* Read data from ramdisk */
    memcpy(buf, (&ramdisk_ptr[sector * SECTOR_SIZE]), count * SECTOR_SIZE);

    return MIPOS_DISK_RES_OK;
}


/* -------------------------------------------------------------------------- */

/** Write sectors
*
* @param pdrv Physical drive nmuber to identify the drive
* @param char *buff Data buffer to store read data
* @param sector Start sector in LBA
* @param uint32_t count  Number of sectors to read
*
* @return  0: Successful, 1: R/W Error, 2: Write Protected, 3: Not Ready, 4 Invalid Parameter
*/
static
int ramdisk_write(mipos_pdrv_t pdrv, const char *buf, mipos_sec_t sector, mipos_sec_t count)
{
    /* If not initialized */
    if (pdrv != 0 || _disk_status & MIPOS_DISK_STS_NOINIT) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    /* Write data into ram disk */
    memcpy( (&ramdisk_ptr[sector * SECTOR_SIZE]), buf, count * SECTOR_SIZE);

    return MIPOS_DISK_RES_OK;
}


/* -------------------------------------------------------------------------- */

/** Control device
*
* @param pdrv Physical drive nmuber to identify the drive
* @param cmd Control code
* @param buf Buffer to send/receive control data
*
* @return  0: Successful, 1: R/W Error, 2: Write Protected, 3: Not Ready, 4 Invalid Parameter
*/
static
int ramdisk_ioctl(mipos_pdrv_t pdrv, mipos_ioctl_cmd_t cmd, void *buf)
{
    switch (cmd) {
        case GET_SECTOR_COUNT:	/* Get sector count (u32) */
            *(uint32_t *)buf = ramdisk_size / SECTOR_SIZE;
            break;
        case GET_BLOCK_SIZE:	/* Get erase block size in sectors (u32) */
            *(uint32_t *)buf = 32;
            break;
        default:
            break;
    }

    return MIPOS_DISK_RES_OK;
}


/* --------------------------------------------------------------------------- */

int exec_cmd_ls(int argc, char *argv[])
{
    unsigned long n_of_files = 0;
    unsigned long n_of_dirs = 0;
    unsigned long totsize = 0;

    int res = 0;
    DIR dir_desc;
    FILINFO file_info = { 0 };
    
    const char * s_dir = g_curr_dir;

    if (argc > 1 && argv[1]) {
        s_dir = argv[1];
    }

    if (f_opendir(&dir_desc, s_dir) != FR_OK) {
        mipos_printf("'%s' not found\n", s_dir);
        return -1;
    }

    while (1) {
        if (f_readdir(&dir_desc, &file_info) != FR_OK) {
            res = -1;
            break;
        }

        // If the file name is blank -> no more file to list
        if (!file_info.fname[0]) {
            break;
        }

        // Print the entry information on a single line with formatting
        // to show the attributes, date, time, size, and name.
        mipos_printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u bytes  %s\n",
            (file_info.fattrib & AM_DIR) ? 'd' : '-',
            (file_info.fattrib & AM_RDO) ? 'r' : '-',
            (file_info.fattrib & AM_HID) ? 'h' : '-',
            (file_info.fattrib & AM_SYS) ? 's' : '-',
            (file_info.fattrib & AM_ARC) ? 'a' : '-',
            (unsigned int)((file_info.fdate >> 9) + 1980),
            (unsigned int)((file_info.fdate >> 5) & 15),
            (unsigned int)(file_info.fdate & 31),
            (unsigned int)((file_info.ftime >> 11)),
            (unsigned int)((file_info.ftime >> 5) & 63),
            (unsigned int)(file_info.fsize),
            file_info.fname);

        
        // If the attribute is directory, then increment the directory count.
        if (file_info.fattrib & AM_DIR) {
            n_of_dirs++;
        }
        else {
            n_of_files++;
            totsize += file_info.fsize;
        }

    }

    if (res != 0) { 
        mipos_printf("\nError reading the directory content\n");
    }
    else {
        mipos_printf("\n%4u File(s),%10u bytes total\n%4u Dir(s)\n",
            (unsigned int) n_of_files, 
            (unsigned int) totsize, 
            (unsigned int) n_of_dirs);
    }

    f_closedir(&dir_desc);

    return res;
}


/* -------------------------------------------------------------------------- */

static int exec_cmd_cat(int argc, char* argv[])
{
    int res = 0;
    size_t bread = 0;
    FIL file;
    char buf[ 64 ] = { 0 };

    if (argc<2) {
        return -1;
    }

    int curdir_len = strlen(g_curr_dir); 

    if((curdir_len + strlen(argv[1]) + 1) >= sizeof(g_curr_dir)) {
        return -1;
    }

    if(curdir_len<1 || g_curr_dir[curdir_len-1] != '/') {
        strcat(g_curr_dir, "/");
    }

    strcat(g_curr_dir, argv[1]);

    res = f_open(&file, g_curr_dir, FA_READ);

    g_curr_dir[curdir_len] = 0;

    if (res != FR_OK) {
       return -1;
    }

    do {
        if (FR_OK != f_read(&file, buf, sizeof(buf)-1, &bread)) {
            f_close(&file);
            return -1;
        }

        buf[bread] = 0;
        mipos_printf("%s", buf);
    }
    while(bread == (sizeof(buf) - 1));

    f_close(&file);

    return 0;
}


/* -------------------------------------------------------------------------- */

static int exec_cmd_chdir(int argc, char* argv[])
{
    int res = 0;
    DIR dir_desc;
    FILINFO file_info = { 0 };
    char *new_dir;

    if (argc < 2) {
        return 0;
    }
    
    new_dir = argv[1];

    if (strcmp(new_dir, ".") == 0) {
        return 0;
    }

    if (new_dir[0] == '/') {
        
        if (strlen(new_dir) >= sizeof(g_curr_dir)) {
            return -1;
        }

        else {
            if (FR_OK == f_opendir(&dir_desc, new_dir)) {
                strcpy(g_curr_dir, new_dir);
                f_closedir(&dir_desc);
                return 0;
            }
        }
    }
    else {
        int pos = 0, dirlen = strlen(g_curr_dir);

        if (strcmp(new_dir, "..") == 0 && strcmp(g_curr_dir, "/")) {

            pos = dirlen;
            if (pos-- < 1) {
                return -1;
            }

            while ((g_curr_dir[pos] != '/') && (pos > 1)) {
                --pos;
            }

            g_curr_dir[pos] = 0;

            if (FR_OK == f_opendir(&dir_desc, g_curr_dir)) {
                f_closedir(&dir_desc);
                return 0;
            }

            // reset old dir
            g_curr_dir[pos] = '/';
        }
        else {
            if ((strlen(g_curr_dir) + 
                    strlen(new_dir) + 1 /* '/' */) >= sizeof(g_curr_dir)) 
            {
                return -1;
            }
            else  {
                if (strcmp(g_curr_dir, "/")) {
                    strcat(g_curr_dir, "/");
                }
                
                strcat(g_curr_dir, new_dir);

                if (FR_OK == f_opendir(&dir_desc, g_curr_dir)) {
                    f_closedir(&dir_desc);
                    return 0;
                }

                // reset old dir
                g_curr_dir[dirlen] = 0;
            }
        }

        return -1;
    }

    return 0;
}


/* -------------------------------------------------------------------------- */

static int exec_cmd_pwd(int argc, char* argv[])
{
    mipos_printf("%s\n", g_curr_dir);
    return 0;
}

/* --------------------------------------------------------------------------- */

static
mipos_console_cmd_t user_cmd_list[] = {
    { "ls", exec_cmd_ls,      "ls             - shows list of files" },
    { "cd", exec_cmd_chdir,   "cd <dir>       - changes current directory" },
    { "pwd", exec_cmd_pwd,    "pwd            - show current directory" },
    { "cat", exec_cmd_cat,    "cat <filename> - show file content" },

    CMD_LIST_END
};


/* --------------------------------------------------------------------------- */

int root_task(task_param_t param)
{

    // Register the ramdisk driver
    mipos_disk_t ramdisk_drv = {
        0,                // logical unit
        ramdisk_init,
        ramdisk_status,
        ramdisk_read,
        ramdisk_write,
        ramdisk_ioctl
    };

    mipos_add_disk_driver(&ramdisk_drv);

    
    // Configure the console
    mipos_console_init_t cinit = { 0 };
    cinit.prompt = "\nmipOS>";
    cinit.end_line_char = '\r';
    cinit.flags = CONSOLE_FLG_ECHO | CONSOLE_TX_CRLF;
    cinit.recv_cbk = 0;

    mipos_console_register_cmd_list(user_cmd_list);
    mipos_console_init(&cinit);
        
    // Mount the ramdisk
    if (FR_OK != f_mount(&g_fat_fs, "0:" /*logical drive 0*/, 1 /*mount now*/)) {
        mipos_puts("f_mount failed\n");
    }

    mipos_t_suspend();

    return 0;
}


/* --------------------------------------------------------------------------- */

int main(int argc, char * argv[])
{
    if (argc > 1 && argv[1]) {
        // Change default ramdisk image file name
        strncpy(ramdisk_fname, argv[1], sizeof(ramdisk_fname) - 1);
    }

    mipos_start(root_task, 0, root_stack, sizeof(root_stack));
    return 0;
}


