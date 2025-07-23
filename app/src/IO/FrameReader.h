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

#include <QObject>
#include <QByteArray>

#include "SerialStudio.h"
#include "ThirdParty/readerwriterqueue.h"
#include "IO/CircularBuffer.h"

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
 */
class FrameReader : public QObject
{
  Q_OBJECT

signals:
  void readyRead();

public:
  explicit FrameReader(QObject *parent = nullptr);

  inline moodycamel::ReaderWriterQueue<QByteArray> &queue() { return m_queue; }

public slots:
  void processData(const QByteArray &data);

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
  qsizetype m_checksumLength;
  SerialStudio::OperationMode m_operationMode;
  SerialStudio::FrameDetection m_frameDetectionMode;

  QString m_checksum;
  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QVector<QByteArray> m_quickPlotEndSequences;

  CircularBuffer<QByteArray, char> m_circularBuffer;
  moodycamel::ReaderWriterQueue<QByteArray> m_queue{4096};
};
} // namespace IO
