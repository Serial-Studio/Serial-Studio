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
enum class ValidationStatus {
  FrameOk,
  ChecksumError,
  ChecksumIncomplete
};

/**
 * @class IO::FrameReader
 * @brief Frame extractor for detecting and processing streamed data.
 *
 * Processes incoming data streams by detecting frames using configurable start
 * and end sequences or delimiters. Supports multiple modes for flexible data
 * handling, such as quick plotting, JSON extraction, and project-specific
 * parsing.
 *
 * **Runs on the main thread.** HAL drivers may emit dataReceived() from
 * their own read threads — the AutoConnection on processData() resolves to
 * a queued hop when that happens.
 *
 * **Thread Safety Model:**
 * This class achieves thread safety through immutability rather than locks.
 * ConnectionManager recreates the FrameReader instance whenever configuration
 * changes (see ConnectionManager::resetFrameReader() and
 * DeviceManager::reconfigure()). This ensures:
 *
 * - Configuration is set ONCE on the FrameReader via setters before any
 *   data is routed through it
 * - No configuration changes occur during the FrameReader's lifetime
 * - processData() can safely read member variables without synchronization
 *
 * DO NOT add mutexes or atomic operations to this class. If configuration
 * needs to change, destroy this instance and create a new one with updated
 * settings via ConnectionManager::resetFrameReader().
 *
 * Lock-free operation for 256 KHz+ data rates.
 *
 * @see ConnectionManager::resetFrameReader()
 * @see DeviceManager::reconfigure()
 */
class FrameReader : public QObject {
  Q_OBJECT

signals:
  void readyRead();

public:
  explicit FrameReader(QObject* parent = nullptr);

  inline void resetOverflowCount() { m_circularBuffer.resetOverflowCount(); }

  inline moodycamel::ReaderWriterQueue<QByteArray>& queue() { return m_queue; }

  inline qsizetype overflowCount() const { return m_circularBuffer.overflowCount(); }

public slots:
  void processData(const IO::ByteArrayPtr& data);

  void setChecksum(const QString& checksum);
  void setStartSequences(const QList<QByteArray>& starts);
  void setFinishSequences(const QList<QByteArray>& finishes);
  void setOperationMode(const SerialStudio::OperationMode mode);
  void setFrameDetectionMode(const SerialStudio::FrameDetection mode);

private:
  void readEndDelimitedFrames();
  void readStartDelimitedFrames();
  void readStartEndDelimitedFrames();

  ValidationStatus checksum(const QByteArray& frame, qsizetype crcPosition);

private:
  QString m_checksum;
  qsizetype m_checksumLength;

  // Unified delimiter storage — one or more patterns per direction.
  // Ordered longest-first so CRLF is preferred over bare CR when both
  // match at the same buffer position.
  QVector<QByteArray> m_startSequences;
  QVector<QByteArray> m_finishSequences;
  QVector<std::vector<int>> m_startSequenceLps;
  QVector<std::vector<int>> m_finishSequenceLps;

  SerialStudio::OperationMode m_operationMode;
  SerialStudio::FrameDetection m_frameDetectionMode;
  CircularBuffer<QByteArray, char> m_circularBuffer;
  moodycamel::ReaderWriterQueue<QByteArray> m_queue{4096};
};
}  // namespace IO
