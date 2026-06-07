

#include <stdint.h>
#include "kernel.h"
#include "gpio.h"
#include "uart.h"
#include "printk.h"
#include "systick.h"
#include "system.h"
#include "task.h"
#include "task_scheduler.h"
#include "context_switch.h"
#include "mutex.h"
#include "semaphore.h"
#include "message_queue.h"


/***********************************************************
 * Optional validation modes.
 * Enable only the validation you are currently executing.
 **********************************************************/
//#define MELK_OS_SLEEP_DEBUG
//#define MELK_OS_MUTEX_DEBUG
//#define MELK_OS_SEMAPHORE_DEBUG
//#define MELK_OS_MESSAGE_QUEUE_DEBUG



/* Set to 1U while validating the counting semaphore service.
   Set to 0U to restore the normal sleep/mutex demonstration.*/
#define MELK_OS_SEMAPHORE_DEMO_ENABLED       0U

/* Set to 1U while validating the blocking static message queue service. */
#define MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED   1U

#define KERNEL_SEMAPHORE_RESOURCE_CAPACITY   2U
#define KERNEL_SEMAPHORE_RESOURCE_HOLD_MS    200U
#define KERNEL_TASK_1_REST_MS                50U
#define KERNEL_TASK_2_REST_MS                75U
#define KERNEL_TASK_3_REST_MS                100U

/* Set to 1U to force queue-full conditions with fast producers and
   a slower consumer. Keep 0U for the normal educational demo. */
#define KERNEL_MESSAGE_QUEUE_STRESS_TEST     0U

#if (KERNEL_MESSAGE_QUEUE_STRESS_TEST != 0U)
#define KERNEL_MESSAGE_QUEUE_CAPACITY        2U
#define KERNEL_PRODUCER_1_SLEEP_MS           20U
#define KERNEL_PRODUCER_2_SLEEP_MS           30U
#define KERNEL_CONSUMER_PROCESSING_MS        300U
#else
#define KERNEL_MESSAGE_QUEUE_CAPACITY        4U
#define KERNEL_PRODUCER_1_SLEEP_MS           500U
#define KERNEL_PRODUCER_2_SLEEP_MS           1000U
#define KERNEL_CONSUMER_PROCESSING_MS        0U
#endif

#if defined(MELK_OS_SLEEP_DEBUG) && \
    ((MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U) || \
     (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U))
    #error "Disable demo modes while validating os_sleep timing"
#endif

