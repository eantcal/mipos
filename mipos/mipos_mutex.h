/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#ifndef mipos_MUTEX_H__
#define mipos_MUTEX_H__

#include "mipos_types.h"


/* -------------------------------------------------------------------------- */

#define MUTEX_UNLOCKED 0
#define MUTEX_LOCKED   1
#define MU_INIT       {0}

typedef char mipos_mtx_flags_t;

typedef struct _mipos_mutex_t {
    mipos_mtx_flags_t state;
} 
mipos_mtx_t;


/* -------------------------------------------------------------------------- */

/** Init a mutex
 *  Callable From: NON-RT TASK, RT TASK
 *
 * @param _MTX: mipos_mtx_t instance
 */
#define mipos_mu_init(_MTX) do {\
    mipos_init_cs();\
    mipos_enter_cs();\
    (_MTX)->state = MUTEX_UNLOCKED;\
    mipos_leave_cs();\
} while(0)


/* -------------------------------------------------------------------------- */

/** This function acquires ownership of the mutex specified.
 *  If the mutex currently is locked by another task, 
 *  the call blocks until that task relinquishes ownership 
 *  by a call to mipos_mu_unlock()
 *
 *  Callable From: NON-RT TASK
 *  @param _MTX: mipos_mtx_t instance 
 */
#define mipos_mu_lock(_MTX) do {\
    while (1) {\
        if ((_MTX)->state == MUTEX_UNLOCKED) {\
           break;\
        }\
        mipos_tm_wkafter(0); \
    }\
    (_MTX)->state |= MUTEX_LOCKED;\
} while(0)


/* -------------------------------------------------------------------------- */

/** This function unlocks the mutex specified. 
 *  If the calling thread does not currently hold the mutex 
 *  (via a previous call to mipos_mu_lock()) the unlock 
 *  request has no effect.
 *
 *  Callable From: NON-RT TASK, RT TASK
 *
 *  @param _MTX: mipos_mtx_t instance  
 */
#define mipos_mu_unlock(_MTX) do {\
    mipos_init_cs();\
    mipos_enter_cs();\
       (_MTX)->state = MUTEX_UNLOCKED;\
    mipos_leave_cs();\
} while(0)


/* -------------------------------------------------------------------------- */

#endif //mipos_MUTEX_H_


/* -------------------------------------------------------------------------- */

