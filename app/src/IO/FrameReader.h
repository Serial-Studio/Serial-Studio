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
#include <vector>
#include <QObject>
#include <QString>
#include <QByteArray>

#include "HAL_Driver.h"
#include "SerialStudio.h"
#include "IO/CircularBuffer.h"
#include "ThirdParty/readerwriterqueue.h"

namespace IO
{
enum class ValidationStatus
{
  FrameOk,
  ChecksumError,
  ChecksumIncomplete
};

/**
 * @class IO::FrameReader
 * @brief Multithreaded frame reader for detecting and processing streamed data.
 *
 * Processes incoming data streams by detecting frames using configurable start
 * and end sequences or delimiters. Supports multiple modes for flexible data
 * handling, such as quick plotting, JSON extraction, and project-specific
 * parsing.
 *
 * **Thread Safety Model:**
 * This class achieves thread safety through immutability rather than locks.
 * The IO::Manager recreates the FrameReader instance whenever configuration
 * changes (see IO::Manager::resetFrameReader()). This ensures:
 *
 * - Configuration is set ONCE in constructor
 * - No configuration changes occur during the FrameReader's lifetime
 * - processData() can safely read member variables without synchronization
 *
 * DO NOT add mutexes or atomic operations to this class. If configuration
 * needs to change, the IO::Manager will destroy this instance and create
 * a new one with updated settings.
 *
 * Lock-free operation for 256 KHz+ data rates.
 *
 * @see IO::Manager::resetFrameReader()
 */
class FrameReader : public QObject
{
  Q_OBJECT

signals:
  void readyRead();

public:
  explicit FrameReader(QObject *parent = nullptr);

  inline moodycamel::ReaderWriterQueue<QByteArray> &queue() { return m_queue; }

  [[nodiscard]] qsizetype overflowCount() const noexcept
  {
    return m_circularBuffer.overflowCount();
  }

  void resetOverflowCount() noexcept { m_circularBuffer.resetOverflowCount(); }

public slots:
  void processData(const IO::ByteArrayPtr &data);

  void setChecksum(const QString &checksum);
  void setStartSequence(const QByteArray &start);
  void setFinishSequence(const QByteArray &finish);
  void setOperationMode(const SerialStudio::OperationMode mode);
  void setFrameDetectionMode(const SerialStudio::FrameDetection mode);

private:
  void readEndDelimitedFrames();
  void readStartDelimitedFrames();
  void readStartEndDelimitedFrames();

  ValidationStatus checksum(const QByteArray &frame, qsizetype crcPosition);

private:
  QString m_checksum;
  qsizetype m_checksumLength;
  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QVector<QByteArray> m_quickPlotEndSequences;
  std::vector<int> m_startSequenceLps;
  std::vector<int> m_finishSequenceLps;
  QVector<std::vector<int>> m_quickPlotEndSequenceLps;
  SerialStudio::OperationMode m_operationMode;
  SerialStudio::FrameDetection m_frameDetectionMode;
  CircularBuffer<QByteArray, char> m_circularBuffer;
  moodycamel::ReaderWriterQueue<QByteArray> m_queue{4096};
};
} // namespace IO
