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

#include "DataModel/Project/ProjectWorkspaceProfiles.h"

#include <algorithm>
#include <QCryptographicHash>
#include <QDebug>
#include <QInputDialog>
#include <QJsonArray>
#include <QSettings>
#include <QTimer>

#include "Core/Prompt/UserPrompt.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/Project/ProjectHistory.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the store to its document; no profile is active until one is chosen.
 */
DataModel::ProjectWorkspaceProfiles::ProjectWorkspaceProfiles(ProjectModel& model)
  : m_model(model), m_activeProfileId(-1)
{}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief The project's named workspace subsets, in file order.
 */
const std::vector<DataModel::WorkspaceProfile>& DataModel::ProjectWorkspaceProfiles::list()
  const noexcept
{
  return m_profiles;
}

/**
 * @brief Mutable profile list, for the loader.
 */
std::vector<DataModel::WorkspaceProfile>& DataModel::ProjectWorkspaceProfiles::
  mutableList() noexcept
{
  return m_profiles;
}

/**
 * @brief The profile the taskbar currently filters by, or -1 for every workspace.
 */
int DataModel::ProjectWorkspaceProfiles::activeProfileId() const noexcept
{
  return m_activeProfileId;
}

/**
 * @brief Returns the profile with @p profileId, or nullptr.
 */
const DataModel::WorkspaceProfile* DataModel::ProjectWorkspaceProfiles::profile(int profileId) const
{
  for (const auto& p : m_profiles)
    if (p.profileId == profileId)
      return &p;

  return nullptr;
}

/**
 * @brief Mutable counterpart of profile().
 */
DataModel::WorkspaceProfile* DataModel::ProjectWorkspaceProfiles::mutableProfile(int profileId)
{
  for (auto& p : m_profiles)
    if (p.profileId == profileId)
      return &p;

  return nullptr;
}

/**
 * @brief Resolves a profile by its title (case-insensitive), or -1; what the CLI and API select by.
 */
int DataModel::ProjectWorkspaceProfiles::profileIdForTitle(const QString& title) const
{
  const QString wanted = title.simplified();
  for (const auto& p : m_profiles)
    if (p.title.compare(wanted, Qt::CaseInsensitive) == 0)
      return p.profileId;

  return -1;
}

/**
 * @brief True when @p folderId is listed by the profile.
 */
bool DataModel::ProjectWorkspaceProfiles::hasFolder(int profileId, int folderId) const
{
  const auto* p = profile(profileId);
  return p && std::find(p->folderIds.begin(), p->folderIds.end(), folderId) != p->folderIds.end();
}

/**
 * @brief True when @p workspaceId is listed by the profile explicitly.
 */
bool DataModel::ProjectWorkspaceProfiles::hasWorkspace(int profileId, int workspaceId) const
{
  const auto* p = profile(profileId);
  return p
      && std::find(p->workspaceIds.begin(), p->workspaceIds.end(), workspaceId)
           != p->workspaceIds.end();
}

/**
 * @brief Whether the active profile shows @p workspace: no profile, an empty profile, an auto
 *        workspace, an explicitly listed id, or a folder ancestor the profile lists. Pure over the
 *        current lists, so nothing has to be invalidated when they change.
 */
bool DataModel::ProjectWorkspaceProfiles::workspaceVisible(const Workspace& workspace) const
{
  const auto* active = profile(m_activeProfileId);
  if (!active || (active->folderIds.empty() && active->workspaceIds.empty()))
    return true;

  if (workspace.workspaceId < WorkspaceIds::UserStart)
    return true;

  if (hasWorkspace(active->profileId, workspace.workspaceId))
    return true;

  const auto& folders = m_model.m_folders.workspaceFolders();
  for (const int folderId : active->folderIds)
    if (folderIsSelfOrDescendant(folders, folderId, workspace.parentFolderId))
      return true;

  return false;
}

