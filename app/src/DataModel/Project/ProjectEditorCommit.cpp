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

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QMessageBox>
#include <QSet>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "UI/WidgetExtensions.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#  include "MQTT/PublisherScriptEditor.h"
#endif
#include "ProjectEditorItemIds.h"
#include "ProjectEditorShared.h"

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
void DataModel::ProjectEditor::handleSourceTitleChange(QStandardItem* item)
{
  const QString newTitle = item->data(EditableValue).toString();
  if (m_selectedSource.title == newTitle)
    return;

  m_selectedSource.title = newTitle;
  m_projectModelRef.setNextUndoHint(
    tr("Rename Device"), QStringLiteral("source-title:%1").arg(m_selectedSource.sourceId));
  m_projectModelRef.updateSourceTitle(m_selectedSource.sourceId, newTitle, false);

  for (auto it = m_sourceItems.begin(); it != m_sourceItems.end(); ++it) {
    if (it.value().sourceId != m_selectedSource.sourceId)
      continue;

    auto* treeItem = it.key();
    treeItem->setText(newTitle);
    treeItem->setData(newTitle, TreeViewText);
    m_sourceItems[treeItem].title = newTitle;
    break;
  }

  Q_EMIT selectedTextChanged();
}

/**
 * @brief Applies a bus-type edit and rebuilds the source form once contexts are ready.
 */
void DataModel::ProjectEditor::handleSourceBusTypeChange(QStandardItem* item)
{
  const int busType = item->data(EditableValue).toInt();
  m_projectModelRef.updateSourceBusType(m_selectedSource.sourceId, busType);
  m_selectedSource.busType = busType;
  auto conn                = std::make_shared<QMetaObject::Connection>();
  *conn                    = connect(
    &m_connectionManager,
    &IO::ConnectionManager::contextsRebuilt,
    this,
    [this, conn] {
      disconnect(*conn);
      buildSourceModel(m_selectedSource);
    },
    Qt::QueuedConnection);
}

/**
 * @brief Applies a live-driver property edit and rebuilds the form on transport-mode or
 *        audio-input-device changes, so dependent option lists never go stale.
 */
void DataModel::ProjectEditor::handleSourcePropertyChange(QStandardItem* item)
{
  const QString key   = item->data(ParameterKey).toString();
  const QVariant val  = item->data(EditableValue);
  IO::HAL_Driver* drv = m_connectionManager.driverForEditing(m_selectedSource.sourceId);
  if (drv)
    drv->setDriverProperty(key, val);

  m_projectModelRef.captureSourceSettings(m_selectedSource.sourceId);

  static const QStringList kModeKeys = {
    QStringLiteral("socketTypeIndex"),
    QStringLiteral("protocolIndex"),
    QStringLiteral("inputDevice"),
  };
  if (kModeKeys.contains(key))
    buildSourceModel(m_selectedSource);
}

/**
 * @brief Dispatches source form edits to ProjectModel or the live driver.
 */
void DataModel::ProjectEditor::onSourceItemChanged(QStandardItem* item)
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

  DataModel::Source updated = m_selectedSource;
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
void DataModel::ProjectEditor::handleSourceFrameDetectionChange(QStandardItem* item,
                                                                DataModel::Source& updated)
{
  const int id  = item->data(ParameterType).toInt();
  const int sid = m_selectedSource.sourceId;

  if (id == kSourceView_FrameDetection) {
    const int idx = item->data(EditableValue).toInt();
    if (idx < 0 || idx >= m_frameDetectionMethodsValues.size())
      return;

    updated.frameDetection = static_cast<int>(m_frameDetectionMethodsValues.at(idx));
  } else {
    updated.hexadecimalDelimiters = item->data(EditableValue).toBool();
  }

  m_projectModelRef.updateSource(sid, updated);
  m_selectedSource = updated;

  buildSourceModel(m_selectedSource);
}

/**
 * @brief Applies a frame start/end delimiter edit to the source.
 */
