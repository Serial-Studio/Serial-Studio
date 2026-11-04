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

#include "Core/IO/FrameConfig.h"
#include "Core/SerialStudio.h"

namespace Core::Bus {
struct ProjectStructureSnapshot;
}  // namespace Core::Bus

namespace IO::FrameConfigBuilder {

/**
 * @brief Repairs a frame-detection mode whose settings the source does not carry (delimiters,
 *        or a usable Fixed Length), so a reader never waits for a frame that cannot arrive.
 */
[[nodiscard]] SerialStudio::FrameDetection repairedDetection(const FrameConfig& cfg);

[[nodiscard]] FrameConfig build(const Core::Bus::ProjectStructureSnapshot& project,
                                SerialStudio::OperationMode mode,
                                const FrameConfig& source0Config,
                                int deviceId);

}  // namespace IO::FrameConfigBuilder