#if (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U) && \
    (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    #error "Enable only one kernel service demo at a time"
#endif

static os_mutex_t g_console_mutex;

#if (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    static os_semaphore_t g_resource_semaphore;
#endif

#if (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
typedef struct
{
    uint32_t producer_id;
    uint32_t sequence;
    uint32_t production_tick;
} kernel_queue_message_t;

static kernel_queue_message_t
    g_message_queue_storage[KERNEL_MESSAGE_QUEUE_CAPACITY];

static os_message_queue_t g_message_queue;
#endif

#ifdef MELK_OS_SLEEP_DEBUG
    typedef struct
    {
        volatile uint32_t sample_count;
        volatile uint32_t requested_ticks;
        volatile uint32_t last_elapsed_ticks;
        volatile uint32_t minimum_elapsed_ticks;
        volatile uint32_t maximum_elapsed_ticks;
        volatile uint32_t early_wakeup_count;
    } sleep_test_statistics_t;

    volatile sleep_test_statistics_t g_task_1_sleep_stats =
    { 0U, 500U, 0U, 0xFFFFFFFFU, 0U, 0U };

    volatile sleep_test_statistics_t g_task_2_sleep_stats =
    { 0U, 1000U, 0U, 0xFFFFFFFFU, 0U, 0U };

    volatile sleep_test_statistics_t g_task_3_sleep_stats =
    { 0U, 2000U, 0U, 0xFFFFFFFFU, 0U, 0U };

    static void kernel_record_sleep_measurement(
            volatile sleep_test_statistics_t *statistics,
            uint32_t elapsed_ticks);
#endif

#ifdef MELK_OS_MUTEX_DEBUG
    volatile uint32_t g_mutex_critical_depth = 0U;
    volatile uint32_t g_mutex_violation_count = 0U;
    volatile uint32_t g_mutex_protected_print_count = 0U;
    volatile uint32_t g_mutex_error_count = 0U;
#endif

#if defined(MELK_OS_SEMAPHORE_DEBUG) && (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    volatile uint32_t g_semaphore_active_users = 0U;
    volatile uint32_t g_semaphore_max_active_users = 0U;
    volatile uint32_t g_semaphore_violation_count = 0U;
    volatile uint32_t g_semaphore_wait_success_count = 0U;
    volatile uint32_t g_semaphore_post_success_count = 0U;
    volatile uint32_t g_semaphore_error_count = 0U;
#endif

#if defined(MELK_OS_MESSAGE_QUEUE_DEBUG) && \
    (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    volatile uint32_t g_message_queue_send_success_count = 0U;
    volatile uint32_t g_message_queue_receive_success_count = 0U;
    volatile uint32_t g_message_queue_error_count = 0U;
    volatile uint32_t g_message_queue_producer_1_expected_sequence = 0U;
    volatile uint32_t g_message_queue_producer_2_expected_sequence = 0U;
    volatile uint32_t g_message_queue_sequence_error_count = 0U;
#endif

static void app_task_1(void *argument);
static void app_task_2(void *argument);
static void app_task_3(void *argument);

#ifndef MELK_OS_SLEEP_DEBUG
    static void kernel_write_task_event(const char *task_name, const char *event);
#if (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    static void kernel_run_semaphore_worker(const char *task_name,
                                        void (*toggle_led)(void),
                                        uint32_t rest_time_ms);
#ifdef MELK_OS_SEMAPHORE_DEBUG
    static void kernel_record_semaphore_enter(void);
    static void kernel_record_semaphore_exit(void);
    static void kernel_record_semaphore_post_success(void);
    static void kernel_record_semaphore_error(void);
#endif
#endif

#if (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    static void kernel_run_queue_producer(const char *task_name,
                                          uint32_t producer_id,
                                          void (*toggle_led)(void),
                                          uint32_t period_ms);
    static void kernel_run_queue_consumer(const char *task_name,
                                          void (*toggle_led)(void));
    static void kernel_write_queue_message(const char *task_name,
                                           const char *event,
                                           const kernel_queue_message_t *message);
#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
    static void kernel_record_message_queue_send_success(void);
    static void kernel_record_message_queue_receive_success(
            const kernel_queue_message_t *message);
    static void kernel_record_message_queue_error(void);
#endif
#endif
#endif



/***********************************************************
 *
 *
 *
 **********************************************************/
void kernel_main(void)
{
    uint32_t systick_status;
    int32_t mutex_status;
#if (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    int32_t semaphore_status;
#endif
#if (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    int32_t message_queue_status;
#endif
    int32_t task1_id;
    int32_t task2_id;
    int32_t task3_id;

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

    mutex_status = os_mutex_init(&g_console_mutex);
    if (mutex_status == OS_MUTEX_OK)
    {
        kernel_print("[OK] Console mutex initialized\n");
    }
    else
    {
        kernel_print("[ERROR] Console mutex initialization failed\n");
        while (1)
        {
        }
    }

#if (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    semaphore_status = os_semaphore_init(&g_resource_semaphore,
                                         KERNEL_SEMAPHORE_RESOURCE_CAPACITY,
                                         KERNEL_SEMAPHORE_RESOURCE_CAPACITY);

    if (semaphore_status == OS_SEMAPHORE_OK)
    {
        kernel_print("[OK] Counting semaphore initialized with 2 resource slots\n");
    }
    else
    {
        kernel_print("[ERROR] Counting semaphore initialization failed\n");
        while (1)
        {
        }
    }
#endif

#if (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    message_queue_status = os_message_queue_init(&g_message_queue,
                                                 g_message_queue_storage,
                                                 sizeof(kernel_queue_message_t),
                                                 KERNEL_MESSAGE_QUEUE_CAPACITY);

    if (message_queue_status == OS_MESSAGE_QUEUE_OK)
    {
        kernel_print("[OK] Message queue initialized with capacity ");
        kernel_print_uint32(KERNEL_MESSAGE_QUEUE_CAPACITY);
        kernel_print("\n");
    }
    else
    {
        kernel_print("[ERROR] Message queue initialization failed\n");
        while (1)
        {
        }
    }
