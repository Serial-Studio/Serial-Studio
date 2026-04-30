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

#include <QCanBus>
#include <QLoggingCategory>

#include "Misc/TimerEvents.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CANBus driver and restores persisted settings.
 */
IO::Drivers::CANBus::CANBus()
  : m_device(nullptr), m_canFD(false), m_pluginIndex(0), m_interfaceIndex(0), m_bitrate(500000)
{
  m_pluginList = QCanBus::instance()->plugins();

  m_canFD          = m_settings.value("CanBusDriver/canFD", false).toBool();
  m_bitrate        = m_settings.value("CanBusDriver/bitrate", 500000).toUInt();
  m_pluginIndex    = m_settings.value("CanBusDriver/pluginIndex", 0).toUInt();
  m_interfaceIndex = m_settings.value("CanBusDriver/interfaceIndex", 0).toUInt();

  if (!m_pluginList.isEmpty() && m_pluginIndex < m_pluginList.count())
    refreshInterfaces();

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
  connect(
    this, &IO::Drivers::CANBus::canFDChanged, this, &IO::Drivers::CANBus::configurationChanged);

  QLoggingCategory::setFilterRules("qt.canbus* = false");
}

/**
 * @brief Closes the CAN bus device.
 */
IO::Drivers::CANBus::~CANBus()
{
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
 * @brief Writes a CAN frame ([ID_hi, ID_lo, DLC, payload...]) to the bus.
 */
qint64 IO::Drivers::CANBus::write(const QByteArray& data)
{
  if (!m_device)
    return 0;

  if (!isWritable())
    return 0;

  if (data.length() < 3)
    return 0;

  // Parse CAN ID, DLC, and payload from the input buffer
  try {
    quint32 can_id = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);

    quint8 dlc     = static_cast<quint8>(data[2]);
    quint8 max_dlc = m_canFD ? 64 : 8;

    if (dlc > max_dlc)
      dlc = max_dlc;

    QByteArray payload = data.mid(3, dlc);

    QCanBusFrame frame(can_id, payload);

    // Extended ID (29-bit) and CAN FD (rate-switch) are orthogonal flags.
    if (can_id > 0x7FF)
      frame.setExtendedFrameFormat(true);

    if (m_canFD)
      frame.setFlexibleDataRateFormat(true);

    if (m_device->writeFrame(frame)) {
      Q_EMIT dataSent(data);
      return data.length();
    }
  } catch (...) {
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

  // Abort if no CAN bus plugins are available
  if (!canSupportAvailable()) {
#if defined(Q_OS_LINUX)
    Misc::Utilities::showMessageBox(
      tr("CAN Bus Not Available"),
      tr(
        "No CAN bus plugins found on this system.\n\nOn Linux, ensure SocketCAN kernel modules are loaded."),
      QMessageBox::Critical);
#elif defined(Q_OS_WIN)
    Misc::Utilities::showMessageBox(
      tr("CAN Bus Not Available"),
      tr(
        "No CAN bus plugins found on this system.\n\nOn Windows, install CAN hardware drivers (PEAK, Vector, etc.)."),
      QMessageBox::Critical);
#elif defined(Q_OS_MAC)
    Misc::Utilities::showMessageBox(
      tr("CAN Bus Not Available"),
      tr(
        "No CAN bus plugins found on this system.\n\nCAN bus support on macOS is limited and may require third-party hardware drivers."),
      QMessageBox::Critical);
#else
    Misc::Utilities::showMessageBox(tr("CAN Bus Not Available"),
                                    tr("No CAN bus plugins are available on this platform."),
                                    QMessageBox::Critical);
#endif
    return false;
  }

  // Validate configuration before connecting
  if (!configurationOk()) {
    Misc::Utilities::showMessageBox(
      tr("Invalid CAN Configuration"),
      tr("The CAN bus configuration is incomplete. Select a valid plugin and interface."),
      QMessageBox::Critical);
    return false;
  }

  if (m_pluginIndex >= m_pluginList.count() || m_interfaceIndex >= m_interfaceList.count()) {
    Misc::Utilities::showMessageBox(
      tr("Invalid Selection"),
      tr(
        "The selected plugin or interface is no longer available. Refresh the lists and try again."),
      QMessageBox::Critical);
    return false;
  }

  if (m_pluginList.isEmpty() || m_interfaceList.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("No Devices Available"),
      tr(
        "The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected."),
      QMessageBox::Critical);
    return false;
  }

  QString plugin    = m_pluginList.at(m_pluginIndex);
  QString interface = m_interfaceList.at(m_interfaceIndex);

  QString error;
  m_device = QCanBus::instance()->createDevice(plugin, interface, &error);

  if (!m_device) {
    Misc::Utilities::showMessageBox(
      tr("CAN Device Creation Failed"),
      error.isEmpty() ? tr("Unable to create CAN bus device. Check your hardware and drivers.")
                      : error,
      QMessageBox::Critical);
    return false;
  }

  // Configure bitrate and CAN FD mode
  m_device->setConfigurationParameter(QCanBusDevice::BitRateKey, m_bitrate);
  if (m_canFD)
    m_device->setConfigurationParameter(QCanBusDevice::CanFdKey, true);

  // Connect state and error signals
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

  // Attempt connection
  if (!m_device->connectDevice()) {
    error = m_device->errorString();
    m_device->deleteLater();
    m_device = nullptr;
    Misc::Utilities::showMessageBox(
      tr("CAN Connection Failed"),
      error.isEmpty()
        ? tr("Unable to connect to CAN bus device. Check your hardware connection and settings.")
        : error,
      QMessageBox::Critical);
    return false;
  }

  Q_EMIT configurationChanged();
  return true;
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
 * @brief Converts a Qt CAN plugin name to a user-friendly display name.
 */
QString IO::Drivers::CANBus::pluginDisplayName(const QString& plugin) const
{
  if (plugin == "socketcan")
    return "SocketCAN";
  else if (plugin == "peakcan")
    return "PEAK CAN";
  else if (plugin == "passthroughcan" || plugin == "passthrucan")
    return "PassThru CAN";
  else if (plugin == "virtualcan")
    return "Virtual CAN";
  else if (plugin == "systeccan")
    return "SysTec CAN";
  else if (plugin == "tinycan")
    return "Tiny CAN";
  else if (plugin == "vectorcan")
    return "Vector CAN";
  else
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
  }
}

