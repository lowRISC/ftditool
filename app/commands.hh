// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "flash/flash.hh"
#include "embeddedpp/gpio.hh"
#include "visuals/progressbar.hh"
#include "visuals/throughput.hh"
#include <fstream>
#include <iostream>
#include <picosha2.h>
#include <print>

namespace commands {

template <typename T>
struct Commands {
  flash::Generic<T> flash;

  Commands(flash::Generic<T> f) : flash(f) {}

  virtual ~Commands() = default;
  virtual int run()   = 0;
};

template <typename T>
struct ReadJedec : public Commands<T> {
  ReadJedec(flash::Generic<T> f) : Commands<T>(f) {}

  int run() override {
    if (auto jedec = this->flash.jedec()) {
      std::println("{}", *jedec);
      return 0;
    } else {
      std::println("Jedec failed");
      return 1;
    }
  }
};

template <typename T>
struct ReadSfdp : public Commands<T> {
  ReadSfdp(flash::Generic<T> f) : Commands<T>(f) {}

  int run() override {
    if (auto sfdp = this->flash.sfdp()) {
      std::println("{}", *sfdp);
      std::println("quad-mode mechanism: {}", sfdp->get_quad_enable_mechanism());
      return 0;
    }
    std::println("Sfdp read failed");
    return 1;
  }
};

template <typename T>
struct ReadPage : public Commands<T> {
  std::size_t addr;
  ReadPage(flash::Generic<T> f, std::size_t addr = 0) : Commands<T>(f), addr(addr) {}

  int run() override {
    if (auto page = this->flash.single_read_page(addr)) {
      std::println("Single read {:#x} : {}", addr, *page);
    } else {
      std::println("page failed");
    }

    return 1;
  }
};

template <typename T>
struct TestPage : public Commands<T> {
  std::size_t addr;
  bool quad;
  TestPage(flash::Generic<T> f, std::size_t addr = 0, bool quad = false)
      : Commands<T>(f), addr(addr), quad(quad) {}

  int run() override {
    if (!this->flash.reset() || !this->flash.erase(addr)) {
      std::println("Erase failed");
      return 0;
    }

    if (quad && !this->flash.enable_quad(true)) {
      std::println("enable quad failed");
    }

    std::vector<uint8_t> data(256, 0x5a);
    if (quad && !this->flash.quad_page_program(addr, std::span<uint8_t, 256>(data))) {
      std::println("Program failed");
      return 0;
    } else if (!quad && !this->flash.single_page_program(addr, std::span<uint8_t, 256>(data))) {
      std::println("Program failed");
      return 0;
    }

    std::optional<flash::Page> page;
    if (quad) {
      page = this->flash.template quad_read_page(addr);
    } else {
      page = this->flash.single_read_page(addr);
    }

    if (page) {
      std::println("page read {:#x} : {}", addr, *page);
      return 0;
    }
    std::println("page failed");
    return 1;
  }
};

template <typename T>
struct VerifyFile : public Commands<T> {
  std::size_t addr;
  std::string& filename;
  bool quad;
  VerifyFile(flash::Generic<T> f, std::string& filename, std::size_t addr = 0, bool quad = false)
      : Commands<T>(f), filename(filename), addr(addr), quad(quad) {}

  int run() override {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      std::println("Could not open the file {}!", filename);
      return 0;
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (file_size == 0) {
      std::println("File is empty");
      return 0;
    }

    std::vector<uint8_t> file_hash(picosha2::k_digest_size, 0xdd);
    picosha2::hash256(file, file_hash.begin(), file_hash.end());

    this->flash.reset();
    if (quad && !this->flash.enable_quad(true)) {
      std::println("enable quad failed");
      return 0;
    }

    picosha2::hash256_one_by_one flash_hasher;
    auto progress_bar = ProgressBar(file_size, 50, "Verifying").with_throughput();
    size_t remainder  = file_size;

    while (remainder > 0) {
      std::optional<flash::Page> page;
      if (quad) {
        page = this->flash.template quad_read_page(addr);
      } else {
        page = this->flash.single_read_page(addr);
      }

      if (!page) {
        std::println("Program page {:#x} failed.", addr);
        return 0;
      }

      uint32_t chunck = std::min(remainder, std::size_t{flash::PageSize});
      std::span<uint8_t> data(*page);
      if (chunck < data.size()) {
        data = data.subspan(0, chunck);
      }
      flash_hasher.process(data.begin(), data.end());

      addr += chunck;
      remainder -= chunck;
      progress_bar.update(file_size - remainder);
    }

    flash_hasher.finish();
    std::vector<uint8_t> flash_hash(picosha2::k_digest_size, 0xee);
    flash_hasher.get_hash_bytes(flash_hash.begin(), flash_hash.end());
    if (file_hash != flash_hash) {
      std::println("Expected: {}\nbut got:    {}", file_hash, flash_hash);
    }

    return 1;
  }
};

template <typename T>
struct LoadFile : public Commands<T> {
  std::size_t start_addr;
  std::string& filename;
  bool quad;
  bool bootstrap;
  LoadFile(flash::Generic<T> f, std::string& filename, std::size_t addr = 0, bool bootstrap = false,
           bool quad = false)
      : Commands<T>(f), filename(filename), start_addr(addr), quad(quad), bootstrap(bootstrap) {}

  int run() override {
    if ((this->start_addr % flash::SectorSize) != 0) {
      // TODO: Support Unaligned addresses.
      std::println("Only {} aligned addresses are supported!", size_t{flash::SectorSize});
      return 0;
    }

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      std::println("Could not open the file {}!", filename);
      return 0;
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (file_size == 0) {
      std::println("File is empty");
      return 0;
    }
    auto size = file_size + (file_size - file_size % flash::PageSize);
    std::vector<uint8_t> buffer(size, 0xff);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
      std::println("Error reading the file.");
      return 0;
    }

    if (!bootstrap) {
      this->flash.reset();
    }
    if (quad && !this->flash.enable_quad(true)) {
      std::println("enable quad failed");
      return 0;
    }

    std::span<uint8_t> data(buffer);
    auto progress_bar = ProgressBar(buffer.size(), 50, "Loading").with_throughput();
    size_t addr       = start_addr;
    while (data.size() > 0) {
      if ((addr % flash::SectorSize) == 0 && !this->flash.erase(addr)) {
        std::println("Failed to erase block {:#x}", addr);
        return 0;
      }

      std::optional<bool> res;
      if (quad) {
        res = this->flash.quad_page_program(addr, data.first<flash::PageSize>());
      } else {
        res = this->flash.single_page_program(addr, data.first<flash::PageSize>());
      }

      if (!res) {
        std::println("Program page {:#x} failed.", addr);
        return 0;
      }

      addr += flash::PageSize;
      data = data.subspan(std::min(data.size(), std::size_t{flash::PageSize}));
      progress_bar.update(buffer.size() - data.size());
    }

    if (bootstrap) {
      this->flash.reset();
      return 1;
    } else {
      return commands::VerifyFile(this->flash, filename, start_addr, quad).run();
    }
  }
};

template <typename T>
  requires embeddedpp::Gpio<T>
struct GpioWrite {
  T& gpio;
  uint8_t pin;
  bool value;

  GpioWrite(T& gpio, uint8_t pin, bool value) : gpio(gpio), pin(pin), value(value) {}

  int run() {
    if (is_error(gpio.set_pin(pin, value))) {
      std::println("GPIO write failed");
      return 1;
    }
    std::println("GPIO{} = {}", pin, value ? 1 : 0);
    return 0;
  }
};

}  // namespace commands
