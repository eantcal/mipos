/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifdef ENABLE_MIPOS_STDIO

/* ------------------------------------------------------------------------- */

#include "mipos_stdio.h"

/* ------------------------------------------------------------------------- */

static mipos_tfs_t* _fs_ptr = 0;

/* ------------------------------------------------------------------------- */

void mipos_init_ramdisk_stdio(void* fs_ptr)
{
    _fs_ptr = fs_ptr;
}

/* ------------------------------------------------------------------------- */

mipos_tfs_FILE* mipos_tfs_fopen(const char* name, const char* mode)
{
    mipos_tfs_file_handle_t file_handle;
    int lm;
    char m[8] = { 0 };

    strncpy(m, mode, sizeof(m) - 1);
    lm = (int)strlen(m);

    /*
      m == "r"
      Opens for reading. If the file does not exist or cannot be found,
      the fopen call fails.

      m == "w"
      Opens an empty file for writing. If the given file exists,
      its contents are destroyed.

      m == "r+"
      Opens for both reading and writing. (The file must exist.)

      m == "w+"
      Opens an empty file for both reading and writing.
      If the given file exists, its contents are destroyed.

    */

    // Binary mode is ignored (binary mode is assumed for default)
    if (!lm && m[lm - 1] == 'b') {
        m[lm - 1] = 0;
    }

    if (strcmp(m, "r") == 0 || strcmp(m, "r+") == 0) {
        if (mipos_tfs_open_file(_fs_ptr,
                                name,
                                &file_handle,
                                strcmp(m, "r") == 0
                                  ? mipos_tfs_FILE_OPEN_READ
                                  : mipos_tfs_FILE_OPEN_WRITE)) {
            mipos_tfs_FILE* f = (mipos_tfs_FILE*)malloc(sizeof(mipos_tfs_FILE));

            if (f) {
                f->_handle = file_handle;
                f->_read_only = TRUE;
            }

            return f;
        }
    } else if (strcmp(m, "w+") == 0 || strcmp(m, "w") == 0) {
        mipos_tfs_delete_file(_fs_ptr, name);

        if (mipos_tfs_create_file(_fs_ptr, name, &file_handle)) {
            mipos_tfs_FILE* f = (mipos_tfs_FILE*)malloc(sizeof(mipos_tfs_FILE));

            if (f) {
                f->_handle = file_handle;
                f->_read_only = FALSE;
            }

            return f;
        }
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

int mipos_tfs_fclose(mipos_tfs_FILE* f)
{
    if (f && (mipos_tfs_close_file(_fs_ptr, f->_handle))) {
        free(f);
        return 0; // success
    }

    return EOF;
}

/* ------------------------------------------------------------------------- */

int mipos_tfs_feof(mipos_tfs_FILE* f)
{
    mipos_tfs_ctl_t fctl;

    if (!f->_read_only) {
        return 0;
    }

    if (mipos_tfs_get_ctl(_fs_ptr, f->_handle, &fctl)) {
        return fctl.eof_while_read;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

long mipos_tfs_ftell(mipos_tfs_FILE* f)
{
    mipos_tfs_ctl_t fctl;

    if (mipos_tfs_get_ctl(_fs_ptr, f->_handle, &fctl)) {
        return fctl.seek_pointer;
    }

    return EOF;
}

/* ------------------------------------------------------------------------- */

int mipos_tiny_fseek(mipos_tfs_FILE* f, long pos, int origin)
{
    /*
      The fseek function moves the file pointer (if any)
      associated with stream to a new location that is
      offset bytes from origin. The next operation
      on the stream takes place at the new location.
      On a stream open for update, the next operation
      can be either a read or a write.

      The argument origin must be one of the following constants:
        SEEK_CUR Current position of file pointer
        SEEK_END End of file
        SEEK_SET Beginning of file
    */

    mipos_tfs_seek_t fseek_code;
    int new_pos;

    switch (origin) {
        case SEEK_SET:
            fseek_code = mipos_tfs_SEEK_BEGIN;
            break;

        case SEEK_CUR:
            fseek_code = mipos_tfs_SEEK_CURRENT;
            break;

        case SEEK_END:
            fseek_code = mipos_tfs_SEEK_END;
            break;

        default:
            return 0;
    }

    new_pos = pos;

    /*
      If successful, fseek (mipos_fseek) returns 0.
      Otherwise, it returns a nonzero value; in our case,
      returns new_pos: you could use this value for debugging purpose
    */
    return mipos_tfs_seek_file(_fs_ptr, fseek_code, &new_pos, f->_handle)
             ? 0
             : new_pos;
}

/* ------------------------------------------------------------------------- */

size_t mipos_tfs_fread(void* buffer,
                       size_t size,
                       size_t count,
                       mipos_tfs_FILE* f)
{
    mipos_tfs_fd_t fd;
    uint32_t rbytes;

    rbytes = (uint32_t)(size * count);

    if (mipos_tfs_read_file(_fs_ptr, (char*)buffer, &rbytes, f->_handle)) {
        return rbytes / size;
    }

    mipos_tfs_get_fd(_fs_ptr, f->_handle, &fd);

    return rbytes < size * count ? rbytes / size : 0;
}

/* ------------------------------------------------------------------------- */

size_t mipos_tfs_fwrite(const void* buffer,
                        size_t size,
                        size_t count,
                        mipos_tfs_FILE* f)
{
    uint32_t wbytes;

    wbytes = (uint32_t)(size * count);

    if (mipos_tfs_write_file(
          _fs_ptr, (const char*)buffer, &wbytes, f->_handle)) {
        return wbytes / size;
    }

    return wbytes < (size * count) ? wbytes / size : 0;
}

/* ------------------------------------------------------------------------- */

int mipos_tfs_fgetc(mipos_tfs_FILE* stream)
{
    char c = 0;
    size_t ret = mipos_tfs_fread(&c, 1, 1, stream);
    return ret == 1 ? c : (int)ret;
}

/* ------------------------------------------------------------------------- */

int mipos_tfs_rename(const char* source_file, const char* dest_file)
{
    return mipos_tfs_rename_file(_fs_ptr, source_file, dest_file) ? 0 : -1;
}

/* ------------------------------------------------------------------------- */

int mipos_tfs_remove(const char* filename)
{
    return mipos_tfs_delete_file(_fs_ptr, filename) ? 0 : -1;
}

/* ------------------------------------------------------------------------- */

int mipos_tfs_filesize(const char* filename)
{
    uint32_t filesize = 0;
    int ret = mipos_tfs_get_fsize(_fs_ptr, filename, &filesize);
    return ret ? filesize : -1;
}

/* ------------------------------------------------------------------------- */

#endif //! ENABLE_MIPOS_STDIO

/* ------------------------------------------------------------------------- */
