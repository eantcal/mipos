/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#include "mipos_kernel.h"


/* -------------------------------------------------------------------------- */

timer_id_t  mipos_timer_start(
    mipos_timer_handler_t on_expire_handler,
    mipos_timer_handler_t on_cancel_handler, /* can be null */
    mipos_timer_intvl_t interval,
    mipos_timer_ctx_t context,
    int periodic
)
{
    int id;
    mipos_timer_bmp_t tmask;

    mipos_timer_t * p_timer = 0;
    mipos_init_cs();

    if (!on_expire_handler) {
        return -1;
    }

    mipos_enter_cs();

    for (id = 0; id < MIPOS_MAX_TIMERS_NO; ++id)
    {
        tmask = 1 << id;

        p_timer = &(KERNEL_ENV.timer_table[id]);

        if (!(p_timer->handler_on_expire_cbk)) {
            //Activate the timer

            KERNEL_ENV.timer_bitmap |= tmask;
            mipos_get_rtc_counter(&p_timer->rtc_timeout);
            p_timer->rtc_timeout += interval;

            if (KERNEL_ENV.next_rtc_timeout == 0 ||
                KERNEL_ENV.next_rtc_timeout > p_timer->rtc_timeout)
            {
                KERNEL_ENV.next_rtc_timeout = p_timer->rtc_timeout;
            }

            break;
        }
    }

    mipos_leave_cs();

    //if id is ok ...
    if (id < MIPOS_MAX_TIMERS_NO) {
        //Set up timer parameters
        p_timer->flgs =
            periodic ? TIMER_FLG_PERIODIC : 0;

        p_timer->handler_on_expire_cbk = on_expire_handler;
        p_timer->handler_on_cancel_cbk = on_cancel_handler;
        p_timer->context = context;
        p_timer->interval = interval;

        return id;
    }

    return -1;
}


/* -------------------------------------------------------------------------- */

int mipos_timer_cancel(timer_id_t id)
{
    mipos_timer_t * p_timer;
    int ret_val = -1;

    mipos_init_cs();

    if (id < 0 || id >= MIPOS_MAX_TIMERS_NO) {
        return -1; // -> failure
    }

    p_timer = &(KERNEL_ENV.timer_table[id]);

    mipos_enter_cs();

    // if the timer is active
    if ((KERNEL_ENV.timer_bitmap & (1 << id)))
    {
        // set cancelled flag notifing
        // the kernel that the timer can be
        // cancelled
        p_timer->flgs |= TIMER_FLG_CANCELLED;

        // force the kernel to
        // visit the timer collection in order 
        // to handle the cencelling-timer
        KERNEL_ENV.next_rtc_timeout = 0;

        //return 0 -> success
        ret_val = 0;
    }

    mipos_leave_cs();

    return ret_val;
}


/* -------------------------------------------------------------------------- */

int mipos_timer_delete(timer_id_t id)
{
    mipos_timer_t * p_timer;
    mipos_init_cs();

    if (id < 0 || id >= MIPOS_MAX_TIMERS_NO) {
        return -1; //failure
    }

    p_timer = &(KERNEL_ENV.timer_table[id]);

    //Remove the timer immediately
    mipos_enter_cs();

    p_timer->handler_on_expire_cbk = 0;
    p_timer->handler_on_cancel_cbk = 0;

    p_timer->context = 0;
    p_timer->rtc_timeout = 0;
    p_timer->flgs = 0;

    KERNEL_ENV.timer_bitmap &= ~(1 << id);

    mipos_leave_cs();

    return 0; //success
}


/* -------------------------------------------------------------------------- */


int mipos_timer_restart(
    timer_id_t id,
    mipos_timer_intvl_t new_interval /* 0 - use old interval */
)
{
    mipos_timer_t * p_timer;
    int ret_val = 0;
    mipos_init_cs();

    if (id < 0) {
        return -1;
    }

    p_timer = &(KERNEL_ENV.timer_table[id]);

    mipos_enter_cs();
    if (!(p_timer->flgs & TIMER_FLG_PERIODIC)
        &&
        (p_timer->flgs & TIMER_FLG_EXPIRED))
    {
        p_timer->flgs &= (mipos_reg_t)~TIMER_FLG_EXPIRED;

        mipos_get_rtc_counter(&p_timer->rtc_timeout);

        if (new_interval > 0) {
            p_timer->interval = new_interval;
        }

        p_timer->rtc_timeout += p_timer->interval;
    }
    else {
        ret_val = -1;
    }
    mipos_leave_cs();

    return ret_val;
}


/* -------------------------------------------------------------------------- */

