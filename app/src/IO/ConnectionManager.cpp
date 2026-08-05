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

#include "IO/ConnectionManager.h"

#include <QApplication>
#include <QSignalBlocker>

#include "API/Server.h"
#include "AppState.h"
#include "Console/Handler.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "IO/FileTransmission.h"
#include "Misc/ConnectionDiagnostics.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SessionContext.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "IO/Drivers/CANBus.h"
#  include "IO/Drivers/HID.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/MQTT.h"
#  include "IO/Drivers/Process.h"
#  include "IO/Drivers/USB.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "MQTT/Publisher.h"
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
  , m_connectFanOut(false)
  , m_connectPending(false)
  , m_waitCursorActive(false)
  , m_lastConnectedState(false)
  , m_syncingFromProject(false)
  , m_rebuildingDevices(false)
  , m_lastConnectingState(false)
  , m_lastConnectedCount(0)
  , m_busType(SerialStudio::BusType::UART)
  , m_startSequence("/*")
  , m_finishSequence("*/")
  , m_replyCaptureArmed(false)
  , m_uartUi(std::make_unique<IO::Drivers::UART>())
  , m_networkUi(std::make_unique<IO::Drivers::Network>())
  , m_bluetoothLEUi(std::make_unique<IO::Drivers::BluetoothLE>())
#ifdef BUILD_COMMERCIAL
  , m_audioUi(std::make_unique<IO::Drivers::Audio>())
  , m_canBusUi(std::make_unique<IO::Drivers::CANBus>())
  , m_hidUi(std::make_unique<IO::Drivers::HID>())
  , m_mqttUi(std::make_unique<IO::Drivers::MQTT>())
  , m_modbusUi(std::make_unique<IO::Drivers::Modbus>())
  , m_processUi(std::make_unique<IO::Drivers::Process>())
  , m_usbUi(std::make_unique<IO::Drivers::USB>())
#endif
{
  connect(this, &ConnectionManager::busTypeChanged, this, &ConnectionManager::configurationChanged);

  m_uiDriverSaveTimer.setSingleShot(true);
  m_uiDriverSaveTimer.setInterval(750);
  connect(&m_uiDriverSaveTimer, &QTimer::timeout, this, [] {
    static auto& model = DataModel::ProjectModel::instance();
    if (model.jsonFilePath().isEmpty())
      return;

    static auto& appState = AppState::instance();
    if (appState.operationMode() != SerialStudio::ProjectFile)
      return;

    if (model.sources().size() != 1)
      return;

    (void)model.saveJsonFile(false);
  });

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
                    static_cast<QObject*>(m_mqttUi.get()),
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
 * @brief Returns this session's connection manager. The object is owned by the SessionContext and
 *        built by the composition root, so a reach before adoption is a named fatal instead of
 *        an out-of-order lazy construction (spec 0039 M2, wave D2).
 */
IO::ConnectionManager& IO::ConnectionManager::instance()
{
  return SessionContext::current().connectionManager();
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
  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    for (const auto& [id, dm] : m_devices)
      if (dm && dm->isOpen())
        return true;

    return false;
  }

  auto it = m_devices.find(0);
  return it != m_devices.end() && it->second && it->second->isOpen();
}

/**
 * @brief Returns whether the device with the given source ID is currently open.
 */
bool IO::ConnectionManager::isDeviceConnected(int deviceId) const
{
  auto it = m_devices.find(deviceId);
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
 * @brief Reports the link as connected, connecting or idle. A live session outranks a device
 *        still dialing beside it (multi-source), so connected wins over connecting.
 */
QString IO::ConnectionManager::linkState() const
{
  if (isConnected())
    return QStringLiteral("connected");

  if (anyDeviceConnecting())
    return QStringLiteral("connecting");

  return QStringLiteral("idle");
}

/**
 * @brief Sums the per-device frame-reader counters for the 1 Hz diagnostics sample. No caching and
 *        no signal: this is pulled once per second and must never be called on the frame path.
 */
IO::LinkStats IO::ConnectionManager::linkStats() const
{
  LinkStats stats{};
  for (const auto& [id, dm] : m_devices) {
    const auto* reader = dm ? dm->frameReader() : nullptr;
    if (!reader)
      continue;

    stats.bytesIn         += reader->bytesReceived();
    stats.droppedFrames   += reader->droppedFrameCount();
    stats.overflowBytes   += reader->overflowBytes();
    stats.checksumErrors  += reader->checksumErrorCount();
    stats.framesExtracted += reader->framesExtracted();
  }

  return stats;
}

/**
 * @brief Returns true when the active connection target(s) are configured.
 */
bool IO::ConnectionManager::configurationOk() const
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile)
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
  list.append(tr("MQTT Subscriber"));
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
  static auto& projectModel       = DataModel::ProjectModel::instance();
  const auto& sources             = projectModel.sources();
  const DataModel::Source* srcPtr = nullptr;
  for (const auto& src : sources) {
    if (src.sourceId == deviceId) {
      srcPtr = &src;
      break;
    }
  }

  if (!srcPtr)
    return nullptr;

  const auto busType = static_cast<SerialStudio::BusType>(srcPtr->busType);
  HAL_Driver* uiDrv  = uiDriverForBusType(busType);
  if (!uiDrv)
    return nullptr;

  if (!srcPtr->connectionSettings.isEmpty()) {
    m_syncingFromProject = true;
    uiDrv->applyConnectionSettings(srcPtr->connectionSettings);
    m_syncingFromProject = false;
  }

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

