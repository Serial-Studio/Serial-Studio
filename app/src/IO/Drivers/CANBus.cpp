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

#include "Misc/Utilities.h"
#include "Misc/TimerEvents.h"
#include "IO/Drivers/CANBus.h"

#include <QCanBus>

//------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructor function
 *
 * Initializes the CAN bus driver with default settings and populates
 * the list of available CAN bus plugins.
 */
IO::Drivers::CANBus::CANBus()
  : m_device(nullptr)
  , m_canFD(false)
  , m_pluginIndex(0)
  , m_interfaceIndex(0)
  , m_bitrate(500000)
{
  m_pluginList = QCanBus::instance()->plugins();

  m_canFD = m_settings.value("CanBusDriver/canFD", false).toBool();
  m_bitrate = m_settings.value("CanBusDriver/bitrate", 500000).toUInt();
  m_pluginIndex = m_settings.value("CanBusDriver/pluginIndex", 0).toUInt();
  m_interfaceIndex
      = m_settings.value("CanBusDriver/interfaceIndex", 0).toUInt();

  if (!m_pluginList.isEmpty() && m_pluginIndex < m_pluginList.count())
    refreshInterfaces();

  connect(this, &IO::Drivers::CANBus::pluginIndexChanged, this,
          &IO::Drivers::CANBus::configurationChanged);
  connect(this, &IO::Drivers::CANBus::interfaceIndexChanged, this,
          &IO::Drivers::CANBus::configurationChanged);
}

/**
 * @brief Destructor function
 *
 * Safely closes the CAN bus device before exiting the application.
 * Uses same cleanup pattern as close() to prevent crashes.
 *
 * @note Disconnects all signals before deletion to prevent callbacks
 * @note Sets m_device to nullptr after deleteLater() for safety
 */
IO::Drivers::CANBus::~CANBus()
{
  if (m_device)
  {
    disconnect(m_device, nullptr, this, nullptr);

    if (m_device->state() == QCanBusDevice::ConnectedState)
      m_device->disconnectDevice();

    m_device->deleteLater();
    m_device = nullptr;
  }
}

/**
 * @brief Returns the only instance of the class
 */
IO::Drivers::CANBus &IO::Drivers::CANBus::instance()
{
  static CANBus singleton;
  return singleton;
}

//------------------------------------------------------------------------------
// HAL-driver implementation
//------------------------------------------------------------------------------

/**
 * @brief Closes the current CAN bus connection
 *
 * Performs complete cleanup of CAN bus device and resources:
 * - Disconnects all signal connections to prevent crashes
 * - Safely disconnects from CAN bus
 * - Deletes device using deleteLater() for thread safety
 * - Clears device reference
 * - Notifies Manager of configuration and list changes
 *
 * @note This method is safe to call multiple times
 * @note All Qt objects are deleted using deleteLater() to prevent crashes
 */
void IO::Drivers::CANBus::close()
{
  if (m_device)
  {
    disconnect(m_device, nullptr, this, nullptr);

    if (m_device->state() != QCanBusDevice::UnconnectedState)
      m_device->disconnectDevice();

    m_device->deleteLater();
    m_device = nullptr;
  }

  Q_EMIT configurationChanged();
  Q_EMIT availablePluginsChanged();
  Q_EMIT availableInterfacesChanged();
}

/**
 * @brief Returns true if a CAN bus connection is currently open
 */
bool IO::Drivers::CANBus::isOpen() const
{
  if (m_device)
    return m_device->state() == QCanBusDevice::ConnectedState;

  return false;
}

/**
 * @brief Returns true if the current CAN bus device is readable
 */
bool IO::Drivers::CANBus::isReadable() const
{
  return isOpen();
}

/**
 * @brief Returns true if the current CAN bus device is writable
 */
bool IO::Drivers::CANBus::isWritable() const
{
  return isOpen();
}

/**
 * @brief Returns true if the user has selected appropriate controls & options
 *        to connect to a CAN bus device
 */
bool IO::Drivers::CANBus::configurationOk() const
{
  if (!canSupportAvailable())
    return false;

  return m_pluginIndex < m_pluginList.count() && !m_interfaceList.isEmpty()
         && m_interfaceIndex < m_interfaceList.count();
}

