/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <hal.h>


int
main(void)
{
    main_clock_init();

    RCC->GPIO_RCC |= GPIO_RCC_EN(GPIO_RCC, BOARD_LED_BLK) | GPIO_RCC_EN(GPIO_RCC, TEST_LEDS_BLK);

    BOARD_LED_REG->MODER &= ~(GPIO_MODER_MASK(BOARD_LED_IDX));
    BOARD_LED_REG->MODER |= GPIO_MODER_OUTPUT(BOARD_LED_IDX);
    board_led_reset();

    TEST_LEDS_REG->MODER &= ~(GPIO_MODER_MASK(TEST_LED1_IDX) | GPIO_MODER_MASK(TEST_LED2_IDX) |
                              GPIO_MODER_MASK(TEST_LED3_IDX) | GPIO_MODER_MASK(TEST_LED4_IDX));
    TEST_LEDS_REG->MODER |= GPIO_MODER_OUTPUT(TEST_LED1_IDX) | GPIO_MODER_OUTPUT(TEST_LED2_IDX) |
                            GPIO_MODER_OUTPUT(TEST_LED3_IDX) | GPIO_MODER_OUTPUT(TEST_LED4_IDX);
    test_leds_reset(TEST_LED1 | TEST_LED2 | TEST_LED3 | TEST_LED4);

    test();
    return 0;
}
