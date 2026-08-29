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

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace AI::ToolDetail {

[[nodiscard]] QJsonObject listCategories();
[[nodiscard]] QString canonicalToolName(const QString& name);
[[nodiscard]] QJsonArray availableTools(const QString& category);
[[nodiscard]] QJsonObject describeCommand(const QString& requestedName);
[[nodiscard]] QJsonObject searchCommands(const QString& query, int offset, int limit);
[[nodiscard]] QJsonObject listCommands(const QString& prefix,
                                       int offset,
                                       int limit,
                                       bool namesOnly);

}  // namespace AI::ToolDetail
