/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * Licensed under the MIT License.
 */

#ifndef MIPOS_QEMU_SEMIHOSTING_H__
#define MIPOS_QEMU_SEMIHOSTING_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    MIPOS_QEMU_SEMIHOST_OPEN_RB = 1,
    MIPOS_QEMU_SEMIHOST_OPEN_RBP = 3
};

int mipos_qemu_semihost_open(const char* path, int mode);
int mipos_qemu_semihost_close(int handle);
int mipos_qemu_semihost_seek(int handle, unsigned long position);
long mipos_qemu_semihost_flen(int handle);
int mipos_qemu_semihost_read(int handle, void* buffer, size_t length);
int mipos_qemu_semihost_write(int handle,
                              const void* buffer,
                              size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MIPOS_QEMU_SEMIHOSTING_H__ */
