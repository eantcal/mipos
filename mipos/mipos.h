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
#  include "mipos_console.h"
#endif

#ifdef ENABLE_MIPOS_FS
#  include "mipos_fs.h"
#endif

#ifdef ENABLE_MIPOS_STDIO
#  include "mipos_fs.h"
#  include "mipos_stdio.h"
#endif


/* -------------------------------------------------------------------------- */

#endif //!__KERNEL_H__

