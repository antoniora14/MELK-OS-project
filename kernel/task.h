/*
 * task.h
 *
 *  Created on: 17 may. 2026
 *      Author: anton
 */

#ifndef KERNEL_TASK_H_
#define KERNEL_TASK_H_

#include <stdint.h>

/*
 * MELK OS - Phase 3: Task Management
 *
 * This module only creates and stores task metadata.
 * It does not implement a scheduler, PendSV, or context switching yet.
 */

#define OS_MAX_TASKS                8U
#define OS_TASK_STACK_SIZE_WORDS    256U

#define TASK_IDLE_TASK_ID           0U

#define TASK_OK                     0
#define TASK_ERROR_NULL_ENTRY       -1
#define TASK_ERROR_NO_SPACE         -2
#define TASK_ERROR_INVALID_ID       -3
#define TASK_ERROR_INVALID_STACK    -4
#define TASK_ERROR_INVALID_STATE    -5

typedef void (*task_entry_t)(void *argument);

typedef enum
{
    TASK_STATE_UNUSED = 0,
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_SLEEPING,
    TASK_STATE_SUSPENDED
} task_state_t;

typedef struct
{
    uint32_t id;
    const char *name;
    task_entry_t entry;
    void *argument;

    task_state_t state;

    uint32_t *stack_base;
    uint32_t *stack_top;

    /* Saved PSP value for this task.
     *
     * During context switch:
     * - PendSV stores the current PSP here.
     * - PendSV loads this value when restoring the task.
    */
    uint32_t *stack_pointer;
    uint32_t stack_size_words;

    /* Absolute kernel tick at which a sleeping task becomes READY again. */
    uint32_t wakeup_tick;
} task_control_block_t;



void task_system_init(void);
int32_t task_create(const char *name, task_entry_t entry, void *argument);
int32_t task_set_state(uint32_t task_id, task_state_t state);
uint32_t task_get_count(void);
const task_control_block_t *task_get_table(void);
const task_control_block_t *task_get_by_id(uint32_t task_id);
void idle_task(void *argument);

uint32_t *task_get_stack_pointer(uint32_t task_id);
int32_t   task_set_stack_pointer(uint32_t task_id, uint32_t *stack_pointer);

int32_t task_sleep_until(uint32_t task_id, uint32_t wakeup_tick);
int32_t task_wake(uint32_t task_id);
uint32_t task_wake_expired_sleeping_tasks(uint32_t current_tick);

#endif /* KERNEL_TASK_H_ */