/**
 * @brief Selects the profile the taskbar filters by (-1 = all). Runtime state, never persisted:
 *        the same file opens under a different profile on the next bench.
 */
void DataModel::ProjectWorkspaceProfiles::setActiveProfile(int profileId)
{
  const int resolved = profile(profileId) ? profileId : -1;
  if (m_activeProfileId == resolved)
    return;

  m_activeProfileId = resolved;
  Q_EMIT m_model.activeWorkspaceProfileChanged();
  Q_EMIT m_model.activeWorkspacesChanged();
}

/**
 * @brief Drops every profile and the selection (new document, load), announcing whichever of the
 *        two actually changed so the editor rows and the taskbar filter never outlive the list.
 */
void DataModel::ProjectWorkspaceProfiles::clear()
{
  const bool hadProfiles = !m_profiles.empty();
  const bool hadActive   = m_activeProfileId >= 0;
  m_profiles.clear();
  m_activeProfileId = -1;

  if (hadProfiles)
    notifyChanged();

  if (hadActive) {
    Q_EMIT m_model.activeWorkspaceProfileChanged();
    Q_EMIT m_model.activeWorkspacesChanged();
  }
}

/**
 * @brief Adds an empty profile and returns its id.
 */
int DataModel::ProjectWorkspaceProfiles::addProfile(const QString& title)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Workspace Profile")};

  int newId = 0;
  for (const auto& p : m_profiles)
    newId = qMax(newId, p.profileId + 1);

  WorkspaceProfile created;
  created.profileId = newId;
  created.title = title.simplified().isEmpty() ? ProjectModel::tr("Profile") : title.simplified();
  m_profiles.push_back(created);

  m_model.setModified(true);
  notifyChanged();
  return newId;
}

/**
 * @brief Renames a profile.
 */
void DataModel::ProjectWorkspaceProfiles::renameProfile(int profileId, const QString& title)
{
  auto* p = mutableProfile(profileId);
  if (!p || title.simplified().isEmpty() || p->title == title.simplified())
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Rename Workspace Profile")};
  p->title = title.simplified();
  m_model.setModified(true);
  notifyChanged();
}

/**
 * @brief Removes a profile; an active one falls back to showing every workspace.
 */
void DataModel::ProjectWorkspaceProfiles::deleteProfile(int profileId)
{
  const auto it = std::find_if(m_profiles.begin(), m_profiles.end(), [profileId](const auto& p) {
    return p.profileId == profileId;
  });
  if (it == m_profiles.end())
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Delete Workspace Profile")};
  m_profiles.erase(it);
  m_model.setModified(true);
  notifyChanged();

  if (m_activeProfileId == profileId)
    setActiveProfile(-1);
}

/**
 * @brief Adds or removes a workspace folder (with its subtree) from a profile.
 */
void DataModel::ProjectWorkspaceProfiles::setFolder(int profileId, int folderId, bool included)
{
  auto* p = mutableProfile(profileId);
  if (!p || folderId < 0 || hasFolder(profileId, folderId) == included)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Workspace Profile"), QStringLiteral("workspace-profile")};
  if (included)
    p->folderIds.push_back(folderId);
  else
    p->folderIds.erase(std::remove(p->folderIds.begin(), p->folderIds.end(), folderId),
                       p->folderIds.end());

  m_model.setModified(true);
  notifyChanged();
  Q_EMIT m_model.activeWorkspacesChanged();
}

/**
 * @brief Adds or removes one workspace from a profile explicitly.
 */
void DataModel::ProjectWorkspaceProfiles::setWorkspace(int profileId,
                                                       int workspaceId,
                                                       bool included)
{
  auto* p = mutableProfile(profileId);
  if (!p || workspaceId < 0 || hasWorkspace(profileId, workspaceId) == included)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Workspace Profile"), QStringLiteral("workspace-profile")};
  if (included)
    p->workspaceIds.push_back(workspaceId);
  else
    p->workspaceIds.erase(std::remove(p->workspaceIds.begin(), p->workspaceIds.end(), workspaceId),
                          p->workspaceIds.end());

  m_model.setModified(true);
  notifyChanged();
  Q_EMIT m_model.activeWorkspacesChanged();
}

