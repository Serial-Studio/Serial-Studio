/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "IO/Manager.h"
#include "IO/Drivers/UART.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/BluetoothLE.h"
#include "Misc/Translator.h"

#include <QApplication>

/**
 * @brief Constructs the `Manager` instance.
 *
 * Initializes the I/O manager with default settings, sets up the `FrameReader`
 * in a separate thread, and connects relevant signals for data and
 * configuration management.
 *
 * By default, the manager is configured for serial communication.
 */
IO::Manager::Manager()
  : m_paused(false)
  , m_writeEnabled(true)
  , m_driver(nullptr)
  , m_startSequence(QByteArray("/*"))
  , m_finishSequence(QByteArray("*/"))
{
  // Move the frame parser worker to its dedicated thread
  m_frameReader.moveToThread(&m_workerThread);

  // Automatically enable/disable the connect button when bus type changes
  connect(this, &IO::Manager::busTypeChanged, this,
          &IO::Manager::configurationChanged);

  // Automatically check/unckeck connected button when configuration changes
  connect(this, &IO::Manager::configurationChanged, this,
          &IO::Manager::connectedChanged);

  // Avoid crashing the app when quitting
  connect(qApp, &QApplication::aboutToQuit, this, [=] {
    disconnect(&m_frameReader);
    m_workerThread.quit();
    if (!m_workerThread.wait(100))
      m_workerThread.terminate();
  });

  // Start the worker thread
  m_workerThread.start(QThread::HighestPriority);

  // Set default data interface to serial port
  setBusType(SerialStudio::BusType::UART);
}

/**
 * @brief Retrieves the singleton instance of the `Manager`.
 *
 * This function implements the singleton pattern, ensuring that there is only
 * one instance of the `Manager` in the application.
 *
 * @return A reference to the `Manager` instance.
 */
IO::Manager &IO::Manager::instance()
{
  static Manager instance;
  return instance;
}

/**
 * @brief Indicates whether data streaming is currently paused while the data
 *        source remains connected.
 *
 * This state is similar to pressing the "HOLD" button on an oscilloscope—data
 * acquisition is temporarily halted, but the input connection is still active.
 *
 * @return true if streaming is paused but the source is still connected;
 *              false otherwise.
 */
bool IO::Manager::paused()
{
  return isConnected() && m_paused;
}

/**
 * @brief Checks if the Manager is in read-only mode.
 *
 * Read-only mode is determined by whether the device is isConnected and
 * write operations are disabled.
 *
 * @return True if the Manager is in read-only mode, false otherwise.
 */
bool IO::Manager::readOnly()
{
  return isConnected() && !m_writeEnabled;
}

/**
 * @brief Checks if the Manager is in read-write mode.
 *
 * Read-write mode is determined by whether the device is isConnected and
 * write operations are enabled.
 *
 * @return True if the Manager is in read-write mode, false otherwise.
 */
bool IO::Manager::readWrite()
{
  return isConnected() && m_writeEnabled;
}

/**
 * @brief Checks if the Manager is isConnected to a device.
 *
 * Determines the connection state by checking the driver status or the MQTT
 * subscription status.
 *
 * @return True if a device is isConnected or subscribed, false otherwise.
 */
bool IO::Manager::isConnected()
{
  if (driver())
    return driver()->isOpen();

  return false;
}

/**
 * @brief Validates the current configuration of the Manager.
 *
 * Checks the configuration validity of the active driver. Returns false
 * if no driver is present or the configuration is invalid.
 *
 * @return True if the configuration is valid, false otherwise.
 */
bool IO::Manager::configurationOk()
{
  if (driver())
    return driver()->configurationOk();

  return false;
}

/**
 * @brief Retrieves the current hardware abstraction layer (HAL) driver.
 *
 * The HAL driver is responsible for interfacing with the connected I/O device.
 *
 * @return A pointer to the active `HAL_Driver` instance, or nullptr if none is
 * active.
 */
IO::HAL_Driver *IO::Manager::driver()
{
  return m_driver;
}

/**
 * @brief Retrieves the current bus type used by the Manager.
 *
 * The bus type determines the communication medium (e.g., Serial, Network,
 * Bluetooth).
 *
 * @return The current bus type as a `SerialStudio::BusType` enum.
 */
SerialStudio::BusType IO::Manager::busType() const
{
  return m_busType;
}

/**
 * @brief Retrieves the start sequence used for frame detection.
 *
 * The start sequence is used to identify the beginning of frames in the data
 * stream.
 *
 * @return A reference to the start sequence string.
 */
const QByteArray &IO::Manager::startSequence() const
{
  return m_startSequence;
}

/**
 * @brief Retrieves the finish sequence used for frame detection.
 *
 * The finish sequence is used to identify the end of frames in the data stream.
 *
 * @return A reference to the finish sequence string.
 */
const QByteArray &IO::Manager::finishSequence() const
{
  return m_finishSequence;
}

