/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/ProjectEditor.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
#include "DataModel/FrameParser.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Private enums to track which item the user selected/modified
//--------------------------------------------------------------------------------------------------

// clang-format off
typedef enum { kRootItem, kFrameParser } TopLevelItem;

typedef enum {
  kProjectView_Title,
  kProjectView_FrameStartSequence,
  kProjectView_FrameEndSequence,
  kProjectView_FrameDecoder,
  kProjectView_HexadecimalSequence,
  kProjectView_FrameDetection,
  kProjectView_ChecksumFunction
} ProjectItem;

typedef enum {
  kDatasetView_Title,
  kDatasetView_Index,
  kDatasetView_Units,
  kDatasetView_Widget,
  kDatasetView_FFT,
  kDatasetView_LED,
  kDatasetView_LED_High,
  kDatasetView_Plot,
  kDatasetView_FFTMin,
  kDatasetView_FFTMax,
  kDatasetView_PltMin,
  kDatasetView_PltMax,
  kDatasetView_WgtMin,
  kDatasetView_WgtMax,
  kDatasetView_AlarmLow,
  kDatasetView_AlarmHigh,
  kDatasetView_FFT_Samples,
  kDatasetView_AlarmEnabled,
  kDatasetView_FFT_SamplingRate,
  kDatasetView_xAxis,
  kDatasetView_Overview
} DatasetItem;

typedef enum {
  kActionView_Title,
  kActionView_Icon,
  kActionView_EOL,
  kActionView_Data,
  kActionView_Binary,
  kActionView_AutoExecute,
  kActionView_TimerMode,
  kActionView_TimerInterval
} ActionItem;

typedef enum {
  kGroupView_Title,
  kGroupView_Widget,
  kGroupView_ImgMode,
  kGroupView_ImgStart,
  kGroupView_ImgEnd
} GroupItem;

// clang-format on

//--------------------------------------------------------------------------------------------------
// Constructor / singleton
//--------------------------------------------------------------------------------------------------

