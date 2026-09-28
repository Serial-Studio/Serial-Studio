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
#include <vector>

#include "Core/DataModel/Frame.h"

class QStandardItem;

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief Pushes form-model edits back into the project model: one itemChanged handler per form
 *        plus the dialog commits (alarm bands, frequency markers). Each edit carries its undo hint
 *        and refreshes the tree-item cache the form row mirrors, so a title edit never forces a
 *        tree rebuild.
 */
class EditorCommit {
  Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor)

public:
  explicit EditorCommit(ProjectEditor& editor, ProjectModel& model);
  EditorCommit(EditorCommit&&)                 = delete;
  EditorCommit(const EditorCommit&)            = delete;
  EditorCommit& operator=(EditorCommit&&)      = delete;
  EditorCommit& operator=(const EditorCommit&) = delete;

  void onGroupItemChanged(QStandardItem* item);
  void onSourceItemChanged(QStandardItem* item);
  void onActionItemChanged(QStandardItem* item);
  void onProjectItemChanged(QStandardItem* item);
  void onDatasetItemChanged(QStandardItem* item);
  void commitAlarmBands(const QVariantList& bands);
  void commitChannelSounds(const QVariantList& rows);
  [[nodiscard]] QVariantList channelSoundRows() const;
  void setProjectSoundSequence(const QString& letter);
  void onOutputWidgetItemChanged(QStandardItem* item);
  void commitFrequencyMarkers(const QVariantList& markers);
  void syncDatasetTreeIcon(const DataModel::Dataset& dataset);
  void syncDatasetTreeVirtualFlag(const DataModel::Dataset& dataset);
  void applyOutputWidgetField(QStandardItem* item, DataModel::OutputWidget& widget);

private:
  [[nodiscard]] bool validateSelectedDatasetAlias(const QString& newAlias);
  [[nodiscard]] bool datasetFormEditAccepted(int formId, const QVariant& value);
  [[nodiscard]] bool datasetAliasInUse(const QString& alias, int selfUniqueId) const;

  bool applyGroupWidgetEdit(int widgetIdx, int groupId);
  bool applyGroupTitleEdit(const QString& newTitle, int groupId);
#ifdef BUILD_COMMERCIAL
  bool applyGroupImgModeEdit(int modeIdx, int groupId);
#endif

  void commitDatasetFormEdit(int formId);
  void applyGroupSourceEdit(int srcIdx, int groupId);
  void handleSourceTitleChange(QStandardItem* item);
  void syncDatasetItemCache(int groupId, int datasetId);
  void handleSourceBusTypeChange(QStandardItem* item);
  void handleSourcePropertyChange(QStandardItem* item);
  void applyGroupBarPanelStyleEdit(int styleIdx, int groupId);
  void applyGroupLogAxisEdit(bool xAxis, bool enabled, int groupId);
  void commitAlarmBandsForSelection(const std::vector<DataModel::AlarmBand>& bands);
  void handleSourceFrameStartEndChange(QStandardItem* item, DataModel::Source& updated);
  void handleSourceFrameDetectionChange(QStandardItem* item, DataModel::Source& updated);
  void handleSourceDecoderChecksumChange(QStandardItem* item, DataModel::Source& updated);

private:
  ProjectEditor& m_editor;
  ProjectModel& m_model;
};

}  // namespace DataModel
