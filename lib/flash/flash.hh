// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "embeddedpp/spi.hh"
#include "jedec.hh"
#include "sfdp.hh"
#include <cstdint>

namespace flash {

enum : std::size_t {
  PageSize    = 256,
  SectorSize  = 1024 * 4,
  BlockSize   = 1024 * 32,
  Block64Size = 1024 * 64,
};

template <typename T>
using Option = std::optional<T>;

using Result = std::optional<std::span<uint8_t>>;

enum Opcode : uint8_t {
  WriteStatus1    = 0x01,
  PageProgram     = 0x02,
  Read            = 0x03,
  WriteDisable    = 0x04,
  ReadStatus1     = 0x05,
  WriteEnable     = 0x06,
  ReadFast        = 0x0b,
  ReadFast4b      = 0x0c,
  WriteStatus3    = 0x11,
  PageProgram4b   = 0x12,
  Read4b          = 0x13,
  ReadStatus3     = 0x15,
  SectorErase     = 0x20,
  SectorErase4b   = 0x21,
  WriteStatus2    = 0x31,
  ReadStatus2     = 0x35,
  ReadDual        = 0x3b,
  BlockErase32k   = 0x52,
  ReadSfdp        = 0x5a,
  BlockErase32k4b = 0x5c,
  ResetEnable     = 0x66,
  ReadQuad        = 0x6b,
  Reset           = 0x99,
  ReadJedec       = 0x9f,
  Enter4bAddr     = 0xb7,
  ChipErase       = 0xc7,
  BlockErase64k   = 0xd8,
  BlockErase64k4b = 0xdc,
  Exit4bAddr      = 0xe9,
};

using Page   = std::array<uint8_t, PageSize>;
using Sector = std::array<uint8_t, SectorSize>;

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
  Sfdp sfdp_;

 public:
  Generic(T& spih) : spih(spih) {};

  Option<Jedec> jedec() {
    std::array<uint8_t, 1> cmd = {Opcode::ReadJedec};
    std::array<uint8_t, sizeof(Jedec)> resp;

    std::span<uint8_t> ret = TRY_OPT(spih.transfer(cmd, resp));
    return Jedec::from(ret.last<sizeof(Jedec)>());
  }

  Option<Sfdp> sfdp() {
    std::array<uint8_t, 5> cmd = {Opcode::ReadSfdp, 0x00, 0x00, 0x00, 0x00};
    std::array<uint8_t, sizeof(Sfdp)> resp;

    TRY_OPT(spih.transfer(cmd, resp));
    return Sfdp::try_from(resp);
  }

  Option<Page> single_read_page(uint32_t address) {
    std::array<uint8_t, 4> cmd = {
        Opcode::Read,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };
    std::array<uint8_t, 256> resp;

    auto ret = TRY_OPT(spih.transfer(cmd, resp));

    Page page = {0x00};
    std::copy(ret.begin() + 4, ret.end(), page.begin());
    return Option{page};
  }

  template <std::size_t N>
  Option<std::array<uint8_t, N>> quad_read(uint32_t address) {
    std::array<uint8_t, 1> cmd = {Opcode::ReadQuad};

    std::array<uint8_t, 3> addr = {
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };

    std::array<uint8_t, 1> dummy    = {0x00};
    std::array<uint8_t, N> ret_data = {0x00};

    std::array<embeddedpp::Transfer, 4> transfers = {
        embeddedpp::Transfer{embeddedpp::SpiIoMode::Single, embeddedpp::SpiDirection::Write, cmd},
        embeddedpp::Transfer{embeddedpp::SpiIoMode::Single, embeddedpp::SpiDirection::Write, addr},
        embeddedpp::Transfer{embeddedpp::SpiIoMode::Single, embeddedpp::SpiDirection::Write, dummy},
        embeddedpp::Transfer{embeddedpp::SpiIoMode::Quad, embeddedpp::SpiDirection::Read, ret_data},
    };

    TRY_OPT(spih.transaction(transfers));
    return Option{ret_data};
  }

