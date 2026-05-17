/*
 * printk.h
 *
 *  Created on: 11 may. 2026
 *      Author: anton
 */

#ifndef KERNEL_PRINTK_H_
#define KERNEL_PRINTK_H_

#include <stdint.h>

void kernel_print(const char *msg);
void kernel_print_uint32(uint32_t value);

#endif /* KERNEL_PRINTK_H_ */
