// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <format>
#include <cstdint>

namespace flash {

struct [[gnu::packed]] Jedec {
  uint8_t manufacturer_id;
  uint8_t memory_type;
  uint8_t capacity_code;

  constexpr inline static Jedec from(const std::array<uint8_t, 3> arr) {
    return std::bit_cast<Jedec>(arr);
  }

  constexpr inline static Jedec from(const std::span<uint8_t, 3> slice) {
    std::array<uint8_t, 3> arr{slice[0], slice[1], slice[2]};
    return from(arr);
  }

  inline size_t get_capacity() { return 1 << this->capacity_code; }
};

}  // namespace flash

template <>
struct std::formatter<flash::Jedec> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const flash::Jedec& info, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "Jedec:\n\tmem_type: {:#x}\n\tcapacity: {:#x}\n\tmid: {:#x}\n",
                          info.memory_type, info.capacity_code, info.manufacturer_id);
  }
};
