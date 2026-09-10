/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/CANBus.h"

#include <chrono>
#include <QCanBus>
#include <QLoggingCategory>
#include <stdexcept>
#include <utility>

#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "Core/TimerEvents.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/CANBus/CanBackends.h"
#include "IO/Drivers/CANBus/GsUsbCanBackend.h"

// Default CAN FD data-phase bitrate (the gs_usb backend applies the same fallback)
static constexpr quint32 kDefaultDataBitrate = 2000000;

// DLC byte marking a reassembled long frame (a bus frame never exceeds 64)
static constexpr quint8 kLongFrameDlcMarker = 0xFF;

// Bus frames published in one chunk before the batch is flushed for latency
static constexpr qsizetype kMaxBatchedFrames = 256;

//--------------------------------------------------------------------------------------------------
// Synthetic (libusb/serial) CAN backend plugin helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Qt CAN plugins plus every supported synthetic backend.
 */
static QStringList enumerateCanPlugins()
{
  static auto* canBus = QCanBus::instance();
  QStringList list    = canBus->plugins();
  for (const auto& backend : IO::Drivers::CanBackends::all())
    if (backend.supported)
      list.append(backend.key);

  return list;
}

/**
 * @brief Serializes one bus frame into the driver's wire layout: standard frames as
 * [ID_hi, ID_lo, DLC, payload...] padded to 11 bytes, extended as [0x80|ID28..24, ID23..16,
 * ID15..8, ID7..0, DLC, payload...] padded to 13.
 */
static QByteArray serializeCanFrame(const quint32 can_id,
                                    const bool extended,
                                    const QByteArray& payload)
{
  QByteArray data;
  data.reserve(extended ? 13 : 11);

  if (extended) {
    data.append(static_cast<char>(0x80 | ((can_id >> 24) & 0x1F)));
    data.append(static_cast<char>((can_id >> 16) & 0xFF));
  }

  data.append(static_cast<char>((can_id >> 8) & 0xFF));
  data.append(static_cast<char>(can_id & 0xFF));
  data.append(static_cast<char>(static_cast<quint8>(payload.size())));
  data.append(payload);

  const qsizetype min_size = extended ? 13 : 11;
  while (data.size() < min_size)
    data.append(static_cast<char>(0));

  return data;
}

/**
 * @brief Copies one reassembler's counters into a variant map for the pulled diagnostics.
 */
static QVariantMap reassemblyCountersMap(const IO::Drivers::CanReassemblyCounters& counters)
{
  QVariantMap map;
  map.insert(QStringLiteral("aborted"), static_cast<qulonglong>(counters.aborted));
  map.insert(QStringLiteral("timeouts"), static_cast<qulonglong>(counters.timeouts));
  map.insert(QStringLiteral("completed"), static_cast<qulonglong>(counters.completed));
  map.insert(QStringLiteral("malformed"), static_cast<qulonglong>(counters.malformed));
  map.insert(QStringLiteral("sizeOverruns"), static_cast<qulonglong>(counters.sizeOverruns));
  map.insert(QStringLiteral("sequenceErrors"), static_cast<qulonglong>(counters.sequenceErrors));
  map.insert(QStringLiteral("sessionOverruns"), static_cast<qulonglong>(counters.sessionOverruns));
  return map;
}

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CANBus driver and restores persisted settings.
 */
IO::Drivers::CANBus::CANBus()
  : m_device(nullptr)
  , m_canFD(false)
  , m_loopback(false)
  , m_listenOnly(false)
  , m_tpReassembly(false)
  , m_pluginIndex(0)
  , m_interfaceIndex(0)
  , m_bitrate(500000)
  , m_dataBitrate(kDefaultDataBitrate)
  , m_hwStampAnchored(false)
  , m_hwStampOffset(CapturedData::SteadyClock::duration::zero())
  , m_batchedFrames(0)
{
  m_pluginList = enumerateCanPlugins();

  m_canFD          = m_settings.value("CanBusDriver/canFD", false).toBool();
  m_loopback       = m_settings.value("CanBusDriver/loopback", false).toBool();
  m_listenOnly     = m_settings.value("CanBusDriver/listenOnly", false).toBool();
  m_tpReassembly   = m_settings.value("CanBusDriver/tpReassembly", false).toBool();
  m_bitrate        = m_settings.value("CanBusDriver/bitrate", 500000).toUInt();
  m_dataBitrate    = m_settings.value("CanBusDriver/dataBitrate", kDefaultDataBitrate).toUInt();
  m_pluginIndex    = m_settings.value("CanBusDriver/pluginIndex", 0).toUInt();
  m_interfaceIndex = m_settings.value("CanBusDriver/interfaceIndex", 0).toUInt();

  if (!m_pluginList.isEmpty() && m_pluginIndex < m_pluginList.count())
    refreshInterfaces();

  m_hotplugDebounce.setInterval(200);
  m_hotplugDebounce.setSingleShot(true);
  connect(&m_hotplugDebounce, &QTimer::timeout, this, &IO::Drivers::CANBus::refreshInterfaces);

  connect(this,
          &IO::Drivers::CANBus::pluginIndexChanged,
          this,
          &IO::Drivers::CANBus::configurationChanged);
  connect(this,
          &IO::Drivers::CANBus::interfaceIndexChanged,
          this,
          &IO::Drivers::CANBus::configurationChanged);
  connect(
    this, &IO::Drivers::CANBus::bitrateChanged, this, &IO::Drivers::CANBus::configurationChanged);
  connect(this,
          &IO::Drivers::CANBus::dataBitrateChanged,
          this,
          &IO::Drivers::CANBus::configurationChanged);
  connect(
    this, &IO::Drivers::CANBus::canFDChanged, this, &IO::Drivers::CANBus::configurationChanged);
  connect(
    this, &IO::Drivers::CANBus::loopbackChanged, this, &IO::Drivers::CANBus::configurationChanged);
  connect(this,
          &IO::Drivers::CANBus::listenOnlyChanged,
          this,
          &IO::Drivers::CANBus::configurationChanged);
  connect(this,
          &IO::Drivers::CANBus::tpReassemblyChanged,
          this,
          &IO::Drivers::CANBus::configurationChanged);

  QLoggingCategory::setFilterRules("qt.canbus* = false");
}

