/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <hal.h>

void
SysTick_Handler(void)
{
    static int counter = 0;
    if ((++counter % 1000) == 0) {
        test_leds_toggle(TEST_LED1 | TEST_LED2 | TEST_LED3 | TEST_LED4);
        board_led_toggle();
    }
}


void
test(void)
{
    SysTick_Config(SystemCoreClock / 1000);
    while (true);
}