/**
 * @brief Writes data to the CAN bus device
 *
 * Safely transmits a CAN frame to the bus. The input data must be in the
 * format: [ID_high, ID_low, DLC, data0, data1, ..., dataN]
 *
 * ## Frame Format
 * - Bytes 0-1: 16-bit CAN identifier (big-endian)
 * - Byte 2: Data Length Code (DLC, 0-8 for standard CAN, 0-64 for CAN FD)
 * - Bytes 3+: Payload data
 *
 * ## Safety Features
 * - Validates device is open and writable
 * - Validates minimum data length (3 bytes)
 * - Validates device pointer before writing
 * - Clamps DLC to valid range (0-8 for standard, 0-64 for CAN FD)
 * - Uses try/catch to prevent crashes during frame construction
 * - Validates write operation success
 *
 * @param data The data to write (CAN frame data)
 * @return Number of bytes successfully written (original data length), or 0 on
 * failure
 *
 * @note Returns 0 if device is not writable or data is invalid
 * @note DLC values exceeding valid range are automatically clamped
 * @note Emits dataSent signal only on successful transmission
 */
quint64 IO::Drivers::CANBus::write(const QByteArray &data)
{
  if (!m_device)
    return 0;

  if (!isWritable())
    return 0;

  if (data.length() < 3)
    return 0;

  try
  {
    quint32 can_id
        = (static_cast<quint8>(data[0]) << 8) | static_cast<quint8>(data[1]);

    quint8 dlc = static_cast<quint8>(data[2]);
    quint8 max_dlc = m_canFD ? 64 : 8;

    if (dlc > max_dlc)
      dlc = max_dlc;

    QByteArray payload = data.mid(3, dlc);

    QCanBusFrame frame(can_id, payload);
    frame.setExtendedFrameFormat(m_canFD);

    if (m_device->writeFrame(frame))
    {
      Q_EMIT dataSent(data);
      return data.length();
    }
  }
  catch (...)
  {
  }

  return 0;
}

/**
 * @brief Opens the CAN bus device with the given mode
 *
 * Performs complete initialization of CAN bus connection:
 * - Closes any existing connection first (clean slate)
 * - Validates platform CAN support availability
 * - Checks configuration parameters are valid
 * - Creates QCanBusDevice using selected plugin and interface
 * - Configures bitrate and CAN FD mode if enabled
 * - Connects signal handlers with Qt::UniqueConnection
 * - Attempts device connection
 * - Handles errors with proper cleanup
 *
 * @param mode The open mode (ReadOnly or ReadWrite, currently unused)
 * @return True if successfully opened and connected
 *
 * @note Uses Qt::UniqueConnection to prevent duplicate signal connections
 * @note Always calls close() first to ensure clean state
 * @note Emits platform-specific error messages when CAN support unavailable
 */
bool IO::Drivers::CANBus::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  close();

  if (!canSupportAvailable())
  {
#if defined(Q_OS_LINUX)
    Misc::Utilities::showMessageBox(
        tr("CAN Bus Not Available"),
        tr("No CAN bus plugins found on this system.\n\nOn Linux, please "
           "ensure SocketCAN kernel modules are loaded."),
        QMessageBox::Critical);
#elif defined(Q_OS_WIN)
    Misc::Utilities::showMessageBox(
        tr("CAN Bus Not Available"),
        tr("No CAN bus plugins found on this system.\n\nOn Windows, please "
           "install CAN hardware drivers (PEAK, Vector, etc.)."),
        QMessageBox::Critical);
#elif defined(Q_OS_MAC)
    Misc::Utilities::showMessageBox(
        tr("CAN Bus Not Available"),
        tr("No CAN bus plugins found on this system.\n\nCAN bus support on "
           "macOS is limited and may require third-party hardware drivers."),
        QMessageBox::Critical);
#else
    Misc::Utilities::showMessageBox(
        tr("CAN Bus Not Available"),
        tr("No CAN bus plugins are available on this platform."),
        QMessageBox::Critical);
#endif
    return false;
  }

  if (!configurationOk())
  {
    Misc::Utilities::showMessageBox(
        tr("Invalid CAN Configuration"),
        tr("The CAN bus configuration is incomplete. Please select a valid "
           "plugin and interface."),
        QMessageBox::Critical);
    return false;
  }

  if (m_pluginIndex >= m_pluginList.count()
      || m_interfaceIndex >= m_interfaceList.count())
  {
    Misc::Utilities::showMessageBox(
        tr("Invalid Selection"),
        tr("The selected plugin or interface is no longer available. Please "
           "refresh the lists and try again."),
        QMessageBox::Critical);
    return false;
  }

  if (m_pluginList.isEmpty() || m_interfaceList.isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("No Devices Available"),
        tr("The plugin or interface list is empty. Please refresh the lists "
           "and ensure your CAN hardware is connected."),
        QMessageBox::Critical);
    return false;
  }

  QString plugin = m_pluginList.at(m_pluginIndex);
  QString interface = m_interfaceList.at(m_interfaceIndex);

  QString error;
  m_device = QCanBus::instance()->createDevice(plugin, interface, &error);

  if (!m_device)
  {
    Misc::Utilities::showMessageBox(
        tr("CAN Device Creation Failed"),
        error.isEmpty() ? tr("Unable to create CAN bus device. Please check "
                             "your hardware and drivers.")
                        : error,
        QMessageBox::Critical);
    return false;
  }

  m_device->setConfigurationParameter(QCanBusDevice::BitRateKey, m_bitrate);
  if (m_canFD)
    m_device->setConfigurationParameter(QCanBusDevice::CanFdKey, true);

  connect(m_device, &QCanBusDevice::framesReceived, this,
          &IO::Drivers::CANBus::onFramesReceived, Qt::UniqueConnection);
  connect(m_device, &QCanBusDevice::stateChanged, this,
          &IO::Drivers::CANBus::onStateChanged, Qt::UniqueConnection);
  connect(m_device, &QCanBusDevice::errorOccurred, this,
          &IO::Drivers::CANBus::onErrorOccurred, Qt::UniqueConnection);

  if (!m_device->connectDevice())
  {
    error = m_device->errorString();
    m_device->deleteLater();
    m_device = nullptr;
    Misc::Utilities::showMessageBox(
        tr("CAN Connection Failed"),
        error.isEmpty()
            ? tr("Unable to connect to CAN bus device. Please check your "
                 "hardware connection and settings.")
            : error,
        QMessageBox::Critical);
    return false;
  }

  Q_EMIT configurationChanged();
  return true;
}