DataModel::ProjectEditor::ProjectEditor()
  : m_currentView(ProjectView)
  , m_treeModel(nullptr)
  , m_selectionModel(nullptr)
  , m_groupModel(nullptr)
  , m_actionModel(nullptr)
  , m_projectModel(nullptr)
  , m_datasetModel(nullptr)
{
  generateComboBoxModels();

  auto& pm = DataModel::ProjectModel::instance();

  connect(&pm,
          &DataModel::ProjectModel::groupsChanged,
          this,
          &DataModel::ProjectEditor::buildTreeModel,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::actionsChanged,
          this,
          &DataModel::ProjectEditor::buildTreeModel,
          Qt::QueuedConnection);
  connect(&pm, &DataModel::ProjectModel::modifiedChanged, this, [this] {
    if (m_currentView == ProjectView)
      buildProjectModel();
  });
  connect(&pm, &DataModel::ProjectModel::frameDetectionChanged, this, [this] {
    if (m_currentView == ProjectView)
      buildProjectModel();
  });
  connect(&pm, &DataModel::ProjectModel::jsonFileChanged, this, [this] {
    if (m_selectionModel) {
      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    }
  });

  connect(
    &pm,
    &DataModel::ProjectModel::groupAdded,
    this,
    [this](int groupId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
        if (it.value().groupId != groupId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        break;
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::groupDeleted,
    this,
    [this] {
      if (m_selectionModel) {
        auto index = m_treeModel->index(0, 0);
        m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::datasetAdded,
    this,
    [this](int groupId, int datasetId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
        if (it.value().groupId != groupId || it.value().datasetId != datasetId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        break;
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::datasetDeleted,
    this,
    [this](int survivingGroupId) {
      if (survivingGroupId >= 0) {
        for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
          if (it.value().groupId != survivingGroupId)
            continue;

          m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
          return;
        }
      }

      if (m_selectionModel) {
        auto index = m_treeModel->index(0, 0);
        m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::actionAdded,
    this,
    [this](int actionId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_actionItems.begin(); it != m_actionItems.end(); ++it) {
        if (it.value().actionId != actionId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        break;
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::actionDeleted,
    this,
    [this] {
      if (m_selectionModel) {
        auto index = m_treeModel->index(0, 0);
        m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);

  connect(this,
          &DataModel::ProjectEditor::groupModelChanged,
          this,
          &DataModel::ProjectEditor::editableOptionsChanged);
  connect(this,
          &DataModel::ProjectEditor::datasetModelChanged,
          this,
          &DataModel::ProjectEditor::editableOptionsChanged);
  connect(this,
          &DataModel::ProjectEditor::datasetModelChanged,
          this,
          &DataModel::ProjectEditor::datasetOptionsChanged);

  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged, this, [this] {
    generateComboBoxModels();
    buildTreeModel();

    switch (m_currentView) {
      case ProjectView:
        buildProjectModel();
        break;
      case GroupView:
        buildGroupModel(m_selectedGroup);
        break;
      case ActionView:
        buildActionModel(m_selectedAction);
        break;
      case DatasetView:
        buildDatasetModel(m_selectedDataset);
        break;
      default:
        break;
    }
  });

  buildTreeModel();
  buildProjectModel();
}

/**
 * @brief Returns the single ProjectEditor instance.
 */
DataModel::ProjectEditor& DataModel::ProjectEditor::instance()
{
  static ProjectEditor singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// View state accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the currently active editor view.
 */
DataModel::ProjectEditor::CurrentView DataModel::ProjectEditor::currentView() const
{
  return m_currentView;
}

/**
 * @brief Returns the display text of the currently selected tree item.
 */
QString DataModel::ProjectEditor::selectedText() const
{
  if (!m_selectionModel || !m_treeModel)
    return "";

  const auto index = m_selectionModel->currentIndex();
  return m_treeModel->data(index, TreeViewText).toString();
}

/**
 * @brief Returns the icon URL of the currently selected tree item.
 */
QString DataModel::ProjectEditor::selectedIcon() const
{
  if (!m_selectionModel || !m_treeModel)
    return "";

  const auto index = m_selectionModel->currentIndex();
  return m_treeModel->data(index, TreeViewIcon).toString();
}

/**
 * @brief Returns the icon identifier of the currently selected action.
 */
const QString DataModel::ProjectEditor::actionIcon() const
{
  return m_selectedAction.icon;
}

/**
 * @brief Returns a sorted list of all available action icon base-names.
 *
 * Scanned once from the embedded @c :/rcc/actions/ resource directory on
 * first call; subsequent calls return the cached list.
 */
const QStringList& DataModel::ProjectEditor::availableActionIcons() const
{
  static QStringList icons;

  if (icons.isEmpty()) {
    const auto path = QStringLiteral(":/rcc/actions/");
    QDirIterator it(path, QStringList() << "*.svg", QDir::Files);
    while (it.hasNext()) {
      const auto filePath = it.next();
      icons.append(QFileInfo(filePath).baseName());
    }
  }

  return icons;
}

/**
 * @brief Returns true when the selected group's dataset list may be freely edited.
 *
 * Groups that carry a fixed-layout widget (Accelerometer, GPS, etc.) are not
 * freely editable; only NoGroupWidget, MultiPlot, and DataGrid allow arbitrary
 * dataset additions and deletions.
 */
bool DataModel::ProjectEditor::currentGroupIsEditable() const
{
  if (m_currentView == GroupView) {
    const auto& widget = m_selectedGroup.widget;
    if (widget != "" && widget != "multiplot" && widget != "datagrid")
      return false;
  }

  return true;
}

/**
 * @brief Returns true when the selected dataset belongs to a freely editable group.
 *
 * Mirrors the logic of currentGroupIsEditable() but evaluated from the dataset
 * side: looks up the parent group's widget type in the live ProjectModel data.
 */
bool DataModel::ProjectEditor::currentDatasetIsEditable() const
{
  if (m_currentView == DatasetView) {
    const auto& groups = DataModel::ProjectModel::instance().groups();
    const auto groupId = m_selectedDataset.groupId;
    if (groups.size() > static_cast<size_t>(groupId)) {
      const auto& widget = groups[groupId].widget;
      if (widget != "" && widget != "multiplot" && widget != "datagrid")
        return false;
    }
  }

  return true;
}

/**
 * @brief Returns a bitmask of the dataset option flags for the selected dataset.
 *
 * Each bit corresponds to a @c SerialStudio::DatasetOption value
 * (Plot, FFT, LED, Bar, Gauge, Compass).
 */
quint8 DataModel::ProjectEditor::datasetOptions() const
{
  quint8 option = SerialStudio::DatasetGeneric;

  if (m_selectedDataset.plt)
    option |= SerialStudio::DatasetPlot;

  if (m_selectedDataset.fft)
    option |= SerialStudio::DatasetFFT;

  if (m_selectedDataset.led)
    option |= SerialStudio::DatasetLED;

  if (m_selectedDataset.widget == QStringLiteral("bar"))
    option |= SerialStudio::DatasetBar;
  else if (m_selectedDataset.widget == QStringLiteral("gauge"))
    option |= SerialStudio::DatasetGauge;
  else if (m_selectedDataset.widget == QStringLiteral("compass"))
    option |= SerialStudio::DatasetCompass;

  return option;
}

//--------------------------------------------------------------------------------------------------
// Model accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the hierarchical tree model shown in the project structure pane.
 */
DataModel::CustomModel* DataModel::ProjectEditor::treeModel() const
{
  return m_treeModel;
}

/**
 * @brief Returns the selection model that tracks the active tree item.
 */
QItemSelectionModel* DataModel::ProjectEditor::selectionModel() const
{
  return m_selectionModel;
}

/**
 * @brief Returns the form model for the currently selected group.
 */
DataModel::CustomModel* DataModel::ProjectEditor::groupModel() const
{
  return m_groupModel;
}

/**
 * @brief Returns the form model for the currently selected action.
 */
DataModel::CustomModel* DataModel::ProjectEditor::actionModel() const
{
  return m_actionModel;
}

/**
 * @brief Returns the form model for the project-level settings.
 */
DataModel::CustomModel* DataModel::ProjectEditor::projectModel() const
{
  return m_projectModel;
}

/**
 * @brief Returns the form model for the currently selected dataset.
 */
DataModel::CustomModel* DataModel::ProjectEditor::datasetModel() const
{
  return m_datasetModel;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects the Frame Parser Code item in the tree after a short delay.
 *
 * The delay allows any pending tree rebuild to complete before the selection
 * is applied, ensuring the item exists in the model.
 */
void DataModel::ProjectEditor::displayFrameParserView()
{
  for (auto it = m_rootItems.begin(); it != m_rootItems.end(); ++it) {
    if (it.value() != kFrameParser)
      continue;

    QTimer::singleShot(100, this, [=, this] {
      if (m_selectionModel)
        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
    });
    break;
  }
}

//--------------------------------------------------------------------------------------------------
// Model builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the full project-structure tree model from scratch.
 *
 * Saves and restores per-item expanded state. After the new tree is emitted via
 * treeModelChanged(), the previously selected item is re-selected by matching
 * its groupId / datasetId / actionId against the fresh item maps, falling back
 * to the root project item when no match is found.
 *
 * Connected to ProjectModel::groupsChanged and ProjectModel::actionsChanged via
 * Qt::QueuedConnection so it always runs after the data mutation completes.
 */
void DataModel::ProjectEditor::buildTreeModel()
{
  m_rootItems.clear();
  m_groupItems.clear();
  m_actionItems.clear();
  m_datasetItems.clear();

  QHash<QString, bool> expandedStates;
  if (m_treeModel)
    saveExpandedStateMap(m_treeModel->invisibleRootItem(), expandedStates, "");

  if (m_selectionModel) {
    disconnect(m_selectionModel);
    m_selectionModel->deleteLater();
    m_selectionModel = nullptr;
  }

  if (m_treeModel) {
    disconnect(m_treeModel);
    m_treeModel->deleteLater();
    m_treeModel = nullptr;
  }

  m_treeModel = new CustomModel(this);

  const auto& pm      = DataModel::ProjectModel::instance();
  const auto& groups  = pm.groups();
  const auto& actions = pm.actions();

  auto* root = new QStandardItem(pm.title());
  root->setData(root->text(), TreeViewText);
  root->setData("qrc:/rcc/icons/project-editor/treeview/project-setup.svg", TreeViewIcon);
  root->setData(true, TreeViewExpanded);

  auto* frameParsingCode = new QStandardItem(tr("Frame Parser Code"));
  frameParsingCode->setData(frameParsingCode->text(), TreeViewText);
  frameParsingCode->setData("qrc:/rcc/icons/project-editor/treeview/code.svg", TreeViewIcon);

  root->appendRow(frameParsingCode);
  m_treeModel->appendRow(root);

  m_rootItems.insert(root, kRootItem);
  m_rootItems.insert(frameParsingCode, kFrameParser);

  for (const auto& action : actions) {
    auto* actionItem = new QStandardItem(action.title);
    actionItem->setData(-1, TreeViewFrameIndex);
    actionItem->setData("qrc:/rcc/icons/project-editor/treeview/action.svg", TreeViewIcon);
    actionItem->setData(action.title, TreeViewText);
    root->appendRow(actionItem);
    m_actionItems.insert(actionItem, action);
  }

  for (const auto& group : groups) {
    auto* groupItem = new QStandardItem(group.title);
    auto widget     = SerialStudio::getDashboardWidget(group);
    auto icon       = SerialStudio::dashboardWidgetIcon(widget, false);

    groupItem->setData(icon, TreeViewIcon);
    groupItem->setData(-1, TreeViewFrameIndex);
    groupItem->setData(group.title, TreeViewText);

    for (const auto& dataset : group.datasets) {
      auto* datasetItem = new QStandardItem(dataset.title);
      auto widgets      = SerialStudio::getDashboardWidgets(dataset);
      QString dIcon     = "qrc:/rcc/icons/project-editor/treeview/dataset.svg";
      if (widgets.count() > 0)
        dIcon = SerialStudio::dashboardWidgetIcon(widgets.first(), false);

      datasetItem->setData(dIcon, TreeViewIcon);
      datasetItem->setData(dataset.title, TreeViewText);
      datasetItem->setData(dataset.index, TreeViewFrameIndex);
      groupItem->appendRow(datasetItem);
      m_datasetItems.insert(datasetItem, dataset);
    }

    restoreExpandedStateMap(groupItem, expandedStates, root->text() + "/" + group.title);
    root->appendRow(groupItem);
    m_groupItems.insert(groupItem, group);
  }

  auto* spacer = new QStandardItem(" ");
  spacer->setData(" ", TreeViewText);
  spacer->setData("", TreeViewIcon);
  spacer->setData(-1, TreeViewFrameIndex);
  spacer->setEnabled(false);
  spacer->setSelectable(false);
  root->appendRow(spacer);

  m_selectionModel = new QItemSelectionModel(m_treeModel);
  connect(m_selectionModel,
          &QItemSelectionModel::currentChanged,
          this,
          &DataModel::ProjectEditor::onCurrentSelectionChanged);

  Q_EMIT treeModelChanged();

  // Restore selection: find the new item that matches the previously selected one.
  // Fall back to the root project item if nothing matches.
  QStandardItem* toSelect = nullptr;

  if (m_currentView == DatasetView) {
    const auto gid = m_selectedDataset.groupId;
    const auto did = m_selectedDataset.datasetId;
    for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
      if (it.value().groupId == gid && it.value().datasetId == did) {
        toSelect = it.key();
        break;
      }
    }
  } else if (m_currentView == GroupView) {
    const auto gid = m_selectedGroup.groupId;
    for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
      if (it.value().groupId == gid) {
        toSelect = it.key();
        break;
      }
    }
  } else if (m_currentView == ActionView) {
    const auto aid = m_selectedAction.actionId;
    for (auto it = m_actionItems.begin(); it != m_actionItems.end(); ++it) {
      if (it.value().actionId == aid) {
        toSelect = it.key();
        break;
      }
    }
  }

  if (!toSelect) {
    for (auto it = m_rootItems.begin(); it != m_rootItems.end(); ++it) {
      if (it.value() == kRootItem) {
        toSelect = it.key();
        break;
      }
    }
  }

  if (toSelect)
    m_selectionModel->setCurrentIndex(toSelect->index(), QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Rebuilds the project-level settings form model.
 *
 * Populates frame detection, delimiter, checksum, and decoder rows.
 * Some rows are conditionally shown depending on the active FrameDetection
 * mode (e.g. start/end delimiter fields are hidden for End-Delimiter-only mode).
 * Connects itemChanged to onProjectItemChanged.
 */
void DataModel::ProjectEditor::buildProjectModel()
{
  if (m_projectModel) {
    disconnect(m_projectModel);
    m_projectModel->deleteLater();
  }

  m_projectModel = new CustomModel(this);
  const auto& pm = DataModel::ProjectModel::instance();

  // -----------------------------------------------------------------------
  // Project Information section
  // -----------------------------------------------------------------------
  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Project Information"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/project.svg", ParameterIcon);
  m_projectModel->appendRow(hdr);

  auto* title = new QStandardItem();
  title->setEditable(true);
  title->setData(true, Active);
  title->setData(TextField, WidgetType);
  title->setData(pm.title(), EditableValue);
  title->setData(kProjectView_Title, ParameterType);
  title->setData(tr("Project Title"), ParameterName);
  title->setData(tr("Untitled Project"), PlaceholderValue);
  title->setData(tr("Name or description of the project"), ParameterDescription);
  m_projectModel->appendRow(title);

  // -----------------------------------------------------------------------
  // Frame Detection section — re-read current state from serialized JSON
  // -----------------------------------------------------------------------
  const auto json          = pm.serializeToJson();
  const bool hexDelimiters = json.value("hexadecimalDelimiters").toBool();
  const auto detection     = pm.frameDetection();
  const auto frameStart    = json.value("frameStart").toString();
  const auto frameEnd      = json.value("frameEnd").toString();
  const auto checksum      = json.value("checksum").toString();
  const auto decoder       = pm.decoderMethod();

  hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Frame Detection"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/frame-detection.svg", ParameterIcon);
  m_projectModel->appendRow(hdr);

  auto* frameDetection = new QStandardItem();
  frameDetection->setEditable(true);
  frameDetection->setData(true, Active);
  frameDetection->setData(ComboBox, WidgetType);
  frameDetection->setData(m_frameDetectionMethods, ComboBoxData);
  frameDetection->setData(m_frameDetectionMethodsValues.indexOf(detection), EditableValue);
  frameDetection->setData(kProjectView_FrameDetection, ParameterType);
  frameDetection->setData(tr("Frame Detection Method"), ParameterName);
  frameDetection->setData(tr("Select how incoming data frames are identified"),
                          ParameterDescription);
  m_projectModel->appendRow(frameDetection);

  auto* hexSeq = new QStandardItem();
  hexSeq->setEditable(true);
  hexSeq->setData(true, Active);
  hexSeq->setData(CheckBox, WidgetType);
  hexSeq->setData(hexDelimiters, EditableValue);
  hexSeq->setData(kProjectView_HexadecimalSequence, ParameterType);
  hexSeq->setData(tr("Hexadecimal Delimiters"), ParameterName);
  hexSeq->setData(tr("Enter frame start/end sequences as hexadecimal values"),
                  ParameterDescription);
  m_projectModel->appendRow(hexSeq);

  const bool showStart = (detection == SerialStudio::StartDelimiterOnly
                          || detection == SerialStudio::StartAndEndDelimiter);
  const bool showEnd   = (detection == SerialStudio::EndDelimiterOnly
                        || detection == SerialStudio::StartAndEndDelimiter);

  if (showStart) {
    auto* startSeq = new QStandardItem();
    startSeq->setEditable(true);
    startSeq->setData(true, Active);
    startSeq->setData(hexDelimiters ? HexTextField : TextField, WidgetType);
    startSeq->setData(frameStart, EditableValue);
    startSeq->setData(kProjectView_FrameStartSequence, ParameterType);
    startSeq->setData(tr("Frame Start Delimiter"), ParameterName);
    startSeq->setData(tr("e.g. /*"), PlaceholderValue);
    startSeq->setData(tr("Sequence that marks the beginning of a data frame"),
                      ParameterDescription);
    m_projectModel->appendRow(startSeq);
  }

  if (showEnd) {
    auto* endSeq = new QStandardItem();
    endSeq->setEditable(true);
    endSeq->setData(true, Active);
    endSeq->setData(hexDelimiters ? HexTextField : TextField, WidgetType);
    endSeq->setData(frameEnd, EditableValue);
    endSeq->setData(kProjectView_FrameEndSequence, ParameterType);
    endSeq->setData(tr("Frame End Delimiter"), ParameterName);
    endSeq->setData(tr("e.g. */"), PlaceholderValue);
    endSeq->setData(tr("Sequence that marks the end of a data frame"), ParameterDescription);
    m_projectModel->appendRow(endSeq);
  }

  // -----------------------------------------------------------------------
  // Payload Processing section
  // -----------------------------------------------------------------------
  hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Payload Processing & Validation"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/data-conversion.svg", ParameterIcon);
  m_projectModel->appendRow(hdr);

  auto* decoderItem = new QStandardItem();
  decoderItem->setEditable(true);
  decoderItem->setData(true, Active);
  decoderItem->setData(ComboBox, WidgetType);
  decoderItem->setData(m_decoderOptions, ComboBoxData);
  decoderItem->setData(static_cast<int>(decoder), EditableValue);
  decoderItem->setData(kProjectView_FrameDecoder, ParameterType);
  decoderItem->setData(tr("Data Conversion Method"), ParameterName);
  decoderItem->setData(tr("Select how incoming binary data is decoded before parsing"),
                       ParameterDescription);
  m_projectModel->appendRow(decoderItem);

  // Find checksum index
  const auto availableChecksums = IO::availableChecksums();
  int checksumIdx               = availableChecksums.indexOf(checksum);
  if (checksumIdx < 0)
    checksumIdx = 0;

  auto* checksumItem = new QStandardItem();
  checksumItem->setEditable(true);
  checksumItem->setData(true, Active);
  checksumItem->setData(ComboBox, WidgetType);
  checksumItem->setData(m_checksumMethods, ComboBoxData);
  checksumItem->setData(checksumIdx, EditableValue);
  checksumItem->setData(kProjectView_ChecksumFunction, ParameterType);
  checksumItem->setData(tr("Checksum Algorithm"), ParameterName);
  checksumItem->setData(tr("Select the checksum algorithm used to validate frames"),
                        ParameterDescription);
  m_projectModel->appendRow(checksumItem);

  connect(m_projectModel,
          &CustomModel::itemChanged,
          this,
          &DataModel::ProjectEditor::onProjectItemChanged);

  Q_EMIT projectModelChanged();
}

/**
 * @brief Rebuilds the group-settings form model for @p group.
 *
 * Stores @p group as the selected group proxy, populates the Title and
 * Composite Widget rows, and connects itemChanged to onGroupItemChanged.
 *
 * @param group  The group whose properties should be displayed.
 */
void DataModel::ProjectEditor::buildGroupModel(const DataModel::Group& group)
{
  if (m_groupModel) {
    disconnect(m_groupModel);
    m_groupModel->deleteLater();
  }

  m_selectedGroup = group;
  m_groupModel    = new CustomModel(this);

  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Group Information"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/group.svg", ParameterIcon);
  m_groupModel->appendRow(hdr);

  auto* titleItem = new QStandardItem();
  titleItem->setEditable(true);
  titleItem->setData(true, Active);
  titleItem->setData(TextField, WidgetType);
  titleItem->setData(group.title, EditableValue);
  titleItem->setData(kGroupView_Title, ParameterType);
  titleItem->setData(tr("Group Title"), ParameterName);
  titleItem->setData(tr("Untitled Group"), PlaceholderValue);
  titleItem->setData(tr("Title or description of this dataset group"), ParameterDescription);
  m_groupModel->appendRow(titleItem);

  int index  = 0;
  bool found = false;
  for (auto it = m_groupWidgets.begin(); it != m_groupWidgets.end(); ++it, ++index) {
    if (it.key() == group.widget) {
      found = true;
      break;
    }
  }

  if (!found)
    index = 0;

  auto* widgetItem = new QStandardItem();
  widgetItem->setEditable(true);
  widgetItem->setData(true, Active);
  widgetItem->setData(ComboBox, WidgetType);
  widgetItem->setData(m_groupWidgets.values(), ComboBoxData);
  widgetItem->setData(index, EditableValue);
  widgetItem->setData(kGroupView_Widget, ParameterType);
  widgetItem->setData(tr("Composite Widget"), ParameterName);
  widgetItem->setData(tr("Select how this group of datasets should be visualized (optional)"),
                      ParameterDescription);
  m_groupModel->appendRow(widgetItem);

  // Image View configuration fields (pro only, shown when widget == "image")
#ifdef BUILD_COMMERCIAL
  if (group.widget == QLatin1String("image")) {
    auto* imgHdr = new QStandardItem();
    imgHdr->setData(SectionHeader, WidgetType);
    imgHdr->setData(tr("Image Configuration"), PlaceholderValue);
    imgHdr->setData("qrc:/rcc/icons/project-editor/model/image.svg", ParameterIcon);
    m_groupModel->appendRow(imgHdr);

    static const QStringList kImgModeLabels = {tr("Auto-detect"), tr("Manual Delimiters")};
    static const QStringList kImgModeValues = {
      QStringLiteral("autodetect"), QStringLiteral("manual")};

    int modeIndex = group.imgDetectionMode == QLatin1String("manual") ? 1 : 0;

    auto* modeItem = new QStandardItem();
    modeItem->setEditable(true);
    modeItem->setData(true, Active);
    modeItem->setData(ComboBox, WidgetType);
    modeItem->setData(kImgModeLabels, ComboBoxData);
    modeItem->setData(modeIndex, EditableValue);
    modeItem->setData(kGroupView_ImgMode, ParameterType);
    modeItem->setData(tr("Detection Mode"), ParameterName);
    modeItem->setData(
      tr("Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences"),
      ParameterDescription);
    m_groupModel->appendRow(modeItem);

    auto* startItem = new QStandardItem();
    startItem->setEditable(true);
    startItem->setData(group.imgDetectionMode == QLatin1String("manual"), Active);
    startItem->setData(TextField, WidgetType);
    startItem->setData(group.imgStartSequence, EditableValue);
    startItem->setData(kGroupView_ImgStart, ParameterType);
    startItem->setData(tr("Start Sequence (Hex)"), ParameterName);
    startItem->setData(tr("e.g. FF D8 FF"), PlaceholderValue);
    startItem->setData(tr("Hex bytes marking the start of an image frame"),
                       ParameterDescription);
    m_groupModel->appendRow(startItem);

    auto* endItem = new QStandardItem();
    endItem->setEditable(true);
    endItem->setData(group.imgDetectionMode == QLatin1String("manual"), Active);
    endItem->setData(TextField, WidgetType);
    endItem->setData(group.imgEndSequence, EditableValue);
    endItem->setData(kGroupView_ImgEnd, ParameterType);
    endItem->setData(tr("End Sequence (Hex)"), ParameterName);
    endItem->setData(tr("e.g. FF D9"), PlaceholderValue);
    endItem->setData(tr("Hex bytes marking the end of an image frame"), ParameterDescription);
    m_groupModel->appendRow(endItem);
  }
#endif

  connect(
    m_groupModel, &CustomModel::itemChanged, this, &DataModel::ProjectEditor::onGroupItemChanged);

  Q_EMIT groupModelChanged();
}

/**
 * @brief Rebuilds the action-settings form model for @p action.
 *
 * Stores @p action as the selected action proxy and populates title, icon,
 * transmit data, EOL sequence, auto-execute, timer mode, and interval rows.
 * Connects itemChanged to onActionItemChanged.
 *
 * @param action  The action whose properties should be displayed.
 */
void DataModel::ProjectEditor::buildActionModel(const DataModel::Action& action)
{
  if (m_actionModel) {
    disconnect(m_actionModel);
    m_actionModel->deleteLater();
  }

  m_selectedAction = action;
  m_actionModel    = new CustomModel(this);

  // General Information
  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("General Information"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/action.svg", ParameterIcon);
  m_actionModel->appendRow(hdr);

  auto* titleItem = new QStandardItem();
  titleItem->setEditable(true);
  titleItem->setData(true, Active);
  titleItem->setData(TextField, WidgetType);
  titleItem->setData(action.title, EditableValue);
  titleItem->setData(tr("Action Title"), ParameterName);
  titleItem->setData(kActionView_Title, ParameterType);
  titleItem->setData(tr("Untitled Action"), PlaceholderValue);
  titleItem->setData(tr("Name or description of this action"), ParameterDescription);
  m_actionModel->appendRow(titleItem);

  auto* iconItem = new QStandardItem();
  iconItem->setEditable(true);
  iconItem->setData(true, Active);
  iconItem->setData(IconPicker, WidgetType);
  iconItem->setData(action.icon, EditableValue);
  iconItem->setData(kActionView_Icon, ParameterType);
  iconItem->setData(tr("Action Icon"), ParameterName);
  iconItem->setData(tr("Default Icon"), PlaceholderValue);
  iconItem->setData(tr("Icon displayed for this action in the dashboard"), ParameterDescription);
  m_actionModel->appendRow(iconItem);

  // Data Payload
  hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Data Payload"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/tx-data.svg", ParameterIcon);
  m_actionModel->appendRow(hdr);

  auto* binaryItem = new QStandardItem();
  binaryItem->setEditable(true);
  binaryItem->setData(true, Active);
  binaryItem->setData(CheckBox, WidgetType);
  binaryItem->setData(0, PlaceholderValue);
  binaryItem->setData(action.binaryData, EditableValue);
  binaryItem->setData(kActionView_Binary, ParameterType);
  binaryItem->setData(tr("Send as Binary"), ParameterName);
  binaryItem->setData(tr("Send raw binary data when this action is triggered"),
                      ParameterDescription);
  m_actionModel->appendRow(binaryItem);

  if (action.binaryData) {
    auto* dataItem = new QStandardItem();
    dataItem->setEditable(true);
    dataItem->setData(true, Active);
    dataItem->setData(HexTextField, WidgetType);
    dataItem->setData(action.txData, EditableValue);
    dataItem->setData(kActionView_Data, ParameterType);
    dataItem->setData(tr("Command"), PlaceholderValue);
    dataItem->setData(tr("Transmit Data (Hex)"), ParameterName);
    dataItem->setData(tr("Hexadecimal payload to send when the action is triggered"),
                      ParameterDescription);
    m_actionModel->appendRow(dataItem);
  } else {
    auto* dataItem = new QStandardItem();
    dataItem->setEditable(true);
    dataItem->setData(true, Active);
    dataItem->setData(TextField, WidgetType);
    dataItem->setData(action.txData, EditableValue);
    dataItem->setData(kActionView_Data, ParameterType);
    dataItem->setData(tr("Command"), PlaceholderValue);
    dataItem->setData(tr("Transmit Data"), ParameterName);
    dataItem->setData(tr("Text payload to send when the action is triggered"),
                      ParameterDescription);
    m_actionModel->appendRow(dataItem);
  }

  int eolIndex = 0;
  bool found   = false;
  for (auto it = m_eolSequences.begin(); it != m_eolSequences.end(); ++it, ++eolIndex) {
    if (it.key() == action.eolSequence) {
      found = true;
      break;
    }
  }

  if (!found)
    eolIndex = 0;

  auto* eolItem = new QStandardItem();
  eolItem->setData(ComboBox, WidgetType);
  eolItem->setEditable(!action.binaryData);
  eolItem->setData(eolIndex, EditableValue);
  eolItem->setData(!action.binaryData, Active);
  eolItem->setData(kActionView_EOL, ParameterType);
  eolItem->setData(m_eolSequences.values(), ComboBoxData);
  eolItem->setData(tr("End-of-Line Sequence"), ParameterName);
  eolItem->setData(tr("EOL characters to append to the message (e.g. \\n, \\r\\n)"),
                   ParameterDescription);
  m_actionModel->appendRow(eolItem);

  // Execution Behavior
  hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Execution Behavior"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/action-behavior.svg", ParameterIcon);
  m_actionModel->appendRow(hdr);

  auto* autoExec = new QStandardItem();
  autoExec->setEditable(true);
  autoExec->setData(true, Active);
  autoExec->setData(0, PlaceholderValue);
  autoExec->setData(CheckBox, WidgetType);
  autoExec->setData(kActionView_AutoExecute, ParameterType);
  autoExec->setData(action.autoExecuteOnConnect, EditableValue);
  autoExec->setData(tr("Auto-Execute on Connect"), ParameterName);
  autoExec->setData(tr("Automatically trigger this action when the device connects"),
                    ParameterDescription);
  m_actionModel->appendRow(autoExec);

  // Timer Behavior
  hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Timer Behavior"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/timer.svg", ParameterIcon);
  m_actionModel->appendRow(hdr);

  auto* timerMode = new QStandardItem();
  timerMode->setEditable(true);
  timerMode->setData(true, Active);
  timerMode->setData(ComboBox, WidgetType);
  timerMode->setData(m_timerModes, ComboBoxData);
  timerMode->setData(tr("Timer Mode"), ParameterName);
  timerMode->setData(kActionView_TimerMode, ParameterType);
  timerMode->setData(static_cast<int>(action.timerMode), EditableValue);
  timerMode->setData(tr("Choose when and how this action should repeat automatically"),
                     ParameterDescription);
  m_actionModel->appendRow(timerMode);

  auto* timerInterval = new QStandardItem();
  timerInterval->setData(IntField, WidgetType);
  timerInterval->setEditable(action.timerMode != DataModel::TimerMode::Off);
  timerInterval->setData(tr("Interval (ms)"), ParameterName);
  timerInterval->setData(timerInterval->isEditable(), Active);
  timerInterval->setData(action.timerIntervalMs, EditableValue);
  timerInterval->setData(kActionView_TimerInterval, ParameterType);
  timerInterval->setData(tr("Timer Interval (ms)"), PlaceholderValue);
  timerInterval->setData(tr("Milliseconds between each repeated trigger of this action"),
                         ParameterDescription);
  m_actionModel->appendRow(timerInterval);

  connect(
    m_actionModel, &CustomModel::itemChanged, this, &DataModel::ProjectEditor::onActionItemChanged);

  Q_EMIT actionModelChanged();
}

/**
 * @brief Rebuilds the dataset-settings form model for @p dataset.
 *
 * Stores @p dataset as the selected dataset proxy, then delegates section
 * construction to addGeneralSection(), addPlotSection(), addFFTSection(),
 * addWidgetSection(), addAlarmSection(), and addLEDSection().
 * Connects itemChanged to onDatasetItemChanged.
 *
 * @param dataset  The dataset whose properties should be displayed.
 */
void DataModel::ProjectEditor::buildDatasetModel(const DataModel::Dataset& dataset)
{
  if (m_datasetModel) {
    disconnect(m_datasetModel);
    m_datasetModel->deleteLater();
  }

  m_selectedDataset = dataset;
  m_datasetModel    = new CustomModel(this);

  addGeneralSection(m_datasetModel, dataset);
  addPlotSection(m_datasetModel, dataset);
  addFFTSection(m_datasetModel, dataset);
  addWidgetSection(m_datasetModel, dataset);
  addAlarmSection(m_datasetModel, dataset);
  addLEDSection(m_datasetModel, dataset);

  connect(m_datasetModel,
          &CustomModel::itemChanged,
          this,
          &DataModel::ProjectEditor::onDatasetItemChanged);

  Q_EMIT datasetModelChanged();
}

//--------------------------------------------------------------------------------------------------
// buildDatasetModel section helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the General section rows (title, frame index, units) to @p model.
 * @param model    Target dataset form model.
 * @param dataset  Source dataset values.
 */
void DataModel::ProjectEditor::addGeneralSection(CustomModel* model,
                                                 const DataModel::Dataset& dataset)
{
  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("General Information"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/dataset.svg", ParameterIcon);
  model->appendRow(hdr);

  auto* titleItem = new QStandardItem();
  titleItem->setEditable(true);
  titleItem->setData(true, Active);
  titleItem->setData(TextField, WidgetType);
  titleItem->setData(dataset.title, EditableValue);
  titleItem->setData(kDatasetView_Title, ParameterType);
  titleItem->setData(tr("Untitled Dataset"), PlaceholderValue);
  titleItem->setData(tr("Dataset Title"), ParameterName);
  titleItem->setData(tr("Name of the dataset, used for labeling and identification"),
                     ParameterDescription);
  model->appendRow(titleItem);

  const auto& pm = DataModel::ProjectModel::instance();

  auto* indexItem = new QStandardItem();
  indexItem->setEditable(true);
  indexItem->setData(true, Active);
  indexItem->setData(IntField, WidgetType);
  indexItem->setData(dataset.index, EditableValue);
  indexItem->setData(kDatasetView_Index, ParameterType);
  indexItem->setData(pm.datasetCount() + 1, PlaceholderValue);
  indexItem->setData(tr("Frame Index"), ParameterName);
  indexItem->setData(tr("Frame position used for aligning datasets in time"), ParameterDescription);
  model->appendRow(indexItem);

  auto* unitsItem = new QStandardItem();
  unitsItem->setEditable(true);
  unitsItem->setData(true, Active);
  unitsItem->setData(TextField, WidgetType);
  unitsItem->setData(dataset.units, EditableValue);
  unitsItem->setData(kDatasetView_Units, ParameterType);
  unitsItem->setData(tr("Measurement Unit"), ParameterName);
  unitsItem->setData(tr("Volts, Amps, etc."), PlaceholderValue);
  unitsItem->setData(tr("Unit of measurement, such as volts or amps (optional)"),
                     ParameterDescription);
  model->appendRow(unitsItem);
}

/**
 * @brief Appends the Plot section rows (enable, X-axis source, min/max) to @p model.
 * @param model    Target dataset form model.
 * @param dataset  Source dataset values.
 */
void DataModel::ProjectEditor::addPlotSection(CustomModel* model, const DataModel::Dataset& dataset)
{
  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Plot Settings"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/plot.svg", ParameterIcon);
  model->appendRow(hdr);

  int plotIndex          = 0;
  bool found             = false;
  const auto currentPair = qMakePair(dataset.plt, dataset.log);
  for (auto it = m_plotOptions.begin(); it != m_plotOptions.end(); ++it, ++plotIndex) {
    if (it.key() == currentPair) {
      found = true;
      break;
    }
  }

  if (!found)
    plotIndex = 0;

  auto* plotItem = new QStandardItem();
  plotItem->setEditable(true);
  plotItem->setData(ComboBox, WidgetType);
  plotItem->setData(plotIndex, EditableValue);
  plotItem->setData(plotItem->isEditable(), Active);
  plotItem->setData(m_plotOptions.values(), ComboBoxData);
  plotItem->setData(tr("Enable Plot Widget"), ParameterName);
  plotItem->setData(kDatasetView_Plot, ParameterType);
  plotItem->setData(tr("Plot data in real-time"), ParameterDescription);
  model->appendRow(plotItem);

  // X-axis source
  const auto& groups = DataModel::ProjectModel::instance().groups();
  int xAxisIdx       = 0;
  for (const auto& group : groups) {
    for (const auto& d : group.datasets) {
      if (d.index == m_selectedDataset.xAxisId) {
        xAxisIdx = d.index;
        break;
      }
    }

    if (xAxisIdx != 0)
      break;
  }

  auto* xAxisItem = new QStandardItem();
  xAxisItem->setEditable(dataset.plt);
  xAxisItem->setData(ComboBox, WidgetType);
  xAxisItem->setData(xAxisIdx, EditableValue);
  xAxisItem->setData(xAxisItem->isEditable(), Active);
  xAxisItem->setData(DataModel::ProjectModel::instance().xDataSources(), ComboBoxData);
  xAxisItem->setData(kDatasetView_xAxis, ParameterType);
  xAxisItem->setData(tr("X-Axis Source"), ParameterName);
  xAxisItem->setData(tr("Choose which dataset to use for the X-Axis in plots"),
                     ParameterDescription);
  model->appendRow(xAxisItem);

  auto* pltMin = new QStandardItem();
  pltMin->setEditable(dataset.plt);
  pltMin->setData(0, PlaceholderValue);
  pltMin->setData(FloatField, WidgetType);
  pltMin->setData(pltMin->isEditable(), Active);
  pltMin->setData(dataset.pltMin, EditableValue);
  pltMin->setData(kDatasetView_PltMin, ParameterType);
  pltMin->setData(tr("Minimum Plot Value (optional)"), ParameterName);
  pltMin->setData(tr("Lower bound for plot display range"), ParameterDescription);
  model->appendRow(pltMin);

  auto* pltMax = new QStandardItem();
  pltMax->setEditable(dataset.plt);
  pltMax->setData(0, PlaceholderValue);
  pltMax->setData(FloatField, WidgetType);
  pltMax->setData(pltMax->isEditable(), Active);
  pltMax->setData(dataset.pltMax, EditableValue);
  pltMax->setData(kDatasetView_PltMax, ParameterType);
  pltMax->setData(tr("Maximum Plot Value (optional)"), ParameterName);
  pltMax->setData(tr("Upper bound for plot display range"), ParameterDescription);
  model->appendRow(pltMax);
}

/**
 * @brief Appends the FFT section rows (enable, sample count, rate, min/max) to @p model.
 * @param model    Target dataset form model.
 * @param dataset  Source dataset values.
 */
void DataModel::ProjectEditor::addFFTSection(CustomModel* model, const DataModel::Dataset& dataset)
{
  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("FFT Configuration"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/fft.svg", ParameterIcon);
  model->appendRow(hdr);

  auto* fftItem = new QStandardItem();
  fftItem->setEditable(true);
  fftItem->setData(0, PlaceholderValue);
  fftItem->setData(CheckBox, WidgetType);
  fftItem->setData(fftItem->isEditable(), Active);
  fftItem->setData(dataset.fft, EditableValue);
  fftItem->setData(kDatasetView_FFT, ParameterType);
  fftItem->setData(tr("Enable FFT Analysis"), ParameterName);
  fftItem->setData(tr("Perform frequency-domain analysis of the dataset"), ParameterDescription);
  model->appendRow(fftItem);

  const auto windowSize = QString::number(dataset.fftSamples);
  int windowIndex       = m_fftSamples.indexOf(windowSize);
  if (windowIndex < 0)
    windowIndex = 7;

  auto* fftWindow = new QStandardItem();
  fftWindow->setEditable(dataset.fft);
  fftWindow->setData(ComboBox, WidgetType);
  fftWindow->setData(m_fftSamples, ComboBoxData);
  fftWindow->setData(windowIndex, EditableValue);
  fftWindow->setData(fftWindow->isEditable(), Active);
  fftWindow->setData(kDatasetView_FFT_Samples, ParameterType);
  fftWindow->setData(tr("FFT Window Size"), ParameterName);
  fftWindow->setData(tr("Number of samples used for each FFT calculation window"),
                     ParameterDescription);
  model->appendRow(fftWindow);

  auto* fftRate = new QStandardItem();
  fftRate->setEditable(dataset.fft);
  fftRate->setData(IntField, WidgetType);
  fftRate->setData(100, PlaceholderValue);
  fftRate->setData(fftRate->isEditable(), Active);
  fftRate->setData(dataset.fftSamplingRate, EditableValue);
  fftRate->setData(kDatasetView_FFT_SamplingRate, ParameterType);
  fftRate->setData(tr("FFT Sampling Rate (Hz, required)"), ParameterName);
  fftRate->setData(tr("Sampling frequency used for FFT (in Hz)"), ParameterDescription);
  model->appendRow(fftRate);

  auto* fftMin = new QStandardItem();
  fftMin->setEditable(dataset.fft);
  fftMin->setData(0, PlaceholderValue);
  fftMin->setData(FloatField, WidgetType);
  fftMin->setData(fftMin->isEditable(), Active);
  fftMin->setData(dataset.fftMin, EditableValue);
  fftMin->setData(kDatasetView_FFTMin, ParameterType);
  fftMin->setData(tr("Minimum Value (recommended)"), ParameterName);
  fftMin->setData(tr("Lower bound for data normalization"), ParameterDescription);
  model->appendRow(fftMin);

  auto* fftMax = new QStandardItem();
  fftMax->setEditable(dataset.fft);
  fftMax->setData(0, PlaceholderValue);
  fftMax->setData(FloatField, WidgetType);
  fftMax->setData(fftMax->isEditable(), Active);
  fftMax->setData(dataset.fftMax, EditableValue);
  fftMax->setData(kDatasetView_FFTMax, ParameterType);
  fftMax->setData(tr("Maximum Value (recommended)"), ParameterName);
  fftMax->setData(tr("Upper bound for data normalization"), ParameterDescription);
  model->appendRow(fftMax);
}

/**
 * @brief Appends the Widget section rows (widget type, min/max display values) to @p model.
 * @param model    Target dataset form model.
 * @param dataset  Source dataset values.
 */
void DataModel::ProjectEditor::addWidgetSection(CustomModel* model,
                                                const DataModel::Dataset& dataset)
{
  const bool showWidget = currentDatasetIsEditable();

  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Widget Settings"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/widget.svg", ParameterIcon);
  model->appendRow(hdr);

  int widgetIndex = 0;
  bool found      = false;
  for (auto it = m_datasetWidgets.begin(); it != m_datasetWidgets.end(); ++it, ++widgetIndex) {
    if (it.key() == dataset.widget) {
      found = true;
      break;
    }
  }

  if (!found)
    widgetIndex = 0;

  auto* widgetItem = new QStandardItem();
  widgetItem->setEditable(showWidget);
  widgetItem->setData(showWidget, Active);
  widgetItem->setData(ComboBox, WidgetType);
  widgetItem->setData(widgetIndex, EditableValue);
  widgetItem->setData(kDatasetView_Widget, ParameterType);
  widgetItem->setData(m_datasetWidgets.values(), ComboBoxData);
  widgetItem->setData(tr("Widget"), ParameterName);
  widgetItem->setData(tr("Select the visual widget used to display this dataset"),
                      ParameterDescription);
  model->appendRow(widgetItem);

  const auto& pm     = DataModel::ProjectModel::instance();
  auto* overviewItem = new QStandardItem();
  overviewItem->setData(CheckBox, WidgetType);
  overviewItem->setEditable(showWidget && pm.groupCount() > 1);
  overviewItem->setData(overviewItem->isEditable(), Active);
  overviewItem->setData(dataset.overviewDisplay, EditableValue);
  overviewItem->setData(kDatasetView_Overview, ParameterType);
  overviewItem->setData(tr("Show in Overview"), ParameterName);
  overviewItem->setData(tr("Display this widget in the dashboard overview (if enabled)"),
                        ParameterDescription);
  model->appendRow(overviewItem);

  const bool rangeEnabled = showWidget && (dataset.widget == "bar" || dataset.widget == "gauge");

  auto* wgtMin = new QStandardItem();
  wgtMin->setEditable(rangeEnabled);
  wgtMin->setData(0, PlaceholderValue);
  wgtMin->setData(FloatField, WidgetType);
  wgtMin->setData(wgtMin->isEditable(), Active);
  wgtMin->setData(dataset.wgtMin, EditableValue);
  wgtMin->setData(kDatasetView_WgtMin, ParameterType);
  wgtMin->setData(tr("Minimum Display Value (required)"), ParameterName);
  wgtMin->setData(tr("Lower bound of the gauge or bar display range"), ParameterDescription);
  model->appendRow(wgtMin);

  auto* wgtMax = new QStandardItem();
  wgtMax->setEditable(rangeEnabled);
  wgtMax->setData(0, PlaceholderValue);
  wgtMax->setData(FloatField, WidgetType);
  wgtMax->setData(wgtMax->isEditable(), Active);
  wgtMax->setData(dataset.wgtMax, EditableValue);
  wgtMax->setData(kDatasetView_WgtMax, ParameterType);
  wgtMax->setData(tr("Maximum Display Value (required)"), ParameterName);
  wgtMax->setData(tr("Upper bound of the gauge or bar display range"), ParameterDescription);
  model->appendRow(wgtMax);
}

/**
 * @brief Appends the Alarm section rows (enable, low/high thresholds) to @p model.
 * @param model    Target dataset form model.
 * @param dataset  Source dataset values.
 */
void DataModel::ProjectEditor::addAlarmSection(CustomModel* model,
                                               const DataModel::Dataset& dataset)
{
  const bool showWidget   = currentDatasetIsEditable();
  const bool rangeEnabled = showWidget && (dataset.widget == "bar" || dataset.widget == "gauge");

  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Alarm Settings"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/alarm.svg", ParameterIcon);
  model->appendRow(hdr);

  auto* alarmEnabled = new QStandardItem();
  alarmEnabled->setEditable(rangeEnabled);
  alarmEnabled->setData(0, PlaceholderValue);
  alarmEnabled->setData(CheckBox, WidgetType);
  alarmEnabled->setData(alarmEnabled->isEditable(), Active);
  alarmEnabled->setData(dataset.alarmEnabled, EditableValue);
  alarmEnabled->setData(kDatasetView_AlarmEnabled, ParameterType);
  alarmEnabled->setData(tr("Enable Alarms"), ParameterName);
  alarmEnabled->setData(tr("Triggers a visual alarm when the value exceeds alarm thresholds"),
                        ParameterDescription);
  model->appendRow(alarmEnabled);

  auto* alarmLow = new QStandardItem();
  alarmLow->setEditable(rangeEnabled && dataset.alarmEnabled);
  alarmLow->setData(0, PlaceholderValue);
  alarmLow->setData(FloatField, WidgetType);
  alarmLow->setData(alarmLow->isEditable(), Active);
  alarmLow->setData(dataset.alarmLow, EditableValue);
  alarmLow->setData(kDatasetView_AlarmLow, ParameterType);
  alarmLow->setData(tr("Low Threshold"), ParameterName);
  alarmLow->setData(tr("Triggers a visual alarm when the value drops below this threshold"),
                    ParameterDescription);
  model->appendRow(alarmLow);

  auto* alarmHigh = new QStandardItem();
  alarmHigh->setEditable(rangeEnabled && dataset.alarmEnabled);
  alarmHigh->setData(0, PlaceholderValue);
  alarmHigh->setData(FloatField, WidgetType);
  alarmHigh->setData(alarmHigh->isEditable(), Active);
  alarmHigh->setData(dataset.alarmHigh, EditableValue);
  alarmHigh->setData(kDatasetView_AlarmHigh, ParameterType);
  alarmHigh->setData(tr("High Threshold"), ParameterName);
  alarmHigh->setData(tr("Triggers a visual alarm when the value exceeds this threshold"),
                     ParameterDescription);
  model->appendRow(alarmHigh);
}

/**
 * @brief Appends the LED Display section rows (enable, high threshold) to @p model.
 * @param model    Target dataset form model.
 * @param dataset  Source dataset values.
 */
void DataModel::ProjectEditor::addLEDSection(CustomModel* model, const DataModel::Dataset& dataset)
{
  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("LED Display Settings"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/led.svg", ParameterIcon);
  model->appendRow(hdr);

  auto* ledItem = new QStandardItem();
  ledItem->setEditable(true);
  ledItem->setData(0, PlaceholderValue);
  ledItem->setData(CheckBox, WidgetType);
  ledItem->setData(ledItem->isEditable(), Active);
  ledItem->setData(dataset.led, EditableValue);
  ledItem->setData(kDatasetView_LED, ParameterType);
  ledItem->setData(tr("Show in LED Panel"), ParameterName);
  ledItem->setData(tr("Enable visual status monitoring using an LED display"),
                   ParameterDescription);
  model->appendRow(ledItem);

  auto* ledHigh = new QStandardItem();
  ledHigh->setEditable(dataset.led);
  ledHigh->setData(0, PlaceholderValue);
  ledHigh->setData(FloatField, WidgetType);
  ledHigh->setData(ledHigh->isEditable(), Active);
  ledHigh->setData(dataset.ledHigh, EditableValue);
  ledHigh->setData(kDatasetView_LED_High, ParameterType);
  ledHigh->setData(tr("LED On Threshold (required)"), ParameterName);
  ledHigh->setData(tr("LED lights up when value meets or exceeds this threshold"),
                   ParameterDescription);
  model->appendRow(ledHigh);
}

//--------------------------------------------------------------------------------------------------
// Private slot: combobox initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief (Re-)populates all combobox string lists used by the form models.
 *
 * Called once at construction and again whenever the application language
 * changes. Fills FFT sample counts, timer modes, decoder options, checksum
 * methods, frame detection methods, EOL sequences, group widgets, and
 * dataset widgets. Must be called before any build*Model() call to guarantee
 * that the lists are available for ComboBox rows.
 */
void DataModel::ProjectEditor::generateComboBoxModels()
{
  m_fftSamples.clear();
  m_fftSamples << "8" << "16" << "32" << "64" << "128" << "256" << "512"
               << "1024" << "2048" << "4096" << "8192" << "16384";

  m_timerModes.clear();
  m_timerModes << tr("Off") << tr("Auto Start") << tr("Start on Trigger")
               << tr("Toggle on Trigger");

  m_decoderOptions.clear();
  m_decoderOptions << tr("Plain Text (UTF8)") << tr("Hexadecimal") << tr("Base64")
                   << tr("Binary (Direct)");

  m_checksumMethods.clear();
  m_checksumMethods         = IO::availableChecksums();
  const int noChecksumIndex = m_checksumMethods.indexOf(QLatin1String(""));
  if (noChecksumIndex >= 0)
    m_checksumMethods[noChecksumIndex] = tr("No Checksum");

  m_frameDetectionMethods.clear();
  m_frameDetectionMethodsValues.clear();
  m_frameDetectionMethods << tr("End Delimiter Only") << tr("Start Delimiter Only")
                          << tr("Start + End Delimiter") << tr("No Delimiters");
  m_frameDetectionMethodsValues << SerialStudio::EndDelimiterOnly
                                << SerialStudio::StartDelimiterOnly
                                << SerialStudio::StartAndEndDelimiter << SerialStudio::NoDelimiters;

  m_groupWidgets.clear();
  m_groupWidgets.insert(QStringLiteral("datagrid"), tr("Data Grid"));
  m_groupWidgets.insert(QStringLiteral("map"), tr("GPS Map"));
  m_groupWidgets.insert(QStringLiteral("gyro"), tr("Gyroscope"));
  m_groupWidgets.insert(QStringLiteral("multiplot"), tr("Multiple Plot"));
  m_groupWidgets.insert(QStringLiteral("accelerometer"), tr("Accelerometer"));
  m_groupWidgets.insert(QStringLiteral("plot3d"), tr("3D Plot"));
#ifdef BUILD_COMMERCIAL
  m_groupWidgets.insert(QStringLiteral("image"), tr("Image View"));
#endif
  m_groupWidgets.insert(QLatin1String(""), tr("None"));

  m_datasetWidgets.clear();
  m_datasetWidgets.insert(QLatin1String(""), tr("None"));
  m_datasetWidgets.insert(QStringLiteral("bar"), tr("Bar"));
  m_datasetWidgets.insert(QStringLiteral("gauge"), tr("Gauge"));
  m_datasetWidgets.insert(QStringLiteral("compass"), tr("Compass"));

  m_eolSequences.clear();
  m_eolSequences.insert(QLatin1String(""), tr("None"));
  m_eolSequences.insert(QStringLiteral("\n"), tr("New Line (\\n)"));
  m_eolSequences.insert(QStringLiteral("\r"), tr("Carriage Return (\\r)"));
  m_eolSequences.insert(QStringLiteral("\r\n"), tr("CRLF (\\r\\n)"));

  m_plotOptions.clear();
  m_plotOptions.insert(qMakePair(false, false), tr("No"));
  m_plotOptions.insert(qMakePair(true, false), tr("Yes"));
}

//--------------------------------------------------------------------------------------------------
// Private slot: view management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Transitions the editor to @p view, prompting to save unsaved frame-parser
 *        changes when leaving FrameParserView.
 *
 * Emits currentViewChanged() and editableOptionsChanged() after the transition.
 * A no-op when @p view equals the current view.
 *
 * @param view  The target editor view.
 */
void DataModel::ProjectEditor::setCurrentView(const DataModel::ProjectEditor::CurrentView view)
{
  auto* parser = DataModel::FrameBuilder::instance().frameParser();
  if (parser && m_currentView == FrameParserView && m_currentView != view) {
    if (parser->isModified()) {
      bool changeView = false;
      const auto ret  = Misc::Utilities::showMessageBox(
        tr("Save changes to frame parser code?"),
        tr("Select 'Save' to keep your changes, 'Discard' to lose them permanently, or 'Cancel' "
            "to return."),
        QMessageBox::Question,
        tr("Save Changes"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

      if (ret == QMessageBox::Save)
        changeView = parser->save(true);
      else if (ret == QMessageBox::Discard) {
        changeView = true;
        parser->readCode();
      }

      if (!changeView)
        displayFrameParserView();
    }
  }

  m_currentView = view;
  Q_EMIT currentViewChanged();
}

//--------------------------------------------------------------------------------------------------
// Private slot: item changed handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles edits to the group form model.
 *
 * Reads the changed item's ParameterType role to identify which group
 * field was modified (title or widget), updates m_selectedGroup, and
 * propagates the change to ProjectModel via updateGroup() or setGroupWidget().
 * When setGroupWidget() returns false (user cancelled), the tree is rebuilt
 * and the group item is re-selected to keep the UI consistent.
 *
 * @param item  The QStandardItem that was edited.
 */
void DataModel::ProjectEditor::onGroupItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id      = static_cast<GroupItem>(item->data(ParameterType).toInt());
  const auto value   = item->data(EditableValue);
  auto& pm           = DataModel::ProjectModel::instance();
  const auto groupId = m_selectedGroup.groupId;

  if (id == kGroupView_Title) {
    if (m_selectedGroup.title == value.toString())
      return;

    m_selectedGroup.title = value.toString();
    pm.updateGroup(groupId, m_selectedGroup);
  } else if (id == kGroupView_Widget) {
    const auto keys      = m_groupWidgets.keys();
    const auto widgetStr = keys.at(value.toInt());

    static const QMap<QString, SerialStudio::GroupWidget> kWidgetEnumMap = {
      {"accelerometer", SerialStudio::Accelerometer},
      {    "multiplot",     SerialStudio::MultiPlot},
      {         "gyro",     SerialStudio::Gyroscope},
      {          "map",           SerialStudio::GPS},
      {     "datagrid",      SerialStudio::DataGrid},
      {       "plot3d",        SerialStudio::Plot3D},
#ifdef BUILD_COMMERCIAL
      {       "image",       SerialStudio::ImageView},
#endif
      {             "", SerialStudio::NoGroupWidget},
    };

    const auto widget = kWidgetEnumMap.value(widgetStr, SerialStudio::NoGroupWidget);
    if (!pm.setGroupWidget(groupId, widget)) {
      QTimer::singleShot(0, this, [this, groupId] {
        buildTreeModel();
        for (auto g = m_groupItems.begin(); g != m_groupItems.end(); ++g) {
          if (g.value().groupId != groupId)
            continue;

          if (m_selectionModel)
            m_selectionModel->setCurrentIndex(g.key()->index(),
                                              QItemSelectionModel::ClearAndSelect);
          break;
        }
      });
      return;
    }

    m_selectedGroup.widget = widgetStr;
#ifdef BUILD_COMMERCIAL
  } else if (id == kGroupView_ImgMode) {
    const QStringList kImgModeValues = {QStringLiteral("autodetect"), QStringLiteral("manual")};
    const int modeIdx                = value.toInt();
    if (modeIdx >= 0 && modeIdx < kImgModeValues.size()) {
      m_selectedGroup.imgDetectionMode = kImgModeValues.at(modeIdx);
      pm.updateGroup(groupId, m_selectedGroup);
      buildGroupModel(m_selectedGroup);
      return;
    }
  } else if (id == kGroupView_ImgStart) {
    m_selectedGroup.imgStartSequence = value.toString();
    pm.updateGroup(groupId, m_selectedGroup);
  } else if (id == kGroupView_ImgEnd) {
    m_selectedGroup.imgEndSequence = value.toString();
    pm.updateGroup(groupId, m_selectedGroup);
#endif
  }

  Q_EMIT editableOptionsChanged();
}

/**
 * @brief Handles edits to the action form model.
 *
 * Maps the changed item's ParameterType to the corresponding m_selectedAction
 * field and calls ProjectModel::updateAction() to persist the change.
 *
 * @param item  The QStandardItem that was edited.
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
    case kActionView_EOL:
      m_selectedAction.eolSequence = eolKeys.at(value.toInt());
      break;
    case kActionView_Icon:
      m_selectedAction.icon = value.toString();
      Q_EMIT actionModelChanged();
      break;
    case kActionView_Binary:
      m_selectedAction.binaryData = value.toBool();
      buildActionModel(m_selectedAction);
      break;
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
    default:
      break;
  }

  auto& pm            = DataModel::ProjectModel::instance();
  const auto actionId = m_selectedAction.actionId;
  pm.setSelectedAction(m_selectedAction);
  pm.updateAction(actionId, m_selectedAction);
}

/**
 * @brief Handles edits to the project-level settings form model.
 *
 * Dispatches title, frame delimiter, decoder method, hexadecimal delimiter,
 * frame detection, and checksum changes to the appropriate ProjectModel slots.
 * Rebuilds the project form after changes that affect row visibility
 * (frame detection mode, hexadecimal delimiter toggle).
 *
 * @param item  The QStandardItem that was edited.
 */
void DataModel::ProjectEditor::onProjectItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);
  auto& pm         = DataModel::ProjectModel::instance();

  switch (static_cast<ProjectItem>(id.toInt())) {
    case kProjectView_Title:
      pm.setTitle(value.toString());
      break;
    case kProjectView_FrameEndSequence:
      pm.setFrameEndSequence(value.toString());
      break;
    case kProjectView_FrameStartSequence:
      pm.setFrameStartSequence(value.toString());
      break;
    case kProjectView_FrameDecoder:
      pm.setDecoderMethod(static_cast<SerialStudio::DecoderMethod>(value.toInt()));
      break;
    case kProjectView_ChecksumFunction:
      pm.setChecksumAlgorithm(IO::availableChecksums().at(value.toInt()));
      break;
    case kProjectView_HexadecimalSequence:
      pm.setHexadecimalDelimiters(value.toBool());
      buildProjectModel();
      break;
    case kProjectView_FrameDetection:
      pm.setFrameDetection(m_frameDetectionMethodsValues.at(value.toInt()));
      buildProjectModel();
      break;
    default:
      break;
  }

  pm.setModified(true);
}

/**
 * @brief Handles edits to the dataset form model.
 *
 * Maps the changed item's ParameterType to the corresponding m_selectedDataset
 * field. Calls ProjectModel::updateDataset() with rebuildTree=true only when
 * the title or frame index changed (the only fields visible in the tree).
 * All other field edits update in-place without triggering a tree rebuild,
 * keeping the UI stable while the user types.
 *
 * @param item  The QStandardItem that was edited.
 */
void DataModel::ProjectEditor::onDatasetItemChanged(QStandardItem* item)
{
  if (!item)
    return;

  static QStringList datasetWidgetKeys;
  static QList<QPair<bool, bool>> plotOptionKeys;

  if (datasetWidgetKeys.isEmpty())
    for (auto i = m_datasetWidgets.begin(); i != m_datasetWidgets.end(); ++i)
      datasetWidgetKeys.append(i.key());

  if (plotOptionKeys.isEmpty())
    for (auto i = m_plotOptions.begin(); i != m_plotOptions.end(); ++i)
      plotOptionKeys.append(i.key());

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  switch (static_cast<DatasetItem>(id.toInt())) {
    case kDatasetView_Title:
      m_selectedDataset.title = value.toString();
      break;
    case kDatasetView_Index:
      m_selectedDataset.index = value.toInt();
      break;
    case kDatasetView_Units:
      m_selectedDataset.units = value.toString();
      break;
    case kDatasetView_Widget:
      m_selectedDataset.widget = datasetWidgetKeys.at(value.toInt());
      if (m_selectedDataset.widget == "compass") {
        m_selectedDataset.wgtMin    = 0;
        m_selectedDataset.wgtMax    = 360;
        m_selectedDataset.alarmLow  = 0;
        m_selectedDataset.alarmHigh = 0;
      }
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_FFT:
      m_selectedDataset.fft = value.toBool();
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_LED:
      m_selectedDataset.led = value.toBool();
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_LED_High:
      m_selectedDataset.ledHigh = value.toDouble();
      break;
    case kDatasetView_Overview:
      m_selectedDataset.overviewDisplay = value.toBool();
      break;
    case kDatasetView_Plot:
      m_selectedDataset.plt = plotOptionKeys.at(value.toInt()).first;
      m_selectedDataset.log = plotOptionKeys.at(value.toInt()).second;
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_xAxis:
      m_selectedDataset.xAxisId = value.toInt();
      break;
    case kDatasetView_FFTMin:
      m_selectedDataset.fftMin = value.toDouble();
      break;
    case kDatasetView_FFTMax:
      m_selectedDataset.fftMax = value.toDouble();
      break;
    case kDatasetView_PltMin:
      m_selectedDataset.pltMin = value.toDouble();
      break;
    case kDatasetView_PltMax:
      m_selectedDataset.pltMax = value.toDouble();
      break;
    case kDatasetView_WgtMin:
      m_selectedDataset.wgtMin = value.toDouble();
      break;
    case kDatasetView_WgtMax:
      m_selectedDataset.wgtMax = value.toDouble();
      break;
    case kDatasetView_AlarmLow:
      m_selectedDataset.alarmLow = value.toDouble();
      break;
    case kDatasetView_AlarmHigh:
      m_selectedDataset.alarmHigh = value.toDouble();
      break;
    case kDatasetView_AlarmEnabled:
      m_selectedDataset.alarmEnabled = value.toBool();
      buildDatasetModel(m_selectedDataset);
      break;
    case kDatasetView_FFT_Samples:
      m_selectedDataset.fftSamples = m_fftSamples.at(value.toInt()).toInt();
      break;
    case kDatasetView_FFT_SamplingRate:
      m_selectedDataset.fftSamplingRate = value.toInt();
      break;
    default:
      break;
  }

  auto& pm             = DataModel::ProjectModel::instance();
  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;
  const auto idInt     = static_cast<DatasetItem>(id.toInt());

  // Rebuild tree only when title or index changes (affects tree display).
  // All other field edits update in-place without triggering a tree rebuild.
  const bool rebuildTree = (idInt == kDatasetView_Title || idInt == kDatasetView_Index);
  pm.updateDataset(groupId, datasetId, m_selectedDataset, rebuildTree);

  Q_EMIT datasetOptionsChanged();
  Q_EMIT editableOptionsChanged();
}

//--------------------------------------------------------------------------------------------------
// Private slot: tree selection changed
//--------------------------------------------------------------------------------------------------

/**
 * @brief Responds to a new tree selection by switching the active editor view.
 *
 * Identifies the selected item against the four item maps (groups, datasets,
 * actions, root items) and calls the appropriate build*Model() function plus
 * setCurrentView(). Also notifies ProjectModel of the new selected proxy via
 * setSelectedGroup(), setSelectedDataset(), or setSelectedAction().
 *
 * @param current   Newly selected model index.
 * @param previous  Previously selected model index (unused).
 */
void DataModel::ProjectEditor::onCurrentSelectionChanged(const QModelIndex& current,
                                                         const QModelIndex& previous)
{
  (void)previous;

  if (!m_treeModel || !current.isValid())
    return;

  auto* item = m_treeModel->itemFromIndex(current);
  if (!item)
    return;

  if (m_groupItems.contains(item)) {
    const auto group = m_groupItems.value(item);
    DataModel::ProjectModel::instance().setSelectedGroup(group);
    setCurrentView(GroupView);
    buildGroupModel(group);
  } else if (m_datasetItems.contains(item)) {
    const auto dataset = m_datasetItems.value(item);
    DataModel::ProjectModel::instance().setSelectedDataset(dataset);
    setCurrentView(DatasetView);
    buildDatasetModel(dataset);
  } else if (m_actionItems.contains(item)) {
    const auto action = m_actionItems.value(item);
    DataModel::ProjectModel::instance().setSelectedAction(action);
    setCurrentView(ActionView);
    buildActionModel(action);
  } else if (m_rootItems.contains(item)) {
    const auto id = m_rootItems.value(item);
    if (id == kFrameParser)
      setCurrentView(FrameParserView);
    else {
      setCurrentView(ProjectView);
      buildProjectModel();
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Private helpers: expanded state persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Recursively records the TreeViewExpanded state of @p item and its children.
 *
 * Keys are slash-separated item text paths (e.g. "Root/Group A/Dataset 1").
 * Called before destroying the old tree model in buildTreeModel() so the
 * state can be restored on the freshly built model.
 *
 * @param item   Root item to start from.
 * @param map    Output map: path → expanded flag.
 * @param title  Accumulated path prefix for this call level.
 */
void DataModel::ProjectEditor::saveExpandedStateMap(QStandardItem* item,
                                                    QHash<QString, bool>& map,
                                                    const QString& title)
{
  if (!item)
    return;

  map[title] = item->data(TreeViewExpanded).toBool();

  for (auto i = 0; i < item->rowCount(); ++i) {
    QStandardItem* child = item->child(i);
    auto childT          = title.isEmpty() ? child->text() : title + "/" + child->text();
    saveExpandedStateMap(child, map, childT);
  }
}

/**
 * @brief Recursively restores the TreeViewExpanded state of @p item and its children.
 *
 * Looks up each item's path in @p map. Items not found in the map default to
 * expanded (true). Counterpart to saveExpandedStateMap().
 *
 * @param item   Root item to start from.
 * @param map    Saved path → expanded flag mapping from saveExpandedStateMap().
 * @param title  Accumulated path prefix for this call level.
 */
void DataModel::ProjectEditor::restoreExpandedStateMap(QStandardItem* item,
                                                       QHash<QString, bool>& map,
                                                       const QString& title)
{
  if (!item)
    return;

  if (map.contains(title))
    item->setData(map[title], TreeViewExpanded);
  else
    item->setData(true, TreeViewExpanded);
}