void DataModel::ProjectEditor::handleSourceFrameStartEndChange(QStandardItem* item,
                                                               DataModel::Source& updated)
{
  const int id  = item->data(ParameterType).toInt();
  const int sid = m_selectedSource.sourceId;

  if (id == kSourceView_FrameStartSequence)
    updated.frameStart = item->data(EditableValue).toString();
  else
    updated.frameEnd = item->data(EditableValue).toString();

  m_projectModelRef.setNextUndoHint(
    tr("Edit Device"),
    QStringLiteral("source-frame:%1:%2").arg(QString::number(sid), QString::number(id)));
  m_projectModelRef.updateSource(sid, updated, false);
  m_selectedSource = updated;
}

/**
 * @brief Applies a decoder-method or checksum-algorithm edit to the source.
 */
void DataModel::ProjectEditor::handleSourceDecoderChecksumChange(QStandardItem* item,
                                                                 DataModel::Source& updated)
{
  const int id  = item->data(ParameterType).toInt();
  const int sid = m_selectedSource.sourceId;

  if (id == kSourceView_FrameDecoder) {
    updated.decoderMethod = item->data(EditableValue).toInt();
    m_projectModelRef.updateSource(sid, updated);
    m_selectedSource = updated;
    return;
  }

  const auto checksums = IO::availableChecksums();
  const int checksumId = item->data(EditableValue).toInt();
  if (checksumId < 0 || checksumId >= checksums.size())
    return;

  updated.checksumAlgorithm = checksums.at(checksumId);
  m_projectModelRef.updateSource(sid, updated);
  m_selectedSource = updated;
}

//--------------------------------------------------------------------------------------------------
// Private slot: item changed handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Propagates group form edits to ProjectModel and the tree.
 */
void DataModel::ProjectEditor::onGroupItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id      = static_cast<GroupItem>(item->data(ParameterType).toInt());
  const auto value   = item->data(EditableValue);
  auto& pm           = m_projectModelRef;
  const auto groupId = m_selectedGroup.groupId;

  if (id == kGroupView_Dataset) {
    const int datasetId = item->data(ParameterKey).toInt();
    QTimer::singleShot(0, this, [this, groupId, datasetId] { selectDataset(groupId, datasetId); });
    return;
  }

  if (id == kGroupView_Title) {
    if (!applyGroupTitleEdit(value.toString(), groupId))
      return;

    Q_EMIT editableOptionsChanged();
    return;
  }

  if (id == kGroupView_Source) {
    applyGroupSourceEdit(value.toInt(), groupId);
    Q_EMIT editableOptionsChanged();
    return;
  }

  if (id == kGroupView_Widget) {
    if (!applyGroupWidgetEdit(value.toInt(), groupId))
      return;

    Q_EMIT editableOptionsChanged();
    return;
  }

  if (id == kGroupView_xAxis) {
    const int xAxisId = (value.toInt() == 1) ? kXAxisSamples : kXAxisTime;
    for (auto& dataset : m_selectedGroup.datasets)
      dataset.xAxisId = xAxisId;

    pm.updateGroup(groupId, m_selectedGroup);
    Q_EMIT editableOptionsChanged();
    return;
  }

  if (id == kGroupView_LogX || id == kGroupView_LogY) {
    applyGroupLogAxisEdit(id == kGroupView_LogX, value.toBool(), groupId);
    Q_EMIT editableOptionsChanged();
    return;
  }

  if (id == kGroupView_WebUrl) {
    m_selectedGroup.webViewUrl = value.toString();
    pm.setNextUndoHint(tr("Edit Group"), QStringLiteral("group-weburl:%1").arg(groupId));
    pm.updateGroup(groupId, m_selectedGroup, false);
    Q_EMIT editableOptionsChanged();
    return;
  }

  if (id == kGroupView_BarPanelStyle) {
    applyGroupBarPanelStyleEdit(value.toInt(), groupId);
    Q_EMIT editableOptionsChanged();
    return;
  }

