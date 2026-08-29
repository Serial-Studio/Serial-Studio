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

#include <QVariantList>

namespace DataModel {

class ProjectModel;

/**
 * @brief Tree multi-selection operations: duplicate, delete, re-file and enable/disable a whole
 *        selection in one undo step. Deletion is the delicate one: children go first, ids descend
 *        within a kind, and group-referencing entries are pinned to uniqueIds and re-resolved per
 *        delete, because a cascading group deletion renumbers every later group.
 */
class ProjectBulkOps {
public:
  explicit ProjectBulkOps(ProjectModel& model);
  ProjectBulkOps(ProjectBulkOps&&)                 = delete;
  ProjectBulkOps(const ProjectBulkOps&)            = delete;
  ProjectBulkOps& operator=(ProjectBulkOps&&)      = delete;
  ProjectBulkOps& operator=(const ProjectBulkOps&) = delete;

  void duplicateSelectedItems(const QVariantList& items);
  void deleteSelectedItems(const QVariantList& items);
  void confirmDeleteSelectedItems(const QVariantList& items);
  void moveSelectedItemsToFolder(const QVariantList& items, int folderId);
  void setItemsEnabled(const QVariantList& items, const bool enabled);

private:
  [[nodiscard]] bool setGroupsInFolderEnabled(const int folderId, const bool enabled);

private:
  ProjectModel& m_model;
};

}  // namespace DataModel
