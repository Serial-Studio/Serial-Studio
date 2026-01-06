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

#include "FrameReader.h"

#include "IO/Manager.h"
#include "IO/Checksum.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"

//------------------------------------------------------------------------------
// Constructor function
//------------------------------------------------------------------------------

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
  , m_checksumLength(0)
  , m_operationMode(SerialStudio::QuickPlot)
  , m_frameDetectionMode(SerialStudio::EndDelimiterOnly)
  , m_circularBuffer(1024 * 1024 * 10)
{
  m_quickPlotEndSequences.append(QByteArray("\n"));
  m_quickPlotEndSequences.append(QByteArray("\r"));
  m_quickPlotEndSequences.append(QByteArray("\r\n"));

  m_quickPlotEndSequenceLps.clear();
  m_quickPlotEndSequenceLps.reserve(m_quickPlotEndSequences.size());
  for (const auto &delimiter : std::as_const(m_quickPlotEndSequences))
    m_quickPlotEndSequenceLps.append(m_circularBuffer.buildKMPTable(delimiter));

  setChecksum(IO::Manager::instance().checksumAlgorithm());
  setStartSequence(IO::Manager::instance().startSequence());
  setFinishSequence(IO::Manager::instance().finishSequence());
  setOperationMode(DataModel::FrameBuilder::instance().operationMode());
  setFrameDetectionMode(DataModel::ProjectModel::instance().frameDetection());
}

//------------------------------------------------------------------------------
// Data entry point function
//------------------------------------------------------------------------------

/**
 * @brief Processes incoming data for frame extraction and UI notification.
 *
 * Handles raw byte input and extracts data frames based on the current
 * parsing mode. Raw data is buffered internally when framing is required.
 *
 * - In passthrough mode (no delimiters), raw data is queued directly.
 * - In all other modes, a circular buffer is used to extract complete frames
 *   according to the configured delimiters.
 *
 * Parsed frames are enqueued for later processing. No signals are emitted
 * per frame to avoid UI flooding. Instead, a single `readyRead()` signal
 * notifies the consumer that new frames are available for reading.
 *
 * **Performance:** Lock-free, optimized for 256 KHz+ data rates.
 *
 * @param data Incoming byte stream from the device.
 */