#endif

    task1_id = task_create("app_task_1", app_task_1, 0);
    task2_id = task_create("app_task_2", app_task_2, 0);
    task3_id = task_create("app_task_3", app_task_3, 0);

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

    kernel_print(" app_task_3 id: ");
    kernel_print_uint32((uint32_t)task3_id);
    kernel_print("\n");

    os_scheduler_init();
    os_context_switch_init();

    if (os_scheduler_start() == SCHEDULER_OK)
    {
        kernel_print("[OK] Preemptive scheduler initialized\n");
    }
    else
    {
        kernel_print("[ERROR] Preemptive scheduler failed to start\n");
        while (1)
        {
        }
    }

    os_scheduler_enable_preemption();

    kernel_print("[OK] Starting first real task using PSP\n");

    os_start_first_task();

    /* os_start_first_task() should never return. */
    while (1)
    {
    }
}


#ifdef MELK_OS_SLEEP_DEBUG
    static void kernel_record_sleep_measurement(
        volatile sleep_test_statistics_t *statistics,
        uint32_t elapsed_ticks)
    {
        statistics->sample_count++;
        statistics->last_elapsed_ticks = elapsed_ticks;

        if (elapsed_ticks < statistics->minimum_elapsed_ticks)
        {
            statistics->minimum_elapsed_ticks = elapsed_ticks;
        }

        if (elapsed_ticks > statistics->maximum_elapsed_ticks)
        {
            statistics->maximum_elapsed_ticks = elapsed_ticks;
        }

        if (elapsed_ticks < statistics->requested_ticks)
        {
            statistics->early_wakeup_count++;
        }
    }
#endif

#ifndef MELK_OS_SLEEP_DEBUG
static void kernel_write_task_event(const char *task_name, const char *event)
{
    int32_t mutex_status;

    mutex_status = os_mutex_lock(&g_console_mutex);

    if (mutex_status != OS_MUTEX_OK)
    {
#ifdef MELK_OS_MUTEX_DEBUG
        g_mutex_error_count++;
#endif
        return;
    }

#ifdef MELK_OS_MUTEX_DEBUG
    g_mutex_critical_depth++;

    if (g_mutex_critical_depth > 1U)
    {
        g_mutex_violation_count++;
    }
#endif

    kernel_print("[");
    kernel_print(task_name);
    kernel_print("] tick=");
    kernel_print_uint32(os_get_ticks());
    kernel_print(" ");
    kernel_print(event);
    kernel_print("\n");

#ifdef MELK_OS_MUTEX_DEBUG
    g_mutex_protected_print_count++;
    g_mutex_critical_depth--;
#endif

    mutex_status = os_mutex_unlock(&g_console_mutex);

#ifdef MELK_OS_MUTEX_DEBUG
    if (mutex_status != OS_MUTEX_OK)
    {
        g_mutex_error_count++;
    }
#else
    (void)mutex_status;
#endif
}

#if (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
#ifdef MELK_OS_SEMAPHORE_DEBUG
static void kernel_record_semaphore_enter(void)
{
    uint32_t irq_state;

    irq_state = os_irq_save();

    g_semaphore_active_users++;
    g_semaphore_wait_success_count++;

    if (g_semaphore_active_users > g_semaphore_max_active_users)
    {
        g_semaphore_max_active_users = g_semaphore_active_users;
    }

    if (g_semaphore_active_users > KERNEL_SEMAPHORE_RESOURCE_CAPACITY)
    {
        g_semaphore_violation_count++;
    }

    os_irq_restore(irq_state);
}

static void kernel_record_semaphore_exit(void)
{
    uint32_t irq_state;

    irq_state = os_irq_save();

    if (g_semaphore_active_users == 0U)
    {
        g_semaphore_violation_count++;
    }
    else
    {
        g_semaphore_active_users--;
    }

    os_irq_restore(irq_state);
}

static void kernel_record_semaphore_post_success(void)
{
    uint32_t irq_state;

    irq_state = os_irq_save();
    g_semaphore_post_success_count++;
    os_irq_restore(irq_state);
}

static void kernel_record_semaphore_error(void)
{
    uint32_t irq_state;

    irq_state = os_irq_save();
    g_semaphore_error_count++;
    os_irq_restore(irq_state);
}
#endif /* MELK_OS_SEMAPHORE_DEBUG */

