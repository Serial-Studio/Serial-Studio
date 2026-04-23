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

#include <QByteArray>
#include <QObject>
#include <QString>

namespace IO {
namespace Protocols {

/**
 * @brief Abstract base class for file transfer protocols.
 *
 * Each protocol implementation manages its own state machine, block
 * framing, error detection, and retransmission logic. The FileTransmission
 * controller feeds incoming device bytes via processInput() and the protocol
 * emits writeRequested() when it needs to send data to the device.
 */
class Protocol : public QObject {
  Q_OBJECT

signals:
  void finished(bool success, const QString& errorMessage);
  void progressChanged(qint64 bytesSent, qint64 bytesTotal);
  void statusMessage(const QString& message);
  void writeRequested(const QByteArray& data);

public:
  explicit Protocol(QObject* parent = nullptr) : QObject(parent) {}

  virtual ~Protocol() = default;

  /**
   * @brief Human-readable name for display in UI.
   */
  [[nodiscard]] virtual QString protocolName() const = 0;

  /**
   * @brief Start sending the file at @p filePath.
   */
  virtual void startTransfer(const QString& filePath) = 0;

  /**
   * @brief Cancel an in-progress transfer.
   */
  virtual void cancelTransfer() = 0;

  /**
   * @brief Feed bytes received from the remote device into the protocol
   *        state machine.
   */
  virtual void processInput(const QByteArray& data) = 0;

  /**
   * @brief Whether the protocol is currently running a transfer.
   */
  [[nodiscard]] virtual bool isActive() const = 0;
};

}  // namespace Protocols
}  // namespace IO