/**
 * @brief Returns the BLE driver that owns the live connection, or the UI-config instance when
 *        none is open. Project mode connects through a per-source driver, so GATT operations
 *        (writes, service/characteristic selection) must resolve this instance, not the UI one.
 */
IO::Drivers::BluetoothLE* IO::ConnectionManager::connectedBluetoothLE() const noexcept
{
  for (const auto& [deviceId, dm] : m_devices) {
    if (!dm)
      continue;

    auto* ble = qobject_cast<IO::Drivers::BluetoothLE*>(dm->driver());
    if (ble && ble->isOpen())
      return ble;
  }

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

/**
 * @brief Returns the UI-config MQTT input driver instance.
 */
IO::Drivers::MQTT* IO::ConnectionManager::mqtt() const noexcept
{
  return m_mqttUi.get();
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
    case SerialStudio::BusType::Mqtt:
      return m_mqttUi.get();
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
    case SerialStudio::BusType::Mqtt:
      return m_mqttUi.get();
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
  if (payload.isEmpty())
    return;

  static auto& console      = Console::Handler::instance();
  static auto& server       = API::Server::instance();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  static auto& appState     = AppState::instance();

  const auto captured = makeCapturedData(payload);
  server.hotpathTxData(captured->data);
  console.hotpathRxData(captured->data);
  if (appState.operationMode() == SerialStudio::ProjectFile)
    frameBuilder.hotpathRxSourceFrame(0, captured);
  else
    frameBuilder.hotpathRxFrame(captured);

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
  SS_ASSERT_LOG(!sourcePayloads.isEmpty());

  if (fullPayload.isEmpty())
    return;

  static auto& console      = Console::Handler::instance();
  static auto& server       = API::Server::instance();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();

  const auto captured = makeCapturedData(fullPayload);
  server.hotpathTxData(captured->data);
  console.hotpathRxData(captured->data);

  for (auto it = sourcePayloads.constBegin(); it != sourcePayloads.constEnd(); ++it)
    frameBuilder.hotpathRxSourceFrame(it.key(), makeCapturedData(it.value(), captured->timestamp));

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
  SS_ASSERT(!data.isEmpty(), return -1);
  SS_ASSERT_LOG(m_devices.find(0) != m_devices.end());

  return writeDataToDevice(0, data);
}

/**
 * @brief Writes @p data to the specified @p deviceId.
 */
qint64 IO::ConnectionManager::writeDataToDevice(int deviceId, const QByteArray& data)
{
  SS_ASSERT(deviceId >= 0, return -1);
  SS_ASSERT(!data.isEmpty(), return -1);

  auto it = m_devices.find(deviceId);
  if (it == m_devices.end() || !it->second)
    return -1;

  const qint64 bytes = it->second->write(data);
  if (bytes > 0) {
    auto writtenData          = data;
    const qint64 boundedBytes = qMin<qint64>(bytes, writtenData.size());
    writtenData.chop(writtenData.length() - boundedBytes);
    static auto& console = Console::Handler::instance();
    console.displaySentData(deviceId, writtenData);
  }

  return bytes;
}

/**
 * @brief Arms reply capture for @p deviceId then writes @p data, atomically on this thread so
 *        no inbound bytes can slip in between the arm and the write. Backs deviceWriteAndWait():
 *        a control-script worker marshals here, then polls pollReplyBuffer() until satisfied.
 */
qint64 IO::ConnectionManager::writeAndArmReply(int deviceId, const QByteArray& data)
{
  SS_ASSERT(deviceId >= 0, return -1);
  SS_ASSERT(!data.isEmpty(), return -1);

  {
    QMutexLocker locker(&m_replyMutex);
    m_replyBuffers.insert(deviceId, QByteArray());
  }
  m_replyCaptureArmed.store(true, std::memory_order_release);

  return writeDataToDevice(deviceId, data);
}

/**
 * @brief Returns a copy of the bytes captured for @p deviceId since the last arm.
 */
QByteArray IO::ConnectionManager::pollReplyBuffer(int deviceId) const
{
  SS_ASSERT(deviceId >= 0, return {});

  QMutexLocker locker(&m_replyMutex);
  return m_replyBuffers.value(deviceId);
}

/**
 * @brief Drops the capture buffer for @p deviceId and disarms the tap once no buffers remain,
 *        so the steady-state raw-data path pays only a single relaxed atomic read.
 */
void IO::ConnectionManager::disarmReplyCapture(int deviceId)
{
  SS_ASSERT(deviceId >= 0, return);

  QMutexLocker locker(&m_replyMutex);
  m_replyBuffers.remove(deviceId);
  if (m_replyBuffers.isEmpty())
    m_replyCaptureArmed.store(false, std::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggles between connected and disconnected states for the primary device. A request or an
 *        asynchronous dial still in flight counts as connected here, so the button aborts the
 *        attempt instead of stacking a second one on top of it.
 */
void IO::ConnectionManager::toggleConnection()
{
  if (isConnected() || m_connectPending || anyDeviceConnecting())
    disconnectDevice();
  else
    connectDevice();
}

/**
 * @brief Returns true while any device is dialing; drives the connect button's feedback so an
 *        asynchronous attempt is visible instead of looking like a dead click.
 */
bool IO::ConnectionManager::isConnecting() const
{
  return anyDeviceConnecting();
}

/**
 * @brief Returns true while any device's driver reports an in-flight dial.
 */
bool IO::ConnectionManager::anyDeviceConnecting() const
{
  for (const auto& [id, dm] : m_devices)
    if (dm && dm->driver() && dm->driver()->isConnecting())
      return true;

  return false;
}

/**
 * @brief Connects the primary device (device 0) and, in ProjectFile mode, all other sources. The
 *        request concludes when the last device stops opening, which for a synchronous driver is
 *        before the fan-out returns and for an orchestrated one is when its flow reports.
 */
void IO::ConnectionManager::connectDevice()
{
#ifdef BUILD_COMMERCIAL
  static auto& trial = Licensing::Trial::instance();
  if ((trial.trialExpired() && !Licensing::CommercialToken::current().isValid())
      || !SS_LICENSE_GUARD()) {
    disconnectDevice();
    Misc::Utilities::showMessageBox(
      tr("Your trial period has ended."),
      tr("To continue using Serial Studio, please activate your license."));
    return;
  }
#endif

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    static auto& controlScript = DataModel::ControlScript::instance();
    controlScript.runOnConnect();
  }

  m_connectPending = true;
  m_connectFanOut  = true;
  beginWaitCursor();

  connectDevice(0);

  if (appState.operationMode() == SerialStudio::ProjectFile)
    connectAllDevices();

  m_connectFanOut = false;
  concludeConnectRequest();
}

/**
 * @brief Ends a connect request once the fan-out that raised it is over: restores the cursor and
 *        publishes the connected state, no matter how many devices the request opened.
 */
void IO::ConnectionManager::concludeConnectRequest()
{
  if (!m_connectPending || m_connectFanOut)
    return;

  m_connectPending = false;
  endWaitCursor();
  notifyConnectedStateChanged();
}

/**
 * @brief Emits connectedChanged() only when the connected flag or the open-device count actually
 *        moved since the last emission. Every lifecycle path funnels through here, so callers
 *        never reason about whether some other path already reported: calling this is always
 *        correct and never produces a duplicate or contradictory notification.
 */
void IO::ConnectionManager::notifyConnectedStateChanged()
{
  const bool connecting = anyDeviceConnecting();
  if (m_lastConnectingState != connecting) {
    m_lastConnectingState = connecting;
    Q_EMIT connectingChanged();
  }

  const bool connected = isConnected();
  const int count      = connectedDeviceCount();
  if (m_lastConnectedState == connected && m_lastConnectedCount == count)
    return;

  m_lastConnectedState = connected;
  m_lastConnectedCount = count;
  Q_EMIT connectedChanged();
}

/**
 * @brief Raises the wait cursor at most once, so two overlapping requests cannot stack it.
 */
void IO::ConnectionManager::beginWaitCursor()
{
  if (m_waitCursorActive)
    return;

  m_waitCursorActive = true;
  QApplication::setOverrideCursor(Qt::WaitCursor);
}

/**
 * @brief Restores the wait cursor if this object raised it, so neither a late completion nor a
 *        cancel can leave it up or pop a cursor it does not own.
 */
void IO::ConnectionManager::endWaitCursor()
{
  if (!m_waitCursorActive)
    return;

  m_waitCursorActive = false;
  QApplication::restoreOverrideCursor();
}

/**
 * @brief Disconnects the primary device and any other project sources, settling a connect request
 *        the user gave up on. The id list is snapshotted first: a close can spin the event loop
 *        (error boxes), and a rebuild landing there would invalidate a live m_devices iterator.
 */
void IO::ConnectionManager::disconnectDevice()
{
  beginWaitCursor();

  disconnectDevice(0);

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    std::vector<int> ids;
    ids.reserve(m_devices.size());
    for (const auto& [id, dm] : m_devices)
      if (id > 0)
        ids.push_back(id);

    for (const int id : ids)
      disconnectDevice(id);
  }

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.registerQuickPlotHeaders(QStringList());

  concludeConnectRequest();
  endWaitCursor();

  Q_EMIT driverChanged();
  notifyConnectedStateChanged();

  qWarning() << "[ConnectionManager] session closed (explicit disconnect)";
  Q_EMIT sessionClosed();
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
  SS_ASSERT(driver != nullptr, return);

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
  auto savedBusType = m_settings.value("IOManager/busType", 0).toInt();
  if (savedBusType < 0 || savedBusType >= availableBuses().count())
    savedBusType = 0;

  if (!m_settings.contains("IOManager/userBusType"))
    m_settings.setValue("IOManager/userBusType", savedBusType);

  setBusType(static_cast<SerialStudio::BusType>(savedBusType));

  m_uartUi->setupExternalConnections();
#ifdef BUILD_COMMERCIAL
  m_modbusUi->setupExternalConnections();
  m_canBusUi->setupExternalConnections();
  m_usbUi->setupExternalConnections();
#endif

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::ConnectionManager::busListChanged);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceStructureChanged,
          this,
          &IO::ConnectionManager::rebuildDevices,
          Qt::DirectConnection);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceChanged,
          this,
          &IO::ConnectionManager::onProjectSourceChanged,
          Qt::DirectConnection);

  connect(
    &AppState::instance(),
    &AppState::frameConfigChanged,
    this,
    [this](const IO::FrameConfig&) { resetFrameReader(); },
    Qt::QueuedConnection);

  connect(&AppState::instance(),
          &AppState::operationModeChanged,
          this,
          &IO::ConnectionManager::rebuildDevices,
          Qt::QueuedConnection);

