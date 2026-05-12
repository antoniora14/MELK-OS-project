

#include <stdint.h>
#include "kernel.h"
#include "gpio.h"
#include "uart.h"
#include "printk.h"


static void kernel_delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm(" NOP");
    }
}



void kernel_main(void)
{
    gpio_init();
    gpio_red_led_on();
    kernel_delay(3000000);

    uart0_init();

    kernel_print("\n");
    kernel_print("================================\n");
    kernel_print(" MELK OS - Phase 1\n");
    kernel_print(" Bare-metal boot successful\n");
    kernel_print(" UART0 console initialized\n");
    kernel_print("================================\n");

    while(1)
    {
        gpio_toggle_red_led();
        kernel_print("MELK OS is running...\n");
        kernel_delay(1000000);
    }
}
