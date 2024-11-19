// SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdbool.h>
#include <stlink.h>
#include "mcu.h"

bool run_tests_for_mcu(stlink_t *stl, mcu_id_t mcu);