#ifdef BUILD_COMMERCIAL
  if (id == kGroupView_ImgMode) {
    if (applyGroupImgModeEdit(value.toInt(), groupId))
      return;

    Q_EMIT editableOptionsChanged();
    return;
  }

  if (id == kGroupView_ImgStart) {
    m_selectedGroup.imgStartSequence = value.toString();
    pm.setNextUndoHint(tr("Edit Group"), QStringLiteral("group-imgstart:%1").arg(groupId));
    pm.updateGroup(groupId, m_selectedGroup, false);
  }

  if (id == kGroupView_ImgEnd) {
    m_selectedGroup.imgEndSequence = value.toString();
    pm.setNextUndoHint(tr("Edit Group"), QStringLiteral("group-imgend:%1").arg(groupId));
    pm.updateGroup(groupId, m_selectedGroup, false);
  }
#endif

  Q_EMIT editableOptionsChanged();
}

/**
 * @brief Applies a group-title edit; returns false when the title is unchanged.
 */
bool DataModel::ProjectEditor::applyGroupTitleEdit(const QString& newTitle, int groupId)
{
  if (m_selectedGroup.title == newTitle)
    return false;

  m_selectedGroup.title = newTitle;
  m_projectModelRef.setNextUndoHint(tr("Rename Group"),
                                    QStringLiteral("group-title:%1").arg(groupId));
  m_projectModelRef.updateGroup(groupId, m_selectedGroup, false);

  for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
    if (it.value().groupId != groupId)
      continue;

    auto* treeItem = it.key();
    treeItem->setText(newTitle);
    treeItem->setData(newTitle, TreeViewText);
    m_groupItems[treeItem].title = newTitle;
    break;
  }

  Q_EMIT selectedTextChanged();
  return true;
}

/**
 * @brief Re-routes the group (and its datasets) to the source at the given combobox index.
 */
void DataModel::ProjectEditor::applyGroupSourceEdit(int srcIdx, int groupId)
{
  const auto& sources = m_projectModelRef.sources();
  if (srcIdx < 0 || srcIdx >= static_cast<int>(sources.size()))
    return;

  m_selectedGroup.sourceId = sources[srcIdx].sourceId;
  for (auto& ds : m_selectedGroup.datasets)
    ds.sourceId = m_selectedGroup.sourceId;

  m_projectModelRef.updateGroup(groupId, m_selectedGroup, true);
}

/**
 * @brief Fans a group-level log-axis toggle onto every member dataset (the multiplot group
 *        combo's per-dataset encoding; read back from datasets.front()).
 */
void DataModel::ProjectEditor::applyGroupLogAxisEdit(bool xAxis, bool enabled, int groupId)
{
  for (auto& dataset : m_selectedGroup.datasets)
    if (xAxis)
      dataset.pltLogX = enabled;
    else
      dataset.pltLogY = enabled;

  m_projectModelRef.updateGroup(groupId, m_selectedGroup);
}

/**
 * @brief Applies the bar-panel orientation combo (0 = auto, 1 = horizontal, 2 = vertical).
 */
void DataModel::ProjectEditor::applyGroupBarPanelStyleEdit(int styleIdx, int groupId)
{
  static const QStringList kStyles = {
    QLatin1String(""), QStringLiteral("horizontal"), QStringLiteral("vertical")};

  m_selectedGroup.barPanelStyle = kStyles.value(styleIdx);
  m_projectModelRef.setNextUndoHint(tr("Edit Group"),
                                    QStringLiteral("group-barstyle:%1").arg(groupId));
  m_projectModelRef.updateGroup(groupId, m_selectedGroup, false);
}

/**
 * @brief Applies a group-widget change; returns false when the change is rejected.
 */
