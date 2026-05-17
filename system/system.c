/*
 * system.c
 *
 *  Created on: 13 may. 2026
 *      Author: anton
 */

#include "system.h"

#define SYSCTL_RIS_R           (*((volatile uint32_t *)0x400FE050))
#define SYSCTL_RCC_R           (*((volatile uint32_t *)0x400FE060))
#define SYSCTL_RCC2_R          (*((volatile uint32_t *)0x400FE070))

#define SYSCTL_RIS_PLLLRIS     (1U << 6)

#define SYSCTL_RCC_MOSCDIS     (1U << 0)
#define SYSCTL_RCC_XTAL_M      0x000007C0U
#define SYSCTL_RCC_XTAL_16MHZ  0x00000540U

#define SYSCTL_RCC2_USERCC2    (1U << 31)
#define SYSCTL_RCC2_DIV400     (1U << 30)
#define SYSCTL_RCC2_SYSDIV2_M  0x1FC00000U
#define SYSCTL_RCC2_PWRDN2     (1U << 13)
#define SYSCTL_RCC2_BYPASS2    (1U << 11)
#define SYSCTL_RCC2_OSCSRC2_M  0x00000070U

#if (SYSTEM_CLOCK_HZ > 80000000U)
#error "SYSTEM_CLOCK_HZ must be 80000000 Hz or lower for TM4C123GH6PM."
#endif

#if (SYSTEM_CLOCK_HZ < 3125000U)
#error "SYSTEM_CLOCK_HZ must be 3125000 Hz or higher when using the 400 MHz PLL."
#endif

#if ((SYSTEM_PLL_VCO_HZ % SYSTEM_CLOCK_HZ) != 0U)
#error "SYSTEM_CLOCK_HZ must divide 400000000 Hz exactly."
#endif

#define SYSTEM_PLL_DIVIDER     (SYSTEM_PLL_VCO_HZ / SYSTEM_CLOCK_HZ)
#define SYSTEM_SYSDIV2_VALUE   (SYSTEM_PLL_DIVIDER - 1U)

#if (SYSTEM_PLL_DIVIDER < 5U)
#error "SYSTEM_CLOCK_HZ is too high for the TM4C123 PLL divider."
#endif

#if (SYSTEM_PLL_DIVIDER > 128U)
#error "SYSTEM_CLOCK_HZ is too low for the TM4C123 PLL divider."
#endif

void SystemInit(void)
{
    SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
    SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;

    SYSCTL_RCC_R &= ~SYSCTL_RCC_MOSCDIS;
    SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;
    SYSCTL_RCC_R |= SYSCTL_RCC_XTAL_16MHZ;

    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
    SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;
    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_SYSDIV2_M;
    SYSCTL_RCC2_R |= (SYSTEM_SYSDIV2_VALUE << 22);

    while ((SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) == 0U)
    {
    }

    SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}

uint32_t system_get_clock_hz(void)
{
    return SYSTEM_CLOCK_HZ;
}
