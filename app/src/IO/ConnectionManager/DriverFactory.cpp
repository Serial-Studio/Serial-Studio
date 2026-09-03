/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "IO/ConnectionManager/DriverFactory.h"

#include "IO/ConnectionManager/DriverUiRegistry.h"
#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "Misc/Diagnostics/DiagnosticsShared.h"
#include "SSAssert.h"

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
#  include "Licensing/CommercialToken.h"
#endif

//--------------------------------------------------------------------------------------------------
// Licensed construction helpers
//--------------------------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL
/**
 * @brief Builds a Pro driver once the entitlement token is valid. The tamper guard is evaluated
 *        at each bus's own case label rather than here, so every bus keeps a distinct guard
 *        dispatch site (SS_LICENSE_GUARD selects on __LINE__).
 */
template<typename Driver>
[[nodiscard]] static std::unique_ptr<IO::HAL_Driver> makeLicensed()
{
  const auto& tk = Licensing::CommercialToken::current();
  if (!tk.isValid())
    return nullptr;

  return std::make_unique<Driver>();
}

/**
 * @brief Builds a licensed PLC driver for a live source and points the UI-config instance at it so
 *        the pane and the API read live counters (spec 0073); the single session peer slot makes
 *        these listen-only buses one-live-session-per-bus (a second same-type source overwrites the
 *        first), as is the MQTT/Sparkplug peer below. Returns nullptr when the licence excludes it.
 */
template<typename Driver>
[[nodiscard]] static std::unique_ptr<IO::HAL_Driver> makePlcDriver(Driver* uiDriver)
{
  auto driver = makeLicensed<Driver>();
  if (!driver)
    return nullptr;

  auto* plc = static_cast<Driver*>(driver.get());
  plc->setPersistent(false);
  if (!uiDriver)
    return driver;

  uiDriver->setSessionPeer(plc);
  QObject::connect(
    plc, &Driver::statusChanged, uiDriver, &Driver::statusChanged, Qt::UniqueConnection);
  return driver;
}
#endif

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the factory to the UI-config registry whose instances the live listen-only
 *        drivers publish their status through.
 */
IO::DriverFactory::DriverFactory([[maybe_unused]] DriverUiRegistry& uiDrivers)
#ifdef BUILD_COMMERCIAL
  : m_uiDrivers(uiDrivers)
#endif
{}

//--------------------------------------------------------------------------------------------------
// Driver construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a fresh driver instance for the given bus @p type, or nullptr when this build
 *        or this licence does not carry that bus.
 */
