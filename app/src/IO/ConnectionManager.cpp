/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/ConnectionManager.h"

#include <QApplication>

#include "API/Server.h"
#include "AppState.h"
#include "Console/Handler.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "IO/FileTransmission.h"
#include "Misc/Translator.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "IO/Drivers/CANBus.h"
#  include "IO/Drivers/HID.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/Process.h"
#  include "IO/Drivers/USB.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "Misc/Utilities.h"
#  include "MQTT/Client.h"
#  include "Sessions/Export.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor, destructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ConnectionManager singleton.
 */
IO::ConnectionManager::ConnectionManager()
  : m_paused(false)
  , m_writeEnabled(true)
  , m_syncingFromProject(false)
  , m_busType(SerialStudio::BusType::UART)
  , m_startSequence("/*")
  , m_finishSequence("*/")
  , m_uartUi(std::make_unique<IO::Drivers::UART>())
  , m_networkUi(std::make_unique<IO::Drivers::Network>())
  , m_bluetoothLEUi(std::make_unique<IO::Drivers::BluetoothLE>())
#ifdef BUILD_COMMERCIAL
  , m_audioUi(std::make_unique<IO::Drivers::Audio>())
  , m_canBusUi(std::make_unique<IO::Drivers::CANBus>())
  , m_hidUi(std::make_unique<IO::Drivers::HID>())
  , m_modbusUi(std::make_unique<IO::Drivers::Modbus>())
  , m_processUi(std::make_unique<IO::Drivers::Process>())
  , m_usbUi(std::make_unique<IO::Drivers::USB>())
#endif
{
  // Re-evaluate configuration when bus type changes
  connect(this, &ConnectionManager::busTypeChanged, this, &ConnectionManager::configurationChanged);

  // Configuration changes may affect connected state
  connect(
    this, &ConnectionManager::configurationChanged, this, &ConnectionManager::connectedChanged);

  // Disconnect all devices on application quit
  connect(qApp, &QApplication::aboutToQuit, this, &ConnectionManager::disconnectAllDevices);
}

/**
 * @brief Disconnects all UI drivers and tears down active connections.
 */
IO::ConnectionManager::~ConnectionManager()
{
  for (auto* drv : {static_cast<QObject*>(m_uartUi.get()),
                    static_cast<QObject*>(m_networkUi.get()),
                    static_cast<QObject*>(m_bluetoothLEUi.get())}) {
    if (drv)
      disconnect(drv, nullptr, this, nullptr);
  }

#ifdef BUILD_COMMERCIAL
  for (auto* drv : {static_cast<QObject*>(m_audioUi.get()),
                    static_cast<QObject*>(m_canBusUi.get()),
                    static_cast<QObject*>(m_hidUi.get()),
                    static_cast<QObject*>(m_modbusUi.get()),
                    static_cast<QObject*>(m_processUi.get()),
                    static_cast<QObject*>(m_usbUi.get())}) {
    if (drv)
      disconnect(drv, nullptr, this, nullptr);
  }
#endif

  disconnectAllDevices();
}

/**
 * @brief Returns the singleton ConnectionManager instance.
 */
IO::ConnectionManager& IO::ConnectionManager::instance()
{
  static ConnectionManager singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when data streaming is paused.
 */
bool IO::ConnectionManager::paused() const noexcept
{
  return m_paused;
}

/**
 * @brief Returns true when the connection is open but writes are disabled.
 */
bool IO::ConnectionManager::readOnly() const
{
  return isConnected() && !m_writeEnabled;
}

/**
 * @brief Returns true when the connection is open and writes are enabled.
 */
bool IO::ConnectionManager::readWrite() const
{
  return isConnected() && m_writeEnabled;
}

/**
 * @brief Returns true when at least one device is currently connected.
 */
bool IO::ConnectionManager::isConnected() const
{
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile) {
    for (const auto& [id, dm] : m_devices)
      if (dm && dm->isOpen())
        return true;

    return false;
  }

  auto it = m_devices.find(0);
  return it != m_devices.end() && it->second && it->second->isOpen();
}

/**
 * @brief Returns the number of currently open devices.
 */
int IO::ConnectionManager::connectedDeviceCount() const
{
  int count = 0;
  for (const auto& [id, dm] : m_devices)
    if (dm && dm->isOpen())
      ++count;

  return count;
}

/**
 * @brief Returns true when the active connection target(s) are configured.
 */
bool IO::ConnectionManager::configurationOk() const
{
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    return projectConfigurationOk();

  auto* uiDriver = activeUiDriver();
  if (uiDriver)
    return uiDriver->configurationOk();

  return false;
}

/**
 * @brief Returns the currently selected bus type.
 */
SerialStudio::BusType IO::ConnectionManager::busType() const noexcept
{
  return m_busType;
}

/**
 * @brief Returns the configured frame start delimiter.
 */
const QByteArray& IO::ConnectionManager::startSequence() const noexcept
{
  return m_startSequence;
}

/**
 * @brief Returns the configured frame end delimiter.
 */
