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
#include "IO/Checksum.h"
#include "IO/Drivers/Serial.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/BluetoothLE.h"

#include "MQTT/Client.h"
#include "Misc/Translator.h"
#include "JSON/FrameBuilder.h"
#include "JSON/ProjectModel.h"

/**
 * Adds support for C escape sequences to the given @a str.
 * When user inputs "\n" in a textbox, Qt automatically converts that string to
 * "\\n". For our purposes, we need to convert "\\n" back to "\n", and so on
 * with the rest of the escape sequences supported by C.
 *
 * TODO: add support for numbers
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
 * Constructor function
 */
IO::Manager::Manager()
  : m_enableCrc(false)
  , m_writeEnabled(true)
  , m_maxBufferSize(1024 * 1024)
  , m_driver(nullptr)
  , m_receivedBytes(0)
  , m_startSequence(QStringLiteral("/*"))
  , m_finishSequence(QStringLiteral("*/"))
  , m_separatorSequence(QStringLiteral(","))
{
  // Set initial settings
  setMaxBufferSize(1024 * 1024);
  setSelectedDriver(SelectedDriver::Serial);

  // Update connect button status when device type is changed
  connect(this, &IO::Manager::selectedDriverChanged, this,
          &IO::Manager::configurationChanged);

  // Update lists when language changes
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &IO::Manager::languageChanged);
}

/**
 * Returns the only instance of the class
 */
IO::Manager &IO::Manager::instance()
{
  static Manager singleton;
  return singleton;
}

/**
 * Returns @c true if a device is connected and its open in read-only mode
 */
bool IO::Manager::readOnly()
{
  return connected() && !m_writeEnabled;
}

/**
 * Returns @c true if a device is connected and its open in read/write mode
 */
bool IO::Manager::readWrite()
{
  return connected() && m_writeEnabled;
}

/**
 * Returns @c true if a device is currently selected and opened or if the MQTT
 * client is currently connected as a subscriber.
 */
bool IO::Manager::connected()
{
  if (driver())
    return driver()->isOpen();

  return MQTT::Client::instance().isSubscribed();
}

/**
 * Returns @c true if a device is currently selected
 */
bool IO::Manager::deviceAvailable()
{
  return driver() != nullptr;
}

/**
 * Returns @c true if we are able to connect to a device/port with the current
 * config.
 */
bool IO::Manager::configurationOk()
{
  if (driver())
    return driver()->configurationOk();

  return false;
}

/**
 * Returns the maximum size of the buffer. This is useful to avoid consuming to
 * much memory when a large block of invalid data is received (for example, when
 * the user selects an invalid baud rate configuration).
 */
int IO::Manager::maxBufferSize() const
{
  return m_maxBufferSize;
}

/**
 * Returns a pointer to the currently selected driver.
 *
 * @warning you need to check this pointer before using it, it can have a @c
 * nullptr value during normal operations.
 */
IO::HAL_Driver *IO::Manager::driver()
{
  return m_driver;
}

/**
 * Returns the currently selected data source, possible return values:
 * - @c DataSource::Serial  use a serial port as a data source
 * - @c DataSource::Network use a network port as a data source
 */
IO::Manager::SelectedDriver IO::Manager::selectedDriver() const
{
  return m_selectedDriver;
}

/**
 * Returns the start sequence string used by the application to know where to
 * consider that a frame begins. If the start sequence is empty, then the
 * application shall ignore incoming data. The only thing that wont ignore the
 * incoming data will be the console.
 */
const QString &IO::Manager::startSequence() const
{
  return m_startSequence;
}

/**
 * Returns the finish sequence string used by the application to know where to
 * consider that a frame ends. If the start sequence is empty, then the
 * application shall ignore incoming data. The only thing that wont ignore the
 * incoming data will be the console.
 */
const QString &IO::Manager::finishSequence() const
{
  return m_finishSequence;
}

/**
 * Returns the separator sequence string used by the application to know where
 * to consider that a data item ends.
 */
const QString &IO::Manager::separatorSequence() const
{
  return m_separatorSequence;
}

/**
 * Returns a list with the possible data source options.
 */
