/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#ifndef __mipos_FS_H__
#define __mipos_FS_H__


/* -------------------------------------------------------------------------- */

#include "mipos_kernel.h"


/* -------------------------------------------------------------------------- */

/* CONSTANTS */
#define mipos_fs_MAX_VERSION 1
#define mipos_fs_MIN_VERSION 0

#ifndef mipos_fs_STORAGE_AREA_SIZE
#define mipos_fs_STORAGE_AREA_SIZE   (1024) /*BYTES*/
#endif // !1

#ifndef mipos_fs_CLUSTER_SIZE
#define mipos_fs_CLUSTER_SIZE    32
#endif

#define mipos_fs_CLUSTERS_COUNT  \
  (mipos_fs_STORAGE_AREA_SIZE/mipos_fs_CLUSTER_SIZE)


#ifndef mipos_fs_MAX_N_OF_FILES
#define mipos_fs_MAX_N_OF_FILES 8
#endif

#ifndef mipos_fs_MAX_FILENAME_LENGTH
#define mipos_fs_MAX_FILENAME_LENGTH 8
#endif

#ifndef mipos_fs_MAX_VOLUME_LENGTH
#define mipos_fs_MAX_VOLUME_LENGTH   8
#endif


/* -------------------------------------------------------------------------- */

typedef int(*mipos_io_write_t)(const char* srcbuf, uint32_t bsize, uint32_t doffset);
typedef int(*mipos_io_read_t)(char* dstbuf, uint32_t bsize, uint32_t soffset);


/* -------------------------------------------------------------------------- */

typedef struct _mipos_io_dev_t {
    mipos_io_write_t io_write;
    mipos_io_read_t  io_read;
}
mipos_io_dev_t;


/* -------------------------------------------------------------------------- */

typedef enum _mipos_fs_err_code_t {
    mipos_fs_ERRCODE__SUCCESS = 0,

    mipos_fs_ERRCODE__NO_ENOUGH_FREE_BLOCKS,

    mipos_fs_ERRCODE__BAD_FILENAME,
    mipos_fs_ERRCODE__FILENAME_ALREADY_PRESENT,
    mipos_fs_ERRCODE__FILE_NOT_FOUND,

    mipos_fs_ERRCODE__DELETE_FAILED,
    mipos_fs_ERRCODE__WRITE_FAILED,
    mipos_fs_ERRCODE__CLOSE_FAILED,
    mipos_fs_ERRCODE__READ_FAILED,
    mipos_fs_ERRCODE__READ_NOT_COMPLETED,
    mipos_fs_ERRCODE__SEEK_FAILED,

} mipos_fs_err_code_t;


/* -------------------------------------------------------------------------- */

typedef enum _mipos_fs_seek_t {
    mipos_fs_SEEK_BEGIN,
    mipos_fs_SEEK_CURRENT,
    mipos_fs_SEEK_END
}
mipos_fs_seek_t;


/* -------------------------------------------------------------------------- */

typedef enum _mipos_fs_fopenmode_t {
    mipos_fs_FILE_OPEN_READ,
    mipos_fs_FILE_OPEN_WRITE
}
mipos_fs_fopenmode_t;


/* -------------------------------------------------------------------------- */

typedef uint16_t mipos_fs_file_handle_t;
typedef uint16_t mipos_fs_cluster_t;


/* -------------------------------------------------------------------------- */

//! File descriptor 
typedef struct _mipos_fs_fd_t {
    char filename[mipos_fs_MAX_FILENAME_LENGTH];
    uint32_t size;

    mipos_fs_cluster_t first_cluster;

    bool_t erased;
    bool_t readonly;

} mipos_fs_fd_t;


//! File status descriptor
typedef struct _mipos_fs_ctl_t {
    uint32_t seek_pointer;
    bool_t eof_while_read;
    bool_t open;
    bool_t locked;

    mipos_fs_fopenmode_t file_open_mode;

} mipos_fs_ctl_t;


/* -------------------------------------------------------------------------- */

//! File system
typedef struct _mipos_fs_ftbl_t {
    char volume_description[mipos_fs_MAX_VOLUME_LENGTH];

    uint8_t maj_version;
    uint8_t min_version;

    mipos_fs_fd_t file_fs_desc[mipos_fs_MAX_N_OF_FILES];
    mipos_fs_cluster_t cluster_tbl[mipos_fs_CLUSTERS_COUNT];

} mipos_fs_ftbl_t;


/* -------------------------------------------------------------------------- */

