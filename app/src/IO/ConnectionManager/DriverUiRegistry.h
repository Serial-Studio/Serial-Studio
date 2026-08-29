/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <vector>

#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "IO/HAL_Driver.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "IO/Drivers/CANBus.h"
#  include "IO/Drivers/EthernetIp.h"
#  include "IO/Drivers/HID.h"
#  include "IO/Drivers/Iec104.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/MQTT.h"
#  include "IO/Drivers/OpcUa.h"
#  include "IO/Drivers/Process.h"
#  include "IO/Drivers/S7.h"
#  include "IO/Drivers/USB.h"
#endif

class QObject;

namespace IO {
/**
 * @brief Owns the one configuration-only driver instance per bus type: the objects QML edits
 *        and the CLI scripts, never the ones that carry data. Live links are per-source
 *        drivers held by DeviceManager, so nothing here is ever opened, wired to a
 *        FrameReader, or touched on the pipeline thread.
 */
class DriverUiRegistry {
public:
  DriverUiRegistry();
  ~DriverUiRegistry();
  DriverUiRegistry(DriverUiRegistry&&)                 = delete;
  DriverUiRegistry(const DriverUiRegistry&)            = delete;
  DriverUiRegistry& operator=(DriverUiRegistry&&)      = delete;
  DriverUiRegistry& operator=(const DriverUiRegistry&) = delete;

  void releaseAll();
  void setupExternalConnections();
  void detachFrom(QObject* receiver) const;

  [[nodiscard]] std::vector<HAL_Driver*> all() const;
  [[nodiscard]] HAL_Driver* forBusType(SerialStudio::BusType type) const noexcept;

  [[nodiscard]] IO::Drivers::UART* uart() const noexcept;
  [[nodiscard]] IO::Drivers::Network* network() const noexcept;
  [[nodiscard]] IO::Drivers::BluetoothLE* bluetoothLE() const noexcept;
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] IO::Drivers::HID* hid() const noexcept;
  [[nodiscard]] IO::Drivers::USB* usb() const noexcept;
  [[nodiscard]] IO::Drivers::MQTT* mqtt() const noexcept;
  [[nodiscard]] IO::Drivers::Audio* audio() const noexcept;
  [[nodiscard]] IO::Drivers::OpcUa* opcUa() const noexcept;
  [[nodiscard]] IO::Drivers::CANBus* canBus() const noexcept;
  [[nodiscard]] IO::Drivers::Modbus* modbus() const noexcept;
  [[nodiscard]] IO::Drivers::Process* process() const noexcept;
  [[nodiscard]] IO::Drivers::S7* s7() const noexcept;
  [[nodiscard]] IO::Drivers::EthernetIp* ethernetIp() const noexcept;
  [[nodiscard]] IO::Drivers::Iec104* iec104() const noexcept;
#endif

private:
  std::unique_ptr<IO::Drivers::UART> m_uart;
  std::unique_ptr<IO::Drivers::Network> m_network;
  std::unique_ptr<IO::Drivers::BluetoothLE> m_bluetoothLE;
#ifdef BUILD_COMMERCIAL
  std::unique_ptr<IO::Drivers::HID> m_hid;
  std::unique_ptr<IO::Drivers::USB> m_usb;
  std::unique_ptr<IO::Drivers::MQTT> m_mqtt;
  std::unique_ptr<IO::Drivers::Audio> m_audio;
  std::unique_ptr<IO::Drivers::OpcUa> m_opcUa;
  std::unique_ptr<IO::Drivers::CANBus> m_canBus;
  std::unique_ptr<IO::Drivers::Modbus> m_modbus;
  std::unique_ptr<IO::Drivers::Process> m_process;
  std::unique_ptr<IO::Drivers::S7> m_s7;
  std::unique_ptr<IO::Drivers::EthernetIp> m_ethernetIp;
  std::unique_ptr<IO::Drivers::Iec104> m_iec104;
#endif
};
}  // namespace IO
