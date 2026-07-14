/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * Licensed under the MIT License.
 */

#ifndef MIPOS_QEMU_HOSTDISK_H__
#define MIPOS_QEMU_HOSTDISK_H__

#include "mipos_diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIPOS_QEMU_HOSTDISK_SECTOR_SIZE 512u

void mipos_qemu_hostdisk_set_image(const char* path);
const char* mipos_qemu_hostdisk_image(void);
const mipos_disk_t* mipos_qemu_hostdisk_driver(void);

#ifdef __cplusplus
}
#endif

#endif /* MIPOS_QEMU_HOSTDISK_H__ */
