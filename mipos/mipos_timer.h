/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#ifndef mipos_TIMER_H__
#define mipos_TIMER_H__

#include "mipos_types.h"


/* -------------------------------------------------------------------------- */

/** Construct and start a new timer 
 * @param on_expire_handler callback-function called upon timer expiration, 
          cannot be null
 * @param on_cancel_handler callback-function called upon timer cancellation, 
          can be null
 * @param interval timer intervall in mS
 * @param context parameter passed to callback-functions
 * @param periodic flag, non-zero-> periodic timer
 *
 * @return timer id, or a negative value in case of error
 */
timer_id_t
mipos_timer_start(
    mipos_timer_handler_t on_expire_handler, 
    mipos_timer_handler_t on_cancel_handler, 
    mipos_timer_intvl_t interval,
    mipos_timer_ctx_t context,
    int periodic
);


/* -------------------------------------------------------------------------- */

/** Stop and remove the timer immediately
 *  @param id must be a valid timer identifier
 *
 *  @return 0 on success, or a negative value in case of error
 */
int mipos_timer_delete(timer_id_t id);


/* -------------------------------------------------------------------------- */

/** Schedule timer cancellation, timer will be no more  active, but it can be 
 *  restarted or removed. On-cancel hadler is invoked by the kernel if
 *  the on_cancel_handler had been registered at timer construction time.
 *
 *  @param id must be a valid timer identifier
 *  @return 0 on success, or a negative value in case of error
 */
int mipos_timer_cancel(timer_id_t id);


/* -------------------------------------------------------------------------- */

/** Restart a stopped (cancelled) timer.
 *
 *  @param id must be a valid timer identifier
 *  @param new_interval set to 0 for using construction interval 
 *         set non-zero value to specify a new interval
 *  @return 0 on success, or a negative value in case of error
 */
int mipos_timer_restart(timer_id_t id, mipos_timer_intvl_t new_interval);


/* -------------------------------------------------------------------------- */

#endif //mipos_TIMER_H_
