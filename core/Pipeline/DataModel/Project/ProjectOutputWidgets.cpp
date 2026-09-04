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

#include "DataModel/Project/ProjectOutputWidgets.h"

#include <algorithm>
#include <QMap>
#include <QMessageBox>

#include "AppInfo.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/Project/ProjectEntities.h"
#include "DataModel/Project/ProjectHistory.h"
#include "DataModel/Project/ProjectNaming.h"
#include "DataModel/Project/ProjectWorkspaces.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the output-widget editor to @p model.
 */
DataModel::ProjectOutputWidgets::ProjectOutputWidgets(ProjectModel& model) : m_model(model) {}

//--------------------------------------------------------------------------------------------------
// Creation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new output group with a default button control.
 */
void DataModel::ProjectOutputWidgets::addOutputPanel(int sourceId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Output Panel")};
  m_model.m_entities.addGroup(
    ProjectModel::tr("Output Controls"), SerialStudio::NoGroupWidget, sourceId, -1);
  auto& group             = m_model.m_groups.back();
  group.groupType         = DataModel::GroupType::Output;
  m_model.m_selectedGroup = group;

  addOutputControl(SerialStudio::OutputButton, sourceId);
}

/**
 * @brief Adds an output control, creating a new output group if needed.
 */
void DataModel::ProjectOutputWidgets::addOutputControl(const SerialStudio::OutputWidgetType type,
                                                       int sourceId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Output Widget")};
  auto& groups   = m_model.m_groups;
  int groupId    = -1;
  const auto sel = m_model.m_selectedGroup.groupId;
  if (sel >= 0 && static_cast<size_t>(sel) < groups.size()
      && groups[sel].groupType == DataModel::GroupType::Output
      && (sourceId < 0 || groups[sel].sourceId == sourceId))
    groupId = sel;

  if (groupId < 0) {
    for (const auto& g : std::as_const(groups)) {
      if (g.groupType != DataModel::GroupType::Output)
        continue;

      if (sourceId >= 0 && g.sourceId != sourceId)
        continue;

      groupId                 = g.groupId;
      m_model.m_selectedGroup = g;
      break;
    }
  }

  if (groupId < 0) {
    m_model.m_entities.addGroup(
      ProjectModel::tr("Output Controls"), SerialStudio::NoGroupWidget, sourceId, -1);
    auto& created           = groups.back();
    created.groupType       = DataModel::GroupType::Output;
    groupId                 = created.groupId;
    m_model.m_selectedGroup = created;
  }

  auto& group = groups[groupId];

  QString title;
  switch (type) {
    case SerialStudio::OutputButton:
      title = ProjectModel::tr("New Button");
      break;
    case SerialStudio::OutputSlider:
      title = ProjectModel::tr("New Slider");
      break;
    case SerialStudio::OutputToggle:
      title = ProjectModel::tr("New Toggle");
      break;
    case SerialStudio::OutputTextField:
      title = ProjectModel::tr("New Text Field");
      break;
    case SerialStudio::OutputKnob:
      title = ProjectModel::tr("New Knob");
      break;
  }

  DataModel::OutputWidget ow;
  ow.widgetId         = static_cast<int>(group.outputWidgets.size());
  ow.groupId          = groupId;
  ow.sourceId         = group.sourceId;
  ow.title            = title;
  ow.type             = static_cast<DataModel::OutputWidgetType>(type);
  ow.transmitFunction = DataModel::OutputCodeEditor::defaultTemplate();

  group.outputWidgets.push_back(ow);
  m_model.m_selectedOutputWidget = ow;

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.outputWidgetAdded(groupId, ow.widgetId);
  m_model.setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Property edits
//--------------------------------------------------------------------------------------------------

/**
 * @brief Changes the type of the currently selected output widget.
 */
void DataModel::ProjectOutputWidgets::setOutputWidgetType(int type)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Output Widget Type")};
  auto& selected = m_model.m_selectedOutputWidget;
  const auto gid = selected.groupId;
  const auto wid = selected.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_model.m_groups.size())
    return;

  auto& widgets = m_model.m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  const auto newType = static_cast<DataModel::OutputWidgetType>(
    qBound(0, type, static_cast<int>(DataModel::OutputWidgetType::Knob)));

  if (widgets[wid].type == newType)
    return;

  widgets[wid].type = newType;
  selected.type     = newType;

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.outputWidgetAdded(gid, wid);
  m_model.setModified(true);
}

