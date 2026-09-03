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
#include <QThread>

#include "API/Server.h"
#include "AppState.h"
#include "Console/Handler.h"
#include "CSV/Export.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "IO/Drivers/BluetoothLE.h"
#include "IO/Drivers/UART.h"
#include "IO/FileTransmission.h"
#include "MDF4/Export.h"
#include "Misc/ConnectionDiagnostics.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SessionContext.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "IO/Drivers/HID.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/USB.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "MQTT/Publisher.h"
#  include "Sessions/Export.h"
#  include "UI/Widgets/AudioExport.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor, destructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ConnectionManager singleton. AppState, ProjectModel and FrameBuilder are
 *        captured here rather than in setupExternalConnections(): the pinned order builds all
 *        three ahead of this object, and the spec-0044 headless bootstrap runs the pinned order
 *        WITHOUT any wiring pass, yet still asks this object for a FrameConfig.
 */
IO::ConnectionManager::ConnectionManager()
  : m_paused(false)
  , m_writeEnabled(true)
  , m_rebuildingDevices(false)
  , m_busType(SerialStudio::BusType::UART)
  , m_appState(AppState::instance())
  , m_frameBuilder(DataModel::FrameBuilder::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_apiServer(nullptr)
  , m_console(nullptr)
  , m_fileTransmission(nullptr)
#ifdef BUILD_COMMERCIAL
  , m_mqttPublisher(nullptr)
  , m_sessionExport(nullptr)
#endif
#ifdef ENABLE_GRPC
  , m_grpcServer(nullptr)
#endif
  , m_io(m_appState,
         m_frameBuilder,
         m_replyCapture,
         m_devices,
         m_paused,
         m_console,
         m_apiServer,
         m_fileTransmission
#ifdef BUILD_COMMERCIAL
         ,
         m_sessionExport,
         m_mqttPublisher
#endif
#ifdef ENABLE_GRPC
         ,
         m_grpcServer
#endif
         )
  , m_query(m_devices, m_projectModel)
  , m_driverFactory(m_uiDrivers)
  , m_streamConfigs(m_appState, m_projectModel)
  , m_streamPool(m_frameBuilder, m_appState, m_streamConfigs)
  , m_uiSync(m_uiDrivers, m_appState, m_projectModel)
{
  connect(this, &ConnectionManager::busTypeChanged, this, &ConnectionManager::configurationChanged);

  m_uiDriverSaveTimer.setSingleShot(true);
  m_uiDriverSaveTimer.setInterval(750);
  connect(&m_uiDriverSaveTimer, &QTimer::timeout, this, [this] { m_uiSync.autosaveSource0(); });

  connect(qApp, &QApplication::aboutToQuit, this, &ConnectionManager::disconnectAllDevices);
}

/**
 * @brief Disconnects all UI drivers and tears down active connections.
 */
IO::ConnectionManager::~ConnectionManager()
{
  m_uiDrivers.detachFrom(this);
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
  if (m_appState.operationMode() == SerialStudio::ProjectFile)
    return m_query.anyOpen();

  return m_query.primaryOpen();
}

/**
 * @brief Returns whether the device with the given source ID is currently open.
 */
bool IO::ConnectionManager::isDeviceConnected(int deviceId) const
{
  return m_query.isDeviceConnected(deviceId);
}

/**
 * @brief Returns the number of currently open devices.
 */
int IO::ConnectionManager::connectedDeviceCount() const
{
  return m_query.connectedDeviceCount();
}

/**
 * @brief Reports the link as connected, connecting or idle. A live session outranks a device
 *        still dialing beside it (multi-source), so connected wins over connecting.
 */
QString IO::ConnectionManager::linkState() const
{
  return DeviceTableQuery::linkState(isConnected(), anyDeviceConnecting());
}

/**
 * @brief Sums the per-device frame-reader counters for the 1 Hz diagnostics sample. No caching and
 *        no signal: this is pulled once per second and must never be called on the frame path.
 */
IO::LinkStats IO::ConnectionManager::linkStats() const
{
  return m_query.linkStats();
}

/**
 * @brief Returns true when the active connection target(s) are configured.
 */
bool IO::ConnectionManager::configurationOk() const
{
  if (m_appState.operationMode() == SerialStudio::ProjectFile)
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
  return m_io.startSequence();
}

/**
 * @brief Returns the configured frame end delimiter.
 */
const QByteArray& IO::ConnectionManager::finishSequence() const noexcept
{
  return m_io.finishSequence();
}

/**
 * @brief Returns the name of the active checksum algorithm.
 */
const QString& IO::ConnectionManager::checksumAlgorithm() const noexcept
{
  return m_io.checksumAlgorithm();
}

/**
 * @brief Returns all available bus type names for the UI combo box.
 */
QStringList IO::ConnectionManager::availableBuses() const
{
  QStringList list{tr("UART/COM"), tr("Network"), tr("Bluetooth LE")};
#ifdef BUILD_COMMERCIAL
  list << tr("Audio") << tr("Modbus") << tr("CAN Bus") << tr("USB Device") << tr("HID Device")
       << tr("Process") << tr("MQTT Subscriber") << tr("OPC UA") << tr("Siemens S7")
       << tr("EtherNet/IP") << tr("IEC 60870-5-104");
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
 * @brief Returns (lazily configuring) the UI-config driver instance that edits source
 *        @p deviceId; the project-to-driver fence lives with the rest of the settings mirror.
 */
IO::HAL_Driver* IO::ConnectionManager::driverForEditing(int deviceId)
{
  return m_uiSync.driverForEditing(deviceId);
}

//--------------------------------------------------------------------------------------------------
// UI driver accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the UI-config UART driver instance.
 */
IO::Drivers::UART* IO::ConnectionManager::uart() const noexcept
{
  return m_uiDrivers.uart();
}

/**
 * @brief Returns the UI-config Network driver instance.
 */
IO::Drivers::Network* IO::ConnectionManager::network() const noexcept
{
  return m_uiDrivers.network();
}

/**
 * @brief Returns the UI-config BluetoothLE driver instance.
 */
IO::Drivers::BluetoothLE* IO::ConnectionManager::bluetoothLE() const noexcept
{
  return m_uiDrivers.bluetoothLE();
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

  return m_uiDrivers.bluetoothLE();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns the UI-config Audio driver instance.
 */
IO::Drivers::Audio* IO::ConnectionManager::audio() const noexcept
{
  return m_uiDrivers.audio();
}

/**
 * @brief Returns the UI-config CANBus driver instance.
 */
IO::Drivers::CANBus* IO::ConnectionManager::canBus() const noexcept
{
  return m_uiDrivers.canBus();
}

/**
 * @brief Returns the UI-config HID driver instance.
 */
IO::Drivers::HID* IO::ConnectionManager::hid() const noexcept
{
  return m_uiDrivers.hid();
}

/**
 * @brief Returns the UI-config Modbus driver instance.
 */
IO::Drivers::Modbus* IO::ConnectionManager::modbus() const noexcept
{
  return m_uiDrivers.modbus();
}

/**
 * @brief Returns the UI-config OPC UA driver instance.
 */
IO::Drivers::OpcUa* IO::ConnectionManager::opcUa() const noexcept
{
  return m_uiDrivers.opcUa();
}

/**
 * @brief Returns the UI-config Process driver instance.
 */
IO::Drivers::Process* IO::ConnectionManager::process() const noexcept
{
  return m_uiDrivers.process();
}

/**
 * @brief Returns the UI-config Siemens S7comm driver instance.
 */
IO::Drivers::S7* IO::ConnectionManager::s7() const noexcept
{
  return m_uiDrivers.s7();
}

/**
 * @brief Returns the UI-config EtherNet/IP driver instance.
 */
IO::Drivers::EthernetIp* IO::ConnectionManager::ethernetIp() const noexcept
{
  return m_uiDrivers.ethernetIp();
}

/**
 * @brief Returns the UI-config IEC 60870-5-104 driver instance.
 */
IO::Drivers::Iec104* IO::ConnectionManager::iec104() const noexcept
{
  return m_uiDrivers.iec104();
}

/**
 * @brief Returns the UI-config USB driver instance.
 */
IO::Drivers::USB* IO::ConnectionManager::usb() const noexcept
{
  return m_uiDrivers.usb();
}

/**
 * @brief Returns the UI-config MQTT input driver instance.
 */
IO::Drivers::MQTT* IO::ConnectionManager::mqtt() const noexcept
{
  return m_uiDrivers.mqtt();
}
#endif

/**
 * @brief Returns the UI-config driver for the currently selected bus type.
 */
IO::HAL_Driver* IO::ConnectionManager::activeUiDriver() const noexcept
{
  return m_uiDrivers.forBusType(m_busType);
}

/**
 * @brief Returns the UI-config driver for a given bus type (not necessarily the active one).
 */
IO::HAL_Driver* IO::ConnectionManager::uiDriverForBusType(SerialStudio::BusType type) const noexcept
{
  return m_uiDrivers.forBusType(type);
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
 * @brief Feeds a pre-built payload into the frame pipeline.
 */
void IO::ConnectionManager::processPayload(const QByteArray& payload)
{
  m_io.processPayload(payload);
}

/**
 * @brief Injects per-source payloads for multi-source playback.
 */
void IO::ConnectionManager::processMultiSourcePayload(const QByteArray& fullPayload,
                                                      const QMap<int, QByteArray>& sourcePayloads)
{
  m_io.processMultiSourcePayload(fullPayload, sourcePayloads);
}

/**
 * @brief Writes @p data to device 0.
 */
qint64 IO::ConnectionManager::writeData(const QByteArray& data)
{
  SS_ASSERT(!data.isEmpty(), return -1);
  SS_ASSERT_LOG(m_devices.find(0) != m_devices.end());

  return m_io.writeToDevice(0, data);
}

/**
 * @brief Writes @p data to the specified @p deviceId.
 */
qint64 IO::ConnectionManager::writeDataToDevice(int deviceId, const QByteArray& data)
{
  return m_io.writeToDevice(deviceId, data);
}

/**
 * @brief Arms reply capture for @p deviceId then writes @p data, atomically on this thread.
 */
qint64 IO::ConnectionManager::writeAndArmReply(int deviceId, const QByteArray& data)
{
  return m_io.writeAndArmReply(deviceId, data);
}

/**
 * @brief Returns a copy of the bytes captured for @p deviceId since the last arm.
 */
QByteArray IO::ConnectionManager::pollReplyBuffer(int deviceId) const
{
  return m_io.pollReplyBuffer(deviceId);
}

/**
 * @brief Drops the capture buffer for @p deviceId, disarming the tap once no buffers remain.
 */
void IO::ConnectionManager::disarmReplyCapture(int deviceId)
{
  m_io.disarmReplyCapture(deviceId);
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
  SS_ASSERT_LOG(thread() == QThread::currentThread());

  if (isConnected() || m_fanOut.requestPending() || anyDeviceConnecting())
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
  return m_query.anyDeviceConnecting();
}

/**
 * @brief Connects device 0 and, in ProjectFile mode, all other sources. The request concludes
 *        when the last device stops opening: before the fan-out returns for a driver that opens
 *        synchronously, on its openFinished verdict for one that dials. Stream workers rebuild so
 *        their config captures the settings the session opens with, not the last bus switch's.
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

  if (!isConnected())
    rebuildStreamWorkers();

  if (m_appState.operationMode() == SerialStudio::ProjectFile) {
    static auto& controlScript = DataModel::ControlScript::instance();
    controlScript.runOnConnect();
  }

  m_fanOut.beginRequest();
  m_fanOut.beginWaitCursor();

  connectDevice(0);

  if (m_appState.operationMode() == SerialStudio::ProjectFile)
    connectAllDevices();

  m_fanOut.endFanOut();
  concludeConnectRequest();
}

/**
 * @brief Ends a connect request once the fan-out that raised it is over: restores the cursor and
 *        publishes the connected state, no matter how many devices the request opened.
 */
void IO::ConnectionManager::concludeConnectRequest()
{
  if (!m_fanOut.concludeRequest())
    return;

  m_fanOut.endWaitCursor();
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
  if (m_fanOut.noteConnecting(anyDeviceConnecting()))
    Q_EMIT connectingChanged();

  if (m_fanOut.noteConnected(isConnected(), connectedDeviceCount()))
    Q_EMIT connectedChanged();
}

/**
 * @brief Settles the pending verdict a driver reported through openFinished(): the id is erased
 *        before reporting so the notify hop inside onDeviceOpenFinished cannot re-enter, and a
 *        failed dial tears the device down quietly (never sessionClosed: helpers survive a
 *        failed attempt). A report with no pending id (user already cancelled) is ignored.
 */
void IO::ConnectionManager::onDriverOpenFinished(bool ok, const QString& reason)
{
  SS_ASSERT_LOG(sender() != nullptr);

  auto* halDriver = qobject_cast<HAL_Driver*>(sender());
  if (halDriver == nullptr)
    return;

  SS_ASSERT_LOG(!halDriver->openReportArmed());

  const int deviceId = deviceIdForDriver(halDriver);
  if (deviceId < 0 || !m_fanOut.takePendingDial(deviceId))
    return;

  if (!ok)
    disconnectDevice(deviceId);

  onDeviceOpenFinished(
    deviceId, ok, ok ? QString() : (reason.isEmpty() ? tr("connection attempt failed") : reason));
}

/**
 * @brief Disconnects the primary device and any other project sources, settling a connect request
 *        the user gave up on. The id list is snapshotted first: a close can spin the event loop.
 *        sessionClosed() is emitted only when a session existed: cancelling a dial ends none, and
 *        ProcessLauncher reaps the helpers the next attempt needs on that signal.
 */
void IO::ConnectionManager::disconnectDevice()
{
  SS_ASSERT_LOG(thread() == QThread::currentThread());

  const bool hadSession = isConnected();

  m_fanOut.beginWaitCursor();

  disconnectDevice(0);

  if (m_appState.operationMode() == SerialStudio::ProjectFile)
    for (const int id : deviceIdSnapshot(true))
      disconnectDevice(id);

  m_frameBuilder.registerQuickPlotHeaders(QStringList());

  concludeConnectRequest();
  m_fanOut.endWaitCursor();

  Q_EMIT driverChanged();
  notifyConnectedStateChanged();

  if (hadSession)
    Q_EMIT sessionClosed();
}

/**
 * @brief Recreates the FrameReader for device 0 with the current settings.
 */
void IO::ConnectionManager::resetFrameReader()
{
  auto it = m_devices.find(0);
  if (it != m_devices.end() && it->second)
    it->second->reconfigure(m_streamConfigs.frameConfig(0));
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
 * @brief Sets up external signal/slot connections after all singletons are initialized. The
 *        modules built AFTER this one in the pinned order are captured first, before setBusType()
 *        can create the primary device whose taps read them.
 */
void IO::ConnectionManager::setupExternalConnections()
{
  m_console          = &Console::Handler::instance();
  m_apiServer        = &API::Server::instance();
  m_fileTransmission = &IO::FileTransmission::instance();
#ifdef BUILD_COMMERCIAL
  m_mqttPublisher = &MQTT::Publisher::instance();
  m_sessionExport = &Sessions::Export::instance();
#endif
#ifdef ENABLE_GRPC
  m_grpcServer = &API::GRPC::GRPCServer::instance();
#endif

  auto savedBusType = m_settings.value("IOManager/busType", 0).toInt();
  if (savedBusType < 0 || savedBusType >= availableBuses().count())
    savedBusType = 0;

  if (!m_settings.contains("IOManager/userBusType"))
    m_settings.setValue("IOManager/userBusType", savedBusType);

  setBusType(static_cast<SerialStudio::BusType>(savedBusType));

  m_uiDrivers.setupExternalConnections();

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::ConnectionManager::busListChanged);

  connect(&m_projectModel,
          &DataModel::ProjectModel::sourceStructureChanged,
          this,
          &IO::ConnectionManager::rebuildDevices,
          Qt::DirectConnection);

  connect(&m_projectModel,
          &DataModel::ProjectModel::sourceChanged,
          this,
          &IO::ConnectionManager::onProjectSourceChanged,
          Qt::DirectConnection);

  wireStreamLifecycle();

  connect(
    &m_appState,
    &AppState::frameConfigChanged,
    this,
    [this](const IO::FrameConfig&) { resetFrameReader(); },
    Qt::QueuedConnection);

  connect(&m_appState,
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

  for (auto* driver : m_uiDrivers.all())
    wireUiDriver(driver);

  auto clearEditing = [this]() {
    Q_EMIT deviceListRefreshed();
  };
  connect(m_uiDrivers.uart(), &IO::Drivers::UART::availablePortsChanged, this, clearEditing);
  connect(m_uiDrivers.bluetoothLE(), &IO::Drivers::BluetoothLE::devicesChanged, this, clearEditing);
#ifdef BUILD_COMMERCIAL
  connect(m_uiDrivers.usb(), &IO::Drivers::USB::deviceListChanged, this, clearEditing);
  connect(m_uiDrivers.hid(), &IO::Drivers::HID::deviceListChanged, this, clearEditing);
  connect(
    m_uiDrivers.modbus(), &IO::Drivers::Modbus::availableSerialPortsChanged, this, clearEditing);

  connect(this, &IO::ConnectionManager::connectedChanged, this, [this] {
    const bool paused = isConnected();
    m_uiDrivers.hid()->setDiscoveryPaused(paused);
    m_uiDrivers.audio()->setDiscoveryPaused(paused);
  });
#endif
}

/**
 * @brief Connects all devices with deviceId > 0 (project sources). Iterates a snapshot of the id
 *        list: an open can spin the event loop (error boxes, control scripts), and a rebuild
 *        landing there would invalidate a live iterator over m_devices.
 */
void IO::ConnectionManager::connectAllDevices()
{
  for (const int id : deviceIdSnapshot(true))
    connectDevice(id);
}

/**
 * @brief Disconnects every registered device, iterating a snapshot for the same reentrancy
 *        reason as connectAllDevices().
 */
void IO::ConnectionManager::disconnectAllDevices()
{
  for (const int id : deviceIdSnapshot(false))
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
  stopStreamWorkers();
  disconnectAllDevices();

  // code-verify off
  // A driver's close() during destruction re-emits configurationChanged, which
  // re-enters isConnected(); drain the member map first so that iteration sees
  // an empty container instead of nodes being destroyed under it.
  // code-verify on
  auto devices = std::move(m_devices);
  m_devices.clear();
  devices.clear();

  m_uiDrivers.releaseAll();
}

/**
 * @brief Connects the device with the given @p deviceId and reports the driver's verdict, which
 *        is what drives the connection diagnostics. A driver still dialing when open() returns
 *        keeps its verdict pending; the driver reports it exactly once through openFinished()
 *        (the latch is armed here and disarmed for attempts that settle synchronously).
 */
void IO::ConnectionManager::connectDevice(int deviceId)
{
  openDevice(deviceId, ResumePolicy::Resume);
}

/**
 * @brief Opens @p deviceId under an explicit resume policy: the verdict handling is identical for
 *        both, only a user-initiated connect lifts the session pause.
 */
void IO::ConnectionManager::openDevice(int deviceId, ResumePolicy policy)
{
  SS_ASSERT(deviceId >= 0, return);

  auto it = m_devices.find(deviceId);
  if (it == m_devices.end() || !it->second)
    return;

  HAL_Driver* halDriver = driver(deviceId);
  if (halDriver)
    halDriver->armOpenReport();

  const QIODevice::OpenMode mode = m_writeEnabled ? QIODevice::ReadWrite : QIODevice::ReadOnly;
  const bool ok                  = it->second->open(mode);
  if (policy == ResumePolicy::Resume)
    setPaused(false);

  if (ok && halDriver && halDriver->isConnecting()) {
    m_fanOut.notePendingDial(deviceId);
    concludeConnectRequest();
    notifyConnectedStateChanged();
    return;
  }

  if (halDriver)
    halDriver->disarmOpenReport();

  onDeviceOpenFinished(deviceId, ok, ok ? QString() : QStringLiteral("device did not open"));
}

/**
 * @brief Disconnects the device with the given @p deviceId. Closing cancels an open still in
 *        flight, so the pending request is settled here too: a device closed mid-open would
 *        otherwise strand the request flag and its wait cursor. A pending dial verdict is
 *        dropped without a report: a user cancel is not an open failure.
 */
void IO::ConnectionManager::disconnectDevice(int deviceId)
{
  SS_ASSERT(deviceId >= 0, return);

  (void)m_fanOut.takePendingDial(deviceId);

  HAL_Driver* halDriver = driver(deviceId);
  if (halDriver)
    halDriver->disarmOpenReport();

  auto it = m_devices.find(deviceId);
  if (it != m_devices.end() && it->second)
    it->second->close();

  concludeConnectRequest();
  notifyConnectedStateChanged();
}

/**
 * @brief Reopens only the device @p driver backs. The counterpart of disconnectDevice(driver), and
 *        what a driver's own recovery must use: the argument-less connectDevice() is the user's
 *        whole-session fan-out, which would redial every other device and re-run the control
 *        script's onConnect on a single link's reappearance, and it keeps the session pause.
 */
void IO::ConnectionManager::connectDevice(HAL_Driver* driver)
{
  const int deviceId = deviceIdForDriver(driver);
  if (deviceId >= 0)
    openDevice(deviceId, ResumePolicy::KeepPause);
}

/**
 * @brief Disconnects the source owned by @p driver, keeping other sources alive. Never emits
 *        sessionClosed: a driver drop is a link event, not the end of the user's session, and
 *        reaping the script-launched helpers here kills the very servers a retry needs.
 *        Only an explicit disconnect ends the session.
 */
void IO::ConnectionManager::disconnectDevice(HAL_Driver* driver)
{
  const int deviceId = deviceIdForDriver(driver);
  if (deviceId < 0)
    return;

  if (m_fanOut.takePendingDial(deviceId))
    onDeviceOpenFinished(deviceId, false, QStringLiteral("connection attempt failed"));

  qWarning() << "[ConnectionManager] device" << deviceId << "dropped ("
             << driver->metaObject()->className() << "); session continues";
  disconnectDevice(deviceId);

  if (!isConnected()) {
    m_frameBuilder.registerQuickPlotHeaders(QStringList());
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
  m_streamPool.setPaused(effective);

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
  if (!m_io.setStartSequence(sequence))
    return;

  resetFrameReader();
  Q_EMIT startSequenceChanged();
}

/**
 * @brief Sets the end delimiter and recreates device 0's FrameReader.
 */
void IO::ConnectionManager::setFinishSequence(const QByteArray& sequence)
{
  if (!m_io.setFinishSequence(sequence))
    return;

  resetFrameReader();
  Q_EMIT finishSequenceChanged();
}

/**
 * @brief Sets the checksum algorithm and recreates device 0's FrameReader.
 */
void IO::ConnectionManager::setChecksumAlgorithm(const QString& algorithm)
{
  if (!m_io.setChecksumAlgorithm(algorithm))
    return;

  resetFrameReader();
  Q_EMIT checksumAlgorithmChanged();
}

/**
 * @brief Changes the bus type for the primary device, disconnecting first. The replaced device
 *        manager is released and deleteLater()'d rather than destroyed here: a driver error box
 *        pumps the event loop, so the swap can land mid socket notification, where freeing the
 *        driver leaves the run loop calling into a dead socket.
 */
void IO::ConnectionManager::setBusType(SerialStudio::BusType type)
{
  SS_ASSERT(static_cast<int>(type) >= 0, return);
  SS_ASSERT_LOG(thread() == QThread::currentThread());

  auto& model = m_projectModel;

  if (m_busType == type && m_devices.find(0) != m_devices.end()) {
    const auto opMode = m_appState.operationMode();
    if (opMode == SerialStudio::ProjectFile && model.sources().size() == 1
        && model.sources()[0].busType != static_cast<int>(type))
      model.setSource0BusType(static_cast<int>(type));
    return;
  }

  disconnectDevice(0);

  m_busType = type;
  m_settings.setValue("IOManager/busType", static_cast<int>(type));

  if (m_appState.operationMode() != SerialStudio::ProjectFile)
    m_settings.setValue("IOManager/userBusType", static_cast<int>(type));

  auto driver = m_driverFactory.create(type);

  if (type == SerialStudio::BusType::BluetoothLE) {
    auto* ble = m_uiDrivers.bluetoothLE();
    if (ble && ble->operatingSystemSupported())
      ble->startDiscovery();
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

    connect(driver.get(),
            &IO::HAL_Driver::openFinished,
            this,
            &IO::ConnectionManager::onDriverOpenFinished,
            Qt::UniqueConnection);

    if (type == SerialStudio::BusType::BluetoothLE) {
      auto* ble = qobject_cast<IO::Drivers::BluetoothLE*>(driver.get());
      if (ble)
        connect(ble,
                &IO::Drivers::BluetoothLE::gattReady,
                this,
                &IO::ConnectionManager::connectedChanged);
    }

    auto dm =
      std::make_unique<DeviceManager>(0, std::move(driver), m_streamConfigs.frameConfig(0), this);
    wireDevice(dm.get());

    auto existing = m_devices.find(0);
    if (existing != m_devices.end() && existing->second) {
      disconnect(existing->second.get(), nullptr, this, nullptr);

      auto* retired = existing->second.release();
      if (retired)
        retired->deleteLater();
    }

    m_devices[0] = std::move(dm);
  } else {
    dropUnavailablePrimaryDevice(type);
  }

  rebuildStreamWorkers();

  Q_EMIT driverChanged();
  Q_EMIT busTypeChanged();

  const auto opMode = m_appState.operationMode();
  if (opMode == SerialStudio::ProjectFile && model.sources().size() == 1) {
    model.setSource0BusType(static_cast<int>(type));

    if (!model.jsonFilePath().isEmpty())
      (void)model.saveJsonFile(false);
  }
}

/**
 * @brief Removes the primary device when no driver could be created for @p type (license gate)
 *        and queues the activation prompt when the bus exists but is not licensed.
 */
void IO::ConnectionManager::dropUnavailablePrimaryDevice(SerialStudio::BusType type)
{
  auto existing = m_devices.find(0);
  if (existing != m_devices.end()) {
    if (existing->second)
      disconnect(existing->second.get(), nullptr, this, nullptr);

    auto* retired = existing->second.release();
    m_devices.erase(existing);
    if (retired)
      retired->deleteLater();
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

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors all properties from the active UI-config driver to the live DeviceManager driver.
 */
void IO::ConnectionManager::syncUiDriverToLive()
{
  m_uiSync.syncToLive(activeUiDriver(), driver(0));
}

/**
 * @brief Applies source[0]'s busType and connectionSettings to the matching UI-config driver. The
 *        bus-type half runs through this lambda so the settings write, the persisted key and
 *        busTypeChanged() still land where they always did: inside the mirror's fence and before
 *        the connection settings reach the driver.
 */
void IO::ConnectionManager::syncUiDriverFromSource0()
{
  const auto applyBusType = [this](SerialStudio::BusType type) {
    m_busType = type;
    m_settings.setValue("IOManager/busType", static_cast<int>(type));
    Q_EMIT busTypeChanged();
  };

  if (m_uiSync.syncFromSource0(m_busType, applyBusType))
    Q_EMIT driverChanged();
}

/**
 * @brief Connects a DeviceManager's raw-byte signal to the console/API fan-out; DirectConnection
 *        avoids per-chunk postEvent overhead on the main-thread hop. Extracted frames no longer
 *        pass through here: FrameReaders route them into FrameBuilder on the pipeline thread
 *        (IO::PipelineHost::routeFrames, spec 0051 M3).
 */
void IO::ConnectionManager::wireDevice(DeviceManager* dm)
{
  SS_ASSERT(dm != nullptr, return);
  SS_ASSERT_LOG(dm->driver() != nullptr);

  connect(dm,
          &IO::DeviceManager::rawDataReceived,
          this,
          &IO::ConnectionManager::onRawDataReceived,
          Qt::DirectConnection);

  connect(dm,
          &IO::DeviceManager::consoleDataReceived,
          this,
          &IO::ConnectionManager::onConsoleDataReceived,
          Qt::DirectConnection);
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
  if (DriverFactory::diagnosticsBus(driver(deviceId), bus)) {
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
 * @brief Reports a connected-state transition a live driver reached on its own; BLE, TCP, CAN,
 *        S7 and EtherNet/IP dial asynchronously, so their open lands after the request settled.
 *        Queued because a driver reporting mid-open is still inside its own open().
 */
void IO::ConnectionManager::refreshConnectedState()
{
  notifyConnectedStateChanged();
}

/**
 * @brief Captures current UI-config driver settings back to source[0], arming the debounced
 *        autosave when the project is a file on disk.
 */
void IO::ConnectionManager::onUiDriverConfigurationChanged()
{
  if (m_uiSync.captureToSource0(m_busType, activeUiDriver(), sender()))
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

  auto driver = m_driverFactory.create(static_cast<SerialStudio::BusType>(src.busType));
  if (!driver)
    return;

  if (!src.connectionSettings.isEmpty())
    driver->applyConnectionSettings(src.connectionSettings);

  auto* rawDriver = driver.get();
  auto dm         = std::make_unique<DeviceManager>(
    src.sourceId, std::move(driver), m_streamConfigs.frameConfig(src.sourceId), this);

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

  connect(rawDriver,
          &IO::HAL_Driver::openFinished,
          this,
          &IO::ConnectionManager::onDriverOpenFinished,
          Qt::UniqueConnection);

  wireDevice(dm.get());
  m_devices[src.sourceId] = std::move(dm);
}

/**
 * @brief Wires the stream-lane lifecycle inputs: worker rebuilds on lane/mode edits, template
 *        publish on the connect edge, export-flag refresh on sink enable changes (spec 0051).
 */
void IO::ConnectionManager::wireStreamLifecycle()
{
  SS_ASSERT(m_apiServer != nullptr, return);

  connect(&m_projectModel,
          &DataModel::ProjectModel::sourceStreamLaneChanged,
          this,
          &IO::ConnectionManager::rebuildStreamWorkers,
          Qt::QueuedConnection);

  connect(&m_projectModel,
          &DataModel::ProjectModel::luaFastModeChanged,
          this,
          &IO::ConnectionManager::rebuildStreamWorkers,
          Qt::QueuedConnection);

  connect(this, &IO::ConnectionManager::connectedChanged, this, [this] {
    if (isConnected() && !m_streamPool.workers().empty())
      m_streamPool.publishTemplates();
  });

  static auto& csvExport = CSV::Export::instance();
  connect(&csvExport,
          &CSV::Export::enabledChanged,
          this,
          &IO::ConnectionManager::refreshStreamExportFlags);

  connect(m_apiServer,
          &API::Server::enabledChanged,
          this,
          &IO::ConnectionManager::refreshStreamExportFlags);
  connect(m_apiServer,
          &API::Server::streamSubscribersChanged,
          this,
          &IO::ConnectionManager::refreshStreamExportFlags);
#ifdef BUILD_COMMERCIAL
  SS_ASSERT(m_sessionExport != nullptr, return);

  static auto& mdf4Export = MDF4::Export::instance();
  connect(&mdf4Export,
          &MDF4::Export::enabledChanged,
          this,
          &IO::ConnectionManager::refreshStreamExportFlags);

  static auto& audioExport = Widgets::AudioExport::instance();
  connect(&audioExport,
          &Widgets::AudioExport::activeSessionsChanged,
          this,
          &IO::ConnectionManager::refreshStreamExportFlags);

  connect(m_sessionExport,
          &Sessions::Export::enabledChanged,
          this,
          &IO::ConnectionManager::refreshStreamExportFlags);
#endif
}

/**
 * @brief Rebuilds the per-source stream workers from the live device map. The pool owns the
 *        workers and their queued FrameBuilder wiring; this snapshot is the only thing it needs
 *        from the device map, and it is taken here so the pool never reaches into it.
 */
void IO::ConnectionManager::rebuildStreamWorkers()
{
  std::vector<StreamWorkerPool::Source> sources;
  sources.reserve(m_devices.size());
  for (const auto& [deviceId, dm] : m_devices)
    if (dm && dm->driver())
      sources.push_back({deviceId, dm->driver()});

  m_streamPool.rebuild(sources, m_paused, isConnected());
}

/**
 * @brief Re-derives the FrameBuilder's cached any-async-sink flag when a typed sink's enable
 *        state moves; it must fire for every sink, or a recording produces a valid-looking file
 *        containing nothing.
 */
void IO::ConnectionManager::refreshStreamExportFlags()
{
  m_streamPool.refreshExportFlags();
}

/**
 * @brief Stops and destroys every stream worker (idempotent; called on rebuilds and at quit
 *        from ModuleManager::stopFrameConsumerWorkers before SessionContext::shutdown).
 */
void IO::ConnectionManager::stopStreamWorkers()
{
  m_streamPool.stop();
}

/**
 * @brief Returns the live stream workers (GUI thread only; Dashboard drains their display
 *        rings on the display tick).
 */
const std::vector<std::unique_ptr<IO::StreamWorker>>& IO::ConnectionManager::streamWorkers()
  const noexcept
{
  return m_streamPool.workers();
}

/**
 * @brief Rebuilds DeviceManagers when the source list changes; reentrant triggers coalesce into
 *        one queued rebuild, none emits sessionClosed. Retired devices die via deleteLater(): an
 *        error box pumps events mid socket notification. Outside ProjectFile mode device 0 is
 *        KEPT, because nothing in this pass rebuilds it (the queued setBusType() does, later).
 */
void IO::ConnectionManager::rebuildDevices()
{
  SS_ASSERT_LOG(thread() == QThread::currentThread());

  if (m_rebuildingDevices) {
    QMetaObject::invokeMethod(this, &IO::ConnectionManager::rebuildDevices, Qt::QueuedConnection);
    return;
  }

  m_rebuildingDevices = true;

  const auto opMode       = m_appState.operationMode();
  const bool wasConnected = isConnected();

  bool willRebuildDevice0 = false;
  bool didChangeBusType   = false;
  if (opMode == SerialStudio::ProjectFile) {
    const auto& srcs = m_projectModel.sources();
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

    (void)m_fanOut.takePendingDial(it->first);
    if (it->second) {
      if (it->second->driver())
        it->second->driver()->disarmOpenReport();

      it->second->close();
      disconnect(it->second.get(), nullptr, this, nullptr);
    }

    auto* retired = it->second.release();
    it            = m_devices.erase(it);
    if (retired)
      retired->deleteLater();
  }

  if (opMode == SerialStudio::ProjectFile) {
    const auto& sources = m_projectModel.sources();
    for (const auto& src : sources)
      buildDeviceForSource(src, willRebuildDevice0);
  }

  concludeConnectRequest();

  rebuildStreamWorkers();

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

  const auto reconnectIfDropped = [this] {
    if (!isConnected())
      connectDevice();
  };

  if (wasConnected)
    QMetaObject::invokeMethod(this, reconnectIfDropped, Qt::QueuedConnection);

  m_rebuildingDevices = false;
}

/**
 * @brief Reconfigures a live project source when its framing settings change.
 */
void IO::ConnectionManager::onProjectSourceChanged(int sourceId)
{
  if (sourceId <= 0 || m_appState.operationMode() != SerialStudio::ProjectFile)
    return;

  auto it = m_devices.find(sourceId);
  if (it == m_devices.end() || !it->second)
    return;

  it->second->reconfigure(m_streamConfigs.frameConfig(sourceId));
  Q_EMIT configurationChanged();
}

/**
 * @brief Returns true when the current project sources are all configured.
 */
bool IO::ConnectionManager::projectConfigurationOk() const
{
  return m_query.projectConfigurationOk();
}

/**
 * @brief Forwards raw bytes from device @p deviceId to Console and API Server.
 */
void IO::ConnectionManager::onRawDataReceived(int deviceId, const IO::CapturedDataPtr& data)
{
  m_io.onRawDataReceived(deviceId, data);
}

/**
 * @brief Forwards a stream-lane source's terminal-only bytes from device @p deviceId to the
 *        console. The typed sample blocks already fed the dashboard, the exports and the API,
 *        so this text stops at the terminal and nothing is recorded twice.
 */
void IO::ConnectionManager::onConsoleDataReceived(int deviceId, const IO::CapturedDataPtr& data)
{
  m_io.onConsoleDataReceived(deviceId, data);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the device id @p driver backs, or -1 when no device owns it. A null driver is an
 *        ordinary miss: the recovery paths call this with whatever the sender handed them.
 */
int IO::ConnectionManager::deviceIdForDriver(const HAL_Driver* driver) const
{
  return m_query.deviceIdForDriver(driver);
}

/**
 * @brief Snapshots the device ids, optionally skipping the primary. Every fan-out iterates this
 *        copy instead of m_devices: an open or a close can spin the event loop (error boxes,
 *        control scripts), and a rebuild landing there would invalidate a live iterator.
 */
std::vector<int> IO::ConnectionManager::deviceIdSnapshot(bool projectSourcesOnly) const
{
  return m_query.deviceIdSnapshot(projectSourcesOnly);
}

/**
 * @brief Builds a FrameConfig for the given @p deviceId from current settings. Public because the
 *        spec-0044 headless verifier rebuilds the production readers from it.
 */
IO::FrameConfig IO::ConnectionManager::buildFrameConfig(int deviceId) const
{
  return m_streamConfigs.frameConfig(deviceId);
}
