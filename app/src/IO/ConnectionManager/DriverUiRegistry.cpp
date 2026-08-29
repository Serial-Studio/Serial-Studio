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

#include "IO/ConnectionManager/DriverUiRegistry.h"

#include <QObject>

#include "SSAssert.h"

/**
 * @brief Upper bound on the drivers a commercial build instantiates.
 */
static constexpr size_t kMaxUiDrivers = 14;

/**
 * @brief Builds one configuration-only driver per bus type available to this build.
 */
IO::DriverUiRegistry::DriverUiRegistry()
  : m_uart(std::make_unique<IO::Drivers::UART>())
  , m_network(std::make_unique<IO::Drivers::Network>())
  , m_bluetoothLE(std::make_unique<IO::Drivers::BluetoothLE>())
#ifdef BUILD_COMMERCIAL
  , m_hid(std::make_unique<IO::Drivers::HID>())
  , m_usb(std::make_unique<IO::Drivers::USB>())
  , m_mqtt(std::make_unique<IO::Drivers::MQTT>())
  , m_audio(std::make_unique<IO::Drivers::Audio>())
  , m_opcUa(std::make_unique<IO::Drivers::OpcUa>())
  , m_canBus(std::make_unique<IO::Drivers::CANBus>())
  , m_modbus(std::make_unique<IO::Drivers::Modbus>())
  , m_process(std::make_unique<IO::Drivers::Process>())
  , m_s7(std::make_unique<IO::Drivers::S7>())
  , m_ethernetIp(std::make_unique<IO::Drivers::EthernetIp>())
  , m_iec104(std::make_unique<IO::Drivers::Iec104>())
#endif
{}

/**
 * @brief Destroys whatever releaseAll() has not already released.
 */
IO::DriverUiRegistry::~DriverUiRegistry()
{
  releaseAll();
}

/**
 * @brief Destroys every driver. ConnectionManager calls this from shutdownDrivers(), after the
 *        QML engine is gone but while QApplication is alive, so driver worker threads join
 *        cleanly instead of during static destruction.
 */
void IO::DriverUiRegistry::releaseAll()
{
  m_uart.reset();
  m_network.reset();
  m_bluetoothLE.reset();
#ifdef BUILD_COMMERCIAL
  m_hid.reset();
  m_usb.reset();
  m_mqtt.reset();
  m_audio.reset();
  m_opcUa.reset();
  m_canBus.reset();
  m_modbus.reset();
  m_process.reset();
  m_s7.reset();
  m_ethernetIp.reset();
  m_iec104.reset();
#endif
}

/**
 * @brief Runs the post-singleton wiring of the drivers that need it. Only the five listed here
 *        declare the hook; the rest are fully configured by their constructors.
 */
void IO::DriverUiRegistry::setupExternalConnections()
{
  m_uart->setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  m_usb->setupExternalConnections();
  m_opcUa->setupExternalConnections();
  m_canBus->setupExternalConnections();
  m_modbus->setupExternalConnections();
#endif
}

/**
 * @brief Drops every connection running from a registry driver into @p receiver. Called from
 *        ConnectionManager's destructor: a driver outliving its receiver would otherwise
 *        deliver a configurationChanged into a half-destroyed object.
 */
void IO::DriverUiRegistry::detachFrom(QObject* receiver) const
{
  SS_ASSERT(receiver != nullptr, return);

  for (auto* driver : all())
    QObject::disconnect(driver, nullptr, receiver, nullptr);
}

/**
 * @brief Returns every driver this build instantiated, skipping the ones already released.
 */
std::vector<IO::HAL_Driver*> IO::DriverUiRegistry::all() const
{
  std::vector<HAL_Driver*> drivers;
  drivers.reserve(kMaxUiDrivers);

  const auto add = [&drivers](HAL_Driver* driver) {
    if (driver)
      drivers.push_back(driver);
  };

  add(m_uart.get());
  add(m_network.get());
  add(m_bluetoothLE.get());
#ifdef BUILD_COMMERCIAL
  add(m_hid.get());
  add(m_usb.get());
  add(m_mqtt.get());
  add(m_audio.get());
  add(m_opcUa.get());
  add(m_canBus.get());
  add(m_modbus.get());
  add(m_process.get());
  add(m_s7.get());
  add(m_ethernetIp.get());
  add(m_iec104.get());
#endif

  SS_ASSERT(drivers.size() <= kMaxUiDrivers, return drivers);
  return drivers;
}

