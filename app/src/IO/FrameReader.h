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

#pragma once

#include <QTimer>
#include <QThread>
#include <QObject>
#include <QByteArray>
#include <QReadWriteLock>

#include "SerialStudio.h"
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
  void frameReady(const QByteArray &frame);
  void dataReceived(const QByteArray &data);

public:
  explicit FrameReader(QObject *parent = nullptr);

public slots:
  void processData(const QByteArray &data);
  void setChecksum(const QString &checksum);
  void setStartSequence(const QByteArray &start);
  void setFinishSequence(const QByteArray &finish);
  void setOperationMode(const SerialStudio::OperationMode mode);
  void setFrameDetectionMode(const SerialStudio::FrameDetection mode);

private slots:
  void readFrames();

private:
  void readEndDelimetedFrames();
  void readStartDelimitedFrames();
  void readStartEndDelimetedFrames();
  ValidationStatus checksum(const QByteArray &frame, qsizetype crcPosition);

private:
  qsizetype m_checksumLength;
  SerialStudio::OperationMode m_operationMode;
  SerialStudio::FrameDetection m_frameDetectionMode;

  CircularBuffer<QByteArray, char> m_dataBuffer;

  QString m_checksum;
  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QList<QByteArray> m_quickPlotEndSequences;

  mutable QReadWriteLock m_dataLock;
};
} // namespace IO