/**
 * @brief Replaces a profile's folder list (the API's whole-list form).
 */
void DataModel::ProjectWorkspaceProfiles::setFolders(int profileId,
                                                     const std::vector<int>& folderIds)
{
  auto* p = mutableProfile(profileId);
  if (!p || p->folderIds == folderIds)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Workspace Profile"), QStringLiteral("workspace-profile")};
  p->folderIds = folderIds;
  m_model.setModified(true);
  notifyChanged();
  Q_EMIT m_model.activeWorkspacesChanged();
}

/**
 * @brief Replaces a profile's workspace list (the API's whole-list form).
 */
void DataModel::ProjectWorkspaceProfiles::setWorkspaces(int profileId,
                                                        const std::vector<int>& workspaceIds)
{
  auto* p = mutableProfile(profileId);
  if (!p || p->workspaceIds == workspaceIds)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Workspace Profile"), QStringLiteral("workspace-profile")};
  p->workspaceIds = workspaceIds;
  m_model.setModified(true);
  notifyChanged();
  Q_EMIT m_model.activeWorkspacesChanged();
}

/**
 * @brief Drops a deleted folder's id from every profile (the caller owns the modified flag).
 */
void DataModel::ProjectWorkspaceProfiles::forgetFolder(int folderId)
{
  bool changed = false;
  for (auto& p : m_profiles) {
    const auto it = std::remove(p.folderIds.begin(), p.folderIds.end(), folderId);
    changed       = changed || it != p.folderIds.end();
    p.folderIds.erase(it, p.folderIds.end());
  }

  if (changed)
    notifyChanged();
}

/**
 * @brief Drops a deleted workspace's id from every profile (the caller owns the modified flag).
 */
void DataModel::ProjectWorkspaceProfiles::forgetWorkspace(int workspaceId)
{
  bool changed = false;
  for (auto& p : m_profiles) {
    const auto it = std::remove(p.workspaceIds.begin(), p.workspaceIds.end(), workspaceId);
    changed       = changed || it != p.workspaceIds.end();
    p.workspaceIds.erase(it, p.workspaceIds.end());
  }

  if (changed)
    notifyChanged();
}

/**
 * @brief Prompts for a name and adds a profile.
 */
void DataModel::ProjectWorkspaceProfiles::promptAddProfile()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(nullptr,
                                             ProjectModel::tr("New Workspace Profile"),
                                             ProjectModel::tr("Name:"),
                                             QLineEdit::Normal,
                                             ProjectModel::tr("Profile"),
                                             &ok);
  if (!ok || name.trimmed().isEmpty())
    return;

  (void)addProfile(name.trimmed());
}

/**
 * @brief Prompts for a new name for a profile.
 */
void DataModel::ProjectWorkspaceProfiles::promptRenameProfile(int profileId)
{
  const auto* p = profile(profileId);
  if (!p)
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(nullptr,
                                             ProjectModel::tr("Rename Workspace Profile"),
                                             ProjectModel::tr("Name:"),
                                             QLineEdit::Normal,
                                             p->title,
                                             &ok);
  if (!ok || name.trimmed().isEmpty())
    return;

  renameProfile(profileId, name.trimmed());
}

/**
 * @brief Asks before deleting a profile.
 */
void DataModel::ProjectWorkspaceProfiles::confirmDeleteProfile(int profileId)
{
  const auto* p = profile(profileId);
  if (!p)
    return;

  const int choice = Core::Prompt::showMessageBox(
    ProjectModel::tr("Delete workspace profile \"%1\"?").arg(p->title),
    ProjectModel::tr(
      "The workspaces and folders it lists are kept; only the selection is removed."),
    Core::Prompt::Warning,
    ProjectModel::tr("Delete Workspace Profile"),
    Core::Prompt::Yes | Core::Prompt::Cancel,
    Core::Prompt::Cancel);
  if (choice == Core::Prompt::Yes)
    deleteProfile(profileId);
}