//------------------------------------------------------------------------------
// Property getters
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Property setters
//------------------------------------------------------------------------------

/**
 * @brief Sets whether CAN FD mode is enabled
 */
void IO::Drivers::CANBus::setCanFD(const bool enabled)
{
  m_canFD = enabled;
  m_settings.setValue("CanBusDriver/canFD", enabled);
  Q_EMIT canFDChanged();
}

/**
 * @brief Sets the bitrate for the CAN bus
 */
void IO::Drivers::CANBus::setBitrate(const quint32 bitrate)
{
  m_bitrate = bitrate;
  m_settings.setValue("CanBusDriver/bitrate", bitrate);
  Q_EMIT bitrateChanged();
}

/**
 * @brief Sets the plugin index and refreshes available interfaces
 */
void IO::Drivers::CANBus::setPluginIndex(const quint8 index)
{
  if (index < m_pluginList.count())
  {
    m_pluginIndex = index;
    m_settings.setValue("CanBusDriver/pluginIndex", index);
    refreshInterfaces();
    Q_EMIT pluginIndexChanged();
  }
}

/**
 * @brief Sets the interface index
 */
void IO::Drivers::CANBus::setInterfaceIndex(const quint8 index)
{
  if (index < m_interfaceList.count())
  {
    m_interfaceIndex = index;
    m_settings.setValue("CanBusDriver/interfaceIndex", index);
    Q_EMIT interfaceIndexChanged();
  }
}

/**
 * @brief Sets up external connections for timer events
 *
 * Connects to the 1Hz timer to periodically refresh the list of
 * available CAN bus plugins and interfaces.
 */
void IO::Drivers::CANBus::setupExternalConnections()
{
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &IO::Drivers::CANBus::refreshPlugins);
}

//------------------------------------------------------------------------------
// Private slots
//------------------------------------------------------------------------------

/**
 * @brief Handles incoming CAN bus frames
 *
 * Safely processes all available CAN frames and converts them to the format
 * expected by Serial Studio's frame parser.
 *
 * ## Frame Format
 * Output byte array format: [ID_high, ID_low, DLC, data0, data1, ..., data7]
 * - ID_high, ID_low: 16-bit CAN identifier (big-endian)
 * - DLC: Data Length Code (0-8 bytes)
 * - data0..data7: Payload bytes (padded with zeros to 11 total bytes)
 *
 * ## Safety Features
 * - Validates device pointer before processing
 * - Checks frame validity before conversion
 * - Uses try/catch to prevent crashes during data processing
 * - Validates payload size before accessing
 * - Pads short frames to ensure consistent length
 *
 * @note Called automatically when CAN frames are received
 * @note Silently skips invalid frames without emitting errors
 * @note All frames are padded to 11 bytes total (3 header + 8 data)
 */
void IO::Drivers::CANBus::onFramesReceived()
{
  if (!m_device)
    return;

  if (!isOpen())
    return;

  try
  {
    while (m_device->framesAvailable() > 0)
    {
      const QCanBusFrame frame = m_device->readFrame();

      if (!frame.isValid())
        continue;

      const QByteArray payload = frame.payload();
      if (payload.size() > 64)
        continue;

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

      Q_EMIT dataReceived(data);
    }
  }
  catch (...)
  {
  }
}

