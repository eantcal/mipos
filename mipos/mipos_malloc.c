/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */

/* ------------------------------------------------------------------------- */

#ifdef ENABLE_MIPOS_MALLOC
#define ENABLE_MIPOS_MM
#endif

#ifdef ENABLE_MIPOS_MM

#include <memory.h>
#include <stdint.h>
#include <stdio.h>

#include "mipos_bsp.h"
#include "mipos_malloc.h"

/* ------------------------------------------------------------------------- */

#ifndef min
#define min(__x, __y) ((__x) < (__y) ? __x : __y)
#endif

#ifndef max
#define max(__x, __y) ((__x) > (__y) ? __x : __y)
#endif

/* ------------------------------------------------------------------------- */

static void mdesc_link_after(mipos_mdesc_t* _this, mipos_mdesc_t* block)
{
    _this->_next = block->_next;
    _this->_prev = block;
    if (_this->_next) {
        _this->_next->_prev = _this;
    }
    block->_next = _this;
}

/* ------------------------------------------------------------------------- */

static void mdesc_link_before(mipos_mdesc_t* _this, mipos_mdesc_t* block)
{
    _this->_prev = block->_prev;
    _this->_next = block;

    if (_this->_prev) {
        _this->_prev->_next = _this;
    }

    block->_prev = _this;
}

/* ------------------------------------------------------------------------- */

static void mdesc_link_detach(mipos_mdesc_t* _this)
{
    if (_this->_next) {
        _this->_next->_prev = _this->_prev;
    }

    if (_this->_prev) {
        _this->_prev->_next = _this->_next;
    }
}

/* ------------------------------------------------------------------------- */

inline void mdesc_setnext(mipos_mdesc_t* _this, mipos_mdesc_t* block)
{
    _this->_next = block;
}

/* ------------------------------------------------------------------------- */

inline void mdesc_setprev(mipos_mdesc_t* _this, mipos_mdesc_t* block)
{
    _this->_prev = block;
}

/* ------------------------------------------------------------------------- */

inline void mdesc_setsize(mipos_mdesc_t* _this, size_t blockSize)
{
    _this->_block_size = blockSize;
}

/* ------------------------------------------------------------------------- */

inline size_t mdesc_getsize(mipos_mdesc_t* _this)
{
    return _this->_block_size;
}

/* ------------------------------------------------------------------------- */

inline mipos_mdesc_t* mdesc_next(mipos_mdesc_t* _this)
{
    return _this->_next;
}

/* ------------------------------------------------------------------------- */

inline mipos_mdesc_t* mdesc_prev(mipos_mdesc_t* _this)
{
    return _this->_prev;
}

/* ------------------------------------------------------------------------- */

inline void mdesc_isolate(mipos_mdesc_t* _this)
{
    _this->_next = 0;
    _this->_prev = 0;
}

/* ------------------------------------------------------------------------- */

inline void mdesc_init(mipos_mdesc_t* _this, size_t size)
{
    mdesc_setsize(_this, size);
    mdesc_isolate(_this);
}

/* ------------------------------------------------------------------------- */

inline void* mdesc_get_data_addr(mipos_mdesc_t* _this)
{
    return (void*)(_this + 1);
}

/* ------------------------------------------------------------------------- */

static mipos_mdesc_t* mdesc_merge(mipos_mdesc_t* mbd1, mipos_mdesc_t* mbd2)
{
    mipos_mdesc_t* leftNode = min(mbd1, mbd2);
    mipos_mdesc_t* rightNode = max(mbd1, mbd2);

    size_t newSize =
      mdesc_getsize(mbd1) + mdesc_getsize(mbd2) + sizeof(mipos_mdesc_t);

    mdesc_setsize(leftNode, newSize);

    mdesc_link_detach(rightNode);
    mdesc_isolate(rightNode);

    return leftNode;
}

/* ------------------------------------------------------------------------- */

