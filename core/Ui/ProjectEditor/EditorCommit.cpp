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

#include "ProjectEditor/EditorCommit.h"

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
#include "Core/IO/HAL_Driver.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "ProjectEditor/EditorForms/OutputStateRows.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditorItemIds.h"
#include "UI/WidgetExtensions.h"

namespace DataModel {

using enum ProjectEditor::CustomRoles;
using enum ProjectEditor::CurrentView;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the model.
 */
EditorCommit::EditorCommit(ProjectEditor& editor, ProjectModel& model)
  : m_editor(editor), m_model(model)
{}

/**
 * @brief Parses the AlarmBandsEditor dialog's QVariantList payload into validated alarm bands,
 *        dropping degenerate (max <= min) entries.
 */
[[nodiscard]] static std::vector<DataModel::AlarmBand> parseAlarmBandList(const QVariantList& bands)
{
  std::vector<DataModel::AlarmBand> out;
  out.reserve(bands.size());
  for (const auto& v : bands) {
    const auto m = v.toMap();
    DataModel::AlarmBand band;
    band.min   = SerialStudio::toDouble(m.value(QStringLiteral("min")));
    band.max   = SerialStudio::toDouble(m.value(QStringLiteral("max")));
    band.blink = m.value(QStringLiteral("blink"), false).toBool();
    band.color = m.value(QStringLiteral("color")).toString().simplified();
    band.label = m.value(QStringLiteral("label")).toString().simplified();
    const int sev =
      m.value(QStringLiteral("severity"), static_cast<int>(DataModel::AlarmSeverity::Warning))
        .toInt();
    band.severity = static_cast<DataModel::AlarmSeverity>(qBound(0, sev, 3));
    if (band.max > band.min)
      out.push_back(std::move(band));
  }

  return out;
}

/**
 * @brief Applies a source-title edit and syncs the tree-item cache.
 */
void EditorCommit::handleSourceTitleChange(QStandardItem* item)
{
  const QString newTitle = item->data(EditableValue).toString();
  if (m_editor.m_selectedSource.title == newTitle)
    return;

  m_editor.m_selectedSource.title = newTitle;
  m_model.setNextUndoHint(
    tr("Rename Device"), QStringLiteral("source-title:%1").arg(m_editor.m_selectedSource.sourceId));
  m_model.updateSourceTitle(m_editor.m_selectedSource.sourceId, newTitle, false);

  for (auto it = m_editor.m_sourceItems.begin(); it != m_editor.m_sourceItems.end(); ++it) {
    if (it.value().sourceId != m_editor.m_selectedSource.sourceId)
      continue;

    auto* treeItem = it.key();
    treeItem->setText(newTitle);
    treeItem->setData(newTitle, TreeViewText);
    m_editor.m_sourceItems[treeItem].title = newTitle;
    break;
  }

  Q_EMIT m_editor.selectedTextChanged();
}

/**
 * @brief Applies a bus-type edit and rebuilds the source form once contexts are ready.
 */
void EditorCommit::handleSourceBusTypeChange(QStandardItem* item)
{
  const int busType = item->data(EditableValue).toInt();
  m_model.updateSourceBusType(m_editor.m_selectedSource.sourceId, busType);
  m_editor.m_selectedSource.busType = busType;
  auto conn                         = std::make_shared<QMetaObject::Connection>();
  *conn                             = QObject::connect(
    &m_editor.m_connectionManager,
    &IO::ConnectionManager::contextsRebuilt,
    &m_editor,
    [this, conn] {
      QObject::disconnect(*conn);
      m_editor.m_forms.buildSourceModel(m_editor.m_selectedSource);
    },
    Qt::QueuedConnection);
}

/**
 * @brief The rows the source form shows for @p props (visible and labelled) with their option
 *        lists; an edit that changes it needs a rebuild, whichever driver gates the row and
 *        however (visibleWhen rules or a list that prunes by mode).
 */
[[nodiscard]] static QStringList sourceFormSignature(const QList<IO::DriverProperty>& props)
{
  QStringList signature;
  for (const auto& prop : props) {
    if (prop.label.isEmpty() || !IO::driverPropertyVisible(prop, props))
      continue;

    signature.append(prop.key + QLatin1Char('|') + prop.options.join(QLatin1Char(',')));
  }

  return signature;
}

/**
 * @brief Applies a UI-driver property edit and rebuilds the form when the edit changed which rows
 *        show or what a combo offers, so dependent rows and option lists never go stale.
 */
void EditorCommit::handleSourcePropertyChange(QStandardItem* item)
{
  const QString key  = item->data(ParameterKey).toString();
  const QVariant val = item->data(EditableValue);
  IO::HAL_Driver* drv =
    m_editor.m_connectionManager.driverForEditing(m_editor.m_selectedSource.sourceId);

  QStringList before;
  if (drv) {
    before = sourceFormSignature(drv->driverProperties());
    drv->setDriverProperty(key, val);
  }

  m_model.captureSourceSettings(m_editor.m_selectedSource.sourceId);

  if (drv && before != sourceFormSignature(drv->driverProperties()))
    m_editor.m_forms.buildSourceModel(m_editor.m_selectedSource);
}

/**
 * @brief Dispatches source form edits to ProjectModel or the live driver.
 */
void EditorCommit::onSourceItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const int id = item->data(ParameterType).toInt();

