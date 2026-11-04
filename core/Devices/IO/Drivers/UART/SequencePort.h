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

#include <QByteArray>
#include <QSerialPort>
#include <QString>

#include "Core/IO/ConnectionSequence.h"

namespace IO {
namespace Drivers {

/**
 * @brief The port operations a Connection Sequence drives (spec 0103): the seam between the step
 *        runner and the open QSerialPort, so the runner is tested against a scripted fake. Every
 *        call acts on the open port at once and never blocks; a false return is a refusal whose
 *        cause errorString() names.
 */
class ISequencePort {
public:
  ISequencePort()                                = default;
  ISequencePort(ISequencePort&&)                 = delete;
  ISequencePort(const ISequencePort&)            = delete;
  ISequencePort& operator=(ISequencePort&&)      = delete;
  ISequencePort& operator=(const ISequencePort&) = delete;
  virtual ~ISequencePort()                       = default;

  [[nodiscard]] virtual bool clearInput()                                    = 0;
  [[nodiscard]] virtual bool setBaud(qint32 rate)                            = 0;
  [[nodiscard]] virtual bool setBreak(bool enabled)                          = 0;
  [[nodiscard]] virtual QString errorString() const                          = 0;
  [[nodiscard]] virtual qint64 write(const QByteArray& data)                 = 0;
  [[nodiscard]] virtual QSerialPort::PinoutSignals lineStates() const        = 0;
  [[nodiscard]] virtual bool setFlowControl(QSerialPort::FlowControl mode)   = 0;
  [[nodiscard]] virtual bool setLine(IO::SequenceStep::Line line, bool high) = 0;
};

}  // namespace Drivers
}  // namespace IO