#ifdef BUILD_COMMERCIAL
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          &IO::ConnectionManager::rebuildDevices,
          Qt::QueuedConnection);
#endif

  wireUiDriver(m_uartUi.get());
  wireUiDriver(m_networkUi.get());
  wireUiDriver(m_bluetoothLEUi.get());
#ifdef BUILD_COMMERCIAL
  wireUiDriver(m_audioUi.get());
  wireUiDriver(m_canBusUi.get());
  wireUiDriver(m_hidUi.get());
  wireUiDriver(m_mqttUi.get());
  wireUiDriver(m_modbusUi.get());
  wireUiDriver(m_processUi.get());
  wireUiDriver(m_usbUi.get());
#endif

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
 * @brief Connects all devices with deviceId > 0 (project sources). Iterates a snapshot of the id
 *        list: an open can spin the event loop (error boxes, control scripts), and a rebuild
 *        landing there would invalidate a live iterator over m_devices.
 */
void IO::ConnectionManager::connectAllDevices()
{
  std::vector<int> ids;
  ids.reserve(m_devices.size());
  for (const auto& [id, dm] : m_devices)
    if (id > 0)
      ids.push_back(id);

  for (const int id : ids)
    connectDevice(id);
}

/**
 * @brief Disconnects every registered device, iterating a snapshot for the same reentrancy
 *        reason as connectAllDevices().
 */
