/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * Licensed under the MIT License.
 */

#include "mipos_qemu_hostdisk.h"

#include "diskio.h"
#include "mipos_qemu_semihosting.h"

#include <stdint.h>
#include <string.h>

static char g_image_path[128] = "qemu-fatfs.img";
static int g_handle = -1;
static uint32_t g_sector_count = 0;
static int g_status = MIPOS_DISK_STS_NOINIT;

static int hostdisk_status(mipos_pdrv_t pdrv);
static int hostdisk_init(mipos_pdrv_t pdrv);
static int hostdisk_read(mipos_pdrv_t pdrv,
                         char* buf,
                         mipos_sec_t sector,
                         mipos_sec_t count);
static int hostdisk_write(mipos_pdrv_t pdrv,
                          const char* buf,
                          mipos_sec_t sector,
                          mipos_sec_t count);
static int hostdisk_ioctl(mipos_pdrv_t pdrv,
                          mipos_ioctl_cmd_t cmd,
                          void* buf);

static const mipos_disk_t g_hostdisk_driver = {
    0,
    hostdisk_init,
    hostdisk_status,
    hostdisk_read,
    hostdisk_write,
    hostdisk_ioctl
};

void mipos_qemu_hostdisk_set_image(const char* path)
{
    if (!path || !path[0]) {
        return;
    }

    strncpy(g_image_path, path, sizeof(g_image_path) - 1u);
    g_image_path[sizeof(g_image_path) - 1u] = 0;
}

const char* mipos_qemu_hostdisk_image(void)
{
    return g_image_path;
}

const mipos_disk_t* mipos_qemu_hostdisk_driver(void)
{
    return &g_hostdisk_driver;
}

static int hostdisk_status(mipos_pdrv_t pdrv)
{
    if (pdrv != 0) {
        return MIPOS_DISK_STS_NODISK;
    }

    return g_status;
}

static int hostdisk_init(mipos_pdrv_t pdrv)
{
    long image_size;

    if (pdrv != 0) {
        return MIPOS_DISK_STS_NODISK;
    }

    if (g_handle >= 0) {
        g_status = 0;
        return g_status;
    }

    g_handle =
      mipos_qemu_semihost_open(g_image_path, MIPOS_QEMU_SEMIHOST_OPEN_RBP);
    if (g_handle < 0) {
        g_status = MIPOS_DISK_STS_NOINIT;
        return g_status;
    }

    image_size = mipos_qemu_semihost_flen(g_handle);
    if (image_size <= 0 ||
        (image_size % MIPOS_QEMU_HOSTDISK_SECTOR_SIZE) != 0) {
        mipos_qemu_semihost_close(g_handle);
        g_handle = -1;
        g_status = MIPOS_DISK_STS_NOINIT;
        return g_status;
    }

    g_sector_count =
      (uint32_t)(image_size / MIPOS_QEMU_HOSTDISK_SECTOR_SIZE);
    g_status = 0;

    return g_status;
}

static int hostdisk_check_range(mipos_sec_t sector, mipos_sec_t count)
{
    if (count == 0) {
        return 0;
    }

    if (sector >= g_sector_count || count > (g_sector_count - sector)) {
        return -1;
    }

    return 0;
}

static int hostdisk_seek_sector(mipos_sec_t sector)
{
    return mipos_qemu_semihost_seek(
      g_handle,
      (unsigned long)sector * MIPOS_QEMU_HOSTDISK_SECTOR_SIZE);
}

static int hostdisk_read(mipos_pdrv_t pdrv,
                         char* buf,
                         mipos_sec_t sector,
                         mipos_sec_t count)
{
    const uint32_t bytes = count * MIPOS_QEMU_HOSTDISK_SECTOR_SIZE;

    if (pdrv != 0 || (g_status & MIPOS_DISK_STS_NOINIT) || !buf) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    if (hostdisk_check_range(sector, count) != 0 ||
        hostdisk_seek_sector(sector) != 0) {
        return MIPOS_DISK_RES_INVPAR;
    }

    if (mipos_qemu_semihost_read(g_handle, buf, bytes) != (int)bytes) {
        return MIPOS_DISK_RES_ERROR;
    }

    return MIPOS_DISK_RES_OK;
}

static int hostdisk_write(mipos_pdrv_t pdrv,
                          const char* buf,
                          mipos_sec_t sector,
                          mipos_sec_t count)
{
    const uint32_t bytes = count * MIPOS_QEMU_HOSTDISK_SECTOR_SIZE;

    if (pdrv != 0 || (g_status & MIPOS_DISK_STS_NOINIT) || !buf) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    if (hostdisk_check_range(sector, count) != 0 ||
        hostdisk_seek_sector(sector) != 0) {
        return MIPOS_DISK_RES_INVPAR;
    }

    if (mipos_qemu_semihost_write(g_handle, buf, bytes) != (int)bytes) {
        return MIPOS_DISK_RES_ERROR;
    }

    return MIPOS_DISK_RES_OK;
}

static int hostdisk_ioctl(mipos_pdrv_t pdrv, mipos_ioctl_cmd_t cmd, void* buf)
{
    if (pdrv != 0 || (g_status & MIPOS_DISK_STS_NOINIT)) {
        return MIPOS_DISK_RES_NOTRDY;
    }

    switch (cmd) {
    case CTRL_SYNC:
        return MIPOS_DISK_RES_OK;
    case GET_SECTOR_COUNT:
        if (!buf) {
            return MIPOS_DISK_RES_INVPAR;
        }
        *(uint32_t*)buf = g_sector_count;
        return MIPOS_DISK_RES_OK;
    case GET_SECTOR_SIZE:
        if (!buf) {
            return MIPOS_DISK_RES_INVPAR;
        }
        *(uint16_t*)buf = MIPOS_QEMU_HOSTDISK_SECTOR_SIZE;
        return MIPOS_DISK_RES_OK;
    case GET_BLOCK_SIZE:
        if (!buf) {
            return MIPOS_DISK_RES_INVPAR;
        }
        *(uint32_t*)buf = 1;
        return MIPOS_DISK_RES_OK;
    default:
        return MIPOS_DISK_RES_INVPAR;
    }
}
