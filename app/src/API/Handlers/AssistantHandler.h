/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "API/CommandProtocol.h"

namespace API {
namespace Handlers {

/**
 * @brief Token-efficient convenience wrappers over project.* low-level commands.
 */
class AssistantHandler {
public:
  static void registerCommands();

private:
  static void registerResolverCommands();
  static void registerEditCommands();
  static void registerCheckpointCommands();

  static CommandResponse snapshot(const QString& id, const QJsonObject& params);
  static CommandResponse datasetResolve(const QString& id, const QJsonObject& params);
  static CommandResponse workspaceResolve(const QString& id, const QJsonObject& params);
  static CommandResponse workspacePlan(const QString& id, const QJsonObject& params);
  static CommandResponse scriptDryRun(const QString& id, const QJsonObject& params);
  static CommandResponse scriptApply(const QString& id, const QJsonObject& params);
  static CommandResponse workspaceAddTile(const QString& id, const QJsonObject& params);
  static CommandResponse projectBulkApply(const QString& id, const QJsonObject& params);

  static CommandResponse checkpoint(const QString& id, const QJsonObject& params);
  static CommandResponse restore(const QString& id, const QJsonObject& params);
  static CommandResponse listCheckpoints(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
