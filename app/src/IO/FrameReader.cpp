/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <utility>

#include "IO/Checksum.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a FrameReader with default operation mode and a 1 MiB buffer.
 */
IO::FrameReader::FrameReader(QObject* parent)
  : QObject(parent)
  , m_checksumLength(0)
  , m_operationMode(SerialStudio::QuickPlot)
  , m_frameDetectionMode(SerialStudio::EndDelimiterOnly)
  , m_circularBuffer(1024 * 1024)
  , m_queue(65536)
{}

//--------------------------------------------------------------------------------------------------
// Data entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes a chunk of received bytes and extracts complete frames.
 */
void IO::FrameReader::processData(const CapturedDataPtr& data)
{
  Q_ASSERT(m_operationMode >= SerialStudio::ProjectFile
           && m_operationMode <= SerialStudio::QuickPlot);
  Q_ASSERT(m_checksumLength >= 0);

  // Validate input
  if (!data || !data->data || data->data->isEmpty())
    return;

  // ConsoleOnly mode skips frame extraction
  if (m_operationMode == SerialStudio::ConsoleOnly)
    return;

  bool framesEnqueued = false;

  // Passthrough when no delimiters are configured
  if (m_operationMode == SerialStudio::ProjectFile
      && m_frameDetectionMode == SerialStudio::NoDelimiters)
    framesEnqueued = m_queue.try_enqueue(data);

  // Delimiter-based processing
  else {
    appendChunk(data);
    const auto overflow = m_circularBuffer.overflowCount();
    if (overflow > 0) [[unlikely]] {
      qWarning() << "[FrameReader] Buffer overflow:" << overflow
                 << "bytes lost -- data rate exceeds processing capacity";
      discardPendingBytes(overflow);
      m_circularBuffer.resetOverflowCount();
    }

    // Call corresponding frame extraction algorithms depending on delimiter configuration
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

    framesEnqueued = (m_queue.size_approx() > initialSize);
  }

  // Notify the consumer whenever the queue has work to drain
  if (framesEnqueued || m_queue.size_approx() > 0)
    Q_EMIT readyRead();
}

/**
 * @brief Appends a captured chunk and tracks its timing metadata.
 */
void IO::FrameReader::appendChunk(const CapturedDataPtr& data)
{
  Q_ASSERT(data);
  Q_ASSERT(data->data);

  m_circularBuffer.append(*data->data);

  PendingChunk chunk;
  chunk.chunk              = data;
  chunk.bytesRemaining     = data->data->size();
  chunk.nextFrameTimestamp = data->timestamp;
  chunk.frameStep          = std::max(std::chrono::nanoseconds(1), data->frameStep);
  m_pendingChunks.push_back(std::move(chunk));
}

/**
 * @brief Drops timing-bookkeeping bytes for data lost to buffer overflow.
 */
void IO::FrameReader::discardPendingBytes(qsizetype size)
{
  if (size <= 0)
    return;

  while (size > 0 && !m_pendingChunks.empty()) {
    auto& chunk          = m_pendingChunks.front();
    const auto toConsume = std::min(size, chunk.bytesRemaining);
    chunk.bytesRemaining -= toConsume;
    size -= toConsume;

    if (chunk.bytesRemaining == 0)
      m_pendingChunks.pop_front();
  }
}

/**
 * @brief Removes consumed bytes from the buffer and the pending-chunk tracker.
 */
void IO::FrameReader::consumeBytes(qsizetype size)
{
  if (size <= 0)
    return;

  (void)m_circularBuffer.read(size);
  discardPendingBytes(size);
}

//--------------------------------------------------------------------------------------------------
// Parameter setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects the checksum algorithm and caches its expected output length.
 */
void IO::FrameReader::setChecksum(const QString& checksum)
{
  // Compute checksum output length from algorithm name
  m_checksum      = checksum;
  const auto& map = IO::checksumFunctionMap();
  const auto it   = map.find(m_checksum);
  if (it != map.end())
    m_checksumLength = it.value()("", 0).size();
  else
    m_checksumLength = 0;
}

/**
 * @brief Configures the frame start delimiters and precomputes their KMP tables.
 */
