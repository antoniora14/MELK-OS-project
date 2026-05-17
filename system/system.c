/*
 * system.c
 *
 *  Created on: 13 may. 2026
 *      Author: anton
 */

#include "system.h"

void SystemInit(void)
{
    /*
     * Phase 2 keeps the TM4C123 reset clock configuration.
     * The precision PLL setup will be added in a later phase.
     */
}

uint32_t system_get_clock_hz(void)
{
    return SYSTEM_CLOCK_HZ;
}
