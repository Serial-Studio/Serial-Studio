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
 * @param filePath Path to the file to transmit.
 */
void IO::Protocols::ZMODEM::startTransfer(const QString& filePath)
{
  // Abort any existing transfer and open the file
  if (isActive())
    cancelTransfer();

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

  Q_EMIT progressChanged(0, m_fileSize);

  // Send ZRQINIT to initiate the session
  sendZRQINIT();
}

/**
 * @brief Cancels the current transfer.
 */
void IO::Protocols::ZMODEM::cancelTransfer()
{
  // Ignore if no transfer is in progress
  if (!isActive())
    return;

  // Send cancel sequence and reset state
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
 *
 * Parses ZMODEM headers from the incoming byte stream. ZMODEM headers
 * begin with ZPAD ZPAD ZDLE followed by a header type byte:
 *   'B' = hex header (human-readable)
 *   'A' = binary header with CRC-16
 *   'C' = binary header with CRC-32
 *
 * @param data Raw bytes from the device.
 */
void IO::Protocols::ZMODEM::processInput(const QByteArray& data)
{
  // Parse each byte through the ZMODEM header state machine
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
 * @param ch The byte to process.
 */
void IO::Protocols::ZMODEM::processHexByte(quint8 ch)
{
  // Hex headers are terminated by CR/LF
  if (ch != '\r' && ch != '\n') {
    m_headerBuf.append(static_cast<char>(ch));
    return;
  }

  // Need at least 14 hex chars: type(2) + arg(8) + crc(4)
  if (m_headerBuf.size() < 14) {
    m_parseState = ParseState::Idle;
    return;
  }

  // Parse type and arg (little-endian)
  quint8 type          = QByteArray::fromHex(m_headerBuf.mid(0, 2)).at(0);
  QByteArray arg_bytes = QByteArray::fromHex(m_headerBuf.mid(2, 8));
  quint32 arg          = 0;
  if (arg_bytes.size() == 4) {
    arg = static_cast<quint32>(static_cast<quint8>(arg_bytes[0]))
        | (static_cast<quint32>(static_cast<quint8>(arg_bytes[1])) << 8)
        | (static_cast<quint32>(static_cast<quint8>(arg_bytes[2])) << 16)
        | (static_cast<quint32>(static_cast<quint8>(arg_bytes[3])) << 24);
  }

  parseReceivedHeader(type, arg);
  m_parseState = ParseState::Idle;
}

/**
 * @brief Processes one byte of an incoming binary-32 header (ZDLE-decoded).
 * @param ch The byte to process.
 */
void IO::Protocols::ZMODEM::processBin32Byte(quint8 ch)
{
  // Handle ZDLE escaping
  if (m_zdleEscape) {
    m_headerBuf.append(static_cast<char>(ch ^ 0x40));
    m_zdleEscape = false;
  } else if (ch == kZDLE) {
    m_zdleEscape = true;
    return;
  } else {
    m_headerBuf.append(static_cast<char>(ch));
  }

  // Wait until we have all expected bytes
  if (m_headerBuf.size() < m_headerBytesExpected)
    return;

  // Extract type and arg (little-endian)
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
 * @param ch The byte to process.
 */
void IO::Protocols::ZMODEM::processBinByte(quint8 ch)
{
  // Handle ZDLE escaping
  if (m_zdleEscape) {
    m_headerBuf.append(static_cast<char>(ch ^ 0x40));
    m_zdleEscape = false;
  } else if (ch == kZDLE) {
    m_zdleEscape = true;
    return;
  } else {
    m_headerBuf.append(static_cast<char>(ch));
  }

  // Wait until we have all expected bytes
  if (m_headerBuf.size() < m_headerBytesExpected)
    return;

  // Extract type and arg (little-endian)
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
  // Transition to ZRQINIT state
  m_state = State::SentZRQINIT;

  // Standard auto-start sequence: rz\r followed by ZRQINIT header
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
  // Transition to ZFILE state
  m_state = State::SentZFILE;

  // Send ZFILE binary header (flags = 0 for default)
  Q_EMIT writeRequested(buildBin32Header(kZFILE, 0));

  // Build file info subpacket: "filename\0size 0 0 0\0"
  QFileInfo info(m_filePath);
  QByteArray fileInfo;
  fileInfo.append(info.fileName().toUtf8());
  fileInfo.append('\0');
  fileInfo.append(QByteArray::number(m_fileSize));
  fileInfo.append(' ');
  fileInfo.append(QByteArray::number(info.lastModified().toSecsSinceEpoch()));
  fileInfo.append(" 0 0 0 0");
  fileInfo.append('\0');

  // Send as ZCRCW subpacket (CRC next, wait for ZACK)
  Q_EMIT writeRequested(buildSubpacket(fileInfo, kZCRCW));

  Q_EMIT statusMessage(tr("Sending file info: %1 (%2 bytes)").arg(info.fileName()).arg(m_fileSize));
  m_timeoutTimer.start(m_timeoutMs);
}

/**
 * @brief Sends file data as a stream of ZDATA subpackets.
 */
void IO::Protocols::ZMODEM::sendDataSubpackets()
{
  // Transition to data-sending state
  m_state = State::SendingData;

  // Seek to the requested offset (for crash recovery)
  m_file.seek(m_fileOffset);
  m_bytesSent = m_fileOffset;

  // Send ZDATA header with current file position
  Q_EMIT writeRequested(buildBin32Header(kZDATA, static_cast<quint32>(m_fileOffset)));

  // Stream subpackets
  while (!m_file.atEnd()) {
    QByteArray chunk = m_file.read(m_blockSize);
    if (chunk.isEmpty())
      break;

    m_bytesSent += chunk.size();
    Q_EMIT progressChanged(m_bytesSent, m_fileSize);

    // Use ZCRCG for intermediate packets, ZCRCE for the last one
    if (m_file.atEnd())
      Q_EMIT writeRequested(buildSubpacket(chunk, kZCRCE));
    else
      Q_EMIT writeRequested(buildSubpacket(chunk, kZCRCG));
  }

  // Send ZEOF with file size
  sendZEOF();
}

/**
 * @brief Sends the ZEOF header.
 */
void IO::Protocols::ZMODEM::sendZEOF()
{
  // Send ZEOF header with the file size
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
  // Send ZFIN header to close the session
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
  // Build cancel sequence: 5 CAN bytes + 5 backspaces
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
 * @param type ZMODEM header type byte.
 * @param arg 32-bit argument (interpretation depends on type).
 */
void IO::Protocols::ZMODEM::parseReceivedHeader(quint8 type, quint32 arg)
{
  // Stop the timeout and dispatch based on header type
  m_timeoutTimer.stop();

  switch (type) {
    // Receiver is ready
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

    // Receiver requests data from a specific position
    case kZRPOS:
      m_fileOffset = static_cast<qint64>(arg);
      Q_EMIT statusMessage(tr("Receiver requests data from offset %1").arg(m_fileOffset));
      sendDataSubpackets();
      break;

    // Receiver acknowledges
    case kZACK:
      break;

    // Receiver wants to skip this file
    case kZSKIP:
      Q_EMIT statusMessage(tr("Receiver skipped the file"));
      sendZFIN();
      break;

    // Receiver NAK — retry
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

      // Resend based on current state
      if (m_state == State::SentZRQINIT)
        sendZRQINIT();
      else if (m_state == State::SentZFILE)
        sendZFILE();
      else if (m_state == State::SentZEOF)
        sendDataSubpackets();
      break;

    // Session complete
    case kZFIN:
      // Send "OO" (Over and Out)
      Q_EMIT writeRequested(QByteArray("OO"));
      m_state = State::Done;

      if (m_file.isOpen())
        m_file.close();

      Q_EMIT statusMessage(tr("Transfer complete"));
      Q_EMIT finished(true, QString());
      break;

    // Receiver cancelled
    case kZABORT:
    case kZCAN:
      m_state = State::Idle;
      if (m_file.isOpen())
        m_file.close();
      Q_EMIT statusMessage(tr("Transfer cancelled by receiver"));
      Q_EMIT finished(false, tr("Receiver cancelled the transfer"));
      break;

    // Error on receiver side
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
 * @brief Builds a ZMODEM hex header.
 *
 * Format: ZPAD ZPAD ZDLE ZHEX type[2hex] arg[8hex] crc[4hex] CR LF
 *
 * @param type Header type (kZRQINIT, kZFIN, etc.).
 * @param arg 32-bit argument.
 * @return Complete hex header bytes.
 */
QByteArray IO::Protocols::ZMODEM::buildHexHeader(quint8 type, quint32 arg)
{
  // Allocate header buffer
  QByteArray header;
  header.reserve(32);

  // Preamble
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZDLE));
  header.append(static_cast<char>(kZHEX));

  // Type (2 hex chars)
  header.append(toHex(type));

  // Argument (4 bytes, little-endian, 8 hex chars)
  header.append(toHex(static_cast<quint8>(arg & 0xFF)));
  header.append(toHex(static_cast<quint8>((arg >> 8) & 0xFF)));
  header.append(toHex(static_cast<quint8>((arg >> 16) & 0xFF)));
  header.append(toHex(static_cast<quint8>((arg >> 24) & 0xFF)));

  // CRC-16 over type + 4 arg bytes
  quint8 crcData[5];
  crcData[0]  = type;
  crcData[1]  = static_cast<quint8>(arg & 0xFF);
  crcData[2]  = static_cast<quint8>((arg >> 8) & 0xFF);
  crcData[3]  = static_cast<quint8>((arg >> 16) & 0xFF);
  crcData[4]  = static_cast<quint8>((arg >> 24) & 0xFF);
  quint16 crc = CRC::crc16(crcData, 5);
  header.append(toHex(static_cast<quint8>((crc >> 8) & 0xFF)));
  header.append(toHex(static_cast<quint8>(crc & 0xFF)));

  // Terminator
  header.append('\r');
  header.append('\n');

  return header;
}

/**
 * @brief Builds a ZMODEM binary-32 header.
 *
 * Format: ZPAD ZPAD ZDLE ZBIN32 type arg[4] crc32[4] (all ZDLE-encoded)
 *
 * @param type Header type.
 * @param arg 32-bit argument.
 * @return Complete binary header bytes.
 */
QByteArray IO::Protocols::ZMODEM::buildBin32Header(quint8 type, quint32 arg)
{
  // Allocate header buffer
  QByteArray header;
  header.reserve(32);

  // Preamble (not ZDLE-encoded)
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZPAD));
  header.append(static_cast<char>(kZDLE));
  header.append(static_cast<char>(kZBIN32));

  // Payload: type + 4 arg bytes
  QByteArray payload;
  payload.append(static_cast<char>(type));
  payload.append(static_cast<char>(arg & 0xFF));
  payload.append(static_cast<char>((arg >> 8) & 0xFF));
  payload.append(static_cast<char>((arg >> 16) & 0xFF));
  payload.append(static_cast<char>((arg >> 24) & 0xFF));

  // CRC-32 over payload
  quint32 crc = CRC::crc32(reinterpret_cast<const quint8*>(payload.constData()), payload.size());
  payload.append(static_cast<char>(crc & 0xFF));
  payload.append(static_cast<char>((crc >> 8) & 0xFF));
  payload.append(static_cast<char>((crc >> 16) & 0xFF));
  payload.append(static_cast<char>((crc >> 24) & 0xFF));

  // ZDLE-encode the payload + CRC
  header.append(zdleEncode(payload));

  return header;
}

/**
 * @brief Builds a ZDLE-encoded data subpacket with CRC-32.
 *
 * @param data Raw data bytes.
 * @param frameEnd Frame-end type (kZCRCE, kZCRCG, kZCRCQ, kZCRCW).
 * @return ZDLE-encoded subpacket.
 */
QByteArray IO::Protocols::ZMODEM::buildSubpacket(const QByteArray& data, quint8 frameEnd)
{
  // Allocate packet buffer with room for ZDLE escaping
  QByteArray packet;
  packet.reserve(data.size() * 2 + 16);

  // ZDLE-encode the data
  packet.append(zdleEncode(data));

  // Append frame-end (ZDLE-escaped)
  packet.append(static_cast<char>(kZDLE));
  packet.append(static_cast<char>(frameEnd));

  // CRC-32 over data + frame-end byte
  QByteArray crcInput = data;
  crcInput.append(static_cast<char>(frameEnd));
  quint32 crc = CRC::crc32(reinterpret_cast<const quint8*>(crcInput.constData()), crcInput.size());

  // ZDLE-encode the CRC bytes
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
 * @brief ZDLE-encodes a byte sequence.
 *
 * Bytes that conflict with ZMODEM framing or XON/XOFF flow control
 * are prefixed with ZDLE and XORed with 0x40.
 *
 * @param data Raw bytes.
 * @return ZDLE-encoded bytes.
 */
QByteArray IO::Protocols::ZMODEM::zdleEncode(const QByteArray& data)
{
  // Allocate output buffer with headroom for escape sequences
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
  // Convert byte to two ASCII hex characters
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
  // Ignore timeouts when idle
  if (!isActive())
    return;

  // Abort if maximum retries exceeded
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

  // Retry based on current state
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