void IO::FrameReader::processData(const ByteArrayPtr &data)
{
  if (IO::Manager::instance().paused())
    return;

  bool framesEnqueued = false;

  if (m_operationMode == SerialStudio::ProjectFile
      && m_frameDetectionMode == SerialStudio::NoDelimiters)
    framesEnqueued = m_queue.try_enqueue(*data);

  else
  {
    m_circularBuffer.append(*data);

    const auto initialSize = m_queue.size_approx();

    switch (m_operationMode)
    {
      case SerialStudio::QuickPlot:
        readEndDelimitedFrames();
        break;
      case SerialStudio::DeviceSendsJSON:
        readStartEndDelimitedFrames();
        break;
      case SerialStudio::ProjectFile:
        switch (m_frameDetectionMode)
        {
          case SerialStudio::EndDelimiterOnly:
            readEndDelimitedFrames();
            break;
          case SerialStudio::StartDelimiterOnly:
            readStartDelimitedFrames();
            break;
          case SerialStudio::StartAndEndDelimiter:
            readStartEndDelimitedFrames();
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }

    framesEnqueued = (m_queue.size_approx() > initialSize);
  }

  if (framesEnqueued)
    Q_EMIT readyRead();
}

//------------------------------------------------------------------------------
// Parameter setters
//------------------------------------------------------------------------------

/**
 * @brief Sets the checksum algorithm used for validating incoming frames.
 *
 * @param checksum The name of the new checksum algorithm.
 */
void IO::FrameReader::setChecksum(const QString &checksum)
{
  m_checksum = checksum;
  const auto &map = IO::checksumFunctionMap();
  const auto it = map.find(m_checksum);
  if (it != map.end())
    m_checksumLength = it.value()("", 0).size();
  else
    m_checksumLength = 0;
}

/**
 * @brief Sets the start sequence used for frame detection.
 *
 * @param start The new start sequence as a QByteArray.
 */
void IO::FrameReader::setStartSequence(const QByteArray &start)
{
  m_startSequence = start;
  m_startSequenceLps = m_circularBuffer.buildKMPTable(m_startSequence);
}

/**
 * @brief Sets the finish sequence used for frame detection.
 *
 * @param finish The new finish sequence as a QByteArray.
 */
void IO::FrameReader::setFinishSequence(const QByteArray &finish)
{
  m_finishSequence = finish;
  m_finishSequenceLps = m_circularBuffer.buildKMPTable(m_finishSequence);
}

/**
 * @brief Sets the operation mode of the FrameReader.
 *
 * Changes how the FrameReader processes data, determining the type of frames
 * to detect.
 *
 * @param mode The new operation mode as a SerialStudio::OperationMode enum.
 */
void IO::FrameReader::setOperationMode(const SerialStudio::OperationMode mode)
{
  m_operationMode = mode;
  if (m_operationMode != SerialStudio::ProjectFile)
  {
    m_checksumLength = 0;
    m_checksum = QLatin1String("");
  }
}

/**
 * @brief Sets the frame detection mode for the FrameReader.
 *
 * Specifies how frames are detected, either by end delimiter only or both
 * start and end delimiters.
 *
 * @param mode The new frame detection mode as a SerialStudio::FrameDetection
 * enum.
 */
void IO::FrameReader::setFrameDetectionMode(
    const SerialStudio::FrameDetection mode)
{
  m_frameDetectionMode = mode;
}

//------------------------------------------------------------------------------
// Frame detection functions
//------------------------------------------------------------------------------

/**
 * @brief Parses frames terminated by a known end delimiter.
 *
 * This function handles both QuickPlot and Project modes where the end of a
 * frame is marked by a specific sequence of bytes. It extracts the frame data
 * before the delimiter, validates the trailing checksum, and emits the frame
 * if valid.
 *
 * - In Quick Plot: searches for the first matching end delimiter from a list.
 * - In Project mode: uses a single configured delimiter.
 *
 * The checksum is expected immediately after the delimiter.
 */
void IO::FrameReader::readEndDelimitedFrames()
{
  while (true)
  {
    int endIndex = -1;
    QByteArray delimiter;

    if (m_operationMode == SerialStudio::QuickPlot)
    {
      for (int i = 0; i < m_quickPlotEndSequences.size(); ++i)
      {
        const auto &d = m_quickPlotEndSequences[i];
        const auto &lps = m_quickPlotEndSequenceLps[i];
        int index = m_circularBuffer.findPatternKMP(d, lps);
        if (index != -1 && (endIndex == -1 || index < endIndex))
        {
          endIndex = index;
          delimiter = d;
          break;
        }
      }
    }

    else if (m_frameDetectionMode == SerialStudio::EndDelimiterOnly)
    {
      delimiter = m_finishSequence;
      endIndex
          = m_circularBuffer.findPatternKMP(delimiter, m_finishSequenceLps);
    }

    // No frame found
    if (endIndex == -1)
      break;

    // Extract frame data
    const auto frame = m_circularBuffer.peek(endIndex);
    const auto crcPosition = endIndex + delimiter.size();
    const auto frameEndPos = crcPosition + m_checksumLength;

    // Read frame
    if (!frame.isEmpty())
    {
      // Validate checksum & register the frame
      auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk)
      {
        m_queue.try_enqueue(std::move(frame));
        (void)m_circularBuffer.read(frameEndPos);
      }

      // Incomplete data to calculate checksum
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Incorrect checksum
      else
        (void)m_circularBuffer.read(frameEndPos);
    }

    // Invalid frame
    else
      (void)m_circularBuffer.read(frameEndPos);
  }
}

/**
 * @brief Parses frames delimited only by a start sequence.
 *
 * This method assumes that each frame begins with a fixed start pattern and
 * ends right before the next occurrence of that same pattern. The frame length
 * is inferred from the gap between two start delimiters.
 *
 * A checksum is expected at the end of each frame and is excluded from the
 * emitted data.
 */
void IO::FrameReader::readStartDelimitedFrames()
{
  while (true)
  {
    int startIndex
        = m_circularBuffer.findPatternKMP(m_startSequence, m_startSequenceLps);
    if (startIndex == -1)
      break;

    int nextStartIndex
        = m_circularBuffer.findPatternKMP(m_startSequence, m_startSequenceLps,
                                          startIndex + m_startSequence.size());

    qsizetype frameEndPos;
    qsizetype frameStart = startIndex + m_startSequence.size();

    // No second start delimiter found...maybe the last frame in the stream
    if (nextStartIndex == -1)
    {
      frameEndPos = m_circularBuffer.size();
      if ((frameEndPos - frameStart) < m_checksumLength)
        break;
    }

    // Valid second start delimiter found, makes life easier for us
    else
      frameEndPos = nextStartIndex;

    // Compute the frame length and validate it's sane
    qsizetype frameLength = frameEndPos - frameStart;
    if (frameLength <= 0)
    {
      (void)m_circularBuffer.read(frameEndPos);
      continue;
    }

    // Compute the position of the checksum, and sanity check it
    const auto crcPosition = frameEndPos - m_checksumLength;
    if (crcPosition < frameStart)
    {
      (void)m_circularBuffer.read(frameEndPos);
      continue;
    }

    // Build the frame byte array
    const auto frame = m_circularBuffer.peek(frameEndPos)
                           .mid(frameStart, frameLength - m_checksumLength);

    // Validate the frame
    if (!frame.isEmpty())
    {
      // Execute checksum algorithm and register the frame
      const auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk)
      {
        m_queue.try_enqueue(std::move(frame));
        (void)m_circularBuffer.read(frameEndPos);
      }

      // Not enough bytes yet to compute checksum, wait for more
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Invalid checksum...discard and move on
      else
        (void)m_circularBuffer.read(frameEndPos);
    }

    // Empty frame or invalid data, discard...
    else
      (void)m_circularBuffer.read(frameEndPos);
  }
}

