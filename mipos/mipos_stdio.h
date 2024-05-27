/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifndef __MIPOS_TFS_STDIO_H__
#define __MIPOS_TFS_STDIO_H__

/* ------------------------------------------------------------------------- */

#include "mipos_tfs.h"

/* ------------------------------------------------------------------------- */

typedef struct _mipos_FILE
{
    mipos_tfs_file_handle_t _handle;
    bool_t _read_only;
} mipos_tfs_FILE;

/* ------------------------------------------------------------------------- */

#define SEEK_SET 0 // Beginning of file
#define SEEK_CUR 1 // Current position of file pointer
#define SEEK_END 2 // End of file

/* ------------------------------------------------------------------------- */

void mipos_init_ramdisk_stdio(void* fs_ptr);

/* ------------------------------------------------------------------------- */

/** Opens the file whose name is specified in the parameter filename and
 *  associates it with a stream that can be identified in future operations
 *  by the mipos_tfs_FILE object whose pointer is returned.
 *  The operations that are allowed on the stream and how these are performed
 *  are defined by the mode parameter.
 *
 *  @param filename string containing the name of the file to be opened.
 *  @param mode parameter string containing a file access modes.
 *  It can be:<cr>
 *  - "r"  Open a file for reading. The file must exist.<cr>
 *  - "w"  Create an empty file for writing. If a file with
 *         the same name already exists its content is erased and the
 *         file is treated as a new empty file.<cr>
 *  - "r+" Open a file for update both reading and writing.
 *         The file must exist.<cr>
 *  - "w+" Create an empty file for both reading and writing.
 *         If a file with the same name already exists its
 *         content is erased and the file is treated as a
 *         new empty file.<cr>
 *
 * @return a pointer to a mipos_tfs_FILE object if the file has been succesfully
 *         opened. A mipos_tfs_FILE object is used to identify the stream on all
 * further operations involving it. Otherwise, a null pointer is returned.
 */
mipos_tfs_FILE* mipos_tfs_fopen(const char* filename, const char* mode);

/* ------------------------------------------------------------------------- */

/** Closes the file associated with the stream and disassociates it.
 *  All internal buffers associated with the stream are flushed: the content
 *  of any unwritten buffer is written and the content of any unread buffer
 *  is discarded. Even if the call fails, the stream passed as parameter
 *  will no longer be associated with the file.
 *
 * @param stream: pointer to a mipos_tfs_FILE object that
 *                specifies the stream to be closed.
 *
 * @return a zero value if the stream is successfully closed
 *         On failure, EOF is returned.
 */
int mipos_tfs_fclose(mipos_tfs_FILE* stream);

/* ------------------------------------------------------------------------- */

/** Checks if the End-of-File indicator associated with
 *  stream is set, returning a value different from zero if it is.
 *  This indicator is generally set by a previous operation on the stream
 *  that reached the End-of-File.
 *  Further operations on the stream once the End-of-File has been reached
 *  will fail until mipos_fseek is successfully called to set the position
 *  indicator to a new value.
 *
 * @param stream: pointer to a mipos_tfs_FILE object that identifies the stream.
 *
 * @return A non-zero value is returned in the case that the
 *         End-of-File indicator associated with the stream
 *         is set. Otherwise, a zero value is returned.
 */
int mipos_tfs_feof(mipos_tfs_FILE* stream);

/* ------------------------------------------------------------------------- */

/** Reads an array of count elements, each one with a size of size bytes,
 *  from the stream and stores them in the block of memory specified by ptr.
 *  The postion indicator of the stream is advanced by the total amount of bytes
 * read. The total amount of bytes read if successful is (size * count).
 *
 * @param ptr: Pointer to a block of memory with a minimum size of (size*count)
 * bytes.
 * @param size: Size in bytes of each element to be read.
 * @param count: Number of elements, each one with a size of size bytes.
 * @param stream: Pointer to a mipos_tfs_FILE object that specifies an input
 * stream.
 *
 * @return The total number of elements successfully read is returned as a
 *         size_t object, which is an integral data type. If this number differs
 *         from the count parameter, either an error occured or the End Of File
 *         was reached.
 */
