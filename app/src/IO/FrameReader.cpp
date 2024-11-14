/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
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

#include "IO/Checksum.h"
#include "FrameReader.h"

#include "JSON/FrameBuilder.h"
#include "JSON/ProjectModel.h"

/**
 * @brief Constructs a FrameReader object.
 *
 * Initializes the FrameReader with default settings, including frame detection
 * mode, operation mode, and buffer settings. Designed to run in a different
 * thread.
 *
 * @param parent The parent QObject (optional).
 */
IO::FrameReader::FrameReader(QObject *parent)
  : QObject(parent)
  , m_enableCrc(false)
  , m_maxBufferSize(1024 * 1024)
  , m_operationMode(SerialStudio::QuickPlot)
  , m_frameDetectionMode(SerialStudio::EndDelimiterOnly)
{
  m_quickPlotEndSequences.append(QByteArray("\n"));
  m_quickPlotEndSequences.append(QByteArray("\r"));
  m_quickPlotEndSequences.append(QByteArray("\r\n"));
}

/**
 * @brief Retrieves the maximum buffer size allowed for the FrameReader.
 *
 * @return The maximum buffer size in bytes.
 */
qsizetype IO::FrameReader::maxBufferSize() const
{
  return m_maxBufferSize;
}

/**
 * @brief Retrieves the current operation mode of the FrameReader.
 *
 * The operation mode determines how the FrameReader processes incoming data.
 *
 * @return The current operation mode as a SerialStudio::OperationMode enum.
 */
SerialStudio::OperationMode IO::FrameReader::operationMode() const
{
  return m_operationMode;
}

/**
 * @brief Retrieves the current frame detection mode of the FrameReader.
 *
 * The frame detection mode specifies how the FrameReader identifies frame
 * boundaries.
 *
 * @return The current frame detection mode as a SerialStudio::FrameDetection
 * enum.
 */
SerialStudio::FrameDetection IO::FrameReader::frameDetectionMode() const
{
  return m_frameDetectionMode;
}

/**
 * @brief Retrieves the start sequence used for frame detection.
 *
 * This sequence marks the beginning of a data frame.
 *
 * @return A reference to the QByteArray containing the start sequence.
 */
const QByteArray &IO::FrameReader::startSequence() const
{
  return m_startSequence;
}

/**
 * @brief Retrieves the finish sequence used for frame detection.
 *
 * This sequence marks the end of a data frame.
 *
 * @return A reference to the QByteArray containing the finish sequence.
 */
const QByteArray &IO::FrameReader::finishSequence() const
{
  return m_finishSequence;
}

/**
 * @brief Resets the FrameReader's state.
 *
 * Clears the internal data buffer, resets CRC settings, and reserves space for
 * the buffer. This is useful when reinitializing or repurposing the FrameReader
 * in a multithreaded environment.
 */
void IO::FrameReader::reset()
{
  m_enableCrc = false;
  m_dataBuffer.clear();
  m_dataBuffer.squeeze();
  m_dataBuffer.reserve(m_maxBufferSize);
}

/**
 * @brief Sets up external connections for FrameReader.
 *
 * Connects the FrameReader's settings to external components, including
 * operation mode and frame detection mode, using signal-slot mechanisms.
 * Ensures synchronization of settings when executed in a different thread.
 */
void IO::FrameReader::setupExternalConnections()
{
  setOperationMode(JSON::FrameBuilder::instance().operationMode());
  setFrameDetectionMode(JSON::ProjectModel::instance().frameDetection());

  connect(&JSON::FrameBuilder::instance(),
          &JSON::FrameBuilder::operationModeChanged, this, [=] {
            setOperationMode(JSON::FrameBuilder::instance().operationMode());
          });

  connect(&JSON::ProjectModel::instance(),
          &JSON::ProjectModel::frameDetectionChanged, this, [=] {
            setFrameDetectionMode(
                JSON::ProjectModel::instance().frameDetection());
          });
}

/**
 * @brief Processes incoming data and detects frames based on the current
 * settings.
 *
 * Appends the incoming data to an internal buffer and attempts to extract
 * frames based on the operation mode and frame detection mode. Removes
 * processed data from the buffer and truncates it if it exceeds the maximum
 * buffer size.
 *
 * @param data The incoming data to process.
 */
