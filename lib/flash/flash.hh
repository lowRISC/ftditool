// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "embeddedpp/spi.hh"
#include "jedec.hh"
#include <cstdint>

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

using Page = std::array<uint8_t, 256>;

enum class Status1 : uint8_t {
  Busy        = 0x01 << 0,  // Bit 0
  WriteEnable = 0x01 << 1,  // Bit 1
  BP0         = 0x01 << 2,  // Bit 2
  BP1         = 0x01 << 3,  // Bit 3
  BP2         = 0x01 << 4,  // Bit 4
  SR_Lock     = 0x01 << 7   // Bit 7
};

template <embeddedpp::SpiHost T>
class Generic {
  T& spih;

 public:
  Generic(T& spih) : spih(spih) {};

  Option<Jedec> jedec() {
    std::array<uint8_t, 5> cmd = {
        Opcode::ReadJedec,
    };
    std::span<uint8_t> ret = TRY_OPT(spih.transfer(cmd));
    return Jedec::from(ret.last<4>());
  }

  Option<Page> single_read_page(uint32_t address) {
    std::array<uint8_t, 256 + 4> cmd = {
        Opcode::Read,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };

    auto ret = TRY_OPT(spih.transfer(cmd));

    Page page;
    std::copy(ret.begin() + 4, ret.end(), page.begin());
    return Option{page};
  }

  Option<uint8_t> read_status1() {
    std::array<uint8_t, 2> cmd = {
        Opcode::ReadStatus1,
    };
    auto ret = TRY_OPT(spih.transfer(cmd));
    return ret[1];
  }

  Option<bool> erase(uint32_t address, Opcode op = Opcode::SectorErase) {
    write_enable();
    std::array<uint8_t, 4> cmd = {
        op,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };
    auto ret = spih.transfer(cmd);
    if (embeddedpp::is_error(ret)) {
      return std::nullopt;
    }
    return wait_not_busy();
  }

  Option<bool> reset() {
    std::array<uint8_t, 1> cmd = {
        Opcode::Reset,
    };
    TRY_OPT(spih.transfer(cmd));
    return true;
  }

  Option<bool> write_enable(bool enable = true) {
    std::array<uint8_t, 1> cmd = {
        enable ? Opcode::WriteEnable : Opcode::WriteDisable,
    };
    TRY_OPT(spih.transfer(cmd));
    return true;
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
    TRY_OPT(spih.transfer(cmd));
    wait_not_busy();
    return write_enable(false);
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
