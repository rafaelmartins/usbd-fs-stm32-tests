/*
 * SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

bool test_configure(uint8_t test_id);
void test_fail(void);

void test_init(void) __WEAK;
void test_loop(void) __WEAK;