const QByteArray& IO::ConnectionManager::finishSequence() const noexcept
{
  return m_finishSequence;
}

/**
 * @brief Returns the name of the active checksum algorithm.
 */
const QString& IO::ConnectionManager::checksumAlgorithm() const noexcept
{
  return m_checksumAlgorithm;
}

/**
 * @brief Returns all available bus type names for the UI combo box.
 */
QStringList IO::ConnectionManager::availableBuses() const
{
  QStringList list;
  list.append(tr("UART/COM"));
  list.append(tr("Network Socket"));
  list.append(tr("Bluetooth LE"));
#ifdef BUILD_COMMERCIAL
  list.append(tr("Audio"));
  list.append(tr("Modbus"));
  list.append(tr("CAN Bus"));
  list.append(tr("USB Device"));
  list.append(tr("HID Device"));
  list.append(tr("Process"));
#endif
  return list;
}

/**
 * @brief Returns the active driver for the given device ID.
 */
IO::HAL_Driver* IO::ConnectionManager::driver(int deviceId) const
{
  auto it = m_devices.find(deviceId);
  if (it == m_devices.end())
    return nullptr;

  return it->second->driver();
}

/**
 * @brief Returns (lazily creating) a driver instance for editing source @p deviceId.
 */
IO::HAL_Driver* IO::ConnectionManager::driverForEditing(int deviceId)
{
  // Find the matching source entry
  const auto& sources             = DataModel::ProjectModel::instance().sources();
  const DataModel::Source* srcPtr = nullptr;
  for (const auto& src : sources) {
    if (src.sourceId == deviceId) {
      srcPtr = &src;
      break;
    }
  }

  if (!srcPtr)
    return nullptr;

  // Apply saved connectionSettings to the UI driver
  const auto busType = static_cast<SerialStudio::BusType>(srcPtr->busType);
  HAL_Driver* uiDrv  = uiDriverForBusType(busType);
  if (!uiDrv)
    return nullptr;

  if (!srcPtr->connectionSettings.isEmpty()) {
    m_syncingFromProject = true;
    for (auto it = srcPtr->connectionSettings.constBegin();
         it != srcPtr->connectionSettings.constEnd();
         ++it)
      uiDrv->setDriverProperty(it.key(), it.value().toVariant());

    const auto deviceIdVal = srcPtr->connectionSettings.value(QStringLiteral("deviceId"));
    if (deviceIdVal.isObject())
      uiDrv->selectByIdentifier(deviceIdVal.toObject());

    m_syncingFromProject = false;
  }

  // Start BLE discovery so the editor's device list populates
  if (busType == SerialStudio::BusType::BluetoothLE) {
    auto* ble = qobject_cast<IO::Drivers::BluetoothLE*>(uiDrv);
    if (ble && ble->deviceCount() == 0)
      ble->startDiscovery();
  }

  return uiDrv;
}

//--------------------------------------------------------------------------------------------------
// UI driver accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the UI-config UART driver instance.
 */
IO::Drivers::UART* IO::ConnectionManager::uart() const noexcept
{
  return m_uartUi.get();
}

/**
 * @brief Returns the UI-config Network driver instance.
 */
IO::Drivers::Network* IO::ConnectionManager::network() const noexcept
{
  return m_networkUi.get();
}

/**
 * @brief Returns the UI-config BluetoothLE driver instance.
 */
