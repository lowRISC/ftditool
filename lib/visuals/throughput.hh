// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <chrono>
#include <print>

struct Throughput {
  size_t progress;
  std::chrono::high_resolution_clock::time_point start_time;
  std::chrono::high_resolution_clock::time_point elapsed;

  Throughput() {
    elapsed = start_time = std::chrono::high_resolution_clock::now();
    progress             = 0;
  }

  Throughput& add(size_t transfered) {
    progress += transfered;
    elapsed = std::chrono::high_resolution_clock::now();
    return *this;
  }

  Throughput& total(size_t transfered) {
    progress = transfered;
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
    return std::format("Transfered {} in {}s, {}", human_size(mb, "b"), seconds,
                       human_size(throughput, "b/s"));
  }

  void show() { std::println("{}", render()); }

 private:
  std::string human_size(float num, const char* suffix = "B") {
    std::array<std::string, 6> units{"", "ki", "Mi", "Gi", "Ti", "Pi"};
    for (auto unit : units) {
      if (abs(num) < 1024) {
        return std::format("{:.1f} {}{}", num, unit, suffix);
      }
      num /= 1024;
    }
    return "";
  }
};
