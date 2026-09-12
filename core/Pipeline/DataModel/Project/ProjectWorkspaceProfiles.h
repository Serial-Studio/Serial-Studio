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
#include <QString>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/WorkspaceProfile.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief The project's named workspace subsets (spec 0083) and the runtime choice of which one the
 *        taskbar shows. Profiles are document state with undo scopes; the active profile is
 *        runtime-only, chosen at load, by the CLI or by the API. Visibility is a pure predicate
 *        over the current lists, so nothing is cached and nothing has to be invalidated.
 */
class ProjectWorkspaceProfiles {
public:
  explicit ProjectWorkspaceProfiles(ProjectModel& model);
  ProjectWorkspaceProfiles(ProjectWorkspaceProfiles&&)                 = delete;
  ProjectWorkspaceProfiles(const ProjectWorkspaceProfiles&)            = delete;
  ProjectWorkspaceProfiles& operator=(ProjectWorkspaceProfiles&&)      = delete;
  ProjectWorkspaceProfiles& operator=(const ProjectWorkspaceProfiles&) = delete;

  [[nodiscard]] const std::vector<WorkspaceProfile>& list() const noexcept;
  [[nodiscard]] std::vector<WorkspaceProfile>& mutableList() noexcept;
  [[nodiscard]] int activeProfileId() const noexcept;
  [[nodiscard]] const WorkspaceProfile* profile(int profileId) const;
  [[nodiscard]] int profileIdForTitle(const QString& title) const;
  [[nodiscard]] bool hasFolder(int profileId, int folderId) const;
  [[nodiscard]] bool hasWorkspace(int profileId, int workspaceId) const;
  [[nodiscard]] bool workspaceVisible(const Workspace& workspace) const;

  void clear();
  void setActiveProfile(int profileId);
  [[nodiscard]] int addProfile(const QString& title);
  void renameProfile(int profileId, const QString& title);
  void deleteProfile(int profileId);
  void setFolder(int profileId, int folderId, bool included);
  void setWorkspace(int profileId, int workspaceId, bool included);
  void setFolders(int profileId, const std::vector<int>& folderIds);
  void setWorkspaces(int profileId, const std::vector<int>& workspaceIds);
  void forgetFolder(int folderId);
  void forgetWorkspace(int workspaceId);
  void promptAddProfile();
  void promptRenameProfile(int profileId);
  void confirmDeleteProfile(int profileId);
  void loadFromJson(const QJsonObject& json);
  void chooseAtLoad(bool interactive);
  void promptAtLoad();

private:
  [[nodiscard]] WorkspaceProfile* mutableProfile(int profileId);
  void notifyChanged();

private:
  ProjectModel& m_model;

  int m_activeProfileId;
  std::vector<WorkspaceProfile> m_profiles;
};

}  // namespace DataModel