/**
 * @brief Sets the icon of the currently selected output widget.
 */
void DataModel::ProjectOutputWidgets::setOutputWidgetIcon(const QString& icon)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Output Widget Icon")};
  auto& selected = m_model.m_selectedOutputWidget;
  const auto gid = selected.groupId;
  const auto wid = selected.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_model.m_groups.size())
    return;

  auto& widgets = m_model.m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  widgets[wid].icon = icon;
  selected.icon     = icon;

  Q_EMIT m_model.groupDataChanged();
  Q_EMIT m_model.outputWidgetAdded(gid, wid);
  m_model.setModified(true);
}

/**
 * @brief Updates an output widget in place.
 */
void DataModel::ProjectOutputWidgets::updateOutputWidget(int groupId,
                                                         int widgetId,
                                                         const DataModel::OutputWidget& widget,
                                                         bool rebuildTree)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Edit Output Widget")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  auto& widgets = groups[groupId].outputWidgets;
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= widgets.size())
    return;

  widgets[widgetId] = widget;

  if (rebuildTree)
    Q_EMIT m_model.groupsChanged();
  else
    Q_EMIT m_model.groupDataChanged();

  m_model.setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Deletion and duplication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the currently selected output widget after confirmation.
 */
void DataModel::ProjectOutputWidgets::deleteCurrentOutputWidget()
{
  if (!m_model.m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      ProjectModel::tr("Do you want to delete output widget \"%1\"?")
        .arg(m_model.m_selectedOutputWidget.title),
      ProjectModel::tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_model.m_selectedOutputWidget.groupId;
  const auto wid = m_model.m_selectedOutputWidget.widgetId;
  auto& groups   = m_model.m_groups;

  if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
    return;

  auto& widgets = groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Delete Output Widget")};
  auto& workspaces = m_model.m_workspaces;
  QMap<int, int> deletedTypeCounts;
  if (workspaces.customizeWorkspaces())
    deletedTypeCounts = workspaces.widgetTypeCountsForGroup(groups[gid]);

  widgets.erase(widgets.begin() + wid);

  if (widgets.empty()) {
    groups.erase(groups.begin() + gid);
    m_model.m_entities.renumberGroupIds();

    if (workspaces.customizeWorkspaces())
      workspaces.shiftWorkspaceRefsAfterGroupDelete(gid, deletedTypeCounts);

    workspaces.shiftHiddenGroupIdsAfterGroupDelete(gid);
    workspaces.shiftLayoutKeysAfterGroupDelete(gid);

    Q_EMIT m_model.groupsChanged();
    Q_EMIT m_model.groupDeleted();
    m_model.setModified(true);
    return;
  }

  for (int i = 0; i < static_cast<int>(widgets.size()); ++i)
    widgets[i].widgetId = i;

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.outputWidgetDeleted(gid);
  m_model.setModified(true);
}

/**
 * @brief Duplicates the currently selected output widget.
 */
void DataModel::ProjectOutputWidgets::duplicateCurrentOutputWidget()
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Output Widget")};
  const auto gid = m_model.m_selectedOutputWidget.groupId;
  auto& groups   = m_model.m_groups;
  if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
    return;

  auto& widgets = groups[gid].outputWidgets;

  DataModel::OutputWidget ow = m_model.m_selectedOutputWidget;
  ow.widgetId                = static_cast<int>(widgets.size());

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(widgets.size()));
  for (const auto& w : widgets)
    existingTitles.append(w.title);

  ow.title = nextDuplicateTitle(m_model.m_selectedOutputWidget.title, existingTitles);

  widgets.push_back(ow);
  m_model.m_selectedOutputWidget = ow;

  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.outputWidgetAdded(gid, ow.widgetId);
  m_model.setModified(true);
}

