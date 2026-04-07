// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <concepts>
#include <cstdint>
#include "helpers.hh"

namespace embeddedpp {

template <typename T>
concept Gpio = requires(T t, uint8_t pin, bool value) {
  { t.set_pin(pin, value) } -> std::same_as<Status>;
  { t.get_pin(pin) } -> std::same_as<Result<bool>>;
};

}  // namespace embeddedpp