/**
 * @brief Sets up external connections for timer events.
 */
void IO::Drivers::CANBus::setupExternalConnections()
{
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &IO::Drivers::CANBus::refreshPlugins);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles incoming CAN bus frames and publishes them as [ID_hi, ID_lo, DLC, payload...].
 */
void IO::Drivers::CANBus::onFramesReceived()
{
  if (!m_device)
    return;

  if (!isOpen())
    return;

  // Process all available frames from the CAN bus device
  try {
    while (m_device->framesAvailable() > 0) {
      const QCanBusFrame frame = m_device->readFrame();

      if (!frame.isValid())
        continue;

      // Skip oversized payloads
      const QByteArray payload = frame.payload();
      if (payload.size() > 64)
        continue;

      // Build output: [ID_hi, ID_lo, DLC, data...] padded to 11 bytes
      QByteArray data;
      data.reserve(11);

      quint32 can_id = frame.frameId();
      data.append(static_cast<char>((can_id >> 8) & 0xFF));
      data.append(static_cast<char>(can_id & 0xFF));

      quint8 dlc = static_cast<quint8>(payload.size());
      data.append(static_cast<char>(dlc));

      data.append(payload);

      while (data.size() < 11)
        data.append(static_cast<char>(0));

      publishReceivedData(std::move(data));
    }
  } catch (...) {
  }
}

/**
 * @brief Handles CAN bus device state changes by emitting configurationChanged().
 */
void IO::Drivers::CANBus::onStateChanged(QCanBusDevice::CanBusDeviceState state)
{
  Q_UNUSED(state)
  Q_EMIT configurationChanged();
}

/**
 * @brief Handles CAN bus errors by showing a message box.
 */
void IO::Drivers::CANBus::onErrorOccurred(QCanBusDevice::CanBusError error)
{
  if (error == QCanBusDevice::NoError)
    return;

  if (!m_device) {
    Misc::Utilities::showMessageBox(
      tr("CAN Bus Error"),
      tr("An error occurred but the CAN device is no longer available."),
      QMessageBox::Warning);
    return;
  }

  QString error_string = m_device->errorString();
  if (error_string.isEmpty())
    error_string = tr("Error code: %1").arg(error);

  Misc::Utilities::showMessageBox(
    tr("CAN Bus Communication Error"), error_string, QMessageBox::Warning);
}