/**
 * @brief Deletes the output widget at @p groupId/@p widgetId.
 */
void DataModel::ProjectOutputWidgets::deleteOutputWidget(int groupId, int widgetId, bool confirm)
{
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return;

  const auto previousSelection = m_model.m_selectedOutputWidget;
  const auto groupCountBefore  = groups.size();
  const auto widgetCountBefore = groups[groupId].outputWidgets.size();
  m_model.setSelectedOutputWidget(groups[groupId].outputWidgets[widgetId]);

  const bool previousSuppress    = m_model.m_suppressMessageBoxes;
  m_model.m_suppressMessageBoxes = !confirm;
  deleteCurrentOutputWidget();
  m_model.m_suppressMessageBoxes = previousSuppress;

  const bool groupDeleted = groups.size() < groupCountBefore;
  const bool sameGroupShrunk =
    !groupDeleted && groups[groupId].outputWidgets.size() < widgetCountBefore;

  if (previousSelection.groupId < 0
      || (previousSelection.groupId == groupId && previousSelection.widgetId == widgetId))
    return;

  int restoreGid = previousSelection.groupId;
  int restoreWid = previousSelection.widgetId;
  if (groupDeleted && restoreGid > groupId)
    restoreGid -= 1;
  else if (sameGroupShrunk && restoreGid == groupId && restoreWid > widgetId)
    restoreWid -= 1;

  if (restoreGid >= 0 && static_cast<size_t>(restoreGid) < groups.size() && restoreWid >= 0
      && static_cast<size_t>(restoreWid) < groups[restoreGid].outputWidgets.size())
    m_model.setSelectedOutputWidget(groups[restoreGid].outputWidgets[restoreWid]);
}

/**
 * @brief Duplicates the output widget at @p groupId/@p widgetId.
 */
void DataModel::ProjectOutputWidgets::duplicateOutputWidget(int groupId, int widgetId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Output Widget")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return;

  const auto previousSelection = m_model.m_selectedOutputWidget;
  m_model.setSelectedOutputWidget(groups[groupId].outputWidgets[widgetId]);
  duplicateCurrentOutputWidget();

  if (previousSelection.groupId >= 0
      && static_cast<size_t>(previousSelection.groupId) < groups.size()
      && previousSelection.widgetId >= 0
      && static_cast<size_t>(previousSelection.widgetId)
           < groups[previousSelection.groupId].outputWidgets.size())
    m_model.setSelectedOutputWidget(
      groups[previousSelection.groupId].outputWidgets[previousSelection.widgetId]);
}

/**
 * @brief Moves an output widget within its group's outputWidgets list.
 */
void DataModel::ProjectOutputWidgets::moveOutputWidget(int groupId,
                                                       int fromWidgetId,
                                                       int toWidgetId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Move Output Widget")};
  auto& groups = m_model.m_groups;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  auto& widgets = groups[groupId].outputWidgets;
  const int n   = static_cast<int>(widgets.size());
  if (fromWidgetId < 0 || fromWidgetId >= n)
    return;

  const int target = std::clamp(toWidgetId, 0, n - 1);
  if (target == fromWidgetId)
    return;

  auto widget = widgets[fromWidgetId];
  widgets.erase(widgets.begin() + fromWidgetId);
  widgets.insert(widgets.begin() + target, widget);

  for (size_t i = 0; i < widgets.size(); ++i)
    widgets[i].widgetId = static_cast<int>(i);

  auto& selected = m_model.m_selectedOutputWidget;
  if (selected.groupId == groupId && selected.widgetId == fromWidgetId)
    selected = widgets[target];

  Q_EMIT m_model.groupsChanged();
  m_model.setModified(true);
}