  Option<Page> quad_read_page(uint32_t address) { return quad_read<256>(address); }

  Option<Sector> quad_read_sector(uint32_t address) { return quad_read<4096>(address); }

  Option<bool>
  quad_page_program(uint32_t address, std::span<uint8_t, 256> data, uint8_t upcode = 0x32) {
    write_enable();
    std::array<uint8_t, 1> cmd = {upcode};

    std::array<uint8_t, 3> addr = {
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };

    std::array<embeddedpp::Transfer, 3> transfers = {
        embeddedpp::Transfer{embeddedpp::SpiIoMode::Single, embeddedpp::SpiDirection::Write, cmd},
        embeddedpp::Transfer{embeddedpp::SpiIoMode::Single, embeddedpp::SpiDirection::Write, addr},
        embeddedpp::Transfer{embeddedpp::SpiIoMode::Quad, embeddedpp::SpiDirection::Write, data},
    };

    TRY_OPT(spih.transaction(transfers));
    wait_not_busy();
    return true;
  }

  Option<bool> write_status(uint8_t val, Opcode code = Opcode::WriteStatus1) {
    std::array<uint8_t, 2> cmd = {code, val};

    auto ret = TRY_OPT(spih.transfer(cmd, std::span<uint8_t>()));
    return true;
  }

  Option<bool> enable_quad(bool enable) {
    if (!this->sfdp_.is_valid()) {
      auto _sfdp = this->sfdp();
      if (!_sfdp) {
        return std::nullopt;
      }
      this->sfdp_ = *_sfdp;
    }

    auto single_bit_method = [&](Opcode rd_op, Opcode wr_op, size_t bit) -> Option<bool> {
      auto status = read_status(rd_op);
      if (!status) {
        return std::nullopt;
      }

      auto val = *status & ~(1 << bit);
      val      = val | (1 << bit);
      write_status(val, wr_op);
      return true;
    };

    switch (sfdp_.get_quad_enable_mechanism()) {
      case 2:
        return single_bit_method(Opcode::ReadStatus1, Opcode::WriteStatus1, 6);
      case 3:
        return single_bit_method(Opcode::ReadStatus2, Opcode::WriteStatus2, 7);
      case 6:
        return single_bit_method(Opcode::ReadStatus2, Opcode::WriteStatus2, 1);
      default:
        return std::nullopt;
    };

    return true;
  }

  Option<bool> erase(uint32_t address, Opcode op = Opcode::SectorErase) {
    write_enable();
    std::array<uint8_t, 4> cmd = {
        op,
        static_cast<uint8_t>(address >> 16),
        static_cast<uint8_t>(address >> 8),
        static_cast<uint8_t>(address >> 0),
    };
    auto ret = spih.transfer(cmd, std::span<uint8_t>());
    if (embeddedpp::is_error(ret)) {
      return std::nullopt;
    }
    return wait_not_busy();
  }

  Option<bool> reset() {
    std::array<uint8_t, 1> cmd = {Opcode::Reset};
    TRY_OPT(spih.transfer(cmd, std::span<uint8_t>()));
    return true;
  }

  Option<bool> write_enable(bool enable = true) {
    std::array<uint8_t, 1> cmd = {enable ? Opcode::WriteEnable : Opcode::WriteDisable};
    TRY_OPT(spih.transfer(cmd, std::span<uint8_t>()));
    return true;
  }

  Option<uint8_t> read_status(Opcode code = Opcode::ReadStatus1) {
    std::array<uint8_t, 1> cmd = {code};

    auto ret = TRY_OPT(spih.transfer(cmd, cmd));
    return ret[0];
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

    std::ranges::copy(data, slice.begin());
    TRY_OPT(spih.transfer(cmd, std::span<uint8_t>()));
    wait_not_busy();
    return write_enable(false);
  }

  Option<bool> is_busy() {
    if (auto status = read_status()) {
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