inline mipos_mdesc_t* mdesc_get_bd_addr(void* ptr)
{
    return (mipos_mdesc_t*)ptr - 1;
}

/* ------------------------------------------------------------------------- */

static void mm_add2used(mipos_mm_t* _this, mipos_mdesc_t* mbd)
{
    if (!_this->_used_memory) {
        mdesc_isolate(mbd);
        _this->_used_memory = mbd;
    } else {
        mdesc_link_after(mbd, _this->_used_memory);
    }
}

/* ------------------------------------------------------------------------- */

static mipos_mdesc_t* mm_find_free(mipos_mm_t* _this, size_t size)
{
    if (!_this->_free_memory) {
        return 0;
    }

    mipos_mdesc_t* mbd = _this->_free_memory;
    mipos_mdesc_t* g_mbd = 0;

    while (mbd && mdesc_getsize(mbd) != size) {
        if (mdesc_getsize(mbd) > size) {
            if (g_mbd) {
                if (mdesc_getsize(g_mbd) < mdesc_getsize(mbd)) {
                    g_mbd = mbd;
                }
            } else {
                g_mbd = mbd;
            }
        }
        mbd = mdesc_next(mbd);
    }

    return mbd ? mbd : g_mbd;
}

/* ------------------------------------------------------------------------- */

mipos_mdesc_t* mm_find_used(mipos_mm_t* _this, void* ptr)
{
    mipos_mdesc_t* mbd = _this->_used_memory;

    if (!mbd) {
        return 0;
    }

    while (mbd && ptr != mdesc_get_data_addr(mbd)) {
        mbd = mdesc_next(mbd);
    }

    return mbd;
}

/* ------------------------------------------------------------------------- */

inline int mipos_mm_is_in(mipos_mm_t* _this, void* ptr)
{
    return mm_find_used(_this, ptr) != 0;
}

/* ------------------------------------------------------------------------- */

mipos_mdesc_t* mm_find_cont_free_pred_mb(mipos_mm_t* _this, mipos_mdesc_t* mbd)
{
    mipos_mdesc_t* mbd_i = _this->_free_memory;

    while (mbd_i != 0) {
        if (mbd != mbd_i) {
            if (((mipos_mdesc_t*)((char*)mdesc_get_data_addr(mbd_i) +
                                  mdesc_getsize(mbd_i)) == mbd) ||
                ((mipos_mdesc_t*)((char*)mdesc_get_data_addr(mbd) +
                                  mdesc_getsize(mbd)) == mbd_i)) {
                break;
            }
        }
        mbd_i = mdesc_next(mbd_i);
    }

    return mbd_i;
}

/* ------------------------------------------------------------------------- */

mipos_mdesc_t* mm_find_cont_free_mb(mipos_mm_t* _this,
                                    mipos_mdesc_t* first_mbd,
                                    mipos_mdesc_t* second_mbd)
{
    mipos_mdesc_t* max_mbd = max(first_mbd, second_mbd);

    if (_this->_free_memory == max_mbd) {
        if (mdesc_next(max_mbd)) {
            _this->_free_memory = mdesc_next(max_mbd);
        } else {
            _this->_free_memory = min(first_mbd, second_mbd);
        }
    }

    return mdesc_merge(first_mbd, second_mbd);
}

/* ------------------------------------------------------------------------- */

