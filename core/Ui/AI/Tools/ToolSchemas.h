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
#include <QVector>

namespace AI::ToolDetail {

/**
 * @brief Tool catalog entry advertised to assistant providers.
 */
struct AssistantToolDef {
  QString name;
  QString description;
  QJsonObject inputSchema;
};

[[nodiscard]] bool isFsTool(const QString& name);
[[nodiscard]] bool isAssistantTool(const QString& name);
[[nodiscard]] const QVector<AssistantToolDef>& fsToolDefs();
[[nodiscard]] const QVector<AssistantToolDef>& assistantToolDefs();

}  // namespace AI::ToolDetail
