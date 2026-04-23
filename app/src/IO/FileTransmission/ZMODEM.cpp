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

#include "IO/FileTransmission/ZMODEM.h"

#include <QFileInfo>

#include "IO/FileTransmission/CRC.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a ZMODEM protocol handler.
 */
IO::Protocols::ZMODEM::ZMODEM(QObject* parent)
  : Protocol(parent)
  , m_state(State::Idle)
  , m_parseState(ParseState::Idle)
  , m_fileSize(0)
  , m_bytesSent(0)
  , m_fileOffset(0)
  , m_blockSize(1024)
  , m_timeoutMs(15000)
  , m_maxRetries(10)
  , m_retryCount(0)
  , m_headerBytesExpected(0)
  , m_zdleEscape(false)
{
  m_timeoutTimer.setSingleShot(true);
  connect(&m_timeoutTimer, &QTimer::timeout, this, &ZMODEM::handleTimeout);
}

//--------------------------------------------------------------------------------------------------
// Protocol interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the human-readable protocol name.
 */
QString IO::Protocols::ZMODEM::protocolName() const
{
  return QStringLiteral("ZMODEM");
}

/**
 * @brief Returns whether a transfer is currently in progress.
 */
bool IO::Protocols::ZMODEM::isActive() const
{
  return m_state != State::Idle && m_state != State::Done;
}

/**
 * @brief Starts a ZMODEM file transfer.
 */
void IO::Protocols::ZMODEM::startTransfer(const QString& filePath)
{
  Q_ASSERT(!filePath.isEmpty());
  Q_ASSERT(m_maxRetries > 0);

  // Abort any existing transfer
  if (isActive())
    cancelTransfer();

  // Open the file
  m_file.setFileName(filePath);
  if (!m_file.open(QIODevice::ReadOnly)) {
    Q_EMIT finished(false, tr("Cannot open file: %1").arg(m_file.errorString()));
    return;
  }

  m_filePath   = filePath;
  m_fileSize   = m_file.size();
  m_bytesSent  = 0;
  m_fileOffset = 0;
  m_retryCount = 0;
  m_parseState = ParseState::Idle;
  m_headerBuf.clear();
  m_zdleEscape = false;

  // Reject >4 GiB up-front — ZMODEM's base offsets are 32-bit unsigned.
  static constexpr qint64 kZmodemMaxFileSize = static_cast<qint64>(0xFFFFFFFFULL);
  if (m_fileSize > kZmodemMaxFileSize) [[unlikely]] {
    m_file.close();
    Q_EMIT finished(false,
                    tr("File is too large for ZMODEM (%1 bytes, limit 4 GiB).").arg(m_fileSize));
    return;
  }

  Q_EMIT progressChanged(0, m_fileSize);

  // Send ZRQINIT to initiate the session
  sendZRQINIT();
}

/**
 * @brief Cancels the current transfer.
 */
void IO::Protocols::ZMODEM::cancelTransfer()
{
  if (!isActive())
    return;

  // Send cancel and reset state
  sendCancel();
  m_timeoutTimer.stop();
  m_state      = State::Idle;
  m_parseState = ParseState::Idle;

  if (m_file.isOpen())
    m_file.close();

  Q_EMIT statusMessage(tr("Transfer cancelled"));
  Q_EMIT finished(false, tr("Transfer cancelled by user"));
}

/**
 * @brief Processes bytes received from the remote device.
 */
