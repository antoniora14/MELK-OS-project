#include <stdint.h>
#include "kernel.h"
#include "gpio.h"
#include "uart.h"
#include "printk.h"
#include "systick.h"
#include "system.h"
void kernel_main(void)
{
    gpio_init();
    gpio_red_led_on();

    uart0_init();
    systick_init(system_get_clock_hz());

    kernel_print("\n");
    kernel_print("================================\n");
    kernel_print(" MELK OS - Phase 2\n");
    kernel_print(" Bare-metal boot successful\n");
    kernel_print(" UART0 console initialized\n");
    kernel_print(" SysTick timer initialized\n");
    kernel_print("================================\n");

    os_delay_ms(500U);

    while(1)
    {
        gpio_toggle_red_led();
        kernel_print("MELK OS is running...\n");
        os_delay_ms(1000U);
    }
}
