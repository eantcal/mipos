/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#ifndef mipos_MEMORY_ALLOCATOR_H__
#define mipos_MEMROY_ALLOCATOR_H__


/* -------------------------------------------------------------------------- */

typedef struct _mdesc {
    struct _mdesc * _next, *_prev;
    size_t _block_size;
}
mdesc_t;


/* -------------------------------------------------------------------------- */

typedef struct {
    size_t   _arena_size;
    mdesc_t* _free_memory;
    mdesc_t* _used_memory;
}
mm_t;


/* -------------------------------------------------------------------------- */

void mipos_mm_init(mm_t * _this, void * arena, size_t size);
int mipos_mm_is_in(mm_t * _this, void * ptr);
int mipos_mm_free(mm_t * _this, void * ptr);
void* mipos_mm_alloc(mm_t * _this, size_t size);
void* mipos_mm_realloc(mm_t * _this, void * ptr, size_t size);
void mipos_mm_dbg_print(mm_t * _this);
size_t mipos_mm_get_used_memory_size(mm_t * _this);
size_t mipos_mm_compact_freemem(mm_t * _this, size_t bytes_needed);


/* -------------------------------------------------------------------------- */

#ifdef ENABLE_MIPOS_MALLOC
void   mipos_set_malloc_mm(mm_t * mm_ptr);
void * mipos_malloc(size_t size);
void * mipos_calloc(size_t nmemb, size_t size);
void   mipos_free(void *ptr);
void * mipos_realloc(void *ptr, size_t size);
#endif

/* -------------------------------------------------------------------------- */

#endif // ! mipos_MEMROY_ALLOCATOR_H__