void IO::Protocols::ZMODEM::processInput(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(isActive());

  for (const char byte : data) {
    const quint8 ch = static_cast<quint8>(byte);

    switch (m_parseState) {
      case ParseState::Idle:
        if (ch == kZPAD)
          m_parseState = ParseState::GotZPAD;

        break;

      case ParseState::GotZPAD:
        if (ch == kZPAD)
          m_parseState = ParseState::GotSecondZPAD;
        else if (ch == kZDLE)
          m_parseState = ParseState::GotZDLE;
        else
          m_parseState = ParseState::Idle;

        break;

      case ParseState::GotSecondZPAD:
        if (ch == kZDLE)
          m_parseState = ParseState::GotZDLE;
        else
          m_parseState = ParseState::Idle;

        break;

      case ParseState::GotZDLE:
        if (ch == kZHEX) {
          m_parseState = ParseState::ReadingHexHeader;
          m_headerBuf.clear();
          m_headerBytesExpected = 14;
        } else if (ch == kZBIN32) {
          m_parseState = ParseState::ReadingBin32Header;
          m_headerBuf.clear();
          m_headerBytesExpected = 9;
          m_zdleEscape          = false;
        } else if (ch == kZBIN) {
          m_parseState = ParseState::ReadingBinHeader;
          m_headerBuf.clear();
          m_headerBytesExpected = 7;
          m_zdleEscape          = false;
        } else {
          m_parseState = ParseState::Idle;
        }

        break;

      case ParseState::ReadingHexHeader:
        processHexByte(ch);
        break;

      case ParseState::ReadingBin32Header:
        processBin32Byte(ch);
        break;

      case ParseState::ReadingBinHeader:
        processBinByte(ch);
        break;
    }
  }
}

/**
 * @brief Processes one byte of an incoming hex header.
 */
void IO::Protocols::ZMODEM::processHexByte(quint8 ch)
{
  // Bound m_headerBuf — peer without a line terminator would otherwise OOM.
  static constexpr int kMaxHexHeaderBytes = 32;

  // Accumulate until line terminator
  if (ch != '\r' && ch != '\n') {
    if (m_headerBuf.size() >= kMaxHexHeaderBytes) [[unlikely]] {
      m_parseState = ParseState::Idle;
      m_headerBuf.clear();
      return;
    }

    m_headerBuf.append(static_cast<char>(ch));
    return;
  }

  // Incomplete header, reset
  if (m_headerBuf.size() < 14) {
    m_parseState = ParseState::Idle;
    return;
  }

  // Decode type and argument from hex
  const auto type_bytes = QByteArray::fromHex(m_headerBuf.mid(0, 2));
  if (type_bytes.isEmpty()) [[unlikely]] {
    m_parseState = ParseState::Idle;
    return;
  }

  quint8 type          = static_cast<quint8>(type_bytes.at(0));
  QByteArray arg_bytes = QByteArray::fromHex(m_headerBuf.mid(2, 8));
  quint32 arg          = 0;
  if (arg_bytes.size() == 4) {
    arg = static_cast<quint32>(static_cast<quint8>(arg_bytes[0]))
        | (static_cast<quint32>(static_cast<quint8>(arg_bytes[1])) << 8)
        | (static_cast<quint32>(static_cast<quint8>(arg_bytes[2])) << 16)
        | (static_cast<quint32>(static_cast<quint8>(arg_bytes[3])) << 24);
  }

  // Validate CRC-16 trailer — corrupted headers would otherwise be obeyed.
  const QByteArray crc_bytes = QByteArray::fromHex(m_headerBuf.mid(10, 4));
  if (crc_bytes.size() == 2) {
    quint8 payload[5];
    payload[0] = type;
    payload[1] = static_cast<quint8>(arg & 0xFF);
    payload[2] = static_cast<quint8>((arg >> 8) & 0xFF);
    payload[3] = static_cast<quint8>((arg >> 16) & 0xFF);
    payload[4] = static_cast<quint8>((arg >> 24) & 0xFF);

    const quint16 calc_crc = CRC::crc16(payload, 5);
    const quint16 rx_crc   = static_cast<quint16>((static_cast<quint8>(crc_bytes[0]) << 8)
                                                | static_cast<quint8>(crc_bytes[1]));
    if (calc_crc != rx_crc) [[unlikely]] {
      Q_EMIT statusMessage(tr("Hex header CRC mismatch, dropping frame"));
      m_parseState = ParseState::Idle;
      return;
    }
  }

  parseReceivedHeader(type, arg);
  m_parseState = ParseState::Idle;
}

