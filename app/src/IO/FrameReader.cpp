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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#include "FrameReader.h"

#include "IO/Manager.h"
#include "IO/Checksum.h"
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
  , m_operationMode(SerialStudio::QuickPlot)
  , m_frameDetectionMode(SerialStudio::EndDelimiterOnly)
  , m_dataBuffer(1024 * 1024)
{
  m_quickPlotEndSequences.append(QByteArray("\n"));
  m_quickPlotEndSequences.append(QByteArray("\r"));
  m_quickPlotEndSequences.append(QByteArray("\r\n"));
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
  // Stop if not connected
  if (!IO::Manager::instance().connected())
    return;

  // Add data to circular buffer
  m_dataBuffer.append(data);
  Q_EMIT dataReceived(data);

  // Read frames in no-delimiter mode directly
  if (m_operationMode == SerialStudio::ProjectFile
      && m_frameDetectionMode == SerialStudio::NoDelimiters)
    Q_EMIT frameReady(m_dataBuffer.read(data.size()));

  // Schedule a frame extraction as soon as possible without blocking the thread
  else
    QMetaObject::invokeMethod(this, &FrameReader::readFrames,
                              Qt::QueuedConnection);
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
 * @brief IO::FrameReader::readFrames
 */
void IO::FrameReader::readFrames()
{
  // Stop parsing data when a device is disconnected
  if (!IO::Manager::instance().connected() && m_dataBuffer.size() > 0)
  {
    reset();
    return;
  }

  // JSON mode, read until default frame start & end sequences are found
  if (m_operationMode == SerialStudio::DeviceSendsJSON)
    readStartEndDelimetedFrames();

  // Project mode, obtain which frame detection method to use
  else if (m_operationMode == SerialStudio::ProjectFile)
  {
    // Read using only an end delimiter
    if (m_frameDetectionMode == SerialStudio::EndDelimiterOnly)
      readEndDelimetedFrames();

    // Read using only a start delimeter
    else if (m_frameDetectionMode == SerialStudio::StartDelimiterOnly)
      readStartDelimitedFrames();

    // Read using both a start & end delimiter
    else if (m_frameDetectionMode == SerialStudio::StartAndEndDelimiter)
      readStartEndDelimetedFrames();
  }

  // Handle quick plot data
  else if (m_operationMode == SerialStudio::QuickPlot)
    readEndDelimetedFrames();
}

/**
 * @brief Reads frames delimited by an end sequence from the buffer.
 *
 * Extracts frames from the circular buffer that are terminated by a specified
 * end delimiter. Emits `frameReady` for each valid frame. Handles oversized
 * frames gracefully and stops processing if data is incomplete.
 */
void IO::FrameReader::readEndDelimetedFrames()
{
  // Cap the number of frames that we can read in a single call
  int framesRead = 0;
  constexpr int maxFrames = 100;

  // Consume the buffer until
  while (framesRead < maxFrames)
  {
    // Initialize variables
    int endIndex = -1;
    QByteArray delimiter;

    // Find the earliest finish sequence in the buffer (QuickPlot mode)
    if (m_operationMode == SerialStudio::QuickPlot)
    {
      for (const QByteArray &d : m_quickPlotEndSequences)
      {
        int index = m_dataBuffer.findPatternKMP(d);
        if (index != -1 && (endIndex == -1 || index < endIndex))
        {
          endIndex = index;
          delimiter = d;
        }
      }
    }

    // Find the earliest finish sequence in the buffer (project mode)
    else if (m_frameDetectionMode == SerialStudio::EndDelimiterOnly)
    {
      delimiter = m_finishSequence;
      endIndex = m_dataBuffer.findPatternKMP(delimiter);
    }

    // No complete frame found
    if (endIndex == -1)
      break;

    // Extract the frame up to the delimiter
    qsizetype frameLength = endIndex;
    QByteArray frame = m_dataBuffer.peek(frameLength);

    // Parse frame if not empty
    if (!frame.isEmpty())
    {
      // Checksum verification & emit frame if valid
      qsizetype chop = 0;
      auto result = integrityChecks(frame, delimiter, &chop);
      if (result == ValidationStatus::FrameOk)
      {
        Q_EMIT frameReady(frame);
        qsizetype bytesToRemove = endIndex + chop;
        (void)m_dataBuffer.read(bytesToRemove);
      }

      // Incomplete data; wait for more data
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Invalid frame; skip past finish sequence
      else
      {
        qsizetype bytesToRemove = endIndex + delimiter.size();
        (void)m_dataBuffer.read(bytesToRemove);
      }
    }

    // Empty frame; move past the finish sequence
    else
    {
      qsizetype bytesToRemove = endIndex + delimiter.size();
      (void)m_dataBuffer.read(bytesToRemove);
    }

    // Increment number of frames read
    ++framesRead;
  }
}

/**
 * @brief Reads frames delimited by a start sequence from the buffer.
 *
 * Extracts frames from the circular buffer that are bounded by specified
 * start delimiters. Emits `frameReady` for each valid frame.
 */
void IO::FrameReader::readStartDelimitedFrames()
{
  // Cap the number of frames that we can read in a single call
  int framesRead = 0;
  constexpr int maxFrames = 100;

  // Consume the buffer until
  while (framesRead < maxFrames)
  {
    // Initialize variables
    int startIndex = -1;
    int nextStartIndex = -1;

    // Find the first start sequence in the buffer (project mode)
    startIndex = m_dataBuffer.findPatternKMP(m_startSequence);
    if (startIndex == -1)
      break;

    // Find the next start sequence after the current one
    nextStartIndex = m_dataBuffer.findPatternKMP(
        m_startSequence, startIndex + m_startSequence.size());
    if (nextStartIndex == -1 || nextStartIndex == startIndex
        || nextStartIndex < startIndex)
      break;

    // Extract the frame from the buffer
    qsizetype frameStart = startIndex + m_startSequence.size();
    qsizetype frameLength = nextStartIndex - frameStart;
    QByteArray frame = m_dataBuffer.peek(frameStart + frameLength)
                           .mid(frameStart, frameLength);

    // Parse frame if not empty
    if (!frame.isEmpty())
    {
      Q_EMIT frameReady(frame);
      (void)m_dataBuffer.read(frameStart + frameLength);
    }

    // Avoid infinite loops when getting a frame length of 0
    else
      (void)m_dataBuffer.read(frameStart);

    // Increment number of frames read
    ++framesRead;
  }
}

/**
 * @brief Reads frames delimited by both start and end sequences from the
 * buffer.
 *
 * Extracts frames from the circular buffer that are enclosed by a specified
 * start and end sequence. Validates frames using integrity checks (e.g., CRC)
 * if applicable, and emits `frameReady` for each valid frame.
 */
void IO::FrameReader::readStartEndDelimetedFrames()
{
  // Consume the buffer until no frames are found
  while (true)
  {
    // Find the first end sequence
    int finishIndex = m_dataBuffer.findPatternKMP(m_finishSequence);
    if (finishIndex == -1)
      break;

    // Find the first start sequence and ensure its before the end sequence
    int startIndex = m_dataBuffer.findPatternKMP(m_startSequence);
    if (startIndex == -1 || startIndex >= finishIndex)
    {
      (void)m_dataBuffer.read(finishIndex + m_finishSequence.size());
      continue;
    }

    // Calculate frame boundaries
    qsizetype frameStart = startIndex + m_startSequence.size();
    qsizetype frameLength = finishIndex - frameStart;

    // Extract the frame between start and finish sequences
    QByteArray frame = m_dataBuffer.peek(frameStart + frameLength)
                           .mid(frameStart, frameLength);

    // Parse the frame if not empty
    if (!frame.isEmpty())
    {
      // Checksum verification & emit frame if valid
      qsizetype chop = 0;
      auto result = integrityChecks(frame, m_finishSequence, &chop);
      if (result == ValidationStatus::FrameOk)
      {
        Q_EMIT frameReady(frame);
        qsizetype bytesToRemove = finishIndex + chop;
        (void)m_dataBuffer.read(bytesToRemove);
      }

      // Incomplete data; wait for more data
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Invalid frame; discard up to the end sequence
      else
      {
        qsizetype bytesToRemove = finishIndex + m_finishSequence.size();
        (void)m_dataBuffer.read(bytesToRemove);
      }
    }

    // Empty frame; discard up to the end sequence
    else
    {
      qsizetype bytesToRemove = finishIndex + m_finishSequence.size();
      (void)m_dataBuffer.read(bytesToRemove);
    }
  }
}

/**
 * @brief Performs integrity checks on a frame.
 *
 * Verifies the validity of a frame using CRC checks (CRC-8, CRC-16, or CRC-32)
 * if the appropriate headers are detected in the circular buffer. Updates the
 * number of bytes to be removed from the buffer and returns the validation
 * status.
 *
 * @param frame The frame data to validate.
 * @param bytes A pointer to the number of bytes to remove from the buffer.
 * @return The validation status as a `ValidationStatus` enum:
 *         - `FrameOk`: Frame is valid.
 *         - `ChecksumError`: CRC mismatch.
 *         - `ChecksumIncomplete`: Not enough data for validation.
 */
IO::ValidationStatus
IO::FrameReader::integrityChecks(const QByteArray &frame,
                                 const QByteArray &delimeter, qsizetype *bytes)
{
  // Get finish sequence as byte array
  auto crc8Header = delimeter + "crc8:";
  auto crc16Header = delimeter + "crc16:";
  auto crc32Header = delimeter + "crc32:";

  // Temporary buffer to peek at the required length of data
  QByteArray cursor = m_dataBuffer.peek(m_dataBuffer.size());

  // Check CRC-8
  if (cursor.contains(crc8Header))
  {
    m_enableCrc = true;
    qsizetype offset = cursor.indexOf(crc8Header) + crc8Header.length() - 1;

    // Check if we have enough data in the buffer
    if (cursor.size() >= offset + 1)
    {
      *bytes += crc8Header.length() + 1;
      quint8 crc = static_cast<quint8>(cursor.at(offset + 1));

      if (crc8(frame.data(), frame.length()) == crc)
        return ValidationStatus::FrameOk;
      else
        return ValidationStatus::ChecksumError;
    }
  }

  // Check CRC-16
  else if (cursor.contains(crc16Header))
  {
    m_enableCrc = true;
    qsizetype offset = cursor.indexOf(crc16Header) + crc16Header.length() - 1;

    // Check if we have enough data in the buffer
    if (cursor.size() >= offset + 2)
    {
      *bytes += crc16Header.length() + 2;

      quint8 a = static_cast<quint8>(cursor.at(offset + 1));
      quint8 b = static_cast<quint8>(cursor.at(offset + 2));
      quint16 crc = (a << 8) | b;

      if (crc16(frame.data(), frame.length()) == crc)
        return ValidationStatus::FrameOk;
      else
        return ValidationStatus::ChecksumError;
    }
  }

  // Check CRC-32
  else if (cursor.contains(crc32Header))
  {
    m_enableCrc = true;
    qsizetype offset = cursor.indexOf(crc32Header) + crc32Header.length() - 1;

    // Check if we have enough data in the buffer
    if (cursor.size() >= offset + 4)
    {
      *bytes += crc32Header.length() + 4;

      quint8 a = static_cast<quint8>(cursor.at(offset + 1));
      quint8 b = static_cast<quint8>(cursor.at(offset + 2));
      quint8 c = static_cast<quint8>(cursor.at(offset + 3));
      quint8 d = static_cast<quint8>(cursor.at(offset + 4));
      quint32 crc = (a << 24) | (b << 16) | (c << 8) | d;

      if (crc32(frame.data(), frame.length()) == crc)
        return ValidationStatus::FrameOk;
      else
        return ValidationStatus::ChecksumError;
    }
  }

  // Buffer does not contain CRC code
  else if (!m_enableCrc)
  {
    *bytes += delimeter.length();
    return ValidationStatus::FrameOk;
  }

  // Checksum data incomplete
  return ValidationStatus::ChecksumIncomplete;
}
