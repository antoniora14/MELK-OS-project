#include <stdint.h>
#include "uart.h"
#include "system.h"

/*
 * UART0:
 * PA0 -> U0RX
 * PA1 -> U0TX
 *
 * Baudrate: 115200
 * Clock source: System clock
 */

/* System Control base registers */
#define SYSCTL_RCGCUART_R      (*((volatile uint32_t *)0x400FE618))
#define SYSCTL_RCGCGPIO_R      (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRUART_R        (*((volatile uint32_t *)0x400FEA18))
#define SYSCTL_PRGPIO_R        (*((volatile uint32_t *)0x400FEA08))

#define SYSCTL_UART0_CLOCK     (1U << 0)
#define SYSCTL_GPIOA_CLOCK     (1U << 0)

#define UART0_BAUD_RATE        115200U

/* UART0 base */
#define UART0_BASE             0x4000C000

#define UART0_DR_R             (*((volatile uint32_t *)(UART0_BASE + 0x000)))
#define UART0_FR_R             (*((volatile uint32_t *)(UART0_BASE + 0x018)))
#define UART0_IBRD_R           (*((volatile uint32_t *)(UART0_BASE + 0x024)))
#define UART0_FBRD_R           (*((volatile uint32_t *)(UART0_BASE + 0x028)))
#define UART0_LCRH_R           (*((volatile uint32_t *)(UART0_BASE + 0x02C)))
#define UART0_CTL_R            (*((volatile uint32_t *)(UART0_BASE + 0x030)))
#define UART0_CC_R             (*((volatile uint32_t *)(UART0_BASE + 0xFC8)))

/* GPIO Port A base */
#define GPIO_PORTA_BASE        0x40004000

#define GPIO_PORTA_AFSEL_R     (*((volatile uint32_t *)(GPIO_PORTA_BASE + 0x420)))
#define GPIO_PORTA_DEN_R       (*((volatile uint32_t *)(GPIO_PORTA_BASE + 0x51C)))
#define GPIO_PORTA_AMSEL_R     (*((volatile uint32_t *)(GPIO_PORTA_BASE + 0x528)))
#define GPIO_PORTA_PCTL_R      (*((volatile uint32_t *)(GPIO_PORTA_BASE + 0x52C)))

#define GPIO_PA0               (1U << 0)
#define GPIO_PA1               (1U << 1)

/* UART flags */
#define UART_FR_TXFF           (1U << 5)

/* UART control bits */
#define UART_CTL_UARTEN        (1U << 0)
#define UART_CTL_TXE           (1U << 8)
#define UART_CTL_RXE           (1U << 9)

/* UART line control */
#define UART_LCRH_FEN          (1U << 4)
#define UART_LCRH_WLEN_8       (0x3U << 5)



void uart0_init(void)
{
    uint32_t system_clock_hz;
    uint32_t baud_divisor;
    uint32_t integer_divisor;
    uint32_t remainder;
    uint32_t fractional_divisor;

    /*
     * 1. Enable UART0 clock.
     */
    SYSCTL_RCGCUART_R |= SYSCTL_UART0_CLOCK;

    /*
     * 2. Enable GPIO Port A clock.
     */
    SYSCTL_RCGCGPIO_R |= SYSCTL_GPIOA_CLOCK;

    /*
     * 3. Wait until UART0 and GPIOA are ready.
     * This is safer than using a dummy delay.
     */
    while ((SYSCTL_PRUART_R & SYSCTL_UART0_CLOCK) == 0)
    {
    }

    while ((SYSCTL_PRGPIO_R & SYSCTL_GPIOA_CLOCK) == 0)
    {
    }

    /*
     * 4. Disable UART0 before configuration.
     */
    UART0_CTL_R &= ~UART_CTL_UARTEN;

    /*
     * 5. Select system clock as UART clock source.
     */
    UART0_CC_R = 0x0;

    /*
     * 6. Configure baudrate from the active system clock.
     */
    system_clock_hz = system_get_clock_hz();
    baud_divisor = 16U * UART0_BAUD_RATE;
    integer_divisor = system_clock_hz / baud_divisor;
    remainder = system_clock_hz % baud_divisor;
    fractional_divisor = ((remainder * 64U) + (baud_divisor / 2U)) /
                         baud_divisor;

    if (fractional_divisor == 64U)
    {
        integer_divisor++;
        fractional_divisor = 0U;
    }

    UART0_IBRD_R = integer_divisor;
    UART0_FBRD_R = fractional_divisor;

    /*
     * 7. 8-bit, no parity, 1 stop bit, FIFO enabled.
     */
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

    /*
     * 8. Configure PA0 and PA1 for UART alternate function.
     */
    GPIO_PORTA_AFSEL_R |= (GPIO_PA0 | GPIO_PA1);

    /*
     * PA0 and PA1 use alternate function 1 for UART0.
     */
    GPIO_PORTA_PCTL_R &= ~0x000000FF;
    GPIO_PORTA_PCTL_R |=  0x00000011;

    /*
     * Disable analog mode on PA0 and PA1.
     */
    GPIO_PORTA_AMSEL_R &= ~(GPIO_PA0 | GPIO_PA1);

    /*
     * Enable digital function on PA0 and PA1.
     */
    GPIO_PORTA_DEN_R |= (GPIO_PA0 | GPIO_PA1);

    /*
     * 9. Enable UART0, TX and RX.
     */
    UART0_CTL_R = UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
}

void uart0_write_char(char c)
{
    while ((UART0_FR_R & UART_FR_TXFF) != 0)
    {}
    UART0_DR_R = (uint32_t)c;
}

void uart0_write_string(const char *str)
{
    while (*str != '\0')
    {
        if (*str == '\n')
        {
            uart0_write_char('\r');
        }

        uart0_write_char(*str);
        str++;
    }
}
