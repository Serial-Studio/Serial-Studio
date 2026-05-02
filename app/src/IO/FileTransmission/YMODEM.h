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

#include "IO/FileTransmission/XMODEM.h"

namespace IO {
namespace Protocols {

/**
 * @brief YMODEM (batch) sender built on top of XMODEM-1K.
 */
class YMODEM : public XMODEM {
  Q_OBJECT

public:
  explicit YMODEM(QObject* parent = nullptr);

  [[nodiscard]] QString protocolName() const override;

  void startTransfer(const QString& filePath) override;
  void processInput(const QByteArray& data) override;

private:
  enum class YState {
    Idle,
    WaitingForInitialC,
    WaitingForBlock0Ack,
    WaitingForDataC,
    SendingData,
    WaitingForDataAck,
    WaitingForFirstEOTResponse,
    WaitingForSecondEOTAck,
    WaitingForEndBatchC,
    WaitingForEndBatchAck,
    Done
  };

  void sendBlock0();
  void sendEndOfBatch();
  void sendDataBlock();
  bool handleDataAckByte(quint8 ch);
  void handleBlock0AckByte(quint8 ch);
  void handleSecondEotAckByte(quint8 ch);

  YState m_yState;
  QString m_filePath;
};

}  // namespace Protocols
}  // namespace IO
