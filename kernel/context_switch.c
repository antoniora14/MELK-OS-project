/*
 * context_switch.c
 *
 *  Created on: 21 may. 2026
 *      Author: anton
 */

#include <stdint.h>
#include "context_switch.h"
#include "task.h"
#include "task_scheduler.h"

/*
 * System Control Block registers.
 */
#define SCB_ICSR_R             (*((volatile uint32_t *)0xE000ED04U))
#define SCB_SHPR3_R            (*((volatile uint32_t *)0xE000ED20U))

#define SCB_ICSR_PENDSVSET     (1U << 28)

#define SCB_SHPR3_PENDSV_MASK  (0xFFU << 16)
#define SCB_SHPR3_PENDSV_LOW   (0xFFU << 16)


static volatile uint32_t g_context_switch_started = 0U;

/*
 * EXC_RETURN value:
 *
 * 0xFFFFFFFD means:
 * - Return to Thread mode
 * - Use PSP after return
 * - Basic stack frame
 */
#define EXC_RETURN_THREAD_PSP  0xFFFFFFFDU

static void os_data_sync_barrier(void)
{
    __asm("    DSB");
}

static void os_instruction_sync_barrier(void)
{
    __asm("    ISB");
}

void os_context_switch_init(void)
{
    g_context_switch_started = 0U;

    /* Set PendSV to the lowest priority.
     * This is important because PendSV should only run when no higher
     * priority interrupt is active.
     */
    SCB_SHPR3_R &= ~SCB_SHPR3_PENDSV_MASK;
    SCB_SHPR3_R |=  SCB_SHPR3_PENDSV_LOW;
}

void os_trigger_context_switch(void)
{
    /* Pend PendSV.
     * The CPU will enter PendSV after the current instruction stream
     * reaches a valid exception boundary.
     */
    SCB_ICSR_R = SCB_ICSR_PENDSVSET;

    os_data_sync_barrier();
    os_instruction_sync_barrier();
}

/*
 * Called from PendSV_Handler.
 *
 * r0 contains the updated PSP after saving R4-R11.
 */
void os_save_current_task_stack_pointer(uint32_t *stack_pointer)
{
    uint32_t current_task_id;

    current_task_id = os_get_current_task_id();

    if (current_task_id == SCHEDULER_INVALID_TASK_ID)
    {
        return;
    }

    (void)task_set_stack_pointer(current_task_id, stack_pointer);
}

/*
 * Called from SVC_Handler when starting the first task.
 */
uint32_t *os_get_current_task_stack_pointer_internal(void)
{
    const task_control_block_t *current_task;

    current_task = os_get_current_task();

    if (current_task == 0)
    {
        return 0;
    }

    return current_task->stack_pointer;
}

/*
 * Called from PendSV_Handler.
 *
 * This keeps the existing round-robin scheduler.
 */
uint32_t *os_schedule_next_stack_pointer(void)
{
    const task_control_block_t *next_task;

    next_task = os_schedule_next();

    if (next_task == 0)
    {
        return 0;
    }

    return next_task->stack_pointer;
}

void os_start_first_task(void)
{
    /*
     * Start the first task through SVC.
     *
     * Why SVC?
     * Because EXC_RETURN only works when returning from Handler mode.
     * SVC puts the CPU into Handler mode, then SVC_Handler performs an
     * exception return into the first task using PSP.
     */
    __asm("    SVC #0");

    /*
     * Should never return.
     */
    while (1)
    {
    }
}

uint32_t os_context_switch_is_started(void)
{
    return g_context_switch_started;
}

void os_context_switch_mark_started(void)
{
    g_context_switch_started = 1U;
}