IO::Drivers::BluetoothLE* IO::ConnectionManager::bluetoothLE() const noexcept
{
  return m_bluetoothLEUi.get();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns the UI-config Audio driver instance.
 */
IO::Drivers::Audio* IO::ConnectionManager::audio() const noexcept
{
  return m_audioUi.get();
}

/**
 * @brief Returns the UI-config CANBus driver instance.
 */
IO::Drivers::CANBus* IO::ConnectionManager::canBus() const noexcept
{
  return m_canBusUi.get();
}

/**
 * @brief Returns the UI-config HID driver instance.
 */
IO::Drivers::HID* IO::ConnectionManager::hid() const noexcept
{
  return m_hidUi.get();
}

/**
 * @brief Returns the UI-config Modbus driver instance.
 */
IO::Drivers::Modbus* IO::ConnectionManager::modbus() const noexcept
{
  return m_modbusUi.get();
}

/**
 * @brief Returns the UI-config Process driver instance.
 */
IO::Drivers::Process* IO::ConnectionManager::process() const noexcept
{
  return m_processUi.get();
}

/**
 * @brief Returns the UI-config USB driver instance.
 */
IO::Drivers::USB* IO::ConnectionManager::usb() const noexcept
{
  return m_usbUi.get();
}
#endif

/**
 * @brief Returns the UI-config driver for the currently selected bus type.
 */
IO::HAL_Driver* IO::ConnectionManager::activeUiDriver() const noexcept
{
  switch (m_busType) {
    case SerialStudio::BusType::UART:
      return m_uartUi.get();
    case SerialStudio::BusType::Network:
      return m_networkUi.get();
    case SerialStudio::BusType::BluetoothLE:
      return m_bluetoothLEUi.get();
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      return m_audioUi.get();
    case SerialStudio::BusType::ModBus:
      return m_modbusUi.get();
    case SerialStudio::BusType::CanBus:
      return m_canBusUi.get();
    case SerialStudio::BusType::RawUsb:
      return m_usbUi.get();
    case SerialStudio::BusType::HidDevice:
      return m_hidUi.get();
    case SerialStudio::BusType::Process:
      return m_processUi.get();
#endif
    default:
      return nullptr;
  }
}

/**
 * @brief Returns the UI-config driver for a given bus type (not necessarily the active one).
 */
IO::HAL_Driver* IO::ConnectionManager::uiDriverForBusType(SerialStudio::BusType type) const noexcept
{
  switch (type) {
    case SerialStudio::BusType::UART:
      return m_uartUi.get();
    case SerialStudio::BusType::Network:
      return m_networkUi.get();
    case SerialStudio::BusType::BluetoothLE:
      return m_bluetoothLEUi.get();
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      return m_audioUi.get();
    case SerialStudio::BusType::ModBus:
      return m_modbusUi.get();
    case SerialStudio::BusType::CanBus:
      return m_canBusUi.get();
    case SerialStudio::BusType::RawUsb:
      return m_usbUi.get();
    case SerialStudio::BusType::HidDevice:
      return m_hidUi.get();
    case SerialStudio::BusType::Process:
      return m_processUi.get();
#endif
    default:
      return nullptr;
  }
}

/**
 * @brief Sets a property on the active UI-config driver and mirrors it to the live driver.
 */
void IO::ConnectionManager::setUiDriverProperty(const QString& key, const QVariant& value)
{
  HAL_Driver* uiDriver = activeUiDriver();
  if (!uiDriver)
    return;

  uiDriver->setDriverProperty(key, value);

  HAL_Driver* liveDriver = driver(0);
  if (liveDriver && liveDriver != uiDriver)
    liveDriver->setDriverProperty(key, value);
}

//--------------------------------------------------------------------------------------------------
// Data transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Feeds a pre-built payload directly into the frame processing pipeline.
 */
void IO::ConnectionManager::processPayload(const QByteArray& payload)
{
  Q_ASSERT(!payload.isEmpty());

  if (payload.isEmpty())
    return;

  // Forward payload to console, API server, and frame builder
  static auto& console      = Console::Handler::instance();
  static auto& server       = API::Server::instance();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();

  const auto captured = makeCapturedData(payload);
  server.hotpathTxData(captured->data);
  console.hotpathRxData(captured->data);
  frameBuilder.hotpathRxFrame(captured);

#ifdef BUILD_COMMERCIAL
  static auto& mqtt = MQTT::Client::instance();
  mqtt.hotpathTxFrame(payload);
#endif

#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
  grpcServer.hotpathTxData(captured->data);
#endif
}

/**
 * @brief Injects per-source payloads for multi-source playback.
 */
void IO::ConnectionManager::processMultiSourcePayload(const QByteArray& fullPayload,
                                                      const QMap<int, QByteArray>& sourcePayloads)
{
  Q_ASSERT(!fullPayload.isEmpty());
  Q_ASSERT(!sourcePayloads.isEmpty());

  if (fullPayload.isEmpty())
    return;

  // Cache singleton references for the hotpath
  static auto& console      = Console::Handler::instance();
  static auto& server       = API::Server::instance();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();

  // Forward full payload to console and API for display
  const auto captured = makeCapturedData(fullPayload);
  server.hotpathTxData(captured->data);
  console.hotpathRxData(captured->data);

  // Route each source's data through per-source frame building
  for (auto it = sourcePayloads.constBegin(); it != sourcePayloads.constEnd(); ++it)
    frameBuilder.hotpathRxSourceFrame(it.key(), makeCapturedData(it.value(), captured->timestamp));

#ifdef BUILD_COMMERCIAL
  static auto& mqtt = MQTT::Client::instance();
  mqtt.hotpathTxFrame(fullPayload);
#endif

#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
  grpcServer.hotpathTxData(captured->data);
#endif
}

/**
 * @brief Writes @p data to device 0.
 */
qint64 IO::ConnectionManager::writeData(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(m_devices.find(0) != m_devices.end());

  return writeDataToDevice(0, data);
}

/**
 * @brief Writes @p data to the specified @p deviceId.
 */
qint64 IO::ConnectionManager::writeDataToDevice(int deviceId, const QByteArray& data)
{
  Q_ASSERT(deviceId >= 0);
  Q_ASSERT(!data.isEmpty());

  auto it = m_devices.find(deviceId);
  if (it == m_devices.end() || !it->second)
    return -1;

  const qint64 bytes = it->second->write(data);
  if (bytes > 0) {
    auto writtenData          = data;
    const qint64 boundedBytes = qMin<qint64>(bytes, writtenData.size());
    writtenData.chop(writtenData.length() - boundedBytes);
    Console::Handler::instance().displaySentData(deviceId, writtenData);
  }

  return bytes;
}

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggles between connected and disconnected states for the primary device.
 */
void IO::ConnectionManager::toggleConnection()
{
  if (isConnected())
    disconnectDevice();
  else
    connectDevice();
}

/**
 * @brief Connects the primary device (device 0) and, in ProjectFile mode, all other sources.
 */
void IO::ConnectionManager::connectDevice()
{
#ifdef BUILD_COMMERCIAL
  if ((Licensing::Trial::instance().trialExpired()
       && !Licensing::CommercialToken::current().isValid())
      || !SS_LICENSE_GUARD()) {
    disconnectDevice();
    Misc::Utilities::showMessageBox(
      tr("Your trial period has ended."),
      tr("To continue using Serial Studio, please activate your license."));
    return;
  }
#endif

  connectDevice(0);

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    connectAllDevices();

  Q_EMIT connectedChanged();
}

/**
 * @brief Disconnects the primary device and any other project sources.
 */
void IO::ConnectionManager::disconnectDevice()
{
  disconnectDevice(0);

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile) {
    for (auto& [id, dm] : m_devices)
      if (id > 0)
        disconnectDevice(id);
  }

  DataModel::FrameBuilder::instance().registerQuickPlotHeaders(QStringList());

  Q_EMIT driverChanged();
  Q_EMIT connectedChanged();
}

