# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0


add_library(flash INTERFACE)

target_sources(flash
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            flash.hh
            jedec.hh
)

target_include_directories(flash
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/flash>
)

target_link_libraries(flash
  INTERFACE magic_enum::magic_enum embeddedpp
)

