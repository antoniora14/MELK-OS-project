/*
 * semaphore.c
 *
 *  Created on: 25 may. 2026
 *      Author: anton
 */


#include "semaphore.h"
#include "context_switch.h"
#include "task.h"
#include "task_scheduler.h"

#if (OS_MAX_TASKS > 32U)
#error "os_semaphore_t task masks require OS_MAX_TASKS <= 32U"
#endif


static uint32_t semaphore_task_mask(uint32_t task_id)
{
    return (1UL << task_id);
}

static uint32_t semaphore_is_valid_task_context(uint32_t task_id)
{
    /*
     * This initial API is intentionally task-only. A wait operation can
     * block and therefore must never run in Handler mode. post() follows
     * the same rule until a dedicated ISR-safe API is designed.
     */
    if (os_get_exception_number() != 0U)
    {
        return 0U;
    }

    if (os_scheduler_get_state() != SCHEDULER_STATE_RUNNING)
    {
        return 0U;
    }

    if (os_context_switch_is_started() == 0U)
    {
        return 0U;
    }

    if ((task_id == SCHEDULER_INVALID_TASK_ID) ||
        (task_id == TASK_IDLE_TASK_ID) ||
        (task_id >= task_get_count()))
    {
        return 0U;
    }

    return 1U;
}

static uint32_t semaphore_find_next_waiting_task(os_semaphore_t *semaphore)
{
    uint32_t task_count;
    uint32_t offset;
    uint32_t candidate_id;
    uint32_t candidate_mask;
    const task_control_block_t *candidate_task;

    task_count = task_get_count();

    for (offset = 1U; offset <= task_count; offset++)
    {
        candidate_id = (semaphore->last_granted_task_id + offset) % task_count;

        if (candidate_id == TASK_IDLE_TASK_ID)
        {
            continue;
        }

        candidate_mask = semaphore_task_mask(candidate_id);

        if ((semaphore->waiting_task_mask & candidate_mask) == 0U)
        {
            continue;
        }

        candidate_task = task_get_by_id(candidate_id);

        if ((candidate_task != 0) &&
            (candidate_task->state == TASK_STATE_BLOCKED))
        {
            return candidate_id;
        }

        /*
         * Defensive cleanup: a waiting marker without a blocked task is
         * stale and must not prevent another task from receiving a unit.
         */
        semaphore->waiting_task_mask &= ~candidate_mask;
    }

    return OS_SEMAPHORE_INVALID_TASK_ID;
}

int32_t os_semaphore_init(os_semaphore_t *semaphore, uint32_t initial_count, uint32_t maximum_count)
{
    if (semaphore == 0)
    {
        return OS_SEMAPHORE_ERROR_NULL_POINTER;
    }

    if ((maximum_count == 0U) || (initial_count > maximum_count))
    {
        return OS_SEMAPHORE_ERROR_INVALID_COUNT;
    }

    semaphore->count = initial_count;
    semaphore->maximum_count = maximum_count;
    semaphore->waiting_task_mask = 0U;
    semaphore->granted_task_mask = 0U;
    semaphore->last_granted_task_id = TASK_IDLE_TASK_ID;
    semaphore->is_initialized = OS_SEMAPHORE_INITIALIZED;

    return OS_SEMAPHORE_OK;
}