QStringList IO::Manager::availableDrivers() const
{
  QStringList list;
  list.append(tr("Serial Port"));
  list.append(tr("Network Socket"));
  list.append(tr("Bluetooth LE"));
  return list;
}

/**
 * Tries to write the given @a data to the current device.
 *
 * @returns the number of bytes written to the target device
 */
qint64 IO::Manager::writeData(const QByteArray &data)
{
  if (connected())
  {
    // Write data to device
    auto bytes = driver()->write(data);

    // Show sent data in console
    if (bytes > 0)
    {
      auto writtenData = data;
      writtenData.chop(data.length() - bytes);
      Q_EMIT dataSent(writtenData);
    }

    // Return number of bytes written
    return bytes;
  }

  return -1;
}

/**
 * Connects/disconnects the application from the currently selected device. This
 * function is used as a convenience for the connect/disconnect button.
 */
void IO::Manager::toggleConnection()
{
  if (connected())
    disconnectDevice();
  else
    connectDevice();
}

/**
 * Closes the currently selected device and tries to open & configure a new
 * connection. A data source must be selected before calling this function.
 */
void IO::Manager::connectDevice()
{
  // Configure current device
  if (deviceAvailable())
  {
    // Set open flag
    QIODevice::OpenMode mode = QIODevice::ReadOnly;
    if (m_writeEnabled)
      mode = QIODevice::ReadWrite;

    // Open device
    if (driver()->open(mode))
    {
      connect(driver(), &IO::HAL_Driver::dataReceived, this,
              &IO::Manager::onDataReceived);
    }

    // Error opening the device
    else
      disconnectDevice();

    // Update UI
    Q_EMIT connectedChanged();
  }
}

/**
 * Disconnects from the current device and clears temp. data
 */
void IO::Manager::disconnectDevice()
{
  if (deviceAvailable())
  {
    // Disconnect device signals/slots
    disconnect(driver(), &IO::HAL_Driver::dataReceived, this,
               &IO::Manager::onDataReceived);

    // Close driver device
    driver()->close();

    // Reset data buffer
    m_receivedBytes = 0;
    m_dataBuffer.clear();
    m_dataBuffer.reserve(maxBufferSize());

    // Update UI
    Q_EMIT driverChanged();
    Q_EMIT connectedChanged();
  }

  // Disable CRC checking
  m_enableCrc = false;
}

/**
 * Enables/disables RW mode
 */
void IO::Manager::setWriteEnabled(const bool enabled)
{
  m_writeEnabled = enabled;
  Q_EMIT writeEnabledChanged();
}

/**
 * Reads the given payload and emits it as if it were received from a device.
 * This function is for convenience to interact with other application modules &
 * plugins.
 */
void IO::Manager::processPayload(const QByteArray &payload)
{
  if (!payload.isEmpty())
  {
    // Update received bytes indicator
    m_receivedBytes += payload.size();
    if (m_receivedBytes >= UINT64_MAX)
      m_receivedBytes = 0;

    // Notify user interface & application modules
    Q_EMIT dataReceived(payload);
    Q_EMIT frameReceived(payload);
    Q_EMIT receivedBytesChanged();
  }
}

/**
 * Changes the maximum permited buffer size. Check the @c maxBufferSize()
 * function for more information.
 */
void IO::Manager::setMaxBufferSize(const int maxBufferSize)
{
  m_maxBufferSize = maxBufferSize;
  Q_EMIT maxBufferSizeChanged();

  m_dataBuffer.reserve(maxBufferSize);
}

/**
 * Changes the frame start sequence. Check the @c startSequence() function for
 * more information.
 */
void IO::Manager::setStartSequence(const QString &sequence)
{
  m_startSequence = ADD_ESCAPE_SEQUENCES(sequence);
  if (m_startSequence.isEmpty())
    m_startSequence = QStringLiteral("/*");

  Q_EMIT startSequenceChanged();
}

/**
 * Changes the frame end sequence. Check the @c finishSequence() function for
 * more information.
 */
void IO::Manager::setFinishSequence(const QString &sequence)
{
  m_finishSequence = ADD_ESCAPE_SEQUENCES(sequence);
  if (m_finishSequence.isEmpty())
    m_finishSequence = QStringLiteral("*/");

  Q_EMIT finishSequenceChanged();
}

