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

namespace CliIndustrialConfig {

/**
 * @brief Translates the Pro industrial-bus command-line options (Modbus, CAN, OPC UA, S7,
 *        EtherNet/IP, IEC 60870-5-104) into driver configuration on @p cm and connects. Returns
 *        true when one of those options claimed the session, so the caller stops dispatching.
 *
 * The unit is deliberately singleton-free: every driver is reached through the connection manager
 * the caller resolved, which is what keeps the Pro driver headers out of the GPL translation units
 * and keeps this file callable only after the composition root exists. In a GPL build the entry
 * point is a stub that always returns false.
 */
[[nodiscard]] bool apply(IO::ConnectionManager& cm,
                         const QCommandLineParser& parser,
                         const CliOptions& opts);

}  // namespace CliIndustrialConfig
}  // namespace Misc
