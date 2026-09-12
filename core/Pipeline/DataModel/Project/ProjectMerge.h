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
#include <QMap>
#include <QSet>
#include <QString>
#include <vector>

#include "Core/DataModel/Frame.h"

/**
 * @brief Pure id remapping for "add to current project" (spec 0083): an importer's standalone
 *        project JSON is rewritten so its sources, groups, datasets, tables and workspaces can be
 *        appended to an open document without colliding with anything already in it. The loader
 *        owns the document, the signals and the undo scope; this unit holds only the arithmetic,
 *        so a ctest can pin it without the application.
 */
namespace DataModel::ProjectMerge {

/**
 * @brief What the open document already occupies.
 */
struct Base {
  int groupCount      = 0;   ///< Positional index the first appended group takes
  int nextSourceId    = 0;   ///< First free sourceId (max existing + 1)
  int nextUniqueId    = 1;   ///< The model's uniqueId allocator
  int nextWorkspaceId = -1;  ///< First free user workspace id (>= WorkspaceIds::UserStart)
  QSet<QString> tableNames;
  QSet<QString> workspaceTitles;
};

/**
 * @brief The imported entities with every id rewritten, ready to append verbatim.
 */
struct Result {
  std::vector<Source> sources;
  std::vector<Group> groups;
  std::vector<TableDef> tables;
  std::vector<Workspace> workspaces;
  QMap<QString, QString> renamedTables;  ///< Original table name -> the name that was free
  int nextUniqueId = 1;                  ///< Allocator value after the merge
};

[[nodiscard]] Result remap(const QJsonObject& imported, const Base& base, const QString& label);
[[nodiscard]] QString freeName(const QString& wanted,
                               const QSet<QString>& taken,
                               const QString& label);

}  // namespace DataModel::ProjectMerge