#define mipos_fs_SIZE (((sizeof(mipos_fs_ftbl_t)/(mipos_fs_CLUSTER_SIZE))+1)*mipos_fs_CLUSTER_SIZE * 2)


/* -------------------------------------------------------------------------- */

#ifndef mipos_fs_OFFSET 
#define mipos_fs_OFFSET 0
#endif


/* -------------------------------------------------------------------------- */

//! File system
typedef struct _mipos_fs_t {
    mipos_io_dev_t  io_dev;       // device driver specific io API
    mipos_fs_ctl_t  file_ctl[mipos_fs_MAX_N_OF_FILES];
    mipos_fs_ftbl_t ftbl;         // sync with device device   
    mipos_mtx_t     lock;
} mipos_fs_t;


/* -------------------------------------------------------------------------- */

/* EXPORTED FUNCTIONS  */


/* -------------------------------------------------------------------------- */

void mipos_fs_lock_access(mipos_fs_t* fs_ptr);
void mipos_fs_unlock_access(mipos_fs_t* fs_ptr);


/* -------------------------------------------------------------------------- */

void mipos_fs_call_os_scheduler(void);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_get_last_error function retrieves the last error code */
uint32_t mipos_fs_get_last_error(void);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_setup function initializes the file system arena */
bool_t mipos_fs_setup(mipos_fs_t* fs_ptr);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_get_label function get the volume description */
bool_t mipos_fs_get_label(
    mipos_fs_t* fs_ptr,
    char* volume_description
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_mkfs function formats the filesystem
 *  and assigns it the new volume_description */
bool_t mipos_fs_mkfs(
    mipos_fs_t* fs_ptr,
    const char* volume_description
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_ffirst function searches a directory for a
    file whose name matches the specified file name */
bool_t mipos_fs_ffirst(
    mipos_fs_t* fs_ptr,
    const char* filename,
    mipos_fs_file_handle_t* file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_fnext function continues a file search
    from a  previous call to the mipos_fs_ffirst function */
bool_t mipos_fs_fnext(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t* file_handle);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_fexist function returns true if the file
    is found in the filesystem */
bool_t mipos_fs_fexist(
    mipos_fs_t*         fs_ptr,
    const char*     filename
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_open_file function opens an existent file */
bool_t mipos_fs_open_file(
    mipos_fs_t* fs_ptr,
    const char* filename,
    mipos_fs_file_handle_t* file_handle,
    mipos_fs_fopenmode_t mode
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_close_file function closes an open file */
bool_t mipos_fs_close_file(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_create_file function creates a new file */
bool_t mipos_fs_create_file(
    mipos_fs_t* fs_ptr,
    const char* filename,
    mipos_fs_file_handle_t* file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_delete_file function erases an existent file */
bool_t mipos_fs_delete_file(
    mipos_fs_t* fs_ptr,
    const char* filename
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_get_fd function retrieves information of a file */
bool_t mipos_fs_get_fd(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t file_handle,
    mipos_fs_fd_t* file_fs_desc
);


/** The mipos_fs_get_fd function retrieves information of a file status */
bool_t mipos_fs_get_ctl(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t file_handle,
    mipos_fs_ctl_t* file_fctl
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_get_fsize function retrieves the size (in bytes) of a file */
bool_t mipos_fs_get_fsize(
    mipos_fs_t* fs_ptr,
    const char* filename,
    uint32_t* file_size
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_lock_file locks function an unlocked file */
bool_t mipos_fs_lock_file(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_unlock_file function unlocks a locked file */
bool_t mipos_fs_unlock_file(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_write_file function writes a buffer in a
 *  file starting at "seek pointer" position */
bool_t mipos_fs_write_file(
    mipos_fs_t* fs_ptr,
    const char* source_buffer,
    uint32_t* wbytes,
    mipos_fs_file_handle_t file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_read_file function reads a buffer from
 *  a file starting at "seek pointer" position */
bool_t mipos_fs_read_file(
    mipos_fs_t* fs_ptr,
    char* dest_buffer,
    uint32_t* rbytes,
    mipos_fs_file_handle_t file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_seek_file function moves "seek pointer" position */
bool_t mipos_fs_seek_file(
    mipos_fs_t* fs_ptr,
    mipos_fs_seek_t seek_code,
    int* seekpos,
    mipos_fs_file_handle_t file_handle
);


/* -------------------------------------------------------------------------- */

/** The mipos_fs_rename_file function renames a file */
bool_t mipos_fs_rename_file(
    mipos_fs_t* fs_ptr,
    const char* filename,
    const char* new_filename
);


/* -------------------------------------------------------------------------- */

#endif
