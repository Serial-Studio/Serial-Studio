/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "ProjectEditor/EditorForms/OutputStateRows.h"

#include <QCoreApplication>
#include <QStandardItemModel>

#include "Core/IconRegistry.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditor/ProjectEditorItemIds.h"

using enum DataModel::ProjectEditor::CustomRoles;
using enum DataModel::ProjectEditor::EditorWidget;

//--------------------------------------------------------------------------------------------------
// Candidate resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief The path the table store indexes @p table by, which is what a state binding stores: a
 *        bare name resolves nothing for a table inside a folder and is ambiguous across folders.
 */
[[nodiscard]] static QString storePath(const DataModel::ProjectModel& project,
                                       const DataModel::TableDef& table)
{
  return DataModel::tableFullPath(project.editorTableFolders(), table.parentFolderId, table.name);
}

/**
 * @brief Labels every table variable the project defines, in picker order.
 */
[[nodiscard]] static QStringList tableCandidates(const DataModel::ProjectModel& project)
{
  QStringList out;
  for (const auto& table : project.tables())
    for (const auto& reg : table.registers)
      out.append(QStringLiteral("%1 / %2").arg(storePath(project, table), reg.name));

  return out;
}

/**
 * @brief Labels every dataset the project defines, in picker order.
 */
[[nodiscard]] static QStringList datasetCandidates(const DataModel::ProjectModel& project)
{
  QStringList out;
  for (const auto& group : project.groups())
    for (const auto& dataset : group.datasets)
      out.append(QStringLiteral("%1 / %2").arg(group.title, dataset.title));

  return out;
}

/**
 * @brief Position of the widget's stored table variable within tableCandidates(), 1-based because
 *        index 0 is "None". Zero when the project no longer defines it.
 */
[[nodiscard]] static int tableTargetIndex(const DataModel::ProjectModel& project,
                                          const DataModel::OutputWidget& widget)
{
  int index = 0;
  for (const auto& table : project.tables()) {
    const bool bound = storePath(project, table) == widget.stateTable;
    for (const auto& reg : table.registers) {
      ++index;
      if (bound && reg.name == widget.stateVariable)
        return index;
    }
  }

  return 0;
}

/**
 * @brief Position of dataset @p uniqueId within datasetCandidates(), 1-based. Zero when the
 *        project no longer defines it, so a deleted source resolves to unbound rather than
 *        silently rebinding the control to a neighbour.
 */
[[nodiscard]] static int datasetTargetIndex(const DataModel::ProjectModel& project,
                                            const int uniqueId)
{
  int index = 0;
  for (const auto& group : project.groups()) {
    for (const auto& dataset : group.datasets) {
      ++index;
      if (dataset.uniqueId == uniqueId)
        return index;
    }
  }

  return 0;
}

/**
 * @brief The uniqueId of the dataset at picker @p index, or -1 for "None" and an index past the
 *        end. The one walk behind both the state and the reply pickers.
 */