/**
 * @brief Closes the CAN bus device and disarms the hot-plug notifier before this object dies.
 */
IO::Drivers::CANBus::~CANBus()
{
  GsUsbCanBackend::clearHotplugNotifier(this);
  doClose();
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the current CAN bus connection.
 */
void IO::Drivers::CANBus::close()
{
  doClose();

  Q_EMIT configurationChanged();
  Q_EMIT availablePluginsChanged();
  Q_EMIT availableInterfacesChanged();
}

/**
 * @brief Non-virtual cleanup implementation shared by close() and ~CANBus().
 */
void IO::Drivers::CANBus::doClose()
{
  m_hwStampAnchored = false;

  if (!m_device)
    return;

  disconnect(m_device, nullptr, this, nullptr);

  if (m_device->state() != QCanBusDevice::UnconnectedState)
    m_device->disconnectDevice();

  m_device->deleteLater();
  m_device = nullptr;
}

/**
 * @brief Returns true when the CAN bus device is connected.
 */
bool IO::Drivers::CANBus::isOpen() const noexcept
{
  if (m_device)
    return m_device->state() == QCanBusDevice::ConnectedState;

  return false;
}

/**
 * @brief Returns true while the CAN device is dialing.
 */
bool IO::Drivers::CANBus::isConnecting() const noexcept
{
  if (m_device)
    return m_device->state() == QCanBusDevice::ConnectingState;

  return false;
}

/**
 * @brief Returns true when the CAN bus device can be read.
 */
bool IO::Drivers::CANBus::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true when the CAN bus device can be written.
 */
bool IO::Drivers::CANBus::isWritable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true if the user has selected appropriate controls & options to connect to a CAN
 * bus device.
 */
bool IO::Drivers::CANBus::configurationOk() const noexcept
{
  if (!canSupportAvailable())
    return false;

  return m_pluginIndex < m_pluginList.count() && !m_interfaceList.isEmpty()
      && m_interfaceIndex < m_interfaceList.count();
}

/**
 * @brief Writes a CAN frame to the bus, capping DLC at the format limit (8 classic, 64 FD).
 * Mirrors the receive header: [ID_hi, ID_lo, DLC, payload...] for standard ids, or a 29-bit
 * extended id as [0x80|ID28..24, ID23..16, ID15..8, ID7..0, DLC, payload...]. Bit 7 of
 * byte 0 selects the form (extended is also set when a 2-byte id exceeds the 11-bit range).
 */
qint64 IO::Drivers::CANBus::write(const QByteArray& data)
{
  if (!m_device)
    return 0;

  if (!isWritable())
    return 0;

  if (data.length() < 3)
    return 0;

  try {
    const bool extended = (static_cast<quint8>(data[0]) & 0x80) != 0;
    if (extended && data.length() < 5)
      return 0;

    quint32 can_id = 0;
    int dlc_index  = 2;
    if (extended) {
      can_id = (static_cast<quint32>(static_cast<quint8>(data[0]) & 0x1F) << 24)
             | (static_cast<quint32>(static_cast<quint8>(data[1])) << 16)
             | (static_cast<quint32>(static_cast<quint8>(data[2])) << 8)
             | static_cast<quint8>(data[3]);
      dlc_index = 4;
    } else
      can_id = ((static_cast<quint8>(data[0]) & 0x07) << 8) | static_cast<quint8>(data[1]);

    const bool fd_active = m_canFD && interfaceSupportsFD();

    quint8 dlc     = static_cast<quint8>(data[dlc_index]);
    quint8 max_dlc = fd_active ? 64 : 8;

    if (dlc > max_dlc)
      dlc = max_dlc;

    QByteArray payload = data.mid(dlc_index + 1, dlc);

    QCanBusFrame frame(can_id, payload);

    if (extended || can_id > 0x7FF)
      frame.setExtendedFrameFormat(true);

    if (fd_active)
      frame.setFlexibleDataRateFormat(true);

    if (m_device->writeFrame(frame)) {
      Q_EMIT dataSent(data);
      return data.length();
    }

    qWarning() << "CAN write failed:" << m_device->errorString();
  } catch (const std::exception& e) {
    qWarning() << "CAN write failed:" << e.what();
  } catch (...) {
    qWarning() << "CAN write failed: unknown exception";
  }

  return 0;
}

/**
 * @brief Opens the CAN bus device with the given mode.
 */
bool IO::Drivers::CANBus::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  close();

  if (!canSupportAvailable()) {
    showCanSupportError();
    return false;
  }

  if (!validateOpenPreconditions())
    return false;

  QString plugin         = m_pluginList.at(m_pluginIndex);
  QString interface_name = m_interfaceList.at(m_interfaceIndex);

  QString error;
  if (const auto* backend = IO::Drivers::CanBackends::find(plugin))
    m_device = backend->create(interface_name);
  else {
    static auto* canBus = QCanBus::instance();
    m_device            = canBus->createDevice(plugin, interface_name, &error);
  }

  if (!m_device) {
    logDriverError(tr("CAN Device Creation Failed"),
                   error.isEmpty()
                     ? tr("Unable to create CAN bus device. Check your hardware and drivers.")
                     : error);
    return false;
  }

  m_device->setConfigurationParameter(QCanBusDevice::BitRateKey, m_bitrate);
  if (m_canFD && interfaceSupportsFD()) {
    m_device->setConfigurationParameter(QCanBusDevice::CanFdKey, true);
    m_device->setConfigurationParameter(QCanBusDevice::DataBitRateKey, m_dataBitrate);
  } else if (m_canFD) {
    qWarning() << "CAN FD requested but" << interface_name
               << "reports no FD support; opening in classic mode";
  }

  if (m_loopback)
    m_device->setConfigurationParameter(QCanBusDevice::LoopbackKey, true);

  if (m_listenOnly)
    m_device->setConfigurationParameter(IO::Drivers::kListenOnlyConfigKey, true);

  wireCanBusSignals();

  if (!m_device->connectDevice()) {
    error = m_device->errorString();
    m_device->deleteLater();
    m_device = nullptr;
    const QString base =
      error.isEmpty()
        ? tr("Unable to connect to CAN bus device. Check your hardware connection and settings.")
        : error;
    logDriverError(tr("CAN Connection Failed"), base + connectionErrorHint(plugin, interface_name));
    return false;
  }

  Q_EMIT configurationChanged();
  return true;
}

