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

#include "IO/Checksum.h"

//--------------------------------------------------------------------------------------------------
// Constructor function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a FrameReader object.
 *
 * Initializes the FrameReader with default settings, including frame detection
 * mode, operation mode, and buffer settings. Runs on the main thread; see
 * the class-level docstring for the threading rationale.
 *
 * @param parent The parent QObject (optional).
 */
IO::FrameReader::FrameReader(QObject* parent)
  : QObject(parent)
  , m_checksumLength(0)
  , m_operationMode(SerialStudio::QuickPlot)
  , m_frameDetectionMode(SerialStudio::EndDelimiterOnly)
  , m_circularBuffer(1024 * 1024)
{}

//--------------------------------------------------------------------------------------------------
// Data entry point function
//--------------------------------------------------------------------------------------------------

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
void IO::FrameReader::processData(const ByteArrayPtr& data)
{
  Q_ASSERT(m_operationMode >= SerialStudio::ProjectFile
           && m_operationMode <= SerialStudio::QuickPlot);
  Q_ASSERT(m_checksumLength >= 0);

  // Validate input
  if (!data || data->isEmpty())
    return;

  // Console-only mode skips frame extraction entirely; raw bytes still reach
  // the terminal via DeviceManager::rawDataReceived, which is a separate path.
  if (m_operationMode == SerialStudio::ConsoleOnly)
    return;

  // Variable to detect if a frame has been detected
  bool framesEnqueued = false;

  // Direct processing (no frame delimiters)
  if (m_operationMode == SerialStudio::ProjectFile
      && m_frameDetectionMode == SerialStudio::NoDelimiters)
    framesEnqueued = m_queue.try_enqueue(*data);

  // Delimiter based processing
  else {
    // Detect data loss from buffer overflow
    m_circularBuffer.append(*data);
    const auto overflow = m_circularBuffer.overflowCount();
    if (overflow > 0) [[unlikely]] {
      qWarning() << "[FrameReader] Buffer overflow:" << overflow
                 << "bytes lost — data rate exceeds processing capacity";
      m_circularBuffer.resetOverflowCount();
    }

    // Read frames
    const auto initialSize = m_queue.size_approx();
    switch (m_operationMode) {
      case SerialStudio::QuickPlot:
        readEndDelimitedFrames();
        break;
      case SerialStudio::ProjectFile:
        switch (m_frameDetectionMode) {
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

    // Detect if we parsed any frame (approximate count is sufficient here
    // because both the enqueue and this check happen on the same thread)
    framesEnqueued = (m_queue.size_approx() > initialSize);
  }

  // Always notify the consumer when frames were enqueued so it can drain
  // the queue. Even if all frames were dropped due to a full queue, emit
  // readyRead so the consumer processes whatever is already queued and
  // makes room for the next processData() call.
  if (framesEnqueued || m_queue.size_approx() > 0)
    Q_EMIT readyRead();
}

//--------------------------------------------------------------------------------------------------
// Parameter setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the checksum algorithm used for validating incoming frames.
 *
 * @param checksum The name of the new checksum algorithm.
 */
void IO::FrameReader::setChecksum(const QString& checksum)
{
  // Compute output length from algorithm name
  m_checksum      = checksum;
  const auto& map = IO::checksumFunctionMap();
  const auto it   = map.find(m_checksum);
  if (it != map.end())
    m_checksumLength = it.value()("", 0).size();
  else
    m_checksumLength = 0;
}

/**
 * @brief Sets the start sequence(s) used for frame detection.
 *
 * Builds KMP tables for each non-empty pattern. The KMP tables are only
 * used by the single-delimiter fast path in readStartDelimitedFrames;
 * multi-pattern scans use findFirstOfPatterns which does its own
 * byte-by-byte comparison.
 *
 * @param starts One or more start delimiter byte sequences.
 */
void IO::FrameReader::setStartSequences(const QList<QByteArray>& starts)
{
  // Clear previous state
  m_startSequences.clear();
  m_startSequenceLps.clear();

  // Build KMP tables for each non-empty pattern
  for (const auto& s : starts) {
    if (s.isEmpty())
      continue;

    m_startSequences.append(s);
    m_startSequenceLps.append(m_circularBuffer.buildKMPTable(s));
  }

  Q_ASSERT(m_startSequences.size() == m_startSequenceLps.size());
}

/**
 * @brief Sets the finish sequence(s) used for frame detection.
 *
 * @param finishes One or more finish delimiter byte sequences.
 */
void IO::FrameReader::setFinishSequences(const QList<QByteArray>& finishes)
{
  // Clear previous state
  m_finishSequences.clear();
  m_finishSequenceLps.clear();

  // Build KMP tables for each non-empty pattern
  for (const auto& f : finishes) {
    if (f.isEmpty())
      continue;

    m_finishSequences.append(f);
    m_finishSequenceLps.append(m_circularBuffer.buildKMPTable(f));
  }

  Q_ASSERT(m_finishSequences.size() == m_finishSequenceLps.size());
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
  if (m_operationMode != SerialStudio::ProjectFile) {
    m_checksumLength = 0;
    m_checksum       = QLatin1String("");
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
void IO::FrameReader::setFrameDetectionMode(const SerialStudio::FrameDetection mode)
{
  m_frameDetectionMode = mode;
}

//--------------------------------------------------------------------------------------------------
// Frame detection functions
//--------------------------------------------------------------------------------------------------

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
  Q_ASSERT(m_circularBuffer.capacity() > 0);
  Q_ASSERT(!m_finishSequences.isEmpty());
  Q_ASSERT(m_operationMode == SerialStudio::QuickPlot
           || m_frameDetectionMode == SerialStudio::EndDelimiterOnly);

  constexpr int kMaxFramesPerCall = 10000;
  int iterations                  = 0;
  while (iterations < kMaxFramesPerCall) {
    ++iterations;

    int endIndex = -1;
    QByteArray delimiter;

    // Locate the next finish delimiter in the buffer
    if (m_finishSequences.size() == 1) [[likely]] {
      endIndex  = m_circularBuffer.findPatternKMP(m_finishSequences[0], m_finishSequenceLps[0]);
      delimiter = m_finishSequences[0];
    } else {
      const auto match = m_circularBuffer.findFirstOfPatterns(m_finishSequences);
      if (match.position >= 0) {
        endIndex  = match.position;
        delimiter = m_finishSequences[match.patternIndex];
      }
    }

    // No frame found
    if (endIndex == -1)
      break;

    // Extract frame data
    const auto frame       = m_circularBuffer.peek(endIndex);
    const auto crcPosition = endIndex + delimiter.size();
    const auto frameEndPos = crcPosition + m_checksumLength;

    // Validate checksum and enqueue if valid
    if (!frame.isEmpty()) {
      auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk) {
        if (!m_queue.try_enqueue(std::move(frame))) [[unlikely]]
          qWarning() << "[FrameReader] Frame queue full — frame dropped";

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

  if (iterations >= kMaxFramesPerCall) [[unlikely]]
    qWarning() << "[FrameReader] Loop iteration limit reached in readEndDelimitedFrames";
}

/**
 * @brief Parses frames delimited only by a start sequence.
 *
 * This method assumes that each frame begins with a fixed start pattern and
 * ends right before the next occurrence of that same pattern. The frame length
 * is inferred from the gap between two consecutive start delimiters.
 *
 * Data is buffered until a second start delimiter arrives, ensuring that slow
 * byte-by-byte streams do not produce truncated frames. A checksum, if
 * configured, is expected at the end of each frame and is excluded from the
 * emitted data.
 */
void IO::FrameReader::readStartDelimitedFrames()
{
  Q_ASSERT(!m_startSequences.isEmpty());
  Q_ASSERT(!m_startSequenceLps.isEmpty());
  Q_ASSERT(m_circularBuffer.capacity() > 0);

  // Start-only mode uses the first start delimiter
  const auto& startSeq = m_startSequences[0];
  const auto& startLps = m_startSequenceLps[0];

  constexpr int kMaxFramesPerCall = 10000;
  int iterations                  = 0;
  while (iterations < kMaxFramesPerCall) {
    ++iterations;

    int startIndex = m_circularBuffer.findPatternKMP(startSeq, startLps);
    if (startIndex == -1)
      break;

    // Discard any bytes before the first start delimiter
    if (startIndex > 0)
      (void)m_circularBuffer.read(startIndex);

    // Locate the next start delimiter to determine frame boundary
    int nextStartIndex = m_circularBuffer.findPatternKMP(startSeq, startLps, startSeq.size());

    // No second start delimiter found — wait for more data
    if (nextStartIndex == -1)
      break;

    // Extract frame payload between the two start delimiters
    qsizetype frameStart  = startSeq.size();
    qsizetype frameEndPos = nextStartIndex;
    qsizetype frameLength = frameEndPos - frameStart;

    // Empty frame, discard and advance to the next start delimiter
    if (frameLength <= 0) {
      (void)m_circularBuffer.read(frameEndPos);
      continue;
    }

    // Compute the position of the checksum, and sanity check it
    const auto crcPosition = frameEndPos - m_checksumLength;
    if (crcPosition < frameStart) {
      (void)m_circularBuffer.read(frameEndPos);
      continue;
    }

    // Extract payload excluding checksum bytes
    const auto frame = m_circularBuffer.peekRange(frameStart, frameLength - m_checksumLength);

    // Validate checksum and enqueue if valid
    if (!frame.isEmpty()) {
      const auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk) {
        if (!m_queue.try_enqueue(std::move(frame))) [[unlikely]]
          qWarning() << "[FrameReader] Frame queue full — frame dropped";
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

  if (iterations >= kMaxFramesPerCall) [[unlikely]]
    qWarning() << "[FrameReader] Loop iteration limit reached in readStartDelimitedFrames";
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
  Q_ASSERT(!m_startSequences.isEmpty());
  Q_ASSERT(!m_startSequenceLps.isEmpty());
  Q_ASSERT(!m_finishSequences.isEmpty());
  Q_ASSERT(!m_finishSequenceLps.isEmpty());

  // Start+end mode uses the first delimiter in each list
  const auto& startSeq  = m_startSequences[0];
  const auto& startLps  = m_startSequenceLps[0];
  const auto& finishSeq = m_finishSequences[0];
  const auto& finishLps = m_finishSequenceLps[0];

  constexpr int kMaxFramesPerCall = 10000;
  int iterations                  = 0;
  while (iterations < kMaxFramesPerCall) {
    ++iterations;

    int finishIndex = m_circularBuffer.findPatternKMP(finishSeq, finishLps);
    if (finishIndex == -1)
      break;

    int startIndex = m_circularBuffer.findPatternKMP(startSeq, startLps);
    if (startIndex == -1 || startIndex >= finishIndex) {
      (void)m_circularBuffer.read(finishIndex + finishSeq.size());
      continue;
    }

    qsizetype frameStart  = startIndex + startSeq.size();
    qsizetype frameLength = finishIndex - frameStart;
    if (frameLength <= 0) {
      (void)m_circularBuffer.read(finishIndex + finishSeq.size());
      continue;
    }

    const auto crcPosition = finishIndex + finishSeq.size();
    const auto frameEndPos = crcPosition + m_checksumLength;
    const auto frame       = m_circularBuffer.peekRange(frameStart, frameLength);

    // Validate checksum and enqueue if valid
    if (!frame.isEmpty()) {
      auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk) {
        if (!m_queue.try_enqueue(std::move(frame))) [[unlikely]]
          qWarning() << "[FrameReader] Frame queue full — frame dropped";
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

  if (iterations >= kMaxFramesPerCall) [[unlikely]]
    qWarning() << "[FrameReader] Loop iteration limit reached in readStartEndDelimitedFrames";
}

//--------------------------------------------------------------------------------------------------
// Checksum validation function
//--------------------------------------------------------------------------------------------------

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
IO::ValidationStatus IO::FrameReader::checksum(const QByteArray& frame, qsizetype crcPosition)
{
  Q_ASSERT(!frame.isEmpty());
  Q_ASSERT(crcPosition >= 0);

  // No checksum configured, always valid
  if (m_checksumLength == 0)
    return ValidationStatus::FrameOk;

  // Not enough data to read checksum bytes yet
  const auto bufferSize = m_circularBuffer.size();
  if (bufferSize < crcPosition + m_checksumLength)
    return ValidationStatus::ChecksumIncomplete;

  const auto calculated     = IO::checksum(m_checksum, frame);
  const QByteArray received = m_circularBuffer.peekRange(crcPosition, m_checksumLength);
  if (calculated == received)
    return ValidationStatus::FrameOk;

  static constexpr qsizetype kMaxLogBytes = 128;
  qWarning() << "\n"
             << m_checksum << "failed:\n"
             << "\t- Received:" << received.toHex(' ') << "\n"
             << "\t- Calculated:" << calculated.toHex(' ') << "\n"
             << "\t- Frame:" << frame.left(kMaxLogBytes).toHex(' ')
             << (frame.size() > kMaxLogBytes ? "...(truncated)" : "");

  return ValidationStatus::ChecksumError;
}
