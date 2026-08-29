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
 * @brief The PATCH surface: project.group/dataset/action/outputWidget.update.
 */
class ProjectUpdateCommands {
public:
  explicit ProjectUpdateCommands(CommandRegistry& registry);

  void registerCommands();

private:
  [[nodiscard]] static CommandResponse groupUpdate(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse actionUpdate(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse datasetUpdate(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse outputWidgetUpdate(const QString& id,
                                                          const QJsonObject& params);

  CommandRegistry& m_registry;
};

}  // namespace Handlers
}  // namespace API
