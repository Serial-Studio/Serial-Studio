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

#include "IO/FileTransmission.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QTime>

#include "IO/ConnectionManager.h"
#include "IO/FileTransmission/XMODEM.h"
#include "IO/FileTransmission/YMODEM.h"
#include "IO/FileTransmission/ZMODEM.h"
#include "Misc/Translator.h"

//--------------------------------------------------------------------------------------------------
// Constructor, destructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the FileTransmission singleton.
 */
IO::FileTransmission::FileTransmission()
  : m_stream(nullptr)
  , m_xmodem(nullptr)
  , m_ymodem(nullptr)
  , m_zmodem(nullptr)
  , m_transferMode(PlainText)
  , m_bytesSent(0)
  , m_bytesTotal(0)
  , m_errorCount(0)
  , m_blockSize(1024)
  , m_maxRetries(10)
  , m_protocolTimeout(10000)
  , m_lastSpeedBytes(0)
{
  // Configure periodic line/block transmission timer
  m_timer.setInterval(100);
  m_timer.setTimerType(Qt::PreciseTimer);

  // Configure speed update timer
  m_speedUpdateTimer.setInterval(1000);
  connect(&m_speedUpdateTimer, &QTimer::timeout, this, &FileTransmission::updateTransferSpeed);

  // Create and wire protocol instances
  m_xmodem = new IO::Protocols::XMODEM(this);
  m_ymodem = new IO::Protocols::YMODEM(this);
  m_zmodem = new IO::Protocols::ZMODEM(this);

  connectProtocol(m_xmodem);
  connectProtocol(m_ymodem);
  connectProtocol(m_zmodem);
}

/**
 * @brief Destructor.
 */
IO::FileTransmission::~FileTransmission()
{
  closeFile();
}

/**
 * @brief Returns the singleton instance.
 */
IO::FileTransmission& IO::FileTransmission::instance()
{
  static FileTransmission instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true if a transmission is currently in progress.
 */
bool IO::FileTransmission::active() const
{
  if (m_transferMode == PlainText || m_transferMode == RawBinary)
    return m_timer.isActive();

  switch (m_transferMode) {
    case XModem:
    case XModem1K:
      return m_xmodem->isActive();
    case YModem:
      return m_ymodem->isActive();
    case ZModem:
      return m_zmodem->isActive();
    default:
      return false;
  }
}

/**
 * @brief Returns @c true if a file is selected and a connection is active.
 */
bool IO::FileTransmission::fileOpen() const
{
  return m_file.isOpen() && IO::ConnectionManager::instance().isConnected();
}

/**
 * @brief Returns the number of protocol errors/retries during transfer.
 */
int IO::FileTransmission::errorCount() const noexcept
{
  return m_errorCount;
}

/**
 * @brief Returns the raw binary block size.
 */
int IO::FileTransmission::blockSize() const noexcept
{
  return m_blockSize;
}

/**
 * @brief Returns the maximum retries for protocol transfers.
 */
int IO::FileTransmission::maxRetries() const noexcept
{
  return m_maxRetries;
}

/**
 * @brief Returns the current transfer mode index.
 */
int IO::FileTransmission::transferMode() const noexcept
{
  return static_cast<int>(m_transferMode);
}

/**
 * @brief Returns the protocol timeout in milliseconds.
 */
int IO::FileTransmission::protocolTimeout() const noexcept
{
  return m_protocolTimeout;
}

/**
 * @brief Returns the transmission progress as a percentage (0-100).
 */
int IO::FileTransmission::transmissionProgress() const
{
  if (m_bytesTotal <= 0)
    return 0;

  return qMin(100, static_cast<int>((m_bytesSent * 100) / m_bytesTotal));
}

/**
 * @brief Returns the line transmission interval for plain text mode.
 */
int IO::FileTransmission::lineTransmissionInterval() const
{
  return m_timer.interval();
}

/**
 * @brief Returns the number of bytes transmitted so far.
 */
qint64 IO::FileTransmission::bytesSent() const noexcept
{
  return m_bytesSent;
}

/**
 * @brief Returns the total file size in bytes.
 */
qint64 IO::FileTransmission::bytesTotal() const noexcept
{
  return m_bytesTotal;
}

/**
 * @brief Returns the filename of the currently selected file.
 */
QString IO::FileTransmission::fileName() const
{
  if (!m_file.isOpen())
    return tr("No file selected...");

  return QFileInfo(m_file).fileName();
}

/**
 * @brief Returns the current status message.
 */
QString IO::FileTransmission::statusText() const noexcept
{
  return m_statusText;
}

/**
 * @brief Returns a human-readable transfer speed string.
 */
QString IO::FileTransmission::transferSpeed() const noexcept
{
  return m_transferSpeed;
}

/**
 * @brief Returns the protocol activity log entries.
 */
QStringList IO::FileTransmission::logEntries() const noexcept
{
  return m_logEntries;
}

/**
 * @brief Returns the list of available transfer mode names.
 */
QStringList IO::FileTransmission::transferModes() const
{
  return {
    tr("Plain Text"),
    tr("Raw Binary"),
    tr("XMODEM"),
    tr("XMODEM-1K"),
    tr("YMODEM"),
    tr("ZMODEM"),
  };
}

//--------------------------------------------------------------------------------------------------
// File management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a file selection dialog.
 */
void IO::FileTransmission::openFile()
{
  auto* dialog = new QFileDialog(nullptr, tr("Select file to transmit"), QDir::homePath());
  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    dialog->deleteLater();

    if (path.isEmpty())
      return;

    if (fileOpen())
      closeFile();

    m_filePath = path;
    m_file.setFileName(path);
    if (m_file.open(QFile::ReadOnly)) {
      m_bytesTotal = m_file.size();
      m_bytesSent  = 0;

      if (m_transferMode == PlainText)
        m_stream = new QTextStream(&m_file);

      Q_EMIT fileChanged();
      Q_EMIT progressChanged();
      appendLog(
        tr("File selected: %1 (%2 bytes)").arg(QFileInfo(path).fileName()).arg(m_bytesTotal));
    } else {
      qWarning() << "File open error:" << m_file.errorString();
      appendLog(tr("Error opening file: %1").arg(m_file.errorString()));
    }
  });

  dialog->open();
}

