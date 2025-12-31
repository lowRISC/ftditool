// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include "libft4222.h"
#include "ftdi.hh"
#include "embeddedpp/helpers.hh"

namespace ftdi {
// SPI Master can assert SS0O in single mode
// SS0O and SS1O in dual mode, and
// SS0O, SS1O, SS2O and SS3O in quad mode.
#define SLAVE_SELECT(x) (1 << (x))

using Result = std::optional<std::span<uint8_t>>;

class SpiHost {
  FT_HANDLE handle;

 public:
  explicit SpiHost(FT_HANDLE handle) noexcept : handle(handle) {}
  ~SpiHost() { FT_Close(handle); }

  embeddedpp::Result<std::span<uint8_t>> transfer(std::span<uint8_t> payload);

  bool write(std::span<uint8_t> payload, bool deassert_cs = true);

  Result read(uint32_t size, bool deassert_cs = true);

  static std::optional<SpiHost> from_device_info(DeviceInfo& device);
};
}  // namespace ftdi
