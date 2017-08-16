/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* -------------------------------------------------------------------------- */

#ifndef __mipos_LIMITS_H__
#define __mipos_LIMITS_H__


/* -------------------------------------------------------------------------- */

/** Size of task table (including the IDLE task) */
#define MIPOS_MAX_TASKS_COUNT 7
#define MIPOS_TASKS_TBL_ENTRIES_CNT (MIPOS_MAX_TASKS_COUNT+1)

/** Size of timer table */
#define MIPOS_MAX_TIMERS_NO 4

/** Root task id is 1 */
#define MIPOS_ROOT_TASK_ID 1

/** Max length of a task name */
#define TASKNAMELEN 8


/* -------------------------------------------------------------------------- */

#endif //__MIPOS_LIMITS_H__