/**
 * @brief Recreates the FrameReader for device 0 with the current settings.
 */
void IO::ConnectionManager::resetFrameReader()
{
  auto it = m_devices.find(0);
  if (it != m_devices.end() && it->second)
    it->second->reconfigure(buildFrameConfig(0));
}

/**
 * @brief Wires a UI driver's configurationChanged to persist, sync-to-live, and QML-forward
 * handlers.
 */
void IO::ConnectionManager::wireUiDriver(IO::HAL_Driver* driver)
{
  Q_ASSERT(driver != nullptr);

  connect(driver,
          &IO::HAL_Driver::configurationChanged,
          this,
          &IO::ConnectionManager::onUiDriverConfigurationChanged,
          Qt::UniqueConnection);
  connect(driver,
          &IO::HAL_Driver::configurationChanged,
          this,
          &IO::ConnectionManager::syncUiDriverToLive,
          Qt::UniqueConnection);
  connect(driver,
          &IO::HAL_Driver::configurationChanged,
          this,
          &IO::ConnectionManager::configurationChanged,
          Qt::UniqueConnection);
}

/**
 * @brief Sets up external signal/slot connections after all singletons are initialized.
 */
void IO::ConnectionManager::setupExternalConnections()
{
  // Restore persisted bus type selection
  auto savedBusType = m_settings.value("IOManager/busType", 0).toInt();
  if (savedBusType < 0 || savedBusType >= availableBuses().count())
    savedBusType = 0;

  if (!m_settings.contains("IOManager/userBusType"))
    m_settings.setValue("IOManager/userBusType", savedBusType);

  setBusType(static_cast<SerialStudio::BusType>(savedBusType));

  // Let drivers that need post-construction init run it now
  m_uartUi->setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  m_modbusUi->setupExternalConnections();
  m_canBusUi->setupExternalConnections();
  m_usbUi->setupExternalConnections();
#endif

  // Refresh bus name translations when language changes
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::ConnectionManager::busListChanged);

  // Rebuild DeviceManagers when the project's source list changes
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceStructureChanged,
          this,
          &IO::ConnectionManager::rebuildDevices,
          Qt::QueuedConnection);

  // Recreate FrameReader when frame config changes (delimiters, detection mode)
  connect(
    &AppState::instance(),
    &AppState::frameConfigChanged,
    this,
    [this](const IO::FrameConfig&) { resetFrameReader(); },
    Qt::QueuedConnection);

  // Rebuild when operation mode changes
  connect(&AppState::instance(),
          &AppState::operationModeChanged,
          this,
          &IO::ConnectionManager::rebuildDevices,
          Qt::QueuedConnection);

  // Wire UI drivers for save + sync + forward
  wireUiDriver(m_uartUi.get());
  wireUiDriver(m_networkUi.get());
  wireUiDriver(m_bluetoothLEUi.get());
#ifdef BUILD_COMMERCIAL
  wireUiDriver(m_audioUi.get());
  wireUiDriver(m_canBusUi.get());
  wireUiDriver(m_hidUi.get());
  wireUiDriver(m_modbusUi.get());
  wireUiDriver(m_processUi.get());
  wireUiDriver(m_usbUi.get());
#endif

  // Notify when device lists change so the editor refreshes
  auto clearEditing = [this]() {
    Q_EMIT deviceListRefreshed();
  };
  connect(m_uartUi.get(), &IO::Drivers::UART::availablePortsChanged, this, clearEditing);
  connect(m_bluetoothLEUi.get(), &IO::Drivers::BluetoothLE::devicesChanged, this, clearEditing);