/**
 * @brief Retrieves a list of available bus types.
 *
 * Provides a list of all supported communication mediums, including Serial,
 * Network, and Bluetooth LE.
 *
 * @return A list of available bus types as strings.
 */
QStringList IO::Manager::availableBuses() const
{
  QStringList list;
  list.append(tr("UART/COM"));
  list.append(tr("Network Socket"));
  list.append(tr("Bluetooth LE"));
#ifdef USE_QT_COMMERCIAL
  // Comment these ports for v3.0.7 release...I will add support for these
  // IO modules later, right now I need testing data to not drown in issues
  // and get the FlatHub publishing done ASAP.
  // list.append(tr("Modbus"));
  // list.append(tr("CAN Bus"));
#endif
  return list;
}

/**
 * @brief Writes data to the isConnected device.
 *
 * Sends the specified data to the isConnected device through the active driver.
 * Emits the `dataSent` signal upon successful transmission.
 *
 * @param data The data to be written.
 * @return The number of bytes written, or -1 if an error occurs or no device is
 *         isConnected.
 */
qint64 IO::Manager::writeData(const QByteArray &data)
{
  if (isConnected())
  {
    const auto bytes = driver()->write(data);

    if (bytes > 0)
    {
      auto writtenData = data;
      writtenData.chop(data.length() - bytes);
      Q_EMIT dataSent(writtenData);
    }

    return bytes;
  }

  return -1;
}

/**
 * @brief Toggles the connection state of the Manager.
 *
 * If the Manager is currently isConnected, it disconnects the device. If it is
 * not isConnected, it attempts to establish a connection.
 */
void IO::Manager::toggleConnection()
{
  if (isConnected())
    disconnectDevice();
  else
    connectDevice();
}

/**
 * @brief Connects to the configured device.
 *
 * Attempts to open the device in the appropriate mode (read-only or read-write)
 * and sets up the `FrameReader` to process incoming data. Emits a signal to
 * update the connection state.
 */
void IO::Manager::connectDevice()
{
  // Configure current device
  if (driver() && m_workerThread.isRunning())
  {
    // Set open flag
    QIODevice::OpenMode mode = QIODevice::ReadOnly;
    if (m_writeEnabled)
      mode = QIODevice::ReadWrite;

    // Open device & instruct frame reader to obtain data from it
    if (driver()->open(mode))
    {
      setPaused(false);
      QMetaObject::invokeMethod(&m_frameReader, &FrameReader::reset,
                                Qt::BlockingQueuedConnection);

      connect(driver(), &IO::HAL_Driver::dataReceived, &m_frameReader,
              &FrameReader::processData, Qt::QueuedConnection);
      connect(
          &m_frameReader, &IO::FrameReader::frameReady, this,
          [this](const QByteArray &frame) {
            if (!paused())
              Q_EMIT frameReceived(frame);
          },
          Qt::QueuedConnection);
      connect(
          &m_frameReader, &IO::FrameReader::dataReceived, this,
          [this](const QByteArray &data) {
            if (!paused())
              Q_EMIT dataReceived(data);
          },
          Qt::QueuedConnection);
    }

    // Error opening the device
    else
      disconnectDevice();

    // Enqueue a UI update request
    QMetaObject::invokeMethod(
        this, [=] { Q_EMIT connectedChanged(); }, Qt::QueuedConnection);
  }
}

/**
 * @brief Disconnects from the current device.
 *
 * Closes the connection to the device, clears the frame reader buffer, and
 * disconnects any associated signals. Emits signals to update the UI and
 * reflect the new state.
 */
void IO::Manager::disconnectDevice()
{
  if (driver())
  {
    // Close driver device
    driver()->close();
    setPaused(false);

    // Disconnect frame reader
    if (m_workerThread.isRunning())
    {
      QMetaObject::invokeMethod(&m_frameReader, &FrameReader::reset,
                                Qt::BlockingQueuedConnection);

      disconnect(&m_frameReader);
      disconnect(driver(), &IO::HAL_Driver::dataReceived, &m_frameReader,
                 &FrameReader::processData);
    }

    // Enqueue a UI update request
    QMetaObject::invokeMethod(
        this,
        [=] {
          Q_EMIT driverChanged();
          Q_EMIT connectedChanged();
        },
        Qt::QueuedConnection);
  }
}

/**
 * @brief Sets up external connections for the Manager.
 *
 * Connects external components, such as the translator for bus list updates and
 * the frame reader for parameter synchronization.
 */
void IO::Manager::setupExternalConnections()
{
  // Update the bus list when the language is changed
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &IO::Manager::busListChanged);

  // Update the frame reader parameters automatically
  QMetaObject::invokeMethod(&m_frameReader,
                            &FrameReader::setupExternalConnections,
                            Qt::QueuedConnection);
}

/**
 * @brief Enables or disables real time data streaming to other modules of
 *        the application without disconnecting the device.
 */
void IO::Manager::setPaused(const bool paused)
{
  m_paused = paused && isConnected();
  Q_EMIT pausedChanged();
}

