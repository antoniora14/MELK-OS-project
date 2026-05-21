/*
 * systick.h
 *
 *  Created on: 13 may. 2026
 *      Author: anton
 */

#ifndef KERNEL_SYSTICK_H_
#define KERNEL_SYSTICK_H_

#include <stdint.h>

#define OS_TICKS_PER_SECOND    1000U
#define SYSTICK_OK             0U
#define SYSTICK_ERROR_CLOCK    1U
#define SYSTICK_MAX_RELOAD     0x00FFFFFFU

uint32_t systick_init(uint32_t system_clock_hz);
void systick_stop(void);
void systick_reset_ticks(void);
void systick_tick(void);
uint32_t os_get_ticks(void);
void os_delay_ms(uint32_t delay_ms);

#endif /* KERNEL_SYSTICK_H_ */
