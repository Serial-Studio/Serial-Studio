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
 * @brief Enumeration, dataset resolvers and reorder commands. Registration is split in two so
 *        the facade can keep the historical order, where project.snapshot lands between the
 *        resolvers and the move commands.
 */
class ProjectListCommands {
public:
  explicit ProjectListCommands(CommandRegistry& registry);

  void registerMoveCommands();
  void registerListCommands();

private:
  void registerResolverCommands();

  [[nodiscard]] static CommandResponse groupMove(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse groupsList(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse actionsList(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse datasetMove(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse datasetsList(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse datasetGetByPath(const QString& id,
                                                        const QJsonObject& params);
  [[nodiscard]] static CommandResponse datasetGetByTitle(const QString& id,
                                                         const QJsonObject& params);
  [[nodiscard]] static CommandResponse datasetGetByUniqueId(const QString& id,
                                                            const QJsonObject& params);
  [[nodiscard]] static CommandResponse datasetGetExecutionOrder(const QString& id,
                                                                const QJsonObject& params);

  CommandRegistry& m_registry;
};

}  // namespace Handlers
}  // namespace API