/**
 * @brief Enables or disables write functionality.
 *
 * Updates the internal write flag and emits a signal to notify about the
 * change.
 *
 * @param enabled True to enable writing, false to disable.
 */
void IO::Manager::setWriteEnabled(const bool enabled)
{
  m_writeEnabled = enabled;
  Q_EMIT writeEnabledChanged();
}

/**
 * @brief Processes a received payload.
 *
 * Invokes signals to notify about the received raw data and parsed frame in a
 * thread-safe manner.
 *
 * @param payload The data payload to process.
 */
void IO::Manager::processPayload(const QByteArray &payload)
{
  if (!payload.isEmpty())
  {
    Q_EMIT dataReceived(payload);
    Q_EMIT frameReceived(payload);
  }
}

/**
 * @brief Sets the start sequence for frame detection.
 *
 * Configures the sequence that identifies the beginning of a frame. If the
 * sequence is empty, a default value is used. Updates the frame reader
 * asynchronously and emits a signal to notify about the change.
 *
 * @param sequence The new start sequence as a QByteArray.
 */
void IO::Manager::setStartSequence(const QByteArray &sequence)
{
  m_startSequence = sequence;
  if (m_startSequence.isEmpty())
    m_startSequence = QString("/*").toUtf8();

  if (m_workerThread.isRunning())
    QMetaObject::invokeMethod(
        &m_frameReader,
        [=] { m_frameReader.setStartSequence(m_startSequence); },
        Qt::QueuedConnection);

  Q_EMIT startSequenceChanged();
}

/**
 * @brief Sets the finish sequence for frame detection.
 *
 * Configures the sequence that identifies the end of a frame. If the sequence
 * is empty, a default value is used. Updates the frame reader asynchronously
 * and emits a signal to notify about the change.
 *
 * @param sequence The new finish sequence as a QByteArray.
 */
void IO::Manager::setFinishSequence(const QByteArray &sequence)
{
  m_finishSequence = sequence;
  if (m_finishSequence.isEmpty())
    m_finishSequence = QString("*/").toUtf8();

  if (m_workerThread.isRunning())
    QMetaObject::invokeMethod(
        &m_frameReader,
        [=] { m_frameReader.setFinishSequence(m_finishSequence); },
        Qt::QueuedConnection);

  Q_EMIT finishSequenceChanged();
}

/**
 * @brief Sets the bus type and updates the associated driver.
 *
 * Configures the data source by setting the appropriate bus type (e.g., Serial,
 * Network, Bluetooth LE). Disconnects the current driver, assigns a new driver
 * instance based on the bus type, and attempts to initialize the connection.
 * Emits a signal to update the UI on bus type changes.
 *
 * Supported bus types:
 * - `SerialStudio::BusType::Serial`: Serial communication.
 * - `SerialStudio::BusType::Network`: Network-based communication.
 * - `SerialStudio::BusType::BluetoothLE`: Bluetooth Low Energy communication.
 *
 * @param driver The new bus type as a `SerialStudio::BusType` enum.
 */
void IO::Manager::setBusType(const SerialStudio::BusType &driver)
{
  // Disconnect current driver
  disconnectDevice();

  // Change data source
  m_busType = driver;

  // Try to open a serial port connection
  if (busType() == SerialStudio::BusType::UART)
    setDriver(static_cast<HAL_Driver *>(&(Drivers::UART::instance())));

  // Try to open a network connection
  else if (busType() == SerialStudio::BusType::Network)
    setDriver(static_cast<HAL_Driver *>(&(Drivers::Network::instance())));

  // Try to open a BLE connection
  else if (busType() == SerialStudio::BusType::BluetoothLE)
  {
    auto *bleDriver = &Drivers::BluetoothLE::instance();
    connect(bleDriver, &IO::Drivers::BluetoothLE::deviceConnectedChanged, this,
            &IO::Manager::connectedChanged);

    if (bleDriver->operatingSystemSupported())
    {
      setDriver(static_cast<HAL_Driver *>(bleDriver));
      bleDriver->startDiscovery();
    }
  }

  // Invalid driver
  else
    setDriver(nullptr);

  // Update UI
  Q_EMIT busTypeChanged();
}

/**
 * @brief Sets the hardware abstraction layer (HAL) driver.
 *
 * Updates the current driver and establishes the necessary signal-slot
 * connections for configuration changes. Disconnects the previous driver,
 * if any, and emits signals to notify about driver and configuration updates.
 *
 * @param driver A pointer to the new `HAL_Driver` instance, or nullptr if no
 *               driver is to be set.
 */
void IO::Manager::setDriver(HAL_Driver *driver)
{
  if (m_driver != driver)
  {
    if (driver)
      connect(driver, &IO::HAL_Driver::configurationChanged, this,
              &IO::Manager::configurationChanged);

    if (m_driver)
      disconnect(m_driver);

    m_driver = driver;
    Q_EMIT driverChanged();
    Q_EMIT configurationChanged();
  }
}