static void kernel_run_semaphore_worker(
        const char *task_name,
        void (*toggle_led)(void),
        uint32_t rest_time_ms)
{
    int32_t semaphore_status;

    while (1)
    {
        semaphore_status = os_semaphore_wait(&g_resource_semaphore);

        if (semaphore_status != OS_SEMAPHORE_OK)
        {
#ifdef MELK_OS_SEMAPHORE_DEBUG
        kernel_record_semaphore_error();
#endif
            os_sleep(rest_time_ms);
            continue;
        }

#ifdef MELK_OS_SEMAPHORE_DEBUG
        kernel_record_semaphore_enter();
#endif

        kernel_write_task_event(task_name, "acquired semaphore resource slot");
        toggle_led();

        /* Hold one simulated resource slot long enough to force contention. */
        os_sleep(KERNEL_SEMAPHORE_RESOURCE_HOLD_MS);

        kernel_write_task_event(task_name, "releasing semaphore resource slot");

#ifdef MELK_OS_SEMAPHORE_DEBUG
        kernel_record_semaphore_exit();
#endif

        semaphore_status = os_semaphore_post(&g_resource_semaphore);

#ifdef MELK_OS_SEMAPHORE_DEBUG
        if (semaphore_status == OS_SEMAPHORE_OK)
        {
            kernel_record_semaphore_post_success();
        }
        else
        {
            kernel_record_semaphore_error();
        }
#else
        (void)semaphore_status;
#endif

        os_sleep(rest_time_ms);
    }
}
#endif /* MELK_OS_SEMAPHORE_DEMO_ENABLED */
#if (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
static void kernel_record_message_queue_send_success(void)
{
    uint32_t irq_state;

    irq_state = os_irq_save();
    g_message_queue_send_success_count++;
    os_irq_restore(irq_state);
}

static void kernel_record_message_queue_receive_success(
        const kernel_queue_message_t *message)
{
    uint32_t irq_state;
    uint32_t expected_sequence;

    irq_state = os_irq_save();

    g_message_queue_receive_success_count++;

    if (message->producer_id == 1U)
    {
        expected_sequence = g_message_queue_producer_1_expected_sequence;

        if (message->sequence != expected_sequence)
        {
            g_message_queue_sequence_error_count++;
        }

        g_message_queue_producer_1_expected_sequence = message->sequence + 1U;
    }
    else if (message->producer_id == 2U)
    {
        expected_sequence = g_message_queue_producer_2_expected_sequence;

        if (message->sequence != expected_sequence)
        {
            g_message_queue_sequence_error_count++;
        }

        g_message_queue_producer_2_expected_sequence = message->sequence + 1U;
    }
    else
    {
        g_message_queue_sequence_error_count++;
    }

    os_irq_restore(irq_state);
}

static void kernel_record_message_queue_error(void)
{
    uint32_t irq_state;

    irq_state = os_irq_save();
    g_message_queue_error_count++;
    os_irq_restore(irq_state);
}
#endif /* MELK_OS_MESSAGE_QUEUE_DEBUG */

static void kernel_write_queue_message(const char *task_name,
                                       const char *event,
                                       const kernel_queue_message_t *message)
{
    int32_t mutex_status;

    mutex_status = os_mutex_lock(&g_console_mutex);

    if (mutex_status != OS_MUTEX_OK)
    {
#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
        kernel_record_message_queue_error();
#endif
        return;
    }

    kernel_print("[");
    kernel_print(task_name);
    kernel_print("] tick=");
    kernel_print_uint32(os_get_ticks());
    kernel_print(" ");
    kernel_print(event);
    kernel_print(" producer=");
    kernel_print_uint32(message->producer_id);
    kernel_print(" sequence=");
    kernel_print_uint32(message->sequence);
    kernel_print(" produced_tick=");
    kernel_print_uint32(message->production_tick);
    kernel_print("\n");

    mutex_status = os_mutex_unlock(&g_console_mutex);

#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
    if (mutex_status != OS_MUTEX_OK)
    {
        kernel_record_message_queue_error();
    }
#else
    (void)mutex_status;
#endif
}

static void kernel_run_queue_producer(const char *task_name,
                                      uint32_t producer_id,
                                      void (*toggle_led)(void),
                                      uint32_t period_ms)
{
    kernel_queue_message_t message;
    uint32_t sequence;
    int32_t queue_status;

    sequence = 0U;

    while (1)
    {
        message.producer_id = producer_id;
        message.sequence = sequence;
        message.production_tick = os_get_ticks();

        queue_status = os_message_queue_send(&g_message_queue, &message);

        if (queue_status == OS_MESSAGE_QUEUE_OK)
        {
#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
            kernel_record_message_queue_send_success();
#endif
            kernel_write_queue_message(task_name, "sent queue message", &message);
            toggle_led();
            sequence++;
        }
        else
        {
#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
            kernel_record_message_queue_error();
#endif
        }

        os_sleep(period_ms);
    }
}

