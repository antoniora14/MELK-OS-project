/*
 * system.h
 *
 *  Created on: 13 may. 2026
 *      Author: anton
 */

#ifndef SYSTEM_SYSTEM_H_
#define SYSTEM_SYSTEM_H_

#include <stdint.h>

#define SYSTEM_CLOCK_HZ        16000000U

void SystemInit(void);
uint32_t system_get_clock_hz(void);

#endif /* SYSTEM_SYSTEM_H_ */