static mipos_mdesc_t* mm_alloc_aux(mipos_mm_t* _this, size_t size)
{
#if defined(MIPOS32) || defined(MIPOS64)
    typedef long long signed_size_t;
#else
    typedef int signed_size_t;
#endif
    mipos_mdesc_t* allocated_mbd = 0;

    mipos_mdesc_t* mbd = mm_find_free(_this, size);

    if (!mbd) {
        // no enough memory
        return 0;
    }

    size_t needed_mbd_size = size + sizeof(mipos_mdesc_t);

    if ((signed_size_t)(mdesc_getsize(mbd) - needed_mbd_size) >
        (signed_size_t)(sizeof(mipos_mdesc_t))) {
        mdesc_setsize(mbd, mdesc_getsize(mbd) - needed_mbd_size);

        allocated_mbd = (mipos_mdesc_t*)(((char*)mdesc_get_data_addr(mbd)) +
                                         mdesc_getsize(mbd));
        mdesc_init(allocated_mbd, size);
    } else {
        if (_this->_free_memory == mbd) {
            _this->_free_memory = mdesc_next(mbd);

            if (_this->_free_memory) {
                mdesc_setprev(_this->_free_memory, 0);
            }
        }
        mdesc_link_detach(mbd);
        allocated_mbd = mbd;
    }

    mm_add2used(_this, allocated_mbd);

    return allocated_mbd;
}

/* ------------------------------------------------------------------------- */

