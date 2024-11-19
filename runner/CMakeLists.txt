# SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
# SPDX-License-Identifier: BSD-3-Clause

find_package(PkgConfig)

pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)
pkg_check_modules(STLINK REQUIRED IMPORTED_TARGET stlink)

add_executable(runner
    main.c
    mcu.c
    runner.c
)

target_link_libraries(runner PRIVATE
    PkgConfig::LIBUSB
    PkgConfig::STLINK
)

target_compile_options(runner PRIVATE
    -Wall
    -Wextra
    -Werror
)