void IO::ConnectionManager::disconnectAllDevices()
{
  std::vector<int> ids;
  ids.reserve(m_devices.size());
  for (const auto& [id, dm] : m_devices)
    ids.push_back(id);

  for (const int id : ids)
    disconnectDevice(id);
}

/**
 * @brief Destroys the device managers and UI drivers. main() calls this after the QML
 *        engine is destroyed (a live engine re-evaluates every driver binding against
 *        null and floods the log with TypeErrors) but while QApplication is still
 *        alive, so worker threads join cleanly instead of during static destruction.
 */
void IO::ConnectionManager::shutdownDrivers()
{
  disconnectAllDevices();

  // code-verify off
  // A driver's close() during destruction re-emits configurationChanged, which
  // re-enters isConnected(); drain the member map first so that iteration sees
  // an empty container instead of nodes being destroyed under it.
  // code-verify on
  auto devices = std::move(m_devices);
  m_devices.clear();
  devices.clear();

  m_uartUi.reset();
  m_networkUi.reset();
  m_bluetoothLEUi.reset();
#ifdef BUILD_COMMERCIAL
  m_audioUi.reset();
  m_canBusUi.reset();
  m_hidUi.reset();
  m_mqttUi.reset();
  m_modbusUi.reset();
  m_processUi.reset();
  m_usbUi.reset();
#endif
}

/**
 * @brief Connects the device with the given @p deviceId and reports the driver's verdict, which
 *        is what drives the connection diagnostics. The verdict is the open call's own return
 *        value, not `isOpen()`: a driver that dials asynchronously is not up yet when it returns.
 */
