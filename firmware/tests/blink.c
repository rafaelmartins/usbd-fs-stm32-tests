/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include "../test.h"

void
SysTick_Handler(void)
{
    static int counter = 0;
    if ((++counter % 1000) == 0)
        leds_toggle(LED1 | LED2 | LED3 | LED4);
}


void
test(void)
{
    SysTick_Config(SystemCoreClock / 1000);
    while (true);
}
