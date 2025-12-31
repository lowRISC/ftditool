
// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <format>
#include <cstdint>

namespace flash {

struct [[gnu::packed]] Jedec {
  uint16_t device_id;
  uint8_t manufacturer_id;
  uint8_t continuation_len;

  constexpr inline static Jedec from(const std::array<uint8_t, 4> arr) {
    return std::bit_cast<Jedec>(arr);
  }

  constexpr inline static Jedec from(const std::span<uint8_t, 4> slice) {
    std::array<uint8_t, 4> arr{slice[0], slice[1], slice[2], slice[3]};
    return from(arr);
  }
};

}  // namespace flash

template <>
struct std::formatter<flash::Jedec> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const flash::Jedec& info, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "Jedec: {{\n\tdid: {:#x}\n\tmid: {:#x}\n\tcont_len: {}}}\n",
                          info.device_id, info.manufacturer_id, info.continuation_len);
  }
};
