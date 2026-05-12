/*
 * printk.c
 *
 *  Created on: 11 may. 2026
 *      Author: anton
 */

#include "printk.h"
#include "uart.h"

void kernel_print(const char *msg)
{
    uart0_write_string(msg);
}