/**
 * @brief Handles CAN bus device state changes
 *
 * Monitors the CAN bus device connection state and emits configurationChanged()
 * signal to notify IO::Manager of state transitions.
 *
 * ## State Transitions
 * - UnconnectedState: Device is disconnected
 * - ConnectingState: Connection attempt in progress
 * - ConnectedState: Device successfully connected and operational
 * - ClosingState: Device is being closed
 *
 * ## Critical for UI Updates
 * This slot is ESSENTIAL for Serial Studio's UI to properly reflect the
 * connection state. The signal chain is:
 * 1. QCanBusDevice state changes
 * 2. onStateChanged() emits configurationChanged()
 * 3. IO::Manager catches configurationChanged()
 * 4. IO::Manager emits connectedChanged()
 * 5. UI updates connect/disconnect button state
 *
 * @param state The new CAN bus device state
 *
 * @note This slot is connected to QCanBusDevice::stateChanged signal
 * @note Always emits configurationChanged() on any state change
 * @note State changes occur asynchronously after open() returns
 */
void IO::Drivers::CANBus::onStateChanged(QCanBusDevice::CanBusDeviceState state)
{
  Q_UNUSED(state)
  Q_EMIT configurationChanged();
}

/**
 * @brief Handles CAN bus errors
 *
 * Safely processes CAN bus device errors and shows appropriate error messages.
 *
 * ## Error Handling
 * - Ignores NoError events (normal operation)
 * - Validates device pointer before accessing error string
 * - Provides fallback error message if device is null
 * - Shows message box to inform user of error
 *
 * ## Error Types
 * - ReadError: Error reading from the device
 * - WriteError: Error writing to the device
 * - ConnectionError: Device connection lost
 * - ConfigurationError: Invalid device configuration
 * - UnknownError: Unspecified error condition
 *
 * @param error The CAN bus error code
 *
 * @note This slot is connected to QCanBusDevice::errorOccurred signal
 * @note Always validates m_device before accessing error strings
 */
void IO::Drivers::CANBus::onErrorOccurred(QCanBusDevice::CanBusError error)
{
  if (error == QCanBusDevice::NoError)
    return;

  if (!m_device)
  {
    Misc::Utilities::showMessageBox(
        tr("CAN Bus Error"),
        tr("An error occurred but the CAN device is no longer available."),
        QMessageBox::Warning);
    return;
  }

  QString error_string = m_device->errorString();
  if (error_string.isEmpty())
    error_string = tr("Error code: %1").arg(error);

  Misc::Utilities::showMessageBox(tr("CAN Bus Communication Error"),
                                  error_string, QMessageBox::Warning);
}

//------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------

/**
 * @brief Refreshes the list of available CAN bus interfaces
 *
 * Called when the plugin selection changes.
 */
void IO::Drivers::CANBus::refreshInterfaces()
{
  m_interfaceList.clear();

  if (m_pluginList.isEmpty() || m_pluginIndex >= m_pluginList.count())
    return;

  QString plugin = m_pluginList[m_pluginIndex];
  QString error;

  const QList<QCanBusDeviceInfo> interfaces
      = QCanBus::instance()->availableDevices(plugin, &error);

  for (const QCanBusDeviceInfo &info : interfaces)
    m_interfaceList.append(info.name());

  if (m_interfaceIndex >= m_interfaceList.count())
  {
    m_interfaceIndex = 0;
    m_settings.setValue("CanBusDriver/interfaceIndex", 0);
  }

  Q_EMIT availableInterfacesChanged();
}

/**
 * @brief Refreshes the list of available CAN bus plugins
 *
 * Scans for available CAN bus plugins and updates the internal lists.
 * This is called periodically to detect newly loaded plugins.
 */
void IO::Drivers::CANBus::refreshPlugins()
{
  const QStringList currentPlugins = QCanBus::instance()->plugins();

  if (m_pluginList != currentPlugins)
  {
    m_pluginList = currentPlugins;
    Q_EMIT availablePluginsChanged();

    if (!m_pluginList.isEmpty() && m_pluginIndex < m_pluginList.count())
      refreshInterfaces();

    else if (m_pluginIndex >= m_pluginList.count())
    {
      m_pluginIndex = 0;
      m_settings.setValue("CanBusDriver/pluginIndex", 0);
      Q_EMIT pluginIndexChanged();
    }
  }
}

/**
 * @brief Checks if CAN bus support is available on this platform
 *
 * @return True if at least one CAN bus plugin is available
 */
bool IO::Drivers::CANBus::canSupportAvailable() const
{
  return !m_pluginList.isEmpty();
}
