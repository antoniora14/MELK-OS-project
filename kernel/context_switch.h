/*
 * context_switch.h
 *
 *  Created on: 21 may. 2026
 *      Author: anton
 */

#ifndef KERNEL_CONTEXT_SWITCH_H_
#define KERNEL_CONTEXT_SWITCH_H_

#include <stdint.h>


/* Initializes low-level context switch support.
 * This only configures PendSV priority.
 */
void os_context_switch_init(void);

/* Starts the first selected task.
 * This function does not return if the first task starts correctly.
 */
void os_start_first_task(void);

/* Triggers PendSV.
 * Used by os_yield() for cooperative context switching.
 */
void os_trigger_context_switch(void);

/* Returns 1 when the first task has already started using PSP.
 * This is used to avoid triggering PendSV from SysTick before the CPU
 * has switched from the kernel startup context to a task context.
 */
uint32_t os_context_switch_is_started(void);

/* Marks that the first task context has started.
 * This function is called from SVC_Handler, right before returning into
 * the first task.
 */
void os_context_switch_mark_started(void);

/* Saves PRIMASK and disables interrupts. */
uint32_t os_irq_save(void);

/* Restores PRIMASK to the value returned by os_irq_save(). */
void os_irq_restore(uint32_t primask);


#endif /* KERNEL_CONTEXT_SWITCH_H_ */
