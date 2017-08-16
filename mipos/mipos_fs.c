/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


 /* -------------------------------------------------------------------------- */

#ifdef ENABLE_MIPOS_FS


/* -------------------------------------------------------------------------- */

#include "mipos_fs.h"


/* -------------------------------------------------------------------------- */

static unsigned int mipos_fs_last_error_code_t = 0;


/* -------------------------------------------------------------------------- */

#define mipos_fs_FREE_CLUSTER           0xFFFF
#define mipos_fs_LAST_ALLOCATED_CLUSTER 0xFFFD
#define mipos_fs_RESERVED_CLUSTER       0xFFFC
#define mipos_fs_EMPTY_CLUSTER          0xFFFB
#define mipos_fs_INVALID_FILE_HANDLE    0xFFFF


/* -------------------------------------------------------------------------- */

void mipos_fs_lock_access(mipos_fs_t* fs_ptr)
{
    mipos_mu_lock(&fs_ptr->lock);
}


/* -------------------------------------------------------------------------- */

void mipos_fs_unlock_access(mipos_fs_t* fs_ptr)
{
    mipos_mu_unlock(&fs_ptr->lock);
}


/* -------------------------------------------------------------------------- */

void mipos_fs_call_os_scheduler(void)
{
    mipos_tm_wkafter(0);
}


/* -------------------------------------------------------------------------- */
/* PRIVATE FUNCTIONS */
/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

static
int mipos_update_fs_on_disk(mipos_fs_t* fs_ptr)
{
    return fs_ptr->io_dev.io_write(
        (const char*)&fs_ptr->ftbl,
        sizeof(mipos_fs_ftbl_t),
        mipos_fs_OFFSET);
}


/* -------------------------------------------------------------------------- */

static
mipos_fs_cluster_t mipos_fs_get_cluster_val(
    mipos_fs_t*          fs_ptr,
    mipos_fs_cluster_t   current
)
{
    mipos_fs_cluster_t cluster_value;

    // Get a cluster value from the cluster table
    mipos_fs_lock_access(fs_ptr);
    cluster_value = MIPOS_LE_U16(fs_ptr->ftbl.cluster_tbl[current]);
    mipos_fs_unlock_access(fs_ptr);

    return cluster_value;
}


/* -------------------------------------------------------------------------- */

static
void set_cluster_val(
    mipos_fs_t*     fs_ptr,
    mipos_fs_cluster_t  current,
    mipos_fs_cluster_t  cluster_value
)
{
    // Set a cluster value in the cluster table
    mipos_fs_lock_access(fs_ptr);
    fs_ptr->ftbl.cluster_tbl[current] = MIPOS_LE_U16(cluster_value);
    mipos_fs_unlock_access(fs_ptr);
}


/* -------------------------------------------------------------------------- */

static
bool_t reserve_cluster(
    mipos_fs_t* fs_ptr,
    mipos_fs_cluster_t* free_cluster
)
{
    mipos_fs_cluster_t i = 0;

    bool_t ret_val = FALSE;

    // Search a free cluster and reserve it
    mipos_fs_lock_access(fs_ptr);

    for (i = 0; i < mipos_fs_CLUSTERS_COUNT; ++i) {
        mipos_fs_call_os_scheduler();

        if (MIPOS_LE_U16(fs_ptr->ftbl.cluster_tbl[i]) == mipos_fs_FREE_CLUSTER) {

            MIPOS_LE_U16(fs_ptr->ftbl.cluster_tbl[i]) = mipos_fs_RESERVED_CLUSTER;
            *free_cluster = MIPOS_LE_U16(i);
            ret_val = TRUE;

            break;
        }
    }

    mipos_fs_unlock_access(fs_ptr);

    return ret_val;
}


/* -------------------------------------------------------------------------- */

static
bool_t get_next_cluster(
    mipos_fs_t* fs_ptr,
    mipos_fs_cluster_t current,
    mipos_fs_cluster_t* next_cluster
)
{
    mipos_fs_cluster_t ret_val;

    if (current >= mipos_fs_CLUSTERS_COUNT) {
        return FALSE;
    }

    // Get the linked cluster to the "current" one
    ret_val = mipos_fs_get_cluster_val(fs_ptr, current);

    if (ret_val >= mipos_fs_CLUSTERS_COUNT) {
        return FALSE;
    }

    *next_cluster = ret_val;

    return TRUE;
}


/* -------------------------------------------------------------------------- */

static
bool_t link_cluster(
    mipos_fs_t*    fs_ptr,
    mipos_fs_cluster_t current,
    mipos_fs_cluster_t link_to
)
{
    if ((current >= mipos_fs_CLUSTERS_COUNT) ||
        (link_to >= mipos_fs_CLUSTERS_COUNT))
    {
        return FALSE;
    }

    set_cluster_val(fs_ptr,
        current,
        link_to);

    return TRUE;
}


/* -------------------------------------------------------------------------- */

static
void mark_cluster_last(
    mipos_fs_t*          fs_ptr,
    mipos_fs_cluster_t       cluster
)
{
    set_cluster_val(fs_ptr,
        cluster,
        mipos_fs_LAST_ALLOCATED_CLUSTER);
}


/* -------------------------------------------------------------------------- */

