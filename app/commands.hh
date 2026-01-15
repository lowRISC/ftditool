// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "flash/flash.hh"
#include <print>
#include <iostream>
#include <fstream>

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

    if (quad && !this->flash.enable_quad(addr)) {
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

struct ProgressBar {
  bool finished = false;
  int width = 100;
  int total = 0;
  float progress = 0;
  std::string label;

  ProgressBar(int total, std::string label = "Progress", int width = 100): total(total), label(label), width(width), finished(false){}
  void update(int progress) {
      if (this->total <= progress && this->finished) {
        std::println("");
        return;
      }
      this->finished = (this->total <= progress); 
      this->progress = static_cast<float>(progress) / this->total;
      int filled = this->width * this->progress;

      std::print("\r\033[32m {} [", label);
      for (int i = 0; i < width; ++i) {
          std::print("{}", i < filled ? "■" : " ");
      }
      std::cout << "] " << (int)(this->progress * 100) << "%\033[0m" << std::flush;
  }

};

template <typename T>
struct LoadFile : public Commands<T> {
  std::size_t addr;
  std::string& filename;
  bool quad;
  LoadFile(flash::Generic<T> f, std::string& filename, std::size_t addr = 0, bool quad = false)
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
    auto size = file_size + (file_size - file_size % flash::PageSize);
    std::vector<uint8_t> buffer(size, 0xff);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
      std::println("Error reading the file.");
      return 0;
    }

    this->flash.reset();
    if (quad && !this->flash.enable_quad(addr)) {
      std::println("enable quad failed");
      return 0;
    }

    std::span<uint8_t> data(buffer);
    auto progress_bar = ProgressBar(buffer.size(), " ", 50);
    while (data.size() > 0) {
      if (addr % flash::SectorSize && !this->flash.erase(addr)) {
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

    std::println("Finished!");
    return 1;
  }
};
}  // namespace commands
