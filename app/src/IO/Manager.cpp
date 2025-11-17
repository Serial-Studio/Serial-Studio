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

#include "IO/Manager.h"
#include "IO/Console.h"
#include "IO/Drivers/UART.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/BluetoothLE.h"

#include "Plugins/Server.h"
#include "Misc/Translator.h"
#include "JSON/FrameBuilder.h"

#include <QApplication>

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Client.h"
#  include "Misc/Utilities.h"
#  include "Licensing/Trial.h"
#  include "IO/Drivers/Audio.h"
#  include "Licensing/LemonSqueezy.h"
#endif

//------------------------------------------------------------------------------
// Constructor, destructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructs the `Manager` instance.
 *
 * Initializes the I/O manager with default settings and connects relevant
 * signals for data and configuration management.
 *
 * By default, the manager is configured for serial communication.
 */
IO::Manager::Manager()
  : m_paused(false)
  , m_writeEnabled(true)
  , m_thrFrameExtr(false)
  , m_driver(nullptr)
  , m_workerThread(nullptr)
  , m_frameReader(nullptr)
  , m_startSequence(QByteArray("/*"))
  , m_finishSequence(QByteArray("*/"))
{
  m_frame.reserve(4096);
  m_thrFrameExtr = m_settings.value("thrFrameExtr", false).toBool();

  setBusType(SerialStudio::BusType::UART);
  connect(this, &IO::Manager::busTypeChanged, this,
          &IO::Manager::configurationChanged);
  connect(this, &IO::Manager::configurationChanged, this,
          &IO::Manager::connectedChanged);
  connect(qApp, &QApplication::aboutToQuit, this,
          &IO::Manager::killFrameReader);
}

/**
 * @brief Destructor for the IO::Manager.
 *
 * Shuts down any device connection before destruction.
 */
IO::Manager::~Manager()
{
  if (m_frameReader)
  {
    m_frameReader->disconnect();
    QObject::disconnect(driver(), &IO::HAL_Driver::dataReceived, m_frameReader,
                        &IO::FrameReader::processData);

    QMetaObject::invokeMethod(m_frameReader, "deleteLater",
                              Qt::QueuedConnection);
    m_frameReader.clear();
  }

  if (m_workerThread.isRunning())
  {
    m_workerThread.quit();
    m_workerThread.wait();
  }
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

//------------------------------------------------------------------------------
// Member & status access functions
//------------------------------------------------------------------------------

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
 * @brief Returns whether threaded frame extraction is enabled.
 *
 * @return true if frame extraction runs in a separate thread, false otherwise.
 */
bool IO::Manager::threadedFrameExtraction()
{
  return m_thrFrameExtr;
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
 * @brief Returns the name of the currently selected checksum algorithm.
 *
 * The returned value matches one of the entries from IO::availableChecksums().
 * An empty string indicates that no checksum is applied.
 *
 * @return Reference to the selected checksum algorithm name.
 */
const QString &IO::Manager::checksumAlgorithm() const
{
  return m_checksumAlgorithm;
}

//------------------------------------------------------------------------------
// Bus/Driver UI list helper
//------------------------------------------------------------------------------

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
#ifdef BUILD_COMMERCIAL
  list.append(tr("Audio Stream"));
  // Comment these ports for the future
  // list.append(tr("Modbus"));
  // list.append(tr("CAN Bus"));
#endif
  return list;
}

//------------------------------------------------------------------------------
// IO <-> Driver data writting
//------------------------------------------------------------------------------

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
      IO::Console::instance().displaySentData(writtenData);
    }

    return bytes;
  }

  return -1;
}

//------------------------------------------------------------------------------
// Device connection setters
//------------------------------------------------------------------------------

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
  // Stop if trial expired
#ifdef BUILD_COMMERCIAL
  bool expired = Licensing::Trial::instance().trialExpired();
  bool activated = Licensing::LemonSqueezy::instance().isActivated();
  if (expired && !activated)
  {
    disconnectDevice();
    Misc::Utilities::showMessageBox(
        tr("Your trial period has ended."),
        tr("To continue using Serial Studio, please activate your license."));
    return;
  }
