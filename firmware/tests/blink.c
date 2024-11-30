/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <hal.h>
#include <test.h>

uint8_t test_count = 2;


void
SysTick_Handler(void)
{
    static int counter = 0;
    if ((++counter % 1000) == 0) {
        test_leds_toggle(TEST_LED1 | TEST_LED2 | TEST_LED3 | TEST_LED4);
        board_led_toggle();
    }
}


bool
test_configure(uint8_t test_id)
{
    (void) test_id;
    return true;
}


void
test_init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}
