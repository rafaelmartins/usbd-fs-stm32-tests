/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stm32f0xx.h>
#include <test-helpers.h>

#define clock_frequency 48000000


void
clock_init(void)
{
    // 1 flash wait cycle required to operate @ 48MHz (RM0091 section 3.5.1)
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY;
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY);

    RCC->CR2 |= RCC_CR2_HSI48ON;
    while ((RCC->CR2 & RCC_CR2_HSI48RDY) != RCC_CR2_HSI48RDY);

    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE | RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE_DIV1 | RCC_CFGR_SW_HSI48;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI48);

    SystemCoreClock = clock_frequency;
}


int
main(void)
{
    clock_init();

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

    GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER4 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8);
    GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0;

    GPIOB->MODER &= ~GPIO_MODER_MODER3;
    GPIOB->MODER |= GPIO_MODER_MODER3_0;
    GPIOB->BSRR = GPIO_BSRR_BS_3;

    test();
    return 0;
}
