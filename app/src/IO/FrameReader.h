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

#include <QObject>
#include <QByteArray>

#include "SerialStudio.h"

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
 * @brief A multithreaded frame reader for processing streamed data with
 *        flexible frame detection.
 *
 * The `FrameReader` class processes incoming data streams by detecting frames
 * based on configurable start and end sequences, or end delimiters. It is
 * designed to handle various operational modes, including quick plotting, JSON
 * streaming, and project file processing.
 *
 * Features include:
 * - Configurable start and end sequences for frame detection.
 * - Support for multiple operation modes:
 *   - `SerialStudio::QuickPlot`: Processes data based on common line
 * delimiters.
 *   - `SerialStudio::DeviceSendsJSON`: Extracts JSON data frames.
 *   - `SerialStudio::ProjectFile`: Uses project-specific detection methods.
 * - Integrity checks using CRC-8, CRC-16, or CRC-32.
 * - Buffer management with a customizable maximum size.
 * - Signal-slot mechanisms for notifying about processed frames and received
 *   data.
 *
 * This class is designed to operate in a multithreaded environment, ensuring
 * thread-safe handling of data streams and synchronization with external
 * components.
 *
 * @note To use `FrameReader`, configure its operational and frame detection
 *       modes and connect to its signals to handle emitted frames.
 *
 * @signals
 * - `frameReady(const QByteArray &frame)`: Emitted when a valid frame is
 *   detected.
 * - `dataReceived(const QByteArray &data)`: Emitted when new raw data is
 *   appended.
 */
class FrameReader : public QObject
{
  Q_OBJECT

signals:
  void frameReady(const QByteArray &frame);
  void dataReceived(const QByteArray &data);

public:
  explicit FrameReader(QObject *parent = nullptr);

  [[nodiscard]] qsizetype maxBufferSize() const;
  [[nodiscard]] SerialStudio::OperationMode operationMode() const;
  [[nodiscard]] SerialStudio::FrameDetection frameDetectionMode() const;

  [[nodiscard]] const QByteArray &startSequence() const;
  [[nodiscard]] const QByteArray &finishSequence() const;

public slots:
  void reset();
  void setupExternalConnections();
  void processData(const QByteArray &data);
  void setMaxBufferSize(const qsizetype size);
  void setStartSequence(const QString &start);
  void setFinishSequence(const QString &finish);
  void setOperationMode(const SerialStudio::OperationMode mode);
  void setFrameDetectionMode(const SerialStudio::FrameDetection mode);

private:
  qsizetype readEndDelimetedFrames();
  qsizetype readStartEndDelimetedFrames();
  ValidationStatus integrityChecks(const QByteArray &frame,
                                   const QByteArray &cursor, qsizetype *bytes);

private:
  bool m_enableCrc;
  qsizetype m_maxBufferSize;

  SerialStudio::OperationMode m_operationMode;
  SerialStudio::FrameDetection m_frameDetectionMode;

  QByteArray m_dataBuffer;
  QByteArray m_startSequence;
  QByteArray m_finishSequence;
  QList<QByteArray> m_quickPlotEndSequences;
};
} // namespace IO
