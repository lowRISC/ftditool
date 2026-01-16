// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <format>
#include <print>
#include <map>
#include <span>
#include <vector>
#include <memory>

#include "ftdi/ftdi.hh"
#include "ftdi/spi_host.hh"
#include "flash/flash.hh"
#include "commands.hh"
#include "ftdi/log.hh"

#include <argparse/argparse.hpp>

using Action = std::function<int()>;

static std::vector<ftdi::DeviceInfo> scan() {
  auto result = ftdi::Discovery::scan();
  if (!result) {
    std::cerr << "Error: Failed to communicate with FTDI driver." << std::endl;
    exit(0);
  }
  auto& devices = *result;
  if (devices.empty()) {
    std::println("No devices found.");
    exit(0);
  }
  return devices;
}

static std::unique_ptr<argparse::ArgumentParser>
new_flash_command(const std::string& name, const std::string& desc) {
  auto cmd = std::make_unique<argparse::ArgumentParser>(name);
  cmd->add_description(desc);
  cmd->add_argument("--interface", "-i").help("One of [spi, i2c, gpio]");
  cmd->add_argument("--ftdi")
      .default_value(std::string("FT4222"))
      .help("Filter ftdi chips connected to USB");
  return cmd;
}

static std::optional<ftdi::SpiHost>
handle_flash_command(std::unique_ptr<argparse::ArgumentParser>& cmd) {
  auto ftdi     = cmd->get<std::string>("--ftdi");
  auto devices  = scan();
  auto filtered = ftdi::DeviceInfo::filter_by_description(devices, ftdi);
  if (filtered.empty()) {
    std::print("No supported ftdi found.\n");
    exit(0);
  }
  if (auto opt = ftdi::SpiHost::from_device_info(filtered[0])) {
    return opt;
  }
  std::println("Can't open spi.");
  exit(0);
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

  auto jedec_cmd = new_flash_command("jedec", "Test the ftdi connection.");
  program.add_subparser(*jedec_cmd);
  commands["jedec"] = [&]() -> int {
    auto spih = handle_flash_command(jedec_cmd);
    commands::ReadJedec(flash::Generic(*spih)).run();
    return 0;
  };

  auto sfdp_cmd = new_flash_command("sfdp", "Test the ftdi connection.");
  program.add_subparser(*sfdp_cmd);
  commands["sfdp"] = [&]() -> int {
    auto spih = handle_flash_command(sfdp_cmd);
    commands::ReadSfdp(flash::Generic(*spih)).run();
    return 0;
  };

  auto read_page_cmd = new_flash_command("read-page", "Read a specific page");
  read_page_cmd->add_argument("--addr").help("The page address").scan<'x', std::size_t>();
  program.add_subparser(*read_page_cmd);
  commands["read-page"] = [&]() -> int {
    auto addr = read_page_cmd->get<std::size_t>("--addr");
    auto spih = handle_flash_command(read_page_cmd);
    commands::ReadPage(flash::Generic(*spih), addr).run();
    return 0;
  };

  auto test_page_cmd = new_flash_command("test-page", "Write a pattern to a page and read it back");
  test_page_cmd->add_argument("--addr")
      .help("The address to be loaded")
      .default_value(std::size_t{0})
      .scan<'x', std::size_t>();
  test_page_cmd->add_argument("--quad").help("Use qSPI").default_value(false).implicit_value(true);
  program.add_subparser(*test_page_cmd);
  commands["test-page"] = [&]() -> int {
    auto addr = test_page_cmd->get<std::size_t>("--addr");
    auto quad = test_page_cmd->get<bool>("--quad");
    auto spih = handle_flash_command(test_page_cmd);
    commands::TestPage(flash::Generic(*spih), addr, quad).run();

    return 0;
  };

  auto load_file_cmd =
      new_flash_command("load-file", "Write the content of a file to the address.");
  load_file_cmd->add_argument("filename").help("The file path.");
  load_file_cmd->add_argument("--addr")
      .help("The address to be loaded")
      .default_value(std::size_t{0})
      .scan<'x', std::size_t>();
  load_file_cmd->add_argument("--quad").help("Use qSPI").default_value(false).implicit_value(true);
  program.add_subparser(*load_file_cmd);
  commands["load-file"] = [&]() -> int {
    auto filename = load_file_cmd->get<std::string>("filename");
    auto addr     = load_file_cmd->get<std::size_t>("--addr");
    auto quad     = load_file_cmd->get<bool>("--quad");
    auto spih     = handle_flash_command(load_file_cmd);
    commands::LoadFile(flash::Generic(*spih), filename, addr, quad).run();

    return 0;
  };

  auto verify_file_cmd = new_flash_command(
      "verify-file", "Compare the hash if a file to hash of the flash content at the address.");
  verify_file_cmd->add_argument("filename").help("The file path.");
  verify_file_cmd->add_argument("--addr")
      .help("The address to be loaded")
      .default_value(std::size_t{0})
      .scan<'x', std::size_t>();
  verify_file_cmd->add_argument("--quad")
      .help("Use qSPI")
      .default_value(false)
      .implicit_value(true);
  program.add_subparser(*verify_file_cmd);
  commands["verify-file"] = [&]() -> int {
    auto filename = verify_file_cmd->get<std::string>("filename");
    auto addr     = verify_file_cmd->get<std::size_t>("--addr");
    auto quad     = verify_file_cmd->get<bool>("--quad");
    auto spih     = handle_flash_command(verify_file_cmd);
    commands::VerifyFile(flash::Generic(*spih), filename, addr, quad).run();

    return 0;
  };

  if (argc == 1) {
    std::cout << program;  // This prints the auto-generated help
    return 0;
  }

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
