# SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22)

include(FetchContent)

FetchContent_Declare(cmake_cmsis_stm32
    GIT_REPOSITORY https://github.com/rafaelmartins/cmake-cmsis-stm32.git
    GIT_TAG 7e5ec681b2d0c06bc529702681ff021e1df1d21a
)
FetchContent_MakeAvailable(
    cmake_cmsis_stm32
)

project(usbd-fs-stm32-test-firmware C ASM)

set(device_map
    f042_STM32F042x6
    g431_STM32G431xx
)

file(GLOB test_sources
    LIST_DIRECTORIES false
    tests/*.c
)

foreach(test_source ${test_sources})
    get_filename_component(test ${test_source} NAME_WE)

    foreach(device_string ${device_map})
        string(FIND ${device_string} _ device_divider)
        string(SUBSTRING ${device_string} 0 ${device_divider} device_family)
        math(EXPR device_divider "${device_divider}+1")
        string(SUBSTRING ${device_string} ${device_divider} -1 device_def)

        add_executable(test-${test}-${device_family}
            main-${device_family}.c
            ${test_source}
        )

        target_compile_options(test-${test}-${device_family} PRIVATE
            -Wall
            -Wextra
            -Werror
        )

        set_property(TARGET test-${test}-${device_family}
            PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE
        )

        cmsis_stm32_target(test-${test}-${device_family}
            DEVICE ${device_def}
            VERSION 20240709193138
            LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${device_def}.ld
            ADDITIONAL_OUTPUTS IHEX
            SHOW_SIZE ON
            STLINK ON
            #INSTALL ON
        )
    endforeach()
endforeach()