void IO::ConnectionManager::connectDevice(int deviceId)
{
  auto it = m_devices.find(deviceId);
  if (it == m_devices.end() || !it->second)
    return;

  const QIODevice::OpenMode mode = m_writeEnabled ? QIODevice::ReadWrite : QIODevice::ReadOnly;
  const bool ok                  = it->second->open(mode);
  setPaused(false);

  onDeviceOpenFinished(deviceId, ok, ok ? QString() : QStringLiteral("device did not open"));
}

/**
 * @brief Disconnects the device with the given @p deviceId. Closing cancels an open still in
 *        flight, so the pending request is settled here too: a device closed mid-open would
 *        otherwise strand the request flag and its wait cursor.
 */
void IO::ConnectionManager::disconnectDevice(int deviceId)
{
  auto it = m_devices.find(deviceId);
  if (it != m_devices.end() && it->second)
    it->second->close();

  concludeConnectRequest();
  notifyConnectedStateChanged();
}

/**
 * @brief Disconnects the source owned by @p driver, keeping other sources alive. Never emits
 *        sessionClosed: a driver drop is a link event, not the end of the user's session, and
 *        reaping the script-launched helpers here kills the very servers a retry needs.
 *        Only an explicit disconnect ends the session.
 */
void IO::ConnectionManager::disconnectDevice(HAL_Driver* driver)
{
  if (!driver)
    return;

  int deviceId = -1;
  for (const auto& [id, dm] : m_devices) {
    if (dm && dm->driver() == driver) {
      deviceId = id;
      break;
    }
  }

  if (deviceId < 0)
    return;

  qWarning() << "[ConnectionManager] device" << deviceId << "dropped ("
             << driver->metaObject()->className() << "); session continues";
  disconnectDevice(deviceId);

  if (!isConnected()) {
    static auto& frameBuilder = DataModel::FrameBuilder::instance();
    frameBuilder.registerQuickPlotHeaders(QStringList());
    Q_EMIT driverChanged();
  }
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
  static auto& appState = AppState::instance();
  static auto& model    = DataModel::ProjectModel::instance();

  if (m_busType == type && m_devices.find(0) != m_devices.end()) {
    const auto opMode = appState.operationMode();
    if (opMode == SerialStudio::ProjectFile && model.sources().size() == 1
        && model.sources()[0].busType != static_cast<int>(type))
      model.setSource0BusType(static_cast<int>(type));
    return;
  }

  disconnectDevice(0);

  m_busType = type;
  m_settings.setValue("IOManager/busType", static_cast<int>(type));

  if (appState.operationMode() != SerialStudio::ProjectFile)
    m_settings.setValue("IOManager/userBusType", static_cast<int>(type));

  auto driver = createDriver(type);

  if (type == SerialStudio::BusType::BluetoothLE) {
    if (m_bluetoothLEUi->operatingSystemSupported())
      m_bluetoothLEUi->startDiscovery();
  }

  if (driver) {
    HAL_Driver* uiDriver = activeUiDriver();
    if (uiDriver)
      for (const auto& prop : uiDriver->driverProperties())
        driver->setDriverProperty(prop.key, prop.value);

    connect(driver.get(),
            &IO::HAL_Driver::configurationChanged,
            this,
            &IO::ConnectionManager::configurationChanged);

    connect(driver.get(),
            &IO::HAL_Driver::configurationChanged,
            this,
            &IO::ConnectionManager::refreshConnectedState,
            Qt::QueuedConnection);

    if (type == SerialStudio::BusType::BluetoothLE) {
      auto* ble = qobject_cast<IO::Drivers::BluetoothLE*>(driver.get());
      if (ble)
        connect(ble,
                &IO::Drivers::BluetoothLE::gattReady,
                this,
                &IO::ConnectionManager::connectedChanged);
    }

    auto dm = std::make_unique<DeviceManager>(0, std::move(driver), buildFrameConfig(0), this);
    wireDevice(dm.get());

    auto existing = m_devices.find(0);
    if (existing != m_devices.end() && existing->second)
      disconnect(existing->second.get(), nullptr, this, nullptr);

    m_devices[0] = std::move(dm);
  } else {
    auto existing = m_devices.find(0);
    if (existing != m_devices.end()) {
      if (existing->second)
        disconnect(existing->second.get(), nullptr, this, nullptr);

      m_devices.erase(existing);
    }

    if (uiDriverForBusType(type) != nullptr) {
      QMetaObject::invokeMethod(
        this,
        [] {
          Misc::Utilities::showMessageBox(
            tr("This connection type requires an active license or trial."),
            tr("Activate Serial Studio Pro or start a trial to use this device type."));
        },
        Qt::QueuedConnection);
    }
  }

  Q_EMIT driverChanged();
  Q_EMIT busTypeChanged();

  const auto opMode = appState.operationMode();
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
 * @brief Mirrors all properties from the active UI-config driver to the live DeviceManager driver;
 * the live driver is signal-blocked to suppress its fan-out (the UI driver's configurationChanged
 * still notifies downstream, so nothing is lost).
 */
void IO::ConnectionManager::syncUiDriverToLive()
{
  if (m_syncingFromProject)
    return;

  static auto& projectModel = DataModel::ProjectModel::instance();
  static auto& appState     = AppState::instance();

  const auto& srcs = projectModel.sources();
  if (appState.operationMode() == SerialStudio::ProjectFile && srcs.size() > 1)
    return;

  HAL_Driver* uiDriver = activeUiDriver();
  if (!uiDriver)
    return;

  HAL_Driver* liveDriver = driver(0);
  if (!liveDriver || liveDriver == uiDriver)
    return;

  QSignalBlocker blocker(liveDriver);
  for (const auto& prop : uiDriver->driverProperties())
    liveDriver->setDriverProperty(prop.key, prop.value);
}

/**
 * @brief Applies source[0]'s busType and connectionSettings to the matching UI-config driver;
 * unsaved projects (empty json path) are skipped so API-configured hardware settings aren't
 * clobbered.
 */
void IO::ConnectionManager::syncUiDriverFromSource0()
{
  static auto& appState = AppState::instance();
  static auto& model    = DataModel::ProjectModel::instance();

  const auto opMode = appState.operationMode();
  const auto& srcs  = model.sources();

  if (opMode != SerialStudio::ProjectFile || srcs.size() != 1)
    return;

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

  HAL_Driver* uiDriver = uiDriverForBusType(newType);
  if (uiDriver && !src.connectionSettings.isEmpty())
    uiDriver->applyConnectionSettings(src.connectionSettings);

  m_syncingFromProject = false;
  Q_EMIT driverChanged();
}

/**
 * @brief Connects a DeviceManager's output signals to ConnectionManager's routing slots; the data
 * hops are DirectConnection to avoid per-frame postEvent overhead on the main-thread hotpath hop,
 * while the open-completion hop fires once per attempt and stays on the default. The lost-link hop
 * is queued so its teardown and modal cannot run inside the finishing flow's own emission.
 */
void IO::ConnectionManager::wireDevice(DeviceManager* dm)
{
  SS_ASSERT(dm != nullptr, return);
  SS_ASSERT_LOG(dm->driver() != nullptr);

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
 * @brief Maps the driver behind @p deviceId onto the diagnostics bus that checks it, reporting
 *        false for a bus that has no checks of its own.
 */
bool IO::ConnectionManager::diagnosticsBusFor(int deviceId, Misc::Diagnostics::Bus& bus) const
{
  HAL_Driver* halDriver = driver(deviceId);
  if (halDriver == nullptr)
    return false;

  if (qobject_cast<IO::Drivers::UART*>(halDriver) != nullptr)
    bus = Misc::Diagnostics::Bus::Serial;
  else if (qobject_cast<IO::Drivers::Network*>(halDriver) != nullptr)
    bus = Misc::Diagnostics::Bus::Network;
  else if (qobject_cast<IO::Drivers::BluetoothLE*>(halDriver) != nullptr)
    bus = Misc::Diagnostics::Bus::Bluetooth;
#ifdef BUILD_COMMERCIAL
  else if (qobject_cast<IO::Drivers::MQTT*>(halDriver) != nullptr)
    bus = Misc::Diagnostics::Bus::Broker;
  else if (qobject_cast<IO::Drivers::Audio*>(halDriver) != nullptr)
    bus = Misc::Diagnostics::Bus::Audio;
#endif
  else
    return false;

  return true;
}

/**
 * @brief Settles the pending connect request when a device finishes opening, and hands the
 *        outcome to the connection diagnostics: a failure diagnoses that bus only, a success
 *        clears what the previous failure reported. Every open reports its outcome here exactly
 *        once, and the connected state is published idempotently afterwards.
 */
void IO::ConnectionManager::onDeviceOpenFinished(int deviceId, bool ok, const QString& reason)
{
  SS_ASSERT_LOG(deviceId >= 0);
  SS_ASSERT_LOG(ok || !reason.isEmpty());

  Misc::Diagnostics::Bus bus = Misc::Diagnostics::Bus::Serial;
  if (diagnosticsBusFor(deviceId, bus)) {
    static auto& diagnostics = Misc::ConnectionDiagnostics::instance();
    if (ok)
      diagnostics.onOpenSucceeded(bus);
    else
      diagnostics.onOpenFailed(bus, reason);
  }

  concludeConnectRequest();
  notifyConnectedStateChanged();
}

/**
 * @brief Reports a connected-state transition a live driver reached on its own; BLE waits out
 *        GATT discovery and TCP/CAN dial asynchronously, so their open lands after the request
 *        settled. Queued because a driver reporting mid-open is still inside its own open().
 */
void IO::ConnectionManager::refreshConnectedState()
{
  notifyConnectedStateChanged();
}

/**
 * @brief Captures current UI-config driver settings back to source[0].
 */
void IO::ConnectionManager::onUiDriverConfigurationChanged()
{
  if (m_syncingFromProject)
    return;

  static auto& appState = AppState::instance();
  static auto& model    = DataModel::ProjectModel::instance();

  const auto opMode = appState.operationMode();
  if (opMode != SerialStudio::ProjectFile || model.sources().size() != 1)
    return;

  HAL_Driver* uiDriver = activeUiDriver();
  if (!uiDriver)
    return;

  if (sender() && sender() != uiDriver)
    return;

  QJsonObject settings;
  for (const auto& prop : uiDriver->driverProperties())
    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));

  const auto deviceId = uiDriver->deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  model.setSource0ConnectionSettings(settings);
  model.setSource0BusType(static_cast<int>(m_busType));

  if (!model.jsonFilePath().isEmpty())
    m_uiDriverSaveTimer.start();
}

