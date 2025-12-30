# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

add_library(flash flash.cc)
target_include_directories(flash PUBLIC "../" "../../")
target_link_libraries(flash PRIVATE magic_enum)

