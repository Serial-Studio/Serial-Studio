/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Logging.h"

namespace AI {
// Default QtInfoMsg: qCDebug silent unless QT_LOGGING_RULES opts in
Q_LOGGING_CATEGORY(serialStudioAI, "serialstudio.ai", QtInfoMsg)
}  // namespace AI