/**
 * @brief Closes the currently selected file and resets state.
 */
void IO::FileTransmission::closeFile()
{
  // Stop active transmission and release resources
  stopTransmission();

  if (m_file.isOpen())
    m_file.close();

  if (m_stream) {
    delete m_stream;
    m_stream = nullptr;
  }

  // Reset counters and notify UI
  m_bytesSent  = 0;
  m_bytesTotal = 0;
  m_filePath.clear();

  Q_EMIT fileChanged();
  Q_EMIT progressChanged();
}

//--------------------------------------------------------------------------------------------------
// Transmission control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears the activity log.
 */
void IO::FileTransmission::clearLog()
{
  m_logEntries.clear();
  Q_EMIT logChanged();
}

/**
 * @brief Stops/pauses the current transmission.
 */
void IO::FileTransmission::stopTransmission()
{
  m_timer.stop();
  m_speedUpdateTimer.stop();

  // Cancel any active protocol transfer
  if (m_xmodem->isActive())
    m_xmodem->cancelTransfer();
  if (m_ymodem->isActive())
    m_ymodem->cancelTransfer();
  if (m_zmodem->isActive())
    m_zmodem->cancelTransfer();

  m_transferSpeed.clear();
  Q_EMIT transferSpeedChanged();
  Q_EMIT activeChanged();
}

/**
 * @brief Starts or resumes the file transmission.
 */
void IO::FileTransmission::beginTransmission()
{
  // Skip if not connected
  if (!IO::ConnectionManager::instance().isConnected()) {
    stopTransmission();
    return;
  }

  if (!m_file.isOpen())
    return;

  // Reset error count and speed tracking
  m_errorCount = 0;
  Q_EMIT errorCountChanged();
  m_lastSpeedBytes = 0;
  m_speedTimer.start();
  m_speedUpdateTimer.start();

  appendLog(tr("Starting %1 transfer...").arg(transferModes().at(m_transferMode)));

  // Wire timer to the correct slot for this mode
  disconnect(&m_timer, &QTimer::timeout, this, &FileTransmission::sendLine);
  disconnect(&m_timer, &QTimer::timeout, this, &FileTransmission::sendRawBlock);

  if (m_transferMode == PlainText)
    connect(&m_timer, &QTimer::timeout, this, &FileTransmission::sendLine);
  else if (m_transferMode == RawBinary)
    connect(&m_timer, &QTimer::timeout, this, &FileTransmission::sendRawBlock);

  switch (m_transferMode) {
    case PlainText: {
      // Rewind if previous file was fully sent
      if (m_stream && transmissionProgress() >= 100) {
        m_stream->seek(0);
        m_bytesSent = 0;
        Q_EMIT progressChanged();
      }

      m_timer.start();
      Q_EMIT activeChanged();
      break;
    }

    case RawBinary: {
      // Rewind if previous file was fully sent
      if (transmissionProgress() >= 100) {
        m_file.seek(0);
        m_bytesSent = 0;
        Q_EMIT progressChanged();
      }

      m_timer.start();
      Q_EMIT activeChanged();
      break;
    }

    case XModem: {
      m_xmodem->setUse1K(false);
      m_xmodem->setMaxRetries(m_maxRetries);
      m_xmodem->setTimeoutMs(m_protocolTimeout);
      m_xmodem->startTransfer(m_filePath);
      Q_EMIT activeChanged();
      break;
    }

    case XModem1K: {
      m_xmodem->setUse1K(true);
      m_xmodem->setMaxRetries(m_maxRetries);
      m_xmodem->setTimeoutMs(m_protocolTimeout);
      m_xmodem->startTransfer(m_filePath);
      Q_EMIT activeChanged();
      break;
    }

    case YModem: {
      m_ymodem->setMaxRetries(m_maxRetries);
      m_ymodem->setTimeoutMs(m_protocolTimeout);
      m_ymodem->startTransfer(m_filePath);
      Q_EMIT activeChanged();
      break;
    }

    case ZModem: {
      m_zmodem->setBlockSize(m_blockSize);
      m_zmodem->setMaxRetries(m_maxRetries);
      m_zmodem->setTimeoutMs(m_protocolTimeout);
      m_zmodem->startTransfer(m_filePath);
      Q_EMIT activeChanged();
      break;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// External module connections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets up signal/slot connections with other modules.
 */
void IO::FileTransmission::setupExternalConnections()
{
  // Stop transmission and refresh UI on connection state changes
  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &FileTransmission::stopTransmission);

  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &FileTransmission::fileChanged);

  // Clean up on application quit
  connect(qApp, &QApplication::aboutToQuit, this, &FileTransmission::closeFile);

  // Retranslate on language change
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::FileTransmission::fileChanged);

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::FileTransmission::transferModeChanged);
}

