/*
 * semaphore.h
 *
 *  Created on: 25 may. 2026
 *      Author: anton
 */

#ifndef KERNEL_SEMAPHORE_H_
#define KERNEL_SEMAPHORE_H_

#include <stdint.h>

#define OS_SEMAPHORE_OK                          0
#define OS_SEMAPHORE_ERROR_NULL_POINTER         -1
#define OS_SEMAPHORE_ERROR_NOT_INITIALIZED      -2
#define OS_SEMAPHORE_ERROR_INVALID_COUNT        -3
#define OS_SEMAPHORE_ERROR_INVALID_CONTEXT      -4
#define OS_SEMAPHORE_ERROR_TASK_OPERATION       -5
#define OS_SEMAPHORE_ERROR_INTERRUPTS_DISABLED  -6
#define OS_SEMAPHORE_ERROR_OVERFLOW             -7

#define OS_SEMAPHORE_NOT_INITIALIZED            0U
#define OS_SEMAPHORE_INITIALIZED                1U

#define OS_SEMAPHORE_INVALID_TASK_ID               0xFFFFFFFFU

typedef struct
{
    volatile uint32_t is_initialized;

    /* Number of immediately available semaphore units. */
    volatile uint32_t count;
    uint32_t maximum_count;

    /* One bit per task blocked while waiting for a semaphore unit.
     * MELK OS currently supports OS_MAX_TASKS <= 32 for this service.
     * Task 0 (idle) is never allowed to block on a semaphore. */
    volatile uint32_t waiting_task_mask;

    /* One bit per task that has received a direct unit handoff but has not
     * yet resumed from os_semaphore_wait(). */
    volatile uint32_t granted_task_mask;

    /* Used to choose blocked tasks in round-robin order on post. */
    volatile uint32_t last_granted_task_id;
} os_semaphore_t;

int32_t os_semaphore_init(os_semaphore_t *semaphore, uint32_t initial_count, uint32_t maximum_count);
int32_t os_semaphore_wait(os_semaphore_t *semaphore);
int32_t os_semaphore_post(os_semaphore_t *semaphore);

#endif /* KERNEL_SEMAPHORE_H_ */
