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

#include <QCoreApplication>
#include <QString>
#include <QVariant>

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief The read-only QVariant summaries the container views render (tables, workspaces,
 *        folder trees and contents, the all-widgets catalogue), the container select-by-id
 *        slots, the tree search filter, and the reorder wrappers behind the Move Up / Move Down
 *        buttons.
 */
class EditorSummaries {
  Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor)

public:
  explicit EditorSummaries(ProjectEditor& editor, ProjectModel& model);
  EditorSummaries(EditorSummaries&&)                 = delete;
  EditorSummaries(const EditorSummaries&)            = delete;
  EditorSummaries& operator=(EditorSummaries&&)      = delete;
  EditorSummaries& operator=(const EditorSummaries&) = delete;

  [[nodiscard]] bool canMoveCurrentUp() const;
  [[nodiscard]] bool canMoveCurrentDown() const;
  [[nodiscard]] bool moveCurrentGroup(int direction);
  [[nodiscard]] bool moveCurrentAction(int direction);
  [[nodiscard]] bool moveCurrentDataset(int direction);
  [[nodiscard]] bool moveCurrentOutputWidget(int direction);
  [[nodiscard]] bool moveWorkspace(int workspaceId, int direction);
  [[nodiscard]] bool workspaceHasUnresolvedRefs(int workspaceId) const;
  int cleanupUnresolvedWorkspaceWidgets();
  [[nodiscard]] int unresolvedWorkspaceWidgetCount() const;
  [[nodiscard]] QVariantMap groupFolderPaths() const;
  [[nodiscard]] QVariantList tablesSummary() const;
  [[nodiscard]] QVariantList groupFolderTree() const;
  [[nodiscard]] QVariantList tableFolderTree() const;
  [[nodiscard]] QVariantList allWidgetsSummary() const;
  [[nodiscard]] QVariantList selectedTreeItems() const;
  [[nodiscard]] QVariantList workspacesSummary() const;
  [[nodiscard]] QVariantList workspaceFolderTree() const;
  [[nodiscard]] QVariantList systemDatasetsSummary() const;
  [[nodiscard]] QVariantList widgetsForWorkspace(int workspaceId) const;
  [[nodiscard]] QVariantList groupFolderContents(int parentFolderId) const;
  [[nodiscard]] QVariantList tableFolderContents(int parentFolderId) const;
  [[nodiscard]] QVariantList workspaceFolderContents(int parentFolderId) const;

  void selectInfluxSink();
  void selectMqttPublisher();
  void selectControlScript();
  void selectTransformLibrary();
  void selectJsLibrary();
  void selectProjectScripts();
  void selectDataExport();
  void selectWorkspace(int workspaceId);
  void selectGroupFolder(int folderId);
  void selectTableFolder(int folderId);
  void selectWorkspaceFolder(int folderId);
  void setTreeSearchQuery(const QString& query);
  void selectUserTable(const QString& tableName);
  void confirmCleanupUnresolvedWorkspaceWidgets();

private:
  [[nodiscard]] bool canMoveCurrent(int direction) const;

private:
  ProjectEditor& m_editor;
  ProjectModel& m_model;
};

}  // namespace DataModel
