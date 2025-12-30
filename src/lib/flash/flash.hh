// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ftdi/spi_host.hh"
#include <format>
#include <cstring>
#include <bit>
#include <algorithm>

namespace flash {

template <typename T>
using Option = std::optional<T>;

using Result = std::optional<std::span<uint8_t>>;

enum Opcode : uint8_t {
  ReadJedec       = 0x9f,
  ReadSfdp        = 0x5a,
  Read            = 0x03,
  Read4b          = 0x13,
  ReadFast        = 0x0b,
  ReadFast4b      = 0x0c,
  ReadDual        = 0x3b,
  ReadQuad        = 0x6b,
  WriteEnable     = 0x06,
  WriteDisable    = 0x04,
  ReadStatus1     = 0x05,
  ReadStatus2     = 0x35,
  ReadStatus3     = 0x15,
  WriteStatus1    = 0x01,
  WriteStatus2    = 0x31,
  WriteStatus3    = 0x11,
  ChipErase       = 0xc7,
  SectorErase     = 0x20,
  BlockErase32k   = 0x52,
  BlockErase64k   = 0xd8,
  PageProgram     = 0x02,
  Enter4bAddr     = 0xb7,
  Exit4bAddr      = 0xe9,
  ResetEnable     = 0x66,
  Reset           = 0x99,
  SectorErase4b   = 0x21,
  BlockErase32k4b = 0x5c,
  BlockErase64k4b = 0xdc,
  PageProgram4b   = 0x12,
};

struct [[gnu::packed]] Jedec {
  uint16_t device_id;
  uint8_t manufacturer_id;
  uint8_t continuation_len;

  static Jedec from(const std::array<uint8_t, 4> arr) { return std::bit_cast<Jedec>(arr); }

  static Jedec from(const std::span<uint8_t, 4> slice) {
    std::array<uint8_t, 4> arr{slice[0], slice[1], slice[2], slice[3]};
    return from(arr);
  }
};

using Page = std::array<uint8_t, 256>;

enum class Status1 : uint8_t {
  Busy        = 0x01 << 0,  // Bit 0
  WriteEnable = 0x01 << 1,  // Bit 1
  BP0         = 0x01 << 2,  // Bit 2
  BP1         = 0x01 << 3,  // Bit 3
  BP2         = 0x01 << 4,  // Bit 4
  SR_Lock     = 0x01 << 7   // Bit 7
};

class Generic {
  ftdi::SpiHost& spih;

 public:
  Generic(ftdi::SpiHost& spih) : spih(spih) {};

  Option<Jedec> jedec() {
    std::array<uint8_t, 5> cmd = {
        Opcode::ReadJedec,
    };
    if (auto res = spih.transfer(cmd)) {
      std::cout << std::format("{} -> {} \n", __func__, *res);
      return Jedec::from(res->last<4>());
    }
    return std::nullopt;
  }

  Option<Page> single_read_page(uint32_t address) {
    std::array<uint8_t, 256 + 4> cmd = {
        Opcode::Read,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };
    if (auto res = spih.transfer(cmd)) {
      Page page;
      std::copy(res->begin() + 4, res->end(), page.begin());
      return Option{page};
    }
    return std::nullopt;
  }

  Option<uint8_t> read_status1() {
    std::array<uint8_t, 2> cmd = {
        Opcode::ReadStatus1,
    };
    if (auto res = spih.transfer(cmd)) {
      return (*res)[1];
    }
    return std::nullopt;
  }

  Option<bool> erase(uint32_t address, Opcode op = Opcode::SectorErase) {
    write_enable();
    std::array<uint8_t, 4> cmd = {
        op,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };
    if (auto res = spih.transfer(cmd)) {
      return wait_not_busy();
    }
    return std::nullopt;
  }

  Option<bool> reset() {
    std::array<uint8_t, 1> cmd = {
        Opcode::Reset,
    };
    if (auto res = spih.transfer(cmd)) {
      return true;
    }
    return std::nullopt;
  }

  Option<bool> write_enable(bool enable = true) {
    std::array<uint8_t, 1> cmd = {
        enable ? Opcode::WriteEnable : Opcode::WriteDisable,
    };
    if (auto res = spih.transfer(cmd)) {
      return true;
    }
    return std::nullopt;
  }

  Option<bool> single_page_program(uint32_t address, std::span<uint8_t, 256> data) {
    write_enable();
    std::array<uint8_t, 256 + 4> cmd = {
        Opcode::PageProgram,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };

    auto slice = std::span<uint8_t>(cmd).last<256>();
    std::copy(data.begin(), data.end(), slice.begin());
    if (auto res = spih.transfer(cmd)) {
      wait_not_busy();
      return write_enable(false);
    }
    return std::nullopt;
  }

  Option<bool> is_busy() {
    if (auto status = read_status1()) {
      auto mask = static_cast<uint8_t>(Status1::Busy);
      return (*status & mask) == mask;
    }
    return std::nullopt;
  }

  Option<bool> wait_not_busy() {
    while (auto busy = is_busy()) {
      if (*busy == false) {
        return true;
      }
    }
    return std::nullopt;
  }
};
}  // namespace flash

template <>
struct std::formatter<flash::Jedec> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const flash::Jedec& info, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "Jedec: {{\n\tdid: {:#x}\n\tmid: {:#x}\n\tcont_len: {}}}\n",
                          info.device_id, info.manufacturer_id, info.continuation_len);
  }
};
