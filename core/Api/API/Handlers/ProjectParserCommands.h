/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QJsonObject>
#include <QString>

#include "API/CommandRegistry.h"

namespace API {
namespace Handlers {

/**
 * @brief Frame parser command surface: script code, language, Built-In templates, and the
 *        per-source frame-extraction configuration.
 */
class ProjectParserCommands {
public:
  explicit ProjectParserCommands(CommandRegistry& registry);

  void registerCommands();

private:
  void registerCodeCommands();
  void registerConfigCommands();
  void registerTemplateCommands();

  [[nodiscard]] static CommandResponse parserSetCode(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse parserGetCode(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse parserSetLanguage(const QString& id,
                                                         const QJsonObject& params);
  [[nodiscard]] static CommandResponse parserGetLanguage(const QString& id,
                                                         const QJsonObject& params);
  [[nodiscard]] static CommandResponse parserGetTemplate(const QString& id,
                                                         const QJsonObject& params);
  [[nodiscard]] static CommandResponse parserSetTemplate(const QString& id,
                                                         const QJsonObject& params);
  [[nodiscard]] static CommandResponse parserListTemplates(const QString& id,
                                                           const QJsonObject& params);
  [[nodiscard]] static CommandResponse frameParserConfigure(const QString& id,
                                                            const QJsonObject& params);
  [[nodiscard]] static CommandResponse frameParserGetConfig(const QString& id,
                                                            const QJsonObject& params);
  [[nodiscard]] static CommandResponse parserGetTemplateSchema(const QString& id,
                                                               const QJsonObject& params);

  CommandRegistry& m_registry;
};

}  // namespace Handlers
}  // namespace API
