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

namespace Misc::ScriptCheckers {

/**
 * @brief Registers the script-error checkers with the problem center: per-source parser failures,
 *        an engine disabled by the watchdog, and per-dataset transform failures, each carrying the
 *        retained error text. Called once from ProblemCenter::setupExternalConnections().
 */
void registerAll();

}  // namespace Misc::ScriptCheckers
