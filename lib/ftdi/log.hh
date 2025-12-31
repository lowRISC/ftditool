// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>
#include <span>
#include <format>

template <std::size_t N>
struct std::formatter<std::array<uint8_t, N>> : std::formatter<std::string_view> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  auto format(const std::array<uint8_t, N>& arr, format_context& ctx) const {
    auto out = ctx.out();
    out      = std::format_to(out, "Array<uint8_t, {} >: {{", N);

    for (std::size_t i = 0; i < N; ++i) {
      out = std::format_to(out, "{:02x}", arr[i]);
      if (i != N - 1) {
        out = std::format_to(out, ", ");
      }
    }
    return std::format_to(out, "}}");
  }
};

template <std::size_t N>
struct std::formatter<std::span<uint8_t, N>> : std::formatter<std::string_view> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  auto format(const std::span<uint8_t, N>& arr, format_context& ctx) const {
    auto out = ctx.out();
    out      = std::format_to(out, "Span<uint8_t, {} >: {{", N);

    for (std::size_t i = 0; i < N; ++i) {
      out = std::format_to(out, "{:02x}", arr[i]);
      if (i != N - 1) {
        out = std::format_to(out, ", ");
      }
    }
    return std::format_to(out, "}}");
  }
};

template <>
struct std::formatter<std::span<uint8_t>> : std::formatter<std::string_view> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  auto format(const std::span<uint8_t>& arr, format_context& ctx) const {
    auto out = ctx.out();
    out      = std::format_to(out, "Span<uint8_t, {}> = {{", arr.size());

    for (std::size_t i = 0; i < arr.size(); ++i) {
      out = std::format_to(out, "{:02x}", arr[i]);
      if (i != arr.size() - 1) {
        out = std::format_to(out, ", ");
      }
    }
    return std::format_to(out, "}}");
  }
};

template <>
struct std::formatter<std::vector<uint8_t>> : std::formatter<std::string_view> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  auto format(const std::vector<uint8_t>& arr, format_context& ctx) const {
    auto out = ctx.out();
    out      = std::format_to(out, "Vector<uint8_t, {}> = {{", arr.size());

    for (std::size_t i = 0; i < arr.size(); ++i) {
      out = std::format_to(out, "{:02x}", arr[i]);
      if (i != arr.size() - 1) {
        out = std::format_to(out, ", ");
      }
    }
    return std::format_to(out, "}}");
  }
};
