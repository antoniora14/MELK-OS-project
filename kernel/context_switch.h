/*
 * context_switch.h
 *
 *  Created on: 21 may. 2026
 *      Author: anton
 */

#ifndef KERNEL_CONTEXT_SWITCH_H_
#define KERNEL_CONTEXT_SWITCH_H_

#include <stdint.h>

/*
 * Initializes low-level context switch support.
 *
 * For Phase 5 this only configures PendSV priority.
 */
void os_context_switch_init(void);

/*
 * Starts the first selected task.
 *
 * This function does not return if the first task starts correctly.
 */
void os_start_first_task(void);

/*
 * Triggers PendSV.
 *
 * Used by os_yield() for cooperative context switching.
 */
void os_trigger_context_switch(void);


#endif /* KERNEL_CONTEXT_SWITCH_H_ */
