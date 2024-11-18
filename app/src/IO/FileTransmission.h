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

#include <QFile>
#include <QTimer>
#include <QObject>
#include <QTextStream>

namespace IO
{
class FileTransmission : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool fileOpen
             READ fileOpen
             NOTIFY fileChanged)
  Q_PROPERTY(bool active
             READ active
             NOTIFY activeChanged)
  Q_PROPERTY(QString fileName
             READ fileName
             NOTIFY fileChanged)
  Q_PROPERTY(int transmissionProgress
             READ transmissionProgress
             NOTIFY transmissionProgressChanged)
  Q_PROPERTY(int lineTransmissionInterval
             READ lineTransmissionInterval
             WRITE setLineTransmissionInterval
             NOTIFY lineTransmissionIntervalChanged)
  // clang-format on

signals:
  void fileChanged();
  void activeChanged();
  void transmissionProgressChanged();
  void lineTransmissionIntervalChanged();

private:
  explicit FileTransmission();
  ~FileTransmission();
  FileTransmission(FileTransmission &&) = delete;
  FileTransmission(const FileTransmission &) = delete;
  FileTransmission &operator=(FileTransmission &&) = delete;
  FileTransmission &operator=(const FileTransmission &) = delete;

public:
  static FileTransmission &instance();

  [[nodiscard]] bool active() const;
  [[nodiscard]] bool fileOpen() const;
  [[nodiscard]] QString fileName() const;
  [[nodiscard]] int transmissionProgress() const;
  [[nodiscard]] int lineTransmissionInterval() const;

public slots:
  void openFile();
  void closeFile();
  void stopTransmission();
  void beginTransmission();
  void setupExternalConnections();
  void setLineTransmissionInterval(const int msec);

private slots:
  void sendLine();

private:
  QFile m_file;
  QTimer m_timer;
  QTextStream *m_stream;
};
} // namespace IO
