// SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdint.h>
#include "mcu.h"

static const uint32_t mcu_chip_ids[] = {
    [MCU_STM32F0] = 0x0445,
    [MCU_STM32G4] = 0x0468,
};
static const char* mcu_names[] = {
    [MCU_STM32F0] = "NUCLEO-F042K6",
    [MCU_STM32G4] = "NUCLEO-G431KB",
};


uint32_t
mcu_get_chip_id(mcu_id_t mcu)
{
    return (mcu < __MCU_COUNT) ? mcu_chip_ids[mcu] : 0;
}


const char*
mcu_get_name(mcu_id_t mcu)
{
    return (mcu < __MCU_COUNT) ? mcu_names[mcu] : "";
}


mcu_id_t
mcu_get_from_chip_id(uint32_t chip_id)
{
    for (uint8_t i = 0; i < __MCU_COUNT; i++)
        if (chip_id == mcu_chip_ids[i])
            return i;
    return __MCU_COUNT;
}
