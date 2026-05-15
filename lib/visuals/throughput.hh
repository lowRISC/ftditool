// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <chrono>
#include <cmath>
#include <print>

struct Throughput {
  size_t progress;
  std::chrono::high_resolution_clock::time_point start_time;
  std::chrono::high_resolution_clock::time_point elapsed;

  Throughput() {
    elapsed = start_time = std::chrono::high_resolution_clock::now();
    progress             = 0;
  }

  Throughput& add(size_t transferred) {
    progress += transferred;
    elapsed = std::chrono::high_resolution_clock::now();
    return *this;
  }

  Throughput& total(size_t transferred) {
    progress = transferred;
    elapsed  = std::chrono::high_resolution_clock::now();
    return *this;
  }

  std::string render(bool short_fmt = true) {
    std::chrono::duration<float> duration = elapsed - start_time;
    float seconds                         = duration.count();
    float mb                              = static_cast<float>(progress);
    float throughput                      = mb * 8 / seconds;

    if (short_fmt) {
      return std::format("{}", human_size(throughput, "bps"));
    }
    return std::format("{} in {}, {}", human_size(mb, "b"), human_time(seconds),
                       human_size(throughput, "b/s"));
  }

  void show(bool short_fmt = true) { std::println("{}", render(short_fmt)); }

 private:
  std::string human_size(float num, const char* suffix = "B") {
    constexpr std::array<std::string_view, 6> units{"", "ki", "Mi", "Gi", "Ti", "Pi"};
    for (auto unit : units) {
      if (abs(num) < 1024) {
        return std::format("{:.1f} {}{}", num, unit, suffix);
      }
      num /= 1024;
    }
    return "";
  }

  std::string human_time(float seconds) {
    if (seconds <= 0.0f) {
      return "0";
    }

    constexpr std::array<std::string_view, 3> units{"s", "m", "h"};
    std::string result;
    auto num = static_cast<int>(std::round(seconds));

    for (size_t i = 0; i < units.size(); ++i) {
      int part = (i < static_cast<int>(units.size()) - 1) ? num % 60 : num;
      if (part != 0) {
        result = std::format("{}{}", part, units[i]) + result;
      }
      num /= 60;
      if (num == 0) {
        break;
      }
    }

    return result;
  }
};