[[nodiscard]] static int datasetIdAtIndex(const DataModel::ProjectModel& project, const int index)
{
  int walked = 0;
  for (const auto& group : project.groups())
    for (const auto& dataset : group.datasets)
      if (++walked == index)
        return dataset.uniqueId;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Rows
//--------------------------------------------------------------------------------------------------

/**
 * @brief One "None" plus dataset picker row for a reply dataset (spec 0110).
 */
static void appendReplyPicker(QStandardItemModel* model,
                              const DataModel::ProjectModel& project,
                              const int boundId,
                              const int parameterType,
                              const QString& name,
                              const QString& description)
{
  QStringList candidates{QCoreApplication::translate("DataModel::ProjectEditor", "None")};
  candidates.append(datasetCandidates(project));

  auto* item = new QStandardItem();
  item->setData(true, Active);
  item->setData(ComboBox, WidgetType);
  item->setData(candidates, ComboBoxData);
  item->setData(datasetTargetIndex(project, boundId), EditableValue);
  item->setData(parameterType, ParameterType);
  item->setData(name, ParameterName);
  item->setData(description, ParameterDescription);
  model->appendRow(item);
}

/**
 * @brief Appends the reply rows (spec 0110): the token picker, then the result code and the
 *        optional message once a token is chosen.
 */
static void appendReplyRows(QStandardItemModel* model,
                            const DataModel::OutputWidget& widget,
                            const DataModel::ProjectModel& project)
{
  appendReplyPicker(model,
                    project,
                    widget.replyTokenDatasetId,
                    kOutputWidget_ReplyToken,
                    QCoreApplication::translate("DataModel::ProjectEditor", "Reply Token"),
                    QCoreApplication::translate(
                      "DataModel::ProjectEditor",
                      "The dataset where the device echoes the token this control sent. Leave it "
                      "unbound and the control does not wait for a reply."));
  if (widget.replyTokenDatasetId < 0)
    return;

  appendReplyPicker(model,
                    project,
                    widget.replyCodeDatasetId,
                    kOutputWidget_ReplyCode,
                    QCoreApplication::translate("DataModel::ProjectEditor", "Result Code"),
                    QCoreApplication::translate(
                      "DataModel::ProjectEditor",
                      "The dataset carrying the device's result: 0 acknowledges, anything else "
                      "refuses. A reply needs both a token and a code."));
  appendReplyPicker(
    model,
    project,
    widget.replyMessageDatasetId,
    kOutputWidget_ReplyMessage,
    QCoreApplication::translate("DataModel::ProjectEditor", "Reply Message"),
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Optional dataset with the device's text, shown beside a refusal"));
}

/**
 * @brief Appends the source target and the on-value of a two-state control; the caller has
 *        already checked that a state source is chosen.
 */
static void appendStateTargetRows(QStandardItemModel* model,
                                  const DataModel::OutputWidget& widget,
                                  const DataModel::ProjectModel& project)
{
  using namespace DataModel;

  const bool table = widget.stateSource == OutputStateSource::Table;
  QStringList candidates{QCoreApplication::translate("DataModel::ProjectEditor", "None")};
  candidates.append(table ? tableCandidates(project) : datasetCandidates(project));

  auto* targetItem = new QStandardItem();
  targetItem->setData(true, Active);
  targetItem->setData(ComboBox, WidgetType);
  targetItem->setData(candidates, ComboBoxData);
  targetItem->setData(table ? tableTargetIndex(project, widget)
                            : datasetTargetIndex(project, widget.stateDatasetId),
                      EditableValue);
  targetItem->setData(kOutputWidget_StateTarget, ParameterType);
  targetItem->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Source"),
                      ParameterName);
  targetItem->setData(
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "The dataset or table variable this control follows"),
    ParameterDescription);
  model->appendRow(targetItem);

  const bool twoState = widget.type == OutputWidgetType::Toggle
                     || (widget.type == OutputWidgetType::Button && widget.checkable);
  if (twoState) {
    auto* onItem = new QStandardItem();
    onItem->setData(true, Active);
    onItem->setData(TextField, WidgetType);
    onItem->setData(widget.stateOnValue, EditableValue);
    onItem->setData(kOutputWidget_StateOnValue, ParameterType);
    onItem->setData(QCoreApplication::translate("DataModel::ProjectEditor", "On Value"),
                    ParameterName);
    onItem->setData(QCoreApplication::translate(
                      "DataModel::ProjectEditor",
                      "Value that means on. Leave it empty and any non-zero number means on, "
                      "which a device reporting text like RUN can never satisfy. Anything "
                      "needing a threshold or a bitmask belongs in the dataset transform."),
                    ParameterDescription);
    model->appendRow(onItem);
  }
}

/**
 * @brief Appends the shared confirm window row (spec 0080, 0110).
 */
static void appendConfirmRow(QStandardItemModel* model, const DataModel::OutputWidget& widget)
{
  auto* confirmItem = new QStandardItem();
  confirmItem->setData(true, Active);
  confirmItem->setData(IntField, WidgetType);
  confirmItem->setData(widget.stateConfirmMs, EditableValue);
  confirmItem->setData(kOutputWidget_StateConfirmMs, ParameterType);
  confirmItem->setData(
    QCoreApplication::translate("DataModel::ProjectEditor", "Confirm Within (ms)"), ParameterName);
  confirmItem->setData(QCoreApplication::translate(
                         "DataModel::ProjectEditor",
                         "How long the control shows a request as outstanding: the wait for the "
                         "state source and for a reply to end, then it goes back to displaying "
                         "what the source reports"),
                       ParameterDescription);
  model->appendRow(confirmItem);
}

