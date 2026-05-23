/*
 * task_scheduler.h
 *
 *  Created on: 20 may. 2026
 *      Author: anton
 */

#ifndef KERNEL_TASK_SCHEDULER_H_
#define KERNEL_TASK_SCHEDULER_H_

#include <stdint.h>
#include "task.h"

#define SCHEDULER_OK                 0
#define SCHEDULER_ERROR_NO_TASK     -1

#define SCHEDULER_INVALID_TASK_ID    0xFFFFFFFFU

#define SCHEDULER_TIME_SLICE_TICKS   10U

typedef enum
{
    SCHEDULER_STATE_STOPPED = 0,
    SCHEDULER_STATE_INITIALIZED,
    SCHEDULER_STATE_RUNNING
} scheduler_state_t;

void os_scheduler_init(void);
int32_t os_scheduler_start(void);

const task_control_block_t *os_get_current_task(void);
uint32_t os_get_current_task_id(void);

const task_control_block_t *os_schedule_next(void);
void os_yield(void);
void os_sleep(uint32_t milliseconds);

scheduler_state_t os_scheduler_get_state(void);

uint32_t os_scheduler_is_preemption_enabled(void);
void os_scheduler_enable_preemption(void);
void os_scheduler_disable_preemption(void);

void os_scheduler_tick(void);

#endif /* KERNEL_TASK_SCHEDULER_H_ */
