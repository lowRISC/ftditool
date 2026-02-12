
// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <concepts>
#include <span>
#include "helpers.hh"

namespace embeddedpp {

template <typename T>
concept SpiHost = requires(T device, Data data, Transfers transfers) {
  { device.transfer(data, data) } -> std::same_as<Result<Data>>;

  { device.transaction(transfers) } -> std::same_as<Status>;
};

}  // namespace embeddedpp
