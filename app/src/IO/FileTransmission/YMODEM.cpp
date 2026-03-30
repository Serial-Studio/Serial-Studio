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

#include "IO/FileTransmission/YMODEM.h"

#include <QFileInfo>

#include "IO/FileTransmission/CRC.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a YMODEM protocol handler.
 */
IO::Protocols::YMODEM::YMODEM(QObject* parent) : XMODEM(parent), m_yState(YState::Idle)
{
  // YMODEM always uses 1K blocks for data
  setUse1K(true);
}

//--------------------------------------------------------------------------------------------------
// Protocol interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the human-readable protocol name.
 */
QString IO::Protocols::YMODEM::protocolName() const
{
  return QStringLiteral("YMODEM");
}

/**
 * @brief Starts a YMODEM file transfer.
 * @param filePath Path to the file to transmit.
 */
void IO::Protocols::YMODEM::startTransfer(const QString& filePath)
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

  // Store metadata
  m_filePath    = filePath;
  m_fileSize    = m_file.size();
  m_bytesSent   = 0;
  m_blockNumber = 0;
  m_retryCount  = 0;
  m_yState      = YState::WaitingForInitialC;
  m_state       = State::WaitingForStart;

  Q_EMIT statusMessage(tr("Waiting for receiver..."));
  Q_EMIT progressChanged(0, m_fileSize);

  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Processes bytes received from the remote device (YMODEM state machine).
 * @param data Raw bytes from the device.
 */