#ifdef BUILD_COMMERCIAL
  connect(m_usbUi.get(), &IO::Drivers::USB::deviceListChanged, this, clearEditing);
  connect(m_hidUi.get(), &IO::Drivers::HID::deviceListChanged, this, clearEditing);
  connect(m_modbusUi.get(), &IO::Drivers::Modbus::availableSerialPortsChanged, this, clearEditing);
#endif
}

/**
 * @brief Connects all devices with deviceId > 0 (project sources).
 */
void IO::ConnectionManager::connectAllDevices()
{
  for (auto& [id, dm] : m_devices)
    if (id > 0)
      connectDevice(id);
}

/**
 * @brief Disconnects every registered device.
 */
void IO::ConnectionManager::disconnectAllDevices()
{
  for (auto& [id, dm] : m_devices)
    disconnectDevice(id);
}

/**
 * @brief Connects the device with the given @p deviceId.
 */
void IO::ConnectionManager::connectDevice(int deviceId)
{
  auto it = m_devices.find(deviceId);
  if (it == m_devices.end() || !it->second)
    return;

  const QIODevice::OpenMode mode = m_writeEnabled ? QIODevice::ReadWrite : QIODevice::ReadOnly;
  it->second->open(mode);
  setPaused(false);
}

/**
 * @brief Disconnects the device with the given @p deviceId.
 */
void IO::ConnectionManager::disconnectDevice(int deviceId)
{
  auto it = m_devices.find(deviceId);
  if (it != m_devices.end() && it->second)
    it->second->close();
}

/**
 * @brief Enables or disables data streaming pause (device stays connected).
 */
void IO::ConnectionManager::setPaused(bool paused)
{
  const bool effective = paused && isConnected();
  if (m_paused == effective)
    return;

  m_paused = effective;
  Q_EMIT pausedChanged();
}

/**
 * @brief Enables or disables write capability.
 */
void IO::ConnectionManager::setWriteEnabled(bool enabled)
{
  if (m_writeEnabled == enabled)
    return;

  m_writeEnabled = enabled;
  Q_EMIT writeEnabledChanged();
}

/**
 * @brief Sets the start delimiter and recreates device 0's FrameReader.
 */
void IO::ConnectionManager::setStartSequence(const QByteArray& sequence)
{
  const auto effective = sequence.isEmpty() ? QByteArray("/*") : sequence;
  if (m_startSequence == effective)
    return;

  m_startSequence = effective;
  resetFrameReader();
  Q_EMIT startSequenceChanged();
}

/**
 * @brief Sets the end delimiter and recreates device 0's FrameReader.
 */
void IO::ConnectionManager::setFinishSequence(const QByteArray& sequence)
{
  const auto effective = sequence.isEmpty() ? QByteArray("*/") : sequence;
  if (m_finishSequence == effective)
    return;

  m_finishSequence = effective;
  resetFrameReader();
  Q_EMIT finishSequenceChanged();
}

/**
 * @brief Sets the checksum algorithm and recreates device 0's FrameReader.
 */
void IO::ConnectionManager::setChecksumAlgorithm(const QString& algorithm)
{
  if (m_checksumAlgorithm == algorithm)
    return;

  m_checksumAlgorithm = algorithm;
  resetFrameReader();
  Q_EMIT checksumAlgorithmChanged();
}

/**
 * @brief Changes the bus type for the primary device, disconnecting first.
 */