//--------------------------------------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the raw binary block size.
 */
void IO::FileTransmission::setBlockSize(int bytes)
{
  int clamped = qBound(64, bytes, 8192);
  if (m_blockSize != clamped) {
    m_blockSize = clamped;
    Q_EMIT blockSizeChanged();
  }
}

/**
 * @brief Sets the maximum retry count for protocol transfers.
 */
void IO::FileTransmission::setMaxRetries(int retries)
{
  int clamped = qMax(1, retries);
  if (m_maxRetries != clamped) {
    m_maxRetries = clamped;
    Q_EMIT maxRetriesChanged();
  }
}

/**
 * @brief Sets the transfer mode.
 */
void IO::FileTransmission::setTransferMode(int mode)
{
  auto newMode = static_cast<TransferMode>(qBound(0, mode, 5));
  if (m_transferMode != newMode) {
    if (active())
      stopTransmission();

    m_transferMode = newMode;

    // Reset file position and stream for the new mode
    if (m_file.isOpen()) {
      m_file.seek(0);
      m_bytesSent = 0;

      if (m_stream) {
        delete m_stream;
        m_stream = nullptr;
      }

      if (m_transferMode == PlainText)
        m_stream = new QTextStream(&m_file);
    }

    Q_EMIT progressChanged();

    Q_EMIT transferModeChanged();
  }
}

/**
 * @brief Sets the protocol timeout in milliseconds.
 */
void IO::FileTransmission::setProtocolTimeout(int msec)
{
  int clamped = qMax(1000, msec);
  if (m_protocolTimeout != clamped) {
    m_protocolTimeout = clamped;
    Q_EMIT protocolTimeoutChanged();
  }
}

/**
 * @brief Sets the inter-line/block transmission interval.
 */
void IO::FileTransmission::setLineTransmissionInterval(int msec)
{
  const int effective = qMax(0, msec);
  if (m_timer.interval() == effective)
    return;

  m_timer.setInterval(effective);
  Q_EMIT lineTransmissionIntervalChanged();
}

//--------------------------------------------------------------------------------------------------
// Plain text transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends the next line from the file (plain text mode).
 */
void IO::FileTransmission::sendLine()
{
  if (!active())
    return;

  if (!IO::ConnectionManager::instance().isConnected())
    return;

  if (m_stream && !m_stream->atEnd()) {
    auto line = m_stream->readLine();
    if (!line.isEmpty()) {
      if (!line.endsWith("\n"))
        line.append("\n");

      auto data = line.toUtf8();
      (void)IO::ConnectionManager::instance().writeData(data);
      m_bytesSent = m_stream->pos();
      Q_EMIT progressChanged();
    }
  } else {
    stopTransmission();
    m_statusText = tr("Transmission complete");
    Q_EMIT statusTextChanged();
    appendLog(tr("Plain text transmission complete"));
  }
}

//--------------------------------------------------------------------------------------------------
// Raw binary transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends the next binary block from the file.
 */