/**
 * @brief Queues a platform-specific error dialog when no CAN bus plugins are available.
 */
void IO::Drivers::CANBus::showCanSupportError()
{
#if defined(Q_OS_LINUX)
  logDriverError(
    tr("CAN Bus Not Available"),
    tr(
      "No CAN bus plugins found on this system.\n\nOn Linux, ensure SocketCAN kernel modules are loaded."));
#elif defined(Q_OS_WIN)
  logDriverError(
    tr("CAN Bus Not Available"),
    tr(
      "No CAN bus plugins found on this system.\n\nOn Windows, install CAN hardware drivers (PEAK, Vector, etc.)."));
#elif defined(Q_OS_MAC)
  logDriverError(
    tr("CAN Bus Not Available"),
    tr(
      "No CAN bus plugins found on this system.\n\nCAN bus support on macOS is limited and may require third-party hardware drivers."));
#else
  logDriverError(tr("CAN Bus Not Available"),
                 tr("No CAN bus plugins are available on this platform."));
#endif
}

/**
 * @brief Validates plugin/interface selection state; queues a dialog and returns false on error.
 */
bool IO::Drivers::CANBus::validateOpenPreconditions()
{
  if (!configurationOk()) {
    logDriverError(
      tr("Invalid CAN Configuration"),
      tr("The CAN bus configuration is incomplete. Select a valid plugin and interface."));
    return false;
  }

  if (m_pluginIndex >= m_pluginList.count() || m_interfaceIndex >= m_interfaceList.count()) {
    logDriverError(
      tr("Invalid Selection"),
      tr(
        "The selected plugin or interface is no longer available. Refresh the lists and try again."));
    return false;
  }

  if (m_pluginList.isEmpty() || m_interfaceList.isEmpty()) {
    logDriverError(
      tr("No Devices Available"),
      tr(
        "The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected."));
    return false;
  }

  return true;
}

/**
 * @brief Connects framesReceived/stateChanged/errorOccurred signals to their slots.
 */
