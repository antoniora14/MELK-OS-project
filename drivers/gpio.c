/*
 * gpio.c
 *
 *  Created on: 10 may. 2026
 *      Author: anton
 */

#include <stdint.h>
#include "gpio.h"

/*
 * System Control register for GPIO clock gating.
 */
#define SYSCTL_RCGCGPIO_R      (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R        (*((volatile uint32_t *)0x400FEA08))

/*
 * GPIO Port F base address.
 */
#define GPIO_PORTF_BASE        0x40025000

/*
 * GPIO Port F registers.
 */
#define GPIO_PORTF_DATA_R      (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x3FC)))
#define GPIO_PORTF_DIR_R       (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x400)))
#define GPIO_PORTF_DEN_R       (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x51C)))
#define GPIO_PORTF_AFSEL_R     (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x420)))
#define GPIO_PORTF_AMSEL_R     (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x528)))
#define GPIO_PORTF_PCTL_R      (*((volatile uint32_t *)(GPIO_PORTF_BASE + 0x52C)))

/*
 * Bit definitions.
 */
#define GPIO_PORTF_CLOCK       (1U << 5)
#define RED_LED                (1U << 1)


void gpio_init(void)
{
    /*
     * Enable GPIO Port F clock.
     */
    SYSCTL_RCGCGPIO_R |= GPIO_PORTF_CLOCK;

    /*
     * Wait until GPIO Port F is ready.
     */
    while ((SYSCTL_PRGPIO_R & GPIO_PORTF_CLOCK) == 0)
    {
    }

    /*
     * Disable alternate function for PF1.
     */
    GPIO_PORTF_AFSEL_R &= ~RED_LED;

    /*
     * Disable analog function for PF1.
     */
    GPIO_PORTF_AMSEL_R &= ~RED_LED;

    /*
     * Configure PF1 as GPIO in PCTL.
     */
    GPIO_PORTF_PCTL_R &= ~0x000000F0;

    /*
     * Configure PF1 as output.
     */
    GPIO_PORTF_DIR_R |= RED_LED;

    /*
     * Enable digital function for PF1.
     */
    GPIO_PORTF_DEN_R |= RED_LED;

    /*
     * Start with LED off.
     */
    GPIO_PORTF_DATA_R &= ~RED_LED;
}

void gpio_toggle_red_led(void)
{
    GPIO_PORTF_DATA_R ^= RED_LED;
}

void gpio_red_led_on(void)
{
    GPIO_PORTF_DATA_R |= RED_LED;
}

void gpio_red_led_off(void)
{
    GPIO_PORTF_DATA_R &= ~RED_LED;
}
