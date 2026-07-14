/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * Licensed under the MIT License.
 */

#include "mipos_qemu_semihosting.h"

#include <stdint.h>
#include <string.h>

enum
{
    SYS_OPEN = 0x01,
    SYS_CLOSE = 0x02,
    SYS_WRITE = 0x05,
    SYS_READ = 0x06,
    SYS_SEEK = 0x0A,
    SYS_FLEN = 0x0C
};

static int semihost_call(int operation, void* argument)
{
    register int r0 __asm("r0") = operation;
    register void* r1 __asm("r1") = argument;

    __asm volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");

    return r0;
}

int mipos_qemu_semihost_open(const char* path, int mode)
{
    uintptr_t args[3];

    if (!path) {
        return -1;
    }

    args[0] = (uintptr_t)path;
    args[1] = (uintptr_t)mode;
    args[2] = (uintptr_t)strlen(path);

    return semihost_call(SYS_OPEN, args);
}

int mipos_qemu_semihost_close(int handle)
{
    uintptr_t args[1];

    args[0] = (uintptr_t)handle;

    return semihost_call(SYS_CLOSE, args);
}

int mipos_qemu_semihost_seek(int handle, unsigned long position)
{
    uintptr_t args[2];

    args[0] = (uintptr_t)handle;
    args[1] = (uintptr_t)position;

    return semihost_call(SYS_SEEK, args);
}

long mipos_qemu_semihost_flen(int handle)
{
    uintptr_t args[1];

    args[0] = (uintptr_t)handle;

    return semihost_call(SYS_FLEN, args);
}

int mipos_qemu_semihost_read(int handle, void* buffer, size_t length)
{
    uintptr_t args[3];
    int not_transferred;

    args[0] = (uintptr_t)handle;
    args[1] = (uintptr_t)buffer;
    args[2] = (uintptr_t)length;

    not_transferred = semihost_call(SYS_READ, args);
    if (not_transferred < 0 || (size_t)not_transferred > length) {
        return -1;
    }

    return (int)(length - (size_t)not_transferred);
}

int mipos_qemu_semihost_write(int handle, const void* buffer, size_t length)
{
    uintptr_t args[3];
    int not_transferred;

    args[0] = (uintptr_t)handle;
    args[1] = (uintptr_t)buffer;
    args[2] = (uintptr_t)length;

    not_transferred = semihost_call(SYS_WRITE, args);
    if (not_transferred < 0 || (size_t)not_transferred > length) {
        return -1;
    }

    return (int)(length - (size_t)not_transferred);
}
