/*
 * uart.h
 *
 *  Created on: 11 may. 2026
 *      Author: anton
 */

#ifndef DRIVERS_UART_H_
#define DRIVERS_UART_H_

void uart0_init(void);
void uart0_write_char(char c);
void uart0_write_string(const char *str);

#endif /* DRIVERS_UART_H_ */
