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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

namespace IO {
namespace Protocols {

/**
 * @brief Abstract base class for file transfer protocols. protocolError() is emitted once per
 *        recoverable protocol error (a NAK, a retry, a timeout); the controller counts those
 *        emissions instead of matching English words in the status text, which counted nothing
 *        at all in any other language.
 */
class Protocol : public QObject {
  Q_OBJECT

signals:
  void protocolError();
  void finished(bool success, const QString& errorMessage);
  void progressChanged(qint64 bytesSent, qint64 bytesTotal);
  void statusMessage(const QString& message);
  void writeRequested(const QByteArray& data);

public:
  explicit Protocol(QObject* parent = nullptr) : QObject(parent) {}

  virtual ~Protocol() = default;

  [[nodiscard]] virtual QString protocolName() const  = 0;
  virtual void startTransfer(const QString& filePath) = 0;
  virtual void cancelTransfer()                       = 0;
  virtual void processInput(const QByteArray& data)   = 0;
  [[nodiscard]] virtual bool isActive() const         = 0;
};

}  // namespace Protocols
}  // namespace IO
