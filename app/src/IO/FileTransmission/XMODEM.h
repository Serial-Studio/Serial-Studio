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
 * @brief XMODEM / XMODEM-1K sender with CRC-16.
 */
class XMODEM : public Protocol {
  Q_OBJECT

public:
  explicit XMODEM(QObject* parent = nullptr);

  [[nodiscard]] QString protocolName() const override;
  [[nodiscard]] bool isActive() const override;

  void startTransfer(const QString& filePath) override;
  void cancelTransfer() override;
  void processInput(const QByteArray& data) override;

  [[nodiscard]] bool use1K() const noexcept;
  void setUse1K(bool enabled);

  [[nodiscard]] int maxRetries() const noexcept;
  void setMaxRetries(int retries);

  [[nodiscard]] int timeoutMs() const noexcept;
  void setTimeoutMs(int ms);

protected:
  static constexpr quint8 kSOH = 0x01;
  static constexpr quint8 kSTX = 0x02;
  static constexpr quint8 kEOT = 0x04;
  static constexpr quint8 kACK = 0x06;
  static constexpr quint8 kNAK = 0x15;
  static constexpr quint8 kCAN = 0x18;
  static constexpr quint8 kCRC = 'C';

  enum class State {
    Idle,
    WaitingForStart,
    SendingBlocks,
    WaitingForAck,
    SendingEOT,
    WaitingForEOTAck,
    Done
  };

  virtual void sendBlock();
  virtual void sendEOT();

  void resetState();
  void handleTimeout();
  bool handleAckByte(quint8 ch);
  void handleEotAckByte(quint8 ch);
  [[nodiscard]] QByteArray buildBlock(const QByteArray& data, quint8 blockNum);

  State m_state;
  QFile m_file;
  QTimer m_timeoutTimer;
  quint8 m_blockNumber;
  int m_retryCount;
  int m_maxRetries;
  int m_timeoutMs;
  bool m_use1K;
  qint64 m_bytesSent;
  qint64 m_fileSize;
  qint64 m_lastBlockStart;
  qint64 m_lastBlockBytes;

private:
  void sendCancel();
};

}  // namespace Protocols
}  // namespace IO