int mipos_mm_free(mipos_mm_t* _this, void* ptr)
{
    mipos_mdesc_t* allocated_mbd = mm_find_used(_this, ptr);

    if (allocated_mbd) {
        if (allocated_mbd == _this->_used_memory) {
            _this->_used_memory = mdesc_next(allocated_mbd);

            if (_this->_used_memory) {
                mdesc_setprev(_this->_used_memory, 0);
            }
        }

        mdesc_link_detach(allocated_mbd);
        mdesc_isolate(allocated_mbd);

        if (!_this->_free_memory) {
            _this->_free_memory = allocated_mbd;
        } else {
            mdesc_link_after(allocated_mbd, _this->_free_memory);
        }

        return 1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

void mipos_mm_init(mipos_mm_t* _this, void* arena, size_t size)
{
    _this->_arena_size = size;
    _this->_free_memory = (mipos_mdesc_t*)arena;
    _this->_used_memory = 0;

    memset(arena, 0, size);
    mdesc_init(_this->_free_memory, size - sizeof(mipos_mdesc_t));
}

/* ------------------------------------------------------------------------- */

size_t mipos_mm_compact_freemem(mipos_mm_t* _this, size_t bytes_needed)
{
    size_t bytes_compacted = 0;

    if (_this->_free_memory) {
        mipos_mdesc_t* mbd = _this->_free_memory;

        while (mbd) {
            mipos_mdesc_t* mbd_i = mm_find_cont_free_pred_mb(_this, mbd);

            if (mbd_i) {
                bytes_compacted =
                  mdesc_getsize(mm_find_cont_free_mb(_this, mbd_i, mbd));

                mbd = _this->_free_memory;

                if (bytes_needed && bytes_compacted >= bytes_needed) {
                    break;
                }
                continue;
            }
            mbd = mdesc_next(mbd);
        }
    }

    return bytes_compacted;
}

/* ------------------------------------------------------------------------- */

void* mipos_mm_alloc(mipos_mm_t* _this, size_t size)
{
    // size must be positive value

    if (!size) {
        return 0;
    }
#ifdef MIPOS64
    size_t aligned_size = (size + 3) & (~3ULL);
#else
    size_t aligned_size = (size + 3) & (~3UL);
#endif
    mipos_mdesc_t* blk = mm_alloc_aux(_this, aligned_size);

    if (!blk) {
        if (mipos_mm_compact_freemem(_this, aligned_size) >= aligned_size) {
            // second chance
            blk = mm_alloc_aux(_this, aligned_size);
        }
    }

    return blk ? mdesc_get_data_addr(blk) : 0;
}

/* ------------------------------------------------------------------------- */

static size_t get_part_ptr_allocated_mem_size(mipos_mm_t* _this, void* ptr)
{
    mipos_mdesc_t* allocated_mbd = mm_find_used(_this, ptr);

    if (allocated_mbd) {
        return mdesc_getsize(allocated_mbd);
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

void* mipos_mm_realloc(mipos_mm_t* _this, void* ptr, size_t size)
{
    size_t alloc_size = 0;
    void* new_ptr = 0;

    if (ptr == 0) {
        return mipos_mm_alloc(_this, size);
    }

    alloc_size = get_part_ptr_allocated_mem_size(_this, ptr);

    if (0 < size && size < alloc_size) {
        return ptr;
    }

    size_t min_size = min(alloc_size, size);

    if (min_size > 0) {
        new_ptr = mipos_mm_alloc(_this, size);
        memcpy(new_ptr, ptr, min_size);
    }

    mipos_mm_free(_this, ptr);

    return new_ptr;
}

/* ------------------------------------------------------------------------- */

void mipos_mm_dbg_print(mipos_mm_t* _this)
{
    mipos_mdesc_t* mbd = _this->_free_memory;

    int i = 0;
    size_t tot_mem = 0;
    size_t tot_part_mem = 0;
    size_t tot_res_mem = 0;

#ifdef MIPOS64
    mipos_printf("Memory Arena %zu bytes - Free Memory Pointer %p\n\n",
                 _this->_arena_size,
                 _this->_free_memory);
#else
    mipos_printf("Memory Arena %u bytes - Free Memory Pointer 0x%08x\n\n",
                 (int)_this->_arena_size,
                 (int)_this->_free_memory);
#endif

#ifdef MIPOS64
    mipos_printf("/- FREE BLOCKS "
                 "-------------------------------------------------------------"
                 "------------------\\\n");
    mipos_printf("| Block  |      address     |  size  |    adj address   |    "
                 "   next       |       prev       |\n");
    mipos_printf("|------------------------------------------------------------"
                 "---------------------------------|\n");
#else
    mipos_printf(
      "/- FREE BLOCKS -----------------------------------------------\\\n");
    mipos_printf(
      "| Block  |  address |  size  | adj addr |   next   |   prev   |\n");
    mipos_printf(
      "|-------------------------------------------------------------|\n");
#endif
    while (mbd) {


        mipos_printf("|%8i| %p |%8zu| %p | %p | %p |\n",
                     i++,
                     (char*)mdesc_get_data_addr(mbd) - sizeof(mipos_mdesc_t),
                     mdesc_getsize(mbd),
                     (char*)mdesc_get_data_addr(mbd) + mdesc_getsize(mbd),
                     mdesc_next(mbd),
                     mdesc_prev(mbd));

        tot_part_mem += mdesc_getsize(mbd);
        mbd = mdesc_next(mbd);
    }
    tot_mem += tot_part_mem;
#ifdef MIPOS64
    mipos_printf("|------------------------------------------------------------"
                 "---------------------------------|\n");
    mipos_printf("| Total %8zu (bytes)                                         "
                 "                             |\n",
                 tot_part_mem);
    mipos_printf("\\-----------------------------------------------------------"
                 "----------------------------------/\n");
#else
    mipos_printf(
      "|-------------------------------------------------------------|\n");
    mipos_printf("| Total %8i (bytes)                                      |\n",
                 tot_part_mem);
    mipos_printf(
      "\\-------------------------------------------------------------/\n");
#endif
    tot_res_mem = i * sizeof(mipos_mdesc_t);

    mipos_printf("\nAllocated Memory Address: %p\n\n", _this->_used_memory);

    mbd = _this->_used_memory;
    i = 0;
    tot_part_mem = 0;

#ifdef MIPOS64
    mipos_printf("/- USED BLOCKS "
                 "-------------------------------------------------------------"
                 "------------------\\\n");
    mipos_printf("| Block  |      address     |  size  |    adj address   |    "
                 "   next       |       prev       |\n");
    mipos_printf("|------------------------------------------------------------"
                 "---------------------------------|\n");
#else

    mipos_printf(
      "/- USED BLOCKS -----------------------------------------------\\\n");
    mipos_printf(
      "| Block  |  address |  size  | adj addr |   next   |   prev   |\n");
    mipos_printf(
      "|-------------------------------------------------------------|\n");
#endif
    while (mbd) {
#ifdef MIPOS64
        mipos_printf("|%8i| %p |%8zu| %p | %p | %p |\n",
                     i++,
                     (char*)mdesc_get_data_addr(mbd) - sizeof(mipos_mdesc_t),
                     mdesc_getsize(mbd),
                     (char*)mdesc_get_data_addr(mbd) + mdesc_getsize(mbd),
                     mdesc_next(mbd),
                     mdesc_prev(mbd));
#else
        mipos_printf(
          "|%8i| %08x |%8i| %08x | %08x | %08x |\n",
          i++,
          (int)((char*)mdesc_get_data_addr(mbd) - sizeof(mipos_mdesc_t)),
          (int)mdesc_getsize(mbd),
          (int)((char*)mdesc_get_data_addr(mbd) + mdesc_getsize(mbd)),
          (int)mdesc_next(mbd),
          (int)mdesc_prev(mbd));
#endif
        tot_part_mem += mdesc_getsize(mbd);
        mbd = mdesc_next(mbd);
    }
    tot_mem += tot_part_mem;

#ifdef MIPOS64
    mipos_printf("|------------------------------------------------------------"
                 "---------------------------------|\n");
    mipos_printf("| Total %8zu (bytes)                                         "
                 "                             |\n",
                 tot_part_mem);
    mipos_printf("\\-----------------------------------------------------------"
                 "----------------------------------/\n");
#else
    mipos_printf(
      "|-------------------------------------------------------------|\n");
    mipos_printf("| Total %8i (bytes)                                      |\n",
                 tot_part_mem);
    mipos_printf(
      "\\-------------------------------------------------------------/\n");
#endif

    tot_res_mem += i * sizeof(mipos_mdesc_t);
    tot_mem += tot_res_mem;

    mipos_printf("\nReserved Memory %8zu (bytes)\n", tot_res_mem);

    mipos_printf("Total Memory    %8zu (bytes)\n", tot_mem);

    if (tot_mem != _this->_arena_size) {
        mipos_printf("\nERROR: totalPool != _poolMemorySize !!!!!\n");
    }
}

/* ------------------------------------------------------------------------- */

size_t mipos_mm_get_used_memory_size(mipos_mm_t* _this)
{
    mipos_mdesc_t* mbd = _this->_used_memory;

    int i = 0;
    size_t used_mem = 0;

    while (mbd) {
        i++;
        used_mem += mdesc_getsize(mbd);
        mbd = mdesc_next(mbd);
    }

    used_mem += i * sizeof(mipos_mdesc_t);

    return used_mem;
}

/* ------------------------------------------------------------------------- */

#ifdef ENABLE_MIPOS_MALLOC

/* ------------------------------------------------------------------------- */

// Pointer to the malloc memory manager instance
static mipos_mm_t* _mm_ptr = 0;

/* ------------------------------------------------------------------------- */

void mipos_set_malloc_mm(mipos_mm_t* mm_ptr)
{
    _mm_ptr = mm_ptr;
}

/* ------------------------------------------------------------------------- */
// standard MALLOC functions

void* mipos_malloc(size_t size)
{
    return mipos_mm_alloc(_mm_ptr, size);
}

/* ------------------------------------------------------------------------- */

void* mipos_calloc(size_t nmemb, size_t size)
{
    char* ptr;
    size *= nmemb;

    ptr = (char*)mipos_mm_alloc(_mm_ptr, size);

    if (ptr == 0) {
        return 0;
    }

    memset(ptr, 0, size);

    return ptr;
}

/* ------------------------------------------------------------------------- */

void mipos_free(void* ptr)
{
    mipos_mm_free(_mm_ptr, ptr);
}

/* ------------------------------------------------------------------------- */

void* mipos_realloc(void* ptr, size_t size)
{
    return mipos_mm_realloc(_mm_ptr, ptr, size);
}

/* ------------------------------------------------------------------------- */

#endif // ENABLE_MIPOS_MALLOC

#endif // ENABLE_MIPOS_MM
