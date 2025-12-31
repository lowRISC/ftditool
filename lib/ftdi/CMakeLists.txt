# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

add_library(ftdi spi_host.cc)
target_include_directories(ftdi PUBLIC "../../app")
target_link_libraries(ftdi PRIVATE magic_enum)

