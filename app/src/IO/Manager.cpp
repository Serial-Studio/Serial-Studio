/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 * Copyright (c) 2021 JP Norair <https://github.com/jpnorair>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "IO/Manager.h"
#include "IO/Drivers/Serial.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/BluetoothLE.h"

#include "Misc/Translator.h"

#include <QApplication>

/**
 * @brief Converts C-style escape sequences in a string to their actual values.
 *
 * This function processes a string to replace escape sequences (e.g., "\\n")
 * with their corresponding character values ("\n"). It is useful for handling
 * user input in a format compatible with C escape sequences.
 *
 * @param str The input string containing escape sequences.
 * @return A string with escape sequences replaced by their actual values.
 *
 * @note Currently supports basic escape sequences. Numbers are not yet
 *       supported (TODO).
 */
static QString ADD_ESCAPE_SEQUENCES(const QString &str)
{
  auto escapedStr = str;
  escapedStr = escapedStr.replace(QStringLiteral("\\a"), QStringLiteral("\a"));
  escapedStr = escapedStr.replace(QStringLiteral("\\b"), QStringLiteral("\b"));
  escapedStr = escapedStr.replace(QStringLiteral("\\f"), QStringLiteral("\f"));
  escapedStr = escapedStr.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
  escapedStr = escapedStr.replace(QStringLiteral("\\r"), QStringLiteral("\r"));
  escapedStr = escapedStr.replace(QStringLiteral("\\t"), QStringLiteral("\t"));
  escapedStr = escapedStr.replace(QStringLiteral("\\v"), QStringLiteral("\v"));
  return escapedStr;
}

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
  : m_writeEnabled(true)
  , m_driver(nullptr)
  , m_startSequence(QStringLiteral("/*"))
  , m_finishSequence(QStringLiteral("*/"))
{
  // Move the frame parser worker to its dedicated thread
  m_frameReader.moveToThread(&m_workerThread);

  // Automatically enable/disable the connect button when bus type changes
  connect(this, &IO::Manager::busTypeChanged, this,
          &IO::Manager::configurationChanged);

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
  setBusType(SerialStudio::BusType::Serial);
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
 * @brief Checks if the Manager is in read-only mode.
 *
 * Read-only mode is determined by whether the device is connected and
 * write operations are disabled.
 *
 * @return True if the Manager is in read-only mode, false otherwise.
 */
bool IO::Manager::readOnly()
{
  return connected() && !m_writeEnabled;
}

/**
 * @brief Checks if the Manager is in read-write mode.
 *
 * Read-write mode is determined by whether the device is connected and
 * write operations are enabled.
 *
 * @return True if the Manager is in read-write mode, false otherwise.
 */
bool IO::Manager::readWrite()
{
  return connected() && m_writeEnabled;
}

/**
 * @brief Checks if the Manager is connected to a device.
 *
 * Determines the connection state by checking the driver status or the MQTT
 * subscription status.
 *
 * @return True if a device is connected or subscribed, false otherwise.
 */
bool IO::Manager::connected()
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
const QString &IO::Manager::startSequence() const
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
const QString &IO::Manager::finishSequence() const
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
  list.append(tr("Serial Port"));
  list.append(tr("Network Socket"));
  list.append(tr("Bluetooth LE"));
  return list;
}

/**
 * @brief Writes data to the connected device.
 *
 * Sends the specified data to the connected device through the active driver.
 * Emits the `dataSent` signal upon successful transmission.
 *
 * @param data The data to be written.
 * @return The number of bytes written, or -1 if an error occurs or no device is
 *         connected.
 */
qint64 IO::Manager::writeData(const QByteArray &data)
{
  if (connected())
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
 * If the Manager is currently connected, it disconnects the device. If it is
 * not connected, it attempts to establish a connection.
 */
void IO::Manager::toggleConnection()
{
  if (connected())
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
      connect(driver(), &IO::HAL_Driver::dataReceived, &m_frameReader,
              &FrameReader::processData, Qt::QueuedConnection);
      connect(&m_frameReader, &IO::FrameReader::frameReady, this,
              &IO::Manager::frameReceived, Qt::QueuedConnection);
      connect(&m_frameReader, &IO::FrameReader::dataReceived, this,
              &IO::Manager::dataReceived, Qt::QueuedConnection);

      QMetaObject::invokeMethod(&m_frameReader, &FrameReader::reset,
                                Qt::QueuedConnection);
    }

    // Error opening the device
    else
      disconnectDevice();

    // Update UI
    Q_EMIT connectedChanged();
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
    // Disconnect frame reader
    if (m_workerThread.isRunning())
    {
      disconnect(driver(), &IO::HAL_Driver::dataReceived, &m_frameReader,
                 &FrameReader::processData);
      disconnect(&m_frameReader, &IO::FrameReader::frameReady, this,
                 &IO::Manager::frameReceived);
      disconnect(&m_frameReader, &IO::FrameReader::dataReceived, this,
                 &IO::Manager::dataReceived);
      QMetaObject::invokeMethod(&m_frameReader, &FrameReader::reset,
                                Qt::QueuedConnection);
    }

    // Close driver device
    driver()->close();

    // Update UI
    Q_EMIT driverChanged();
    Q_EMIT connectedChanged();
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
    QByteArray copy = payload;
    QMetaObject::invokeMethod(
        this,
        [=] {
          Q_EMIT dataReceived(copy);
          Q_EMIT frameReceived(copy);
        },
        Qt::QueuedConnection);
  }
}

/**
 * @brief Sets the start sequence for frame detection.
 *
 * Configures the sequence that identifies the beginning of a frame. If the
 * sequence is empty, a default value is used. Updates the frame reader
 * asynchronously and emits a signal to notify about the change.
 *
 * @param sequence The new start sequence as a QString.
 */
void IO::Manager::setStartSequence(const QString &sequence)
{
  m_startSequence = ADD_ESCAPE_SEQUENCES(sequence);
  if (m_startSequence.isEmpty())
    m_startSequence = QStringLiteral("/*");

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
 * @param sequence The new finish sequence as a QString.
 */
void IO::Manager::setFinishSequence(const QString &sequence)
{
  m_finishSequence = ADD_ESCAPE_SEQUENCES(sequence);
  if (m_finishSequence.isEmpty())
    m_finishSequence = QStringLiteral("*/");

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
  if (busType() == SerialStudio::BusType::Serial)
    setDriver(static_cast<HAL_Driver *>(&(Drivers::Serial::instance())));

  // Try to open a network connection
  else if (busType() == SerialStudio::BusType::Network)
    setDriver(static_cast<HAL_Driver *>(&(Drivers::Network::instance())));

  // Try to open a BLE connection
  else if (busType() == SerialStudio::BusType::BluetoothLE)
  {
    auto *driver = &Drivers::BluetoothLE::instance();
    connect(driver, &IO::Drivers::BluetoothLE::deviceConnectedChanged, this,
            &IO::Manager::connectedChanged);

    if (driver->operatingSystemSupported())
    {
      setDriver(static_cast<HAL_Driver *>(driver));
      driver->startDiscovery();
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