int32_t os_semaphore_wait(os_semaphore_t *semaphore)
{
    uint32_t current_task_id;
    uint32_t current_task_mask;
    uint32_t irq_state;
    int32_t task_status;

    if (semaphore == 0)
    {
        return OS_SEMAPHORE_ERROR_NULL_POINTER;
    }

    if (semaphore->is_initialized != OS_SEMAPHORE_INITIALIZED)
    {
        return OS_SEMAPHORE_ERROR_NOT_INITIALIZED;
    }

    current_task_id = os_get_current_task_id();

    if (semaphore_is_valid_task_context(current_task_id) == 0U)
    {
        return OS_SEMAPHORE_ERROR_INVALID_CONTEXT;
    }

    current_task_mask = semaphore_task_mask(current_task_id);

    while (1)
    {
        irq_state = os_irq_save();

        if (semaphore->is_initialized != OS_SEMAPHORE_INITIALIZED)
        {
            os_irq_restore(irq_state);
            return OS_SEMAPHORE_ERROR_NOT_INITIALIZED;
        }

        if ((semaphore->granted_task_mask & current_task_mask) != 0U)
        {
            /* Consume a unit that post() transferred directly to this task. */
            semaphore->granted_task_mask &= ~current_task_mask;
            semaphore->waiting_task_mask &= ~current_task_mask;

            os_irq_restore(irq_state);
            return OS_SEMAPHORE_OK;
        }

        if (semaphore->count > 0U)
        {
            semaphore->count--;
            semaphore->waiting_task_mask &= ~current_task_mask;

            os_irq_restore(irq_state);
            return OS_SEMAPHORE_OK;
        }

        /*
         * Blocking while interrupts were already disabled would prevent
         * PendSV from selecting another task and would deadlock execution.
         */
        if (irq_state != 0U)
        {
            os_irq_restore(irq_state);
            return OS_SEMAPHORE_ERROR_INTERRUPTS_DISABLED;
        }

        semaphore->waiting_task_mask |= current_task_mask;

        task_status = task_block(current_task_id);

        if (task_status != TASK_OK)
        {
            semaphore->waiting_task_mask &= ~current_task_mask;
            os_irq_restore(irq_state);
            return OS_SEMAPHORE_ERROR_TASK_OPERATION;
        }

        /*
         * The current task is BLOCKED. PendSV remains the only component
         * that performs an actual context switch. When post() grants a unit,
         * the task becomes READY and resumes at the top of this loop.
         */
        os_trigger_context_switch();
        os_irq_restore(irq_state);
    }
}

int32_t os_semaphore_post(os_semaphore_t *semaphore)
{
    uint32_t current_task_id;
    uint32_t irq_state;
    uint32_t granted_task_id;
    uint32_t granted_task_mask;
    int32_t task_status;

    if (semaphore == 0)
    {
        return OS_SEMAPHORE_ERROR_NULL_POINTER;
    }

    if (semaphore->is_initialized != OS_SEMAPHORE_INITIALIZED)
    {
        return OS_SEMAPHORE_ERROR_NOT_INITIALIZED;
    }

    current_task_id = os_get_current_task_id();

    if (semaphore_is_valid_task_context(current_task_id) == 0U)
    {
        return OS_SEMAPHORE_ERROR_INVALID_CONTEXT;
    }

    irq_state = os_irq_save();

    granted_task_id = semaphore_find_next_waiting_task(semaphore);

    if (granted_task_id != OS_SEMAPHORE_INVALID_TASK_ID)
    {
        granted_task_mask = semaphore_task_mask(granted_task_id);

        /*
         * Direct handoff: the posted unit is reserved for the selected
         * waiting task rather than becoming available for a later caller.
         */
        semaphore->waiting_task_mask &= ~granted_task_mask;
        semaphore->granted_task_mask |= granted_task_mask;
        semaphore->last_granted_task_id = granted_task_id;

        task_status = task_unblock(granted_task_id);

        if (task_status != TASK_OK)
        {
            semaphore->granted_task_mask &= ~granted_task_mask;
            semaphore->waiting_task_mask |= granted_task_mask;
            os_irq_restore(irq_state);
            return OS_SEMAPHORE_ERROR_TASK_OPERATION;
        }

        os_irq_restore(irq_state);
        return OS_SEMAPHORE_OK;
    }

    if (semaphore->count >= semaphore->maximum_count)
    {
        os_irq_restore(irq_state);
        return OS_SEMAPHORE_ERROR_OVERFLOW;
    }

    semaphore->count++;

    os_irq_restore(irq_state);

    return OS_SEMAPHORE_OK;
}

