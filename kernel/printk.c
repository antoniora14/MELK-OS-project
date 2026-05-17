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

void kernel_print_uint32(uint32_t value)
{
    char buffer[11];
    uint32_t index;

    if (value == 0U)
    {
        uart0_write_char('0');
        return;
    }

    index = 0U;
    while (value > 0U)
    {
        buffer[index] = (char)('0' + (value % 10U));
        value /= 10U;
        index++;
    }

    while (index > 0U)
    {
        index--;
        uart0_write_char(buffer[index]);
    }
}
