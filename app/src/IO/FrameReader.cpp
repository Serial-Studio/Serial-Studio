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

#include <chrono>
#include <utility>

#include "DataModel/NotificationCenter.h"
#include "IO/Checksum.h"
#include "Platform/AppPlatform.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a FrameReader with a 1 MiB scan buffer. Pre-allocates the frame-slot
 *        pool so steady-state extraction never touches the heap, and best-effort pins the
 *        scan buffer resident so a page fault can't stall the hotpath at rate.
 */
IO::FrameReader::FrameReader(QObject* parent)
  : QObject(parent)
  , m_checksumLength(0)
  , m_operationMode(SerialStudio::QuickPlot)
  , m_frameDetectionMode(SerialStudio::EndDelimiterOnly)
  , m_circularBuffer(1024 * 1024)
  , m_queue(65536)
  , m_capturedPoolHint(0)
  , m_bufferPinned(false)
  , m_droppedFrames(0)
  , m_lastDropNotify()
  , m_lastOverflowLog()
{
  m_capturedPool.reserve(kCapturedPoolSize);
  for (int i = 0; i < kCapturedPoolSize; ++i)
    m_capturedPool.emplace_back(std::make_shared<CapturedData>());

  m_bufferPinned = Platform::AppPlatform::lockMemoryResident(
    m_circularBuffer.storage(), static_cast<size_t>(m_circularBuffer.capacity()));
}

/**
 * @brief Releases the scan-buffer pin so the lock quota returns before the buffer is freed.
 */
IO::FrameReader::~FrameReader()
{
  if (m_bufferPinned)
    Platform::AppPlatform::unlockMemoryResident(m_circularBuffer.storage(),
                                                static_cast<size_t>(m_circularBuffer.capacity()));
}

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

  if (!data || data->data.isEmpty())
    return;

  if (m_operationMode == SerialStudio::ConsoleOnly)
    return;

  bool framesEnqueued = false;

  if (m_operationMode == SerialStudio::ProjectFile
      && m_frameDetectionMode == SerialStudio::NoDelimiters) {
    framesEnqueued = m_queue.try_enqueue(data);
    if (!framesEnqueued) [[unlikely]]
      noteDroppedFrame();
  }

  else {
    appendChunk(data);
    const auto overflow = m_circularBuffer.overflowCount();
    if (overflow > 0) [[unlikely]] {
      const auto now = CapturedData::SteadyClock::now();
      if (now - m_lastOverflowLog >= std::chrono::seconds(5)) {
        m_lastOverflowLog = now;
        qWarning() << "[FrameReader] Buffer overflow:" << overflow
                   << "bytes lost -- data rate exceeds processing capacity";
      }
      discardPendingBytes(overflow);
      m_circularBuffer.resetOverflowCount();
    }

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

  if (framesEnqueued || m_queue.size_approx() > 0)
    Q_EMIT readyRead();
}

/**
 * @brief Appends a captured chunk and tracks its timing metadata.
 */
void IO::FrameReader::appendChunk(const CapturedDataPtr& data)
{
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());

  // code-verify off
  // CircularBuffer is pre-sized SPSC; append is memcpy
  m_circularBuffer.append(data->data);
  // code-verify on

  PendingChunk chunk;
  chunk.chunk              = data;
  chunk.bytesRemaining     = data->data.size();
  chunk.nextFrameTimestamp = data->timestamp;
  chunk.frameStep          = std::max(std::chrono::nanoseconds(1), data->frameStep);
  // code-verify off
  // per-chunk timing record; deque only grows by 1 per chunk
  m_pendingChunks.push_back(std::move(chunk));
  // code-verify on
}

/**
 * @brief Drops timing-bookkeeping bytes for data lost to buffer overflow.
 */
