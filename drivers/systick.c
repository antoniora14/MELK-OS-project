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

static uint32_t systick_is_valid_clock(uint32_t system_clock_hz)
{
    uint32_t reload_value;

    if (system_clock_hz == 0U)
    {
        return 0U;
    }

    if ((system_clock_hz % OS_TICKS_PER_SECOND) != 0U)
    {
        return 0U;
    }

    reload_value = (system_clock_hz / OS_TICKS_PER_SECOND) - 1U;
    if (reload_value > SYSTICK_MAX_RELOAD)
    {
        return 0U;
    }

    return 1U;
}

uint32_t systick_init(uint32_t system_clock_hz)
{
    uint32_t reload_value;

    if (systick_is_valid_clock(system_clock_hz) == 0U)
    {
        systick_stop();
        g_os_ticks = 0U;
        return SYSTICK_ERROR_CLOCK;
    }

    reload_value = (system_clock_hz / OS_TICKS_PER_SECOND) - 1U;

    SYSTICK_CTRL_R = 0U;
    SYSTICK_RELOAD_R = reload_value;
    SYSTICK_CURRENT_R = 0U;
    g_os_ticks = 0U;
    SYSTICK_CTRL_R = SYSTICK_CTRL_CLKSOURCE |
                     SYSTICK_CTRL_TICKINT |
                     SYSTICK_CTRL_ENABLE;

    return SYSTICK_OK;
}

void systick_stop(void)
{
    SYSTICK_CTRL_R = 0U;
    SYSTICK_CURRENT_R = 0U;
}

void systick_reset_ticks(void)
{
    g_os_ticks = 0U;
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

