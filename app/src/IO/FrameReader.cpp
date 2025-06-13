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
  , m_dataBuffer(1024 * 1024)
{
  m_quickPlotEndSequences.append(QByteArray("\n"));
  m_quickPlotEndSequences.append(QByteArray("\r"));
  m_quickPlotEndSequences.append(QByteArray("\r\n"));

  setChecksum(IO::Manager::instance().checksumAlgorithm());
  setStartSequence(IO::Manager::instance().startSequence());
  setFinishSequence(IO::Manager::instance().finishSequence());
  setOperationMode(JSON::FrameBuilder::instance().operationMode());
  setFrameDetectionMode(JSON::ProjectModel::instance().frameDetection());
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
  // Lock access to frame data
  QWriteLocker locker(&m_dataLock);

  // Add data to circular buffer
  m_dataBuffer.append(data);
  Q_EMIT dataReceived(data);

  // Read frames in no-delimiter mode directly
  if (m_operationMode == SerialStudio::ProjectFile
      && m_frameDetectionMode == SerialStudio::NoDelimiters)
    Q_EMIT frameReady(m_dataBuffer.read(data.size()));

  // Read frame data
  else
    readFrames();
}

/**
 * @brief Sets the checksum algorithm used for validating incoming frames.
 *
 * If the algorithm changes, the internal buffer is cleared to prevent
 * inconsistencies during frame parsing.
 *
 * @param checksum The name of the new checksum algorithm.
 */
void IO::FrameReader::setChecksum(const QString &checksum)
{
  QWriteLocker locker(&m_dataLock);
  if (m_checksum != checksum)
  {
    m_checksum = checksum;
    m_dataBuffer.clear();
  }
}

/**
 * @brief Sets the start sequence used for frame detection.
 *
 * Updates the sequence that marks the beginning of a frame. Resets the
 * FrameReader state if the start sequence changes.
 *
 * @param start The new start sequence as a QByteArray.
 */
void IO::FrameReader::setStartSequence(const QByteArray &start)
{
  QWriteLocker locker(&m_dataLock);
  if (m_startSequence != start)
  {
    m_startSequence = start;
    m_dataBuffer.clear();
  }
}

/**
 * @brief Sets the finish sequence used for frame detection.
 *
 * Updates the sequence that marks the end of a frame. Resets the FrameReader
 * state if the finish sequence changes.
 *
 * @param finish The new finish sequence as a QByteArray.
 */
void IO::FrameReader::setFinishSequence(const QByteArray &finish)
{
  QWriteLocker locker(&m_dataLock);
  if (m_finishSequence != finish)
  {
    m_finishSequence = finish;
    m_dataBuffer.clear();
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
  QWriteLocker locker(&m_dataLock);
  if (m_operationMode != mode)
  {
    m_operationMode = mode;
    m_dataBuffer.clear();
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
  QWriteLocker locker(&m_dataLock);
  if (m_frameDetectionMode != mode)
  {
    m_frameDetectionMode = mode;
    m_dataBuffer.clear();
  }
}

/**
 * @brief IO::FrameReader::readFrames
 */
void IO::FrameReader::readFrames()
{
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
      for (const QByteArray &d : std::as_const(m_quickPlotEndSequences))
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
 * @brief Validates the integrity of a frame using the configured checksum
 * algorithm.
 *
 * This function computes the checksum of the given frame using the currently
 * selected algorithm (as defined by @c m_checksum) and compares it against the
 * checksum bytes that follow the frame's delimiter in the input buffer.
 *
 * If no checksum algorithm is configured (i.e., @c m_checksum is empty), the
 * frame is considered valid by default.
 *
 * @param frame The payload data to validate (excluding checksum bytes).
 * @param delimiter The delimiter that marks the end of the frame.
 * @param bytes Pointer to the number of bytes to discard from the input buffer
 *              after processing (frame + delimiter + checksum).
 *
 * @return ValidationStatus enum indicating:
 *         - FrameOk: The checksum is valid or not required.
 *         - ChecksumError: The computed checksum does not match the received
 * one.
 *         - ChecksumIncomplete: Not enough data available to perform
 * validation.
 */
IO::ValidationStatus
IO::FrameReader::integrityChecks(const QByteArray &frame,
                                 const QByteArray &delimiter, qsizetype *bytes)
{
  const QByteArray buffer = m_dataBuffer.peek(m_dataBuffer.size());

  // Get expected checksum from function map
  const auto &map = IO::checksumFunctionMap();
  const auto it = map.find(m_checksum);
  if (it == map.end())
  {
    *bytes += delimiter.length();
    return ValidationStatus::FrameOk;
  }

  // Compute expected checksum size
  const QByteArray actual = it.value()(frame.constData(), frame.size());
  const int checksumSize = actual.size();

  // If no checksum is expected, treat frame as valid
  if (checksumSize == 0)
  {
    *bytes += delimiter.size();
    return ValidationStatus::FrameOk;
  }

  // Ensure enough data is available for the checksum bytes
  const qsizetype offset = delimiter.size();
  if (buffer.size() < offset + checksumSize)
    return ValidationStatus::ChecksumIncomplete;

  // Extract received checksum directly after delimiter
  *bytes += delimiter.size() + checksumSize;
  const QByteArray expected = buffer.mid(offset, checksumSize);

  // Return validation result
  if (actual == expected)
    return ValidationStatus::FrameOk;

  // Log invalid frames
  else
  {
    qWarning() << "Invalid checksum, expected" << expected.toHex() << "got"
               << actual.toHex();

    return ValidationStatus::ChecksumError;
  }
}
