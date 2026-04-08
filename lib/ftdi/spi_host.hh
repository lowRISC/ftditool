// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include "libft4222.h"
#include "libmpsse_spi.h"
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
  bool traces = false;
  bool mpsse  = false;

 public:
  explicit SpiHost(FT_HANDLE handle, bool mpsse = false) noexcept
      : handle(handle), mpsse(mpsse), traces(false) {}
  ~SpiHost() {
  }

  void close() {
    if (mpsse) {
      SPI_CloseChannel(handle);
      Cleanup_libMPSSE();
    } else {
      FT_Close(handle);
    }
  }

  embeddedpp::Result<std::span<uint8_t>>
  transfer(std::span<uint8_t> write, std::span<uint8_t> read);

  embeddedpp::Status transaction(embeddedpp::Transfers transfers);

  bool write(std::span<uint8_t> payload, bool deassert_cs = true);

  Result read(uint32_t size, bool deassert_cs = true);

  bool set_clock(size_t clock);

  void with_traces(bool enable) { traces = enable; };

  static std::optional<SpiHost> from_device_info(DeviceInfo& device);

 private:
  template <typename... Args>
  void log(std::format_string<Args...> fmt, Args&&... args) {
    if (this->traces) {
      std::println(fmt, std::forward<Args>(args)...);
    }
  }
};
}  // namespace ftdi