/**
 * @brief Processes one byte of an incoming binary-32 header (ZDLE-decoded).
 */
void IO::Protocols::ZMODEM::processBin32Byte(quint8 ch)
{
  // Bound m_headerBuf — a hostile peer sending only ZDLE-escape prefixes would otherwise OOM.
  static constexpr int kMaxBinHeaderBytes = 128;

  // Drop the in-flight header if growth exceeds the hard cap
  if (m_headerBuf.size() >= kMaxBinHeaderBytes) [[unlikely]] {
    m_parseState = ParseState::Idle;
    m_headerBuf.clear();
    m_zdleEscape = false;
    return;
  }

  // Decode ZDLE-escaped byte
  if (m_zdleEscape) {
    m_headerBuf.append(static_cast<char>(ch ^ 0x40));
    m_zdleEscape = false;
  } else if (ch == kZDLE) {
    m_zdleEscape = true;
    return;
  } else {
    m_headerBuf.append(static_cast<char>(ch));
  }

  if (m_headerBuf.size() < m_headerBytesExpected)
    return;

  // Extract type and argument
  quint8 type = static_cast<quint8>(m_headerBuf[0]);
  quint32 arg = static_cast<quint32>(static_cast<quint8>(m_headerBuf[1]))
              | (static_cast<quint32>(static_cast<quint8>(m_headerBuf[2])) << 8)
              | (static_cast<quint32>(static_cast<quint8>(m_headerBuf[3])) << 16)
              | (static_cast<quint32>(static_cast<quint8>(m_headerBuf[4])) << 24);

  parseReceivedHeader(type, arg);
  m_parseState = ParseState::Idle;
}

/**
 * @brief Processes one byte of an incoming binary header (ZDLE-decoded).
 */
void IO::Protocols::ZMODEM::processBinByte(quint8 ch)
{
  // Bound m_headerBuf — a hostile peer sending only ZDLE-escape prefixes would otherwise OOM.
  static constexpr int kMaxBinHeaderBytes = 128;

  // Drop the in-flight header if growth exceeds the hard cap
  if (m_headerBuf.size() >= kMaxBinHeaderBytes) [[unlikely]] {
    m_parseState = ParseState::Idle;
    m_headerBuf.clear();
    m_zdleEscape = false;
    return;
  }

  // Decode ZDLE-escaped byte
  if (m_zdleEscape) {
    m_headerBuf.append(static_cast<char>(ch ^ 0x40));
    m_zdleEscape = false;
  } else if (ch == kZDLE) {
    m_zdleEscape = true;
    return;
  } else {
    m_headerBuf.append(static_cast<char>(ch));
  }

  if (m_headerBuf.size() < m_headerBytesExpected)
    return;

  // Extract type and argument
  quint8 type = static_cast<quint8>(m_headerBuf[0]);
  quint32 arg = static_cast<quint32>(static_cast<quint8>(m_headerBuf[1]))
              | (static_cast<quint32>(static_cast<quint8>(m_headerBuf[2])) << 8)
              | (static_cast<quint32>(static_cast<quint8>(m_headerBuf[3])) << 16)
              | (static_cast<quint32>(static_cast<quint8>(m_headerBuf[4])) << 24);

  parseReceivedHeader(type, arg);
  m_parseState = ParseState::Idle;
}

//--------------------------------------------------------------------------------------------------
// Configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the data subpacket size.
 */
int IO::Protocols::ZMODEM::blockSize() const noexcept
{
  return m_blockSize;
}

/**
 * @brief Sets the data subpacket size (clamped to 64–8192).
 */
void IO::Protocols::ZMODEM::setBlockSize(int size)
{
  m_blockSize = qBound(64, size, 8192);
}

/**
 * @brief Returns the timeout in milliseconds.
 */
int IO::Protocols::ZMODEM::timeoutMs() const noexcept
{
  return m_timeoutMs;
}

