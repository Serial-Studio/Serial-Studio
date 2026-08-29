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

#include <QCommandLineParser>

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace Misc {

struct CliOptions;

namespace CliBusConfig {

/**
 * @brief Translates the command-line bus options into driver configuration on @p cm and connects.
 *        The transport options (UART, TCP, UDP, WebSocket, HTTP) are handled here; the Pro
 *        industrial buses are delegated to CliIndustrialConfig, which keeps the Modbus, CAN,
 *        OPC UA, S7, EtherNet/IP and IEC 60870-5-104 headers out of this translation unit.
 *
 * The unit is singleton-free by construction: the caller resolves the connection manager, which is
 * what proves the whole tree runs after the composition root rather than lazily constructing it.
 */
void apply(IO::ConnectionManager& cm, const QCommandLineParser& parser, const CliOptions& opts);

}  // namespace CliBusConfig
}  // namespace Misc
