/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/

/* -------------------------------------------------------------------------- */

#ifndef __MIPOS_H__
#define __MIPOS_H__

#include "mipos_kernel.h"

#ifdef ENABLE_MIPOS_CONSOLE
#include "mipos_console.h"
#endif

#if defined(ENABLE_MIPOS_FS) || defined(ENABLE_MIPOS_STDIO)
#include "mipos_fs.h"
#endif

#ifdef ENABLE_MIPOS_STDIO
#include "mipos_stdio.h"
#endif

#if defined(ENABLE_MIPOS_MM) || defined(ENABLE_MIPOS_MALLOC)
#include "mipos_malloc.h"
#endif

#if defined(ENABLE_MIPOS_MPOOL)
#include "mipos_mpool.h"
#endif


/* -------------------------------------------------------------------------- */

#endif //!__KERNEL_H__

