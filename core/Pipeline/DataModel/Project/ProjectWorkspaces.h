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

#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <vector>

#include "DataModel/Frame.h"
#include "DataModel/Project/ProjectWorkspaceRefs.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief The dashboard workspace list and the auto/customized state machine behind it. In auto
 *        state the list is an unpersisted derived view rebuilt from the groups; once customized it
 *        is user-owned and mergeAutoWorkspaceUpdates() adds first-appearance auto refs without
 *        resurrecting user-removed ones. Workspace CRUD stays outside the undo history (spec 0031).
 */
class ProjectWorkspaces {
public:
  using RefAnchor  = WorkspaceRefs::RefAnchor;
  using RefAnchors = WorkspaceRefs::RefAnchors;

  explicit ProjectWorkspaces(ProjectModel& model);
  ProjectWorkspaces(ProjectWorkspaces&&)                 = delete;
  ProjectWorkspaces(const ProjectWorkspaces&)            = delete;
  ProjectWorkspaces& operator=(ProjectWorkspaces&&)      = delete;
  ProjectWorkspaces& operator=(const ProjectWorkspaces&) = delete;

  [[nodiscard]] const std::vector<Workspace>& list() const noexcept;
  [[nodiscard]] std::vector<Workspace>& mutableList() noexcept;
  [[nodiscard]] const std::vector<Workspace>& activeList() const;
  [[nodiscard]] int count() const noexcept;

  [[nodiscard]] const QSet<int>& hiddenGroupIds() const noexcept;
  [[nodiscard]] bool isGroupHidden(int groupId) const;
  [[nodiscard]] bool customizeWorkspaces() const noexcept;

  void resetDocument();
  void clearList();
  void clearHiddenGroupIds();
  void insertHiddenGroupId(int groupId);
  void setCustomizeFlagFromFile(bool enabled);
  void refreshAutoSnapshot();
  void rebuildSessionWorkspaces();
  void clearSessionWorkspaces();
  [[nodiscard]] bool clearTransientState();

  void setCustomizeWorkspaces(const bool enabled);

  [[nodiscard]] int addWorkspace(const QString& title);
  void deleteWorkspace(int workspaceId);
  void clearAllWorkspaces();
  void renameWorkspace(int workspaceId, const QString& title);
  void updateWorkspace(int workspaceId,
                       const QString& title,
                       const QString& icon,
                       const QString& description,
                       bool setTitle,
                       bool setIcon,
                       bool setDescription);
  void setWorkspaceIcon(int workspaceId, const QString& icon);
  void reorderWorkspaces(const QList<int>& userWorkspaceIds);
  void moveWorkspace(int workspaceId, int targetIndex);
  void addWidgetToWorkspace(int workspaceId, int widgetType, int groupUniqueId, int relativeIndex);
  void removeWidgetFromWorkspace(int workspaceId, int index);
  void removeWidgetFromWorkspace(int workspaceId,
                                 int widgetType,
                                 int groupUniqueId,
                                 int relativeIndex);
  [[nodiscard]] int cleanupWorkspaceWidgetRefs(const QSet<qint64>& validKeys);

  [[nodiscard]] QString workspaceTitle(int workspaceId) const;
  [[nodiscard]] QString workspaceIcon(int workspaceId) const;

  void promptAddWorkspace();
  void promptRenameWorkspace(int workspaceId);
  void confirmDeleteWorkspace(int workspaceId);

  [[nodiscard]] std::vector<Workspace> buildAutoWorkspaces() const;
  [[nodiscard]] std::vector<WorkspaceFolder> buildAutoWorkspaceFoldersFor(
    const std::vector<Workspace>& workspaces) const;
  void regenerateAutoWorkspacesUnnotified();
  [[nodiscard]] bool mergeAutoWorkspaceUpdates();
  [[nodiscard]] int autoGenerateWorkspaces();
  void resetWorkspacesToAuto();
  void confirmResetWorkspacesToAuto();

  void hideGroup(int groupId);
  void showGroup(int groupId);
  void showAllHiddenGroups();
  [[nodiscard]] QVariantList hiddenGroupsSummary() const;

  [[nodiscard]] QMap<int, int> widgetTypeCountsForGroup(const Group& g) const
  {
    return WorkspaceRefs::widgetTypeCountsForGroup(g);
  }

  void shiftWorkspaceRefsAfterGroupDelete(int deletedGid, const QMap<int, int>& deletedTypeCounts);
  void shiftWorkspaceRefsAfterDatasetDelete(int groupId, const QMap<int, int>& datasetTypeCounts);

  void shiftHiddenGroupIdsAfterGroupDelete(int deletedGid)
  {
    WorkspaceRefs::shiftHiddenGroupIdsAfterGroupDelete(m_hiddenGroupIds, deletedGid);
  }

  void shiftLayoutKeysAfterGroupDelete(int deletedGid);

  void remapHiddenGroupIdsAfterReorder(const std::vector<int>& oldToNewGid)
  {
    WorkspaceRefs::remapHiddenGroupIdsAfterReorder(m_hiddenGroupIds, oldToNewGid);
  }

  void remapLayoutKeysAfterReorder(const std::vector<int>& oldToNewGid);

  void remapAutoWorkspaceIdsAfterReorder(const std::vector<int>& oldToNewGid)
  {
    WorkspaceRefs::remapAutoWorkspaceIdsAfterReorder(m_workspaces, oldToNewGid);
  }

  [[nodiscard]] RefAnchors snapshotRefAnchors() const;
  void resolveRefAnchors(const RefAnchors& anchors);

private:
  void appendAutoGroupWorkspaces(std::vector<Workspace>& result,
                                 const std::vector<Group>& groups,
                                 const QMap<int, std::vector<WidgetRef>>& perGroupRefs) const;
  void notifyWorkspaceListChanged();

private:
  ProjectModel& m_model;

  bool m_customizeWorkspaces;
  QSet<int> m_hiddenGroupIds;
  std::vector<Workspace> m_workspaces;
  std::vector<Workspace> m_autoSnapshot;
  std::vector<Workspace> m_sessionWorkspaces;
};

}  // namespace DataModel