  if (id == kSourceView_Title) {
    handleSourceTitleChange(item);
    return;
  }

  if (id == kSourceView_BusType) {
    handleSourceBusTypeChange(item);
    return;
  }

  if (id == kSourceView_Property) {
    handleSourcePropertyChange(item);
    return;
  }

  DataModel::Source updated = m_editor.m_selectedSource;
  switch (static_cast<SourceItem>(id)) {
    case kSourceView_FrameDetection:
    case kSourceView_HexadecimalSequence:
      handleSourceFrameDetectionChange(item, updated);
      break;
    case kSourceView_FrameStartSequence:
    case kSourceView_FrameEndSequence:
      handleSourceFrameStartEndChange(item, updated);
      break;
    case kSourceView_FrameDecoder:
    case kSourceView_ChecksumFunction:
      handleSourceDecoderChecksumChange(item, updated);
      break;
    default:
      break;
  }
}

/**
 * @brief Applies a frame-detection-method or hex-delimiter edit and rebuilds the source form.
 */
void EditorCommit::handleSourceFrameDetectionChange(QStandardItem* item, DataModel::Source& updated)
{
  const int id  = item->data(ParameterType).toInt();
  const int sid = m_editor.m_selectedSource.sourceId;

  if (id == kSourceView_FrameDetection) {
    const int idx = item->data(EditableValue).toInt();
    if (idx < 0 || idx >= m_editor.m_frameDetectionMethodsValues.size())
      return;

    updated.frameDetection = static_cast<int>(m_editor.m_frameDetectionMethodsValues.at(idx));
  } else {
    updated.hexadecimalDelimiters = item->data(EditableValue).toBool();
  }

  m_model.updateSource(sid, updated);
  m_editor.m_selectedSource = updated;

  m_editor.m_forms.buildSourceModel(m_editor.m_selectedSource);
}

/**
 * @brief Applies a frame start/end delimiter edit to the source.
 */
void EditorCommit::handleSourceFrameStartEndChange(QStandardItem* item, DataModel::Source& updated)
{
  const int id  = item->data(ParameterType).toInt();
  const int sid = m_editor.m_selectedSource.sourceId;

  if (id == kSourceView_FrameStartSequence)
    updated.frameStart = item->data(EditableValue).toString();
  else
    updated.frameEnd = item->data(EditableValue).toString();

  m_model.setNextUndoHint(
    tr("Edit Device"),
    QStringLiteral("source-frame:%1:%2").arg(QString::number(sid), QString::number(id)));
  m_model.updateSource(sid, updated, false);
  m_editor.m_selectedSource = updated;
}

/**
 * @brief Applies a decoder-method or checksum-algorithm edit to the source.
 */
void EditorCommit::handleSourceDecoderChecksumChange(QStandardItem* item,
                                                     DataModel::Source& updated)
{
  const int id  = item->data(ParameterType).toInt();
  const int sid = m_editor.m_selectedSource.sourceId;

  if (id == kSourceView_FrameDecoder) {
    updated.decoderMethod = item->data(EditableValue).toInt();
    m_model.updateSource(sid, updated);
    m_editor.m_selectedSource = updated;
    return;
  }

  const auto checksums = IO::availableChecksums();
  const int checksumId = item->data(EditableValue).toInt();
  if (checksumId < 0 || checksumId >= checksums.size())
    return;

  updated.checksumAlgorithm = checksums.at(checksumId);
  m_model.updateSource(sid, updated);
  m_editor.m_selectedSource = updated;
}

//--------------------------------------------------------------------------------------------------
// Private slot: item changed handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Propagates group form edits to ProjectModel and the tree.
 */