void IO::FrameReader::processData(const QByteArray &data)
{
  // Add incoming data to buffer
  m_dataBuffer.append(data);
  qsizetype bytesRead = 0;

  // JSON mode, read until default frame start & end sequences are found
  if (m_operationMode == SerialStudio::DeviceSendsJSON)
    bytesRead = readStartEndDelimetedFrames();

  // Project mode, obtain which frame detection method to use
  else if (m_operationMode == SerialStudio::ProjectFile)
  {
    if (m_frameDetectionMode == SerialStudio::EndDelimiterOnly)
      bytesRead = readEndDelimetedFrames();
    else if (m_frameDetectionMode == SerialStudio::StartAndEndDelimiter)
      bytesRead = readStartEndDelimetedFrames();
  }

  // Handle quick plot data
  else if (m_operationMode == SerialStudio::QuickPlot)
    bytesRead = readEndDelimetedFrames();

  // Remove processed data from buffer
  if (bytesRead > 0)
    m_dataBuffer.remove(0, bytesRead);

  // Truncate buffer if it exceeds the maximum allowed size
  if (m_dataBuffer.size() > m_maxBufferSize)
  {
    qWarning() << "Data buffer exceeds maximum size; truncating to retain "
                  "unprocessed data.";
    m_dataBuffer = m_dataBuffer.right(m_maxBufferSize);
  }

  // Notify user interface about received raw data
  Q_EMIT dataReceived(data);
}

/**
 * @brief Sets the maximum buffer size for the FrameReader.
 *
 * Changes the maximum allowed size of the internal data buffer. Resets the
 * FrameReader state if the buffer size changes.
 *
 * @param size The new maximum buffer size in bytes.
 */
void IO::FrameReader::setMaxBufferSize(const qsizetype size)
{
  if (m_maxBufferSize != size)
  {
    m_maxBufferSize = size;
    reset();
  }
}

/**
 * @brief Sets the start sequence used for frame detection.
 *
 * Updates the sequence that marks the beginning of a frame. Resets the
 * FrameReader state if the start sequence changes.
 *
 * @param start The new start sequence as a QString.
 */
void IO::FrameReader::setStartSequence(const QString &start)
{
  const auto data = start.toUtf8();
  if (m_startSequence != data)
  {
    m_startSequence = data;
    reset();
  }
}

/**
 * @brief Sets the finish sequence used for frame detection.
 *
 * Updates the sequence that marks the end of a frame. Resets the FrameReader
 * state if the finish sequence changes.
 *
 * @param finish The new finish sequence as a QString.
 */
void IO::FrameReader::setFinishSequence(const QString &finish)
{
  const auto data = finish.toUtf8();
  if (m_finishSequence != data)
  {
    m_finishSequence = data;
    reset();
  }
}

/**
 * @brief Sets the operation mode of the FrameReader.
 *
 * Changes how the FrameReader processes data, determining the type of frames
 * to detect. Resets the FrameReader state if the operation mode changes.
 *
 * @param mode The new operation mode as a SerialStudio::OperationMode enum.
 */
void IO::FrameReader::setOperationMode(const SerialStudio::OperationMode mode)
{
  if (m_operationMode != mode)
  {
    m_operationMode = mode;
    reset();
  }
}

/**
 * @brief Sets the frame detection mode for the FrameReader.
 *
 * Specifies how frames are detected, either by end delimiter only or both
 * start and end delimiters. Resets the FrameReader state if the frame detection
 * mode changes.
 *
 * @param mode The new frame detection mode as a SerialStudio::FrameDetection
 * enum.
 */
void IO::FrameReader::setFrameDetectionMode(
    const SerialStudio::FrameDetection mode)
{
  if (m_frameDetectionMode != mode)
  {
    m_frameDetectionMode = mode;
    reset();
  }
}

/**
 * @brief Reads frames delimited by an end sequence from the buffer.
 *
 * Extracts frames from the data buffer that are terminated by a specified end
 * delimiter. Emits `frameReady` for each valid frame. Discards oversized frames
 * and truncates the buffer if it exceeds the maximum size.
 *
 * @return The number of bytes processed from the buffer.
 */