void IO::Drivers::CANBus::wireCanBusSignals()
{
  connect(m_device,
          &QCanBusDevice::framesReceived,
          this,
          &IO::Drivers::CANBus::onFramesReceived,
          Qt::UniqueConnection);
  connect(m_device,
          &QCanBusDevice::stateChanged,
          this,
          &IO::Drivers::CANBus::onStateChanged,
          Qt::UniqueConnection);
  connect(m_device,
          &QCanBusDevice::errorOccurred,
          this,
          &IO::Drivers::CANBus::onErrorOccurred,
          Qt::UniqueConnection);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if CAN FD mode is enabled
 */
bool IO::Drivers::CANBus::canFD() const
{
  return m_canFD;
}

/**
 * @brief Returns true if hardware loopback mode is enabled
 */
bool IO::Drivers::CANBus::loopback() const
{
  return m_loopback;
}

/**
 * @brief Returns true when the currently selected interface reports CAN FD capability.
 */
bool IO::Drivers::CANBus::interfaceSupportsFD() const
{
  if (m_interfaceIndex < m_interfaceFdCapable.count())
    return m_interfaceFdCapable.at(m_interfaceIndex);

  return false;
}

/**
 * @brief Returns true if listen-only (silent) mode is enabled
 */
bool IO::Drivers::CANBus::listenOnly() const
{
  return m_listenOnly;
}

/**
 * @brief Returns true if J1939 transport-protocol and ISO-TP reassembly is enabled
 */
bool IO::Drivers::CANBus::tpReassembly() const
{
  return m_tpReassembly;
}

/**
 * @brief Returns the pulled reassembly diagnostics, one sub-map per protocol (spec 0033): the
 * counters are plain integers the reassemblers increment in place, so reading them costs a copy
 * on the caller's cadence and nothing on the receive path.
 */
QVariantMap IO::Drivers::CANBus::reassemblyCounters() const
{
  QVariantMap map;
  map.insert(QStringLiteral("enabled"), m_tpReassembly);
  map.insert(QStringLiteral("j1939"), reassemblyCountersMap(m_j1939.counters()));
  map.insert(QStringLiteral("isotp"), reassemblyCountersMap(m_isoTp.counters()));
  map.insert(QStringLiteral("j1939Sessions"), m_j1939.activeSessions());
  map.insert(QStringLiteral("isotpSessions"), m_isoTp.activeSessions());
  return map;
}

/**
 * @brief Returns the current plugin index
 */
quint8 IO::Drivers::CANBus::pluginIndex() const
{
  return m_pluginIndex;
}

/**
 * @brief Returns the current interface index
 */
quint8 IO::Drivers::CANBus::interfaceIndex() const
{
  return m_interfaceIndex;
}

/**
 * @brief Returns the current bitrate
 */
quint32 IO::Drivers::CANBus::bitrate() const
{
  return m_bitrate;
}

/**
 * @brief Returns the current CAN FD data-phase bitrate
 */
quint32 IO::Drivers::CANBus::dataBitrate() const
{
  return m_dataBitrate;
}

/**
 * @brief Returns the list of available CAN bus plugins
 */
QStringList IO::Drivers::CANBus::pluginList() const
{
  return m_pluginList;
}

/**
 * @brief Returns the list of available CAN bus interfaces
 */
QStringList IO::Drivers::CANBus::interfaceList() const
{
  return m_interfaceList;
}

/**
 * @brief Returns the last error that occurred while querying interfaces
 */
QString IO::Drivers::CANBus::interfaceError() const
{
  return m_interfaceError;
}

/**
 * @brief Returns the list of standard CAN bus bitrates
 */
QStringList IO::Drivers::CANBus::bitrateList() const
{
  QStringList list;
  list << "10000";
  list << "20000";
  list << "50000";
  list << "100000";
  list << "125000";
  list << "250000";
  list << "500000";
  list << "800000";
  list << "1000000";
  return list;
}

/**
 * @brief Returns the list of standard CAN FD data-phase bitrates
 */
QStringList IO::Drivers::CANBus::dataBitrateList() const
{
  QStringList list;
  list << "1000000";
  list << "2000000";
  list << "4000000";
  list << "5000000";
  list << "8000000";
  return list;
}

/**
 * @brief Converts a Qt CAN plugin name to a user-friendly display name.
 */
QString IO::Drivers::CANBus::pluginDisplayName(const QString& plugin) const
{
  if (const auto* backend = IO::Drivers::CanBackends::find(plugin))
    return backend->displayName;

  if (plugin == "socketcan")
    return "SocketCAN";

  if (plugin == "peakcan")
    return "PEAK CAN";

  if (plugin == "passthroughcan" || plugin == "passthrucan")
    return "PassThru CAN";

  if (plugin == "virtualcan")
    return "Virtual CAN";

  if (plugin == "systeccan")
    return "SysTec CAN";

  if (plugin == "tinycan")
    return "Tiny CAN";

  if (plugin == "vectorcan")
    return "Vector CAN";

  return plugin;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets whether CAN FD mode is enabled.
 */
void IO::Drivers::CANBus::setCanFD(const bool enabled)
{
  if (m_canFD == enabled)
    return;

  m_canFD = enabled;
  m_settings.setValue("CanBusDriver/canFD", enabled);
  Q_EMIT canFDChanged();
}

/**
 * @brief Sets whether hardware loopback mode is enabled.
 */
void IO::Drivers::CANBus::setLoopback(const bool enabled)
{
  if (m_loopback == enabled)
    return;

  m_loopback = enabled;
  m_settings.setValue("CanBusDriver/loopback", enabled);
  Q_EMIT loopbackChanged();
}

/**
 * @brief Sets whether listen-only (silent) mode is enabled.
 */
void IO::Drivers::CANBus::setListenOnly(const bool enabled)
{
  if (m_listenOnly == enabled)
    return;

  m_listenOnly = enabled;
  m_settings.setValue("CanBusDriver/listenOnly", enabled);
  Q_EMIT listenOnlyChanged();
}

/**
 * @brief Sets whether J1939 transport-protocol and ISO-TP frames are reassembled. Defaults to off
 * because interpreting the TP parameter groups on a bus that is not J1939 would turn ordinary
 * traffic into synthesized long frames; toggling it discards any half-collected session, so the
 * two settings never mix inside one message.
 */
void IO::Drivers::CANBus::setTpReassembly(const bool enabled)
{
  if (m_tpReassembly == enabled)
    return;

  m_tpReassembly = enabled;
  m_settings.setValue("CanBusDriver/tpReassembly", enabled);

  m_j1939.reset();
  m_isoTp.reset();

  Q_EMIT tpReassemblyChanged();
}

/**
 * @brief Sets the bitrate for the CAN bus.
 */
void IO::Drivers::CANBus::setBitrate(const quint32 bitrate)
{
  if (m_bitrate == bitrate)
    return;

  m_bitrate = bitrate;
  m_settings.setValue("CanBusDriver/bitrate", bitrate);
  Q_EMIT bitrateChanged();
}

/**
 * @brief Sets the CAN FD data-phase bitrate.
 */
void IO::Drivers::CANBus::setDataBitrate(const quint32 bitrate)
{
  if (m_dataBitrate == bitrate)
    return;

  m_dataBitrate = bitrate;
  m_settings.setValue("CanBusDriver/dataBitrate", bitrate);
  Q_EMIT dataBitrateChanged();
}

/**
 * @brief Sets the plugin index and refreshes available interfaces.
 */
void IO::Drivers::CANBus::setPluginIndex(const quint8 index)
{
  if (index < m_pluginList.count() && m_pluginIndex != index) {
    m_pluginIndex = index;
    m_settings.setValue("CanBusDriver/pluginIndex", index);
    refreshInterfaces();
    Q_EMIT pluginIndexChanged();
  }
}

/**
 * @brief Sets the interface index.
 */
void IO::Drivers::CANBus::setInterfaceIndex(const quint8 index)
{
  if (index < m_interfaceList.count() && m_interfaceIndex != index) {
    m_interfaceIndex = index;
    m_settings.setValue("CanBusDriver/interfaceIndex", index);
    Q_EMIT interfaceIndexChanged();
    Q_EMIT interfaceSupportsFDChanged();
  }
}

/**
 * @brief Sets up external connections for timer events and USB hot-plug notifications.
 */
void IO::Drivers::CANBus::setupExternalConnections()
{
  connect(&Core::services().timerEvents,
          &Misc::TimerEvents::timeout1Hz,
          this,
          &IO::Drivers::CANBus::refreshPlugins);

  GsUsbCanBackend::setHotplugNotifier(this, [this] { m_hotplugDebounce.start(); });
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drains available CAN frames and publishes them in the legacy layout: standard frames as
 * [ID_hi, ID_lo, DLC, payload...] (11 bytes), extended as [0x80|ID28..24, ID23..16, ID15..8,
 * ID7..0, DLC, payload...] (13 bytes); payloads over 64 bytes are dropped. Plugin stamps rebase
 * through rebaseFrameTimestamp(). Reassembly, when on, consumes transport frames beforehand.
 */
void IO::Drivers::CANBus::onFramesReceived()
{
  if (!m_device)
    return;

  if (!isOpen())
    return;

  try {
    const auto now = CapturedData::SteadyClock::now();
    while (m_device->framesAvailable() > 0) {
      const QCanBusFrame frame = m_device->readFrame();

      if (!frame.isValid())
        continue;

      const QByteArray payload = frame.payload();
      if (payload.size() > 64)
        continue;

      const bool extended  = frame.hasExtendedFrameFormat();
      const quint32 can_id = frame.frameId() & (extended ? 0x1FFFFFFFu : 0x7FFu);
      SS_ASSERT_LOG(payload.size() <= 64);
      SS_ASSERT_LOG(can_id <= 0x1FFFFFFFu);

      const auto raw         = frame.timeStamp();
      const qint64 stampUsec = raw.seconds() * 1000000 + raw.microSeconds();
      const auto stamp       = stampUsec > 0 ? rebaseFrameTimestamp(stampUsec, now) : now;

      if (m_tpReassembly && routeReassembly(can_id, extended, payload, stamp))
        continue;

      if (m_batchedFrames == 0)
        m_batchFirstStamp = stamp;

      m_batchLastStamp = stamp;
      m_frameBatch.append(serializeCanFrame(can_id, extended, payload));
      ++m_batchedFrames;

      if (m_batchedFrames >= kMaxBatchedFrames)
        flushFrameBatch();
    }

    flushFrameBatch();
  } catch (const std::exception& e) {
    qWarning() << "CAN frame read failed:" << e.what();
  } catch (...) {
    qWarning() << "CAN frame read failed: unknown exception";
  }
}

/**
 * @brief Routes one frame into the reassemblers and reports whether it was consumed. Only the two
 * J1939 transport parameter groups and the ISO 15765-4 standardized diagnostic ranges are claimed,
 * and an ISO-TP SingleFrame is never claimed at all, so everything else falls through to the
 * unchanged publish path.
 */
bool IO::Drivers::CANBus::routeReassembly(const quint32 can_id,
                                          const bool extended,
                                          const QByteArray& payload,
                                          const CapturedData::SteadyTimePoint stamp)
{
  SS_ASSERT(m_tpReassembly, return false);
  SS_ASSERT_LOG(payload.size() <= 64);

  if (extended && J1939TransportReassembler::isTransportFrame(can_id)) {
    const auto done = m_j1939.feed(can_id, payload, stamp);
    if (done.has_value()) {
      const quint32 id = (static_cast<quint32>(done->priority & 0x07u) << 26)
                       | ((done->pgn & 0x3FFFFu) << 8) | done->sourceAddr;
      publishReassembled(id, done->bytes, done->firstSeen);
    }

    return true;
  }

  if (!IsoTpReassembler::isDiagnosticId(can_id, extended))
    return false;

  if (!IsoTpReassembler::isMultiFrame(payload))
    return false;

  const auto done = m_isoTp.feed(can_id, extended, payload, stamp);
  if (done.has_value())
    publishReassembled(done->canId, done->bytes, done->firstSeen);

  return true;
}

/**
 * @brief Publishes one reassembled message as a single extended-format frame whose DLC byte is the
 * long-frame marker and whose payload is appended whole, deliberately bypassing the 64-byte drop
 * that still guards the raw path. @p stamp is the capture time of the session's FIRST packet: the
 * source owns time, so the message is dated when it began, not when its last fragment landed.
 */
void IO::Drivers::CANBus::publishReassembled(const quint32 can_id,
                                             const QByteArray& payload,
                                             const CapturedData::SteadyTimePoint stamp)
{
  SS_ASSERT(!payload.isEmpty(), return);
  SS_ASSERT_LOG(can_id <= 0x1FFFFFFFu);

  flushFrameBatch();

  QByteArray data;
  data.reserve(payload.size() + 5);
  data.append(static_cast<char>(0x80 | ((can_id >> 24) & 0x1F)));
  data.append(static_cast<char>((can_id >> 16) & 0xFF));
  data.append(static_cast<char>((can_id >> 8) & 0xFF));
  data.append(static_cast<char>(can_id & 0xFF));
  data.append(static_cast<char>(kLongFrameDlcMarker));
  data.append(payload);

  publishReceivedData(std::move(data), stamp);
}

/**
 * @brief Publishes one burst of bus frames as a single chunk carrying its frame count and spacing.
 *        One CapturedData per frame put up to 8k queued emissions per second on the GUI thread;
 *        the logicalFramesHint/frameStep pair is what lets the batch keep per-frame times.
 *        The gap count casts to nanoseconds::rep, long on libstdc++, since qsizetype is long long.
 */
void IO::Drivers::CANBus::flushFrameBatch()
{
  if (m_batchedFrames == 0)
    return;

  auto step = std::chrono::nanoseconds(1);
  if (m_batchedFrames > 1) {
    const auto span =
      std::chrono::duration_cast<std::chrono::nanoseconds>(m_batchLastStamp - m_batchFirstStamp);
    const auto gaps = static_cast<std::chrono::nanoseconds::rep>(m_batchedFrames - 1);
    step            = std::max(std::chrono::nanoseconds(1), span / gaps);
  }

  const qsizetype frames = m_batchedFrames;
  m_batchedFrames        = 0;

  publishReceivedData(std::move(m_frameBatch), m_batchFirstStamp, step, frames);
  m_frameBatch = QByteArray();
}

/**
 * @brief Handles CAN bus device state changes. An UNCONNECTED transition on an established link is
 *        a drop, not a dial verdict: it goes through the manager like every other driver's drop,
 *        queued, because this runs inside QCanBusDevice's own emission.
 */
void IO::Drivers::CANBus::onStateChanged(QCanBusDevice::CanBusDeviceState state)
{
  if (state == QCanBusDevice::ConnectedState)
    reportOpenFinished(true);

  const bool dropped = (state == QCanBusDevice::UnconnectedState) && !openReportArmed();
  if (state == QCanBusDevice::UnconnectedState)
    reportOpenFinished(false, m_device ? m_device->errorString() : QString());

  Q_EMIT configurationChanged();

  if (!dropped)
    return;

  reportDropToManager();
}

/**
 * @brief Routes a bus that went Unconnected on an established link to the manager, which closes
 *        the device and republishes the state. Queued: this runs inside QCanBusDevice's own
 *        emission, and the manager destroys this driver.
 */
void IO::Drivers::CANBus::reportDropToManager()
{
  static auto& connectionManager = ConnectionManager::instance();
  QMetaObject::invokeMethod(
    this, [this] { connectionManager.disconnectDevice(this); }, Qt::QueuedConnection);
}

/**
 * @brief Handles CAN bus errors with a queued, rate-limited message box: the handler runs inside
 *        QCanBusDevice's own emission, and a flapping bus (bus-off cycling, write errors) would
 *        otherwise stack one modal per error until the user clicks through the storm.
 */
void IO::Drivers::CANBus::onErrorOccurred(QCanBusDevice::CanBusError error)
{
  static constexpr qint64 kErrorBoxIntervalMs = 5000;

  if (error == QCanBusDevice::NoError)
    return;

  QString error_string = m_device ? m_device->errorString() : QString();
  if (error_string.isEmpty())
    error_string = tr("Error code: %1").arg(error);

  if (m_errorBoxTimer.isValid() && m_errorBoxTimer.elapsed() < kErrorBoxIntervalMs) {
    qWarning() << "CAN bus error (suppressed dialog):" << error_string;
    return;
  }

  m_errorBoxTimer.restart();
  logDriverError(tr("CAN Bus Communication Error"), error_string);
}

//--------------------------------------------------------------------------------------------------
// Private methods
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebases plugin frame stamps onto the steady clock: QCanBusFrame stamps have no
 * defined clock (SocketCAN: CLOCK_REALTIME, others device-relative), so verbatim use broke
 * steady-clock consumers (io.getLatestFrame ageMs). An anchored offset preserves inter-frame
 * spacing; stamps outside the plausible window re-anchor at @p now (device resets, NTP steps).
 */
IO::CapturedData::SteadyTimePoint IO::Drivers::CANBus::rebaseFrameTimestamp(
  const qint64 stampUsec, const CapturedData::SteadyTimePoint now) noexcept
{
  constexpr auto kMaxStampLag = std::chrono::seconds(5);

  const CapturedData::SteadyTimePoint hw{std::chrono::microseconds(stampUsec)};

  if (m_hwStampAnchored) {
    const auto arrival = hw + m_hwStampOffset;
    if (arrival <= now && arrival >= now - kMaxStampLag)
      return arrival;
  }

  m_hwStampAnchored = true;
  m_hwStampOffset   = now - hw;
  return now;
}

/**
 * @brief Returns a platform-specific hint when no CAN interfaces are found for a plugin.
 */
QString IO::Drivers::CANBus::noInterfacesHint(const QString& plugin) const
{
  const QString driverName = pluginDisplayName(plugin);

  if (IO::Drivers::CanBackends::find(plugin))
    return tr("Connect a %1 adapter, then refresh").arg(driverName);

#if defined(Q_OS_LINUX)
  if (plugin == "socketcan")
    return tr("Load SocketCAN kernel modules first");

  if (plugin == "virtualcan")
    return tr("Set up a virtual CAN interface first");

  return tr("No interfaces found for %1").arg(driverName);

#elif defined(Q_OS_WIN)
  if (plugin == "peakcan")
    return tr(
      "Install <a href='https://www.peak-system.com/Drivers.523.0.html?&L=1'>PEAK CAN drivers</a>");

  if (plugin == "vectorcan")
    return tr(
      "Install <a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'>Vector CAN drivers</a>");

  if (plugin == "systeccan")
    return tr(
      "Install <a href='https://www.systec-electronic.com/en/company/support/driver'>SysTec CAN drivers</a>");

  return tr("Install %1 drivers").arg(driverName);

#elif defined(Q_OS_MAC)
  return tr("Install %1 drivers for macOS").arg(driverName);
#else
  return tr("No interfaces found for %1").arg(driverName);
#endif
}

/**
 * @brief Returns a platform-specific hint to append when connecting to a CAN interface fails.
 */
QString IO::Drivers::CANBus::connectionErrorHint(const QString& plugin,
                                                 const QString& interface_name) const
{
#if defined(Q_OS_LINUX)
  if (plugin == "socketcan")
    return tr("\n\nIf the interface is down, bring it up first:\n"
              "sudo ip link set %1 up type can bitrate %2")
      .arg(interface_name)
      .arg(m_bitrate);
#else
  Q_UNUSED(plugin)
  Q_UNUSED(interface_name)
#endif

  return QString();
}

/**
 * @brief Refreshes the list of available CAN bus interfaces. Each distinct plugin enumeration
 *        error is logged once to avoid per-refresh spam from plugins whose vendor library
 *        (e.g. vectorcan's vxlapi64) is absent on this machine.
 */
void IO::Drivers::CANBus::refreshInterfaces()
{
  const QString previous_error = m_interfaceError;

  m_interfaceList.clear();
  m_interfaceFdCapable.clear();
  m_interfaceError.clear();

  if (m_pluginList.isEmpty() || m_pluginIndex >= m_pluginList.count()) {
    m_interfaceError = tr("No CAN driver selected");
    if (m_interfaceError != previous_error)
      Q_EMIT interfaceErrorChanged();

    Q_EMIT availableInterfacesChanged();
    Q_EMIT interfaceSupportsFDChanged();
    return;
  }

  QString plugin = m_pluginList[m_pluginIndex];

  if (const auto* backend = IO::Drivers::CanBackends::find(plugin)) {
    m_interfaceList = backend->availableInterfaces();
    for (const QString& name : std::as_const(m_interfaceList))
      m_interfaceFdCapable.append(backend->interfaceSupportsFD
                                  && backend->interfaceSupportsFD(name));
  } else {
    QString error;
    static auto* canBus                       = QCanBus::instance();
    const QList<QCanBusDeviceInfo> interfaces = canBus->availableDevices(plugin, &error);

    const QString errorKey = plugin + QLatin1Char('|') + error;
    if (!error.isEmpty() && !m_loggedPluginErrors.contains(errorKey)) {
      m_loggedPluginErrors.insert(errorKey);
      qWarning() << "CAN plugin error:" << plugin << error;
    }

    for (const QCanBusDeviceInfo& info : interfaces) {
      m_interfaceList.append(info.name());
      m_interfaceFdCapable.append(info.hasFlexibleDataRate());
    }
  }

  if (m_interfaceList.isEmpty() && m_interfaceError.isEmpty())
    m_interfaceError = noInterfacesHint(plugin);

  if (m_interfaceError != previous_error)
    Q_EMIT interfaceErrorChanged();

  if (m_interfaceIndex >= m_interfaceList.count() && m_interfaceIndex != 0) {
    m_interfaceIndex = 0;
    m_settings.setValue("CanBusDriver/interfaceIndex", 0);
    Q_EMIT interfaceIndexChanged();
  }

  Q_EMIT availableInterfacesChanged();
  Q_EMIT interfaceSupportsFDChanged();
}

/**
 * @brief Refreshes the list of available CAN bus plugins, plus the interface list of the
 *        serial-port backends (cheap port diff; gs_usb refreshes via hot-plug events instead,
 *        because its enumeration opens every adapter to read serial strings).
 */
void IO::Drivers::CANBus::refreshPlugins()
{
  const QStringList currentPlugins = enumerateCanPlugins();

  if (m_pluginList != currentPlugins) {
    m_pluginList = currentPlugins;

    if (m_pluginIndex >= m_pluginList.count()) {
      m_pluginIndex = m_pluginList.isEmpty()
                      ? 0
                      : qMin(m_pluginIndex, static_cast<quint8>(m_pluginList.count() - 1));
      m_settings.setValue("CanBusDriver/pluginIndex", m_pluginIndex);
      Q_EMIT pluginIndexChanged();
    }

    Q_EMIT availablePluginsChanged();

    if (!m_pluginList.isEmpty() && m_pluginIndex < m_pluginList.count())
      refreshInterfaces();

    return;
  }

  if (m_pluginIndex >= m_pluginList.count())
    return;

  const QString plugin = m_pluginList[m_pluginIndex];
  if (plugin == IO::Drivers::GsUsbCanBackend::pluginKey())
    return;

  const auto* backend = IO::Drivers::CanBackends::find(plugin);
  if (backend && backend->availableInterfaces() != m_interfaceList)
    refreshInterfaces();
}

/**
 * @brief Checks if CAN bus support is available on this platform.
 */
bool IO::Drivers::CANBus::canSupportAvailable() const
{
  return !m_pluginList.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the CAN Bus configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::CANBus::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty plugin;
  plugin.key     = QStringLiteral("pluginIndex");
  plugin.label   = tr("Plugin");
  plugin.type    = IO::DriverProperty::ComboBox;
  plugin.value   = m_pluginIndex;
  plugin.options = m_pluginList;
  props.append(plugin);

  IO::DriverProperty iface;
  iface.key     = QStringLiteral("interfaceIndex");
  iface.label   = tr("Interface");
  iface.type    = IO::DriverProperty::ComboBox;
  iface.value   = m_interfaceIndex;
  iface.options = m_interfaceList;
  props.append(iface);

  IO::DriverProperty bitrate;
  bitrate.key   = QStringLiteral("bitrate");
  bitrate.label = tr("Bitrate");
  bitrate.type  = IO::DriverProperty::IntField;
  bitrate.value = m_bitrate;
  bitrate.min   = 10000;
  bitrate.max   = 1000000;
  props.append(bitrate);

  IO::DriverProperty canFd;
  canFd.key   = QStringLiteral("canFD");
  canFd.label = tr("CAN FD");
  canFd.type  = IO::DriverProperty::CheckBox;
  canFd.value = m_canFD;
  props.append(canFd);

  IO::DriverProperty dataBitrate;
  dataBitrate.key   = QStringLiteral("dataBitrate");
  dataBitrate.label = tr("Data Bitrate");
  dataBitrate.type  = IO::DriverProperty::IntField;
  dataBitrate.value = m_dataBitrate;
  dataBitrate.min   = 100000;
  dataBitrate.max   = 8000000;
  dataBitrate.showWhen(QStringLiteral("canFD"), {true});
  props.append(dataBitrate);

  IO::DriverProperty loopback;
  loopback.key   = QStringLiteral("loopback");
  loopback.label = tr("Loopback");
  loopback.type  = IO::DriverProperty::CheckBox;
  loopback.value = m_loopback;
  props.append(loopback);

  IO::DriverProperty listenOnly;
  listenOnly.key   = QStringLiteral("listenOnly");
  listenOnly.label = tr("Listen-Only");
  listenOnly.type  = IO::DriverProperty::CheckBox;
  listenOnly.value = m_listenOnly;
  props.append(listenOnly);

  IO::DriverProperty reassembly;
  reassembly.key   = QStringLiteral("tpReassembly");
  reassembly.label = tr("Multi-Frame Reassembly");
  reassembly.type  = IO::DriverProperty::CheckBox;
  reassembly.value = m_tpReassembly;
  props.append(reassembly);

  return props;
}

/**
 * @brief Applies a single CAN Bus configuration change by key. The bitrate value is overloaded:
 * a value >= 10000 is a raw bitrate in bits/s, anything smaller is treated as an index into
 * bitrateList().
 */
void IO::Drivers::CANBus::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("pluginIndex")) {
    setPluginIndex(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("interfaceIndex")) {
    setInterfaceIndex(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("canFD")) {
    setCanFD(value.toBool());
    return;
  }

  if (key == QLatin1String("dataBitrate")) {
    setDataBitrate(value.toUInt());
    return;
  }

  if (key == QLatin1String("loopback")) {
    setLoopback(value.toBool());
    return;
  }

  if (key == QLatin1String("listenOnly")) {
    setListenOnly(value.toBool());
    return;
  }

  if (key == QLatin1String("tpReassembly")) {
    setTpReassembly(value.toBool());
    return;
  }

  if (key != QLatin1String("bitrate"))
    return;

  const auto v = value.toUInt();
  if (v >= 10000) {
    setBitrate(static_cast<quint32>(v));
    return;
  }

  const auto list = bitrateList();
  const int idx   = static_cast<int>(v);
  if (idx >= 0 && idx < list.size())
    setBitrate(static_cast<quint32>(list.at(idx).toUInt()));
}

/**
 * @brief Returns a JSON identifier for the currently selected plugin and interface.
 */
QJsonObject IO::Drivers::CANBus::deviceIdentifier() const
{
  QJsonObject id;

  if (m_pluginIndex < m_pluginList.size())
    id.insert(QStringLiteral("plugin"), m_pluginList.at(m_pluginIndex));

  if (m_interfaceIndex < m_interfaceList.size())
    id.insert(QStringLiteral("interface"), m_interfaceList.at(m_interfaceIndex));

  return id;
}

/**
 * @brief Selects the plugin and interface matching a previously saved identifier.
 */
bool IO::Drivers::CANBus::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  bool matched = false;

  const auto saved_plugin = id.value(QStringLiteral("plugin")).toString();
  if (!saved_plugin.isEmpty()) {
    for (int i = 0; i < m_pluginList.size(); ++i) {
      if (m_pluginList.at(i) == saved_plugin) {
        setPluginIndex(static_cast<quint8>(i));
        matched = true;
        break;
      }
    }
  }

  const auto saved_iface = id.value(QStringLiteral("interface")).toString();
  if (!saved_iface.isEmpty()) {
    for (int i = 0; i < m_interfaceList.size(); ++i) {
      if (m_interfaceList.at(i) == saved_iface) {
        setInterfaceIndex(static_cast<quint8>(i));
        matched = true;
        break;
      }
    }
  }

  return matched;
}
