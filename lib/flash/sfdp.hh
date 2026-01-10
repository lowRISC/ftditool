
// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <format>
#include <cstdint>
#include <cstring>
#include <bit>
#include <algorithm>
#include <optional>
#include <span>
#include <print>

namespace flash {
struct [[gnu::packed]] SfdpHeader {
  uint8_t signature[4];
  uint8_t minor;
  uint8_t major;
  uint8_t num_headers;
  uint8_t access_proto;

  static SfdpHeader from(std::span<const uint8_t> buffer) {
    SfdpHeader header;
    std::memcpy(&header, buffer.data(), sizeof(header));
    return header;
  }

  static std::optional<SfdpHeader> try_from(std::span<const uint8_t> buffer) {
    if (buffer.size() < sizeof(SfdpHeader)) {
      return std::nullopt;
    }

    SfdpHeader header = from(buffer);

    if (!header.is_valid()) {
      return std::nullopt;
    }
    return std::optional<SfdpHeader>{header};
  }

  uint32_t get_signature() const {
    return (signature[0] << 24 | signature[1] << 16 | signature[2] << 8 | signature[3] << 0);
  }
  bool is_valid() { return get_signature() == 0x53464450; }
};

struct [[gnu::packed]] ParameterHeader {
  uint8_t id;
  uint8_t minor;
  uint8_t major;
  uint8_t length;
  uint8_t table_offset[3];
  uint8_t reserved;

  static ParameterHeader from(std::span<const uint8_t> buffer) {
    ParameterHeader param;
    std::memcpy(&param, buffer.data(), sizeof(param));
    return param;
  }

  static std::optional<ParameterHeader> try_from(std::span<const uint8_t> buffer) {
    if (buffer.size() < sizeof(ParameterHeader)) {
      return std::nullopt;
    }

    auto table = from(buffer);
    return std::optional<ParameterHeader>{table};
  }

  uint32_t get_address() const {
    return (table_offset[2] << 16 | table_offset[1] << 8 | table_offset[0] << 0);
  }

  uint16_t get_id() const { return id; }
};

struct [[gnu::packed]] Sfdp {
  SfdpHeader header;
  ParameterHeader param;
  uint32_t remainder[(256 - (sizeof(SfdpHeader) + sizeof(ParameterHeader))) / sizeof(uint32_t)];

  static Sfdp from(std::span<const uint8_t> buffer) {
    Sfdp sfdp;
    std::memcpy(&sfdp, buffer.data(), sizeof(sfdp));
    return sfdp;
  }

  static std::optional<Sfdp> try_from(std::span<const uint8_t> buffer) {
    if (buffer.size() < sizeof(Sfdp)) {
      return std::nullopt;
    }

    auto sfdp = from(buffer);

    if (!sfdp.header.is_valid()) {
      return std::nullopt;
    }
    return std::optional<Sfdp>{sfdp};
  }

  uint8_t get_quad_enable_mechanism() {
    // TODO: Improve
    uint32_t offset = param.get_address() / sizeof(uint32_t);
    uint32_t word   = remainder[offset + 14];
    return (word >> 17) & 0x03;
  }
  bool is_valid() { return header.is_valid(); }
};
}  // namespace flash

template <>
struct std::formatter<flash::SfdpHeader> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const flash::SfdpHeader& header, std::format_context& ctx) const {
    return std::format_to(ctx.out(),
                          "SfdpHeader: {{\n\tsignature: {:#x}\n\tminor: {}\n\tmajor: "
                          "{}\n\tnum_headers: {}\n\taccess_proto: {:#x}}}\n",
                          header.get_signature(), header.minor, header.major, header.num_headers,
                          header.access_proto);
  }
};

template <>
struct std::formatter<flash::ParameterHeader> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const flash::ParameterHeader& data, std::format_context& ctx) const {
    return std::format_to(ctx.out(),
                          "ParamHeader: {{\n\tid: {:#x}\n\tminor: {}\n\tmajor: "
                          "{}\n\tlength: {}\n\ttable_offset: {:#x}}}\n",
                          data.id, data.minor, data.major, data.length, data.get_address());
  }
};

template <>
struct std::formatter<flash::Sfdp> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
  auto format(const flash::Sfdp& data, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "Sfdp: {{\n\theader: {}\n\tparameter: {}}}\n", data.header,
                          data.param);
  }
};