/**
 * @brief Constructs a DeviceManager for one project source and stores it in m_devices.
 */
void IO::ConnectionManager::buildDeviceForSource(const DataModel::Source& src,
                                                 bool willRebuildDevice0)
{
  if (src.sourceId == 0 && !willRebuildDevice0)
    return;

  auto driver = createDriver(static_cast<SerialStudio::BusType>(src.busType));
  if (!driver)
    return;

  if (!src.connectionSettings.isEmpty())
    driver->applyConnectionSettings(src.connectionSettings);

  auto* rawDriver = driver.get();
  auto dm         = std::make_unique<DeviceManager>(
    src.sourceId, std::move(driver), buildFrameConfig(src.sourceId), this);

  connect(rawDriver,
          &IO::HAL_Driver::configurationChanged,
          this,
          &IO::ConnectionManager::configurationChanged,
          Qt::UniqueConnection);

  connect(rawDriver,
          &IO::HAL_Driver::configurationChanged,
          this,
          &IO::ConnectionManager::refreshConnectedState,
          static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));

  wireDevice(dm.get());
  m_devices[src.sourceId] = std::move(dm);
}

/**
 * @brief Rebuilds DeviceManagers for all sources when the project source list changes. Reentrant
 *        triggers coalesce into one queued follow-up rebuild. Never emits sessionClosed: a
 *        rebuild is transient churn that reconnects on its own, and ProcessLauncher reaps the
 *        script-launched helpers serving the very links being rebuilt on that signal.
 */