size_t mipos_tfs_fread(void* ptr,
                       size_t size,
                       size_t count,
                       mipos_tfs_FILE* stream);

/* ------------------------------------------------------------------------- */

/** Writes an array of count elements, each one with a size of size bytes,
 *  from the block of memory pointed by ptr to the current position in the
 * stream. The postion indicator of the stream is advanced by the total amount
 * of bytes written. The total amount of bytes written is (size * count).
 *
 * @param ptr:Pointer to the array of elements to be written
 * @param size: Size in bytes of each element to be written.
 * @param count: Number of elements, each one with a size of size bytes.
 * @param stream: Pointer to a mipos_tfs_FILE object wich specifies an output
 * stream
 *
 * @return The total number of elements successfully written
 * is returned as a size_t object, which is an integral data
 * type.If this number differs from the count parameter, it
 * indicates an error.
 */
size_t mipos_tfs_fwrite(const void* ptr,
                        size_t size,
                        size_t count,
                        mipos_tfs_FILE* stream);

/* ------------------------------------------------------------------------- */

/** Sets the position indicator associated with the stream to a new position
 *  defined by adding offset to a reference position specified by origin.
 *  On streams open for update (read+write), a call to mipos_fseek
 *  allows to switch between reading and writing.
 *
 * @param stream: Pointer to a FILE object that identifies the stream.
 * @param offset: Number of bytes to offset from origin.
 * @param origin: Position from where offset is added. It is
 *  specified by one of the following constants:<cr>
 *   - SEEK_SET	Beginning of file <cr>
 *   - SEEK_CUR	Current position of the file pointer <cr>
 *   - SEEK_END	End of file <cr>
 *
 * @return If successful, the function returns a zero value.
 *         Otherwise, it returns nonzero value.
 */
int mipos_tiny_fseek(mipos_tfs_FILE* stream, long offset, int origin);

/* ------------------------------------------------------------------------- */

/**
 *  Returns the current value of the position indicator of the stream.
 *  For binary streams, the value returned corresponds to the number of bytes
 *  from the beginning of the file. For text streams, the value is not
 * guaranteed to be the exact number of bytes from the beginning of the file,
 *  but the value returned can still be used to restore the position indicator
 *  to this position using mipos_fseek.
 *
 * @param Pointer to a mipos_tfs_FILE object that identifies the stream.
 *
 * @return On success, the current value of the position
 *         indicator is returned.if an error occurs,-1L is returned.
 */
long mipos_tfs_ftell(mipos_tfs_FILE* stream);

/* ------------------------------------------------------------------------- */

/** Returns the character currently pointed by the internal file position
 *  indicator of the specified stream. The internal file position indicator is
 *  then advanced by one character to point to the next character.
 *
 * @param Pointer to a mipos_tfs_FILE object that identifies the stream.
 *
 * @return The character read is returned as an int value.
 *         If the End-of-File is reached or a reading error happens, the
 *         function returns EOF and the corresponding error or eof indicator is
 * set. You can use either ferror or mipos_feof to determine whether an error
 * happened or the End-Of-File was reached.
 */
int mipos_tfs_fgetc(mipos_tfs_FILE* stream);

/* ------------------------------------------------------------------------- */

/** Renames or moves a file from one location to another location.
 *
 * @param source_file:  Path of the file to rename/move.
 * @param dest_file:  New name/path of the file.
 *
 * @return On success, the zero value is returned.
 *         If an error occurs, -1L is returned.
 */
int mipos_tfs_rename(const char* source_file, const char* dest_file);

/* ------------------------------------------------------------------------- */

/** Deletes an existing file.
 *
 * @param filename: The name of the file to be deleted.
 *
 * @return On success, the zero value is returned.
 *         If an error occurs, -1L is returned.
 */
int mipos_tfs_remove(const char* filename);

/* ------------------------------------------------------------------------- */

/** Retrieves the size of the specified file, in bytes.
 *
 * @param filename: The name of the file.
 *
 * @return On success, the size value is returned.
 *         If an error occurs, -1L is returned.
 */
int mipos_tfs_filesize(const char* filename);

/* ------------------------------------------------------------------------- */

#endif //! __mipos_STDIO_H__

/* ------------------------------------------------------------------------- */
