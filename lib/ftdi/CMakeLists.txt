# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

add_library(ftdipp)

target_sources(ftdipp
    PRIVATE
        spi_host.cc
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            ftdi.hh
            spi_host.hh
            log.hh
)

target_include_directories(ftdipp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include/ftdipp
)

target_link_libraries(ftdipp
    PUBLIC magic_enum ft4222
)

