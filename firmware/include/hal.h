/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#if defined(STM32F042x6)
#include <stm32f0xx.h>

#define GPIO_RCC AHBENR

#define _GPIO_BSRR_BS(idx)      GPIO_BSRR_BS_ ## idx
#define _GPIO_BSRR_BR(idx)      GPIO_BSRR_BR_ ## idx
#define _GPIO_MODER_MASK(idx)   GPIO_MODER_MODER ## idx
#define _GPIO_MODER_OUTPUT(idx) GPIO_MODER_MODER ## idx ## _0

#define BOARD_LED_BLK B
#define BOARD_LED_IDX 3

#define TEST_LEDS_BLK A
#define TEST_LED1_IDX 1
#define TEST_LED2_IDX 4
#define TEST_LED3_IDX 7
#define TEST_LED4_IDX 8

static inline void
main_clock_init(void)
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

    SystemCoreClock = 48000000;
}

#elif defined(STM32G431xx)
#include <stm32g4xx.h>

#define GPIO_RCC AHB2ENR

#define _GPIO_BSRR_BS(idx)      GPIO_BSRR_BS ## idx
#define _GPIO_BSRR_BR(idx)      GPIO_BSRR_BR ## idx
#define _GPIO_MODER_MASK(idx)   GPIO_MODER_MODE ## idx
#define _GPIO_MODER_OUTPUT(idx) GPIO_MODER_MODE ## idx ## _0

#define BOARD_LED_BLK B
#define BOARD_LED_IDX 8

#define TEST_LEDS_BLK A
#define TEST_LED1_IDX 1
#define TEST_LED2_IDX 4
#define TEST_LED3_IDX 7
#define TEST_LED4_IDX 8

static inline void
main_clock_init(void)
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

    SystemCoreClock = 170000000;
}

#else
#error "Unsupported microcontroller."
#endif

#define _GPIO_RCC_EN(reg, block) RCC_ ## reg ## _GPIO ## block ## EN
#define GPIO_RCC_EN(reg, block)  _GPIO_RCC_EN(reg, block)
#define _GPIO_REG(block)         GPIO ## block
#define GPIO_REG(block)          _GPIO_REG(block)
#define GPIO_BSRR_BS(idx)        _GPIO_BSRR_BS(idx)
#define GPIO_BSRR_BR(idx)        _GPIO_BSRR_BR(idx)
#define GPIO_MODER_MASK(idx)     _GPIO_MODER_MASK(idx)
#define GPIO_MODER_OUTPUT(idx)   _GPIO_MODER_OUTPUT(idx)

#define BOARD_LED_REG GPIO_REG(BOARD_LED_BLK)
#define TEST_LEDS_REG GPIO_REG(TEST_LEDS_BLK)

#define _TEST_LED(idx) _GPIO_BSRR_BS(idx)
#define TEST_LED1      _TEST_LED(TEST_LED1_IDX)
#define TEST_LED2      _TEST_LED(TEST_LED2_IDX)
#define TEST_LED3      _TEST_LED(TEST_LED3_IDX)
#define TEST_LED4      _TEST_LED(TEST_LED4_IDX)

#define board_led_set()     { BOARD_LED_REG->BSRR = GPIO_BSRR_BS(BOARD_LED_IDX); }
#define board_led_reset()   { BOARD_LED_REG->BSRR = GPIO_BSRR_BR(BOARD_LED_IDX); }
#define board_led_toggle()  { BOARD_LED_REG->BSRR = ((((BOARD_LED_REG->ODR) << 16) & (GPIO_BSRR_BR(BOARD_LED_IDX))) | \
                                                     (~(BOARD_LED_REG->ODR)        & (GPIO_BSRR_BS(BOARD_LED_IDX)))); }

#define test_leds_set(l)    { TEST_LEDS_REG->BSRR = (l); }
#define test_leds_reset(l)  { TEST_LEDS_REG->BRR  = (l); }
#define test_leds_toggle(l) { TEST_LEDS_REG->BSRR = (((TEST_LEDS_REG->ODR  & (l)) << 16) | \
                                                    (~(TEST_LEDS_REG->ODR) & (l))); }

void test(void);
