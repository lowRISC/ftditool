// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "throughput.hh"

struct ProgressBar {
  bool finished   = false;
  int width       = 100;
  int total       = 0;
  float progress  = 0;
  int last_length = 0;
  std::string label;
  std::optional<Throughput> tp;

  ProgressBar(int total, int width = 100, std::string label = "Progress")
      : total(total), label(label), width(width), finished(false) {}

  ProgressBar& with_throughput() {
    tp = std::optional<Throughput>{Throughput()};

    return *this;
  }

  void update(int progress) {
    if (this->total <= progress && this->finished) {
      return;
    }
    this->finished = (this->total <= progress);
    this->progress = static_cast<float>(progress) / this->total;
    int filled     = this->width * this->progress;

    std::string tp_str = "";
    if (tp) {
      tp->total(progress);
      tp_str = tp->render(false);
    }

    std::print("\r\033[32m{} [", label);
    for (int i = 0; i < width; ++i) {
      std::print("{}", i < filled ? "■" : " ");
    }

    std::string output =
        std::format("] {}%\033[33m {}\033[0m", (int)(this->progress * 100), tp_str);

    std::cout << output;

    if (tp) {
      auto current_length = output.length();
      if (this->last_length > current_length) {
        /* There are some trailing characters left over from the last time
         * throughput was printed. Overwrite them with spaces, and then
         * backspace back to the current string length. */
        auto n = this->last_length - current_length;
        std::cout << std::string(n, ' ') << std::string(n, '\b');
      }
      this->last_length = current_length;
    }

    std::cout << std::flush;

    if (this->finished) {
      std::cout << '\n';
    }
  }
};