std::unique_ptr<IO::HAL_Driver> IO::DriverFactory::create(SerialStudio::BusType type) const
{
  SS_ASSERT(static_cast<int>(type) >= static_cast<int>(SerialStudio::BusType::UART),
            return nullptr);

  switch (type) {
    case SerialStudio::BusType::UART:
      return std::make_unique<IO::Drivers::UART>();
    case SerialStudio::BusType::Network:
      return std::make_unique<IO::Drivers::Network>();
    case SerialStudio::BusType::BluetoothLE:
      return std::make_unique<IO::Drivers::BluetoothLE>();
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      return SS_LICENSE_GUARD() ? makeLicensed<IO::Drivers::Audio>() : nullptr;
    case SerialStudio::BusType::ModBus:
      return SS_LICENSE_GUARD() ? makeLicensed<IO::Drivers::Modbus>() : nullptr;
    case SerialStudio::BusType::CanBus:
      return SS_LICENSE_GUARD() ? makeLicensed<IO::Drivers::CANBus>() : nullptr;
    case SerialStudio::BusType::RawUsb:
      return SS_LICENSE_GUARD() ? makeLicensed<IO::Drivers::USB>() : nullptr;
    case SerialStudio::BusType::HidDevice:
      return SS_LICENSE_GUARD() ? makeLicensed<IO::Drivers::HID>() : nullptr;
    case SerialStudio::BusType::Process:
      return SS_LICENSE_GUARD() ? makeLicensed<IO::Drivers::Process>() : nullptr;
    case SerialStudio::BusType::Mqtt:
      return SS_LICENSE_GUARD() ? createMqtt() : nullptr;
    case SerialStudio::BusType::OpcUa:
      return SS_LICENSE_GUARD() ? createOpcUa() : nullptr;
    case SerialStudio::BusType::S7:
      return SS_LICENSE_GUARD() ? makePlcDriver<IO::Drivers::S7>(m_uiDrivers.s7()) : nullptr;
    case SerialStudio::BusType::EthernetIp:
      return SS_LICENSE_GUARD() ? makePlcDriver<IO::Drivers::EthernetIp>(m_uiDrivers.ethernetIp())
                                : nullptr;
    case SerialStudio::BusType::Iec104:
      return SS_LICENSE_GUARD() ? makePlcDriver<IO::Drivers::Iec104>(m_uiDrivers.iec104())
                                : nullptr;
#endif
  }

  return nullptr;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Builds the live MQTT subscriber and points the UI-config instance's Sparkplug peer slot
 *        at it, so the pane reads the session that actually carries the birth certificates.
 */
std::unique_ptr<IO::HAL_Driver> IO::DriverFactory::createMqtt() const
{
  auto driver = makeLicensed<IO::Drivers::MQTT>();
  if (!driver)
    return nullptr;

  auto* mqttUi = m_uiDrivers.mqtt();
  if (mqttUi)
    mqttUi->setSparkplugPeer(static_cast<IO::Drivers::MQTT*>(driver.get()));

  return driver;
}

/**
 * @brief Builds the live OPC UA session driver: non-persistent (only the UI instance saves its
 *        settings) and mirroring its status onto the UI-config instance the pane binds to.
 */
std::unique_ptr<IO::HAL_Driver> IO::DriverFactory::createOpcUa() const
{
  auto driver = makeLicensed<IO::Drivers::OpcUa>();
  if (!driver)
    return nullptr;

  auto* opcUa = static_cast<IO::Drivers::OpcUa*>(driver.get());
  opcUa->setPersistent(false);

  auto* opcUaUi = m_uiDrivers.opcUa();
  if (opcUaUi) {
    opcUaUi->setSessionPeer(opcUa);
    QObject::connect(opcUa,
                     &IO::Drivers::OpcUa::statusChanged,
                     opcUaUi,
                     &IO::Drivers::OpcUa::statusChanged,
                     Qt::UniqueConnection);
  }

  return driver;
}
#endif

//--------------------------------------------------------------------------------------------------
// Diagnostics taxonomy
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps @p driver onto the diagnostics bus that checks it, reporting false for a bus that
 *        has no checks of its own. A null driver is an ordinary answer, not a defect: a device
 *        whose bus this build or licence excludes never gets one.
 */
bool IO::DriverFactory::diagnosticsBus(HAL_Driver* driver, Misc::Diagnostics::Bus& bus)
{
  if (driver == nullptr)
    return false;

  SS_ASSERT_LOG(driver->metaObject() != nullptr);

  if (qobject_cast<IO::Drivers::UART*>(driver) != nullptr)
    bus = Misc::Diagnostics::Bus::Serial;
  else if (qobject_cast<IO::Drivers::Network*>(driver) != nullptr)
    bus = Misc::Diagnostics::Bus::Network;
  else if (qobject_cast<IO::Drivers::BluetoothLE*>(driver) != nullptr)
    bus = Misc::Diagnostics::Bus::Bluetooth;
#ifdef BUILD_COMMERCIAL
  else if (qobject_cast<IO::Drivers::MQTT*>(driver) != nullptr)
    bus = Misc::Diagnostics::Bus::Broker;
  else if (qobject_cast<IO::Drivers::Audio*>(driver) != nullptr)
    bus = Misc::Diagnostics::Bus::Audio;
#endif
  else
    return false;

  return true;
}
