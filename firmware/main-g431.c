/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This code runs on a NUCLEO-G431KB with the following solder bridge changes:
 *
 * - SB1 open
 * - SB9 closed
 * - SB10 closed
 * - SB12 open
 * - SB14 closed
 */

#include <stm32g4xx.h>
#include <test-helpers.h>

#define clock_frequency 170000000


void
clock_init(void)
{
    // 4 flash wait cycles required to operate @ 170MHz (RM0440 section 5.3.3 table 29)
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_4WS;  // RM0440 5.3.3
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS);

    // regulator boost mode required to operate @ 170MHz (RM0440 section 6.1.5 table 38)
    PWR->CR5 &= ~PWR_CR5_R1MODE;

    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY);

    RCC->CRRCR |= RCC_CRRCR_HSI48ON;
    while ((RCC->CRRCR & RCC_CRRCR_HSI48RDY) != RCC_CRRCR_HSI48RDY);

    // pll configuration (RM0440 section 7.4.4)
    //      f(PLL_R) = F(HSE/HSI) * PLLN / (PLLM * PLLR)
    //
    // HSE:
    //      24e6 * 85 / (2 * 6) = 170e6
    // HSI:
    //      16e6 * 85 / (2 * 4) = 170e6
    RCC->PLLCFGR = (85 << RCC_PLLCFGR_PLLN_Pos) |  // 0b00: PLLR = 2 (RM0440 section 7.4.4)
        RCC_PLLCFGR_PLLSRC_HSI | RCC_PLLCFGR_PLLM_1 | RCC_PLLCFGR_PLLM_0;  // 0b0011: PLLM = 4

    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY);

    // to switch to the high speed 170MHz clock, it is recommended to put
    // system clock into an intermediate frequency, for at least 1us.
    // run @ 85MHz by setting AHB prescaler to 2.
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL | RCC_CFGR_HPRE_DIV2;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // by iterating 85 times @ 170MHz and assuming that each iteration takes
    // at least two cycles (in practice it takes more), this loop will take at
    // least 1us
    for (__IO uint32_t i = 85; i; i--);

    // disable AHB prescaler.
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    SystemCoreClock = clock_frequency;
}


int
main(void)
{
    clock_init();

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;

    GPIOA->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0;

    GPIOB->MODER &= ~GPIO_MODER_MODE8;
    GPIOB->MODER |= GPIO_MODER_MODE8_0;
    GPIOB->BSRR = GPIO_BSRR_BS8;

    test();
    return 0;
}
