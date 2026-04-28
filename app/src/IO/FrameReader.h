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

#pragma once

#include <deque>
#include <memory>
#include <QByteArray>
#include <QObject>
#include <QString>
#include <vector>

#include "HAL_Driver.h"
#include "IO/CircularBuffer.h"
#include "SerialStudio.h"
#include "ThirdParty/readerwriterqueue.h"

namespace IO {
/**
 * @brief Outcome of validating a candidate frame's checksum bytes.
 */
enum class ValidationStatus {
  FrameOk,
  ChecksumError,
  ChecksumIncomplete
};

/**
 * @class IO::FrameReader
 * @brief Frame extractor for detecting and processing streamed data.
 *
 * Runs on the main thread. Configuration is immutable for the FrameReader's
 * lifetime — callers must recreate the instance via
 * ConnectionManager::resetFrameReader() or DeviceManager::reconfigure() to
 * apply new settings. Do NOT add mutexes.
 */
class FrameReader : public QObject {
  Q_OBJECT

signals:
  void readyRead();

public:
  explicit FrameReader(QObject* parent = nullptr);

  inline void resetOverflowCount() { m_circularBuffer.resetOverflowCount(); }

  inline moodycamel::ReaderWriterQueue<IO::CapturedDataPtr>& queue() { return m_queue; }

  inline qsizetype overflowCount() const { return m_circularBuffer.overflowCount(); }

public slots:
  void processData(const IO::CapturedDataPtr& data);

  void setChecksum(const QString& checksum);
  void setStartSequences(const QList<QByteArray>& starts);
  void setFinishSequences(const QList<QByteArray>& finishes);
  void setOperationMode(const SerialStudio::OperationMode mode);
  void setFrameDetectionMode(const SerialStudio::FrameDetection mode);

private:
  struct PendingChunk {
    IO::CapturedDataPtr chunk;
    qsizetype bytesRemaining = 0;
    IO::CapturedData::SteadyTimePoint nextFrameTimestamp;
    std::chrono::nanoseconds frameStep = std::chrono::nanoseconds(1);
  };

  void appendChunk(const IO::CapturedDataPtr& data);
  void discardPendingBytes(qsizetype size);
  void consumeBytes(qsizetype size);
  [[nodiscard]] IO::CapturedData::SteadyTimePoint frameTimestamp(qsizetype endOffsetExclusive);
  [[nodiscard]] IO::CapturedDataPtr buildFrame(QByteArray&& data, qsizetype endOffsetExclusive);

  void readEndDelimitedFrames();
  void readStartDelimitedFrames();
  void readStartEndDelimitedFrames();

  ValidationStatus checksum(const QByteArray& frame, qsizetype crcPosition);

private:
  QString m_checksum;
  qsizetype m_checksumLength;

  QVector<QByteArray> m_startSequences;
  QVector<QByteArray> m_finishSequences;
  QVector<std::vector<int>> m_startSequenceLps;
  QVector<std::vector<int>> m_finishSequenceLps;

  SerialStudio::OperationMode m_operationMode;
  SerialStudio::FrameDetection m_frameDetectionMode;
  CircularBuffer<QByteArray, char> m_circularBuffer;
  std::deque<PendingChunk> m_pendingChunks;
  moodycamel::ReaderWriterQueue<IO::CapturedDataPtr> m_queue{4096};
};
}  // namespace IO
