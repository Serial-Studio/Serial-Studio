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
 * SPDX-License-Identifier: GPL-3.0-only
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
