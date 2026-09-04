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
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <vector>

#include "DataModel/Frame.h"

/**
 * @brief Workspace widget-ref repair after a structural project change: the positional identities a
 *        ref carries (per-type slot index, per-group workspace id, layout:N settings key) do not
 *        survive a group or dataset delete/reorder on their own. Every function here is pure over
 *        the containers it is handed, so ProjectWorkspaces keeps ownership of the state, the signal
 *        emissions and the modified flag while this unit holds only the index arithmetic.
 */
namespace DataModel::WorkspaceRefs {

/**
 * @brief Stable anchor for a workspace ref across group/dataset reorders.
 */
struct RefAnchor {
  int widgetType;
  int sourceGid;
  int datasetFrameIndex;
  bool isGroupOrLed;
};

/**
 * @brief One anchor bucket per workspace, keyed by workspaceId so the pairing survives a reorder
 *        of the workspace list itself.
 */
using RefAnchors = QHash<int, std::vector<RefAnchor>>;

[[nodiscard]] QMap<int, int> widgetTypeCountsForGroup(const Group& group);

void shiftRefsAfterGroupDelete(std::vector<Workspace>& workspaces,
                               const std::vector<Group>& groups,
                               int deletedGid,
                               const QMap<int, int>& deletedTypeCounts);
void shiftRefsAfterDatasetDelete(std::vector<Workspace>& workspaces,
                                 const std::vector<Group>& groups,
                                 int groupId,
                                 int groupUniqueId,
                                 const QMap<int, int>& datasetTypeCounts);

void shiftHiddenGroupIdsAfterGroupDelete(QSet<int>& hiddenGroupIds, int deletedGid);
[[nodiscard]] bool shiftLayoutKeysAfterGroupDelete(QJsonObject& widgetSettings, int deletedGid);

void remapHiddenGroupIdsAfterReorder(QSet<int>& hiddenGroupIds,
                                     const std::vector<int>& oldToNewGid);
void remapLayoutKeysAfterReorder(QJsonObject& widgetSettings, const std::vector<int>& oldToNewGid);
void remapAutoWorkspaceIdsAfterReorder(std::vector<Workspace>& workspaces,
                                       const std::vector<int>& oldToNewGid);

[[nodiscard]] RefAnchors snapshotRefAnchors(const std::vector<Workspace>& workspaces,
                                            const std::vector<Group>& groups);
void resolveRefAnchors(std::vector<Workspace>& workspaces,
                       const RefAnchors& anchors,
                       const std::vector<Group>& groups);

}  // namespace DataModel::WorkspaceRefs
