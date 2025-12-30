// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <vector>
#include <format>
#include <magic_enum/magic_enum.hpp>

namespace ftdi {
enum class DeviceType {
  Ftdi_bm,
  Ftdi_am,
  Ftdi_100ax,
  Ftdi_unknown,
  Ftdi_2232c,
  Ftdi_232r,
  Ftdi_2232h,
  Ftdi_4232h,
  Ftdi_232h,
  Ftdi_x_series,
  Ftdi_4222h_0,
  Ftdi_4222h_1_2,
  Ftdi_4222h_3,
  Ftdi_4222_prog,
  Ftdi_900,
  Ftdi_930,
  Ftdi_umftpd3a,
  Ftdi_2233hp,
  Ftdi_4233hp,
  Ftdi_2232hp,
  Ftdi_4232hp,
  Ftdi_233hp,
  Ftdi_232hp,
  Ftdi_2232ha,
  Ftdi_4232ha,
};

struct DeviceInfo {
  uint32_t flags;
  DeviceType type;
  uint32_t id;
  uint32_t loc_id;
  std::string serial_number;
  std::string description;
  FT_HANDLE handle;

  /**
   * @brief Finds devices by description string.
   * @return std::nullopt if the scan failed, otherwise a vector (empty or populated).
   */
  static std::vector<DeviceInfo> filter_by_description(std::span<DeviceInfo> list,
                                                       const std::string& search_str) {
    std::vector<DeviceInfo> filtered;
    for (const auto& dev : list) {
      if (dev.description.find(search_str) != std::string::npos) {
        filtered.push_back(dev);
      }
    }
    return filtered;
  }
};
}  // namespace ftdi

template <>
struct std::formatter<FT4222_STATUS> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const FT4222_STATUS& type, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{}", magic_enum::enum_name(type));
  }
};

template <>
struct std::formatter<ftdi::DeviceType> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const ftdi::DeviceType& type, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "DeviceType::{}", magic_enum::enum_name(type));
  }
};

template <>
struct std::formatter<ftdi::DeviceInfo> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const ftdi::DeviceInfo& info, std::format_context& ctx) const {
    return std::format_to(
        ctx.out(), "device: {{\n\ttype: {}\n\tid: {}\n\tloc_id: {}\n\tserial: {}\n\tdesc: {}\n}}",
        info.type, info.id, info.loc_id, info.serial_number, info.description);
  }
};
