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

#include <QElapsedTimer>
#include <QFile>
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

namespace IO {

namespace Protocols {
class Protocol;
class XMODEM;
class YMODEM;
class ZMODEM;
}  // namespace Protocols

/**
 * @class FileTransmission
 * @brief Manages file transmission through the active I/O device using
 *        multiple industry-standard protocols.
 *
 * Supported transfer modes:
 * - **Plain Text**: Line-by-line text file transmission with configurable
 *   interval (original behavior).
 * - **Raw Binary**: Block-by-block binary file transmission with configurable
 *   block size and inter-block delay.
 * - **XMODEM**: 128-byte blocks with CRC-16, ACK/NAK flow control.
 * - **XMODEM-1K**: 1024-byte blocks with CRC-16.
 * - **YMODEM**: XMODEM-1K with batch header (filename + size).
 * - **ZMODEM**: Streaming protocol with 32-bit CRC, auto-start, crash recovery.
 */
class FileTransmission : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool active
             READ active
             NOTIFY activeChanged)
  Q_PROPERTY(bool fileOpen
             READ fileOpen
             NOTIFY fileChanged)
  Q_PROPERTY(int errorCount
             READ errorCount
             NOTIFY errorCountChanged)
  Q_PROPERTY(int transferMode
             READ transferMode
             WRITE setTransferMode
             NOTIFY transferModeChanged)
  Q_PROPERTY(QString fileName
             READ fileName
             NOTIFY fileChanged)
  Q_PROPERTY(QString statusText
             READ statusText
             NOTIFY statusTextChanged)
  Q_PROPERTY(QString transferSpeed
             READ transferSpeed
             NOTIFY transferSpeedChanged)
  Q_PROPERTY(qint64 bytesTotal
             READ bytesTotal
             NOTIFY progressChanged)
  Q_PROPERTY(qint64 bytesSent
             READ bytesSent
             NOTIFY progressChanged)
  Q_PROPERTY(int transmissionProgress
             READ transmissionProgress
             NOTIFY progressChanged)
  Q_PROPERTY(int lineTransmissionInterval
             READ lineTransmissionInterval
             WRITE setLineTransmissionInterval
             NOTIFY lineTransmissionIntervalChanged)
  Q_PROPERTY(int blockSize
             READ blockSize
             WRITE setBlockSize
             NOTIFY blockSizeChanged)
  Q_PROPERTY(int protocolTimeout
             READ protocolTimeout
             WRITE setProtocolTimeout
             NOTIFY protocolTimeoutChanged)
  Q_PROPERTY(int maxRetries
             READ maxRetries
             WRITE setMaxRetries
             NOTIFY maxRetriesChanged)
  Q_PROPERTY(QStringList transferModes
             READ transferModes
             CONSTANT)
  Q_PROPERTY(QStringList logEntries
             READ logEntries
             NOTIFY logChanged)
  // clang-format on

signals:
  void fileChanged();
  void logChanged();
  void activeChanged();
  void progressChanged();
  void blockSizeChanged();
  void maxRetriesChanged();
  void statusTextChanged();
  void errorCountChanged();
  void transferModeChanged();
  void transferSpeedChanged();
  void protocolTimeoutChanged();
  void lineTransmissionIntervalChanged();

private:
  explicit FileTransmission();
  ~FileTransmission();
  FileTransmission(FileTransmission&&)                 = delete;
  FileTransmission(const FileTransmission&)            = delete;
  FileTransmission& operator=(FileTransmission&&)      = delete;
  FileTransmission& operator=(const FileTransmission&) = delete;

public:
  /**
   * @brief Available file transfer modes.
   */
  enum TransferMode {
    PlainText = 0,
    RawBinary = 1,
    XModem    = 2,
    XModem1K  = 3,
    YModem    = 4,
    ZModem    = 5
  };
  Q_ENUM(TransferMode)

  [[nodiscard]] static FileTransmission& instance();

  [[nodiscard]] bool active() const;
  [[nodiscard]] bool fileOpen() const;
  [[nodiscard]] int errorCount() const noexcept;
  [[nodiscard]] int blockSize() const noexcept;
  [[nodiscard]] int maxRetries() const noexcept;
  [[nodiscard]] int transferMode() const noexcept;
  [[nodiscard]] int protocolTimeout() const noexcept;
  [[nodiscard]] int transmissionProgress() const;
  [[nodiscard]] int lineTransmissionInterval() const;
  [[nodiscard]] qint64 bytesSent() const noexcept;
  [[nodiscard]] qint64 bytesTotal() const noexcept;
  [[nodiscard]] QString fileName() const;
  [[nodiscard]] QString statusText() const noexcept;
  [[nodiscard]] QString transferSpeed() const noexcept;
  [[nodiscard]] QStringList logEntries() const noexcept;
  [[nodiscard]] QStringList transferModes() const;

public slots:
  void openFile();
  void closeFile();
  void clearLog();
  void stopTransmission();
  void beginTransmission();
  void setupExternalConnections();
  void setBlockSize(int bytes);
  void setMaxRetries(int retries);
  void setTransferMode(int mode);
  void setProtocolTimeout(int msec);
  void setLineTransmissionInterval(int msec);
  void onRawDataReceived(const QByteArray& data);

private slots:
  void sendLine();
  void sendRawBlock();
  void onProtocolFinished(bool success, const QString& errorMessage);
  void onProtocolProgress(qint64 sent, qint64 total);
  void onProtocolStatus(const QString& message);
  void onProtocolWriteRequested(const QByteArray& data);
  void updateTransferSpeed();

private:
  void appendLog(const QString& message);
  void connectProtocol(Protocols::Protocol* protocol);

  // Plain text / raw binary state
  QFile m_file;
  QTimer m_timer;
  QTextStream* m_stream;

  // Protocol instances
  Protocols::XMODEM* m_xmodem;
  Protocols::YMODEM* m_ymodem;
  Protocols::ZMODEM* m_zmodem;

  // Transfer state
  TransferMode m_transferMode;
  QString m_filePath;
  QString m_statusText;
  QString m_transferSpeed;
  QStringList m_logEntries;
  qint64 m_bytesSent;
  qint64 m_bytesTotal;
  int m_errorCount;
  int m_blockSize;
  int m_maxRetries;
  int m_protocolTimeout;

  // Speed tracking
  QElapsedTimer m_speedTimer;
  QTimer m_speedUpdateTimer;
  qint64 m_lastSpeedBytes;

  static constexpr int kMaxLogEntries = 200;
};

}  // namespace IO