void IO::ConnectionManager::rebuildDevices()
{
  if (m_rebuildingDevices) {
    QMetaObject::invokeMethod(this, &IO::ConnectionManager::rebuildDevices, Qt::QueuedConnection);
    return;
  }

  m_rebuildingDevices = true;

  static auto& appState     = AppState::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();

  const auto opMode       = appState.operationMode();
  const bool wasConnected = isConnected();

  bool willRebuildDevice0 = (opMode != SerialStudio::ProjectFile);
  bool didChangeBusType   = false;
  if (opMode == SerialStudio::ProjectFile) {
    const auto& srcs = projectModel.sources();
    for (const auto& src : srcs) {
      if (src.sourceId != 0)
        continue;

      const bool missingPrimary = (m_devices.find(0) == m_devices.end());
      const bool busTypeChanged = m_busType != static_cast<SerialStudio::BusType>(src.busType);
      willRebuildDevice0 = missingPrimary || busTypeChanged || !src.connectionSettings.isEmpty();

      if (busTypeChanged && willRebuildDevice0) {
        m_busType = static_cast<SerialStudio::BusType>(src.busType);
        m_settings.setValue("IOManager/busType", static_cast<int>(m_busType));
        didChangeBusType = true;
      }

      break;
    }
  }

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

  if (opMode == SerialStudio::ProjectFile) {
    const auto& sources = projectModel.sources();
    for (const auto& src : sources)
      buildDeviceForSource(src, willRebuildDevice0);
  }

  concludeConnectRequest();

  Q_EMIT configurationChanged();
  Q_EMIT driverChanged();
  notifyConnectedStateChanged();

  Q_EMIT contextsRebuilt();

  if (didChangeBusType)
    Q_EMIT busTypeChanged();

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

  if (wasConnected)
    QMetaObject::invokeMethod(this, [this] { connectDevice(); }, Qt::QueuedConnection);

  m_rebuildingDevices = false;
}

/**
 * @brief Reconfigures a live project source when its framing settings change.
 */
void IO::ConnectionManager::onProjectSourceChanged(int sourceId)
{
  static auto& appState = AppState::instance();
  if (sourceId <= 0 || appState.operationMode() != SerialStudio::ProjectFile)
    return;

  auto it = m_devices.find(sourceId);
  if (it == m_devices.end() || !it->second)
    return;

  it->second->reconfigure(buildFrameConfig(sourceId));
  Q_EMIT configurationChanged();
}

