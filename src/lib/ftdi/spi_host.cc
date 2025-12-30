// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "libft4222.h"
#include "ftd2xx.h"
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <array>
#include <iostream>
#include "log.hh"
#include "ftdi.hh"
#include "spi_host.hh"

namespace ftdi {
Result SpiHost::transfer(std::span<uint8_t> payload, bool deassert_cs) {
  uint16_t received = 0;

  FT4222_STATUS status;
  status = FT4222_SPIMaster_SingleReadWrite(handle, payload.data(), payload.data(), payload.size(),
                                            &received, deassert_cs);
  if (FT4222_OK != status) {
    std::cerr << std::format("write: SingleReadWrite:{}\n", status);
    return std::nullopt;
  }

  if (received < payload.size()) {
    std::cerr << std::format("Wrote only {}/{}\n", payload, payload.size());
    return std::nullopt;
  }
  return Result{payload};
}

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
                                 CLK_DIV_128,       // 60 MHz / 32 == 1.875 MHz
                                 CLK_IDLE_LOW,      // clock idles at logic 0
                                 CLK_LEADING,       // data captured on rising edge
                                 SLAVE_SELECT(0));  // Use SS0O for cs
  if (FT4222_OK != status) {
    std::cerr << std::format("SpiInit:{}\n", status);
    return std::nullopt;
  }

  status = FT4222_SPI_SetDrivingStrength(handle, DS_4MA, DS_4MA, DS_4MA);
  if (FT4222_OK != status) {
    std::cerr << std::format("SetDrivingStrenght:{}\n", status);
    return std::nullopt;
  }

  return std::optional<SpiHost>{handle};
}
};  // namespace ftdi
