/*
 * mutex.c
 *
 * MELK OS - Simple blocking mutex service
 *
 * Properties of this first implementation:
 * - Blocking mutex for task context only.
 * - Non-recursive.
 * - No timeout and no priority inheritance.
 * - Direct ownership handoff to a waiting task on unlock.
 * - No dynamic memory allocation.
 */

#include "mutex.h"
#include "context_switch.h"
#include "task.h"
#include "task_scheduler.h"

#if (OS_MAX_TASKS > 32U)
#error "os_mutex_t waiting_task_mask requires OS_MAX_TASKS <= 32U"
#endif

static uint32_t mutex_task_mask(uint32_t task_id)
{
    return (1UL << task_id);
}

static uint32_t mutex_is_valid_task_context(uint32_t task_id)
{
    /*
     * The initial mutex service is intended only for running tasks.
     * It must not be used before task execution starts or by idle.
     *
     * ISR callers are forbidden by the API contract. A future kernel
     * assertion layer can add an IPSR-based runtime check.
     */
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

static uint32_t mutex_find_next_waiting_task(os_mutex_t *mutex,
                                             uint32_t owner_task_id)
{
    uint32_t task_count;
    uint32_t offset;
    uint32_t candidate_id;
    uint32_t candidate_mask;
    const task_control_block_t *candidate_task;

    task_count = task_get_count();

    for (offset = 1U; offset <= task_count; offset++)
    {
        candidate_id = (owner_task_id + offset) % task_count;

        if (candidate_id == TASK_IDLE_TASK_ID)
        {
            continue;
        }

        candidate_mask = mutex_task_mask(candidate_id);

        if ((mutex->waiting_task_mask & candidate_mask) == 0U)
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
         * Defensive cleanup: a waiting bit without a blocked task is stale.
         * Under normal operation this branch should never be needed.
         */
        mutex->waiting_task_mask &= ~candidate_mask;
    }

    return OS_MUTEX_INVALID_OWNER_ID;
}

int32_t os_mutex_init(os_mutex_t *mutex)
{
    if (mutex == 0)
    {
        return OS_MUTEX_ERROR_NULL_POINTER;
    }

    mutex->is_locked = OS_MUTEX_UNLOCKED;
    mutex->owner_task_id = OS_MUTEX_INVALID_OWNER_ID;
    mutex->waiting_task_mask = 0U;
    mutex->is_initialized = OS_MUTEX_INITIALIZED;

    return OS_MUTEX_OK;
}

int32_t os_mutex_lock(os_mutex_t *mutex)
{
    uint32_t current_task_id;
    uint32_t current_task_mask;
    uint32_t irq_state;
    int32_t task_status;

    if (mutex == 0)
    {
        return OS_MUTEX_ERROR_NULL_POINTER;
    }

    if (mutex->is_initialized != OS_MUTEX_INITIALIZED)
    {
        return OS_MUTEX_ERROR_NOT_INITIALIZED;
    }

    current_task_id = os_get_current_task_id();

    if (mutex_is_valid_task_context(current_task_id) == 0U)
    {
        return OS_MUTEX_ERROR_INVALID_CONTEXT;
    }

    current_task_mask = mutex_task_mask(current_task_id);

    while (1)
    {
        irq_state = os_irq_save();

        if (mutex->is_initialized != OS_MUTEX_INITIALIZED)
        {
            os_irq_restore(irq_state);
            return OS_MUTEX_ERROR_NOT_INITIALIZED;
        }

        if (mutex->is_locked == OS_MUTEX_UNLOCKED)
        {
            mutex->is_locked = OS_MUTEX_LOCKED;
            mutex->owner_task_id = current_task_id;
            mutex->waiting_task_mask &= ~current_task_mask;

            os_irq_restore(irq_state);
            return OS_MUTEX_OK;
        }

        if (mutex->owner_task_id == current_task_id)
        {
            if ((mutex->waiting_task_mask & current_task_mask) != 0U)
            {
                /*
                 * Ownership was transferred directly by unlock while this
                 * task was blocked. Consume its waiting marker and proceed.
                 */
                mutex->waiting_task_mask &= ~current_task_mask;

                os_irq_restore(irq_state);
                return OS_MUTEX_OK;
            }

            os_irq_restore(irq_state);
            return OS_MUTEX_ERROR_RECURSIVE_LOCK;
        }

        /*
         * Blocking while interrupts were already disabled would prevent
         * PendSV from running and deadlock the task.
         */
        if (irq_state != 0U)
        {
            os_irq_restore(irq_state);
            return OS_MUTEX_ERROR_INTERRUPTS_DISABLED;
        }

        mutex->waiting_task_mask |= current_task_mask;

        task_status = task_block(current_task_id);

        if (task_status != TASK_OK)
        {
            mutex->waiting_task_mask &= ~current_task_mask;
            os_irq_restore(irq_state);
            return OS_MUTEX_ERROR_TASK_OPERATION;
        }

        /*
         * The current task is now BLOCKED. PendSV remains the only code
         * that performs the real context switch. When this task eventually
         * runs again, it continues at the top of this loop and completes
         * the direct-handoff acquisition path.
         */
        os_trigger_context_switch();
        os_irq_restore(irq_state);
    }
}

int32_t os_mutex_unlock(os_mutex_t *mutex)
{
    uint32_t current_task_id;
    uint32_t irq_state;
    uint32_t next_owner_task_id;
    int32_t task_status;

    if (mutex == 0)
    {
        return OS_MUTEX_ERROR_NULL_POINTER;
    }

    if (mutex->is_initialized != OS_MUTEX_INITIALIZED)
    {
        return OS_MUTEX_ERROR_NOT_INITIALIZED;
    }

    current_task_id = os_get_current_task_id();

    if (mutex_is_valid_task_context(current_task_id) == 0U)
    {
        return OS_MUTEX_ERROR_INVALID_CONTEXT;
    }

    irq_state = os_irq_save();

    if ((mutex->is_locked != OS_MUTEX_LOCKED) ||
        (mutex->owner_task_id != current_task_id))
    {
        os_irq_restore(irq_state);
        return OS_MUTEX_ERROR_NOT_OWNER;
    }

    next_owner_task_id = mutex_find_next_waiting_task(mutex, current_task_id);

    if (next_owner_task_id == OS_MUTEX_INVALID_OWNER_ID)
    {
        mutex->is_locked = OS_MUTEX_UNLOCKED;
        mutex->owner_task_id = OS_MUTEX_INVALID_OWNER_ID;

        os_irq_restore(irq_state);
        return OS_MUTEX_OK;
    }

    /*
     * Direct handoff: the mutex remains locked, but ownership changes
     * to the selected blocked task before it becomes READY. This prevents
     * another runnable task from stealing the mutex first.
     */
    task_status = task_unblock(next_owner_task_id);

    if (task_status != TASK_OK)
    {
        os_irq_restore(irq_state);
        return OS_MUTEX_ERROR_TASK_OPERATION;
    }

    mutex->owner_task_id = next_owner_task_id;
    mutex->is_locked = OS_MUTEX_LOCKED;

    os_irq_restore(irq_state);

    return OS_MUTEX_OK;
}
