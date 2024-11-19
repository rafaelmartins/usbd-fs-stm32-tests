// SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdint.h>

typedef enum {
    MCU_STM32F0,
    MCU_STM32G4,
    __MCU_COUNT,
} mcu_id_t;

uint32_t mcu_get_chip_id(mcu_id_t mcu);
const char* mcu_get_name(mcu_id_t mcu);
mcu_id_t mcu_get_from_chip_id(uint32_t chip_id);
