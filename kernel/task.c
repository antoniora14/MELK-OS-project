/*
 * task.c
 *
 *  Created on: 17 may. 2026
 *      Author: anton
 */

#include "task.h"

/*
 * Static task table.
 * For now, MELK OS uses a fixed-size task table.
 * Dynamic memory allocation is intentionally avoided.
 */
static task_control_block_t g_task_table[OS_MAX_TASKS];

/*
 * Static stacks for all tasks.
 * Each task receives one private stack.
 */
static uint32_t g_task_stacks[OS_MAX_TASKS][OS_TASK_STACK_SIZE_WORDS];

/*
 * Number of created tasks.
 */
static uint32_t g_task_count = 0U;


static void task_exit_trap(void)
{
    /*
     * A task should never return.
     *
     * If execution reaches this function, it means a task returned from
     * its entry function. For now, stop here.
     */
    while (1)
    {
    }
}


static uint32_t *task_prepare_initial_stack(uint32_t *stack_top,
                                            task_entry_t entry,
                                            void *argument)
{
    uint32_t *sp;

    /*
     * Cortex-M expects the process stack to be 8-byte aligned on exception
     * entry/return. Align the initial top of stack down to 8 bytes.
     */
    sp = (uint32_t *)((uint32_t)stack_top & ~0x7U);

    /*
     * Initial Cortex-M hardware exception frame.
     *
     * This is the frame that the CPU automatically restores on exception
     * return:
     *
     *   R0, R1, R2, R3, R12, LR, PC, xPSR
     *
     * The order below is reversed because the stack grows downward.
     */
    *(--sp) = 0x01000000U;              /* xPSR: Thumb bit set */
    *(--sp) = (uint32_t)entry;          /* PC: task entry point */
    *(--sp) = (uint32_t)task_exit_trap; /* LR: where task goes if it returns */
    *(--sp) = 0x12121212U;              /* R12 */
    *(--sp) = 0x03030303U;              /* R3 */
    *(--sp) = 0x02020202U;              /* R2 */
    *(--sp) = 0x01010101U;              /* R1 */
    *(--sp) = (uint32_t)argument;       /* R0: task argument */

    /*
     * Software-saved registers.
     *
     * PendSV restores these manually before exception return.
     */
    *(--sp) = 0x11111111U;              /* R11 */
    *(--sp) = 0x10101010U;              /* R10 */
    *(--sp) = 0x09090909U;              /* R9 */
    *(--sp) = 0x08080808U;              /* R8 */
    *(--sp) = 0x07070707U;              /* R7 */
    *(--sp) = 0x06060606U;              /* R6 */
    *(--sp) = 0x05050505U;              /* R5 */
    *(--sp) = 0x04040404U;              /* R4 */

    return sp;
}

void task_system_init(void)
{
    uint32_t i;

    g_task_count = 0U;

    for (i = 0U; i < OS_MAX_TASKS; i++)
    {
        g_task_table[i].id = i;
        g_task_table[i].name = 0;
        g_task_table[i].entry = 0;
        g_task_table[i].argument = 0;
        g_task_table[i].state = TASK_STATE_UNUSED;
        g_task_table[i].stack_base = 0;
        g_task_table[i].stack_top = 0;
        g_task_table[i].stack_pointer = 0;
        g_task_table[i].stack_size_words = 0U;
    }

    /*
     * Reserve task 0 for the idle task.
     * The scheduler will use this task later when no other task is READY.
     */
    (void)task_create("idle", idle_task, 0);
}

int32_t task_create(const char *name,
                    task_entry_t entry,
                    void *argument)
{
    uint32_t task_id;
    uint32_t *stack_base;
    uint32_t *stack_top;

    if (entry == 0)
    {
        return TASK_ERROR_NULL_ENTRY;
    }

    if (g_task_count >= OS_MAX_TASKS)
    {
        return TASK_ERROR_NO_SPACE;
    }

    task_id = g_task_count;

    stack_base = &g_task_stacks[task_id][0];
    stack_top = &g_task_stacks[task_id][OS_TASK_STACK_SIZE_WORDS];

    g_task_table[task_id].id = task_id;
    g_task_table[task_id].name = name;
    g_task_table[task_id].entry = entry;
    g_task_table[task_id].argument = argument;
    g_task_table[task_id].state = TASK_STATE_READY;
    g_task_table[task_id].stack_base = stack_base;
    g_task_table[task_id].stack_top = stack_top;
    g_task_table[task_id].stack_pointer =
        task_prepare_initial_stack(stack_top, entry, argument);
    g_task_table[task_id].stack_size_words = OS_TASK_STACK_SIZE_WORDS;

    g_task_count++;

    return (int32_t)task_id;
}

uint32_t task_get_count(void)
{
    return g_task_count;
}

const task_control_block_t *task_get_table(void)
{
    return g_task_table;
}

const task_control_block_t *task_get_by_id(uint32_t task_id)
{
    if (task_id >= g_task_count)
    {
        return 0;
    }

    return &g_task_table[task_id];
}

void idle_task(void *argument)
{
    (void)argument;

    while (1)
    {
        /*
         * Phase 3:
         * The idle task exists only as a registered task.
         * It is not executed by a scheduler yet.
         *
         * Later this can use WFI:
         * __asm(" WFI");
         */
    }
}

int32_t task_set_state(uint32_t task_id, task_state_t state)
{
    if (task_id >= g_task_count)
    {
        return TASK_ERROR_INVALID_ID;
    }

    g_task_table[task_id].state = state;

    return TASK_OK;
}

uint32_t *task_get_stack_pointer(uint32_t task_id)
{
    if (task_id >= g_task_count)
    {
        return 0;
    }

    return g_task_table[task_id].stack_pointer;
}

int32_t task_set_stack_pointer(uint32_t task_id, uint32_t *stack_pointer)
{
    if (task_id >= g_task_count)
    {
        return TASK_ERROR_INVALID_ID;
    }

    if (stack_pointer == 0)
    {
        return TASK_ERROR_INVALID_STACK;
    }

    if ((stack_pointer < g_task_table[task_id].stack_base) ||
        (stack_pointer > g_task_table[task_id].stack_top))
    {
        return TASK_ERROR_INVALID_STACK;
    }

    g_task_table[task_id].stack_pointer = stack_pointer;

    return TASK_OK;
}