void IO::FrameReader::setStartSequences(const QList<QByteArray>& starts)
{
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
 * @brief Configures the frame end delimiters and precomputes their KMP tables.
 */
void IO::FrameReader::setFinishSequences(const QList<QByteArray>& finishes)
{
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
 * @brief Sets the operation mode and resets checksum state outside ProjectFile.
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
 * @brief Sets the frame detection mode (start/end/start+end/no delimiters).
 */
void IO::FrameReader::setFrameDetectionMode(const SerialStudio::FrameDetection mode)
{
  m_frameDetectionMode = mode;
}

//--------------------------------------------------------------------------------------------------
// Frame detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Extracts frames terminated by an end delimiter from the buffer.
 */
void IO::FrameReader::readEndDelimitedFrames()
{
  Q_ASSERT(m_circularBuffer.capacity() > 0);
  Q_ASSERT(!m_finishSequences.isEmpty());
  Q_ASSERT(m_operationMode == SerialStudio::QuickPlot
           || m_frameDetectionMode == SerialStudio::EndDelimiterOnly);

  constexpr int kMaxFramesPerCall = 32768;
  int iterations                  = 0;
  while (iterations < kMaxFramesPerCall) {
    ++iterations;

    int endIndex = -1;
    QByteArray delimiter;

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

    if (endIndex == -1)
      break;

    auto frame             = m_circularBuffer.peek(endIndex);
    const auto crcPosition = endIndex + delimiter.size();
    const auto frameEndPos = crcPosition + m_checksumLength;
    if (!frame.isEmpty()) {
      auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk) {
        if (!m_queue.try_enqueue(buildFrame(std::move(frame), frameEndPos))) [[unlikely]]
          qWarning() << "[FrameReader] Frame queue full - frame dropped";

        consumeBytes(frameEndPos);
      }

      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      else
        consumeBytes(frameEndPos);
    }

    else
      consumeBytes(frameEndPos);
  }

  if (iterations >= kMaxFramesPerCall) [[unlikely]]
    qWarning() << "[FrameReader] Loop iteration limit reached in readEndDelimitedFrames";
}

/**
 * @brief Extracts frames bounded by consecutive start delimiters.
 */
void IO::FrameReader::readStartDelimitedFrames()
{
  Q_ASSERT(!m_startSequences.isEmpty());
  Q_ASSERT(!m_startSequenceLps.isEmpty());
  Q_ASSERT(m_circularBuffer.capacity() > 0);

  const auto& startSeq = m_startSequences[0];
  const auto& startLps = m_startSequenceLps[0];

  constexpr int kMaxFramesPerCall = 32768;
  int iterations                  = 0;
  while (iterations < kMaxFramesPerCall) {
    ++iterations;

    int startIndex = m_circularBuffer.findPatternKMP(startSeq, startLps);
    if (startIndex == -1)
      break;

    if (startIndex > 0)
      consumeBytes(startIndex);

    int nextStartIndex = m_circularBuffer.findPatternKMP(startSeq, startLps, startSeq.size());
    if (nextStartIndex == -1)
      break;

    qsizetype frameStart  = startSeq.size();
    qsizetype frameEndPos = nextStartIndex;
    qsizetype frameLength = frameEndPos - frameStart;

    if (frameLength <= 0) {
      (void)m_circularBuffer.read(frameEndPos);
      continue;
    }

    const auto crcPosition = frameEndPos - m_checksumLength;
    if (crcPosition < frameStart) {
      (void)m_circularBuffer.read(frameEndPos);
      continue;
    }

    auto frame = m_circularBuffer.peekRange(frameStart, frameLength - m_checksumLength);
    if (!frame.isEmpty()) {
      const auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk) {
        if (!m_queue.try_enqueue(buildFrame(std::move(frame), frameEndPos))) [[unlikely]]
          qWarning() << "[FrameReader] Frame queue full -- frame dropped";
        consumeBytes(frameEndPos);
      }

      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      else
        consumeBytes(frameEndPos);
    }

    else
      consumeBytes(frameEndPos);
  }

  if (iterations >= kMaxFramesPerCall) [[unlikely]]
    qWarning() << "[FrameReader] Loop iteration limit reached in readStartDelimitedFrames";
}

/**
 * @brief Extracts frames bounded by matching start and end delimiters.
 */
