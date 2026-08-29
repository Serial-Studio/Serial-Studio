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

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>

namespace Misc {
namespace CliSpecParsers {

/**
 * @brief Pure text-to-value translation for the command-line bus options: no driver, no singleton
 *        and no application state is reachable from here, which is what makes the validation rules
 *        testable on their own. Every entry point either assigns its out-parameters and returns
 *        true, or warns on stderr and leaves them untouched, so a caller only ever writes a value
 *        that actually validated.
 */

[[nodiscard]] bool parseIntOption(const QCommandLineParser& parser,
                                  const QCommandLineOption& opt,
                                  const int lo,
                                  const int hi,
                                  const QString& label,
                                  int& out);

[[nodiscard]] bool parseModbusTcpAddress(const QString& tcpAddress, QString& host, quint16& port);

[[nodiscard]] bool parseModbusRegisterSpec(const QString& spec,
                                           quint8& type,
                                           quint16& start,
                                           quint16& count);

[[nodiscard]] int modbusParityIndex(const QString& parity);
[[nodiscard]] int modbusDataBitsIndex(const QString& dataBits);
[[nodiscard]] int modbusStopBitsIndex(const QString& stopBits);

}  // namespace CliSpecParsers
}  // namespace Misc