//--------------------------------------------------------------------------------------------------
// Private methods
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refreshes the list of available CAN bus interfaces.
 */
void IO::Drivers::CANBus::refreshInterfaces()
{
  m_interfaceList.clear();
  m_interfaceError.clear();

  // Abort if no valid plugin is selected
  if (m_pluginList.isEmpty() || m_pluginIndex >= m_pluginList.count()) {
    m_interfaceError = tr("No CAN driver selected");
    Q_EMIT interfaceErrorChanged();
    Q_EMIT availableInterfacesChanged();
    return;
  }

  // Query available interfaces from the selected plugin
  QString plugin = m_pluginList[m_pluginIndex];
  QString error;

  const QList<QCanBusDeviceInfo> interfaces = QCanBus::instance()->availableDevices(plugin, &error);

  if (!error.isEmpty())
    qWarning() << "CAN plugin error:" << plugin << error;

  for (const QCanBusDeviceInfo& info : interfaces)
    m_interfaceList.append(info.name());

  // Provide a platform-specific hint when no interfaces are found
  if (m_interfaceList.isEmpty() && m_interfaceError.isEmpty()) {
    const QString driverName = pluginDisplayName(plugin);

#if defined(Q_OS_LINUX)
    if (plugin == "socketcan")
      m_interfaceError = tr("Load SocketCAN kernel modules first");
    else if (plugin == "virtualcan")
      m_interfaceError = tr("Set up a virtual CAN interface first");
    else
      m_interfaceError = tr("No interfaces found for %1").arg(driverName);

#elif defined(Q_OS_WIN)
    if (plugin == "peakcan") {
      m_interfaceError = tr(
        "Install <a href='https://www.peak-system.com/Drivers.523.0.html?&L=1'>PEAK CAN drivers</a>");
    } else if (plugin == "vectorcan") {
      m_interfaceError = tr(
        "Install <a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'>Vector CAN drivers</a>");
    } else if (plugin == "systeccan") {
      m_interfaceError = tr(
        "Install <a href='https://www.systec-electronic.com/en/company/support/driver'>SysTec CAN drivers</a>");
    } else
      m_interfaceError = tr("Install %1 drivers").arg(driverName);

#elif defined(Q_OS_MAC)
    m_interfaceError = tr("Install %1 drivers for macOS").arg(driverName);
#else
    m_interfaceError = tr("No interfaces found for %1").arg(driverName);
#endif
    Q_EMIT interfaceErrorChanged();
  }

  // Clamp index if the selected interface was removed
  if (m_interfaceIndex >= m_interfaceList.count()) {
    m_interfaceIndex = 0;
    m_settings.setValue("CanBusDriver/interfaceIndex", 0);
  }

  Q_EMIT availableInterfacesChanged();
}

/**
 * @brief Refreshes the list of available CAN bus plugins.
 */
void IO::Drivers::CANBus::refreshPlugins()
{
  const QStringList currentPlugins = QCanBus::instance()->plugins();

  // Update only if the plugin list changed
  if (m_pluginList != currentPlugins) {
    m_pluginList = currentPlugins;

    // Clamp index if the selected plugin was removed
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
  }
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

  return props;
}

/**
 * @brief Applies a single CAN Bus configuration change by key.
 */
void IO::Drivers::CANBus::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("pluginIndex"))
    setPluginIndex(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("interfaceIndex"))
    setInterfaceIndex(static_cast<quint8>(value.toInt()));

  else if (key == QLatin1String("bitrate")) {
    const auto v = value.toUInt();
    if (v >= 10000)
      setBitrate(static_cast<quint32>(v));
    else {
      const auto list = bitrateList();
      const int idx   = static_cast<int>(v);
      if (idx >= 0 && idx < list.size())
        setBitrate(static_cast<quint32>(list.at(idx).toUInt()));
    }
  }

  else if (key == QLatin1String("canFD"))
    setCanFD(value.toBool());
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
