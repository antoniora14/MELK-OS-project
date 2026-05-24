/*
 * mutex.h
 *
 * MELK OS - Simple blocking mutex service
 */

#ifndef KERNEL_MUTEX_H_
#define KERNEL_MUTEX_H_

#include <stdint.h>

#define OS_MUTEX_OK                            0
#define OS_MUTEX_ERROR_NULL_POINTER           -1
#define OS_MUTEX_ERROR_NOT_INITIALIZED        -2
#define OS_MUTEX_ERROR_INVALID_CONTEXT        -3
#define OS_MUTEX_ERROR_RECURSIVE_LOCK         -4
#define OS_MUTEX_ERROR_NOT_OWNER              -5
#define OS_MUTEX_ERROR_TASK_OPERATION         -6
#define OS_MUTEX_ERROR_INTERRUPTS_DISABLED    -7

#define OS_MUTEX_NOT_INITIALIZED               0U
#define OS_MUTEX_INITIALIZED                   1U

#define OS_MUTEX_UNLOCKED                      0U
#define OS_MUTEX_LOCKED                        1U

#define OS_MUTEX_INVALID_OWNER_ID              0xFFFFFFFFU

typedef struct
{
    volatile uint32_t is_initialized;
    volatile uint32_t is_locked;
    volatile uint32_t owner_task_id;

    /*
     * One bit per task waiting for this mutex.
     * MELK OS currently supports OS_MAX_TASKS <= 32 for this service.
     * Task 0 (idle) is never allowed to wait for a mutex.
     */
    volatile uint32_t waiting_task_mask;
} os_mutex_t;

int32_t os_mutex_init(os_mutex_t *mutex);
int32_t os_mutex_lock(os_mutex_t *mutex);
int32_t os_mutex_unlock(os_mutex_t *mutex);

#endif /* KERNEL_MUTEX_H_ */
