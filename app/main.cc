// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <format>
#include <print>
#include <map>
#include <vector>

#include "ftdi/ftdi.hh"
#include "flash/flash.hh"
#include "log.hh"

#include <argparse/argparse.hpp>

using Action = std::function<int()>;

static std::span<ftdi::DeviceInfo> scan() {
  auto result = ftdi::Discovery::scan();
  if (!result) {
    std::cerr << "Error: Failed to communicate with FTDI driver." << std::endl;
    exit(0);
  }
  auto& devices = *result;
  if (devices.empty()) {
    std::print("No devices found.\n");
    exit(0);
  }
  return devices;
}

int main(int argc, char* argv[]) {
  std::map<std::string, Action> commands;

  argparse::ArgumentParser program("ftditool");

  argparse::ArgumentParser list_devices_cmd("list-devices");
  list_devices_cmd.add_description("List FTDI devices.");
  program.add_subparser(list_devices_cmd);
  commands["list-devices"] = [&]() -> int {
    auto idx = 0;
    for (auto device : scan()) {
      std::print("{}: {}\n", idx++, device);
    }
    return 0;
  };

  argparse::ArgumentParser jedec_cmd("jedec");
  jedec_cmd.add_description("Test the ftdi connection.");
  jedec_cmd.add_argument("--interface", "-i").help("One of [spi, i2c, gpio]");
  jedec_cmd.add_argument("--ftdi").default_value("FT4222").help(
      "Filter ftdi chips connected to USB");
  program.add_subparser(jedec_cmd);
  commands["jedec"] = [&]() -> int {
    auto ftdi     = jedec_cmd.get<std::string>("--ftdi");
    auto filtered = ftdi::DeviceInfo::filter_by_description(scan(), ftdi);
    if (filtered.empty()) {
      std::print("No supported ftdi found.\n");
      return 0;
    }
    if (auto spih = ftdi::SpiHost::from_device_info(filtered[0])) {
      auto flash = flash::Generic(*spih);
      if (auto jedec = flash.jedec()) {
        std::print("{}\n", *jedec);
      } else {
        std::print("Jedec failed\n");
      }
    }
    return 0;
  };

  argparse::ArgumentParser read_page_cmd("read-page");
  read_page_cmd.add_description("Test the ftdi connection.");
  read_page_cmd.add_argument("--interface", "-i").help("One of [spi, i2c, gpio]");
  read_page_cmd.add_argument("--ftdi").default_value("FT4222").help(
      "Filter ftdi chips connected to USB");
  read_page_cmd.add_argument("--addr").help("The page address").scan<'x', std::size_t>();
  program.add_subparser(read_page_cmd);
  commands["read-page"] = [&]() -> int {
    auto ftdi     = read_page_cmd.get<std::string>("--ftdi");
    auto addr     = read_page_cmd.get<std::size_t>("--addr");
    auto filtered = ftdi::DeviceInfo::filter_by_description(scan(), ftdi);
    if (filtered.empty()) {
      std::print("No supported ftdi found.\n");
      return 0;
    }
    if (auto spih = ftdi::SpiHost::from_device_info(filtered[0])) {
      auto flash = flash::Generic(*spih);

      if (auto page = flash.single_read_page(addr)) {
        std::print("{:#x} : {}\n", addr, *page);
      } else {
        std::print("page failed\n");
      }
    }
    return 0;
  };

  argparse::ArgumentParser write_page_cmd("write-page");
  write_page_cmd.add_description("Test the ftdi connection.");
  write_page_cmd.add_argument("--interface", "-i").help("One of [spi, i2c, gpio]");
  write_page_cmd.add_argument("--ftdi").default_value("FT4222").help(
      "Filter ftdi chips connected to USB");
  write_page_cmd.add_argument("--addr").help("The page address").scan<'x', std::size_t>();
  program.add_subparser(write_page_cmd);
  commands["write-page"] = [&]() -> int {
    auto ftdi     = write_page_cmd.get<std::string>("--ftdi");
    auto addr     = write_page_cmd.get<std::size_t>("--addr");
    auto filtered = ftdi::DeviceInfo::filter_by_description(scan(), ftdi);
    if (filtered.empty()) {
      std::print("No supported ftdi found.\n");
      return 0;
    }
    if (auto spih = ftdi::SpiHost::from_device_info(filtered[0])) {
      auto flash = flash::Generic(*spih);

      if (!flash.reset() || !flash.erase(addr)) {
        std::print("Erase failed\n");
        return 0;
      }
      std::vector<uint8_t> data(256, 0xaa);
      if (!flash.single_page_program(addr, std::span<uint8_t, 256>(data))) {
        std::print("Program failed\n");
        return 0;
      }

      if (auto page = flash.single_read_page(addr)) {
        std::print("{:#x} : {}\n", addr, *page);
      } else {
        std::print("page failed\n");
      }
    }
    return 0;
  };

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  for (const auto& [cmd, action] : commands) {
    if (program.is_subcommand_used(cmd)) {
      return action();
    }
  }

  return 0;
}
