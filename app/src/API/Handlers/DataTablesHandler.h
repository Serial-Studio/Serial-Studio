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
 * @class DataTablesHandler
 * @brief Registers API commands for DataModel::ProjectModel's user-defined
 *        data tables (shared-memory tables exposed to transforms).
 *
 * Table commands:
 *  - project.tables.list                 List all user tables + register counts
 *  - project.tables.get                  Return full register list for a table
 *  - project.tables.add                  Create a new table with a unique name
 *  - project.tables.delete               Remove a table by name
 *  - project.tables.rename               Rename a table
 *
 * Register commands:
 *  - project.tables.register.add         Append a register (constant or computed)
 *  - project.tables.register.delete      Remove a register
 *  - project.tables.register.update      Modify type / default value / name
 */
class DataTablesHandler {
public:
  static void registerCommands();

private:
  static CommandResponse tablesList(const QString& id, const QJsonObject& params);
  static CommandResponse tableGet(const QString& id, const QJsonObject& params);
  static CommandResponse tableAdd(const QString& id, const QJsonObject& params);
  static CommandResponse tableDelete(const QString& id, const QJsonObject& params);
  static CommandResponse tableRename(const QString& id, const QJsonObject& params);

  static CommandResponse registerAdd(const QString& id, const QJsonObject& params);
  static CommandResponse registerDelete(const QString& id, const QJsonObject& params);
  static CommandResponse registerUpdate(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
