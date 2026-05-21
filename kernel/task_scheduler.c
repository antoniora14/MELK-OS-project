/*
 * task_scheduler.c
 *
 *  Created on: 20 may. 2026
 *      Author: anton
 */

#include "task_scheduler.h"
#include "task.h"

#define SCHEDULER_IDLE_TASK_ID    0U

static uint32_t g_current_task_id = SCHEDULER_INVALID_TASK_ID;
static scheduler_state_t g_scheduler_state = SCHEDULER_STATE_STOPPED;

static uint8_t scheduler_is_valid_task(const task_control_block_t *task)
{
    if (task == 0)
    {
        return 0U;
    }

    if (task->state == TASK_STATE_UNUSED)
    {
        return 0U;
    }

    return 1U;
}

static uint8_t scheduler_is_ready_task(const task_control_block_t *task)
{
    if (task == 0)
    {
        return 0U;
    }

    if (task->state == TASK_STATE_READY)
    {
        return 1U;
    }

    return 0U;
}

static int32_t scheduler_set_current_task(uint32_t task_id)
{
    int32_t status;

    status = task_set_state(task_id, TASK_STATE_RUNNING);

    if (status != TASK_OK)
    {
        return status;
    }

    g_current_task_id = task_id;

    return SCHEDULER_OK;
}

void os_scheduler_init(void)
{
    g_current_task_id = SCHEDULER_IDLE_TASK_ID;
    g_scheduler_state = SCHEDULER_STATE_INITIALIZED;
}

int32_t os_scheduler_start(void)
{
    if (g_scheduler_state == SCHEDULER_STATE_STOPPED)
    {
        os_scheduler_init();
    }

    g_scheduler_state = SCHEDULER_STATE_RUNNING;

    /*
     * This only selects the first logical task.
     * It does not jump to the selected task.
     */
    if (os_schedule_next() == 0)
    {
        return SCHEDULER_ERROR_NO_TASK;
    }

    return SCHEDULER_OK;
}

const task_control_block_t *os_get_current_task(void)
{
    if (g_current_task_id == SCHEDULER_INVALID_TASK_ID)
    {
        return 0;
    }

    return task_get_by_id(g_current_task_id);
}

uint32_t os_get_current_task_id(void)
{
    return g_current_task_id;
}

scheduler_state_t os_scheduler_get_state(void)
{
    return g_scheduler_state;
}

const task_control_block_t *os_schedule_next(void)
{
    uint32_t task_count;
    uint32_t offset;
    uint32_t candidate_id;
    const task_control_block_t *current_task;
    const task_control_block_t *candidate_task;
    const task_control_block_t *idle_task;

    task_count = task_get_count();

    if (task_count == 0U)
    {
        g_current_task_id = SCHEDULER_INVALID_TASK_ID;
        return 0;
    }

    /*
     * If the current task was logically RUNNING,
     * return it to READY before selecting the next one.
     */
    current_task = os_get_current_task();

    if ((current_task != 0) &&
        (current_task->state == TASK_STATE_RUNNING))
    {
        (void)task_set_state(current_task->id, TASK_STATE_READY);
    }

    /*
     * Round-robin selection.
     *
     * Task 0 is reserved for idle.
     * The scheduler skips idle during normal search and only selects it
     * if no application task is READY.
     */
    for (offset = 1U; offset <= task_count; offset++)
    {
        candidate_id = (g_current_task_id + offset) % task_count;

        if (candidate_id == SCHEDULER_IDLE_TASK_ID)
        {
            continue;
        }

        candidate_task = task_get_by_id(candidate_id);

        if (scheduler_is_ready_task(candidate_task) != 0U)
        {
            if (scheduler_set_current_task(candidate_id) == SCHEDULER_OK)
            {
                return task_get_by_id(candidate_id);
            }
        }
    }

    /*
     * No application task was READY.
     * Select idle task.
     */
    idle_task = task_get_by_id(SCHEDULER_IDLE_TASK_ID);

    if (scheduler_is_valid_task(idle_task) != 0U)
    {
        if (scheduler_set_current_task(SCHEDULER_IDLE_TASK_ID) == SCHEDULER_OK)
        {
            return task_get_by_id(SCHEDULER_IDLE_TASK_ID);
        }
    }

    g_current_task_id = SCHEDULER_INVALID_TASK_ID;

    return 0;
}

void os_yield(void)
{
    if (g_scheduler_state != SCHEDULER_STATE_RUNNING)
    {
        return;
    }

    /*
     * Phase 4:
     * Only select the next logical task.
     *
     * No CPU context is saved.
     * No stack pointer is changed.
     * No PendSV is triggered.
     */
    (void)os_schedule_next();
}

