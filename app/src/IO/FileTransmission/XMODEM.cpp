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

#include "IO/FileTransmission/XMODEM.h"

#include "IO/FileTransmission/CRC.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an XMODEM protocol handler.
 */
IO::Protocols::XMODEM::XMODEM(QObject* parent)
  : Protocol(parent)
  , m_state(State::Idle)
  , m_blockNumber(1)
  , m_retryCount(0)
  , m_maxRetries(10)
  , m_timeoutMs(10000)
  , m_use1K(false)
  , m_bytesSent(0)
  , m_fileSize(0)
{
  m_timeoutTimer.setSingleShot(true);
  connect(&m_timeoutTimer, &QTimer::timeout, this, &XMODEM::handleTimeout);
}

//--------------------------------------------------------------------------------------------------
// Protocol interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the human-readable protocol name.
 */
QString IO::Protocols::XMODEM::protocolName() const
{
  return m_use1K ? QStringLiteral("XMODEM-1K") : QStringLiteral("XMODEM");
}

/**
 * @brief Returns whether a transfer is currently in progress.
 */
bool IO::Protocols::XMODEM::isActive() const
{
  return m_state != State::Idle && m_state != State::Done;
}

/**
 * @brief Starts an XMODEM file transfer.
 * @param filePath Path to the file to transmit.
 */
