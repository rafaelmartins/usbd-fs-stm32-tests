#pragma once

#if defined(STM32F042x6)
#include <stm32f0xx.h>
#define LED_REGISTER GPIOA
#define LED1 GPIO_BSRR_BS_0
#define LED2 GPIO_BSRR_BS_1
#define LED3 GPIO_BSRR_BS_2
#define LED4 GPIO_BSRR_BS_3
#elif defined(STM32G431xx)
#include <stm32g4xx.h>
#define LED_REGISTER GPIOA
#define LED1 GPIO_BSRR_BS0
#define LED2 GPIO_BSRR_BS1
#define LED3 GPIO_BSRR_BS2
#define LED4 GPIO_BSRR_BS3
#else
#error "Unsupported microcontroller."
#endif

#define leds_set(l)    (LED_REGISTER->BSRR = l)
#define leds_reset(l)  (LED_REGISTER->BRR  = l)
#define leds_toggle(l) (LED_REGISTER->BSRR = ((LED_REGISTER->ODR & (l)) << 16) | (~(LED_REGISTER->ODR) & (l)))

void test(void);
