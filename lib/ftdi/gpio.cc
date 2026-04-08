// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "gpio.hh"
#include "libft4222.h"
#include "libmpsse_spi.h"
#include "ftd2xx.h"
#include <iostream>
#include <format>
#include <print>

namespace ftdi {

void Gpio::close() {
  if (mpsse) {
    SPI_CloseChannel(handle);
    Cleanup_libMPSSE();
  } else {
    FT4222_UnInitialize(handle);
    FT_Close(handle);
  }
}

embeddedpp::Result<bool> Gpio::get_pin(uint8_t pin) {
  uint8_t mask = static_cast<uint8_t>(1 << pin);
  if (mpsse) {
    if (pin > 7) {
      std::cerr << std::format("gpio pin must be 0-7\n");
      return embeddedpp::Code::InvalidArgument;
    }
    uint8_t value    = 0;
    FT_STATUS status = FT_ReadGPIO(handle, &value);
    return (value & mask) == mask;
  }

  if (pin > 3) {
    std::cerr << std::format("gpio pin must be 0-3\n");
    return embeddedpp::Code::InvalidArgument;
  }

  GPIO_Port port = static_cast<GPIO_Port>(pin);
  BOOL enable;
  FT4222_STATUS status = FT4222_GPIO_Read(handle, port, &enable);
  if (FT4222_OK != status) {
    std::cerr << std::format("FT4222_GPIO_Read: {}\n", status);
    return embeddedpp::Code::Generic;
  }
  return enable > 0;
}

embeddedpp::Status Gpio::set_pin(uint8_t pin, bool en) {
  if (mpsse) {
    if (pin > 7) {
      std::cerr << std::format("gpio pin must be 0-7\n");
      return embeddedpp::Code::InvalidArgument;
    }
    uint8_t value = 0;
    FT_STATUS status;
    status = FT_ReadGPIO(handle, &value);
    if (en) {
      value |= static_cast<uint8_t>(1 << pin);
    } else {
      value &= static_cast<uint8_t>(~(1 << pin));
    }
    status = FT_WriteGPIO(handle, 0xff, value);
    if (FT_OK != status) {
      std::cerr << std::format("FT_WriteGPIO: {}\n", status);
      return embeddedpp::Code::Generic;
    }
    return true;
  }

  if (pin > 3) {
    std::cerr << std::format("gpio pin must be 0-3\n");
    return embeddedpp::Code::InvalidArgument;
  }

  GPIO_Port port       = static_cast<GPIO_Port>(pin);
  FT4222_STATUS status = FT4222_GPIO_Write(handle, port, en ? TRUE : FALSE);
  if (FT4222_OK != status) {
    std::cerr << std::format("FT4222_GPIO_Write: {}\n", status);
    return embeddedpp::Code::Generic;
  }
  return embeddedpp::Code::Ok;
}

static std::optional<Gpio> new_ft4222_gpio(DeviceInfo& device) {
  FT_HANDLE handle;
  FT_STATUS res = FT_OpenEx((PVOID)(uintptr_t)device.loc_id, FT_OPEN_BY_LOCATION, &handle);
  if (res != FT_OK) {
    std::cerr << std::format("Gpio Open:{}\n", res);
    return std::nullopt;
  }

  // Initialize all four GPIO pins as outputs.
  GPIO_Dir dir[4]      = {GPIO_OUTPUT, GPIO_OUTPUT, GPIO_OUTPUT, GPIO_OUTPUT};
  FT4222_STATUS status = FT4222_GPIO_Init(handle, dir);
  if (FT4222_OK != status) {
    std::cerr << std::format("FT4222_GPIO_Init:{}\n", status);
    FT_Close(handle);
    return std::nullopt;
  }

  return std::optional<Gpio>{handle};
}

static std::optional<Gpio> new_ft2232_gpio(DeviceInfo& device) {
  FT_HANDLE handle;
  FT_STATUS res;
  const uint32_t channel = 0;
  Init_libMPSSE();

  std::println("Opening {}, channel {}", device.serial_number, channel);
  res = SPI_OpenChannel(channel, &handle);
  if (res != FT_OK) {
    std::cerr << std::format("Gpio Open:{}\n", res);
    Cleanup_libMPSSE();
    return std::nullopt;
  }

  ChannelConfig config = {.ClockRate     = 1000000,  // 1MHz
                          .LatencyTimer  = 2,
                          .configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 |
                                           SPI_CONFIG_OPTION_CS_ACTIVELOW};
  res                  = SPI_InitChannel(handle, &config);
  if (res != FT_OK) {
    std::cerr << std::format("Spi InitChannel: {}\n", res);
    return std::nullopt;
  }

  return std::optional<Gpio>{Gpio(handle, true)};
}

std::optional<Gpio> Gpio::from_device_info(DeviceInfo& device) {
  switch (device.type) {
    case DeviceType::Ftdi_2232h:
      return new_ft2232_gpio(device);
    case DeviceType::Ftdi_4222h_0:
    case DeviceType::Ftdi_4222h_1_2:
    case DeviceType::Ftdi_4222h_3:
      return new_ft4222_gpio(device);
    default:
      break;
  }
  std::cerr << std::format("Gpio: device not supported {}\n", device);
  return std::nullopt;
}

};  // namespace ftdi
