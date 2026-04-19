/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * @class WorkspacesHandler
 * @brief Registers API commands for dashboard workspace management.
 *
 * Commands:
 *  - project.workspaces.list                List all workspaces (ids, titles, widget counts)
 *  - project.workspaces.get                 Return widget refs for a specific workspace
 *  - project.workspaces.add                 Create a new workspace
 *  - project.workspaces.delete              Delete a workspace
 *  - project.workspaces.rename              Rename a workspace
 *  - project.workspaces.autoGenerate        Materialise auto workspaces into customized set
 *  - project.workspaces.customize.get       Read the customizeWorkspaces flag
 *  - project.workspaces.customize.set       Flip the customizeWorkspaces flag
 *  - project.workspaces.widgets.add         Attach a widget ref to a workspace
 *  - project.workspaces.widgets.remove      Detach a widget ref from a workspace
 */
class WorkspacesHandler {
public:
  static void registerCommands();

private:
  static CommandResponse list(const QString& id, const QJsonObject& params);
  static CommandResponse get(const QString& id, const QJsonObject& params);
  static CommandResponse add(const QString& id, const QJsonObject& params);
  static CommandResponse remove(const QString& id, const QJsonObject& params);
  static CommandResponse rename(const QString& id, const QJsonObject& params);
  static CommandResponse autoGenerate(const QString& id, const QJsonObject& params);

  static CommandResponse customizeGet(const QString& id, const QJsonObject& params);
  static CommandResponse customizeSet(const QString& id, const QJsonObject& params);

  static CommandResponse widgetAdd(const QString& id, const QJsonObject& params);
  static CommandResponse widgetRemove(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
