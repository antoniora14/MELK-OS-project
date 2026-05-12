/*
 * gpio.h
 *
 *  Created on: 11 may. 2026
 *      Author: anton
 */

#ifndef DRIVERS_GPIO_H_
#define DRIVERS_GPIO_H_

void gpio_init(void);
void gpio_toggle_red_led(void);
void gpio_red_led_on(void);
void gpio_red_led_off(void);

#endif /* DRIVERS_GPIO_H_ */