void IO::ConnectionManager::setBusType(SerialStudio::BusType type)
{
  if (m_busType == type && m_devices.find(0) != m_devices.end())
    return;

  disconnectDevice(0);

  m_busType = type;
  m_settings.setValue("IOManager/busType", static_cast<int>(type));

  // Persist user's manual selection for mode-switch restore
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    m_settings.setValue("IOManager/userBusType", static_cast<int>(type));

  auto driver = createDriver(type);

  // Start BLE discovery when selected
  if (type == SerialStudio::BusType::BluetoothLE) {
    if (m_bluetoothLEUi->operatingSystemSupported())
      m_bluetoothLEUi->startDiscovery();
  }

  if (driver) {
    // Copy UI-config driver properties into the fresh live driver
    HAL_Driver* uiDriver = activeUiDriver();
    if (uiDriver)
      for (const auto& prop : uiDriver->driverProperties())
        driver->setDriverProperty(prop.key, prop.value);

    // Re-evaluate configurationOk when the live driver changes
    connect(driver.get(),
            &IO::HAL_Driver::configurationChanged,
            this,
            &IO::ConnectionManager::configurationChanged);

    // BLE uses deviceConnectedChanged instead of open/close signals
    if (type == SerialStudio::BusType::BluetoothLE) {
      auto* ble = qobject_cast<IO::Drivers::BluetoothLE*>(driver.get());
      if (ble)
        connect(ble,
                &IO::Drivers::BluetoothLE::deviceConnectedChanged,
                this,
                &IO::ConnectionManager::connectedChanged);
    }

    // Create DeviceManager and replace previous device 0
    auto dm = std::make_unique<DeviceManager>(0, std::move(driver), buildFrameConfig(0), this);
    wireDevice(dm.get());

    auto existing = m_devices.find(0);
    if (existing != m_devices.end() && existing->second)
      disconnect(existing->second.get(), nullptr, this, nullptr);

    m_devices[0] = std::move(dm);
  } else {
    // No driver for this bus type — remove device 0 entirely
    auto existing = m_devices.find(0);
    if (existing != m_devices.end()) {
      if (existing->second)
        disconnect(existing->second.get(), nullptr, this, nullptr);

      m_devices.erase(existing);
    }
  }

  Q_EMIT driverChanged();
  Q_EMIT busTypeChanged();

  // Keep source[0].busType in sync for single-source projects
  const auto opMode = AppState::instance().operationMode();
  auto& model       = DataModel::ProjectModel::instance();
  if (opMode == SerialStudio::ProjectFile && model.sources().size() == 1) {
    model.setSource0BusType(static_cast<int>(type));

    if (!model.jsonFilePath().isEmpty())
      (void)model.saveJsonFile(false);
  }
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors all properties from the active UI-config driver to the live DeviceManager driver.
 */
void IO::ConnectionManager::syncUiDriverToLive()
{
  // Re-entrancy guard: skip while syncing from project
  if (m_syncingFromProject)
    return;

  // Only sync in single-source or non-ProjectFile modes
  const auto& srcs = DataModel::ProjectModel::instance().sources();
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile && srcs.size() > 1)
    return;

  HAL_Driver* uiDriver = activeUiDriver();
  if (!uiDriver)
    return;

  HAL_Driver* liveDriver = driver(0);
  if (!liveDriver || liveDriver == uiDriver)
    return;

  for (const auto& prop : uiDriver->driverProperties())
    liveDriver->setDriverProperty(prop.key, prop.value);
}

/**
 * @brief Applies source[0]'s busType and connectionSettings to the matching UI-config driver.
 */
void IO::ConnectionManager::syncUiDriverFromSource0()
{
  const auto opMode = AppState::instance().operationMode();
  const auto& model = DataModel::ProjectModel::instance();
  const auto& srcs  = model.sources();

  if (opMode != SerialStudio::ProjectFile || srcs.size() != 1)
    return;

  // Skip unsaved projects to preserve API-configured hardware settings
  if (model.jsonFilePath().isEmpty())
    return;

  const auto& src    = srcs[0];
  const auto newType = static_cast<SerialStudio::BusType>(src.busType);

  m_syncingFromProject = true;

  if (m_busType != newType) {
    m_busType = newType;
    m_settings.setValue("IOManager/busType", static_cast<int>(newType));
    Q_EMIT busTypeChanged();
  }

  // Apply saved connection settings to the UI driver
  HAL_Driver* uiDriver = uiDriverForBusType(newType);
  if (uiDriver && !src.connectionSettings.isEmpty())
    for (auto it = src.connectionSettings.constBegin(); it != src.connectionSettings.constEnd();
         ++it)
      uiDriver->setDriverProperty(it.key(), it.value().toVariant());

  m_syncingFromProject = false;
  Q_EMIT driverChanged();
}

/**
 * @brief Connects a DeviceManager's output signals to ConnectionManager's routing slots.
 */
void IO::ConnectionManager::wireDevice(DeviceManager* dm)
{
  Q_ASSERT(dm);
  Q_ASSERT(dm->driver());

  // DirectConnection: avoid per-frame postEvent overhead on main-thread hop
  connect(dm,
          &IO::DeviceManager::frameReady,
          this,
          &IO::ConnectionManager::onFrameReady,
          Qt::DirectConnection);

  connect(dm,
          &IO::DeviceManager::rawDataReceived,
          this,
          &IO::ConnectionManager::onRawDataReceived,
          Qt::DirectConnection);
}

/**
 * @brief Captures current UI-config driver settings back to source[0].
 */
void IO::ConnectionManager::onUiDriverConfigurationChanged()
{
  // Re-entrancy guard: skip while applying project settings
  if (m_syncingFromProject)
    return;

  // Only persist in single-source ProjectFile mode
  const auto opMode = AppState::instance().operationMode();
  auto& model       = DataModel::ProjectModel::instance();

  if (opMode != SerialStudio::ProjectFile || model.sources().size() != 1)
    return;

  // Resolve the active UI driver for the current bus type
  HAL_Driver* uiDriver = activeUiDriver();
  if (!uiDriver)
    return;

  // Discard signals from non-active UI drivers (bus type mismatch)
  if (sender() && sender() != uiDriver)
    return;

  // Snapshot driver properties into a JSON object
  QJsonObject settings;
  for (const auto& prop : uiDriver->driverProperties())
    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));

  // Append stable hardware identifiers for cross-platform device matching
  const auto deviceId = uiDriver->deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  // Persist connection settings and bus type into source[0]
  model.setSource0ConnectionSettings(settings);
  model.setSource0BusType(static_cast<int>(m_busType));

  // Auto-save if a project file path exists
  if (!model.jsonFilePath().isEmpty())
    (void)model.saveJsonFile(false);
}