/**
 * Changes the frame separator sequence. Check the @c separatorSequence()
 * function for more information.
 */
void IO::Manager::setSeparatorSequence(const QString &sequence)
{
  m_separatorSequence = ADD_ESCAPE_SEQUENCES(sequence);
  if (m_separatorSequence.isEmpty())
    m_separatorSequence = QStringLiteral(",");

  Q_EMIT separatorSequenceChanged();
}

/**
 * Changes the data source type. Check the @c dataSource() funciton for more
 * information.
 */
void IO::Manager::setSelectedDriver(const IO::Manager::SelectedDriver &driver)
{
  // Disconnect current driver
  disconnectDevice();

  // Change data source
  m_selectedDriver = driver;

  // Try to open a serial port connection
  if (selectedDriver() == SelectedDriver::Serial)
    setDriver(&(Drivers::Serial::instance()));

  // Try to open a network connection
  else if (selectedDriver() == SelectedDriver::Network)
    setDriver(&(Drivers::Network::instance()));

  // Try to open a BLE connection
  else if (selectedDriver() == SelectedDriver::BluetoothLE)
  {
    auto *driver = &Drivers::BluetoothLE::instance();
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
  Q_EMIT selectedDriverChanged();
}

/**
 * Read frames from temporary buffer, every frame that contains the appropiate
 * start/end sequence is removed from the buffer as soon as its read.
 *
 * This function also checks that the buffer size does not exceed specified size
 * limitations.
 *
 * Implemementation credits: @jpnorair and @alex-spataru
 */
void IO::Manager::readFrames()
{
  // Obtain pointer to JSON generator
  static auto *model = &JSON::ProjectModel::instance();
  static auto *generator = &JSON::FrameBuilder::instance();

  // Initialize a static list with possible line breaks
  static QList<QByteArray> linebreaks;
  if (linebreaks.isEmpty())
  {
    linebreaks.append(QByteArray("\n"));
    linebreaks.append(QByteArray("\r"));
    linebreaks.append(QByteArray("\r\n"));
  }

  // No device connected, abort
  if (!connected())
    return;

  // Obtain operation mode from JSON generator
  const auto mode = generator->operationMode();

  // Read until start/finish combinations are not found
  if (mode == JSON::FrameBuilder::DeviceSendsJSON)
    readStartEndDelimetedFrames();

  // Project mode, obtain which frame detection method to use
  else if (mode == JSON::FrameBuilder::ProjectFile)
  {
    if (model->frameDetection() == WC::EndDelimiterOnly)
      readEndDelimetedFrames(QList<QByteArray>{finishSequence().toUtf8()});
    else if (model->frameDetection() == WC::StartAndEndDelimiter)
      readStartEndDelimetedFrames();
  }

  // Handle data with line breaks (\n, \r, or \r\n)
  else if (mode == JSON::FrameBuilder::CommaSeparatedValues)
    readEndDelimetedFrames(linebreaks);
}

/**
 * Deletes the contents of the temporary buffer. This function is called
 * automatically by the class when the temporary buffer size exceeds the limit
 * imposed by the
 * @c maxBufferSize() function.
 */
void IO::Manager::clearTempBuffer()
{
  m_dataBuffer.clear();
}

/**
 * Changes the target device pointer. Deletion should be handled by the
 * interface implementation, not by this class.
 */
void IO::Manager::setDriver(HAL_Driver *driver)
{
  if (driver)
    connect(driver, &IO::HAL_Driver::configurationChanged, this,
            &IO::Manager::configurationChanged);

  m_driver = driver;

  Q_EMIT driverChanged();
  Q_EMIT configurationChanged();
}

/**
 * Reads incoming data from the I/O device, updates the console object,
 * registers incoming data to temporary buffer & extracts valid data frames from
 * the buffer using the @c readFrame() function.
 */
void IO::Manager::onDataReceived(const QByteArray &data)
{
  // Verify that device is still valid
  if (!driver())
    disconnectDevice();

  // Read data & append it to buffer
  auto bytes = data.length();

  // Obtain frames from data buffer
  m_dataBuffer.append(data);
  readFrames();

  // Update received bytes indicator
  m_receivedBytes += bytes;
  if (m_receivedBytes >= UINT64_MAX)
    m_receivedBytes = 0;

  // Notify user interface
  Q_EMIT receivedBytesChanged();
  Q_EMIT dataReceived(data);
}

/**
 * @brief Processes data buffer to detect and emit frames using start and end
 *        delimiters.
 *
 * This function continuously scans the internal data buffer for frames that are
 * delimited by predefined start and end sequences.
 *
 * When a frame is detected, its content is extracted and validated.
 * If the frame passes integrity checks, it is emitted as a complete frame.
 *
 * - If the start delimiter is found but the end delimiter is missing, the
 *   function waits for more data.
 * - If the frame fails validation, it is discarded, and processing resumes
 *   after the end delimiter.
 * - Processed data is removed from the buffer to maintain memory efficiency.
 *
 * The function also ensures the buffer does not exceed a maximum size by
 * clearing temporary data if necessary.
 */
void IO::Manager::readStartEndDelimetedFrames()
{
  auto start = startSequence().toUtf8();
  auto finish = finishSequence().toUtf8();
  int bytesProcessed = 0;

  while (true)
  {
    // Get start index
    int sIndex = m_dataBuffer.indexOf(start, bytesProcessed);
    if (sIndex == -1)
      break;

    // Get end index
    int fIndex = m_dataBuffer.indexOf(finish, sIndex + start.size());
    if (fIndex == -1)
      break;

    // Extract the frame between start and finish sequences
    int frameStart = sIndex + start.size();
    int frameLength = fIndex - frameStart;
    QByteArray frame = m_dataBuffer.mid(frameStart, frameLength);

    // Parse frame if not empty
    if (!frame.isEmpty())
    {
      // Checksum verification & emit frame if valid
      int chop = 0;
      auto result = integrityChecks(frame, m_dataBuffer, &chop);
      if (result == ValidationStatus::FrameOk)
      {
        Q_EMIT frameReceived(frame);
        bytesProcessed = fIndex + finish.size() + chop;
      }

      // Incomplete data; wait for more data
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Invalid frame; skip past finish sequence
      else
        bytesProcessed = fIndex + finish.size() + chop;
    }

    // Empty frame; move past the finish sequence
    else
      bytesProcessed = fIndex + finish.size();
  }

  // Remove processed data from buffer
  if (bytesProcessed > 0)
    m_dataBuffer.remove(0, bytesProcessed);

  // Clear temp buffer if it exceeds maximum size
  if (m_dataBuffer.size() > maxBufferSize())
    clearTempBuffer();
}

/**
 * @brief Processes data buffer to detect and emit frames using end delimiters
 *        only.
 *
 * This function scans the internal data buffer for frames separated by
 * specified line break sequences.
 *
 * When a frame is detected up to a line break, it is emitted as a complete
 * frame.
 *
 * - Frames exceeding a set maximum size (10 KB) are discarded to avoid
 *   excessive memory usage.
 * - If the frame contains non-numeric data, it is marked invalid and skipped.
 * - Processed data is removed from the buffer, and the buffer size is monitored
 *   to prevent overflow.
 *
 * This function is designed for continuous streams where frames end with
 * newlines or other delimiters.
 *
 * @param delimeters List of QByteArray delimiters that indicate the end of a
 *                   frame.
 */
void IO::Manager::readEndDelimetedFrames(const QList<QByteArray> &delimeters)
{
  // Set maximum frame size to 10 KB
  const int maxFrameSize = 10 * 1024;
  int bytesProcessed = 0;

  while (true)
  {
    // Find the earliest line break in the buffer
    int endIndex = -1;
    QByteArray delimeter;
    for (const QByteArray &d : delimeters)
    {
      int index = m_dataBuffer.indexOf(d, bytesProcessed);
      if (index != -1 && (endIndex == -1 || index < endIndex))
      {
        endIndex = index;
        delimeter = d;
      }
    }

    // No complete line found
    if (endIndex == -1)
      break;

    // Extract the frame up to the line break
    int frameLength = endIndex - bytesProcessed;
    QByteArray frame = m_dataBuffer.mid(bytesProcessed, frameLength);

    // Process frame
    if (!frame.isEmpty())
    {
      // Skip larger frames
      if (frame.size() > maxFrameSize)
      {
        qWarning() << "Frame size exceeds maximum allowed size and will be "
                      "discarded.";
      }

      // Emit frame
      else
        Q_EMIT frameReceived(frame);
    }

    // Move past the line break
    bytesProcessed = endIndex + delimeter.size();
  }

  // Remove processed data from buffer
  if (bytesProcessed > 0)
    m_dataBuffer.remove(0, bytesProcessed);

  // Clear temp buffer if it exceeds maximum size
  if (m_dataBuffer.size() > maxBufferSize())
    clearTempBuffer();
}

/**
 * Checks if the @c cursor has a checksum corresponding to the given @a frame.
 * If so, the function shall calculate the appropiate checksum to for the @a
 * frame and compare it with the received checksum to verify the integrity of
 * received data.
 *
 * @param frame data in which we shall perform integrity checks
 * @param cursor master buffer, should start with checksum type header
 * @param bytes pointer to the number of bytes that we need to chop from the
 * master buffer
 */
IO::Manager::ValidationStatus
IO::Manager::integrityChecks(const QByteArray &frame, const QByteArray &cursor,
                             int *bytes)
{
  // Get finish sequence as byte array
  auto finish = finishSequence().toUtf8();
  auto crc8Header = finish + "crc8:";
  auto crc16Header = finish + "crc16:";
  auto crc32Header = finish + "crc32:";

  // Check CRC-8
  if (cursor.contains(crc8Header))
  {
    // Enable the CRC flag
    m_enableCrc = true;
    auto offset = cursor.indexOf(crc8Header) + crc8Header.length() - 1;

    // Check if we have enough data in the buffer
    if (cursor.length() >= offset + 1)
    {
      // Increment the number of bytes to remove from master buffer
      *bytes += crc8Header.length() + 1;

      // Get 8-bit checksum
      const quint8 crc = cursor.at(offset + 1);

      // Compare checksums
      if (crc8(frame.data(), frame.length()) == crc)
        return ValidationStatus::FrameOk;
      else
        return ValidationStatus::ChecksumError;
    }
  }

  // Check CRC-16
  else if (cursor.contains(crc16Header))
  {
    // Enable the CRC flag
    m_enableCrc = true;
    auto offset = cursor.indexOf(crc16Header) + crc16Header.length() - 1;

    // Check if we have enough data in the buffer
    if (cursor.length() >= offset + 2)
    {
      // Increment the number of bytes to remove from master buffer
      *bytes += crc16Header.length() + 2;

      // Get 16-bit checksum
      const quint8 a = cursor.at(offset + 1);
      const quint8 b = cursor.at(offset + 2);
      const quint16 crc = (a << 8) | (b & 0xff);

      // Compare checksums
      if (crc16(frame.data(), frame.length()) == crc)
        return ValidationStatus::FrameOk;
      else
        return ValidationStatus::ChecksumError;
    }
  }

  // Check CRC-32
  else if (cursor.contains(crc32Header))
  {
    // Enable the CRC flag
    m_enableCrc = true;
    auto offset = cursor.indexOf(crc32Header) + crc32Header.length() - 1;

    // Check if we have enough data in the buffer
    if (cursor.length() >= offset + 4)
    {
      // Increment the number of bytes to remove from master buffer
      *bytes += crc32Header.length() + 4;

      // Get 32-bit checksum
      const quint8 a = cursor.at(offset + 1);
      const quint8 b = cursor.at(offset + 2);
      const quint8 c = cursor.at(offset + 3);
      const quint8 d = cursor.at(offset + 4);
      const quint32 crc = (a << 24) | (b << 16) | (c << 8) | (d & 0xff);

      // Compare checksums
      if (crc32(frame.data(), frame.length()) == crc)
        return ValidationStatus::FrameOk;
      else
        return ValidationStatus::ChecksumError;
    }
  }

  // Buffer does not contain CRC code
  else if (!m_enableCrc)
  {
    *bytes += finish.length();
    return ValidationStatus::FrameOk;
  }

  // Checksum data incomplete
  return ValidationStatus::ChecksumIncomplete;
}