bool DataModel::ProjectEditor::applyGroupWidgetEdit(int widgetIdx, int groupId)
{
  const auto keys = m_groupWidgets.keys();
  if (widgetIdx < 0 || widgetIdx >= keys.size())
    return false;

  const auto widgetStr = keys.at(widgetIdx);

  static auto& catalog = UI::WidgetExtensions::instance();
  if (catalog.contains(widgetStr)
      && catalog.descriptor(widgetStr).scope == UI::WidgetExtensions::GroupScope) {
    m_selectedGroup.widget = widgetStr;
    m_projectModelRef.updateGroup(groupId, m_selectedGroup, true);
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
  if (m_projectModelRef.setGroupWidget(groupId, widget)) {
    m_selectedGroup.widget = widgetStr;
    return true;
  }

  QTimer::singleShot(0, this, [this, groupId] {
    buildTreeModel();
    for (auto g = m_groupItems.begin(); g != m_groupItems.end(); ++g) {
      if (g.value().groupId != groupId)
        continue;

      if (m_selectionModel)
        m_selectionModel->setCurrentIndex(g.key()->index(), QItemSelectionModel::ClearAndSelect);

      break;
    }
  });

  return false;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Applies an image-mode edit; returns true when handled (caller skips Q_EMIT).
 */
bool DataModel::ProjectEditor::applyGroupImgModeEdit(int modeIdx, int groupId)
{
  const QStringList kImgModeValues = {QStringLiteral("autodetect"), QStringLiteral("manual")};
  if (modeIdx < 0 || modeIdx >= kImgModeValues.size())
    return false;

  m_selectedGroup.imgDetectionMode = kImgModeValues.at(modeIdx);
  m_projectModelRef.updateGroup(groupId, m_selectedGroup);
  buildGroupModel(m_selectedGroup);
  return true;
}
#endif

/**
 * @brief Handles edits to the action form model.
 */
void DataModel::ProjectEditor::onActionItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  static QStringList eolKeys;
  if (eolKeys.isEmpty())
    for (auto i = m_eolSequences.begin(); i != m_eolSequences.end(); ++i)
      eolKeys.append(i.key());

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  switch (static_cast<ActionItem>(id.toInt())) {
    case kActionView_Title:
      m_selectedAction.title = value.toString();
      break;
    case kActionView_Data:
      m_selectedAction.txData = value.toString();
      break;
    case kActionView_EOL: {
      const int eolIdx = value.toInt();
      if (eolIdx < 0 || eolIdx >= eolKeys.size())
        return;

      m_selectedAction.eolSequence = eolKeys.at(eolIdx);
      break;
    }
    case kActionView_Icon:
      m_selectedAction.icon = value.toString();
      Q_EMIT actionModelChanged();
      break;
    case kActionView_Binary:
      m_selectedAction.binaryData = value.toBool();
      buildActionModel(m_selectedAction);
      break;
    case kActionView_TxEncoding:
      m_selectedAction.txEncoding = value.toInt();
      break;
    case kActionView_SourceId: {
      const auto& sources = m_projectModelRef.sources();
      const int srcIdx    = value.toInt();
      if (srcIdx >= 0 && srcIdx < static_cast<int>(sources.size()))
        m_selectedAction.sourceId = sources[srcIdx].sourceId;

      break;
    }
    case kActionView_AutoExecute:
      m_selectedAction.autoExecuteOnConnect = value.toBool();
      break;
    case kActionView_TimerMode:
      m_selectedAction.timerMode = static_cast<DataModel::TimerMode>(value.toInt());
      buildActionModel(m_selectedAction);
      break;
    case kActionView_TimerInterval:
      m_selectedAction.timerIntervalMs = value.toInt();
      break;
    case kActionView_RepeatCount:
      m_selectedAction.repeatCount = qMax(1, value.toInt());
      break;
    default:
      break;
  }

  auto& pm            = m_projectModelRef;
  const auto actionId = m_selectedAction.actionId;
  pm.setSelectedAction(m_selectedAction);
  pm.setNextUndoHint(
    tr("Edit Action"),
    QStringLiteral("action:%1:%2").arg(QString::number(actionId), QString::number(id.toInt())));
  pm.updateAction(actionId, m_selectedAction, false);

  if (static_cast<ActionItem>(id.toInt()) == kActionView_Title) {
    const auto newTitle = value.toString();
    for (auto it = m_actionItems.begin(); it != m_actionItems.end(); ++it) {
      if (it.value().actionId != actionId)
        continue;

      auto* treeItem = it.key();
      treeItem->setText(newTitle);
      treeItem->setData(newTitle, TreeViewText);
      m_actionItems[treeItem].title = newTitle;
      break;
    }

    Q_EMIT selectedTextChanged();
  } else {
    for (auto it = m_actionItems.begin(); it != m_actionItems.end(); ++it) {
      if (it.value().actionId == actionId) {
        m_actionItems[it.key()] = m_selectedAction;
        break;
      }
    }
  }
}

/**
 * @brief Dispatches project-level form edits to ProjectModel.
 */
void DataModel::ProjectEditor::onProjectItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);
  auto& pm         = m_projectModelRef;

  switch (static_cast<ProjectItem>(id.toInt())) {
    case kProjectView_Title:
      pm.setNextUndoHint(tr("Rename Project"), QStringLiteral("project-title"));
      pm.setTitle(value.toString());
      break;
    default:
      break;
  }

  pm.setModified(true);
}

