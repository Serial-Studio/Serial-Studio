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

#include <QTest>

#include "IO/Drivers/UART/UartPolicy.h"

using namespace IO::Drivers::UartPolicy;

/**
 * @brief The two error-handling decisions of the UART driver, taken apart from any real port.
 */
class TstUartPolicy : public QObject {
  Q_OBJECT

private slots:
  void noErrorIsNeverFatal();
  void ordinaryErrorsAreFatal();
  void customPathIgnoresUnsupportedOperations();
  void customPathHonoursResourceErrors();
  void autoReconnectOwnsResourceErrorsOnly();
};

/**
 * @brief NoError is the idle state the handler is called with on every recovery, not a drop.
 */
void TstUartPolicy::noErrorIsNeverFatal()
{
  QVERIFY(!isFatalPortError(QSerialPort::NoError, false));
  QVERIFY(!isFatalPortError(QSerialPort::NoError, true));
}

/**
 * @brief An enumerated port reports every error as fatal: the link is gone either way.
 */
void TstUartPolicy::ordinaryErrorsAreFatal()
{
  QVERIFY(isFatalPortError(QSerialPort::ResourceError, false));
  QVERIFY(isFatalPortError(QSerialPort::PermissionError, false));
  QVERIFY(isFatalPortError(QSerialPort::UnsupportedOperationError, false));
}

/**
 * @brief A custom device path is exempt from UnsupportedOperationError alone: by-id nodes and
 *        socat ptys reject ioctls the driver never needs.
 */
void TstUartPolicy::customPathIgnoresUnsupportedOperations()
{
  QVERIFY(!isFatalPortError(QSerialPort::UnsupportedOperationError, true));
}

/**
 * @brief It is NOT exempt from ResourceError. Swallowing that one left an unplugged pty "open"
 *        for the rest of the session, with write() still returning byte counts.
 */
void TstUartPolicy::customPathHonoursResourceErrors()
{
  QVERIFY(isFatalPortError(QSerialPort::ResourceError, true));
}

/**
 * @brief Only a resource loss is handed to the opt-in auto-reconnect; every other error is
 *        reported to the user instead of being retried silently.
 */
void TstUartPolicy::autoReconnectOwnsResourceErrorsOnly()
{
  QVERIFY(shouldAutoReconnect(QSerialPort::ResourceError, true));
  QVERIFY(!shouldAutoReconnect(QSerialPort::ResourceError, false));
  QVERIFY(!shouldAutoReconnect(QSerialPort::PermissionError, true));
  QVERIFY(!shouldAutoReconnect(QSerialPort::NoError, true));
}

QTEST_APPLESS_MAIN(TstUartPolicy)

#include "tst_uart_policy.moc"