void IO::FrameReader::readStartEndDelimitedFrames()
{
  Q_ASSERT(!m_startSequences.isEmpty());
  Q_ASSERT(!m_startSequenceLps.isEmpty());
  Q_ASSERT(!m_finishSequences.isEmpty());
  Q_ASSERT(!m_finishSequenceLps.isEmpty());

  const auto& startSeq  = m_startSequences[0];
  const auto& startLps  = m_startSequenceLps[0];
  const auto& finishSeq = m_finishSequences[0];
  const auto& finishLps = m_finishSequenceLps[0];

  constexpr int kMaxFramesPerCall = 32768;
  int iterations                  = 0;
  while (iterations < kMaxFramesPerCall) {
    ++iterations;

    int finishIndex = m_circularBuffer.findPatternKMP(finishSeq, finishLps);
    if (finishIndex == -1)
      break;

    int startIndex = m_circularBuffer.findPatternKMP(startSeq, startLps);
    if (startIndex == -1 || startIndex >= finishIndex) {
      consumeBytes(finishIndex + finishSeq.size());
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
    auto frame             = m_circularBuffer.peekRange(frameStart, frameLength);
    if (!frame.isEmpty()) {
      auto result = checksum(frame, crcPosition);
      if (result == ValidationStatus::FrameOk) {
        if (!m_queue.try_enqueue(buildFrame(std::move(frame), frameEndPos))) [[unlikely]]
          qWarning() << "[FrameReader] Frame queue full -- frame dropped";
        consumeBytes(frameEndPos);
      }

      else if (result == ValidationStatus::ChecksumIncomplete)
        break;

      else
        consumeBytes(frameEndPos);
    }

    else
      consumeBytes(frameEndPos);
  }

  if (iterations >= kMaxFramesPerCall) [[unlikely]]
    qWarning() << "[FrameReader] Loop iteration limit reached in readStartEndDelimitedFrames";
}

//--------------------------------------------------------------------------------------------------
// Checksum validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates the checksum trailing the frame against the configured algorithm.
 */
IO::ValidationStatus IO::FrameReader::checksum(const QByteArray& frame, qsizetype crcPosition)
{
  Q_ASSERT(!frame.isEmpty());
  Q_ASSERT(crcPosition >= 0);

  // No checksum configured -- always valid
  if (m_checksumLength == 0)
    return ValidationStatus::FrameOk;

  // Wait for more data if checksum bytes aren't yet available
  const auto bufferSize = m_circularBuffer.size();
  if (bufferSize < crcPosition + m_checksumLength)
    return ValidationStatus::ChecksumIncomplete;

  // Compute the checksums & compare
  const auto calculated     = IO::checksum(m_checksum, frame);
  const QByteArray received = m_circularBuffer.peekRange(crcPosition, m_checksumLength);
  if (calculated == received)
    return ValidationStatus::FrameOk;

  // Report checksum errors
  static constexpr qsizetype kMaxLogBytes = 128;
  qWarning() << "\n"
             << m_checksum << "failed:\n"
             << "\t- Received:" << received.toHex(' ') << "\n"
             << "\t- Calculated:" << calculated.toHex(' ') << "\n"
             << "\t- Frame:" << frame.left(kMaxLogBytes).toHex(' ')
             << (frame.size() > kMaxLogBytes ? "...(truncated)" : "");

  // Return error status
  return ValidationStatus::ChecksumError;
}

//--------------------------------------------------------------------------------------------------
// Frame finalization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the source-derived timestamp for a frame ending at the given offset.
 */
IO::CapturedData::SteadyTimePoint IO::FrameReader::frameTimestamp(qsizetype endOffsetExclusive)
{
  if (m_pendingChunks.empty())
    return IO::CapturedData::SteadyClock::now();

  qsizetype offset = std::max<qsizetype>(0, endOffsetExclusive - 1);
  for (auto& chunk : m_pendingChunks) {
    if (offset < chunk.bytesRemaining) {
      const auto timestamp = chunk.nextFrameTimestamp;
      chunk.nextFrameTimestamp += chunk.frameStep;
      return timestamp;
    }

    offset -= chunk.bytesRemaining;
  }

  auto& chunk      = m_pendingChunks.back();
  const auto stamp = chunk.nextFrameTimestamp;
  chunk.nextFrameTimestamp += chunk.frameStep;
  return stamp;
}

/**
 * @brief Wraps extracted frame bytes into a CapturedData with proper timing.
 */
IO::CapturedDataPtr IO::FrameReader::buildFrame(QByteArray&& data, qsizetype endOffsetExclusive)
{
  return IO::makeCapturedData(std::move(data), frameTimestamp(endOffsetExclusive));
}
