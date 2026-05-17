/*
 * systick.h
 *
 *  Created on: 13 may. 2026
 *      Author: anton
 */

#ifndef DRIVERS_SYSTICK_H_
#define DRIVERS_SYSTICK_H_

#include <stdint.h>

#define OS_TICKS_PER_SECOND    1000U

void systick_init(uint32_t system_clock_hz);
void systick_tick(void);
uint32_t os_get_ticks(void);
void os_delay_ms(uint32_t delay_ms);

#endif /* DRIVERS_SYSTICK_H_ */
