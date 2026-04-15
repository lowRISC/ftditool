// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "libusb.hh"

#include <libudev.h>
#include <libusb-1.0/libusb.h>

#include <iostream>

namespace ftdi {

enum { kFtdiVid = 0x0403 };

// Use libusb to temporarily detach ftdi_sio from every interface of
// every FTDI USB device.  The kernel driver is re-attached automatically when
// the D2XX / libft4222 handle is closed.
void detach_ftdi_sio(uint16_t pid) {
  libusb_context* ctx = nullptr;
  if (libusb_init(&ctx) != LIBUSB_SUCCESS) {
    std::cerr << "libusb_init failed\n";
    return;
  }

  libusb_device** devs = nullptr;
  ssize_t cnt          = libusb_get_device_list(ctx, &devs);
  if (cnt < 0) {
    libusb_exit(ctx);
    return;
  }

  for (ssize_t i = 0; i < cnt; i++) {
    libusb_device_descriptor desc{};
    if (libusb_get_device_descriptor(devs[i], &desc) != LIBUSB_SUCCESS) continue;
    if (desc.idVendor != kFtdiVid || desc.idProduct != pid) continue;

    libusb_device_handle* handle = nullptr;
    if (libusb_open(devs[i], &handle) != LIBUSB_SUCCESS) continue;

    libusb_config_descriptor* config = nullptr;
    if (libusb_get_active_config_descriptor(devs[i], &config) == LIBUSB_SUCCESS) {
      for (uint8_t j = 0; j < config->bNumInterfaces; j++) {
        if (libusb_kernel_driver_active(handle, j) != 1) continue;
        int ret = libusb_detach_kernel_driver(handle, j);
        if (ret != LIBUSB_SUCCESS) {
          std::cerr << "Failed to detach kernel driver on interface " << (int)j << ": "
                    << libusb_error_name(ret) << "\n";
        }
      }
      libusb_free_config_descriptor(config);
    }

    libusb_close(handle);
  }

  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);
}

}  // namespace ftdi
