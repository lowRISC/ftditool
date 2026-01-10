// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "flash/flash.hh"

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
      std::println("{:#x} : {}", addr, *page);
    } else {
      std::println("page failed");
    }

    if (!this->flash.enable_quad(addr)) {
      std::println("enable quad failed");
    }

    if (auto page = this->flash.quad_read_page(addr)) {
      std::println("{:#x} : {}", addr, *page);
      return 0;
    }
    std::println("quad read failed");
    return 1;
  }
};

template <typename T>
struct TestPage : public Commands<T> {
  std::size_t addr;
  TestPage(flash::Generic<T> f, std::size_t addr = 0) : Commands<T>(f), addr(addr) {}

  int run() override {
    if (!this->flash.reset() || !this->flash.erase(addr)) {
      std::println("Erase failed");
      return 0;
    }
    std::vector<uint8_t> data(256, 0xaa);
    if (!this->flash.single_page_program(addr, std::span<uint8_t, 256>(data))) {
      std::println("Program failed");
      return 0;
    }

    if (auto page = this->flash.single_read_page(addr)) {
      std::println("{:#x} : {}", addr, *page);
      return 0;
    }
    std::println("page failed");
    return 1;
  }
};

template <typename T>
struct LoadFile : public Commands<T> {
  std::size_t addr;
  std::string& filename;
  LoadFile(flash::Generic<T> f, std::string& filename, std::size_t addr = 0)
      : Commands<T>(f), filename(filename), addr(addr) {}

  int run() override {
    std::println("Not implemented");
    return 1;
  }
};
}  // namespace commands
