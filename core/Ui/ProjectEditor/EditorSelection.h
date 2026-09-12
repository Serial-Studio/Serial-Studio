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

#include <QModelIndex>

#include "ProjectEditor/ProjectNavHistory.h"

class QStandardItem;

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief Routes a tree selection to the editor view it opens (entity, container or singleton
 *        node), keeps the back/forward navigation history, and hosts the programmatic
 *        select-by-id slots the facade forwards. The view state itself stays on the facade.
 */
class EditorSelection {
public:
  explicit EditorSelection(ProjectEditor& editor, ProjectModel& model);
  EditorSelection(EditorSelection&&)                 = delete;
  EditorSelection(const EditorSelection&)            = delete;
  EditorSelection& operator=(EditorSelection&&)      = delete;
  EditorSelection& operator=(const EditorSelection&) = delete;

  [[nodiscard]] bool canGoBack() const noexcept;
  [[nodiscard]] bool canGoForward() const noexcept;
  [[nodiscard]] int navDirection() const noexcept;

  void navigateBack();
  void clearNavHistory();
  void navigateForward();
  void selectGroup(int groupId);
  void selectSource(int sourceId);
  void selectAction(int actionId);
  void selectFrameParser(int sourceId);
  void displayFrameParserView(int sourceId);
  void selectDataset(int groupId, int datasetId);
  void selectOutputWidget(int groupId, int widgetId);
  void onCurrentSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

private:
  using NavEntry = ProjectNavHistory::Entry;

  [[nodiscard]] NavEntry captureNavEntry(QStandardItem* item) const;
  [[nodiscard]] QStandardItem* resolveNavEntry(const NavEntry& entry) const;

  bool selectGroupItem(QStandardItem* item);
  bool selectSourceItem(QStandardItem* item);
  bool selectActionItem(QStandardItem* item);
  bool selectDatasetItem(QStandardItem* item);
  bool selectDataTableItem(QStandardItem* item);
  bool selectInfluxSinkItem(QStandardItem* item);
  bool selectGroupFolderItem(QStandardItem* item);
  bool selectSourceParserItem(QStandardItem* item);
  bool selectOutputWidgetItem(QStandardItem* item);
  bool selectControlScriptItem(QStandardItem* item);
  bool selectTransformLibraryItem(QStandardItem* item);
  bool selectScriptsTreeItem(QStandardItem* item);
  bool selectExportRootItem(QStandardItem* item);
  bool selectMqttPublisherItem(QStandardItem* item);
  bool selectWorkspaceTreeItem(QStandardItem* item);

private:
  ProjectEditor& m_editor;
  ProjectModel& m_model;

  ProjectNavHistory m_nav;
};

}  // namespace DataModel
