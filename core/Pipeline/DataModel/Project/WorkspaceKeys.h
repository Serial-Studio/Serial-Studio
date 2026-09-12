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

#include <QHash>
#include <QString>
#include <vector>

#include "Core/DataModel/Frame.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief Workspace widget-reference keys shared by the project editor and the workspaces API:
 *        the (widgetType, groupId, relativeIndex) encoding and the lookup of every widget the
 *        project currently exposes, so neither side has to reach the other's library.
 */
namespace WorkspaceKeys {

/**
 * @brief Resolved (groupTitle, datasetTitle) pair for a workspace widget reference.
 */
struct ResolvedWidget {
  QString groupTitle;
  QString datasetTitle;
  int uniqueId       = -1;  ///< Dataset.uniqueId for dataset scope, Group.uniqueId otherwise
  int groupUniqueId  = -1;
  bool isGroupWidget = false;
  bool isLedPanel    = false;
};

[[nodiscard]] qint64 workspaceWidgetKey(int widgetType, int groupId, int relIdx);
[[nodiscard]] int widgetTypeOfKey(qint64 key);
[[nodiscard]] int relativeIndexOfKey(qint64 key);
[[nodiscard]] QHash<qint64, ResolvedWidget> buildResolvedWidgetLookup(const ProjectModel& pm);
[[nodiscard]] QHash<qint64, ResolvedWidget> buildResolvedWidgetLookup(
  const std::vector<Group>& groups, bool pro);

/**
 * @brief Re-derives every ref's relativeIndex from its (widgetType, groupUniqueId,
 *        datasetUniqueId) identity against @p lookup (spec 0083); a legacy ref that carries no
 *        datasetUniqueId is resolved by its ordinal once and back-filled. Returns how many refs
 *        resolved to nothing and were left untouched.
 */
[[nodiscard]] int rebindWidgetRefs(std::vector<Workspace>& workspaces,
                                   const QHash<qint64, ResolvedWidget>& lookup);

}  // namespace WorkspaceKeys
}  // namespace DataModel
