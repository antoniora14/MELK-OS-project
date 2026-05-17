

#include <stdint.h>
#include "kernel.h"
#include "gpio.h"
#include "uart.h"
#include "printk.h"
#include "systick.h"
#include "system.h"


void kernel_main(void)
{
    uint32_t systick_status;

    gpio_init();
    uart0_init();
    systick_status = systick_init(system_get_clock_hz());

    kernel_print("WELCOME to MELK OS\n");
    kernel_print("[OK] Booting..\n");
    kernel_print("[OK] PLL System clock configured\n");
    kernel_print("[OK] GPIO configured explicitly\n");
    kernel_print("[OK] UART0 configured\n");
    kernel_print("[OK] SysTick configured for 1 ms tick\n");
    kernel_print(" System clock Hz: ");
    kernel_print_uint32(system_get_clock_hz());
    kernel_print(" OS ticks per second: ");
    kernel_print_uint32(OS_TICKS_PER_SECOND);

    os_delay_ms(500U);
    while(1)
    {
        gpio_toggle_green_led();
       //kernel_print("MELK OS is running...\n");
        os_delay_ms(1000);
    }
}
