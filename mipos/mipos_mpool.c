/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* ------------------------------------------------------------------------- */

#include "mipos_mpool.h"

#include <stdint.h>


/* ------------------------------------------------------------------------- */

#ifdef ENABLE_MIPOS_MPOOL

/* ------------------------------------------------------------------------- */

void mipos_mpool_init(mipos_mpool_t* _this,
                      size_t block_size,
                      size_t pool_length,
                      void* arena)
{
    size_t i = 0;
    mipos_mpool_block_ptr_t block_ptr;
    mipos_mpool_block_ptr_t pnext;

    _this->_n_of_blocks = pool_length;

    if (block_size < sizeof(mipos_mpool_block_ptr_t))
        block_size = sizeof(mipos_mpool_block_ptr_t);

    _this->_block_size = block_size;
    _this->_free_blocks = _this->_n_of_blocks;
    _this->_free_head_ptr = 0;

    _this->_arena_ptr = arena;

    block_ptr = (mipos_mpool_block_ptr_t)arena;

    for (i = 0; i < (pool_length - 1); ++i) {
        pnext =
          (mipos_mpool_block_ptr_t)((char*)block_ptr + _this->_block_size);

        //      (mipos_mpool_block_ptr_t)*block_ptr = pnext; // VC++ alternative
        *((uintptr_t*)block_ptr) = (unsigned int)pnext;

        block_ptr = pnext;
    }

    //   (mipos_mpool_block_ptr_t)(*block_ptr) = 0;// VC++ alternative
    *((uintptr_t*)block_ptr) = 0;

    _this->_free_head_ptr = (mipos_mpool_block_ptr_t)_this->_arena_ptr;
}


/* ------------------------------------------------------------------------- */

void* mipos_mpool_alloc(mipos_mpool_t* _this)
{
    if (_this->_free_blocks == 0) {
        return 0;
    }

    mipos_mpool_block_ptr_t alloc_ptr = _this->_free_head_ptr;
    _this->_free_head_ptr = (mipos_mpool_block_ptr_t)*alloc_ptr;

    --_this->_free_blocks;

    return alloc_ptr;
}


/* ------------------------------------------------------------------------- */

void mipos_mpool_free(mipos_mpool_t* _this, void* ptr)
{
    mipos_mpool_block_ptr_t head = _this->_free_head_ptr;
    _this->_free_head_ptr = (mipos_mpool_block_ptr_t)ptr;

    *((uintptr_t*)(ptr)) = (unsigned int)head;

    ++_this->_free_blocks;
}


/* ------------------------------------------------------------------------- */

size_t mpool_get_used_blocks(const mipos_mpool_t* _this)
{
    return _this->_n_of_blocks;
}


/* ------------------------------------------------------------------------- */

size_t mpool_get_free_blocks(const mipos_mpool_t* _this)
{
    return _this->_free_blocks;
}


/* ------------------------------------------------------------------------- */

#endif // ENABLE_MIPOS_MPOOL