#endif

  // Configure current device
  if (driver())
  {
    // Set open flag
    QIODevice::OpenMode mode = QIODevice::ReadOnly;
    if (m_writeEnabled)
      mode = QIODevice::ReadWrite;

    // Open device & instruct frame reader to obtain data from it
    if (driver()->open(mode))
    {
      setPaused(false);
      startFrameReader();
    }

    // Error opening the device
    else
      disconnectDevice();

    // Notify UI
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
    // Close driver device
    driver()->close();
    setPaused(false);

    // Stop frame parsing thread
    killFrameReader();

    // Notify UI
    Q_EMIT driverChanged();
    Q_EMIT connectedChanged();
  }
}

/**
 * @brief Restarts the frame reader if the device is currently connected.
 *
 * This function stops and reinitializes the frame reader. Useful for applying
 * updated settings such as checksum algorithm or parsing configuration without
 * disconnecting the serial device.
 */
void IO::Manager::resetFrameReader()
{
  if (isConnected())
  {
    killFrameReader();
    startFrameReader();
  }
}

//------------------------------------------------------------------------------
// External module interface setup
//------------------------------------------------------------------------------

/**
 * @brief Sets up external connections for the Manager.
 * Connects external components, such as the translator for bus list updates.
 */
void IO::Manager::setupExternalConnections()
{
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &IO::Manager::busListChanged);
}

//------------------------------------------------------------------------------
// Member/status modification slots
//------------------------------------------------------------------------------

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
    static auto &console = IO::Console::instance();
    static auto &server = Plugins::Server::instance();
    static auto &frameBuilder = JSON::FrameBuilder::instance();

    server.hotpathTxData(payload);
    console.hotpathRxData(payload);
    frameBuilder.hotpathRxFrame(payload);

#ifdef BUILD_COMMERCIAL
    static auto &mqtt = MQTT::Client::instance();
    mqtt.hotpathTxFrame(payload);
#endif
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

  if (m_frameReader)
    resetFrameReader();

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

  if (m_frameReader)
    resetFrameReader();

  Q_EMIT finishSequenceChanged();
}

/**
 * @brief Sets the active checksum algorithm used for frame validation.
 *
 * Updates the internal checksum algorithm and notifies the frame reader,
 * if available, via a queued connection. Also emits the
 * checksumAlgorithmChanged() signal.
 *
 * @param algorithm The new checksum algorithm name. Must match an entry
 *                  in IO::availableChecksums(). An empty string disables
 *                  checksum validation.
 */
void IO::Manager::setChecksumAlgorithm(const QString &algorithm)
{
  m_checksumAlgorithm = algorithm;

  if (m_frameReader)
    resetFrameReader();

  Q_EMIT checksumAlgorithmChanged();
}

/**
 * @brief Enables or disables threaded frame extraction.
 *
 * If the IO manager is not connected and the requested state differs
 * from the current setting, this function updates the internal flag
 * controlling whether frame extraction runs on a separate thread.
 * The new value is stored in persistent settings and a signal is emitted
 * to notify listeners of the change.
 *
 * @param enabled Set to true to enable threaded frame extraction,
 *                or false to disable it.
 */
void IO::Manager::setThreadedFrameExtraction(const bool enabled)
{
  if (!isConnected())
  {
    m_thrFrameExtr = enabled;
    m_settings.setValue("thrFrameExtr", enabled);

    Q_EMIT threadedFrameExtractionChanged();
  }
}

//------------------------------------------------------------------------------
// Driver configuration functions
//------------------------------------------------------------------------------

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
    {
      connect(driver, &IO::HAL_Driver::dataReceived, this,
              &IO::Manager::onDataReceived);
      connect(driver, &IO::HAL_Driver::configurationChanged, this,
              &IO::Manager::configurationChanged);
    }

    if (m_driver)
      disconnect(m_driver);

    m_driver = driver;
    Q_EMIT driverChanged();
    Q_EMIT configurationChanged();
  }
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

#ifdef BUILD_COMMERCIAL
  // Try to open an Audio connection
  else if (busType() == SerialStudio::BusType::Audio)
    setDriver(static_cast<HAL_Driver *>(&(Drivers::Audio::instance())));
#endif

  // Invalid driver
  else
    setDriver(nullptr);

  // Update UI
  Q_EMIT busTypeChanged();
}

