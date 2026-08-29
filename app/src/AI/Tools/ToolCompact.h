/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonObject>
#include <QString>

namespace AI::ToolDetail {

[[nodiscard]] QString optionSlugForWidget(const QString& widgetType);
[[nodiscard]] QJsonObject compactProjectSnapshotResult(const QJsonObject& projectResult,
                                                       const QJsonObject& workspaceResult,
                                                       bool includeRaw);

}  // namespace AI::ToolDetail