/**
 * @brief Returns true if @a alias is already assigned to a dataset other than @a selfUniqueId.
 */
bool DataModel::ProjectEditor::datasetAliasInUse(const QString& alias, int selfUniqueId) const
{
  for (const auto& group : m_projectModelRef.groups()) {
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
bool DataModel::ProjectEditor::validateSelectedDatasetAlias(const QString& newAlias)
{
  if (newAlias.isEmpty())
    return true;

  const int uid           = m_selectedDataset.uniqueId;
  const QString candidate = newAlias;
  QTimer::singleShot(800, this, [this, uid, candidate] {
    if (m_selectedDataset.uniqueId != uid || m_selectedDataset.alias != candidate)
      return;

    if (datasetAliasInUse(candidate, uid)) {
      m_selectedDataset.alias.clear();
      m_projectModelRef.updateDataset(
        m_selectedDataset.groupId, m_selectedDataset.datasetId, m_selectedDataset, false);
      buildDatasetModel(m_selectedDataset);
      Misc::Utilities::showMessageBox(
        tr("Alias \"%1\" is already in use").arg(candidate),
        tr("Dataset aliases must be unique across the project. The change was not applied."),
        QMessageBox::Warning,
        tr("Duplicate Alias"));
      return;
    }

    bool allDigits = true;
    for (const QChar c : candidate)
      if (!c.isDigit())
        allDigits = false;

    if (allDigits)
      Misc::Utilities::showMessageBox(
        tr("Alias \"%1\" contains only digits").arg(candidate),
        tr("Scripts must quote it as a string, e.g. getDataset(\"%1\"); a numeric argument is "
           "read as a uniqueId, not this alias.")
          .arg(candidate),
        QMessageBox::Information,
        tr("Numeric Alias"));
  });

  return true;
}

/**
 * @brief Commits the result of the AlarmBandsEditor dialog into the currently-selected dataset.
 */
void DataModel::ProjectEditor::commitAlarmBands(const QVariantList& bands)
{
  const auto parsed = parseAlarmBandList(bands);
  if (m_currentView == MultiSelectionView && m_batchKind == KindDataset) {
    commitAlarmBandsForSelection(parsed);
    return;
  }

  auto& pm                     = m_projectModelRef;
  m_selectedDataset.alarmBands = parsed;
  pm.updateDataset(
    m_selectedDataset.groupId, m_selectedDataset.datasetId, m_selectedDataset, false);
  buildDatasetModel(m_selectedDataset);
}

/**
 * @brief Writes @p bands onto every dataset in the current multi-selection, as one modified state
 *        and one autosave, then rebuilds the aggregate model.
 */
void DataModel::ProjectEditor::commitAlarmBandsForSelection(
  const std::vector<DataModel::AlarmBand>& bands)
{
  auto& pm = m_projectModelRef;

  QVector<DataModel::Dataset> sel;
  QVector<QPair<int, int>> ids;
  {
    const auto& groups = pm.groups();
    for (const auto& pr : m_batchItems) {
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

  buildMultiDatasetModel();
  pm.flushAutoSave();
}

/**
 * @brief Commits the FrequencyMarkersEditor result into the currently-selected dataset; entries
 *        are validated with the same rules as the JSON reader (positive finite frequency wins).
 */
void DataModel::ProjectEditor::commitFrequencyMarkers(const QVariantList& markers)
{
  constexpr double nan        = std::numeric_limits<double>::quiet_NaN();
  constexpr double max_freqHz = 2147483648.0;

  m_selectedDataset.fftMarkers.clear();
  m_selectedDataset.fftMarkers.reserve(markers.size());
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

    m_selectedDataset.fftMarkers.push_back(std::move(marker));
  }

  auto& pm = m_projectModelRef;
  pm.updateDataset(
    m_selectedDataset.groupId, m_selectedDataset.datasetId, m_selectedDataset, false);
  buildDatasetModel(m_selectedDataset);
}

/**
 * @brief Rejects a dataset form edit before it reaches the registry: an out-of-range combo index
 *        (which would otherwise fall back to the domain's first entry) or a duplicate alias.
 */
bool DataModel::ProjectEditor::datasetFormEditAccepted(int formId, const QVariant& value)
{
  const int index = value.toInt();
  if (formId == kDatasetView_Widget)
    return index >= 0 && index < m_datasetWidgets.size();

  if (formId == kDatasetView_Plot)
    return index >= 0 && index < m_plotOptions.size();

  if (formId == kDatasetView_DisplayFormat)
    return index >= 0 && index < m_displayFormats.size();

  if (formId == kDatasetView_FFT_Samples)
    return index >= 0 && index < m_fftSamples.size();

  if (formId == kDatasetView_FFT_Window)
    return index >= 0 && index < m_fftWindowValues.size();

  if (formId == kDatasetView_Alias)
    return validateSelectedDatasetAlias(value.toString().simplified());

  return true;
}

/**
 * @brief Mirrors a dataset's virtual flag onto its tree item, which paints a different badge.
 */
void DataModel::ProjectEditor::syncDatasetTreeVirtualFlag(const DataModel::Dataset& dataset)
{
  for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
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
void DataModel::ProjectEditor::commitDatasetFormEdit(int formId)
{
  auto& pm             = m_projectModelRef;
  const auto* prop     = Registry::datasetPropertyForFormId(formId);
  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;
  const bool coalesces = prop && prop->coalesce;
  const QString prefix = coalesces ? QString::fromLatin1(prop->coalesceKey) : QString();
  const QString key =
    coalesces
      ? QStringLiteral("%1:%2:%3:%4")
          .arg(
            prefix, QString::number(groupId), QString::number(datasetId), QString::number(formId))
      : QString();

  if (formId == kDatasetView_Title) {
    const auto newTitle = m_selectedDataset.title;
    pm.setNextUndoHint(tr("Rename Dataset"), key);
    pm.updateDataset(groupId, datasetId, m_selectedDataset, false);

    for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
      if (it.value().groupId != groupId || it.value().datasetId != datasetId)
        continue;

      auto* treeItem = it.key();
      treeItem->setText(newTitle);
      treeItem->setData(newTitle, TreeViewText);
      m_datasetItems[treeItem].title = newTitle;
      break;
    }

    Q_EMIT selectedTextChanged();
    Q_EMIT datasetOptionsChanged();
    Q_EMIT editableOptionsChanged();
    return;
  }

  const bool rebuildTree = prop && prop->rebuildTree;
  pm.setNextUndoHint(tr("Edit Dataset"), key);
  pm.updateDataset(groupId, datasetId, m_selectedDataset, rebuildTree);
  if (!rebuildTree)
    syncDatasetItemCache(groupId, datasetId);

  Q_EMIT datasetOptionsChanged();
  Q_EMIT editableOptionsChanged();
}

/**
 * @brief Dispatches dataset form edits to ProjectModel, rebuilding only on tree-visible changes.
 */
void DataModel::ProjectEditor::onDatasetItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto idInt = static_cast<DatasetItem>(item->data(ParameterType).toInt());
  const auto value = item->data(EditableValue);
  const int formId = static_cast<int>(idInt);
  if (!datasetFormEditAccepted(formId, value))
    return;

  const auto hint =
    Registry::applyDatasetFormEdit(formId, value, m_selectedDataset, m_projectModelRef);
  if (idInt == kDatasetView_Virtual)
    syncDatasetTreeVirtualFlag(m_selectedDataset);

  const bool rebuildNow   = !m_batchApplying && hint == PropertyHooks::RebuildHint::Sync;
  const bool rebuildLater = !m_batchApplying && hint == PropertyHooks::RebuildHint::Deferred;
  if (rebuildNow)
    buildDatasetModel(m_selectedDataset);

  if (rebuildLater) {
    const int uid = m_selectedDataset.uniqueId;
    QTimer::singleShot(0, this, [this, uid] {
      if (m_selectedDataset.uniqueId == uid)
        buildDatasetModel(m_selectedDataset);
    });
  }

  commitDatasetFormEdit(formId);
}

/**
 * @brief Refreshes the cached dataset record bound to the matching tree item.
 */
void DataModel::ProjectEditor::syncDatasetItemCache(int groupId, int datasetId)
{
  for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
    if (it.value().groupId != groupId || it.value().datasetId != datasetId)
      continue;

    m_datasetItems[it.key()] = m_selectedDataset;
    break;
  }
}

/**
 * @brief Applies one output-widget form field from @p item onto @p widget; pure field mutation with
 *        no side effects, shared by the single-selection and multi-selection edit paths.
 */
void DataModel::ProjectEditor::applyOutputWidgetField(QStandardItem* item,
                                                      DataModel::OutputWidget& widget)
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
    case kOutputWidget_Type:
      widget.type = static_cast<DataModel::OutputWidgetType>(value.toInt());
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
void DataModel::ProjectEditor::onOutputWidgetItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);
  const auto param = static_cast<OutputWidgetItem>(id.toInt());

  if (param == kOutputWidget_Type) {
    const auto newType = static_cast<DataModel::OutputWidgetType>(value.toInt());
    if (m_selectedOutputWidget.type != newType) {
      m_selectedOutputWidget.type = newType;
      buildOutputWidgetModel(m_selectedOutputWidget);
    }
  } else {
    applyOutputWidgetField(item, m_selectedOutputWidget);
  }

  if (param == kOutputWidget_Title) {
    const auto newTitle = value.toString();
    for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
      if (it.value().groupId == m_selectedOutputWidget.groupId
          && it.value().widgetId == m_selectedOutputWidget.widgetId) {
        it.key()->setData(newTitle, TreeViewText);
        m_outputWidgetItems[it.key()].title = newTitle;
        Q_EMIT selectedTextChanged();
        break;
      }
    }
  } else {
    for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
      if (it.value().groupId == m_selectedOutputWidget.groupId
          && it.value().widgetId == m_selectedOutputWidget.widgetId) {
        m_outputWidgetItems[it.key()] = m_selectedOutputWidget;
        break;
      }
    }
  }

  m_projectModelRef.setNextUndoHint(tr("Edit Output Widget"),
                                    QStringLiteral("owidget:%1:%2:%3")
                                      .arg(QString::number(m_selectedOutputWidget.groupId),
                                           QString::number(m_selectedOutputWidget.widgetId),
                                           QString::number(id.toInt())));
  m_projectModelRef.updateOutputWidget(
    m_selectedOutputWidget.groupId, m_selectedOutputWidget.widgetId, m_selectedOutputWidget, false);
}
