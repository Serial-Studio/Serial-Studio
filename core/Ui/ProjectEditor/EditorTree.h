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
#include <QHash>
#include <QJsonObject>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QTimer>

#include "Core/DataModel/Frame.h"

class QStandardItem;

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief Builds the project-structure tree model (sources, actions, groups, tables, workspaces,
 *        the singleton nodes), restores selection and path-keyed expansion across rebuilds, and
 *        coalesces model mutation bursts into one rebuild. The item-to-entity maps it fills stay
 *        on the facade because every other concern reads them.
 */
class EditorTree {
  Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor)

public:
  explicit EditorTree(ProjectEditor& editor, ProjectModel& model);
  EditorTree(EditorTree&&)                 = delete;
  EditorTree(const EditorTree&)            = delete;
  EditorTree& operator=(EditorTree&&)      = delete;
  EditorTree& operator=(const EditorTree&) = delete;

  [[nodiscard]] bool treeIndexExpanded(const QModelIndex& index) const;
  [[nodiscard]] bool treeIndexHasChildren(const QModelIndex& index) const;
  [[nodiscard]] static QString datasetTreeIcon(const DataModel::Dataset& dataset);

  void buildTreeModel();
  void expandAllTreeItems();
  void scheduleTreeRebuild();
  void persistTreeExpansion();
  void collapseTreeToOverview();
  void expandTreeToIndex(const QModelIndex& index);
  void setTreeIndexExpanded(const QModelIndex& index, bool expanded);

private:
  [[nodiscard]] QModelIndex consumePendingSelection();
  [[nodiscard]] QJsonObject snapshotTreeExpansion();
  [[nodiscard]] QStandardItem* entitySelectionItem() const;
  [[nodiscard]] QStandardItem* containerSelectionItem() const;
  [[nodiscard]] QStandardItem* createWorkspaceItem(const DataModel::Workspace& ws);
  [[nodiscard]] QHash<int, QStandardItem*> appendGroupFolderItems(
    QStandardItem* groupsRoot, const QString& pathPrefix, QHash<QString, bool>& expandedStates);
  [[nodiscard]] QHash<int, QStandardItem*> appendTableFolderItems(
    QStandardItem* tablesRoot, const QString& pathPrefix, QHash<QString, bool>& expandedStates);

  void restoreTreeSelection();
  void appendSourceTreeItems(QStandardItem* root);
  void appendActionTreeItems(QStandardItem* root);
  void appendExportTree(QStandardItem* root, QHash<QString, bool>& expandedStates);
  void appendScriptsTree(QStandardItem* root, QHash<QString, bool>& expandedStates);
  [[nodiscard]] QStandardItem* createSingleLeaf(
    const QString& title,
    const QString& iconName,
    int kind,
    const QString& iconSet = QStringLiteral("editor")) const;
  void clearItemMaps();
  void buildTreeItems(QStandardItem* root, QHash<QString, bool>& expandedStates);
  void appendGroupTreeItems(QStandardItem* root, QHash<QString, bool>& expandedStates);
  void appendDatasetChildren(QStandardItem* groupItem, const DataModel::Group& group);
  void appendOutputWidgetChildren(QStandardItem* groupItem, const DataModel::Group& group);
  void appendWorkspaceTreeItems(QStandardItem* root, QHash<QString, bool>& expandedStates);
  void appendSharedMemoryTreeItems(QStandardItem* root, QHash<QString, bool>& expandedStates);
  void saveExpandedStateMap(QStandardItem* item, QHash<QString, bool>& map, const QString& title);
  void buildWorkspaceFolderTree(QStandardItem* wsRoot,
                                const QString& pathPrefix,
                                QHash<QString, bool>& expandedStates);
  void restoreExpandedStateMap(QStandardItem* item,
                               QHash<QString, bool>& map,
                               const QString& title,
                               bool defaultExpanded = true);

private:
  ProjectEditor& m_editor;
  ProjectModel& m_model;

  QTimer m_rebuildTimer;
  QMetaObject::Connection m_currentSelectionConnection;
};

}  // namespace DataModel