inline
void mark_cluster_free(
    mipos_fs_t*       fs_ptr,
    mipos_fs_cluster_t    cluster
)
{
    set_cluster_val(fs_ptr,
        cluster,
        mipos_fs_FREE_CLUSTER);
}


/* -------------------------------------------------------------------------- */

inline
void mark_cluster_reserved(
    mipos_fs_t*          fs_ptr,
    mipos_fs_cluster_t       cluster
)
{
    set_cluster_val(fs_ptr,
        cluster,
        mipos_fs_RESERVED_CLUSTER);
}


/* -------------------------------------------------------------------------- */

static
bool_t get_cluster_seek_pt(
    mipos_fs_t* fs_ptr,
    mipos_fs_fd_t* file_fs_desc,
    mipos_fs_ctl_t* file_ctl,
    mipos_fs_cluster_t*  cluster
)
{
    mipos_fs_cluster_t next_cluster,
        current_cluster,
        clusters_num,
        cluster_counter = 0;

    bool_t break_loop = FALSE;
    uint32_t seek_pointer;

    seek_pointer = file_ctl->seek_pointer;
    clusters_num =
        (mipos_fs_cluster_t)(seek_pointer / mipos_fs_CLUSTER_SIZE);

    next_cluster = file_fs_desc->first_cluster;

    do
    {
        current_cluster = next_cluster;

        if (cluster_counter == clusters_num)
            break;

        break_loop =
            bool_t_cast(!get_next_cluster(fs_ptr,
                current_cluster,
                &next_cluster));

        cluster_counter++;
    } while (!break_loop);

    *cluster = current_cluster;

    return bool_t_cast(cluster_counter == clusters_num);
}


/* -------------------------------------------------------------------------- */

static
int write_cluster(
    mipos_fs_t* fs_ptr,
    mipos_fs_cluster_t cluster,
    uint32_t cluster_offset,
    const char* source_buffer,

    // IN num of byte to write, OUT bytes written
    uint32_t* wbytes
)
{
    uint32_t offset = 0;
    uint32_t destination_pointer = 0;
    uint32_t bytes_written = 0, left_cluster_size = 0;
    int ret = 0;

    mipos_fs_lock_access(fs_ptr);

    offset = cluster_offset % mipos_fs_CLUSTER_SIZE;
    destination_pointer = 
        cluster * mipos_fs_CLUSTER_SIZE + offset + 
        mipos_fs_SIZE + 
        mipos_fs_OFFSET;
        
    left_cluster_size = mipos_fs_CLUSTER_SIZE - offset;

    // Is the left part of cluster buffer enough to copy all
    //  *wbytes data ? If No copy only left_cluster_size 
    // data bytes 
    bytes_written =
        (*wbytes) < left_cluster_size ?
        (*wbytes) : left_cluster_size;

    if (bytes_written) {
        ret = fs_ptr->io_dev.io_write(
            source_buffer,
            bytes_written,
            destination_pointer);
    }

    *wbytes = bytes_written;

    mipos_fs_unlock_access(fs_ptr);

    return ret;
}


/* -------------------------------------------------------------------------- */

static
int read_cluster(
    mipos_fs_t* fs_ptr,
    mipos_fs_cluster_t cluster,
    uint32_t cluster_offset,
    char* destination_buffer,

    // IN num of byte to read, OUT bytes read
    uint32_t*          rbytes
)
{
    uint32_t offset = 0;
    uint32_t source_pointer = 0;
    uint32_t bytes_read = 0, left_cluster_size = 0;
    int ret = 0;

    mipos_fs_lock_access(fs_ptr);

    offset = cluster_offset % mipos_fs_CLUSTER_SIZE;

    source_pointer = 
        cluster * mipos_fs_CLUSTER_SIZE + offset + 
        mipos_fs_SIZE + 
        mipos_fs_OFFSET;

    left_cluster_size = mipos_fs_CLUSTER_SIZE - offset;

    // Is the left part of cluster buffer enough to copy all
      //  *rbytes data ?
    // If No copy only left_cluster_size data bytes
    bytes_read =
        (*rbytes) < left_cluster_size ?
        (*rbytes) : left_cluster_size;

    if (bytes_read) {
        ret = fs_ptr->io_dev.io_read(
            destination_buffer, 
            bytes_read, 
            source_pointer);
    }

    *rbytes = bytes_read;

    mipos_fs_unlock_access(fs_ptr);

    return ret;
}


/* -------------------------------------------------------------------------- */

static
bool_t mipos_fs_check_file_name(const char* filename)
{
    int len, i;
    len = (int)strlen(filename);

    if (len >= mipos_fs_MAX_FILENAME_LENGTH) {
        return FALSE;
    }

    for (i = 0; i < len; ++i) {
        char c = filename[i];

        if (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'Z')) ||
            ((c >= 'a') && (c <= 'z')) ||
            (c == '_') || (c == '-')
            || (c == '.'))
        {
            continue;
        }
        else {
            return FALSE;
        }
    }

    return TRUE;
}


/* -------------------------------------------------------------------------- */


/* PUBLIC FUNCTIONS DEFINITIONs */

/* -------------------------------------------------------------------------- */