/**
 * @brief Rebuilds DeviceManagers for all sources when the project source list changes.
 */
void IO::ConnectionManager::rebuildDevices()
{
  // Snapshot current state before rebuilding
  const auto opMode       = AppState::instance().operationMode();
  const bool wasConnected = isConnected();

  // Only rebuild device 0 if it has saved connectionSettings
  bool willRebuildDevice0 = (opMode != SerialStudio::ProjectFile);
  if (opMode == SerialStudio::ProjectFile) {
    const auto& srcs = DataModel::ProjectModel::instance().sources();
    for (const auto& src : srcs) {
      if (src.sourceId == 0 && !src.connectionSettings.isEmpty()) {
        willRebuildDevice0 = true;
        break;
      }
    }
  }

  // Tear down devices that will be rebuilt
  for (auto it = m_devices.begin(); it != m_devices.end();) {
    const bool skipPrimary = (it->first == 0 && !willRebuildDevice0);
    if (skipPrimary) {
      ++it;
      continue;
    }

    if (it->second) {
      it->second->close();
      disconnect(it->second.get(), nullptr, this, nullptr);
    }

    it = m_devices.erase(it);
  }

  // Create DeviceManagers from project sources
  if (opMode == SerialStudio::ProjectFile) {
    const auto& sources = DataModel::ProjectModel::instance().sources();
    for (const auto& src : sources) {
      if (src.sourceId == 0 && !willRebuildDevice0)
        continue;

      auto driver = createDriver(static_cast<SerialStudio::BusType>(src.busType));
      if (!driver)
        continue;

      // Apply saved connection settings (port, baud, device index, etc.)
      if (!src.connectionSettings.isEmpty()) {
        for (auto it = src.connectionSettings.constBegin(); it != src.connectionSettings.constEnd();
             ++it)
          driver->setDriverProperty(it.key(), it.value().toVariant());

        // Match saved hardware identifiers to available devices
        const auto deviceIdVal = src.connectionSettings.value(QStringLiteral("deviceId"));
        if (deviceIdVal.isObject())
          driver->selectByIdentifier(deviceIdVal.toObject());
      }

      // Wrap driver in a DeviceManager (owns driver + thread + FrameReader)
      auto* rawDriver = driver.get();
      auto dm         = std::make_unique<DeviceManager>(
        src.sourceId, std::move(driver), buildFrameConfig(src.sourceId), this);

      connect(rawDriver,
              &IO::HAL_Driver::configurationChanged,
              this,
              &IO::ConnectionManager::configurationChanged,
              Qt::UniqueConnection);

      wireDevice(dm.get());
      m_devices[src.sourceId] = std::move(dm);
    }
  }

  // Notify QML and dependent systems that the device topology changed
  Q_EMIT configurationChanged();
  Q_EMIT driverChanged();
  Q_EMIT connectedChanged();
  Q_EMIT contextsRebuilt();

  // Sync UI drivers (ProjectFile) or restore user bus type (other modes)
  if (opMode == SerialStudio::ProjectFile) {
    QMetaObject::invokeMethod(
      this, &IO::ConnectionManager::syncUiDriverFromSource0, Qt::QueuedConnection);
  } else {
    auto userBus = m_settings.value("IOManager/userBusType", 0).toInt();
    if (userBus < 0 || userBus >= availableBuses().count())
      userBus = 0;

    const auto restored = static_cast<SerialStudio::BusType>(userBus);
    QMetaObject::invokeMethod(
      this, [this, restored] { setBusType(restored); }, Qt::QueuedConnection);
  }

  // Reconnect if we were connected before the rebuild
  if (wasConnected)
    QMetaObject::invokeMethod(this, [this] { connectDevice(); }, Qt::QueuedConnection);
}

/**
 * @brief Returns true when the current project sources are all configured.
 */
bool IO::ConnectionManager::projectConfigurationOk() const
{
  const auto& sources = DataModel::ProjectModel::instance().sources();
  if (sources.empty())
    return false;

  for (const auto& src : sources) {
    auto it = m_devices.find(src.sourceId);
    if (it == m_devices.end() || !it->second || !it->second->driver())
      return false;

    if (!it->second->driver()->configurationOk())
      return false;
  }

  return true;
}

/**
 * @brief Routes a completed frame from device @p deviceId to FrameBuilder.
 */
void IO::ConnectionManager::onFrameReady(int deviceId, const IO::CapturedDataPtr& frame)
{
  Q_ASSERT(deviceId >= 0);
  Q_ASSERT(frame);
  Q_ASSERT(frame->data);
  Q_ASSERT(!frame->data->isEmpty());

  if (m_paused)
    return;

  static auto& frameBuilder = DataModel::FrameBuilder::instance();

#ifdef BUILD_COMMERCIAL
  static auto& mqtt = MQTT::Client::instance();
  mqtt.hotpathTxFrame(*frame->data);
#endif

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    frameBuilder.hotpathRxSourceFrame(deviceId, frame);
  else
    frameBuilder.hotpathRxFrame(frame);
}

