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
#include <QObject>

#include "Core/DataModel/Frame.h"

namespace DataModel {

class DatasetTransformEditor;
class FrameBuilder;
class ProjectEditor;
class ProjectModel;

/**
 * @brief Builds the per-entity form models (project, group, source, action, dataset, output
 *        widget) row by row from the facade's combo vocabulary, and opens the dataset editing
 *        dialogs (alarm bands, frequency markers, value transform). The dataset rows themselves
 *        come from the generated DatasetForm.cpp members that stay on the facade.
 */
class EditorForms {
  Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor)

public:
  explicit EditorForms(ProjectEditor& editor, ProjectModel& model);
  ~EditorForms();
  EditorForms(EditorForms&&)                 = delete;
  EditorForms(const EditorForms&)            = delete;
  EditorForms& operator=(EditorForms&&)      = delete;
  EditorForms& operator=(const EditorForms&) = delete;

  void buildProjectModel();
  void openTransformEditor();
  void openAlarmBandsEditorForSelection();
  void openTransformEditorFor(int groupId, int datasetId);
  void openFrequencyMarkersEditorForSelection();
  void buildGroupModel(const DataModel::Group& group);
  void buildSourceModel(const DataModel::Source& source);
  void buildActionModel(const DataModel::Action& action);
  void buildDatasetModel(const DataModel::Dataset& dataset);
  void buildOutputWidgetModel(const DataModel::OutputWidget& widget);
  void buildOutputWidgetValueRows(const DataModel::OutputWidget& widget);
  void buildOutputWidgetCommonRows(const DataModel::OutputWidget& widget);
  void buildOutputWidgetLabelRows(const DataModel::OutputWidget& widget);
  void buildOutputWidgetTransmitRow(const DataModel::OutputWidget& widget);

private:
  void openAlarmBandsEditorForMultiSelection();
  void buildGroupXAxisRow(const DataModel::Group& group);
  void buildGroupWebViewRow(const DataModel::Group& group);
  void buildGroupImageSection(const DataModel::Group& group);
  void buildGroupSourceSection(const DataModel::Group& group);
  void buildGroupGeneralSection(const DataModel::Group& group);
  void buildGroupDatasetsSection(const DataModel::Group& group);
  void buildGroupBarPanelStyleRow(const DataModel::Group& group);
  void buildSourceCommonRows(const DataModel::Source& source);
  void appendDriverPropertyRows(const DataModel::Source& source);
  void buildSourcePayloadRows(const DataModel::Source& source);
  void buildSourceFrameDetectionRows(const DataModel::Source& source);
  void buildActionTimingRows(const DataModel::Action& action);
  void buildActionGeneralRows(const DataModel::Action& action);
  void buildActionPayloadRows(const DataModel::Action& action);

private:
  ProjectEditor& m_editor;
  ProjectModel& m_model;
  DataModel::FrameBuilder& m_frameBuilder;

  void ensureTransformEditor();

  DatasetTransformEditor* m_transformEditor;
  QMetaObject::Connection m_deviceListConn;
};

}  // namespace DataModel
