/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QThread>
#include <QObject>
#include <QByteArray>

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

  [[nodiscard]] SerialStudio::OperationMode operationMode() const;
  [[nodiscard]] SerialStudio::FrameDetection frameDetectionMode() const;

  [[nodiscard]] const QByteArray &startSequence() const;
  [[nodiscard]] const QByteArray &finishSequence() const;

public slots:
  void reset();
  void setupExternalConnections();
  void processData(const QByteArray &data);
  void setStartSequence(const QString &start);
  void setFinishSequence(const QString &finish);
  void setOperationMode(const SerialStudio::OperationMode mode);
  void setFrameDetectionMode(const SerialStudio::FrameDetection mode);

private:
  void readEndDelimetedFrames();
  void readStartEndDelimetedFrames();
  ValidationStatus integrityChecks(const QByteArray &frame,
                                   const QByteArray &delimeter,
                                   qsizetype *bytes);

private:
  bool m_enableCrc;

  SerialStudio::OperationMode m_operationMode;
  SerialStudio::FrameDetection m_frameDetectionMode;

  CircularBuffer<QByteArray, char> m_dataBuffer;

  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QList<QByteArray> m_quickPlotEndSequences;
};
} // namespace IO