/**
 * @brief Sets the timeout in milliseconds.
 */
void IO::Protocols::ZMODEM::setTimeoutMs(int ms)
{
  m_timeoutMs = qMax(1000, ms);
}

/**
 * @brief Returns the maximum number of retries.
 */
int IO::Protocols::ZMODEM::maxRetries() const noexcept
{
  return m_maxRetries;
}

/**
 * @brief Sets the maximum number of retries.
 */
void IO::Protocols::ZMODEM::setMaxRetries(int retries)
{
  m_maxRetries = qMax(1, retries);
}

//--------------------------------------------------------------------------------------------------
// Transmit state machine
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends ZRQINIT to invite the receiver to begin.
 */
void IO::Protocols::ZMODEM::sendZRQINIT()
{
  // Send auto-start sequence followed by ZRQINIT header
  m_state = State::SentZRQINIT;
  QByteArray autoStart;
  autoStart.append("rz\r");
  autoStart.append(buildHexHeader(kZRQINIT, 0));
  Q_EMIT writeRequested(autoStart);

  Q_EMIT statusMessage(tr("Sending ZRQINIT, waiting for receiver..."));
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Sends the ZFILE header followed by file metadata subpacket.
 */
void IO::Protocols::ZMODEM::sendZFILE()
{
  // Send ZFILE binary header
  m_state = State::SentZFILE;
  Q_EMIT writeRequested(buildBin32Header(kZFILE, 0));

  // Build file metadata subpacket
  QFileInfo info(m_filePath);
  QByteArray fileInfo;
  fileInfo.append(info.fileName().toUtf8());
  fileInfo.append('\0');
  fileInfo.append(QByteArray::number(m_fileSize));
  fileInfo.append(' ');
  fileInfo.append(QByteArray::number(info.lastModified().toSecsSinceEpoch()));
  fileInfo.append(" 0 0 0 0");
  fileInfo.append('\0');

  // Send as ZCRCW subpacket and wait for ZACK
  Q_EMIT writeRequested(buildSubpacket(fileInfo, kZCRCW));

  Q_EMIT statusMessage(tr("Sending file info: %1 (%2 bytes)").arg(info.fileName()).arg(m_fileSize));
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Initiates async file data streaming as ZDATA subpackets.
 */
void IO::Protocols::ZMODEM::sendDataSubpackets()
{
  Q_ASSERT(m_file.isOpen());
  Q_ASSERT(m_fileSize > 0);

  m_state = State::SendingData;

  // Seek to requested offset for crash recovery
  if (!m_file.seek(m_fileOffset)) [[unlikely]] {
    Q_EMIT finished(false, tr("Failed to seek to offset %1").arg(m_fileOffset));
    return;
  }

  m_bytesSent = m_fileOffset;

  // Send ZDATA header and begin async chunk transmission
  Q_EMIT writeRequested(buildBin32Header(kZDATA, static_cast<quint32>(m_fileOffset)));
  sendNextDataChunk();
}

/**
 * @brief Sends a batch of data chunks, then yields to the event loop.
 */
void IO::Protocols::ZMODEM::sendNextDataChunk()
{
  Q_ASSERT(m_blockSize >= 64 && m_blockSize <= 8192);
  Q_ASSERT(m_file.isOpen());

  if (m_state != State::SendingData)
    return;

  // Send up to kChunksPerYield subpackets before yielding
  for (int i = 0; i < kChunksPerYield && !m_file.atEnd(); ++i) {
    QByteArray chunk = m_file.read(m_blockSize);
    if (chunk.isEmpty()) {
      qWarning() << "[ZMODEM] File read returned empty data at offset" << m_bytesSent;
      break;
    }

    if (chunk.size() > m_blockSize) [[unlikely]] {
      Q_EMIT finished(false, tr("File read returned more data than requested"));
      return;
    }

    m_bytesSent += chunk.size();
    Q_EMIT progressChanged(m_bytesSent, m_fileSize);

    // ZCRCG for intermediate, ZCRCE for last
    if (m_file.atEnd())
      Q_EMIT writeRequested(buildSubpacket(chunk, kZCRCE));
    else
      Q_EMIT writeRequested(buildSubpacket(chunk, kZCRCG));
  }

  // Yield to event loop or finalize
  if (!m_file.atEnd() && m_state == State::SendingData)
    QTimer::singleShot(0, this, &ZMODEM::sendNextDataChunk);
  else if (m_state == State::SendingData)
    sendZEOF();
}

/**
 * @brief Sends the ZEOF header.
 */
void IO::Protocols::ZMODEM::sendZEOF()
{
  m_state = State::SentZEOF;
  Q_EMIT writeRequested(buildBin32Header(kZEOF, static_cast<quint32>(m_fileSize)));
  Q_EMIT statusMessage(tr("File data sent, waiting for confirmation..."));
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Sends the ZFIN header to end the session.
 */
void IO::Protocols::ZMODEM::sendZFIN()
{
  m_state = State::SentZFIN;
  Q_EMIT writeRequested(buildHexHeader(kZFIN, 0));
  Q_EMIT statusMessage(tr("Sending ZFIN..."));
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Sends a cancel sequence (5 CAN + 5 backspace).
 */
void IO::Protocols::ZMODEM::sendCancel()
{
  QByteArray cancel;
  for (int i = 0; i < 5; ++i)
    cancel.append(static_cast<char>(0x18));
  for (int i = 0; i < 5; ++i)
    cancel.append(static_cast<char>(0x08));

  Q_EMIT writeRequested(cancel);
}

//--------------------------------------------------------------------------------------------------
// Header parsing / response handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles a fully parsed receiver header.
 */
void IO::Protocols::ZMODEM::parseReceivedHeader(quint8 type, quint32 arg)
{
  Q_ASSERT(type <= kZCAN);
  Q_ASSERT(isActive());

  m_timeoutTimer.stop();

  switch (type) {
    // Receiver ready — send file info or close session
    case kZRINIT:
      if (m_state == State::SentZRQINIT) {
        Q_EMIT statusMessage(tr("Receiver ready, sending file info..."));
        sendZFILE();
      } else if (m_state == State::SentZEOF) {
        // File accepted, send ZFIN to close session
        Q_EMIT progressChanged(m_fileSize, m_fileSize);
        sendZFIN();
      }
      break;

    // Resume from requested offset — clamp so a bogus arg can't seek past EOF.
    case kZRPOS:
      m_fileOffset = qBound<qint64>(0, static_cast<qint64>(arg), m_fileSize);
      Q_EMIT statusMessage(tr("Receiver requests data from offset %1").arg(m_fileOffset));
      sendDataSubpackets();
      break;

    case kZACK:
      break;

    // File skipped by receiver
    case kZSKIP:
      Q_EMIT statusMessage(tr("Receiver skipped the file"));
      sendZFIN();
      break;

    // NAK — retry current phase
    case kZNAK:
      ++m_retryCount;
      if (m_retryCount >= m_maxRetries) {
        sendCancel();
        m_state = State::Idle;
        if (m_file.isOpen())
          m_file.close();
        Q_EMIT statusMessage(tr("Too many errors, transfer aborted"));
        Q_EMIT finished(false, tr("Maximum retries exceeded"));
        return;
      }

      Q_EMIT statusMessage(
        tr("NAK received, retrying (%1/%2)...").arg(m_retryCount).arg(m_maxRetries));

      if (m_state == State::SentZRQINIT)
        sendZRQINIT();
      else if (m_state == State::SentZFILE)
        sendZFILE();
      else if (m_state == State::SentZEOF)
        sendDataSubpackets();
      break;

    // Session complete — send "OO" (Over and Out)
    case kZFIN:
      Q_EMIT writeRequested(QByteArray("OO"));
      m_state = State::Done;

      if (m_file.isOpen())
        m_file.close();

      Q_EMIT statusMessage(tr("Transfer complete"));
      Q_EMIT finished(true, QString());
      break;

    // Receiver cancelled or aborted
    case kZABORT:
    case kZCAN:
      m_state = State::Idle;
      if (m_file.isOpen())
        m_file.close();
      Q_EMIT statusMessage(tr("Transfer cancelled by receiver"));
      Q_EMIT finished(false, tr("Receiver cancelled the transfer"));
      break;

    // Receiver file error
    case kZFERR:
      m_state = State::Idle;
      if (m_file.isOpen())
        m_file.close();
      Q_EMIT statusMessage(tr("Receiver reported a file error"));
      Q_EMIT finished(false, tr("Receiver reported a file error"));
      break;

    default:
      break;
  }
}

//--------------------------------------------------------------------------------------------------
// Header building
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a ZMODEM hex header (ZPAD ZPAD ZDLE ZHEX type[2hex] arg[8hex] crc[4hex] CR LF).
 */
QByteArray IO::Protocols::ZMODEM::buildHexHeader(quint8 type, quint32 arg)
{
  Q_ASSERT(type <= kZCAN);
  Q_ASSERT(isActive());

  QByteArray header;
  header.reserve(32);

  // Preamble + type + argument (little-endian hex)
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZDLE));
  header.append(static_cast<char>(kZHEX));
  header.append(toHex(type));
  header.append(toHex(static_cast<quint8>(arg & 0xFF)));
  header.append(toHex(static_cast<quint8>((arg >> 8) & 0xFF)));
  header.append(toHex(static_cast<quint8>((arg >> 16) & 0xFF)));
  header.append(toHex(static_cast<quint8>((arg >> 24) & 0xFF)));

  // Append CRC-16
  quint8 crcData[5];
  crcData[0]  = type;
  crcData[1]  = static_cast<quint8>(arg & 0xFF);
  crcData[2]  = static_cast<quint8>((arg >> 8) & 0xFF);
  crcData[3]  = static_cast<quint8>((arg >> 16) & 0xFF);
  crcData[4]  = static_cast<quint8>((arg >> 24) & 0xFF);
  quint16 crc = CRC::crc16(crcData, 5);
  header.append(toHex(static_cast<quint8>((crc >> 8) & 0xFF)));
  header.append(toHex(static_cast<quint8>(crc & 0xFF)));

  header.append('\r');
  header.append('\n');

  return header;
}

/**
 * @brief Builds a ZMODEM binary-32 header (ZPAD ZPAD ZDLE ZBIN32 type arg[4] crc32[4], ZDLE-encoded).
 */
QByteArray IO::Protocols::ZMODEM::buildBin32Header(quint8 type, quint32 arg)
{
  Q_ASSERT(type <= kZCAN);
  Q_ASSERT(m_file.isOpen());

  QByteArray header;
  header.reserve(32);

  // Preamble (not ZDLE-encoded)
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZDLE));
  header.append(static_cast<char>(kZBIN32));

  // Build payload: type + argument bytes (little-endian)
  QByteArray payload;
  payload.append(static_cast<char>(type));
  payload.append(static_cast<char>(arg & 0xFF));
  payload.append(static_cast<char>((arg >> 8) & 0xFF));
  payload.append(static_cast<char>((arg >> 16) & 0xFF));
  payload.append(static_cast<char>((arg >> 24) & 0xFF));

  // Append CRC-32 to payload
  quint32 crc = CRC::crc32(reinterpret_cast<const quint8*>(payload.constData()), payload.size());
  payload.append(static_cast<char>(crc & 0xFF));
  payload.append(static_cast<char>((crc >> 8) & 0xFF));
  payload.append(static_cast<char>((crc >> 16) & 0xFF));
  payload.append(static_cast<char>((crc >> 24) & 0xFF));

  // ZDLE-encode and append
  header.append(zdleEncode(payload));

  return header;
}