/**
 * @brief Parses frames using both a start and end delimiter.
 *
 * This is used in JSON and Project modes where a frame starts with a known byte
 * sequence and ends with another. The payload lies between the two, and a
 * fixed-length checksum follows the end delimiter.
 *
 * The function extracts the frame payload, validates it against the checksum,
 * and emits the frame if valid.
 */
void IO::FrameReader::readStartEndDelimitedFrames()
{
  while (true)
  {
    int finishIndex = m_circularBuffer.findPatternKMP(m_finishSequence,
                                                      m_finishSequenceLps);
    if (finishIndex == -1)
      break;

    int startIndex
        = m_circularBuffer.findPatternKMP(m_startSequence, m_startSequenceLps);
    if (startIndex == -1 || startIndex >= finishIndex)
    {
      (void)m_circularBuffer.read(finishIndex + m_finishSequence.size());
      continue;
    }

    qsizetype frameStart = startIndex + m_startSequence.size();
    qsizetype frameLength = finishIndex - frameStart;
    if (frameLength <= 0)
    {
      (void)m_circularBuffer.read(finishIndex + m_finishSequence.size());
      continue;
    }

    const auto crcPosition = finishIndex + m_finishSequence.size();
    const auto frameEndPos = crcPosition + m_checksumLength;
    const auto frame = m_circularBuffer.peek(frameStart + frameLength)
                           .mid(frameStart, frameLength);

    // Read frame
    if (!frame.isEmpty())
    {
      // Validate checksum and register the frame
      auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk)
      {
        m_queue.try_enqueue(std::move(frame));
        (void)m_circularBuffer.read(frameEndPos);
      }

      // Incomplete data to calculate checksum
      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      // Incorrect checksum
      else
        (void)m_circularBuffer.read(frameEndPos);
    }

    // Invalid frame
    else
      (void)m_circularBuffer.read(frameEndPos);
  }
}

//------------------------------------------------------------------------------
// Checksum validation function
//------------------------------------------------------------------------------

/**
 * @brief Validates the checksum of a frame against trailing data in the buffer.
 *
 * Calculates the checksum of the provided frame payload using the configured
 * algorithm and compares it against the raw bytes found at the specified
 * `crcPosition` in the input buffer.
 *
 * @param frame The extracted frame payload (excluding checksum bytes).
 * @param crcPosition The byte offset in the buffer where the checksum begins.
 *
 * @return ValidationStatus::FrameOk if the checksum is correct,
 *         ValidationStatus::ChecksumIncomplete if there isn’t enough data to
 *         validate, or ValidationStatus::ChecksumError on mismatch.
 */
IO::ValidationStatus IO::FrameReader::checksum(const QByteArray &frame,
                                               qsizetype crcPosition)
{
  // Early stop if checksum is null
  if (m_checksumLength == 0)
    return ValidationStatus::FrameOk;

  // Validate that we can read the checksum
  const auto bufferSize = m_circularBuffer.size();
  if (bufferSize < crcPosition + m_checksumLength)
    return ValidationStatus::ChecksumIncomplete;

  const auto calculated = IO::checksum(m_checksum, frame);
  const QByteArray received
      = m_circularBuffer.peekRange(crcPosition, m_checksumLength);
  if (calculated == received)
    return ValidationStatus::FrameOk;

  qWarning() << "\n"
             << m_checksum.toStdString().c_str() << "failed:\n"
             << "\t- Received:" << received.toHex(' ') << "\n"
             << "\t- Calculated:" << calculated.toHex(' ') << "\n"
             << "\t- Frame:" << frame.toHex(' ') << "\n"
             << "\t- Buffer:" << m_circularBuffer.peek(bufferSize).toHex(' ');

  // Return error
  return ValidationStatus::ChecksumError;
}