void IO::FrameReader::discardPendingBytes(qsizetype size)
{
  if (size <= 0)
    return;

  while (size > 0 && !m_pendingChunks.empty()) {
    auto& chunk           = m_pendingChunks.front();
    const auto toConsume  = std::min(size, chunk.bytesRemaining);
    chunk.bytesRemaining -= toConsume;
    size                 -= toConsume;

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

  m_circularBuffer.discard(size);
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
 * @brief Claims a free CapturedData slot (the pool holds its only reference) for in-place
 *        filling; falls back to a heap allocation when the queue backlog pins every slot.
 */
IO::CapturedData* IO::FrameReader::claimCapturedSlot(IO::CapturedDataPtr& ptr)
{
  const size_t n = m_capturedPool.size();
  Q_ASSERT(n == static_cast<size_t>(kCapturedPoolSize));

  const size_t probes = std::min<size_t>(n, kMaxClaimProbes);
  for (size_t k = 0; k < probes; ++k) {
    const size_t idx      = (m_capturedPoolHint + k) % n;
    const auto& slotOwner = m_capturedPool[idx];
    if (slotOwner.use_count() != 1)
      continue;

    m_capturedPoolHint = idx;
    ptr                = CapturedDataPtr(slotOwner, slotOwner.get());
    return slotOwner.get();
  }

  auto heap = std::make_shared<CapturedData>();
  auto* raw = heap.get();
  ptr       = std::move(heap);
  return raw;
}

/**
 * @brief Stamps a filled slot with source timing and enqueues it, reporting queue overflow.
 */
void IO::FrameReader::enqueueCaptured(IO::CapturedDataPtr&& ptr,
                                      IO::CapturedData* cd,
                                      qsizetype frameEndPos)
{
  Q_ASSERT(cd != nullptr);
  Q_ASSERT(frameEndPos >= 0);

  cd->timestamp         = frameTimestamp(frameEndPos);
  cd->frameStep         = std::chrono::nanoseconds(1);
  cd->logicalFramesHint = 0;

  if (!m_queue.try_enqueue(std::move(ptr))) [[unlikely]]
    noteDroppedFrame();

  consumeBytes(frameEndPos);
}

/**
 * @brief Counts a dropped frame and posts a throttled saturation warning.
 */
void IO::FrameReader::noteDroppedFrame()
{
  ++m_droppedFrames;

  const auto now = CapturedData::SteadyClock::now();
  if (now - m_lastDropNotify < std::chrono::seconds(5)) [[likely]]
    return;

  m_lastDropNotify = now;
  qWarning() << "[FrameReader] Frame queue full -- frame dropped (total" << m_droppedFrames << ")";

  QMetaObject::invokeMethod(
    &DataModel::NotificationCenter::instance(),
    "postWarning",
    Qt::QueuedConnection,
    Q_ARG(QString, QStringLiteral("FrameReader")),
    Q_ARG(QString, tr("Frames dropped")),
    Q_ARG(QString,
          tr("Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) "
             "have been dropped. Reduce the data rate or disable a heavy consumer.")
            .arg(m_droppedFrames)));
}

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

    int endIndex                = -1;
    const QByteArray* delimiter = nullptr;

    if (m_finishSequences.size() == 1) [[likely]] {
      endIndex  = m_circularBuffer.findPatternKMP(m_finishSequences[0], m_finishSequenceLps[0]);
      delimiter = &m_finishSequences[0];
    } else {
      const auto match = m_circularBuffer.findFirstOfPatterns(m_finishSequences);
      if (match.position >= 0) {
        endIndex  = match.position;
        delimiter = &m_finishSequences[match.patternIndex];
      }
    }

    if (endIndex == -1)
      break;

    CapturedDataPtr ptr;
    CapturedData* cd = claimCapturedSlot(ptr);
    m_circularBuffer.peekRangeInto(0, endIndex, cd->data);

    const auto crcPosition = endIndex + delimiter->size();
    const auto frameEndPos = crcPosition + m_checksumLength;
    if (cd->data.isEmpty()) {
      consumeBytes(frameEndPos);
      continue;
    }

    const auto result = checksum(cd->data, crcPosition);
    if (result == ValidationStatus::ChecksumIncomplete)
      break;

    if (result == ValidationStatus::FrameOk)
      enqueueCaptured(std::move(ptr), cd, frameEndPos);
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
      consumeBytes(frameEndPos);
      continue;
    }

    const auto crcPosition = frameEndPos - m_checksumLength;
    if (crcPosition < frameStart) {
      consumeBytes(frameEndPos);
      continue;
    }

    CapturedDataPtr ptr;
    CapturedData* cd = claimCapturedSlot(ptr);
    m_circularBuffer.peekRangeInto(frameStart, frameLength - m_checksumLength, cd->data);
    if (cd->data.isEmpty()) {
      consumeBytes(frameEndPos);
      continue;
    }

    const auto result = checksum(cd->data, crcPosition);
    if (result == ValidationStatus::ChecksumIncomplete)
      break;

    if (result == ValidationStatus::FrameOk)
      enqueueCaptured(std::move(ptr), cd, frameEndPos);
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
      consumeBytes(finishIndex + finishSeq.size());
      continue;
    }

    const auto crcPosition = finishIndex + finishSeq.size();
    const auto frameEndPos = crcPosition + m_checksumLength;

    CapturedDataPtr ptr;
    CapturedData* cd = claimCapturedSlot(ptr);
    m_circularBuffer.peekRangeInto(frameStart, frameLength, cd->data);
    if (cd->data.isEmpty()) {
      consumeBytes(frameEndPos);
      continue;
    }

    const auto result = checksum(cd->data, crcPosition);
    if (result == ValidationStatus::ChecksumIncomplete)
      break;

    if (result == ValidationStatus::FrameOk)
      enqueueCaptured(std::move(ptr), cd, frameEndPos);
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

  if (m_checksumLength == 0)
    return ValidationStatus::FrameOk;

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
      const auto timestamp      = chunk.nextFrameTimestamp;
      chunk.nextFrameTimestamp += chunk.frameStep;
      return timestamp;
    }

    offset -= chunk.bytesRemaining;
  }

  auto& chunk               = m_pendingChunks.back();
  const auto stamp          = chunk.nextFrameTimestamp;
  chunk.nextFrameTimestamp += chunk.frameStep;
  return stamp;
}