/**
 * @brief Builds a ZDLE-encoded data subpacket with CRC-32.
 */
QByteArray IO::Protocols::ZMODEM::buildSubpacket(const QByteArray& data, quint8 frameEnd)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(frameEnd >= kZCRCE && frameEnd <= kZCRCW);

  QByteArray packet;
  packet.reserve(data.size() * 2 + 16);

  // ZDLE-encoded data + frame-end marker
  packet.append(zdleEncode(data));
  packet.append(static_cast<char>(kZDLE));
  packet.append(static_cast<char>(frameEnd));

  // Compute and append ZDLE-encoded CRC-32
  QByteArray crcInput = data;
  crcInput.append(static_cast<char>(frameEnd));
  quint32 crc = CRC::crc32(reinterpret_cast<const quint8*>(crcInput.constData()), crcInput.size());
  QByteArray crcBytes;
  crcBytes.append(static_cast<char>(crc & 0xFF));
  crcBytes.append(static_cast<char>((crc >> 8) & 0xFF));
  crcBytes.append(static_cast<char>((crc >> 16) & 0xFF));
  crcBytes.append(static_cast<char>((crc >> 24) & 0xFF));
  packet.append(zdleEncode(crcBytes));

  return packet;
}

//--------------------------------------------------------------------------------------------------
// ZDLE encoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief ZDLE-encodes a byte sequence (escapes framing/flow-control bytes).
 */
