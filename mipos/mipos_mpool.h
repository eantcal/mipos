/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* ------------------------------------------------------------------------- */

#ifndef mipos_MPOOL_H__
#define mipos_MPOOL_H__

#include <stddef.h>
#include <stdint.h>


/* ------------------------------------------------------------------------- */

typedef char** mipos_mpool_block_ptr_t;


/* ------------------------------------------------------------------------- */

typedef struct
{
    size_t _block_size;
    size_t _n_of_blocks;
    size_t _free_blocks;
    mipos_mpool_block_ptr_t _free_head_ptr;

    char* _arena_ptr;
} mipos_mpool_t;


/* ------------------------------------------------------------------------- */

/** Initialize a memory pool.
 *  @param _this pointer to memory pool descriptor
 *  @block_size size of each block release by block allocator
 *  @pool_length number of preallocated blocks
 *  @param arena must point a valid memory area with
 *         size >= (max(block_size, sizeof(void*))*pool_length
 */
void mipos_mpool_init(mipos_mpool_t* _this,
                      size_t block_size,
                      size_t pool_length,
                      void* arena);


/* ------------------------------------------------------------------------- */

/** Block allocator. Return a pointer to a memory block belonging to the pool
 *  @param _this pointer to memory pool descriptor
 *  @return the pointer to the block or 0 in case of error (e.g. no more blocks)
 */
void* mipos_mpool_alloc(mipos_mpool_t* _this);


/* ------------------------------------------------------------------------- */

/** Release a pointer to a memory block belonging to a pool.
 *  @param _this pointer to memory pool descriptor
 *  @param ptr pointer to the block
 */
void mipos_mpool_free(mipos_mpool_t* _this, void* ptr);


/* ------------------------------------------------------------------------- */

/** Return the number of allocated blocks
 *  @param number of blocks
 */
size_t mpool_get_used_blocks(const mipos_mpool_t* _this);


/* ------------------------------------------------------------------------- */

/** Return the number of available free blocks
 *  @param number of blocks
 */
size_t mpool_get_free_blocks(const mipos_mpool_t* _this);


/* ------------------------------------------------------------------------- */

#endif // ! mipos_MPOOL_H__


/* ------------------------------------------------------------------------- */
