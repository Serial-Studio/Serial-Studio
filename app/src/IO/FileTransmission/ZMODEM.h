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

#include "IO/FileTransmission/Protocol.h"

namespace IO {
namespace Protocols {

/**
 * @brief ZMODEM file transfer protocol implementation (sender side).
 *
 * ZMODEM is a streaming protocol with the following key features:
 * - Full-duplex streaming (no stop-and-wait per block)
 * - 32-bit CRC error detection
 * - ZDLE byte-stuffing for transparent data transfer
 * - Auto-start capability via ZRQINIT
 * - File metadata in ZFILE header (name, size, modification time)
 * - Crash recovery via file offset negotiation
 *
 * Simplified sender state machine:
 *   ZRQINIT → (wait ZRINIT) → ZFILE → (wait ZRPOS) → ZDATA + subpackets
 *   → ZEOF → (wait ZRINIT) → ZFIN → (wait ZFIN) → OO → Done
 */
class ZMODEM : public Protocol {
  Q_OBJECT

public:
  explicit ZMODEM(QObject* parent = nullptr);

  [[nodiscard]] QString protocolName() const override;
  [[nodiscard]] bool isActive() const override;

  void startTransfer(const QString& filePath) override;
  void cancelTransfer() override;
  void processInput(const QByteArray& data) override;

  [[nodiscard]] int blockSize() const noexcept;
  void setBlockSize(int size);

  [[nodiscard]] int timeoutMs() const noexcept;
  void setTimeoutMs(int ms);

  [[nodiscard]] int maxRetries() const noexcept;
  void setMaxRetries(int retries);

private:
  // ZMODEM frame types
  static constexpr quint8 kZPAD   = '*';
  static constexpr quint8 kZDLE   = 0x18;
  static constexpr quint8 kZBIN   = 'A';
  static constexpr quint8 kZBIN32 = 'C';
  static constexpr quint8 kZHEX   = 'B';

  // ZMODEM header types
  static constexpr quint8 kZRQINIT    = 0;
  static constexpr quint8 kZRINIT     = 1;
  static constexpr quint8 kZSINIT     = 2;
  static constexpr quint8 kZACK       = 3;
  static constexpr quint8 kZFILE      = 4;
  static constexpr quint8 kZSKIP      = 5;
  static constexpr quint8 kZNAK       = 6;
  static constexpr quint8 kZABORT     = 7;
  static constexpr quint8 kZFIN       = 8;
  static constexpr quint8 kZRPOS      = 9;
  static constexpr quint8 kZDATA      = 10;
  static constexpr quint8 kZEOF       = 11;
  static constexpr quint8 kZFERR      = 12;
  static constexpr quint8 kZCRC       = 13;
  static constexpr quint8 kZCHALLENGE = 14;
  static constexpr quint8 kZCOMPL     = 15;
  static constexpr quint8 kZCAN       = 16;

  // Subpacket frame end types
  static constexpr quint8 kZCRCE = 'h';
  static constexpr quint8 kZCRCG = 'i';
  static constexpr quint8 kZCRCQ = 'j';
  static constexpr quint8 kZCRCW = 'k';

  // XON/XOFF flow control bytes
  static constexpr quint8 kXON  = 0x11;
  static constexpr quint8 kXOFF = 0x13;

  enum class State {
    Idle,
    SentZRQINIT,
    SentZFILE,
    SendingData,
    WaitingForZRPOS,
    SentZEOF,
    SentZFIN,
    Done
  };

  // Header parsing states
  enum class ParseState {
    Idle,
    GotZPAD,
    GotSecondZPAD,
    GotZDLE,
    ReadingHexHeader,
    ReadingBin32Header,
    ReadingBinHeader
  };

  static constexpr int kChunksPerYield = 64;

  // Transmit helpers
  void sendZRQINIT();
  void sendZFILE();
  void sendDataSubpackets();
  void sendNextDataChunk();
  void sendZEOF();
  void sendZFIN();
  void sendCancel();

  // Header building
  [[nodiscard]] QByteArray buildHexHeader(quint8 type, quint32 arg);
  [[nodiscard]] QByteArray buildBin32Header(quint8 type, quint32 arg);
  [[nodiscard]] QByteArray buildSubpacket(const QByteArray& data, quint8 frameEnd);

  // ZDLE encoding
  [[nodiscard]] QByteArray zdleEncode(const QByteArray& data);
  [[nodiscard]] static bool needsEscape(quint8 ch);

  // Header parsing
  void parseReceivedHeader(quint8 type, quint32 arg);
  void processHexByte(quint8 ch);
  void processBin32Byte(quint8 ch);
  void processBinByte(quint8 ch);

  // Hex conversion
  [[nodiscard]] static QByteArray toHex(quint8 byte);

  // Timeout
  void handleTimeout();

  State m_state;
  ParseState m_parseState;
  QFile m_file;
  QTimer m_timeoutTimer;
  QString m_filePath;
  qint64 m_fileSize;
  qint64 m_bytesSent;
  qint64 m_fileOffset;
  int m_blockSize;
  int m_timeoutMs;
  int m_maxRetries;
  int m_retryCount;

  // Header parsing buffer
  QByteArray m_headerBuf;
  int m_headerBytesExpected;
  bool m_zdleEscape;
};

}  // namespace Protocols
}  // namespace IO
