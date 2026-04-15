// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <vector>

namespace ftdi {

// Detach ftdi_sio (or any kernel driver) from all FTDI USB interfaces
// using libusb. This allows the D2XX library to claim the device afterwards.
// Requires write access to the USB device files in the udev rules.
void detach_ftdi_sio(uint16_t pid);

}  // namespace ftdi
