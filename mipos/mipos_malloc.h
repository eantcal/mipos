/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* ------------------------------------------------------------------------- */

#ifndef mipos_MALLOC_H__
#define mipos_MALLOC_H__


/* ------------------------------------------------------------------------- */

typedef struct _mdesc
{
    struct _mdesc* _next;
    struct _mdesc* _prev;
    size_t _block_size;
} mipos_mdesc_t;


/* ------------------------------------------------------------------------- */

typedef struct
{
    size_t _arena_size;
    mipos_mdesc_t* _free_memory;
    mipos_mdesc_t* _used_memory;
} mipos_mm_t;


/* ------------------------------------------------------------------------- */

void mipos_mm_init(mipos_mm_t* _this, void* arena, size_t size);
int mipos_mm_is_in(mipos_mm_t* _this, void* ptr);
int mipos_mm_free(mipos_mm_t* _this, void* ptr);
void* mipos_mm_alloc(mipos_mm_t* _this, size_t size);
void* mipos_mm_realloc(mipos_mm_t* _this, void* ptr, size_t size);
void mipos_mm_dbg_print(mipos_mm_t* _this);
size_t mipos_mm_get_used_memory_size(mipos_mm_t* _this);
size_t mipos_mm_compact_freemem(mipos_mm_t* _this, size_t bytes_needed);


/* ------------------------------------------------------------------------- */

void mipos_set_malloc_mm(mipos_mm_t* mm_ptr);
void* mipos_malloc(size_t size);
void* mipos_calloc(size_t nmemb, size_t size);
void mipos_free(void* ptr);
void* mipos_realloc(void* ptr, size_t size);

/* ------------------------------------------------------------------------- */

#endif