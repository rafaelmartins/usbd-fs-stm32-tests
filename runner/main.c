// SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: BSD-3-Clause

#include <stdint.h>
#include <stdio.h>
#include <stlink.h>
#include "runner.h"


int
main(int argc, char **argv)
{
    stlink_t **devices;
    uint8_t failures = 0;
    uint8_t total = 0;
    uint32_t n = stlink_probe_usb(&devices, CONNECT_NORMAL, 24000);

    for (uint8_t i; i < n; i++) {
        mcu_id_t mcu = mcu_get_from_chip_id(devices[i]->chip_id);
        if (mcu >= __MCU_COUNT)
            continue;
        total++;

        fprintf(stderr, "========================================\n");
        fprintf(stderr, "Running tests for %s\n", mcu_get_name(mcu));
        fprintf(stderr, "----------------------------------------\n");
        bool rv = run_tests_for_mcu(devices[i], mcu);
        fprintf(stderr, "----------------------------------------\n");
        fprintf(stderr, "%s (%s)\n", rv ? "PASSED" : "FAILED", mcu_get_name(mcu));
        if (!rv)
            failures++;
    }

    fprintf(stderr, "========================================\n");
    fprintf(stderr, "%s (%d/%d)\n", failures > 0 ? "FAILED" : "PASSED",
        failures > 0 ? failures : total, total);

    stlink_probe_usb_free(&devices, n);
    return (failures > 0) ? 1 : 0;
}
