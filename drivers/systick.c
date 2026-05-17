/*
 * systick.c
 *
 *  Created on: 10 may. 2026
 *      Author: anton
 */

#include <stdint.h>
#include "systick.h"

#define SYSTICK_CTRL_R         (*((volatile uint32_t *)0xE000E010))
#define SYSTICK_RELOAD_R       (*((volatile uint32_t *)0xE000E014))
#define SYSTICK_CURRENT_R      (*((volatile uint32_t *)0xE000E018))

#define SYSTICK_CTRL_ENABLE    (1U << 0)
#define SYSTICK_CTRL_TICKINT   (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)

static volatile uint32_t g_os_ticks;

void systick_init(uint32_t system_clock_hz)
{
    uint32_t reload_value;

    reload_value = (system_clock_hz / OS_TICKS_PER_SECOND) - 1U;

    SYSTICK_CTRL_R = 0U;
    SYSTICK_RELOAD_R = reload_value;
    SYSTICK_CURRENT_R = 0U;
    g_os_ticks = 0U;
    SYSTICK_CTRL_R = SYSTICK_CTRL_CLKSOURCE |
                     SYSTICK_CTRL_TICKINT |
                     SYSTICK_CTRL_ENABLE;
}

void systick_tick(void)
{
    g_os_ticks++;
}

uint32_t os_get_ticks(void)
{
    return g_os_ticks;
}

void os_delay_ms(uint32_t delay_ms)
{
    uint32_t start_tick;

    start_tick = os_get_ticks();
    while ((uint32_t)(os_get_ticks() - start_tick) < delay_ms)
    {
    }
}