void IO::FileTransmission::sendRawBlock()
{
  if (!IO::ConnectionManager::instance().isConnected())
    return;

  if (m_file.atEnd()) {
    stopTransmission();
    m_statusText = tr("Transmission complete");
    Q_EMIT statusTextChanged();
    appendLog(tr("Raw binary transmission complete (%1 bytes)").arg(m_bytesSent));
    return;
  }

  QByteArray block = m_file.read(m_blockSize);
  if (!block.isEmpty()) {
    (void)IO::ConnectionManager::instance().writeData(block);
    m_bytesSent += block.size();
    Q_EMIT progressChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Protocol callbacks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles protocol transfer completion.
 */
void IO::FileTransmission::onProtocolFinished(bool success, const QString& errorMessage)
{
  m_speedUpdateTimer.stop();

  if (success) {
    m_statusText = tr("Transfer complete");
    appendLog(tr("Transfer completed successfully (%1 bytes)").arg(m_bytesSent));
  } else {
    m_statusText = tr("Transfer failed: %1").arg(errorMessage);
    appendLog(tr("Transfer failed: %1").arg(errorMessage));
  }

  Q_EMIT statusTextChanged();
  Q_EMIT activeChanged();
}

/**
 * @brief Updates progress from protocol handlers.
 */
void IO::FileTransmission::onProtocolProgress(qint64 sent, qint64 total)
{
  m_bytesSent  = sent;
  m_bytesTotal = total;
  Q_EMIT progressChanged();
}

/**
 * @brief Receives status messages from protocols and adds them to the log.
 */
void IO::FileTransmission::onProtocolStatus(const QString& message)
{
  m_statusText = message;
  Q_EMIT statusTextChanged();
  appendLog(message);

  if (message.contains("NAK") || message.contains("retry", Qt::CaseInsensitive)
      || message.contains("Timeout")) {
    ++m_errorCount;
    Q_EMIT errorCountChanged();
  }
}

/**
 * @brief Writes data to the device on behalf of a protocol handler.
 */
void IO::FileTransmission::onProtocolWriteRequested(const QByteArray& data)
{
  (void)IO::ConnectionManager::instance().writeData(data);
}

/**
 * @brief Routes incoming device data to the active protocol handler.
 */
void IO::FileTransmission::onRawDataReceived(const QByteArray& data)
{
  if (!active())
    return;

  switch (m_transferMode) {
    case XModem:
    case XModem1K:
      m_xmodem->processInput(data);
      break;
    case YModem:
      m_ymodem->processInput(data);
      break;
    case ZModem:
      m_zmodem->processInput(data);
      break;
    default:
      break;
  }
}

/**
 * @brief Periodically computes and updates the transfer speed display.
 */
void IO::FileTransmission::updateTransferSpeed()
{
  if (!active())
    return;

  qint64 elapsed = m_speedTimer.elapsed();
  if (elapsed <= 0)
    return;

  qint64 bytesPerSec = ((m_bytesSent - m_lastSpeedBytes) * 1000) / elapsed;
  m_lastSpeedBytes   = m_bytesSent;
  m_speedTimer.restart();

  if (bytesPerSec < 1024)
    m_transferSpeed = tr("%1 B/s").arg(bytesPerSec);
  else if (bytesPerSec < 1024 * 1024)
    m_transferSpeed = tr("%1 KB/s").arg(bytesPerSec / 1024.0, 0, 'f', 1);
  else
    m_transferSpeed = tr("%1 MB/s").arg(bytesPerSec / (1024.0 * 1024.0), 0, 'f', 2);

  Q_EMIT transferSpeedChanged();
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a timestamped entry to the activity log.
 */
void IO::FileTransmission::appendLog(const QString& message)
{
  auto timestamp = QTime::currentTime().toString("HH:mm:ss");
  m_logEntries.append(QStringLiteral("[%1] %2").arg(timestamp, message));

  // Trim oldest entries when capacity is exceeded
  while (m_logEntries.size() > kMaxLogEntries)
    m_logEntries.removeFirst();

  Q_EMIT logChanged();
}

/**
 * @brief Connects a protocol's signals to the FileTransmission controller slots.
 */
void IO::FileTransmission::connectProtocol(IO::Protocols::Protocol* protocol)
{
  connect(
    protocol, &IO::Protocols::Protocol::finished, this, &FileTransmission::onProtocolFinished);
  connect(protocol,
          &IO::Protocols::Protocol::progressChanged,
          this,
          &FileTransmission::onProtocolProgress);
  connect(
    protocol, &IO::Protocols::Protocol::statusMessage, this, &FileTransmission::onProtocolStatus);
  connect(protocol,
          &IO::Protocols::Protocol::writeRequested,
          this,
          &FileTransmission::onProtocolWriteRequested);
}
