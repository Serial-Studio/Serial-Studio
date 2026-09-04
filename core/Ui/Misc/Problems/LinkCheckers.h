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

namespace Misc::LinkCheckers {

/**
 * @brief Registers the link-statistics checker with the problem center: bytes without frames,
 *        frames without parsed values, checksum failures, frame-queue drops and buffer overruns,
 *        all derived from deltas of the 1 Hz counter sample. Called once from
 *        ProblemCenter::setupExternalConnections().
 */
void registerAll();

}  // namespace Misc::LinkCheckers