void IO::Protocols::YMODEM::processInput(const QByteArray& data)
{
  for (const char byte : data) {
    const quint8 ch = static_cast<quint8>(byte);

    switch (m_yState) {
      // Receiver sends 'C' to start — send block 0 (filename + size)
      case YState::WaitingForInitialC:
        if (ch == kCRC) {
          m_timeoutTimer.stop();
          sendBlock0();
        }
        break;

      // Waiting for ACK of block 0
      case YState::WaitingForBlock0Ack:
        if (ch == kACK) {
          m_timeoutTimer.stop();
          m_yState = YState::WaitingForDataC;
          m_timeoutTimer.start(m_timeoutMs);
        } else if (ch == kCAN) {
          m_timeoutTimer.stop();
          resetState();
          m_yState = YState::Idle;
          Q_EMIT statusMessage(tr("Transfer cancelled by receiver"));
          Q_EMIT finished(false, tr("Receiver cancelled the transfer"));
        }
        break;

      // Receiver sends 'C' after ACK of block 0 — start data blocks
      case YState::WaitingForDataC:
        if (ch == kCRC) {
          m_timeoutTimer.stop();
          m_blockNumber = 1;
          m_yState      = YState::SendingData;
          sendDataBlock();
        }
        break;

      // Waiting for ACK/NAK of a data block
      case YState::WaitingForDataAck:
        if (ch == kACK) {
          m_timeoutTimer.stop();
          m_retryCount = 0;

          // Check if file is fully sent
          if (m_file.atEnd()) {
            m_yState = YState::WaitingForFirstEOTResponse;
            Q_EMIT writeRequested(QByteArray(1, static_cast<char>(kEOT)));
            Q_EMIT statusMessage(tr("Sending first EOT..."));
            m_timeoutTimer.start(m_timeoutMs);
          } else {
            m_yState = YState::SendingData;
            sendDataBlock();
          }
        } else if (ch == kNAK) {
          m_timeoutTimer.stop();
          ++m_retryCount;
          if (m_retryCount >= m_maxRetries) {
            QByteArray cancel(5, static_cast<char>(kCAN));
            Q_EMIT writeRequested(cancel);
            resetState();
            m_yState = YState::Idle;
            Q_EMIT statusMessage(tr("Too many retries, transfer aborted"));
            Q_EMIT finished(false, tr("Maximum retries exceeded"));
            return;
          }

          // Resend current block
          Q_EMIT statusMessage(tr("NAK received, retrying block %1").arg(m_blockNumber));
          int blockSize     = 1024;
          qint64 blockStart = static_cast<qint64>(m_blockNumber - 1) * blockSize;
          m_file.seek(blockStart);
          m_yState = YState::SendingData;
          sendDataBlock();
        } else if (ch == kCAN) {
          m_timeoutTimer.stop();
          resetState();
          m_yState = YState::Idle;
          Q_EMIT statusMessage(tr("Transfer cancelled by receiver"));
          Q_EMIT finished(false, tr("Receiver cancelled the transfer"));
        }
        break;

      // YMODEM requires two EOTs: first one gets NAK, second gets ACK
      case YState::WaitingForFirstEOTResponse:
        if (ch == kNAK || ch == kACK) {
          m_timeoutTimer.stop();
          m_yState = YState::WaitingForSecondEOTAck;
          Q_EMIT writeRequested(QByteArray(1, static_cast<char>(kEOT)));
          Q_EMIT statusMessage(tr("Sending second EOT..."));
          m_timeoutTimer.start(m_timeoutMs);
        }
        break;

      case YState::WaitingForSecondEOTAck:
        if (ch == kACK) {
          m_timeoutTimer.stop();
          m_yState = YState::WaitingForEndBatchC;
          m_timeoutTimer.start(m_timeoutMs);
        }
        // Some receivers send 'C' immediately after ACK of second EOT
        else if (ch == kCRC) {
          m_timeoutTimer.stop();
          sendEndOfBatch();
        }
        break;

      // Receiver sends 'C' after final EOT — send empty block 0
      case YState::WaitingForEndBatchC:
        if (ch == kCRC) {
          m_timeoutTimer.stop();
          sendEndOfBatch();
        }
        break;

      // Waiting for ACK of the empty end-of-batch block 0
      case YState::WaitingForEndBatchAck:
        if (ch == kACK) {
          m_timeoutTimer.stop();
          m_yState = YState::Done;
          resetState();
          Q_EMIT progressChanged(m_fileSize, m_fileSize);
          Q_EMIT statusMessage(tr("Transfer complete"));
          Q_EMIT finished(true, QString());
        }
        break;

      default:
        break;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// YMODEM-specific block helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends block 0 containing the filename and file size.
 */
void IO::Protocols::YMODEM::sendBlock0()
{
  QFileInfo info(m_filePath);

  // Build block 0 payload: filename\0filesize\0
  QByteArray payload;
  payload.append(info.fileName().toUtf8());
  payload.append('\0');
  payload.append(QByteArray::number(m_fileSize));
  payload.append('\0');

  // Pad to 128 bytes (block 0 always uses SOH/128 in YMODEM)
  while (payload.size() < 128)
    payload.append('\0');

  // Build the framed block
  QByteArray packet = buildBlock(payload, 0);

  Q_EMIT writeRequested(packet);
  Q_EMIT statusMessage(
    tr("Sending file header: %1 (%2 bytes)").arg(info.fileName()).arg(m_fileSize));

  m_yState = YState::WaitingForBlock0Ack;
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Sends an empty block 0 to signal end of batch.
 */
void IO::Protocols::YMODEM::sendEndOfBatch()
{
  QByteArray payload(128, '\0');
  QByteArray packet = buildBlock(payload, 0);

  Q_EMIT writeRequested(packet);
  Q_EMIT statusMessage(tr("Sending end-of-batch marker..."));

  m_yState = YState::WaitingForEndBatchAck;
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Reads and sends the next 1K data block.
 */
void IO::Protocols::YMODEM::sendDataBlock()
{
  QByteArray data = m_file.read(1024);
  if (data.isEmpty()) {
    // File fully read, send EOT
    m_yState = YState::WaitingForFirstEOTResponse;
    Q_EMIT writeRequested(QByteArray(1, static_cast<char>(kEOT)));
    Q_EMIT statusMessage(tr("Sending first EOT..."));
    m_timeoutTimer.start(m_timeoutMs);
    return;
  }

  // Pad the last block with SUB (0x1A)
  while (data.size() < 1024)
    data.append(static_cast<char>(0x1A));

  // Build and send
  QByteArray packet = buildBlock(data, m_blockNumber);
  Q_EMIT writeRequested(packet);

  // Update progress
  m_bytesSent = qMin(m_bytesSent + 1024, m_fileSize);
  Q_EMIT progressChanged(m_bytesSent, m_fileSize);
  Q_EMIT statusMessage(
    tr("Sending block %1 (%2/%3 bytes)").arg(m_blockNumber).arg(m_bytesSent).arg(m_fileSize));

  m_blockNumber = static_cast<quint8>((m_blockNumber + 1) & 0xFF);
  m_yState      = YState::WaitingForDataAck;
  m_timeoutTimer.start(m_timeoutMs);
}