uint32_t mipos_fs_get_last_error(void)
{
    return mipos_fs_last_error_code_t;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_setup(mipos_fs_t* fs_ptr)
{
    int i = 0;
    mipos_fs_fd_t* fd_ptr = 0;
    mipos_fs_ctl_t* fctl_ptr = 0;

    if (!fs_ptr->io_dev.io_read ||
        !fs_ptr->io_dev.io_write)
    {
        return FALSE;
    }

    if (0 > fs_ptr->io_dev.io_read(
        (char*)&fs_ptr->ftbl,
        sizeof(fs_ptr->ftbl),
        mipos_fs_OFFSET))
    {
        return FALSE;
    }

    // Reset initial state for each file
    for (i = 0; i < mipos_fs_MAX_N_OF_FILES; ++i) {
        fctl_ptr = &(fs_ptr->file_ctl[i]);
        memset(fctl_ptr, 0, sizeof(mipos_fs_ctl_t));
    }

    mipos_update_fs_on_disk(fs_ptr);

    mipos_mu_init(&fs_ptr->lock);

    return TRUE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_get_label(
    mipos_fs_t* fs_ptr,
    char* volume_description
)
{
    mipos_fs_lock_access(fs_ptr);
    strcpy(volume_description, fs_ptr->ftbl.volume_description);
    mipos_fs_unlock_access(fs_ptr);

    return TRUE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_seek_file(
    mipos_fs_t*         fs_ptr,
    mipos_fs_seek_t         seek_code,
    int*                    seekpos,
    mipos_fs_file_handle_t  file_handle
)
{
    mipos_fs_fd_t* fd_ptr = 0;
    mipos_fs_ctl_t* fctl_ptr = 0;
    uint32_t size;

    // Lock the file
    if (!mipos_fs_lock_file(fs_ptr, file_handle)) {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__SEEK_FAILED;
        return FALSE;
    }

    // Get the size of the file
    mipos_fs_lock_access(fs_ptr);
    fd_ptr = &(fs_ptr->ftbl.file_fs_desc[file_handle]);
    fctl_ptr = &(fs_ptr->file_ctl[file_handle]);
    size = fd_ptr->size;
    mipos_fs_unlock_access(fs_ptr);

    // Test for the seek_code
    switch (seek_code) {
    case mipos_fs_SEEK_BEGIN:
        mipos_fs_lock_access(fs_ptr);

        if ((*seekpos) > 0 && (((uint32_t)(*seekpos)) < size)) {
            fctl_ptr->seek_pointer = *seekpos;
        }
        else {
            mipos_fs_last_error_code_t =
                mipos_fs_ERRCODE__SEEK_FAILED;

            mipos_fs_unlock_file(fs_ptr, file_handle);
            mipos_fs_unlock_access(fs_ptr);
            return FALSE;
        }

        mipos_fs_unlock_access(fs_ptr);
        break;

    case mipos_fs_SEEK_CURRENT:
        mipos_fs_lock_access(fs_ptr);
        fctl_ptr->seek_pointer += *seekpos;
        mipos_fs_unlock_access(fs_ptr);
        break;

    case mipos_fs_SEEK_END:
        mipos_fs_lock_access(fs_ptr);
        fctl_ptr->seek_pointer = fd_ptr->size + *seekpos;
        mipos_fs_unlock_access(fs_ptr);
        break;

    default:
        // The seek code is not valid (all the previous 
        // cases are skipped) the function fails
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__SEEK_FAILED;

        mipos_fs_unlock_file(fs_ptr, file_handle);
        return FALSE;
    }

    mipos_fs_unlock_file(fs_ptr, file_handle);

    return TRUE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_read_file(
    mipos_fs_t*              fs_ptr,
    char*                    dest_buffer,
    uint32_t*                rbytes,
    mipos_fs_file_handle_t   file_handle
)
{
    uint32_t cluster_offset = 0;
    uint32_t bytes_to_read = 0;
    uint32_t bytes_read = 0;

    uint32_t tmp_rbytes;

    mipos_fs_fd_t file_fs_desc, *fd_ptr = 0;
    mipos_fs_ctl_t file_ctl, *fclt_ptr = 0;

    mipos_fs_cluster_t cluster,
        next_cluster;

    char* dest_pointer = dest_buffer;

    bool_t ret_val = TRUE;

    // Lock the file
    if (!mipos_fs_lock_file(fs_ptr, file_handle)) {
        goto RD_EXCEPTION_HANDLER;
    }

    // Get the file descriptor
    mipos_fs_get_fd(fs_ptr, file_handle, &file_fs_desc);
    mipos_fs_get_ctl(fs_ptr, file_handle, &file_ctl);

    if ((!file_ctl.open) ||
        (file_ctl.seek_pointer >= MIPOS_LE_U32(file_fs_desc.size)))
    {
        goto RD_EXCEPTION_HANDLER;
    }

    // Get the cluster pointed by the current "seek pointer"
    if (!get_cluster_seek_pt(fs_ptr,
        &file_fs_desc,
        &file_ctl,
        &cluster))
    {
        goto RD_EXCEPTION_HANDLER;
    }

    // Calculate the cluster offset using 
    // the file "seek pointer"
    cluster_offset =
        file_ctl.seek_pointer % mipos_fs_CLUSTER_SIZE;

    bytes_to_read = *rbytes;

    if (
        (bytes_to_read + file_ctl.seek_pointer) >
        MIPOS_LE_U32(file_fs_desc.size))
    {
        mipos_fs_lock_access(fs_ptr);
        fclt_ptr = &(fs_ptr->file_ctl[file_handle]);
        fclt_ptr->eof_while_read = TRUE;
        mipos_fs_unlock_access(fs_ptr);

        bytes_to_read =
            MIPOS_LE_U32(file_fs_desc.size) - file_ctl.seek_pointer;
    }

    tmp_rbytes = bytes_to_read;

    for (;;) {
        // Read from the cluster min between the bytes_to_read 
        // and remainig bytes to read
        read_cluster(fs_ptr,
            cluster,
            cluster_offset,
            dest_pointer,
            &tmp_rbytes);

        // If an error occours terminate the loop
        if (tmp_rbytes == 0) {
            mipos_fs_last_error_code_t =
                mipos_fs_ERRCODE__READ_NOT_COMPLETED;

            ret_val = FALSE;
            break;
        }

        // Reset cluster_offset (set it to begin of cluster)
        cluster_offset = 0;

        // Increment bytes_read and dest_pointer 
        // of the read bytes count
        bytes_read += tmp_rbytes;
        dest_pointer += tmp_rbytes;

        // Move the seek pointer of the read bytes count
        mipos_fs_lock_access(fs_ptr);

        fs_ptr->file_ctl[file_handle].seek_pointer += tmp_rbytes;

        mipos_fs_unlock_access(fs_ptr);

        // Break the loop when all bytes are read
        if (bytes_read >= bytes_to_read) {
            break;
        }

        // Get the next linked cluster of the file
        else if (!get_next_cluster(fs_ptr,
            cluster,
            &next_cluster))
        {
            mipos_fs_last_error_code_t =
                mipos_fs_ERRCODE__READ_NOT_COMPLETED;

            ret_val = FALSE;
            break;
        }

        cluster = next_cluster;
        // Set tmp_rbytes to the left bytes count
        tmp_rbytes = bytes_to_read - bytes_read;
    }

    // Set *rbytes to the read bytes count
    *rbytes = bytes_read;

    // Unlock the file and return the ret_val
    mipos_fs_unlock_file(fs_ptr, file_handle);
    return ret_val;

RD_EXCEPTION_HANDLER:
    mipos_fs_last_error_code_t =
        mipos_fs_ERRCODE__READ_FAILED;

    mipos_fs_unlock_file(fs_ptr, file_handle);
    return FALSE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_write_file(
    mipos_fs_t*          fs_ptr,
    const char*           source_buffer,
    uint32_t*             wbytes,
    mipos_fs_file_handle_t   file_handle
)
{
    mipos_fs_fd_t file_fs_desc, *fd_ptr = 0;
    mipos_fs_ctl_t file_ctl, *fctl_ptr = 0;

    mipos_fs_cluster_t cluster, prev_cluster;
    const char* source_ptr;
    uint32_t tot_wbytes, tmp_wbytes;
    bool_t end_loop = FALSE;

    mipos_fs_cluster_t first_cluster = 0;

    uint32_t cluster_offset = 0;
    uint32_t bytes_to_write = 0;
    uint32_t bytes_written = 0;
    uint32_t fsize = 0;
    
    int ret = 0;

    // Lock the file
    if (!mipos_fs_lock_file(fs_ptr, file_handle))
        goto WR_EXCEPTION_HANDLER;

    // Get the file descriptor
    mipos_fs_get_fd(fs_ptr, file_handle, &file_fs_desc);
    mipos_fs_get_ctl(fs_ptr, file_handle, &file_ctl);

    fsize = MIPOS_LE_U32(file_fs_desc.size);

    if ((file_fs_desc.readonly) ||
        (file_ctl.file_open_mode != mipos_fs_FILE_OPEN_WRITE)
        ||
        (file_ctl.open == 0))
    {
        goto WR_EXCEPTION_HANDLER;
    }

    bytes_to_write = *wbytes;

    // The seek pointer cannot be after the size of the file
    if (file_ctl.seek_pointer > fsize)
        goto WR_EXCEPTION_HANDLER;

    cluster_offset =
        file_ctl.seek_pointer % mipos_fs_CLUSTER_SIZE;

    tot_wbytes = *wbytes;
    source_ptr = source_buffer;

    // If it is required, allocate a new cluster ?
    if ((file_ctl.seek_pointer == fsize)
        && (cluster_offset == 0))
    {
        // Are we overwriting an existing file ?
        if (file_ctl.seek_pointer > 0) {

            // We must link last allocated 
            // cluster with the new one
            if (!get_cluster_seek_pt(fs_ptr,
                &file_fs_desc,
                &file_ctl,
                &cluster))
            {
                goto WR_EXCEPTION_HANDLER;
            }

            if (!reserve_cluster(fs_ptr, &first_cluster))
                goto WR_EXCEPTION_HANDLER;

            if (!link_cluster(fs_ptr,
                cluster,
                first_cluster))
            {
                goto WR_EXCEPTION_HANDLER;
            }
        }
        else if (!reserve_cluster(fs_ptr,
            &first_cluster))
        {
            goto WR_EXCEPTION_HANDLER;
        }
    }
    else {
        // Get cluster at the seek_pointer
        if (!get_cluster_seek_pt(fs_ptr,
            &file_fs_desc,
            &file_ctl,
            &first_cluster))
        {
            goto WR_EXCEPTION_HANDLER;
        }
    }

    cluster = first_cluster;

    do {
        tmp_wbytes = tot_wbytes;

        // Write the cluster
        ret = write_cluster(fs_ptr,
            cluster,
            cluster_offset,
            source_ptr,
            &tmp_wbytes);

        if (ret != 0) {
            end_loop = TRUE;
        }

        cluster_offset = 0;
        tot_wbytes -= tmp_wbytes;

        // Move the "seek pointer"
        mipos_fs_lock_access(fs_ptr);
        fd_ptr = &(fs_ptr->ftbl.file_fs_desc[file_handle]);
        fctl_ptr = &(fs_ptr->file_ctl[file_handle]);

        fctl_ptr->seek_pointer += tmp_wbytes;
        mipos_fs_unlock_access(fs_ptr);

        // No more bytes written ?
        if (tot_wbytes == 0) {

            // Is the "seek pointer" out of range ?
            if (fctl_ptr->seek_pointer > fd_ptr->size) {
                // Set it to last position, 
                // mark this cluster as the last of the file 
                fd_ptr->size = fctl_ptr->seek_pointer;
                mark_cluster_last(fs_ptr, cluster);
            }

            // terminate the loop
            end_loop = TRUE;
        }
        else {
            // Remember the current cluster
            prev_cluster = cluster;

            // Have we to allocate a new cluster ?
            if (fctl_ptr->seek_pointer > fd_ptr->size) {
                fd_ptr->size += bytes_written;

                if (!reserve_cluster(fs_ptr, &cluster)) {
                    // Mark this cluster as
                    // the last of the file and terminate the loop
                    mark_cluster_last(fs_ptr, prev_cluster);
                    end_loop = TRUE;
                }
            }
            // NO, get the next cluster
            else if (!get_next_cluster(fs_ptr,
                prev_cluster,
                &cluster))
            {
                // If it is non-possible mark this cluster as
                // the last of the file and terminate the loop
                mark_cluster_last(fs_ptr,
                    prev_cluster);
                end_loop = TRUE;
            }

            // If loop continues link current 
            // cluster with previous one
            if (!end_loop) {
                link_cluster(fs_ptr, prev_cluster, cluster);
            }
        }

        // Move the source buffer pointer to the new position
        // adding the number of written bytes
        source_ptr += tmp_wbytes;

    } while (end_loop == FALSE);

    if (ret == 0) {

        bytes_written = bytes_to_write - tot_wbytes;

        // If the first cluster of 
        // file descriptor was not set, do it
        mipos_fs_lock_access(fs_ptr);

        fd_ptr = &(fs_ptr->ftbl.file_fs_desc[file_handle]);

        if (MIPOS_LE_U16(fd_ptr->first_cluster) >= mipos_fs_CLUSTERS_COUNT) {
            fd_ptr->first_cluster = MIPOS_LE_U16(first_cluster);
        }

        mipos_fs_unlock_access(fs_ptr);

        *wbytes = bytes_written;
    }

    // Unlock file
    mipos_fs_unlock_file(fs_ptr, file_handle);

    if (ret != 0) {
        return FALSE;
    }

    // If no all bytes was written return FALSE, TRUE else
    return bool_t_cast(tot_wbytes == 0);

WR_EXCEPTION_HANDLER:
    *wbytes = 0;

    mipos_fs_last_error_code_t =
        mipos_fs_ERRCODE__WRITE_FAILED;

    mipos_fs_unlock_file(fs_ptr, file_handle);

    return FALSE;
}


/* -------------------------------------------------------------------------- */

static char _zero_buf[mipos_fs_CLUSTER_SIZE] = { 0 };

bool_t mipos_fs_mkfs(mipos_fs_t* fs_ptr, const char* volume_description)
{
    mipos_fs_file_handle_t i;
    mipos_fs_cluster_t j;
    mipos_fs_fd_t* fd_ptr = 0;
    mipos_fs_ctl_t* fctl_ptr = 0;
    int ret = 0;
    
    mipos_fs_lock_access(fs_ptr);

    // Set the volume label for the file system
    // (if the volume description is NULL, 
    //this label is filled with the null character)
    if (volume_description) {
        strncpy(fs_ptr->ftbl.volume_description,
            volume_description,
            mipos_fs_MAX_VOLUME_LENGTH);
    }
    else  {
        memset(fs_ptr->ftbl.volume_description,
            0,
            sizeof volume_description);
    }

    // Set the version 0.1
    fs_ptr->ftbl.maj_version = mipos_fs_MAX_VERSION;
    fs_ptr->ftbl.min_version = mipos_fs_MIN_VERSION;

    // Clean the file descriptor table
    for (i = 0; i < mipos_fs_MAX_N_OF_FILES; ++i) {
        fd_ptr = &(fs_ptr->ftbl.file_fs_desc[i]);
        fctl_ptr = &fs_ptr->file_ctl[i];

        memset(fctl_ptr, 0, sizeof(mipos_fs_ctl_t));
        memset((void*)fd_ptr, 0, sizeof(mipos_fs_fd_t));

        fd_ptr->erased = TRUE;

        mipos_fs_call_os_scheduler();
    }

    // Format the cluster arena
    for (j = 0; j < mipos_fs_CLUSTERS_COUNT; ++j) {
        fs_ptr->ftbl.cluster_tbl[j] = MIPOS_LE_U16(mipos_fs_FREE_CLUSTER);
        
        ret = fs_ptr->io_dev.io_write(
            _zero_buf, 
            mipos_fs_CLUSTER_SIZE, 
            j*mipos_fs_CLUSTER_SIZE + mipos_fs_OFFSET);

        mipos_fs_call_os_scheduler();

        if (ret != 0)
            break;
    }   
    
    mipos_fs_unlock_access(fs_ptr);

    if (ret == 0) {
        ret = mipos_update_fs_on_disk(fs_ptr);
    }

    return ret == 0 ? TRUE : FALSE;
};


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_rename_file(
    mipos_fs_t* fs_ptr,
    const char* filename,
    const char* new_filename
)
{
    // If the file exists and the new_filename is valid, 
    // the old file name is replaced with the new one
    mipos_fs_file_handle_t file_handle;

    if ((mipos_fs_ffirst(fs_ptr,
        filename,
        &file_handle))
        &&
        (mipos_fs_check_file_name(new_filename)))
    {
        mipos_fs_lock_access(fs_ptr);

        strncpy(
            &fs_ptr->ftbl.file_fs_desc->filename[file_handle],
            new_filename,
            mipos_fs_MAX_FILENAME_LENGTH);

        

        mipos_fs_unlock_access(fs_ptr);
    }
    else {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__BAD_FILENAME;

        return FALSE;
    }

    return mipos_update_fs_on_disk(fs_ptr) == 0 ? TRUE : FALSE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_ffirst(
    mipos_fs_t* fs_ptr,
    const char* filename,
    mipos_fs_file_handle_t* file_handle
)
{
    int erased, jolly_pos, slen;
    mipos_fs_file_handle_t i;

    char tmpFileName[mipos_fs_MAX_FILENAME_LENGTH] = { 0 };
    char tmpDescFileName[mipos_fs_MAX_FILENAME_LENGTH] = { 0 };

    if (!filename) {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__BAD_FILENAME;

        return FALSE;
    }

    slen = (int)strlen(filename);
    jolly_pos = slen;
    for (i = 0; i < slen; ++i) {
        if (filename[i] == '*') {
            jolly_pos = i;
            break;
        }
    }

    strncpy(tmpFileName,
        filename,
        jolly_pos);

    if (TRUE != mipos_fs_check_file_name(tmpFileName)) {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__BAD_FILENAME;

        return FALSE;
    }

    //For each element of the file descriptor table ...
    for (i = 0; i < mipos_fs_MAX_N_OF_FILES; ++i) {
        mipos_fs_lock_access(fs_ptr);
        erased = fs_ptr->ftbl.file_fs_desc[i].erased;
        mipos_fs_unlock_access(fs_ptr);

        // If the file descriptor is not-erased...
        if (!erased) {
            // Get the file name and compare with one passed as 
            // function's argument
            mipos_fs_lock_access(fs_ptr);

            strncpy(tmpDescFileName,
                fs_ptr->ftbl.file_fs_desc[i].filename,
                jolly_pos);

            mipos_fs_unlock_access(fs_ptr);

            // If it matchs update *file_handle with the fd index 
            // and return TRUE
            if (0 == strcmp(tmpDescFileName, tmpFileName)) {
                *file_handle = i;
                return TRUE;
            }
        }
    }

    return FALSE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_fnext(
    mipos_fs_t*          fs_ptr,
    mipos_fs_file_handle_t*  file_handle)
{
    int erased;
    mipos_fs_file_handle_t i;

    // Search a valid file descriptor starting from the 
    // *file_handle position 
    // *file_handle is set to the index of the first file 
    // descriptor that's found
    for (i = ++(*file_handle);
        i < mipos_fs_MAX_N_OF_FILES;
        ++i)
    {
        mipos_fs_lock_access(fs_ptr);
        erased = fs_ptr->ftbl.file_fs_desc[i].erased;
        mipos_fs_unlock_access(fs_ptr);

        if (!erased) {
            *file_handle = i;
            return TRUE;
        }
    }

    return FALSE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_fexist(
    mipos_fs_t*     fs_ptr,
    const char* filename
)
{
    mipos_fs_file_handle_t file_handle;

    return mipos_fs_ffirst(fs_ptr,
        filename,
        &file_handle);
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_lock_file(
    mipos_fs_t* fs_ptr, 
    mipos_fs_file_handle_t file_handle)
{
    bool_t ret_val = TRUE;
    mipos_fs_ctl_t * fctl_ptr;

    if (file_handle >= mipos_fs_MAX_N_OF_FILES) {
        return FALSE;
    }

    fctl_ptr = &(fs_ptr->file_ctl)[file_handle];

    if (!fctl_ptr->locked) {
        fctl_ptr->locked = 1;
    }
    else {
        ret_val = FALSE;
    }

    return ret_val;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_unlock_file(
    mipos_fs_t* fs_ptr, 
    mipos_fs_file_handle_t file_handle)
{
    mipos_fs_ctl_t * fctl_ptr;
    bool_t ret_val = TRUE;

    if (file_handle >= mipos_fs_MAX_N_OF_FILES) {
        return FALSE;
    }

    fctl_ptr = &(fs_ptr->file_ctl)[file_handle];

    if (fctl_ptr->locked) {
        fctl_ptr->locked = 0;
    }
    else {
        ret_val = FALSE;
    }

    return ret_val;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_get_ctl(
    mipos_fs_t*            fs_ptr,
    mipos_fs_file_handle_t file_handle,
    mipos_fs_ctl_t*        file_ctl
)
{
    if (file_handle >= mipos_fs_MAX_N_OF_FILES) {
        return FALSE;
    }

    // Copy the file descriptor in the buffer pointed 
    // by file_ctl pointer
    mipos_fs_lock_access(fs_ptr);
    memcpy(file_ctl,
        &fs_ptr->file_ctl[file_handle],
        sizeof(mipos_fs_ctl_t));
    mipos_fs_unlock_access(fs_ptr);

    return TRUE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_get_fd(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t file_handle,
    mipos_fs_fd_t* file_fs_desc
)
{
    if (file_handle >= mipos_fs_MAX_N_OF_FILES) {
        return FALSE;
    }

    // Copy the file descriptor in the buffer pointed 
    // by file_fs_desc pointer
    mipos_fs_lock_access(fs_ptr);
    memcpy(file_fs_desc,
        &fs_ptr->ftbl.file_fs_desc[file_handle],
        sizeof(mipos_fs_fd_t));
    mipos_fs_unlock_access(fs_ptr);

    // Return false if the file does not exist
    return bool_t_cast(file_fs_desc->erased == FALSE);
}




/* -------------------------------------------------------------------------- */

bool_t mipos_fs_get_fsize(
    mipos_fs_t*     fs_ptr,
    const char* filename,
    uint32_t*           file_size
)
{
    mipos_fs_file_handle_t file_handle;
    mipos_fs_fd_t file_fs_desc;

    if (!mipos_fs_ffirst(fs_ptr,
        filename,
        &file_handle))
    {
        return FALSE;
    }

    if (!mipos_fs_get_fd(fs_ptr,
        file_handle,
        &file_fs_desc))
    {
        return FALSE;
    }

    *file_size = file_fs_desc.size;

    return TRUE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_open_file(
    mipos_fs_t* fs_ptr,                 //fs ptr
    const char*     filename,   //file to open
    mipos_fs_file_handle_t* file_handle,//returend file handle
    mipos_fs_fopenmode_t    mode        //open mode 
)
{
    int erased, readonly, locked, open = 0;
    mipos_fs_file_handle_t i;
    char tmpFileName[mipos_fs_MAX_FILENAME_LENGTH] = { 0 };
    mipos_fs_fd_t* fd_ptr = 0;
    mipos_fs_ctl_t* fctl_ptr = 0;

    *file_handle = mipos_fs_INVALID_FILE_HANDLE;

    // Test if the file name is a valid one
    if (TRUE != mipos_fs_check_file_name(filename)) {
        return FALSE;
    }

    i = 0;

    // Search the file in the file descritor table
    for (; i < mipos_fs_MAX_N_OF_FILES; ++i) {
        // Get the attributes of the file
        mipos_fs_lock_access(fs_ptr);
        fd_ptr = &(fs_ptr->ftbl.file_fs_desc[i]);
        fctl_ptr = &(fs_ptr->file_ctl[i]);
        erased = fd_ptr->erased;
        readonly = fd_ptr->readonly;
        mipos_fs_unlock_access(fs_ptr);

        if (!erased) { // Skip erased files

            mipos_fs_lock_access(fs_ptr);
            strcpy(tmpFileName, fd_ptr->filename);
            open = fctl_ptr->open;
            locked = fctl_ptr->locked;
            mipos_fs_unlock_access(fs_ptr);

            if ((0 == strcmp(filename, tmpFileName)) &&
                (0 == locked && 0 == open))
                // File exists and it is not already opened and 
                // not locked...
            {
                // Do you have the privilege to write it ?
                if ((mode == mipos_fs_FILE_OPEN_WRITE) &&
                    (readonly))
                {
                    return FALSE;
                }

                // Ok, open it
                *file_handle = i;
                mipos_fs_lock_access(fs_ptr);
                fctl_ptr->open = 1;
                fctl_ptr->file_open_mode = mode;
                fctl_ptr->seek_pointer = 0;
                fctl_ptr->eof_while_read = FALSE;
                mipos_fs_unlock_access(fs_ptr);

                return TRUE;
            }
        }
    }

    return FALSE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_create_file(
    mipos_fs_t* fs_ptr,
    const char* filename,
    mipos_fs_file_handle_t* file_handle
)
{
    bool_t found;
    int erased = 0;
    mipos_fs_file_handle_t i;
    mipos_fs_fd_t* fd_ptr = 0;
    mipos_fs_ctl_t* fctl_ptr = 0;

    *file_handle = mipos_fs_INVALID_FILE_HANDLE;

    // Is filename valid ? NO, error
    if (!mipos_fs_check_file_name(filename)) {

        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__BAD_FILENAME;

        return FALSE;
    }

    // Is the file already existent ? NO, error
    if (mipos_fs_fexist(fs_ptr, filename)) {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__FILENAME_ALREADY_PRESENT;

        return FALSE;
    }

    found = FALSE;

    // Search a free file descriptor
    for (i = 0; i < mipos_fs_MAX_N_OF_FILES; ++i) {
        mipos_fs_lock_access(fs_ptr);
        fd_ptr = &(fs_ptr->ftbl.file_fs_desc[i]);
        fctl_ptr = &(fs_ptr->file_ctl[i]);
        erased = fd_ptr->erased;
        mipos_fs_unlock_access(fs_ptr);

        if (erased != 0) {
            found = TRUE;
            break;
        }
    }

    // If it is not found return 
    // mipos_fs_ERRCODE__NO_ENOUGH_FREE_BLOCKS error
    if (!found) {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__NO_ENOUGH_FREE_BLOCKS;
        return FALSE;
    }

    // Create update the file descriptor
    mipos_fs_lock_access(fs_ptr);
    fd_ptr->first_cluster = mipos_fs_EMPTY_CLUSTER;
    fd_ptr->erased = FALSE;
    fctl_ptr->open = 1;
    fctl_ptr->locked = 0;
    fd_ptr->readonly = 0;
    fd_ptr->size = 0;
    fctl_ptr->seek_pointer = 0;
    fctl_ptr->file_open_mode = mipos_fs_FILE_OPEN_WRITE;

    strncpy(fd_ptr->filename,
        filename,
        mipos_fs_MAX_FILENAME_LENGTH - 1);

    mipos_fs_unlock_access(fs_ptr);

    // The file handle is set to current file descriptor index
    *file_handle = i;

    return mipos_update_fs_on_disk(fs_ptr) == 0 ? TRUE : FALSE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_close_file(
    mipos_fs_t* fs_ptr,
    mipos_fs_file_handle_t file_handle
)
{
    mipos_fs_ctl_t* fctl_ptr = 0;

    if (file_handle >= mipos_fs_MAX_N_OF_FILES) {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__CLOSE_FAILED;

        return FALSE;
    }

    // Close the file and reset the "seek pointer"
    mipos_fs_lock_access(fs_ptr);
    fctl_ptr = &(fs_ptr->file_ctl[file_handle]);
    fctl_ptr->open = 0;
    fctl_ptr->seek_pointer = 0;
    mipos_fs_unlock_access(fs_ptr);

    return TRUE;
}


/* -------------------------------------------------------------------------- */

bool_t mipos_fs_delete_file(
    mipos_fs_t*     fs_ptr,
    const char* filename
)
{
    int readonly, erased;
    mipos_fs_cluster_t current_cluster, next_cluster;
    bool_t break_loop = FALSE;
    mipos_fs_fd_t* fd_ptr = 0;
    mipos_fs_file_handle_t file_handle;
    int ret = 0;

    // Get the handle of the file
    if (!mipos_fs_ffirst(fs_ptr, filename, &file_handle)) {
        mipos_fs_last_error_code_t = mipos_fs_ERRCODE__CLOSE_FAILED;

        return FALSE;
    }

    // Lock the file
    if (!mipos_fs_lock_file(fs_ptr, file_handle)) {
        mipos_fs_last_error_code_t =
            mipos_fs_ERRCODE__DELETE_FAILED;

        return FALSE;
    }

    // Get the attributes of the file
    mipos_fs_lock_access(fs_ptr);
    fd_ptr = &(fs_ptr->ftbl.file_fs_desc[file_handle]);
    readonly = fd_ptr->readonly;
    erased = fd_ptr->erased;
    mipos_fs_unlock_access(fs_ptr);

    // Is it an erased or a readonly file ?
    if ((erased) || (readonly)) {
        // Unlock it and return an error
        mipos_fs_unlock_file(fs_ptr, file_handle);

        mipos_fs_last_error_code_t = mipos_fs_ERRCODE__DELETE_FAILED;

        return FALSE;
    }

    // Clean the file descriptor 
    // and set the flag erased to TRUE
    mipos_fs_lock_access(fs_ptr);
    next_cluster = fd_ptr->first_cluster;
    memset((void*)fd_ptr, 0, sizeof(mipos_fs_fd_t));
    fd_ptr->erased = TRUE;
    mipos_fs_unlock_access(fs_ptr);

    do {
        // For each cluster of the file set it to FREE
        current_cluster = next_cluster;

        break_loop =
            bool_t_cast(!get_next_cluster(fs_ptr,
                current_cluster,
                &next_cluster));

        if (current_cluster < mipos_fs_CLUSTERS_COUNT) {
            mark_cluster_free(fs_ptr, current_cluster);
        }

        mipos_fs_call_os_scheduler();

    } 
    while (!break_loop);

    return mipos_update_fs_on_disk(fs_ptr) == 0 ? TRUE : FALSE;
}


/* -------------------------------------------------------------------------- */

#endif //!ENABLE_MIPOS_FS


/* -------------------------------------------------------------------------- */
