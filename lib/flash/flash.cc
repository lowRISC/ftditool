// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "flash.hh"
#include "ftdi/spi_host.hh"
#include <format>
#include <cstdint>
#include <algorithm>

namespace flash {

Option<Jedec> Generic::jedec() {
  std::array<uint8_t, 5> cmd = {
      Opcode::ReadJedec,
  };
  if (auto res = spih.transfer(cmd)) {
    return Jedec::from(res->last<4>());
  }
  return std::nullopt;
}

Option<Page> Generic::single_read_page(uint32_t address) {
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

Option<uint8_t> Generic::read_status1() {
  std::array<uint8_t, 2> cmd = {
      Opcode::ReadStatus1,
  };
  if (auto res = spih.transfer(cmd)) {
    return (*res)[1];
  }
  return std::nullopt;
}

Option<bool> Generic::erase(uint32_t address, Opcode op) {
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

Option<bool> Generic::reset() {
  std::array<uint8_t, 1> cmd = {
      Opcode::Reset,
  };
  if (auto res = spih.transfer(cmd)) {
    return true;
  }
  return std::nullopt;
}

Option<bool> Generic::write_enable(bool enable) {
  std::array<uint8_t, 1> cmd = {
      enable ? Opcode::WriteEnable : Opcode::WriteDisable,
  };
  if (auto res = spih.transfer(cmd)) {
    return true;
  }
  return std::nullopt;
}

Option<bool> Generic::single_page_program(uint32_t address, std::span<uint8_t, 256> data) {
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

Option<bool> Generic::is_busy() {
  if (auto status = read_status1()) {
    auto mask = static_cast<uint8_t>(Status1::Busy);
    return (*status & mask) == mask;
  }
  return std::nullopt;
}

Option<bool> Generic::wait_not_busy() {
  while (auto busy = is_busy()) {
    if (*busy == false) {
      return true;
    }
  }
  return std::nullopt;
}
}  // namespace flash