QByteArray IO::Protocols::ZMODEM::zdleEncode(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(data.size() <= 16384);

  QByteArray encoded;
  encoded.reserve(data.size() + data.size() / 4);

  for (const char byte : data) {
    quint8 ch = static_cast<quint8>(byte);
    if (needsEscape(ch)) {
      encoded.append(static_cast<char>(kZDLE));
      encoded.append(static_cast<char>(ch ^ 0x40));
    } else {
      encoded.append(static_cast<char>(ch));
    }
  }

  return encoded;
}

/**
 * @brief Returns whether a byte needs ZDLE escaping.
 */
bool IO::Protocols::ZMODEM::needsEscape(quint8 ch)
{
  switch (ch) {
    case kZDLE:
    case kXON:
    case kXOFF:
    case 0x10:
    case 0x90:
    case 0x91:
    case 0x93:
      return true;

    default:
      return false;
  }
}

//--------------------------------------------------------------------------------------------------
// Hex conversion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts a byte to two ASCII hex characters.
 */
QByteArray IO::Protocols::ZMODEM::toHex(quint8 byte)
{
  static const char hexDigits[] = "0123456789abcdef";
  QByteArray result(2, '\0');
  result[0] = hexDigits[(byte >> 4) & 0x0F];
  result[1] = hexDigits[byte & 0x0F];
  return result;
}

//--------------------------------------------------------------------------------------------------
// Timeout handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles timeout while waiting for receiver response.
 */
void IO::Protocols::ZMODEM::handleTimeout()
{
  Q_ASSERT(m_maxRetries > 0);
  Q_ASSERT(m_timeoutMs >= 1000);

  if (!isActive())
    return;

  // Abort if retries exhausted
  ++m_retryCount;
  if (m_retryCount >= m_maxRetries) {
    sendCancel();
    m_state = State::Idle;
    if (m_file.isOpen())
      m_file.close();
    Q_EMIT statusMessage(tr("Transfer timed out"));
    Q_EMIT finished(false, tr("Timeout: no response from receiver"));
    return;
  }

  Q_EMIT statusMessage(tr("Timeout, retrying (%1/%2)...").arg(m_retryCount).arg(m_maxRetries));

  switch (m_state) {
    case State::SentZRQINIT:
      sendZRQINIT();
      break;

    case State::SentZFILE:
      sendZFILE();
      break;

    case State::SentZEOF:
      sendZEOF();
      break;

    case State::SentZFIN:
      sendZFIN();
      break;

    default:
      m_timeoutTimer.start(m_timeoutMs);
      break;
  }
}
