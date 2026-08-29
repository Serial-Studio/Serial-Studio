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

[[nodiscard]] QJsonObject assistantToolDescription(const QString& name);
[[nodiscard]] QJsonObject makeRepairHint(const QString& name, const QString& message);
[[nodiscard]] QJsonObject runCommand(const QString& name, const QJsonObject& args = {});
[[nodiscard]] QJsonObject attachRepairHint(QJsonObject reply, const QString& commandName);
[[nodiscard]] QJsonObject executeAssistantTool(const QString& name, const QJsonObject& args);

}  // namespace AI::ToolDetail
