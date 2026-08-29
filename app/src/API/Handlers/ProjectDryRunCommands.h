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
 * @brief Compile-and-run previews that never touch live project state: frame parser, dataset
 *        transform, painter, output widget, and the end-to-end pipeline.
 */
class ProjectDryRunCommands {
public:
  explicit ProjectDryRunCommands(CommandRegistry& registry);

  void registerCommands();

private:
  void registerScriptDryRunCommands();
  void registerEndToEndDryRunCommand();
  void registerFrameParserDryRunCommands();

  [[nodiscard]] static CommandResponse painterDryRun(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse endToEndDryRun(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse transformDryRun(const QString& id,
                                                       const QJsonObject& params);
  [[nodiscard]] static CommandResponse frameParserDryRun(const QString& id,
                                                         const QJsonObject& params);
  [[nodiscard]] static CommandResponse outputWidgetDryRun(const QString& id,
                                                          const QJsonObject& params);
  [[nodiscard]] static CommandResponse frameParserDryCompile(const QString& id,
                                                             const QJsonObject& params);

  CommandRegistry& m_registry;
};

}  // namespace Handlers
}  // namespace API
