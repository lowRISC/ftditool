# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 4.1)

project(ftditool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

find_package(argparse QUIET)
find_package(magic_enum QUIET)
find_package(picosha2 QUIET)
find_package(elfio QUIET)
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

  FetchContent_Declare(
      picosha2
      GIT_REPOSITORY https://github.com/okdshin/PicoSHA2
      GIT_TAG v1.0.1
  )
  FetchContent_MakeAvailable(picosha2)

  FetchContent_Declare(
      elfio
      GIT_REPOSITORY https://github.com/serge1/ELFIO
      GIT_TAG Release_3.12
  )
  FetchContent_MakeAvailable(elfio)

else()
  # This is a hack becase picosha2 does not define an install phase in CMakefile.
  add_library(picosha2 INTERFACE)
  target_include_directories(picosha2 INTERFACE
      ${CMAKE_PREFIX_PATH}/include
  )
  # This is a hack becase elfio does not define an install phase in CMakefile.
  add_library(elfio INTERFACE)
  target_include_directories(elfio INTERFACE
      ${CMAKE_PREFIX_PATH}/elfio
  )
endif()

if(NOT DEFINED FTDITOOL_VERSION)
  set(FTDITOOL_VERSION "dev")
endif()


add_subdirectory(lib/embeddedpp)
add_subdirectory(lib/ftdi)
add_subdirectory(lib/flash)
add_subdirectory(lib/visuals)

add_executable(ftditool app/main.cc)
target_include_directories(ftditool PUBLIC "./src/lib" "./src/")
target_compile_definitions(ftditool PRIVATE FTDITOOL_VERSION="${FTDITOOL_VERSION}")
target_link_libraries(ftditool PRIVATE visuals picosha2 elfio argparse::argparse flash ftdipp)
install(TARGETS ftditool DESTINATION bin)