//------------------------------------------------------------------------------
// Threaded frame reading/extraction setup
//------------------------------------------------------------------------------

/**
 * @brief Stops and cleans up the frame reader and its worker thread.
 *
 * Safely disconnects all signals, schedules the FrameReader for deletion in its
 * own thread, shuts down the thread, and releases associated resources.
 *
 * This method is safe to call multiple times and is also triggered
 * automatically during shutdown.
 */
void IO::Manager::killFrameReader()
{
  // Disconnect all signals from the frame reader & delete it
  if (m_frameReader)
  {
    m_frameReader->disconnect();
    QObject::disconnect(driver(), &IO::HAL_Driver::dataReceived, m_frameReader,
                        &IO::FrameReader::processData);

    QMetaObject::invokeMethod(m_frameReader, &QObject::deleteLater,
                              Qt::QueuedConnection);
    m_frameReader.clear();
  }

  // Stop the worker thread
  if (m_workerThread.isRunning())
  {
    m_workerThread.quit();
    m_workerThread.wait();
  }
}

/**
 * @brief Starts the frame reader in a dedicated worker thread.
 *
 * Initializes a new FrameReader instance, configures it, moves it to a separate
 * thread, and connects it to the driver and IO::Manager. This ensures
 * non-blocking frame processing.
 *
 * If a previous frame reader is running, it will be cleanly shut down before
 * starting a new one.
 *
 * @note The FrameReader is connected to the driver via queued signals to ensure
 * thread safety.
 */
void IO::Manager::startFrameReader()
{
  // Ensure driver is set and driver is open
  Q_ASSERT(driver() != nullptr);

  // Stop the frame reader thread if needed
  killFrameReader();

  // Create new thread and frame reader instance
  m_frameReader = new FrameReader();
  if (!m_frameReader)
  {
    qCritical() << "Failed to allocate memory for frame reader";
    return;
  }

  // Move to the worker thread
  if (m_thrFrameExtr)
    m_frameReader->moveToThread(&m_workerThread);

  // Configure initial state for the frame reader
  QObject::connect(driver(), &IO::HAL_Driver::dataReceived, m_frameReader,
                   &IO::FrameReader::processData);

  // Connect frame reader events to IO::Manager
  connect(m_frameReader, &IO::FrameReader::readyRead, this,
          &IO::Manager::onReadyRead);

  // Start the worker thread
  if (m_thrFrameExtr)
  {
    if (!m_workerThread.isRunning())
      m_workerThread.start();
  }
}

//------------------------------------------------------------------------------
// Hotpath data publishing functions
//------------------------------------------------------------------------------

/**
 * @brief Processes dequeued frames and routes them to consumers.
 *
 * Called when new frames are available in the frame reader's queue.
 * Pulls frames in a loop and dispatches them to the appropriate handlers:
 * - The JSON FrameBuilder for internal parsing.
 * - The MQTT client (if enabled) for external transmission.
 *
 * Frame dispatch occurs only when the system is not paused.
 */
void IO::Manager::onReadyRead()
{
  static auto &frameBuilder = JSON::FrameBuilder::instance();
#ifdef BUILD_COMMERCIAL
  static auto &mqtt = MQTT::Client::instance();
#endif

  auto reader = m_frameReader;
  if (!m_paused && reader) [[likely]]
  {
    auto &queue = reader->queue();
    while (queue.try_dequeue(m_frame))
    {
      frameBuilder.hotpathRxFrame(m_frame);
#ifdef BUILD_COMMERCIAL
      mqtt.hotpathTxFrame(m_frame);
#endif
    }
  }
}

/**
 * @brief Handles raw data received from the device.
 *
 * Forwards incoming byte streams to registered components for live processing:
 * - The Console for display/logging.
 * - The Server plugin for external broadcasting.
 *
 * Data is processed only if the system is not paused.
 *
 * @param data Raw input bytes from the communication channel.
 */
void IO::Manager::onDataReceived(const QByteArray &data)
{
  static auto &console = IO::Console::instance();
  static auto &server = Plugins::Server::instance();

  if (!m_paused) [[likely]]
  {
    server.hotpathTxData(data);
    console.hotpathRxData(data);
  }
}
