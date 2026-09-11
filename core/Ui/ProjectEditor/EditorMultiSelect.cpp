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

#include "ProjectEditor/EditorMultiSelect.h"

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>
#include <QVector>

#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditorItemIds.h"

namespace DataModel {

using enum ProjectEditor::CustomRoles;
using enum ProjectEditor::EditorWidget;
using enum ProjectEditor::CurrentView;
using ItemKind = ProjectEditor::ItemKind;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the model.
 */
EditorMultiSelect::EditorMultiSelect(ProjectEditor& editor, ProjectModel& model)
  : m_editor(editor), m_model(model)
{}

//--------------------------------------------------------------------------------------------------
// Multi-selection aggregate editing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Detects a homogeneous multi-selection (>=2 items of one editable kind) and switches to the
 *        aggregate MultiSelectionView. Returns true when it took over the selection.
 */
bool EditorMultiSelect::tryMultiSelection()
{
  if (!m_editor.m_selectionModel || !m_editor.m_treeModel)
    return false;

  int kind = KindNone;
  QSet<qint64> seen;
  QVector<QPair<int, int>> items;
  const auto indexes = m_editor.m_selectionModel->selectedIndexes();
  for (const auto& idx : indexes) {
    if (!idx.isValid() || idx.column() != 0)
      continue;

    const auto key =
      (static_cast<qint64>(idx.row()) << 32) ^ reinterpret_cast<qint64>(idx.internalPointer());
    if (seen.contains(key))
      continue;

    seen.insert(key);

    const int k = m_editor.m_treeModel->data(idx, TreeItemKind).toInt();
    if (k != KindDataset && k != KindOutputWidget)
      return false;

    if (kind == KindNone)
      kind = k;
    else if (kind != k)
      return false;

    const int id     = m_editor.m_treeModel->data(idx, TreeItemId).toInt();
    const int parent = m_editor.m_treeModel->data(idx, TreeItemParentId).toInt();
    items.append(qMakePair(parent, id));
  }

  if (items.size() < 2)
    return false;

  if (m_editor.m_currentView == MultiSelectionView
      && m_editor.m_batchKind == static_cast<ItemKind>(kind) && m_editor.m_batchItems == items)
    return true;

  m_editor.m_batchKind  = static_cast<ItemKind>(kind);
  m_editor.m_batchItems = items;
  m_editor.setCurrentView(MultiSelectionView);
  buildMultiSelectionModel();
  Q_EMIT m_editor.currentViewChanged();
  return true;
}

/**
 * @brief Builds the aggregate form model for the current multi-selection's kind.
 */
void EditorMultiSelect::buildMultiSelectionModel()
{
  if (m_editor.m_batchKind == ProjectEditor::KindDataset)
    buildMultiDatasetModel();
  else if (m_editor.m_batchKind == ProjectEditor::KindOutputWidget)
    buildMultiOutputWidgetModel();
}

/**
 * @brief Extracts a ParameterType -> EditableValue map for a dataset straight from the descriptor
 *        table, used to decide which aggregate fields agree across the selection.
 */
QHash<int, QVariant> EditorMultiSelect::datasetEditValues(const DataModel::Dataset& dataset)
{
  QHash<int, QVariant> out;
  for (int i = 0; i < Registry::kDatasetPropertyCount; ++i) {
    const auto& property = Registry::kDatasetProperties[i];
    if (!property.hasFormRow)
      continue;

    const auto value = Registry::datasetFormValue(property.formId, dataset, m_model);
    if (value.isValid())
      out.insert(property.formId, value);
  }

  return out;
}

/**
 * @brief Builds the dataset aggregate form: rows from the first selection member, identity fields
 *        greyed, shared fields marked "Mixed" when the selection disagrees. Fans edits out on
 *        change via onMultiSelectionItemChanged.
 */
void EditorMultiSelect::buildMultiDatasetModel()
{
  auto& pm           = m_model;
  const auto& groups = pm.groups();

  QVector<DataModel::Dataset> sel;
  sel.reserve(static_cast<qsizetype>(m_editor.m_batchItems.size()));
  for (const auto& pr : m_editor.m_batchItems) {
    const int gid = pr.first, dsid = pr.second;
    if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
      continue;

    for (const auto& d : groups[gid].datasets)
      if (d.datasetId == dsid) {
        sel.append(d);
        break;
      }
  }

  if (sel.isEmpty())
    return;

  QVector<QHash<int, QVariant>> maps;
  maps.reserve(sel.size());
  for (const auto& d : sel)
    maps.append(datasetEditValues(d));

  if (m_editor.m_datasetModel) {
    m_editor.m_datasetModel->disconnect(&m_editor);
    m_editor.m_datasetModel->deleteLater();
  }

  m_editor.m_selectedDataset = sel.first();
  m_editor.m_datasetModel    = new CustomModel(&m_editor);

  m_editor.addGeneralSection(m_editor.m_datasetModel, m_editor.m_selectedDataset);
  m_editor.addPlotSection(m_editor.m_datasetModel, m_editor.m_selectedDataset);
  m_editor.addFFTSection(m_editor.m_datasetModel, m_editor.m_selectedDataset);
  m_editor.addWidgetSection(m_editor.m_datasetModel, m_editor.m_selectedDataset);
  m_editor.addLEDSection(m_editor.m_datasetModel, m_editor.m_selectedDataset);

  const int rows = m_editor.m_datasetModel->rowCount();
  for (int r = 0; r < rows; ++r) {
    auto* it = m_editor.m_datasetModel->item(r);
    if (!it)
      continue;

    const auto ptVar = it->data(ParameterType);
    if (!ptVar.isValid())
      continue;

    const int wt         = it->data(WidgetType).toInt();
    const bool intWidget = (wt == ComboBox || wt == CheckBox || wt == AutoIntField);
    const QVariant blank = intWidget ? QVariant(-1) : QVariant(QString());

    const int pt = ptVar.toInt();
    if (pt == kDatasetView_Title || pt == kDatasetView_Index || pt == kDatasetView_Alias) {
      it->setEditable(false);
      it->setData(false, Active);
      it->setData(blank, EditableValue);
      it->setData(tr("(multiple)"), PlaceholderValue);
      continue;
    }

    const QVariant first = maps.first().value(pt);
    bool mixed           = false;
    for (int i = 1; i < maps.size(); ++i)
      if (maps[i].contains(pt) && maps[i].value(pt) != first) {
        mixed = true;
        break;
      }

    if (mixed) {
      it->setData(blank, EditableValue);
      it->setData(tr("Mixed"), PlaceholderValue);
    }
  }

  QObject::connect(m_editor.m_datasetModel,
                   &CustomModel::itemChanged,
                   &m_editor,
                   [this](QStandardItem* item) { onMultiSelectionItemChanged(item); });

  Q_EMIT m_editor.datasetModelChanged();
  Q_EMIT m_editor.datasetOptionsChanged();
}

/**
 * @brief Maps an OutputWidget's editable fields to a ParameterType -> value hash, used to decide
 *        which aggregate rows agree across the selection.
 */
QHash<int, QVariant> EditorMultiSelect::outputWidgetEditValues(
  const DataModel::OutputWidget& widget)
{
  QHash<int, QVariant> out;
  out.insert(kOutputWidget_Title, widget.title);
  out.insert(kOutputWidget_Icon, widget.icon);
  out.insert(kOutputWidget_MonoIcon, widget.monoIcon);
  out.insert(kOutputWidget_Checkable, widget.checkable);
  out.insert(kOutputWidget_Color, widget.color);
  out.insert(kOutputWidget_Size, static_cast<int>(widget.size));
  out.insert(kOutputWidget_OnLabel, widget.onLabel);
  out.insert(kOutputWidget_OffLabel, widget.offLabel);
  out.insert(kOutputWidget_MinValue, widget.minValue);
  out.insert(kOutputWidget_MaxValue, widget.maxValue);
  out.insert(kOutputWidget_StepSize, widget.stepSize);
  out.insert(kOutputWidget_InitialValue, widget.initialValue);
  out.insert(kOutputWidget_TxEncoding, widget.txEncoding);
  return out;
}

/**
 * @brief Builds the output-widget aggregate form: rows from the first selection member, the
 * per-item title greyed, shared fields marked "Mixed" when the selection disagrees. Fans edits out
 * on change via onMultiSelectionItemChanged.
 */
void EditorMultiSelect::buildMultiOutputWidgetModel()
{
  auto& pm           = m_model;
  const auto& groups = pm.groups();

  QVector<DataModel::OutputWidget> sel;
  sel.reserve(static_cast<qsizetype>(m_editor.m_batchItems.size()));
  for (const auto& pr : m_editor.m_batchItems) {
    const int gid = pr.first, wid = pr.second;
    if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
      continue;

    for (const auto& w : groups[gid].outputWidgets)
      if (w.widgetId == wid) {
        sel.append(w);
        break;
      }
  }

  if (sel.isEmpty())
    return;

  QVector<QHash<int, QVariant>> maps;
  maps.reserve(sel.size());
  for (const auto& w : sel)
    maps.append(outputWidgetEditValues(w));

  if (m_editor.m_outputWidgetModel) {
    m_editor.m_outputWidgetModel->disconnect(&m_editor);
    m_editor.m_outputWidgetModel->deleteLater();
  }

  m_editor.m_selectedOutputWidget = sel.first();
  m_editor.m_outputWidgetModel    = new CustomModel(&m_editor);

  m_editor.m_forms.buildOutputWidgetCommonRows(m_editor.m_selectedOutputWidget);
  m_editor.m_forms.buildOutputWidgetTransmitRow(m_editor.m_selectedOutputWidget);
  m_editor.m_forms.buildOutputWidgetValueRows(m_editor.m_selectedOutputWidget);

  const int rows = m_editor.m_outputWidgetModel->rowCount();
  for (int r = 0; r < rows; ++r) {
    auto* it = m_editor.m_outputWidgetModel->item(r);
    if (!it)
      continue;

    const auto ptVar = it->data(ParameterType);
    if (!ptVar.isValid())
      continue;

    const int wt         = it->data(WidgetType).toInt();
    const bool intWidget = (wt == ComboBox || wt == CheckBox);
    const QVariant blank = intWidget ? QVariant(-1) : QVariant(QString());

    const int pt = ptVar.toInt();
    if (pt == kOutputWidget_Title) {
      it->setEditable(false);
      it->setData(false, Active);
      it->setData(blank, EditableValue);
      it->setData(tr("(multiple)"), PlaceholderValue);
      continue;
    }

    const QVariant first = maps.first().value(pt);
    bool mixed           = false;
    for (int i = 1; i < maps.size(); ++i)
      if (maps[i].contains(pt) && maps[i].value(pt) != first) {
        mixed = true;
        break;
      }

    if (mixed) {
      it->setData(blank, EditableValue);
      it->setData(tr("Mixed"), PlaceholderValue);
    }
  }

  QObject::connect(m_editor.m_outputWidgetModel,
                   &CustomModel::itemChanged,
                   &m_editor,
                   [this](QStandardItem* item) { onMultiSelectionItemChanged(item); });

  Q_EMIT m_editor.outputWidgetModelChanged();
}

/**
 * @brief True when a fan-out may keep the live aggregate model: the edit came from a typed field
 *        (rebuilding would destroy the focused delegate after every keystroke) and no commit hook
 *        asked for a reshape. Clears the row's stale "Mixed" placeholder in place, since the fan
 *        just made the selection agree; the caller still holds m_editor.m_batchApplying.
 */
static bool multiSelectionEditKeepsModel(QStandardItem* item,
                                         DataModel::PropertyHooks::RebuildHint hint)
{
  using Editor = DataModel::ProjectEditor;
  if (hint != DataModel::PropertyHooks::RebuildHint::None)
    return false;

  const int wt     = item->data(Editor::WidgetType).toInt();
  const bool typed = wt == Editor::TextField || wt == Editor::IntField || wt == Editor::FloatField
                  || wt == Editor::HexTextField || wt == Editor::PasswordField;
  if (!typed)
    return false;

  if (item->data(Editor::PlaceholderValue).toString() == Editor::tr("Mixed"))
    item->setData(QString(), Editor::PlaceholderValue);

  return true;
}

/**
 * @brief Fans a single aggregate-form field edit out across every selected item, as one modified
 *        state and one autosave, then rebuilds the aggregate model to refresh common/mixed.
 */
void EditorMultiSelect::onMultiSelectionItemChanged(QStandardItem* item)
{
  if (!item || m_editor.m_batchApplying)
    return;

  if (m_editor.m_batchKind == ProjectEditor::KindOutputWidget) {
    fanOutputWidgetSelectionEdit(item);
    return;
  }

  if (m_editor.m_batchKind != ProjectEditor::KindDataset)
    return;

  const auto idInt = static_cast<DatasetItem>(item->data(ParameterType).toInt());
  const auto value = item->data(EditableValue);
  const int vIdx   = value.toInt();
  if (idInt == kDatasetView_Widget && (vIdx < 0 || vIdx >= m_editor.m_datasetWidgets.size()))
    return;

  if (idInt == kDatasetView_Plot && (vIdx < 0 || vIdx >= m_editor.m_plotOptions.size()))
    return;

  if (idInt == kDatasetView_DisplayFormat && (vIdx < 0 || vIdx >= m_editor.m_displayFormats.size()))
    return;

  if (idInt == kDatasetView_FFT_Samples && (vIdx < 0 || vIdx >= m_editor.m_fftSamples.size()))
    return;

  if (idInt == kDatasetView_FFT_Window && (vIdx < 0 || vIdx >= m_editor.m_fftWindowValues.size()))
    return;

  auto& pm = m_model;

  QVector<DataModel::Dataset> sel;
  QVector<QPair<int, int>> ids;
  {
    const auto& groups = pm.groups();
    for (const auto& pr : m_editor.m_batchItems) {
      const int gid = pr.first, dsid = pr.second;
      if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
        continue;

      for (const auto& d : groups[gid].datasets)
        if (d.datasetId == dsid) {
          sel.append(d);
          ids.append(pr);
          break;
        }
    }
  }

  const ProjectUndoFrame undo_frame{pm, tr("Edit Selection")};
  pm.setNextUndoHint(tr("Edit Selection"),
                     QStringLiteral("multi-dataset:%1:%2")
                       .arg(QString::number(static_cast<int>(idInt)),
                            QString::number(m_editor.m_batchItems.size())));
  pm.setAutoSaveSuspended(true);
  m_editor.m_batchApplying = true;
  const int formId         = static_cast<int>(idInt);
  auto rebuild             = PropertyHooks::RebuildHint::None;
  for (int i = 0; i < sel.size(); ++i) {
    DataModel::Dataset ds = sel[i];
    const auto hint       = Registry::applyDatasetFormEdit(formId, value, ds, pm);
    if (hint != PropertyHooks::RebuildHint::None)
      rebuild = hint;

    if (idInt == kDatasetView_Virtual)
      m_editor.m_commit.syncDatasetTreeVirtualFlag(ds);

    pm.updateDataset(ids[i].first, ids[i].second, ds, false);
    m_editor.m_commit.syncDatasetTreeIcon(ds);
  }

  if (multiSelectionEditKeepsModel(item, rebuild)) {
    m_editor.m_batchApplying = false;
    pm.setAutoSaveSuspended(false);
    pm.flushAutoSave();
    return;
  }

  m_editor.m_batchApplying = false;
  pm.setAutoSaveSuspended(false);

  buildMultiDatasetModel();
  pm.flushAutoSave();
}

/**
 * @brief Fans one aggregate-form field edit out across every selected output widget, as one
 * modified state and one autosave, then rebuilds the aggregate model to refresh common/mixed.
 */
void EditorMultiSelect::fanOutputWidgetSelectionEdit(QStandardItem* item)
{
  auto& pm = m_model;

  QVector<DataModel::OutputWidget> sel;
  QVector<QPair<int, int>> ids;
  {
    const auto& groups = pm.groups();
    for (const auto& pr : m_editor.m_batchItems) {
      const int gid = pr.first, wid = pr.second;
      if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
        continue;

      for (const auto& w : groups[gid].outputWidgets)
        if (w.widgetId == wid) {
          sel.append(w);
          ids.append(pr);
          break;
        }
    }
  }

  const ProjectUndoFrame undo_frame{pm, tr("Edit Selection")};
  pm.setNextUndoHint(tr("Edit Selection"),
                     QStringLiteral("multi-owidget:%1:%2")
                       .arg(QString::number(item->data(ParameterType).toInt()),
                            QString::number(m_editor.m_batchItems.size())));
  pm.setAutoSaveSuspended(true);
  m_editor.m_batchApplying = true;
  for (int i = 0; i < sel.size(); ++i) {
    DataModel::OutputWidget widget = sel[i];
    m_editor.m_commit.applyOutputWidgetField(item, widget);
    pm.updateOutputWidget(ids[i].first, ids[i].second, widget, false);
  }

  if (multiSelectionEditKeepsModel(item, PropertyHooks::RebuildHint::None)) {
    m_editor.m_batchApplying = false;
    pm.setAutoSaveSuspended(false);
    pm.flushAutoSave();
    return;
  }

  m_editor.m_batchApplying = false;
  pm.setAutoSaveSuspended(false);

  buildMultiOutputWidgetModel();
  pm.flushAutoSave();
}

/**
 * @brief Applies a dataset visualization toggle (plot/FFT/waterfall/widget/LED) to every dataset in
 *        the current multi-selection, as one modified state and one autosave.
 */
void EditorMultiSelect::changeDatasetOptionForSelection(int option, bool checked)
{
  if (m_editor.m_batchKind != ProjectEditor::KindDataset)
    return;

  const auto opt = static_cast<SerialStudio::DatasetOption>(option);
  auto& pm       = m_model;

  QVector<DataModel::Dataset> sel;
  QVector<QPair<int, int>> ids;
  {
    const auto& groups = pm.groups();
    for (const auto& pr : m_editor.m_batchItems) {
      const int gid = pr.first, dsid = pr.second;
      if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
        continue;

      for (const auto& d : groups[gid].datasets)
        if (d.datasetId == dsid) {
          sel.append(d);
          ids.append(pr);
          break;
        }
    }
  }

  const ProjectUndoFrame undo_frame{pm, tr("Edit Selection")};
  pm.setAutoSaveSuspended(true);
  for (int i = 0; i < sel.size(); ++i) {
    DataModel::Dataset ds = sel[i];
    switch (opt) {
      case SerialStudio::DatasetPlot:
        ds.plt = checked;
        break;
      case SerialStudio::DatasetFFT:
        ds.fft = checked;
        break;
      case SerialStudio::DatasetBar:
        ds.widget = checked ? QStringLiteral("bar") : QString();
        break;
      case SerialStudio::DatasetGauge:
        ds.widget = checked ? QStringLiteral("gauge") : QString();
        break;
      case SerialStudio::DatasetCompass:
        ds.widget = checked ? QStringLiteral("compass") : QString();
        break;
      case SerialStudio::DatasetMeter:
        ds.widget = checked ? QStringLiteral("meter") : QString();
        break;
      case SerialStudio::DatasetLED:
        ds.led = checked;
        break;
      case SerialStudio::DatasetWaterfall:
        ds.waterfall = checked;
        break;
      default:
        break;
    }

    pm.updateDataset(ids[i].first, ids[i].second, ds, false);
    m_editor.m_commit.syncDatasetTreeIcon(ds);
  }
  pm.setAutoSaveSuspended(false);

  buildMultiDatasetModel();
  Q_EMIT m_editor.datasetOptionsChanged();
  pm.flushAutoSave();
}

}  // namespace DataModel