/**
 * @brief Forwards raw bytes from device @p deviceId to Console and API Server.
 */
void IO::ConnectionManager::onRawDataReceived(int deviceId, const IO::CapturedDataPtr& data)
{
  Q_ASSERT(data);
  Q_ASSERT(data->data);
  Q_ASSERT(deviceId >= 0);

  if (m_paused)
    return;

  static auto& console = Console::Handler::instance();
  static auto& server  = API::Server::instance();
  static auto& fileTx  = IO::FileTransmission::instance();

  server.hotpathTxData(data->data);
  console.hotpathRxDeviceData(deviceId, data->data);

  // Route incoming data to file transfer protocols
  if (fileTx.active()) [[unlikely]]
    fileTx.onRawDataReceived(*data->data);

#ifdef BUILD_COMMERCIAL
  static auto& sqliteExport = Sessions::Export::instance();
  sqliteExport.hotpathTxRawBytes(deviceId, data);
#endif

#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
  grpcServer.hotpathTxData(data->data);
#endif
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a FrameConfig for the given @p deviceId from current settings.
 */
IO::FrameConfig IO::ConnectionManager::buildFrameConfig(int deviceId) const
{
  const auto opMode = AppState::instance().operationMode();

  // QuickPlot/ConsoleOnly have fixed configs — use AppState
  if (opMode == SerialStudio::QuickPlot || opMode == SerialStudio::ConsoleOnly)
    return AppState::instance().frameConfig();

  // ProjectFile mode — read from source settings
  FrameConfig cfg;
  cfg.operationMode = opMode;

  const auto& sources = DataModel::ProjectModel::instance().sources();
  for (const auto& src : sources) {
    if (src.sourceId != deviceId)
      continue;

    // Extract delimiter/checksum from the source's JSON representation
    QByteArray start, end;
    QString checksum;
    DataModel::read_io_settings(start, end, checksum, DataModel::serialize(src));

    cfg.startSequences    = start.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{start};
    cfg.finishSequences   = end.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{end};
    cfg.checksumAlgorithm = checksum;
    cfg.frameDetection    = static_cast<SerialStudio::FrameDetection>(src.frameDetection);

    // Downgrade frame detection when delimiters are empty
    if ((cfg.frameDetection == SerialStudio::StartDelimiterOnly
         || cfg.frameDetection == SerialStudio::StartAndEndDelimiter)
        && cfg.startSequences.isEmpty()) [[unlikely]]
      cfg.frameDetection =
        cfg.finishSequences.isEmpty() ? SerialStudio::NoDelimiters : SerialStudio::EndDelimiterOnly;

    if (cfg.frameDetection == SerialStudio::EndDelimiterOnly && cfg.finishSequences.isEmpty())
      [[unlikely]]
      cfg.frameDetection = SerialStudio::NoDelimiters;

    return cfg;
  }

  // Fall back to defaults when no source matches
  cfg.startSequences    = {QByteArray("/*")};
  cfg.finishSequences   = {QByteArray("*/")};
  cfg.checksumAlgorithm = QString();
  cfg.frameDetection    = DataModel::ProjectModel::instance().frameDetection();
  return cfg;
}

/**
 * @brief Creates a fresh driver instance for the given bus @p type.
 */
std::unique_ptr<IO::HAL_Driver> IO::ConnectionManager::createDriver(
  SerialStudio::BusType type) const
{
  Q_ASSERT(static_cast<int>(type) >= 0);
  Q_ASSERT(static_cast<int>(type) >= static_cast<int>(SerialStudio::BusType::UART));

  switch (type) {
    case SerialStudio::BusType::UART:
      return std::make_unique<IO::Drivers::UART>();
    case SerialStudio::BusType::Network:
      return std::make_unique<IO::Drivers::Network>();
    case SerialStudio::BusType::BluetoothLE:
      return std::make_unique<IO::Drivers::BluetoothLE>();
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD()
          || tk.featureTier() < Licensing::FeatureTier::Hobbyist)
        return nullptr;

      return std::make_unique<IO::Drivers::Audio>();
    }
    case SerialStudio::BusType::ModBus: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Pro)
        return nullptr;

      return std::make_unique<IO::Drivers::Modbus>();
    }
    case SerialStudio::BusType::CanBus: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Pro)
        return nullptr;

      return std::make_unique<IO::Drivers::CANBus>();
    }
    case SerialStudio::BusType::RawUsb: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Pro)
        return nullptr;

      return std::make_unique<IO::Drivers::USB>();
    }
    case SerialStudio::BusType::HidDevice: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Pro)
        return nullptr;

      return std::make_unique<IO::Drivers::HID>();
    }
    case SerialStudio::BusType::Process: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD() || tk.featureTier() < Licensing::FeatureTier::Pro)
        return nullptr;

      return std::make_unique<IO::Drivers::Process>();
    }
#endif
    default:
      return nullptr;
  }
}