void EditorCommit::onGroupItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id      = static_cast<GroupItem>(item->data(ParameterType).toInt());
  const auto value   = item->data(EditableValue);
  auto& pm           = m_model;
  const auto groupId = m_editor.m_selectedGroup.groupId;

  if (id == kGroupView_Dataset) {
    const int datasetId = item->data(ParameterKey).toInt();
    QTimer::singleShot(0, &m_editor, [this, groupId, datasetId] {
      m_editor.m_selection.selectDataset(groupId, datasetId);
    });
    return;
  }

  if (id == kGroupView_Title) {
    if (!applyGroupTitleEdit(value.toString(), groupId))
      return;

    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  if (id == kGroupView_Source) {
    applyGroupSourceEdit(value.toInt(), groupId);
    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  if (id == kGroupView_Widget) {
    if (!applyGroupWidgetEdit(value.toInt(), groupId))
      return;

    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  if (id == kGroupView_xAxis) {
    const int xAxisId = (value.toInt() == 1) ? kXAxisSamples : kXAxisTime;
    for (auto& dataset : m_editor.m_selectedGroup.datasets)
      dataset.xAxisId = xAxisId;

    pm.updateGroup(groupId, m_editor.m_selectedGroup);
    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  if (id == kGroupView_LogX || id == kGroupView_LogY) {
    applyGroupLogAxisEdit(id == kGroupView_LogX, value.toBool(), groupId);
    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  if (id == kGroupView_WebUrl) {
    m_editor.m_selectedGroup.webViewUrl = value.toString();
    pm.setNextUndoHint(tr("Edit Group"), QStringLiteral("group-weburl:%1").arg(groupId));
    pm.updateGroup(groupId, m_editor.m_selectedGroup, false);
    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  if (id == kGroupView_BarPanelStyle) {
    applyGroupBarPanelStyleEdit(value.toInt(), groupId);
    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

#ifdef BUILD_COMMERCIAL
  if (id == kGroupView_ImgMode) {
    if (applyGroupImgModeEdit(value.toInt(), groupId))
      return;

    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  if (id == kGroupView_ImgStart) {
    m_editor.m_selectedGroup.imgStartSequence = value.toString();
    pm.setNextUndoHint(tr("Edit Group"), QStringLiteral("group-imgstart:%1").arg(groupId));
    pm.updateGroup(groupId, m_editor.m_selectedGroup, false);
  }

  if (id == kGroupView_ImgEnd) {
    m_editor.m_selectedGroup.imgEndSequence = value.toString();
    pm.setNextUndoHint(tr("Edit Group"), QStringLiteral("group-imgend:%1").arg(groupId));
    pm.updateGroup(groupId, m_editor.m_selectedGroup, false);
  }
#endif

  Q_EMIT m_editor.editableOptionsChanged();
}

/**
 * @brief Applies a group-title edit; returns false when the title is unchanged.
 */
bool EditorCommit::applyGroupTitleEdit(const QString& newTitle, int groupId)
{
  if (m_editor.m_selectedGroup.title == newTitle)
    return false;

  m_editor.m_selectedGroup.title = newTitle;
  m_model.setNextUndoHint(tr("Rename Group"), QStringLiteral("group-title:%1").arg(groupId));
  m_model.updateGroup(groupId, m_editor.m_selectedGroup, false);

  for (auto it = m_editor.m_groupItems.begin(); it != m_editor.m_groupItems.end(); ++it) {
    if (it.value().groupId != groupId)
      continue;

    auto* treeItem = it.key();
    treeItem->setText(newTitle);
    treeItem->setData(newTitle, TreeViewText);
    m_editor.m_groupItems[treeItem].title = newTitle;
    break;
  }

  Q_EMIT m_editor.selectedTextChanged();
  return true;
}

/**
 * @brief Re-routes the group (and its datasets) to the source at the given combobox index.
 */
void EditorCommit::applyGroupSourceEdit(int srcIdx, int groupId)
{
  const auto& sources = m_model.sources();
  if (srcIdx < 0 || srcIdx >= static_cast<int>(sources.size()))
    return;

  m_editor.m_selectedGroup.sourceId = sources[srcIdx].sourceId;
  for (auto& ds : m_editor.m_selectedGroup.datasets)
    ds.sourceId = m_editor.m_selectedGroup.sourceId;

  m_model.updateGroup(groupId, m_editor.m_selectedGroup, true);
}

/**
 * @brief Fans a group-level log-axis toggle onto every member dataset (the multiplot group
 *        combo's per-dataset encoding; read back from datasets.front()).
 */
void EditorCommit::applyGroupLogAxisEdit(bool xAxis, bool enabled, int groupId)
{
  for (auto& dataset : m_editor.m_selectedGroup.datasets)
    if (xAxis)
      dataset.pltLogX = enabled;
    else
      dataset.pltLogY = enabled;

  m_model.updateGroup(groupId, m_editor.m_selectedGroup);
}

/**
 * @brief Applies the bar-panel orientation combo (0 = auto, 1 = horizontal, 2 = vertical).
 */
void EditorCommit::applyGroupBarPanelStyleEdit(int styleIdx, int groupId)
{
  static const QStringList kStyles = {
    QLatin1String(""), QStringLiteral("horizontal"), QStringLiteral("vertical")};

  m_editor.m_selectedGroup.barPanelStyle = kStyles.value(styleIdx);
  m_model.setNextUndoHint(tr("Edit Group"), QStringLiteral("group-barstyle:%1").arg(groupId));
  m_model.updateGroup(groupId, m_editor.m_selectedGroup, false);
}

/**
 * @brief Applies a group-widget change; returns false when the change is rejected.
 */
bool EditorCommit::applyGroupWidgetEdit(int widgetIdx, int groupId)
{
  const auto keys = m_editor.m_groupWidgets.keys();
  if (widgetIdx < 0 || widgetIdx >= keys.size())
    return false;

  const auto widgetStr = keys.at(widgetIdx);

  static auto& catalog = UI::WidgetExtensions::instance();
  if (catalog.contains(widgetStr)
      && catalog.descriptor(widgetStr).scope == UI::WidgetExtensions::GroupScope) {
    m_editor.m_selectedGroup.widget = widgetStr;
    m_model.updateGroup(groupId, m_editor.m_selectedGroup, true);
    return true;
  }

  static const QMap<QString, SerialStudio::GroupWidget> kWidgetEnumMap = {
    {"accelerometer", SerialStudio::Accelerometer},
    {    "multiplot",     SerialStudio::MultiPlot},
    {         "gyro",     SerialStudio::Gyroscope},
    {          "map",           SerialStudio::GPS},
    {     "datagrid",      SerialStudio::DataGrid},
    {     "barpanel",      SerialStudio::BarPanel},
    {       "plot3d",        SerialStudio::Plot3D},
    {        "image",     SerialStudio::ImageView},
    {      "painter",       SerialStudio::Painter},
    {      "webview",       SerialStudio::WebView},
    {             "", SerialStudio::NoGroupWidget},
  };

  const auto widget = kWidgetEnumMap.value(widgetStr, SerialStudio::NoGroupWidget);
  if (m_model.setGroupWidget(groupId, widget)) {
    m_editor.m_selectedGroup.widget = widgetStr;
    return true;
  }

  QTimer::singleShot(0, &m_editor, [this, groupId] {
    m_editor.m_tree.buildTreeModel();
    for (auto g = m_editor.m_groupItems.begin(); g != m_editor.m_groupItems.end(); ++g) {
      if (g.value().groupId != groupId)
        continue;

      if (m_editor.m_selectionModel)
        m_editor.m_selectionModel->setCurrentIndex(g.key()->index(),
                                                   QItemSelectionModel::ClearAndSelect);

      break;
    }
  });

  return false;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Applies an image-mode edit; returns true when handled (caller skips Q_EMIT).
 */
bool EditorCommit::applyGroupImgModeEdit(int modeIdx, int groupId)
{
  const QStringList kImgModeValues = {QStringLiteral("autodetect"), QStringLiteral("manual")};
  if (modeIdx < 0 || modeIdx >= kImgModeValues.size())
    return false;

  m_editor.m_selectedGroup.imgDetectionMode = kImgModeValues.at(modeIdx);
  m_model.updateGroup(groupId, m_editor.m_selectedGroup);
  m_editor.m_forms.buildGroupModel(m_editor.m_selectedGroup);
  return true;
}
#endif

/**
 * @brief Handles edits to the action form model.
 */
void EditorCommit::onActionItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  static QStringList eolKeys;
  if (eolKeys.isEmpty())
    for (auto i = m_editor.m_eolSequences.begin(); i != m_editor.m_eolSequences.end(); ++i)
      eolKeys.append(i.key());

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  switch (static_cast<ActionItem>(id.toInt())) {
    case kActionView_Title:
      m_editor.m_selectedAction.title = value.toString();
      break;
    case kActionView_Data:
      m_editor.m_selectedAction.txData = value.toString();
      break;
    case kActionView_EOL: {
      const int eolIdx = value.toInt();
      if (eolIdx < 0 || eolIdx >= eolKeys.size())
        return;

      m_editor.m_selectedAction.eolSequence = eolKeys.at(eolIdx);
      break;
    }
    case kActionView_Icon:
      m_editor.m_selectedAction.icon = value.toString();
      Q_EMIT m_editor.actionModelChanged();
      break;
    case kActionView_Binary:
      m_editor.m_selectedAction.binaryData = value.toBool();
      m_editor.m_forms.buildActionModel(m_editor.m_selectedAction);
      break;
    case kActionView_TxEncoding:
      m_editor.m_selectedAction.txEncoding = value.toInt();
      break;
    case kActionView_SourceId: {
      const auto& sources = m_model.sources();
      const int srcIdx    = value.toInt();
      if (srcIdx >= 0 && srcIdx < static_cast<int>(sources.size()))
        m_editor.m_selectedAction.sourceId = sources[srcIdx].sourceId;

      break;
    }
    case kActionView_AutoExecute:
      m_editor.m_selectedAction.autoExecuteOnConnect = value.toBool();
      break;
    case kActionView_TimerMode:
      m_editor.m_selectedAction.timerMode = static_cast<DataModel::TimerMode>(value.toInt());
      m_editor.m_forms.buildActionModel(m_editor.m_selectedAction);
      break;
    case kActionView_TimerInterval:
      m_editor.m_selectedAction.timerIntervalMs = value.toInt();
      break;
    case kActionView_RepeatCount:
      m_editor.m_selectedAction.repeatCount = qMax(1, value.toInt());
      break;
    default:
      break;
  }

  auto& pm            = m_model;
  const auto actionId = m_editor.m_selectedAction.actionId;
  pm.setSelectedAction(m_editor.m_selectedAction);
  pm.setNextUndoHint(
    tr("Edit Action"),
    QStringLiteral("action:%1:%2").arg(QString::number(actionId), QString::number(id.toInt())));
  pm.updateAction(actionId, m_editor.m_selectedAction, false);

  if (static_cast<ActionItem>(id.toInt()) == kActionView_Title) {
    const auto newTitle = value.toString();
    for (auto it = m_editor.m_actionItems.begin(); it != m_editor.m_actionItems.end(); ++it) {
      if (it.value().actionId != actionId)
        continue;

      auto* treeItem = it.key();
      treeItem->setText(newTitle);
      treeItem->setData(newTitle, TreeViewText);
      m_editor.m_actionItems[treeItem].title = newTitle;
      break;
    }

    Q_EMIT m_editor.selectedTextChanged();
  } else {
    for (auto it = m_editor.m_actionItems.begin(); it != m_editor.m_actionItems.end(); ++it) {
      if (it.value().actionId == actionId) {
        m_editor.m_actionItems[it.key()] = m_editor.m_selectedAction;
        break;
      }
    }
  }
}

/**
 * @brief Dispatches project-level form edits to ProjectModel.
 */
void EditorCommit::onProjectItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);
  auto& pm         = m_model;

  switch (static_cast<ProjectItem>(id.toInt())) {
    case kProjectView_Title:
      pm.setNextUndoHint(tr("Rename Project"), QStringLiteral("project-title"));
      pm.setTitle(value.toString());
      return;
    default:
      break;
  }
}

/**
 * @brief Returns true if @a alias is already assigned to a dataset other than @a selfUniqueId.
 */
bool EditorCommit::datasetAliasInUse(const QString& alias, int selfUniqueId) const
{
  for (const auto& group : m_model.groups()) {
    for (const auto& other : group.datasets)
      if (other.uniqueId != selfUniqueId && other.alias == alias)
        return true;
  }

  return false;
}

/**
 * @brief Debounced alias validation: the value always applies so typing is never interrupted.
 *        The alias field commits per keystroke, so a synchronous modal would fire mid-word
 *        ("temp" while typing "temp2"). Once the alias settles for a moment, a duplicate is
 *        reported and cleared, and an all-digit alias gets the scripting warning.
 */
bool EditorCommit::validateSelectedDatasetAlias(const QString& newAlias)
{
  if (newAlias.isEmpty())
    return true;

  const int uid           = m_editor.m_selectedDataset.uniqueId;
  const QString candidate = newAlias;
  QTimer::singleShot(800, &m_editor, [this, uid, candidate] {
    if (m_editor.m_selectedDataset.uniqueId != uid || m_editor.m_selectedDataset.alias != candidate)
      return;

    if (datasetAliasInUse(candidate, uid)) {
      m_editor.m_selectedDataset.alias.clear();
      m_model.updateDataset(m_editor.m_selectedDataset.groupId,
                            m_editor.m_selectedDataset.datasetId,
                            m_editor.m_selectedDataset,
                            false);
      m_editor.m_forms.buildDatasetModel(m_editor.m_selectedDataset);
      Core::Prompt::showMessageBox(
        tr("Alias \"%1\" is already in use").arg(candidate),
        tr("Dataset aliases must be unique across the project. The change was not applied."),
        Core::Prompt::Warning,
        tr("Duplicate Alias"));
      return;
    }

    bool allDigits = true;
    for (const QChar c : candidate)
      if (!c.isDigit())
        allDigits = false;

    if (allDigits)
      Core::Prompt::showMessageBox(
        tr("Alias \"%1\" contains only digits").arg(candidate),
        tr("Scripts must quote it as a string, e.g. getDataset(\"%1\"); a numeric argument is "
           "read as a uniqueId, not &m_editor alias.")
          .arg(candidate),
        Core::Prompt::Information,
        tr("Numeric Alias"));
  });

  return true;
}

/**
 * @brief Commits the result of the AlarmBandsEditor dialog into the currently-selected dataset.
 */
void EditorCommit::commitAlarmBands(const QVariantList& bands)
{
  const auto parsed = parseAlarmBandList(bands);
  if (m_editor.m_currentView == MultiSelectionView
      && m_editor.m_batchKind == ProjectEditor::KindDataset) {
    commitAlarmBandsForSelection(parsed);
    return;
  }

  auto& pm                              = m_model;
  m_editor.m_selectedDataset.alarmBands = parsed;
  pm.updateDataset(m_editor.m_selectedDataset.groupId,
                   m_editor.m_selectedDataset.datasetId,
                   m_editor.m_selectedDataset,
                   false);
  m_editor.m_forms.buildDatasetModel(m_editor.m_selectedDataset);
}

/**
 * @brief Writes @p bands onto every dataset in the current multi-selection, as one modified state
 *        and one autosave, then rebuilds the aggregate model.
 */
void EditorCommit::commitAlarmBandsForSelection(const std::vector<DataModel::AlarmBand>& bands)
{
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

  const ProjectUndoFrame undo_frame{pm, tr("Edit Alarms")};
  pm.setAutoSaveSuspended(true);
  for (int i = 0; i < sel.size(); ++i) {
    DataModel::Dataset ds = sel[i];
    ds.alarmBands         = bands;
    pm.updateDataset(ids[i].first, ids[i].second, ds, false);
  }
  pm.setAutoSaveSuspended(false);

  m_editor.m_multiSelect.buildMultiDatasetModel();
  pm.flushAutoSave();
}

/**
 * @brief Commits the FrequencyMarkersEditor result into the currently-selected dataset; entries
 *        are validated with the same rules as the JSON reader (positive finite frequency wins).
 */
void EditorCommit::commitFrequencyMarkers(const QVariantList& markers)
{
  constexpr double nan        = std::numeric_limits<double>::quiet_NaN();
  constexpr double max_freqHz = 2147483648.0;

  m_editor.m_selectedDataset.fftMarkers.clear();
  m_editor.m_selectedDataset.fftMarkers.reserve(markers.size());
  for (const auto& v : markers) {
    const auto m = v.toMap();
    DataModel::FrequencyMarker marker;
    marker.frequency    = SerialStudio::toDouble(m.value(QStringLiteral("freq")));
    marker.endFrequency = SerialStudio::toDouble(m.value(QStringLiteral("endFreq")));
    marker.label        = m.value(QStringLiteral("label")).toString().simplified();
    marker.color        = m.value(QStringLiteral("color")).toString().simplified();

    const auto warning = m.value(QStringLiteral("warningDb"));
    const auto alarm   = m.value(QStringLiteral("alarmDb"));
    marker.warningDb   = warning.isValid() ? SerialStudio::toDouble(warning) : nan;
    marker.alarmDb     = alarm.isValid() ? SerialStudio::toDouble(alarm) : nan;

    if (!std::isfinite(marker.frequency) || marker.frequency <= 0.0
        || marker.frequency > max_freqHz)
      continue;

    if (!std::isfinite(marker.endFrequency) || marker.endFrequency <= marker.frequency)
      marker.endFrequency = 0.0;
    else
      marker.endFrequency = qMin(marker.endFrequency, max_freqHz);

    if (std::isfinite(marker.warningDb) && std::isfinite(marker.alarmDb)
        && marker.warningDb > marker.alarmDb)
      std::swap(marker.warningDb, marker.alarmDb);

    m_editor.m_selectedDataset.fftMarkers.push_back(std::move(marker));
  }

  auto& pm = m_model;
  pm.updateDataset(m_editor.m_selectedDataset.groupId,
                   m_editor.m_selectedDataset.datasetId,
                   m_editor.m_selectedDataset,
                   false);
  m_editor.m_forms.buildDatasetModel(m_editor.m_selectedDataset);
}

/**
 * @brief Rejects a dataset form edit before it reaches the registry: an out-of-range combo index
 *        (which would otherwise fall back to the domain's first entry) or a duplicate alias.
 */
bool EditorCommit::datasetFormEditAccepted(int formId, const QVariant& value)
{
  const int index = value.toInt();
  if (formId == kDatasetView_Widget)
    return index >= 0 && index < m_editor.m_datasetWidgets.size();

  if (formId == kDatasetView_Plot)
    return index >= 0 && index < m_editor.m_plotOptions.size();

  if (formId == kDatasetView_DisplayFormat)
    return index >= 0 && index < m_editor.m_displayFormats.size();

  if (formId == kDatasetView_FFT_Samples)
    return index >= 0 && index < m_editor.m_fftSamples.size();

  if (formId == kDatasetView_FFT_Window)
    return index >= 0 && index < m_editor.m_fftWindowValues.size();

  if (formId == kDatasetView_Alias)
    return validateSelectedDatasetAlias(value.toString().simplified());

  return true;
}

/**
 * @brief Repaints a dataset's tree glyph in place after an edit changed its widgets; the
 *        multi-selection paths use it because a tree rebuild would drop the selection.
 */
void EditorCommit::syncDatasetTreeIcon(const DataModel::Dataset& dataset)
{
  for (auto it = m_editor.m_datasetItems.begin(); it != m_editor.m_datasetItems.end(); ++it) {
    if (it.value().groupId != dataset.groupId || it.value().datasetId != dataset.datasetId)
      continue;

    it.key()->setData(EditorTree::datasetTreeIcon(dataset), TreeViewIcon);
    it.value() = dataset;
    break;
  }
}

/**
 * @brief Mirrors a dataset's virtual flag onto its tree item, which paints a different badge.
 */
void EditorCommit::syncDatasetTreeVirtualFlag(const DataModel::Dataset& dataset)
{
  for (auto it = m_editor.m_datasetItems.begin(); it != m_editor.m_datasetItems.end(); ++it) {
    if (it.value().groupId != dataset.groupId || it.value().datasetId != dataset.datasetId)
      continue;

    it.key()->setData(dataset.virtual_, TreeViewVirtual);
    break;
  }
}

/**
 * @brief Routes one applied dataset edit through the undo choke point, taking the coalesce key
 *        and the tree-rebuild flag from the registry descriptor of the edited property.
 */
void EditorCommit::commitDatasetFormEdit(int formId)
{
  auto& pm             = m_model;
  const auto* prop     = Registry::datasetPropertyForFormId(formId);
  const auto groupId   = m_editor.m_selectedDataset.groupId;
  const auto datasetId = m_editor.m_selectedDataset.datasetId;
  const bool coalesces = prop && prop->coalesce;
  const QString prefix = coalesces ? QString::fromLatin1(prop->coalesceKey) : QString();
  const QString key =
    coalesces
      ? QStringLiteral("%1:%2:%3:%4")
          .arg(
            prefix, QString::number(groupId), QString::number(datasetId), QString::number(formId))
      : QString();

  if (formId == kDatasetView_Title) {
    const auto newTitle = m_editor.m_selectedDataset.title;
    pm.setNextUndoHint(tr("Rename Dataset"), key);
    pm.updateDataset(groupId, datasetId, m_editor.m_selectedDataset, false);

    for (auto it = m_editor.m_datasetItems.begin(); it != m_editor.m_datasetItems.end(); ++it) {
      if (it.value().groupId != groupId || it.value().datasetId != datasetId)
        continue;

      auto* treeItem = it.key();
      treeItem->setText(newTitle);
      treeItem->setData(newTitle, TreeViewText);
      m_editor.m_datasetItems[treeItem].title = newTitle;
      break;
    }

    Q_EMIT m_editor.selectedTextChanged();
    Q_EMIT m_editor.datasetOptionsChanged();
    Q_EMIT m_editor.editableOptionsChanged();
    return;
  }

  const bool rebuildTree = prop && prop->rebuildTree;
  pm.setNextUndoHint(tr("Edit Dataset"), key);
  pm.updateDataset(groupId, datasetId, m_editor.m_selectedDataset, rebuildTree);
  if (!rebuildTree)
    syncDatasetItemCache(groupId, datasetId);

  Q_EMIT m_editor.datasetOptionsChanged();
  Q_EMIT m_editor.editableOptionsChanged();
}

/**
 * @brief Dispatches dataset form edits to ProjectModel, rebuilding only on tree-visible changes.
 */
void EditorCommit::onDatasetItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto idInt = static_cast<DatasetItem>(item->data(ParameterType).toInt());
  const auto value = item->data(EditableValue);
  const int formId = static_cast<int>(idInt);
  if (!datasetFormEditAccepted(formId, value))
    return;

  const auto hint =
    Registry::applyDatasetFormEdit(formId, value, m_editor.m_selectedDataset, m_model);
  if (idInt == kDatasetView_Virtual)
    syncDatasetTreeVirtualFlag(m_editor.m_selectedDataset);

  const bool rebuildNow = !m_editor.m_batchApplying && hint == PropertyHooks::RebuildHint::Sync;
  const bool rebuildLater =
    !m_editor.m_batchApplying && hint == PropertyHooks::RebuildHint::Deferred;
  if (rebuildNow)
    m_editor.m_forms.buildDatasetModel(m_editor.m_selectedDataset);

  if (rebuildLater) {
    const int uid = m_editor.m_selectedDataset.uniqueId;
    QTimer::singleShot(0, &m_editor, [this, uid] {
      if (m_editor.m_selectedDataset.uniqueId == uid)
        m_editor.m_forms.buildDatasetModel(m_editor.m_selectedDataset);
    });
  }

  commitDatasetFormEdit(formId);
}

/**
 * @brief Refreshes the cached dataset record bound to the matching tree item.
 */
void EditorCommit::syncDatasetItemCache(int groupId, int datasetId)
{
  for (auto it = m_editor.m_datasetItems.begin(); it != m_editor.m_datasetItems.end(); ++it) {
    if (it.value().groupId != groupId || it.value().datasetId != datasetId)
      continue;

    m_editor.m_datasetItems[it.key()] = m_editor.m_selectedDataset;
    break;
  }
}

/**
 * @brief Applies one output-widget form field from @p item onto @p widget; pure field mutation with
 *        no side effects, shared by the single-selection and multi-selection edit paths.
 */
void EditorCommit::applyOutputWidgetField(QStandardItem* item, DataModel::OutputWidget& widget)
{
  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  switch (static_cast<OutputWidgetItem>(id.toInt())) {
    case kOutputWidget_Title:
      widget.title = value.toString();
      break;
    case kOutputWidget_Icon:
      widget.icon = value.toString();
      break;
    case kOutputWidget_MonoIcon:
      widget.monoIcon = value.toBool();
      break;
    case kOutputWidget_Checkable:
      widget.checkable = value.toBool();
      break;
    case kOutputWidget_Color:
      widget.color = value.toString();
      break;
    case kOutputWidget_Size:
      widget.size = static_cast<DataModel::OutputWidgetSize>(
        qBound(0, value.toInt(), static_cast<int>(DataModel::OutputWidgetSize::ExtraLarge)));
      break;
    case kOutputWidget_OnLabel:
      widget.onLabel = value.toString();
      break;
    case kOutputWidget_OffLabel:
      widget.offLabel = value.toString();
      break;
    case kOutputWidget_Type:
      widget.type = static_cast<DataModel::OutputWidgetType>(value.toInt());
      break;
    case kOutputWidget_StateSource:
      widget.stateSource = static_cast<DataModel::OutputStateSource>(
        qBound(0, value.toInt(), static_cast<int>(DataModel::OutputStateSource::Table)));
      DataModel::applyOutputStateTarget(widget, m_model, 0);
      break;
    case kOutputWidget_StateTarget:
      DataModel::applyOutputStateTarget(widget, m_model, value.toInt());
      break;
    case kOutputWidget_StateOnValue:
      widget.stateOnValue = value.toString();
      break;
    case kOutputWidget_StateConfirmMs:
      widget.stateConfirmMs = qBound(0, value.toInt(), DataModel::kMaxOutputStateConfirmMs);
      break;
    case kOutputWidget_MinValue:
      widget.minValue = SerialStudio::toDouble(value);
      break;
    case kOutputWidget_MaxValue:
      widget.maxValue = SerialStudio::toDouble(value);
      break;
    case kOutputWidget_StepSize:
      widget.stepSize = SerialStudio::toDouble(value);
      break;
    case kOutputWidget_InitialValue:
      widget.initialValue = SerialStudio::toDouble(value);
      break;
    case kOutputWidget_TransmitFunction:
      widget.transmitFunction = value.toString();
      break;
    case kOutputWidget_TxEncoding:
      widget.txEncoding = value.toInt();
      break;
  }
}

/**
 * @brief Handles changes to output widget form fields.
 */
void EditorCommit::onOutputWidgetItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);
  const auto param = static_cast<OutputWidgetItem>(id.toInt());

  if (param == kOutputWidget_Type) {
    const auto newType = static_cast<DataModel::OutputWidgetType>(value.toInt());
    if (m_editor.m_selectedOutputWidget.type != newType) {
      m_editor.m_selectedOutputWidget.type = newType;
      m_editor.m_forms.buildOutputWidgetModel(m_editor.m_selectedOutputWidget);
    }
  } else {
    applyOutputWidgetField(item, m_editor.m_selectedOutputWidget);
    if (param == kOutputWidget_Checkable || param == kOutputWidget_StateSource)
      m_editor.m_forms.buildOutputWidgetModel(m_editor.m_selectedOutputWidget);
  }

  if (param == kOutputWidget_Title) {
    const auto newTitle = value.toString();
    for (auto it = m_editor.m_outputWidgetItems.begin(); it != m_editor.m_outputWidgetItems.end();
         ++it) {
      if (it.value().groupId == m_editor.m_selectedOutputWidget.groupId
          && it.value().widgetId == m_editor.m_selectedOutputWidget.widgetId) {
        it.key()->setData(newTitle, TreeViewText);
        m_editor.m_outputWidgetItems[it.key()].title = newTitle;
        Q_EMIT m_editor.selectedTextChanged();
        break;
      }
    }
  } else {
    for (auto it = m_editor.m_outputWidgetItems.begin(); it != m_editor.m_outputWidgetItems.end();
         ++it) {
      if (it.value().groupId == m_editor.m_selectedOutputWidget.groupId
          && it.value().widgetId == m_editor.m_selectedOutputWidget.widgetId) {
        m_editor.m_outputWidgetItems[it.key()] = m_editor.m_selectedOutputWidget;
        break;
      }
    }
  }

  m_model.setNextUndoHint(tr("Edit Output Widget"),
                          QStringLiteral("owidget:%1:%2:%3")
                            .arg(QString::number(m_editor.m_selectedOutputWidget.groupId),
                                 QString::number(m_editor.m_selectedOutputWidget.widgetId),
                                 QString::number(id.toInt())));
  m_model.updateOutputWidget(m_editor.m_selectedOutputWidget.groupId,
                             m_editor.m_selectedOutputWidget.widgetId,
                             m_editor.m_selectedOutputWidget,
                             false);
}

}  // namespace DataModel
