

#include <stdint.h>
#include "kernel.h"
#include "gpio.h"
#include "uart.h"
#include "printk.h"
#include "systick.h"
#include "system.h"
#include "task.h"

static void app_task_1(void *argument);
static void app_task_2(void *argument);

void kernel_main(void)
{
    uint32_t systick_status;
    int32_t task1_id;
    int32_t task2_id;

    gpio_init();
    uart0_init();
    systick_status = systick_init(system_get_clock_hz());

    kernel_print("WELCOME to MELK OS\n");
    kernel_print("[OK] Booting..\n");
    kernel_print("[OK] PLL System clock configured\n");
    kernel_print("[OK] GPIO configured explicitly\n");
    kernel_print("[OK] UART0 configured\n");

    if (systick_status == SYSTICK_OK)
    {
        kernel_print("[OK] SysTick configured for 1 ms tick\n");
    }
    else
    {
        kernel_print("[ERROR] SysTick configuration failed\n");
    }

    kernel_print(" System clock Hz: ");
    kernel_print_uint32(system_get_clock_hz());
    kernel_print("\n");

    kernel_print(" OS ticks per second: ");
    kernel_print_uint32(OS_TICKS_PER_SECOND);
    kernel_print("\n");

    task_system_init();

    task1_id = task_create("app_task_1", app_task_1, 0);
    task2_id = task_create("app_task_2", app_task_2, 0);

    kernel_print("[OK] Task system initialized\n");

    kernel_print(" Created tasks: ");
    kernel_print_uint32(task_get_count());
    kernel_print("\n");

    kernel_print(" app_task_1 id: ");
    kernel_print_uint32((uint32_t)task1_id);
    kernel_print("\n");

    kernel_print(" app_task_2 id: ");
    kernel_print_uint32((uint32_t)task2_id);
    kernel_print("\n");

    os_delay_ms(500U);

    while (1)
    {
        gpio_toggle_green_led();
        os_delay_ms(1000U);
    }
}

static void app_task_1(void *argument)
{
    (void)argument;

    while (1)
    {
    }
}

static void app_task_2(void *argument)
{
    (void)argument;

    while (1)
    {
    }
}
