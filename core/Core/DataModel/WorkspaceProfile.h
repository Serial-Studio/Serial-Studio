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

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <vector>

#include "Core/DataModel/Frame.h"

namespace DataModel {

/**
 * @brief A named subset of the project's workspaces (spec 0083): the workspace folders (with their
 *        subtrees) and the individual workspaces an operator profile shows. Empty sets show all.
 */
struct WorkspaceProfile {
  int profileId = -1;
  QString title;
  std::vector<int> folderIds;
  std::vector<int> workspaceIds;
};

/**
 * @brief Serializes a WorkspaceProfile to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const WorkspaceProfile& p)
{
  QJsonObject obj;
  obj.insert(Keys::ProfileId, p.profileId);
  obj.insert(Keys::Title, p.title.simplified());

  QJsonArray folders;
  for (const int id : p.folderIds)
    folders.append(id);

  QJsonArray workspaces;
  for (const int id : p.workspaceIds)
    workspaces.append(id);

  obj.insert(Keys::FolderIds, folders);
  obj.insert(Keys::WorkspaceIds, workspaces);
  return obj;
}

/**
 * @brief Deserializes a WorkspaceProfile from a QJsonObject.
 */
[[nodiscard]] inline bool read(WorkspaceProfile& p, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  p.profileId = ss_jsr(obj, Keys::ProfileId, -1).toInt();
  p.title     = ss_jsr(obj, Keys::Title, "").toString().simplified();
  p.folderIds.clear();
  p.workspaceIds.clear();
  for (const auto& id : obj.value(Keys::FolderIds).toArray())
    p.folderIds.push_back(id.toInt());

  for (const auto& id : obj.value(Keys::WorkspaceIds).toArray())
    p.workspaceIds.push_back(id.toInt());

  return p.profileId >= 0 && !p.title.isEmpty();
}

}  // namespace DataModel
