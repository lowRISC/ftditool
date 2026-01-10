# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 4.1)

project(ftditool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(argparse QUIET)
find_package(magic_enum QUIET)

# Check if nix is not providing these dependencies.
if (NOT argparse_FOUND)
  include(FetchContent)
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
endif()

add_subdirectory(lib/embeddedpp)
add_subdirectory(lib/ftdi)
add_subdirectory(lib/flash)

add_executable(ftditool app/main.cc)
target_include_directories(ftditool PUBLIC "./src/lib" "./src/")
target_link_libraries(ftditool PRIVATE argparse::argparse flash ftdipp)
install(TARGETS ftditool DESTINATION bin)