qsizetype IO::FrameReader::readEndDelimetedFrames()
{
  // Read all frames inside the buffer
  qsizetype bytesProcessed = 0;
  while (true)
  {
    // Initialize variables
    int endIndex = -1;
    QByteArray delimeter;

    // Find the earliest finish sequence in the buffer (QuickPlot)
    if (m_operationMode == SerialStudio::QuickPlot)
    {
      for (const QByteArray &d : m_quickPlotEndSequences)
      {
        int index = m_dataBuffer.indexOf(d, bytesProcessed);
        if (index != -1 && (endIndex == -1 || index < endIndex))
        {
          endIndex = index;
          delimeter = d;
        }
      }
    }

    // Find the earliest finish sequence in the buffer (project mode)
    else if (m_frameDetectionMode == SerialStudio::EndDelimiterOnly)
    {
      delimeter = m_finishSequence;
      int index = m_dataBuffer.indexOf(delimeter, bytesProcessed);
      if (index != -1 && (endIndex == -1 || index < endIndex))
        endIndex = index;
    }

    // No complete line found
    if (endIndex == -1)
      break;

    // Extract the frame up to the line break
    int frameLength = endIndex - bytesProcessed;
    QByteArray frame = m_dataBuffer.mid(bytesProcessed, frameLength);

    // Parse frame if not empty
    if (!frame.isEmpty())
    {
      // Checksum verification & emit frame if valid
      qsizetype chop = 0;
      auto result = integrityChecks(frame, m_dataBuffer, &chop);
      if (result == ValidationStatus::FrameOk)
      {
        Q_EMIT frameReady(frame);
        bytesProcessed = endIndex + delimeter.size() + chop;
      }

      // Incomplete data; wait for more data
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Invalid frame; skip past finish sequence
      else
        bytesProcessed = endIndex + delimeter.size() + chop;
    }

    // Empty frame; move past the finish sequence
    else
      bytesProcessed = endIndex + delimeter.size();
  }

  // Return number of bytes read
  return bytesProcessed;
}

/**
 * @brief Reads frames delimited by both start and end sequences from the
 * buffer.
 *
 * Extracts frames from the data buffer that are enclosed by a specified start
 * and end sequence. Validates frames using integrity checks (e.g., CRC) if
 * applicable, and emits `frameReady` for each valid frame.
 *
 * @return The number of bytes processed from the buffer.
 */
qsizetype IO::FrameReader::readStartEndDelimetedFrames()
{
  // Read all frames inside the buffer
  qsizetype bytesProcessed = 0;
  while (true)
  {
    // Get start index
    int sIndex = m_dataBuffer.indexOf(m_startSequence, bytesProcessed);
    if (sIndex == -1)
      break;

    // Get end index
    int fIndex = m_dataBuffer.indexOf(m_finishSequence,
                                      sIndex + m_startSequence.size());
    if (fIndex == -1)
      break;

    // Extract the frame between start and finish sequences
    int frameStart = sIndex + m_startSequence.size();
    int frameLength = fIndex - frameStart;
    QByteArray frame = m_dataBuffer.mid(frameStart, frameLength);

    // Parse frame if not empty
    if (!frame.isEmpty())
    {
      // Checksum verification & emit frame if valid
      qsizetype chop = 0;
      auto result = integrityChecks(frame, m_dataBuffer, &chop);
      if (result == ValidationStatus::FrameOk)
      {
        Q_EMIT frameReady(frame);
        bytesProcessed = fIndex + m_finishSequence.size() + chop;
      }

      // Incomplete data; wait for more data
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Invalid frame; skip past finish sequence
      else
        bytesProcessed = fIndex + m_finishSequence.size() + chop;
    }

    // Empty frame; move past the finish sequence
    else
      bytesProcessed = fIndex + m_finishSequence.size();
  }

  // Return number of bytes read
  return bytesProcessed;
}

/**
 * @brief Performs integrity checks on a frame.
 *
 * Verifies the validity of a frame using CRC checks (CRC-8, CRC-16, or CRC-32)
 * if the appropriate headers are detected in the cursor. Updates the number of
 * bytes to be removed from the buffer and returns the validation status.
 *
 * @param frame The frame data to validate.
 * @param cursor The current data buffer being processed.
 * @param bytes A pointer to the number of bytes to remove from the buffer.
 * @return The validation status as a `ValidationStatus` enum:
 *         - `FrameOk`: Frame is valid.
 *         - `ChecksumError`: CRC mismatch.
 *         - `ChecksumIncomplete`: Not enough data for validation.
 */
IO::ValidationStatus IO::FrameReader::integrityChecks(const QByteArray &frame,
                                                      const QByteArray &cursor,
                                                      qsizetype *bytes)
{
  // Get finish sequence as byte array
  auto crc8Header = m_finishSequence + "crc8:";
  auto crc16Header = m_finishSequence + "crc16:";
  auto crc32Header = m_finishSequence + "crc32:";

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
    *bytes += m_finishSequence.length();
    return ValidationStatus::FrameOk;
  }

  // Checksum data incomplete
  return ValidationStatus::ChecksumIncomplete;
}
