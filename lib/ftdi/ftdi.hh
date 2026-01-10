// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ftd2xx.h"
#include <cstdint>
#include <optional>
#include <vector>
#include <iostream>
#include "device_info.hh"

namespace ftdi {

class Discovery {
 public:
  /**
   * @brief Scans for all connected FTDI devices.
   * @return A vector of devices if successful, or std::nullopt if the driver call failed.
   */
  static std::optional<std::vector<DeviceInfo>> scan() {
    DWORD num_devs = 0;

    // 1. Get the number of devices
    FT_STATUS ftStatus = FT_CreateDeviceInfoList(&num_devs);
    if (ftStatus != FT_OK) {
      // Optional: Log error to stderr here if you need to know *why* it failed
      std::cerr << "FT_CreateDeviceInfoList failed: " << ftStatus << "\n";
      return std::nullopt;
    }

    if (num_devs == 0) {
      // Success, but empty
      return std::vector<DeviceInfo>{};
    }

    // 2. Allocate buffer
    std::vector<FT_DEVICE_LIST_INFO_NODE> rawList(num_devs);

    // 3. Populate list
    ftStatus = FT_GetDeviceInfoList(rawList.data(), &num_devs);
    if (ftStatus != FT_OK) {
      return std::nullopt;
    }

    // 4. Convert to C++ structs
    std::vector<DeviceInfo> devices;
    devices.reserve(num_devs);

    for (const auto& node : rawList) {
      devices.push_back((DeviceInfo){node.Flags, static_cast<DeviceType>(node.Type), node.ID,
                                     node.LocId, std::string(node.SerialNumber),
                                     std::string(node.Description), node.ftHandle});
    }

    return std::optional<std::vector<DeviceInfo>>{devices};
  }
};

}  // namespace ftdi
