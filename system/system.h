/*
 * system.h
 *
 *  Created on: 13 may. 2026
 *      Author: anton
 */

#ifndef SYSTEM_SYSTEM_H_
#define SYSTEM_SYSTEM_H_

#include <stdint.h>

/*
 * Configurable PLL system clock.
 * Common values: 80000000U, 50000000U, 40000000U, 25000000U, 16000000U.
 */
#ifndef SYSTEM_CLOCK_HZ
#define SYSTEM_CLOCK_HZ        80000000U
#endif

#define SYSTEM_PLL_VCO_HZ      400000000U
#define SYSTEM_MAIN_OSC_HZ     16000000U

void SystemInit(void);
uint32_t system_get_clock_hz(void);

#endif /* SYSTEM_SYSTEM_H_ */
