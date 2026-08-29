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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QString>
#include <QVariantList>
#include <vector>

#include "DataModel/Frame.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief The project's user-defined data tables (Variables) and every operation over them: table
 *        and register CRUD, folder-qualified path resolution, duplication, and CSV import/export.
 *        Owns the table vector; the folder tree the paths are built from belongs to ProjectFolders
 *        and is reached through the injected facade.
 */
class ProjectTables {
public:
  explicit ProjectTables(ProjectModel& model);
  ProjectTables(ProjectTables&&)                 = delete;
  ProjectTables(const ProjectTables&)            = delete;
  ProjectTables& operator=(ProjectTables&&)      = delete;
  ProjectTables& operator=(const ProjectTables&) = delete;

  [[nodiscard]] const std::vector<TableDef>& list() const noexcept;
  [[nodiscard]] std::vector<TableDef>& mutableList() noexcept;
  [[nodiscard]] int count() const noexcept;
  void clear();

  [[nodiscard]] QString tablePathFor(const TableDef& table) const;
  [[nodiscard]] int findTableIndexByPath(const QString& tablePath) const;
  [[nodiscard]] QVariantList registersForTable(const QString& table) const;

  [[nodiscard]] QString addTable(const QString& name, int parentFolderId);
  void deleteTable(const QString& name);
  void renameTable(const QString& oldName, const QString& newName);
  void appendTableCopyToFolder(const TableDef& src, int parentFolderId);
  void duplicateTableByPath(const QString& tablePath);

  void addRegister(const QString& table,
                   const QString& registerName,
                   bool computed,
                   const QVariant& defaultValue);
  void deleteRegister(const QString& table, const QString& registerName);
  [[nodiscard]] bool updateRegister(const QString& table,
                                    const QString& registerName,
                                    const QString& newName,
                                    bool computed,
                                    const QVariant& defaultValue);

  void promptAddTable();
  void promptRenameTable(const QString& oldName);
  void promptAddRegister(const QString& table);
  void promptRenameRegister(const QString& table, const QString& registerName);
  void confirmDeleteTable(const QString& name);
  void confirmDeleteRegister(const QString& table, const QString& registerName);

  void exportTableToCsv(const QString& tableName);
  void importTableFromCsv(const QString& tableName);

private:
  ProjectModel& m_model;

  std::vector<TableDef> m_tables;
};

}  // namespace DataModel
