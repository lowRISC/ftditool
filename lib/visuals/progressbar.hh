// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

struct ProgressBar {
  bool finished  = false;
  int width      = 100;
  int total      = 0;
  float progress = 0;
  std::string label;

  ProgressBar(int total, int width = 100, std::string label = "Progress")
      : total(total), label(label), width(width), finished(false) {}
  void update(int progress) {
    if (this->total <= progress && this->finished) {
      return;
    }
    this->finished = (this->total <= progress);
    this->progress = static_cast<float>(progress) / this->total;
    int filled     = this->width * this->progress;

    std::print("\r\033[32m{} [", label);
    for (int i = 0; i < width; ++i) {
      std::print("{}", i < filled ? "■" : " ");
    }
    std::cout << "] " << (int)(this->progress * 100) << "%\033[0m " << std::flush;
    if (this->finished) {
      std::println("");
    }
  }
};

