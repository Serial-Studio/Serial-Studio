/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QList>
#include <QString>

class QJsonObject;

namespace DataModel {

struct ScriptTemplateDefinition {
  QString file;
  QString name;
  bool isDefault = false;
};

[[nodiscard]] QString readTextResource(const QString& path);

[[nodiscard]] QString localizedTemplateName(const QJsonObject& object,
                                            const char* translationContext = nullptr);

[[nodiscard]] QList<ScriptTemplateDefinition> loadScriptTemplateManifest(
  const QString& manifestPath, const char* translationContext = nullptr);

[[nodiscard]] QString templateResourcePath(const QString& directory,
                                           const QString& file,
                                           const QString& suffix);

}  // namespace DataModel
