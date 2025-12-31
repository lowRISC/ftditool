# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 4.1)
include(FetchContent)

project(ftditool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/neargye/magic_enum.git
)
FetchContent_MakeAvailable(magic_enum)

add_subdirectory(lib/flash)
add_subdirectory(lib/ftdi)

add_executable(ftditool app/main.cc)
target_include_directories(ftditool PUBLIC "./src/lib" "./src/")
target_link_libraries(ftditool PRIVATE argparse flash ftdipp)