/**
 * @brief Announces a profile list change to the editor.
 */
void DataModel::ProjectWorkspaceProfiles::notifyChanged()
{
  Q_EMIT m_model.workspaceProfilesChanged();
}

//--------------------------------------------------------------------------------------------------
// Load-time selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the workspace profiles (spec 0083). The selection is dropped with the old list:
 *        a file open starts unfiltered, and only the undo path restores what it captured.
 */
void DataModel::ProjectWorkspaceProfiles::loadFromJson(const QJsonObject& json)
{
  clear();

  for (const auto& value : json.value(Keys::WorkspaceProfiles).toArray()) {
    DataModel::WorkspaceProfile profile;
    if (DataModel::read(profile, value.toObject()))
      m_profiles.push_back(profile);
  }

  if (!m_profiles.empty())
    notifyChanged();
}

/**
 * @brief The settings key remembering which profile a project file was last opened under.
 */
[[nodiscard]] static QString profileMemoryKey(const QString& path)
{
  const auto digest = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QStringLiteral("workspaceProfiles/") + QString::fromLatin1(digest);
}

/**
 * @brief Picks the profile a project with two or more of them opens under: the CLI or API
 *        request (consumed once), then the choice remembered for this file. When neither
 *        decides, a user-initiated open asks on the next event-loop turn, after the load has
 *        unwound; API, mirror, replay and benchmark loads show every workspace.
 */
void DataModel::ProjectWorkspaceProfiles::chooseAtLoad(bool interactive)
{
  if (m_profiles.size() < 2)
    return;

  const QString requested = m_model.requestedWorkspaceProfile();
  m_model.setRequestedWorkspaceProfile(QString());

  QSettings settings;
  const QString key = profileMemoryKey(m_model.m_filePath);
  int chosen        = profileIdForTitle(requested);
  if (chosen < 0 && !m_model.m_filePath.isEmpty())
    chosen = profileIdForTitle(settings.value(key).toString());

  if (chosen < 0 && !requested.isEmpty())
    qWarning() << "[ProjectModel] No workspace profile named" << requested << "in this project";

  const bool ask =
    chosen < 0 && interactive && !m_model.m_suppressMessageBoxes && !m_model.m_filePath.isEmpty();
  if (ask) {
    QTimer::singleShot(0, &m_model, [this] { promptAtLoad(); });
    return;
  }

  setActiveProfile(chosen);
  if (!m_model.m_filePath.isEmpty())
    settings.setValue(key, requested.isEmpty() ? settings.value(key).toString() : requested);
}

/**
 * @brief The deferred operator prompt: lists the profiles plus "All workspaces", applies the pick
 *        and remembers it for this file. Runs on its own event-loop turn, never inside a load.
 */
void DataModel::ProjectWorkspaceProfiles::promptAtLoad()
{
  if (m_profiles.size() < 2 || m_model.m_filePath.isEmpty())
    return;

  QStringList items{ProjectModel::tr("All workspaces")};
  for (const auto& profile : m_profiles)
    items.append(profile.title);

  bool ok            = false;
  const QString pick = QInputDialog::getItem(nullptr,
                                             ProjectModel::tr("Workspace Profile"),
                                             ProjectModel::tr("Show the workspaces of:"),
                                             items,
                                             0,
                                             false,
                                             &ok);
  if (!ok)
    return;

  const int chosen = profileIdForTitle(pick);
  setActiveProfile(chosen);

  QSettings settings;
  const auto* active = profile(chosen);
  settings.setValue(profileMemoryKey(m_model.m_filePath), active ? active->title : QString());
}
