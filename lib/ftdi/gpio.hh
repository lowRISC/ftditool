// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <optional>
#include "libft4222.h"
#include "ftdi.hh"
#include "embeddedpp/helpers.hh"

namespace ftdi {

class Gpio {
  FT_HANDLE handle;
  bool mpsse = false;

 public:
  explicit Gpio(FT_HANDLE handle) noexcept : handle(handle) {}
  Gpio(FT_HANDLE handle, bool mpsse) noexcept : handle(handle), mpsse(mpsse) {}
  ~Gpio() = default;

  void close();

  embeddedpp::Status set_pin(uint8_t pin, bool value);
  embeddedpp::Result<bool> get_pin(uint8_t pin);

  static std::optional<Gpio> from_device_info(DeviceInfo& device);
};

}  // namespace ftdi