/**
 * @brief Returns the configuration driver for @p type, or nullptr when this build has none.
 */
IO::HAL_Driver* IO::DriverUiRegistry::forBusType(SerialStudio::BusType type) const noexcept
{
  switch (type) {
    case SerialStudio::BusType::UART:
      return m_uart.get();
    case SerialStudio::BusType::Network:
      return m_network.get();
    case SerialStudio::BusType::BluetoothLE:
      return m_bluetoothLE.get();
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      return m_audio.get();
    case SerialStudio::BusType::ModBus:
      return m_modbus.get();
    case SerialStudio::BusType::CanBus:
      return m_canBus.get();
    case SerialStudio::BusType::RawUsb:
      return m_usb.get();
    case SerialStudio::BusType::HidDevice:
      return m_hid.get();
    case SerialStudio::BusType::Process:
      return m_process.get();
    case SerialStudio::BusType::Mqtt:
      return m_mqtt.get();
    case SerialStudio::BusType::OpcUa:
      return m_opcUa.get();
    case SerialStudio::BusType::S7:
      return m_s7.get();
    case SerialStudio::BusType::EthernetIp:
      return m_ethernetIp.get();
    case SerialStudio::BusType::Iec104:
      return m_iec104.get();
#endif
  }

  return nullptr;
}

//--------------------------------------------------------------------------------------------------
// Typed accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the UI-config UART driver instance.
 */
IO::Drivers::UART* IO::DriverUiRegistry::uart() const noexcept
{
  return m_uart.get();
}

/**
 * @brief Returns the UI-config Network driver instance.
 */
IO::Drivers::Network* IO::DriverUiRegistry::network() const noexcept
{
  return m_network.get();
}

/**
 * @brief Returns the UI-config BluetoothLE driver instance.
 */
IO::Drivers::BluetoothLE* IO::DriverUiRegistry::bluetoothLE() const noexcept
{
  return m_bluetoothLE.get();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns the UI-config HID driver instance.
 */
IO::Drivers::HID* IO::DriverUiRegistry::hid() const noexcept
{
  return m_hid.get();
}

/**
 * @brief Returns the UI-config USB driver instance.
 */
IO::Drivers::USB* IO::DriverUiRegistry::usb() const noexcept
{
  return m_usb.get();
}

/**
 * @brief Returns the UI-config MQTT input driver instance.
 */
IO::Drivers::MQTT* IO::DriverUiRegistry::mqtt() const noexcept
{
  return m_mqtt.get();
}

/**
 * @brief Returns the UI-config Audio driver instance.
 */
IO::Drivers::Audio* IO::DriverUiRegistry::audio() const noexcept
{
  return m_audio.get();
}

/**
 * @brief Returns the UI-config OPC UA driver instance.
 */
IO::Drivers::OpcUa* IO::DriverUiRegistry::opcUa() const noexcept
{
  return m_opcUa.get();
}

/**
 * @brief Returns the UI-config CANBus driver instance.
 */
IO::Drivers::CANBus* IO::DriverUiRegistry::canBus() const noexcept
{
  return m_canBus.get();
}

/**
 * @brief Returns the UI-config Modbus driver instance.
 */
IO::Drivers::Modbus* IO::DriverUiRegistry::modbus() const noexcept
{
  return m_modbus.get();
}

/**
 * @brief Returns the UI-config Process driver instance.
 */
IO::Drivers::Process* IO::DriverUiRegistry::process() const noexcept
{
  return m_process.get();
}

/**
 * @brief Returns the UI-config Siemens S7comm driver instance.
 */
IO::Drivers::S7* IO::DriverUiRegistry::s7() const noexcept
{
  return m_s7.get();
}

/**
 * @brief Returns the UI-config EtherNet/IP driver instance.
 */
IO::Drivers::EthernetIp* IO::DriverUiRegistry::ethernetIp() const noexcept
{
  return m_ethernetIp.get();
}

/**
 * @brief Returns the UI-config IEC 60870-5-104 driver instance.
 */
IO::Drivers::Iec104* IO::DriverUiRegistry::iec104() const noexcept
{
  return m_iec104.get();
}
#endif