/**
 * @brief Appends the state-feedback rows. The picker is rebuilt from the project every time the
 *        form is built, so a source the project no longer defines cannot be re-selected.
 */
void DataModel::appendOutputStateRows(QStandardItemModel* model,
                                      const OutputWidget& widget,
                                      const ProjectModel& project)
{
  SS_ASSERT(model != nullptr, return);

  auto& registry = Core::services().iconRegistry;
  auto* header   = new QStandardItem();
  header->setData(true, Active);
  header->setData(SectionHeader, WidgetType);
  header->setData(QCoreApplication::translate("DataModel::ProjectEditor", "State Feedback"),
                  PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("dataset-values"), 16),
                  ParameterIcon);
  model->appendRow(header);

  auto* kindItem = new QStandardItem();
  kindItem->setData(true, Active);
  kindItem->setData(ComboBox, WidgetType);
  kindItem->setData(
    QStringList{QCoreApplication::translate("DataModel::ProjectEditor", "None"),
                QCoreApplication::translate("DataModel::ProjectEditor", "Dataset"),
                QCoreApplication::translate("DataModel::ProjectEditor", "Table Variable")},
    ComboBoxData);
  kindItem->setData(static_cast<int>(widget.stateSource), EditableValue);
  kindItem->setData(kOutputWidget_StateSource, ParameterType);
  kindItem->setData(QCoreApplication::translate("DataModel::ProjectEditor", "State Source"),
                    ParameterName);
  kindItem->setData(QCoreApplication::translate(
                      "DataModel::ProjectEditor",
                      "Where this control reads the state it displays. Unbound, it shows what "
                      "you last set rather than what the equipment reports."),
                    ParameterDescription);
  model->appendRow(kindItem);

  if (widget.stateSource != OutputStateSource::None)
    appendStateTargetRows(model, widget, project);

  appendReplyRows(model, widget, project);
  if (widget.stateSource != OutputStateSource::None || widget.replyTokenDatasetId >= 0
      || widget.replyCodeDatasetId >= 0)
    appendConfirmRow(model, widget);
}

/**
 * @brief Walks the table variables in picker order and adopts the one at @p index.
 */
static void applyTableTarget(DataModel::OutputWidget& widget,
                             const DataModel::ProjectModel& project,
                             const int index)
{
  int walked = 0;
  for (const auto& table : project.tables())
    for (const auto& reg : table.registers) {
      if (++walked != index)
        continue;

      widget.stateTable    = storePath(project, table);
      widget.stateVariable = reg.name;
      return;
    }
}

/**
 * @brief Resolves a picker index back onto the widget. Index 0 is "None", which clears the
 *        binding; an index past the end also clears it rather than binding to something arbitrary.
 */
void DataModel::applyOutputStateTarget(OutputWidget& widget,
                                       const ProjectModel& project,
                                       const int index)
{
  widget.stateDatasetId = -1;
  widget.stateTable.clear();
  widget.stateVariable.clear();
  if (index <= 0)
    return;

  if (widget.stateSource == OutputStateSource::Table) {
    applyTableTarget(widget, project, index);
    return;
  }

  widget.stateDatasetId = datasetIdAtIndex(project, index);
}

/**
 * @brief Resolves a reply picker index onto the widget's token, code or message id (spec 0110).
 */
void DataModel::applyOutputReplyTarget(OutputWidget& widget,
                                       const ProjectModel& project,
                                       const OutputReplyRole role,
                                       const int index)
{
  const int id = datasetIdAtIndex(project, index);
  switch (role) {
    case OutputReplyRole::Token:
      widget.replyTokenDatasetId = id;
      break;
    case OutputReplyRole::Code:
      widget.replyCodeDatasetId = id;
      break;
    case OutputReplyRole::Message:
      widget.replyMessageDatasetId = id;
      break;
  }
}
