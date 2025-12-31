// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ftdi/spi_host.hh"
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

class Generic {
  ftdi::SpiHost& spih;

 public:
  Generic(ftdi::SpiHost& spih) : spih(spih) {};
  Option<Jedec> jedec();
  Option<Page> single_read_page(uint32_t address);
  Option<uint8_t> read_status1();
  Option<bool> erase(uint32_t address, Opcode op = Opcode::SectorErase);
  Option<bool> reset();
  Option<bool> write_enable(bool enable = true);
  Option<bool> single_page_program(uint32_t address, std::span<uint8_t, 256> data);
  Option<bool> is_busy();
  Option<bool> wait_not_busy();
};
}  // namespace flash
