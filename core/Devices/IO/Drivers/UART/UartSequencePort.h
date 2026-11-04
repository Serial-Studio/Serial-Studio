/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <cstddef>
#include <QCoreApplication>
#include <QPointer>
#include <QSerialPort>

#include "IO/Drivers/UART/SequencePort.h"

namespace IO {
namespace Drivers {

/**
 * @brief The UART driver's ISequencePort (spec 0103): acts on the driver's open QSerialPort for
 *        the running state only, so every reopen starts from the saved settings again. Its static
 *        helpers size the port's buffers for a rate, at open() and on every live baud change.
 */
class UartSequencePort final : public ISequencePort {
  Q_DECLARE_TR_FUNCTIONS(IO::Drivers::UART)

public:
  explicit UartSequencePort(QSerialPort* port);

  [[nodiscard]] bool clearInput() override;
  [[nodiscard]] bool setBaud(qint32 rate) override;
  [[nodiscard]] bool setBreak(bool enabled) override;
  [[nodiscard]] QString errorString() const override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] QSerialPort::PinoutSignals lineStates() const override;
  [[nodiscard]] bool setFlowControl(QSerialPort::FlowControl mode) override;
  [[nodiscard]] bool setLine(IO::SequenceStep::Line line, bool high) override;

  [[nodiscard]] static std::size_t readBufferSize(qint32 baud);
  [[nodiscard]] static bool applyBaud(QSerialPort& port, qint32 rate);
  static void tuneNativeBuffer(QSerialPort* port, qint32 baud);

private:
  [[nodiscard]] bool portReady();
  [[nodiscard]] bool refuse(const QString& reason);

private:
  QString m_error;
  QPointer<QSerialPort> m_port;
};

}  // namespace Drivers
}  // namespace IO
