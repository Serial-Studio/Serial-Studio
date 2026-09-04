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

[[nodiscard]] bool innerOpAllowed(const QString& commandName);
[[nodiscard]] QJsonObject executeBulkApply(const QJsonObject& args);
[[nodiscard]] QJsonObject makeInnerOpRejection(const QString& commandName);

}  // namespace AI::ToolDetail
