// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "throughput.hh"

struct ProgressBar {
  bool finished  = false;
  int width      = 100;
  int total      = 0;
  float progress = 0;
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
      tp_str = tp->render();
    }

    std::print("\r\033[32m{} [", label);
    for (int i = 0; i < width; ++i) {
      std::print("{}", i < filled ? "■" : " ");
    }
    std::cout << "] " << (int)(this->progress * 100) << "%\033[33m " << tp_str << "\033[0m "
              << std::flush;
    if (this->finished) {
      std::println("");
    }
  }
};