void IO::Protocols::XMODEM::startTransfer(const QString& filePath)
{
  // Abort any existing transfer
  if (isActive())
    cancelTransfer();

  // Open the file
  m_file.setFileName(filePath);
  if (!m_file.open(QIODevice::ReadOnly)) {
    Q_EMIT finished(false, tr("Cannot open file: %1").arg(m_file.errorString()));
    return;
  }

  // Initialize state
  m_fileSize    = m_file.size();
  m_bytesSent   = 0;
  m_blockNumber = 1;
  m_retryCount  = 0;
  m_state       = State::WaitingForStart;

  Q_EMIT statusMessage(tr("Waiting for receiver..."));
  Q_EMIT progressChanged(0, m_fileSize);

  // Start timeout for initial handshake
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Cancels the current transfer by sending CAN bytes.
 */
void IO::Protocols::XMODEM::cancelTransfer()
{
  if (!isActive())
    return;

  sendCancel();
  resetState();
  Q_EMIT statusMessage(tr("Transfer cancelled"));
  Q_EMIT finished(false, tr("Transfer cancelled by user"));
}

/**
 * @brief Processes bytes received from the remote device.
 * @param data Raw bytes from the device.
 */
void IO::Protocols::XMODEM::processInput(const QByteArray& data)
{
  // Process each byte through the state machine
  for (const char byte : data) {
    const quint8 ch = static_cast<quint8>(byte);

    switch (m_state) {
      // Waiting for receiver to send 'C' (CRC mode)
      case State::WaitingForStart:
        if (ch == kCRC) {
          m_timeoutTimer.stop();
          m_state = State::SendingBlocks;
          Q_EMIT statusMessage(tr("Receiver ready (CRC mode), sending data..."));
          sendBlock();
        }
        break;

      // Waiting for ACK/NAK after a data block
      case State::WaitingForAck:
        if (ch == kACK) {
          m_timeoutTimer.stop();
          m_retryCount  = 0;
          m_blockNumber = static_cast<quint8>((m_blockNumber + 1) & 0xFF);
          m_state       = State::SendingBlocks;

          // Check if we've sent everything
          if (m_file.atEnd())
            sendEOT();
          else
            sendBlock();
        } else if (ch == kNAK) {
          m_timeoutTimer.stop();
          ++m_retryCount;
          if (m_retryCount >= m_maxRetries) {
            sendCancel();
            resetState();
            Q_EMIT statusMessage(tr("Too many retries, transfer aborted"));
            Q_EMIT finished(false, tr("Maximum retries exceeded"));
            return;
          }

          Q_EMIT statusMessage(tr("NAK received, retrying block %1 (%2/%3)")
                                 .arg(m_blockNumber)
                                 .arg(m_retryCount)
                                 .arg(m_maxRetries));

          // Rewind file to re-read the current block
          int blockSize     = m_use1K ? 1024 : 128;
          qint64 blockStart = qMax(static_cast<qint64>(0), m_bytesSent - blockSize);
          m_bytesSent = blockStart;
          if (!m_file.seek(blockStart)) [[unlikely]] {
            Q_EMIT finished(false, tr("Failed to seek in file"));
            return;
          }
          sendBlock();
        } else if (ch == kCAN) {
          m_timeoutTimer.stop();
          resetState();
          Q_EMIT statusMessage(tr("Transfer cancelled by receiver"));
          Q_EMIT finished(false, tr("Receiver cancelled the transfer"));
        }
        break;

      // Waiting for ACK after EOT
      case State::WaitingForEOTAck:
        if (ch == kACK) {
          m_timeoutTimer.stop();
          resetState();
          Q_EMIT statusMessage(tr("Transfer complete"));
          Q_EMIT finished(true, QString());
        } else if (ch == kNAK) {
          // Resend EOT
          m_timeoutTimer.stop();
          sendEOT();
        }
        break;

      default:
        break;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether 1024-byte blocks are enabled.
 */
bool IO::Protocols::XMODEM::use1K() const noexcept
{
  return m_use1K;
}

/**
 * @brief Enables or disables 1024-byte block mode.
 */
void IO::Protocols::XMODEM::setUse1K(bool enabled)
{
  m_use1K = enabled;
}

/**
 * @brief Returns the maximum number of retries per block.
 */
int IO::Protocols::XMODEM::maxRetries() const noexcept
{
  return m_maxRetries;
}

/**
 * @brief Sets the maximum number of retries per block.
 */
void IO::Protocols::XMODEM::setMaxRetries(int retries)
{
  m_maxRetries = qMax(1, retries);
}

/**
 * @brief Returns the timeout in milliseconds.
 */
int IO::Protocols::XMODEM::timeoutMs() const noexcept
{
  return m_timeoutMs;
}

/**
 * @brief Sets the timeout in milliseconds for waiting for receiver responses.
 */
void IO::Protocols::XMODEM::setTimeoutMs(int ms)
{
  m_timeoutMs = qMax(1000, ms);
}

//--------------------------------------------------------------------------------------------------
// Block transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the next block from the file and sends it.
 */
void IO::Protocols::XMODEM::sendBlock()
{
  // Read the next block from file, send EOT if nothing left
  int blockSize   = m_use1K ? 1024 : 128;
  QByteArray data = m_file.read(blockSize);
  if (data.isEmpty()) {
    sendEOT();
    return;
  }

  // Pad with SUB (0x1A) if the block is incomplete
  while (data.size() < blockSize)
    data.append(static_cast<char>(0x1A));

  // Build and send the framed block
  QByteArray packet = buildBlock(data, m_blockNumber);
  Q_EMIT writeRequested(packet);

  // Update progress
  m_bytesSent = qMin(m_bytesSent + blockSize, m_fileSize);
  Q_EMIT progressChanged(m_bytesSent, m_fileSize);
  Q_EMIT statusMessage(tr("Sending block %1 (%2 bytes)").arg(m_blockNumber).arg(m_bytesSent));

  // Advance state
  m_state = State::WaitingForAck;
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Sends the End-of-Transmission marker.
 */
void IO::Protocols::XMODEM::sendEOT()
{
  // Transition to EOT state and send the marker
  m_state = State::WaitingForEOTAck;
  Q_EMIT writeRequested(QByteArray(1, static_cast<char>(kEOT)));
  Q_EMIT statusMessage(tr("Sending EOT..."));
  m_timeoutTimer.start(m_timeoutMs);
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resets all transfer state to idle.
 */
void IO::Protocols::XMODEM::resetState()
{
  // Reset all transfer state to defaults
  m_timeoutTimer.stop();
  m_state       = State::Idle;
  m_blockNumber = 1;
  m_retryCount  = 0;
  m_bytesSent   = 0;
  m_fileSize    = 0;

  if (m_file.isOpen())
    m_file.close();
}

/**
 * @brief Handles a timeout while waiting for a receiver response.
 */
void IO::Protocols::XMODEM::handleTimeout()
{
  // Ignore timeouts when idle
  if (!isActive())
    return;

  // Abort if maximum retries exceeded
  ++m_retryCount;
  if (m_retryCount >= m_maxRetries) {
    sendCancel();
    resetState();
    Q_EMIT statusMessage(tr("Transfer timed out"));
    Q_EMIT finished(false, tr("Timeout: no response from receiver"));
    return;
  }

  Q_EMIT statusMessage(tr("Timeout, retrying (%1/%2)...").arg(m_retryCount).arg(m_maxRetries));

  // Resend the current block or EOT
  if (m_state == State::WaitingForEOTAck) {
    sendEOT();
  } else if (m_state == State::WaitingForAck) {
    int blockSize     = m_use1K ? 1024 : 128;
    qint64 blockStart = qMax(static_cast<qint64>(0), m_bytesSent - blockSize);
    m_bytesSent = blockStart;
    if (!m_file.seek(blockStart)) [[unlikely]] {
      Q_EMIT finished(false, tr("Failed to seek in file"));
      return;
    }
    sendBlock();
  } else if (m_state == State::WaitingForStart) {
    m_timeoutTimer.start(m_timeoutMs);
  }
}

/**
 * @brief Builds a complete XMODEM block with header, data, and CRC.
 * @param data Block payload (must be exactly 128 or 1024 bytes).
 * @param blockNum Block sequence number (wraps at 256).
 * @return Framed block ready for transmission.
 */
QByteArray IO::Protocols::XMODEM::buildBlock(const QByteArray& data, quint8 blockNum)
{
  // Allocate packet buffer
  QByteArray packet;
  packet.reserve(data.size() + 5);

  // Header byte
  if (data.size() == 1024)
    packet.append(static_cast<char>(kSTX));
  else
    packet.append(static_cast<char>(kSOH));

  // Block number and complement
  packet.append(static_cast<char>(blockNum));
  packet.append(static_cast<char>(~blockNum));

  // Data payload
  packet.append(data);

  // CRC-16
  quint16 crc = CRC::crc16(reinterpret_cast<const quint8*>(data.constData()), data.size());
  packet.append(static_cast<char>((crc >> 8) & 0xFF));
  packet.append(static_cast<char>(crc & 0xFF));

  return packet;
}

/**
 * @brief Sends a cancel sequence (multiple CAN bytes).
 */
void IO::Protocols::XMODEM::sendCancel()
{
  QByteArray cancel(5, static_cast<char>(kCAN));
  Q_EMIT writeRequested(cancel);
}
