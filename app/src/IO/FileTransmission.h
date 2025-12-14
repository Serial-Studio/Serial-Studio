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

#include <QFile>
#include <QTimer>
#include <QObject>
#include <QTextStream>

namespace IO
{
/**
 * @class FileTransmission
 * @brief Manages line-by-line file transmission through the active I/O device.
 *
 * The FileTransmission class is a singleton that handles the sequential
 * transmission of text files through the currently connected I/O device (serial
 * port, network socket, etc.). Files are sent line-by-line at a configurable
 * interval, making it ideal for script execution, command sequences, or data
 * replay.
 *
 * Key Features:
 * - **Line-by-Line Transmission**: Sends file content one line at a time with
 *   configurable delays
 * - **Progress Tracking**: Provides real-time progress updates (0-100%)
 * - **Pause/Resume**: Transmission can be started and stopped on demand
 * - **Configurable Interval**: Adjustable delay between line transmissions
 *   (default: 100ms)
 * - **Device Integration**: Automatically uses the currently connected I/O
 *   device from IO::Manager
 *
 * Typical Use Cases:
 * - Sending initialization scripts to embedded devices
 * - Replaying captured command sequences
 * - Automated testing with predefined command sets
 * - Configuration file upload to target devices
 *
 * @note Transmission requires an active connection through IO::Manager.
 *       The file selection and transmission state are exposed as Qt properties
 *       for easy QML integration.
 */
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
