// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "libft4222.h"
#include "ftd2xx.h"
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <algorithm>
#include <iostream>
#include <print>
#include "log.hh"
#include "spi_host.hh"

namespace ftdi {
embeddedpp::Result<std::span<uint8_t>> SpiHost::transfer(std::span<uint8_t> payload) {
  uint16_t received = 0;
  log("SPI -->> {}", payload);

  FT4222_SPIMaster_SetLines(handle, SPI_IO_SINGLE);

  FT4222_STATUS status;
  status = FT4222_SPIMaster_SingleReadWrite(handle, payload.data(), payload.data(), payload.size(),
                                            &received, true);
  if (FT4222_OK != status) {
    std::cerr << std::format("SingleReadWrite:{}\n", status);
    return embeddedpp::Code::Generic;
  }

  if (received < payload.size()) {
    std::cerr << std::format("Wrote only {}/{}\n", received, payload.size());
    return embeddedpp::Code::Generic;
  }
  log("SPI <<-- {}", payload);
  return payload;
}

embeddedpp::Status SpiHost::transaction(embeddedpp::Transfers transfers) {
  std::vector<uint8_t> payload;
  uint8_t single_bytes_wr          = 0;
  uint32_t multi_bytes_wr          = 0;
  uint32_t multi_bytes_rd          = 0;
  embeddedpp::SpiIoMode multi_mode = embeddedpp::SpiIoMode::Single;
  std::span<uint8_t> rd_buffer;

  // The FT4222 API supports at most 2 modes per transaction, this means that the transfer mode
  // bellow are supported:
  // * 111: Single for cmd, addr and data. 112: Single for cmd and addr, but dual for data.
  // * 122: Single for cmd, but dual for addr and data.
  // * 114: Single for cmd and addr, but quad for data.
  // * 144: Single for cmd, but quad for addr and data.
  for (auto transfer : transfers) {
    payload.insert(payload.end(), transfer.data.begin(), transfer.data.end());
    if (transfer.mode == embeddedpp::SpiIoMode::Single) {
      if (transfer.direction == embeddedpp::SpiDirection::Write) {
        single_bytes_wr += transfer.data.size();
      }
    } else {
      if (multi_mode != embeddedpp::SpiIoMode::Single && multi_mode != transfer.mode) {
        // Max of two modes supported per transaction.
        return embeddedpp::Code::InvalidArgument;
      }
      multi_mode = transfer.mode;
      if (transfer.direction == embeddedpp::SpiDirection::Write) {
        multi_bytes_wr += transfer.data.size();
        continue;
      }

      if (multi_bytes_rd > 0) {
        // This implementation only supports one read per transaction.
        return embeddedpp::Code::InvalidArgument;
      }
      multi_bytes_rd = transfer.data.size();
      rd_buffer      = transfer.data.subspan(0);
    }
  }

  auto map_mode = [](embeddedpp::SpiIoMode mode) -> FT4222_SPIMode {
    switch (mode) {
      case embeddedpp::SpiIoMode::Quad:
        return SPI_IO_QUAD;
      case embeddedpp::SpiIoMode::Dual:
        return SPI_IO_DUAL;
      case embeddedpp::SpiIoMode::Single:
      default:
        return SPI_IO_SINGLE;
    }
  };

  uint32_t received = 0;
  FT4222_STATUS status;
  status = FT4222_SPIMaster_SetLines(handle, map_mode(multi_mode));
  if (FT4222_OK != status) {
    std::cerr << std::format("SetLines:{}\n", status);
    return embeddedpp::Code::Generic;
  }

  log("SPI -->> {}", payload);
  status =
      FT4222_SPIMaster_MultiReadWrite(handle, rd_buffer.data(), payload.data(), single_bytes_wr,
                                      multi_bytes_wr, multi_bytes_rd, &received);
  if (FT4222_OK != status) {
    std::cerr << std::format("MultiReadWrite:{}\n", status);
    return embeddedpp::Code::Generic;
  }
  log("SPI <<-- {}", rd_buffer);

  return embeddedpp::Code::Ok;
};

bool SpiHost::write(std::span<uint8_t> payload, bool deassert_cs) {
  uint16_t received = 0;

  FT4222_STATUS status;
  status =
      FT4222_SPIMaster_SingleWrite(handle, payload.data(), payload.size(), &received, deassert_cs);
  if (FT4222_OK != status) {
    std::cerr << std::format("write: SingleWrite:{}\n", status);
    return false;
  }

  if (received < payload.size()) {
    std::cerr << std::format("Wrote only {}/{}\n", received, payload.size());
    return false;
  }
  return true;
}

Result SpiHost::read(uint32_t size, bool deassert_cs) {
  std::vector<uint8_t> buffer(size, 0xfe);
  uint16_t received = 0;

  FT4222_STATUS status =
      FT4222_SPIMaster_SingleRead(handle, buffer.data(), size, &received, deassert_cs);
  if (FT4222_OK != status) {
    std::cerr << std::format("read: SingleRead:{}\n", status);
    return std::nullopt;
  }

  if (received < size) {
    std::cerr << std::format("Read only {}/{}\n", received, size);
    return std::nullopt;
  }
  return Result{buffer};
}

std::optional<SpiHost> SpiHost::from_device_info(DeviceInfo& device) {
  FT_HANDLE handle;
  FT_STATUS res = FT_OpenEx((PVOID)(uintptr_t)device.loc_id, FT_OPEN_BY_LOCATION, &handle);
  if (res != FT_OK) {
    std::cerr << std::format("Open:{}\n", res);
    return std::nullopt;
  }

  FT4222_STATUS status;
  status = FT4222_SPIMaster_Init(handle,
                                 SPI_IO_SINGLE,     // 1 channel
                                 CLK_DIV_4,         // 60 MHz / 4 == 15 MHz
                                 CLK_IDLE_LOW,      // clock idles at logic 0
                                 CLK_LEADING,       // data captured on rising edge
                                 SLAVE_SELECT(0));  // Use SS0O for cs
  if (FT4222_OK != status) {
    std::cerr << std::format("SpiInit:{}\n", status);
    return std::nullopt;
  }

  status = FT4222_SPI_SetDrivingStrength(handle, DS_8MA, DS_8MA, DS_8MA);
  if (FT4222_OK != status) {
    std::cerr << std::format("SetDrivingStrenght:{}\n", status);
    return std::nullopt;
  }

  return std::optional<SpiHost>{handle};
}
};  // namespace ftdi
