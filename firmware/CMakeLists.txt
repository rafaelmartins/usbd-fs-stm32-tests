# SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22)

include(FetchContent)

FetchContent_Declare(cmake_cmsis_stm32
    GIT_REPOSITORY https://github.com/rafaelmartins/cmake-cmsis-stm32.git
    GIT_TAG c199670047e5adb373d8a033ece58c34f173ad4b
)
FetchContent_MakeAvailable(
    cmake_cmsis_stm32
)

project(usbd-fs-stm32-test-firmware C ASM)

set(USBD_FS_STM32_GIT_REF b5fbdf367c8883986c37869c71fc38a9ab4adbf0 CACHE STRING "usbd-fs-stm32 Git reference to test against.")

FetchContent_Declare(usbd_fs_stm32
    GIT_REPOSITORY https://github.com/rafaelmartins/usbd-fs-stm32.git
    GIT_TAG ${USBD_FS_STM32_GIT_REF}
)
FetchContent_MakeAvailable(
    usbd_fs_stm32
)

set(device_map
    STM32F042x6
    STM32G431xx
)

file(GLOB test_sources
    LIST_DIRECTORIES false
    tests/*.c
)

foreach(test_source ${test_sources})
    get_filename_component(test ${test_source} NAME_WE)

    foreach(device_definition ${device_map})
        string(SUBSTRING ${device_definition} 5 4 device_family)
        string(TOLOWER ${device_family} device_family)

        add_executable(test-${test}-${device_family}
            main.c
            ${test_source}
        )

        if(EXISTS "${test_source}make")
            block()
                include("${test_source}make")
                if(DEFINED compile_definitions)
                    target_compile_definitions(usbd-fs-stm32 INTERFACE
                        ${compile_definitions}
                    )
                endif()
            endblock()
        endif()

        target_include_directories(test-${test}-${device_family} PRIVATE
            include
            ../common
        )

        target_link_libraries(test-${test}-${device_family} PRIVATE
            usbd-fs-stm32
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
            DEVICE ${device_definition}
            VERSION 20241122015203
            LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${device_definition}.ld
            SHOW_SIZE ON
            STLINK ON
        )
    endforeach()
endforeach()
