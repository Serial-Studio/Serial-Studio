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

#include <QString>
#include <QVariantList>
#include <vector>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief Group, dataset and action editing for the project document (the fixed-layout templates
 *        live in ProjectFixedLayouts); the vectors stay on the facade, behaviour only lives here.
 *        Every mutating entry point opens its ProjectUndoScope and commits it through
 *        ProjectModel::setModified, keeping the two-phase memento contract intact.
 */
class ProjectEntities {
public:
  explicit ProjectEntities(ProjectModel& model);
  ProjectEntities(ProjectEntities&&)                 = delete;
  ProjectEntities(const ProjectEntities&)            = delete;
  ProjectEntities& operator=(ProjectEntities&&)      = delete;
  ProjectEntities& operator=(const ProjectEntities&) = delete;

  void renumberGroupIds();

  void updateGroup(const int groupId, const Group& group, const bool rebuildTree);
  void updateDataset(const int groupId,
                     const int datasetId,
                     const Dataset& dataset,
                     const bool rebuildTree);
  void updateAction(const int actionId, const Action& action, const bool rebuildTree);

  [[nodiscard]] int seedDatasetAliases();

  void setGroupEnabled(const int groupId, const bool enabled);
  void setDatasetEnabled(const int groupId, const int datasetId, const bool enabled);

  void addGroup(const QString& title,
                const SerialStudio::GroupWidget widget,
                int sourceId,
                int parentFolderId);
  void addDataset(const SerialStudio::DatasetOption option, int sourceId);
  void addAction(int sourceId);
  void ensureValidGroup(int sourceId);
  void ensurePainterDatasets(int groupId, const QVariantList& specs);
  void changeDatasetOption(const SerialStudio::DatasetOption option, const bool checked);
  [[nodiscard]] bool setGroupWidget(const int group, const SerialStudio::GroupWidget widget);

  void deleteCurrentGroup();
  void deleteCurrentAction();
  void deleteCurrentDataset();
  void duplicateCurrentGroup();
  void duplicateCurrentAction();
  void duplicateCurrentDataset();

  void deleteGroup(int groupId, bool confirm);
  void duplicateGroup(int groupId);
  void deleteDataset(int groupId, int datasetId, bool confirm);
  void duplicateDataset(int groupId, int datasetId);
  void deleteAction(int actionId, bool confirm);
  void duplicateAction(int actionId);

  void moveGroup(int fromGroupId, int toGroupId);
  void moveDataset(int groupId, int fromDatasetId, int toDatasetId);
  void moveAction(int fromActionId, int toActionId);

  void promptRenameGroup(int groupId);
  void promptRenameDataset(int groupId, int datasetId);
  void promptRenameAction(int actionId);

private:
  void remapGroupIdsAfterReorder(const std::vector<int>& oldToNewGid);
  [[nodiscard]] bool confirmGroupWidgetChange(Group& grp, SerialStudio::GroupWidget widget);
  [[nodiscard]] bool applyGroupWidget(Group& grp, SerialStudio::GroupWidget widget);

private:
  ProjectModel& m_model;
};

}  // namespace DataModel