static void kernel_run_queue_consumer(const char *task_name,
                                      void (*toggle_led)(void))
{
    kernel_queue_message_t message;
    int32_t queue_status;

    while (1)
    {
        queue_status = os_message_queue_receive(&g_message_queue, &message);

        if (queue_status == OS_MESSAGE_QUEUE_OK)
        {
#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
            kernel_record_message_queue_receive_success(&message);
#endif
            kernel_write_queue_message(task_name, "received queue message", &message);
            toggle_led();

#if (KERNEL_CONSUMER_PROCESSING_MS > 0U)
            os_sleep(KERNEL_CONSUMER_PROCESSING_MS);
#endif
        }
        else
        {
#ifdef MELK_OS_MESSAGE_QUEUE_DEBUG
            kernel_record_message_queue_error();
#endif
            os_sleep(100U);
        }
    }
}
#endif /* MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED */

#endif /* MELK_OS_SLEEP_DEBUG */

static void app_task_1(void *argument)
{
	(void)argument;

#ifdef MELK_OS_SLEEP_DEBUG
    while (1)
    {
        uint32_t start_tick;
        uint32_t end_tick;
        uint32_t elapsed_ticks;

        start_tick = os_get_ticks();
        os_sleep(500U);
        end_tick = os_get_ticks();
        elapsed_ticks = (uint32_t)(end_tick - start_tick);

        kernel_record_sleep_measurement(&g_task_1_sleep_stats,
                                        elapsed_ticks);
        gpio_toggle_green_led();
    }
#elif (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    kernel_run_queue_producer("TASK 1",
                              1U,
                              gpio_toggle_green_led,
                              KERNEL_PRODUCER_1_SLEEP_MS);
#elif (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    kernel_run_semaphore_worker("TASK 1", gpio_toggle_green_led, KERNEL_TASK_1_REST_MS);
#else
    while (1)
    {
        kernel_write_task_event("TASK 1", "protected UART output");
        gpio_toggle_green_led();
        os_sleep(500U);
    }
#endif
}

static void app_task_2(void *argument)
{
    (void)argument;

#ifdef MELK_OS_SLEEP_DEBUG
    while (1)
    {
        uint32_t start_tick;
        uint32_t end_tick;
        uint32_t elapsed_ticks;

        start_tick = os_get_ticks();
        os_sleep(1000U);
        end_tick = os_get_ticks();
        elapsed_ticks = (uint32_t)(end_tick - start_tick);

        kernel_record_sleep_measurement(&g_task_2_sleep_stats,
                                        elapsed_ticks);
        gpio_toggle_red_led();
    }
#elif (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    kernel_run_queue_producer("TASK 2",
                              2U,
                              gpio_toggle_red_led,
                              KERNEL_PRODUCER_2_SLEEP_MS);
#elif (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    kernel_run_semaphore_worker("TASK 2", gpio_toggle_red_led, KERNEL_TASK_2_REST_MS);
#else
    while (1)
    {
        kernel_write_task_event("TASK 2", "protected UART output");
        gpio_toggle_red_led();
        os_sleep(1000U);
    }
#endif
}

static void app_task_3(void *argument)
{
	(void)argument;

#ifdef MELK_OS_SLEEP_DEBUG
    while (1)
    {
        uint32_t start_tick;
        uint32_t end_tick;
        uint32_t elapsed_ticks;

        start_tick = os_get_ticks();
        os_sleep(2000U);
        end_tick = os_get_ticks();
        elapsed_ticks = (uint32_t)(end_tick - start_tick);

        kernel_record_sleep_measurement(&g_task_3_sleep_stats,
                                        elapsed_ticks);
        gpio_toggle_blue_led();
    }
#elif (MELK_OS_MESSAGE_QUEUE_DEMO_ENABLED != 0U)
    kernel_run_queue_consumer("TASK 3", gpio_toggle_blue_led);
#elif (MELK_OS_SEMAPHORE_DEMO_ENABLED != 0U)
    kernel_run_semaphore_worker("TASK 3", gpio_toggle_blue_led, KERNEL_TASK_3_REST_MS);
#else
    while (1)
    {
        kernel_write_task_event("TASK 3", "protected UART output");
        gpio_toggle_blue_led();
        os_sleep(2000U);
    }
#endif
}
