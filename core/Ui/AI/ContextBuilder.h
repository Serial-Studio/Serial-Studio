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
#include <QString>
#include <QStringList>

namespace AI {

/**
 * @brief Builds the system-prompt blocks fed to the LLM each turn.
 */
class ContextBuilder {
public:
  [[nodiscard]] static QString roleBlock();
  [[nodiscard]] static QString scriptingDocsBlock();
  [[nodiscard]] static QString liveProjectStateBlock();
  [[nodiscard]] static QString memoryIndexBlock();
  [[nodiscard]] static QString handoffBlock();
  [[nodiscard]] static QString scriptingDocFor(const QString& kind);

  [[nodiscard]] static QStringList howToTasks();
  [[nodiscard]] static QString howToRecipe(const QString& task);

  [[nodiscard]] static QStringList skillIds();
  [[nodiscard]] static QString skillBody(const QString& id);

  [[nodiscard]] static QJsonArray buildSystemArray(bool includeScriptingDocs = true);
};

}  // namespace AI
