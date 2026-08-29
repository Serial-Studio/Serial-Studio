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
#include <QStringList>

namespace AI::MetaToolCatalog {

[[nodiscard]] QJsonObject stringProp(const QString& description, const QJsonArray& enumValues = {});

[[nodiscard]] QJsonObject objectSchemaWithProperty(const QString& key,
                                                   const QJsonObject& propSchema,
                                                   bool required);

[[nodiscard]] QJsonObject makeMetaTool(const QString& name,
                                       const QString& description,
                                       const QJsonObject& schema);

[[nodiscard]] QJsonArray metaTools(const QStringList& howToTasks, const QStringList& skillIds);

[[nodiscard]] QStringList essentialToolNames(bool smallToolSurface, bool memoryEnabled);

[[nodiscard]] QJsonObject remapDispatcherTool(const QJsonObject& raw);

}  // namespace AI::MetaToolCatalog