/**
 * @brief Returns true when the current project sources are all configured.
 */
bool IO::ConnectionManager::projectConfigurationOk() const
{
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& sources       = projectModel.sources();
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
  SS_ASSERT(frame != nullptr, return);
  SS_ASSERT_LOG(deviceId >= 0);
  SS_ASSERT_LOG(!frame->data.isEmpty());

  if (m_paused)
    return;

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  static auto& appState     = AppState::instance();

  if (appState.operationMode() == SerialStudio::ProjectFile)
    frameBuilder.hotpathRxSourceFrame(deviceId, frame);
  else
    frameBuilder.hotpathRxFrame(frame);

#ifdef BUILD_COMMERCIAL
  static auto& mqttPublisher = MQTT::Publisher::instance();
  mqttPublisher.hotpathTxRawFrame(deviceId, frame);
#endif
}

/**
 * @brief Forwards raw bytes from device @p deviceId to Console and API Server.
 */
void IO::ConnectionManager::onRawDataReceived(int deviceId, const IO::CapturedDataPtr& data)
{
  SS_ASSERT(data != nullptr, return);
  SS_ASSERT_LOG(!data->data.isEmpty());
  SS_ASSERT_LOG(deviceId >= 0);

  if (m_paused)
    return;

  if (m_replyCaptureArmed.load(std::memory_order_acquire)) [[unlikely]] {
    QMutexLocker locker(&m_replyMutex);
    auto it = m_replyBuffers.find(deviceId);
    if (it != m_replyBuffers.end())
      it->append(data->data);
  }

  static auto& console = Console::Handler::instance();
  static auto& server  = API::Server::instance();
  static auto& fileTx  = IO::FileTransmission::instance();

  server.hotpathTxData(data->data);
  console.hotpathRxDeviceData(deviceId, data->data);

  if (fileTx.active()) [[unlikely]]
    fileTx.onRawDataReceived(data->data);

#ifdef BUILD_COMMERCIAL
  static auto& sqliteExport = Sessions::Export::instance();
  sqliteExport.hotpathTxRawBytes(deviceId, data);

  static auto& mqttPublisher = MQTT::Publisher::instance();
  mqttPublisher.hotpathTxRawBytes(deviceId, data);
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
  static auto& appState = AppState::instance();

  const auto opMode = appState.operationMode();
  if (opMode == SerialStudio::QuickPlot || opMode == SerialStudio::ConsoleOnly)
    return appState.frameConfig();

  FrameConfig cfg;
  cfg.operationMode = opMode;

  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& sources       = projectModel.sources();
  for (const auto& src : sources) {
    if (src.sourceId != deviceId)
      continue;

    QByteArray start, end;
    QString checksum;
    DataModel::read_io_settings(start, end, checksum, DataModel::serialize(src));

    cfg.startSequences    = start.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{start};
    cfg.finishSequences   = end.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{end};
    cfg.checksumAlgorithm = checksum;
    cfg.frameDetection    = static_cast<SerialStudio::FrameDetection>(src.frameDetection);

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

  cfg.startSequences    = {QByteArray("/*")};
  cfg.finishSequences   = {QByteArray("*/")};
  cfg.checksumAlgorithm = QString();
  cfg.frameDetection    = projectModel.frameDetection();
  return cfg;
}

/**
 * @brief Creates a fresh driver instance for the given bus @p type.
 */
std::unique_ptr<IO::HAL_Driver> IO::ConnectionManager::createDriver(
  SerialStudio::BusType type) const
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
    case SerialStudio::BusType::Audio: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD())
        return nullptr;

      return std::make_unique<IO::Drivers::Audio>();
    }
    case SerialStudio::BusType::ModBus: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD())
        return nullptr;

      return std::make_unique<IO::Drivers::Modbus>();
    }
    case SerialStudio::BusType::CanBus: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD())
        return nullptr;

      return std::make_unique<IO::Drivers::CANBus>();
    }
    case SerialStudio::BusType::RawUsb: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD())
        return nullptr;

      return std::make_unique<IO::Drivers::USB>();
    }
    case SerialStudio::BusType::HidDevice: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD())
        return nullptr;

      return std::make_unique<IO::Drivers::HID>();
    }
    case SerialStudio::BusType::Process: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD())
        return nullptr;

      return std::make_unique<IO::Drivers::Process>();
    }
    case SerialStudio::BusType::Mqtt: {
      const auto& tk = Licensing::CommercialToken::current();
      if (!tk.isValid() || !SS_LICENSE_GUARD())
        return nullptr;

      return std::make_unique<IO::Drivers::MQTT>();
    }
#endif
    default:
      return nullptr;
  }
}
