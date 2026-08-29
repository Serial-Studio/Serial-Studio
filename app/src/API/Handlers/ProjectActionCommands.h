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
 * @brief Outgoing-action CRUD command surface (project.action.add / delete / duplicate).
 */
class ProjectActionCommands {
public:
  explicit ProjectActionCommands(CommandRegistry& registry);

  void registerCommands();

private:
  [[nodiscard]] static CommandResponse actionAdd(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse actionDelete(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse actionDuplicate(const QString& id,
                                                       const QJsonObject& params);

  CommandRegistry& m_registry;
};

}  // namespace Handlers
}  // namespace API
