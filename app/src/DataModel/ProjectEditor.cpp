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

#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Private enums to track which item the user selected/modified
//--------------------------------------------------------------------------------------------------

// clang-format off
typedef enum { kRootItem, kFrameParser } TopLevelItem;

typedef enum {
  kProjectView_Title
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
  kDatasetView_Overview,
  kDatasetView_TransformCode,
  kDatasetView_Virtual,
} DatasetItem;

typedef enum {
  kActionView_Title,
  kActionView_Icon,
  kActionView_EOL,
  kActionView_Data,
  kActionView_Binary,
  kActionView_TxEncoding,
  kActionView_SourceId,
  kActionView_AutoExecute,
  kActionView_TimerMode,
  kActionView_TimerInterval,
  kActionView_RepeatCount
} ActionItem;

typedef enum {
  kGroupView_Title,
  kGroupView_Widget,
  kGroupView_Source,
  kGroupView_ImgMode,
  kGroupView_ImgStart,
  kGroupView_ImgEnd,
  kGroupView_Columns
} GroupItem;

typedef enum {
  kSourceView_Title,
  kSourceView_BusType,
  kSourceView_Property,
  kSourceView_FrameParser,
  kSourceView_FrameDetection,
  kSourceView_HexadecimalSequence,
  kSourceView_FrameStartSequence,
  kSourceView_FrameEndSequence,
  kSourceView_FrameDecoder,
  kSourceView_ChecksumFunction
} SourceItem;

typedef enum {
  kOutputWidget_Title,
  kOutputWidget_Icon,
  kOutputWidget_MonoIcon,
  kOutputWidget_Type,
  kOutputWidget_MinValue,
  kOutputWidget_MaxValue,
  kOutputWidget_StepSize,
  kOutputWidget_InitialValue,
  kOutputWidget_TransmitFunction,
  kOutputWidget_TxEncoding
} OutputWidgetItem;

// clang-format on

static QString busTypeIcon(int busType)
{
  switch (static_cast<SerialStudio::BusType>(busType)) {
    case SerialStudio::BusType::UART:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/uart.svg");
    case SerialStudio::BusType::Network:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/network.svg");
    case SerialStudio::BusType::BluetoothLE:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/bluetooth.svg");
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/audio.svg");
    case SerialStudio::BusType::ModBus:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/modbus.svg");
    case SerialStudio::BusType::CanBus:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/canbus.svg");
    case SerialStudio::BusType::RawUsb:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/usb.svg");
    case SerialStudio::BusType::HidDevice:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/hid.svg");
    case SerialStudio::BusType::Process:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/process.svg");
#endif
    default:
      return QStringLiteral("qrc:/rcc/icons/devices/drivers/uart.svg");
  }
}

//--------------------------------------------------------------------------------------------------
// Constructor / singleton
//--------------------------------------------------------------------------------------------------

DataModel::ProjectEditor::ProjectEditor()
  : m_currentView(ProjectView)
  , m_groupsRootItem(nullptr)
  , m_tablesRootItem(nullptr)
  , m_systemDatasetsItem(nullptr)
  , m_workspacesRootItem(nullptr)
  , m_selectedWorkspaceId(-1)
  , m_treeModel(nullptr)
  , m_selectionModel(nullptr)
  , m_groupModel(nullptr)
  , m_sourceModel(nullptr)
  , m_actionModel(nullptr)
  , m_projectModel(nullptr)
  , m_datasetModel(nullptr)
  , m_outputWidgetModel(nullptr)
  , m_transformEditor(nullptr)
{
  generateComboBoxModels();

  // Coalesce rapid mutation bursts (keystrokes, batch edits) into one rebuild
  m_rebuildTimer.setSingleShot(true);
  m_rebuildTimer.setInterval(0);
  connect(&m_rebuildTimer, &QTimer::timeout, this, &DataModel::ProjectEditor::buildTreeModel);

  auto& pm = DataModel::ProjectModel::instance();

  connect(&pm,
          &DataModel::ProjectModel::groupsChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::actionsChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::tablesChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::workspacesChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(
    &pm,
    &DataModel::ProjectModel::sourcesChanged,
    this,
    [this] {
      scheduleTreeRebuild();

      if (m_currentView == GroupView)
        buildGroupModel(m_selectedGroup);
      else if (m_currentView == DatasetView)
        buildDatasetModel(m_selectedDataset);
    },
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
      if (!m_selectionModel)
        return;

      // Fall back to the "Groups" parent category (the deleted group's parent
      // in the tree) rather than jumping all the way to the project root.
      if (m_groupsRootItem) {
        m_selectionModel->setCurrentIndex(m_groupsRootItem->index(),
                                          QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
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

      if (!m_selectionModel)
        return;

      // Group was emptied and removed — fall back to the Groups parent node
      // rather than the project root.
      if (m_groupsRootItem) {
        m_selectionModel->setCurrentIndex(m_groupsRootItem->index(),
                                          QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
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

  connect(
    &pm,
    &DataModel::ProjectModel::outputWidgetAdded,
    this,
    [this](int groupId, int widgetId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
        if (it.value().groupId != groupId || it.value().widgetId != widgetId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        break;
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::outputWidgetDeleted,
    this,
    [this](int groupId) {
      for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
        if (it.value().groupId != groupId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        return;
      }

      if (m_selectionModel) {
        auto index = m_treeModel->index(0, 0);
        m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::sourceAdded,
    this,
    [this](int sourceId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_sourceItems.begin(); it != m_sourceItems.end(); ++it) {
        if (it.value().sourceId != sourceId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        break;
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::sourceDeleted,
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
      case SourceView:
        buildSourceModel(m_selectedSource);
        break;
      default:
        break;
    }
  });

  // Rebuild source model on bus type change so the form stays current
  connect(&IO::ConnectionManager::instance(), &IO::ConnectionManager::driverChanged, this, [this] {
    if (m_currentView != SourceView)
      return;

    const auto& sources = DataModel::ProjectModel::instance().sources();
    for (const auto& src : sources) {
      if (src.sourceId == m_selectedSource.sourceId) {
        m_selectedSource = src;
        break;
      }
    }

    buildSourceModel(m_selectedSource);
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
 * @brief Returns the icon identifier of the currently selected output widget.
 */
const QString DataModel::ProjectEditor::outputWidgetIcon() const
{
  return m_selectedOutputWidget.icon;
}

/**
 * @brief Returns a sorted list of all available action icon base-names.
 *
 * Scanned once from the embedded @c :/rcc/actions/ resource directory on
 * first call; subsequent calls return the cached list.
 */
const QStringList& DataModel::ProjectEditor::availableActionIcons() const
{
  // Scan and cache action icon names from embedded resources
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
 * @brief Returns the form model for the currently selected source.
 */
DataModel::CustomModel* DataModel::ProjectEditor::sourceModel() const
{
  return m_sourceModel;
}

/**
 * @brief Returns the sourceId of the currently selected source.
 */
int DataModel::ProjectEditor::selectedSourceId() const noexcept
{
  return m_selectedSource.sourceId;
}

/**
 * @brief Returns the bus type (int) of the currently selected source.
 */
int DataModel::ProjectEditor::selectedSourceBusType() const noexcept
{
  return m_selectedSource.busType;
}

/**
 * @brief Returns the JavaScript frame parser code of the currently selected source.
 */
QString DataModel::ProjectEditor::selectedSourceFrameParserCode() const
{
  return m_selectedSource.frameParserCode;
}

/**
 * @brief Updates the frame parser code of the selected source.
 * @param code The new JavaScript source string.
 */
void DataModel::ProjectEditor::setSelectedSourceFrameParserCode(const QString& code)
{
  if (m_selectedSource.frameParserCode == code)
    return;

  m_selectedSource.frameParserCode = code;
  DataModel::ProjectModel::instance().updateSourceFrameParser(m_selectedSource.sourceId, code);
  Q_EMIT selectedSourceFrameParserCodeChanged();
}

/**
 * @brief Opens the dataset value transform editor dialog for the currently
 * selected dataset.
 *
 * Pre-populates the editor with the dataset's existing transform code and
 * the source's scripting language. When the user clicks Apply, updates the
 * dataset and project model.
 */
void DataModel::ProjectEditor::openTransformEditor()
{
  // Determine the scripting language from the dataset's source
  int lang            = SerialStudio::Lua;
  const auto& sources = DataModel::ProjectModel::instance().sources();
  for (const auto& src : sources)
    if (src.sourceId == m_selectedDataset.sourceId) {
      lang = src.frameParserLanguage;
      break;
    }

  // Create the dialog on first use (avoids destruction-order crash)
  if (!m_transformEditor) {
    m_transformEditor = new DatasetTransformEditor(nullptr);

    connect(m_transformEditor,
            &DatasetTransformEditor::transformApplied,
            this,
            [this](const QString& code, int /*lang*/, int gId, int dId) {
              // Update the authoritative data in ProjectModel
              auto& pm     = DataModel::ProjectModel::instance();
              auto& groups = pm.groups();
              if (gId < 0 || static_cast<size_t>(gId) >= groups.size())
                return;

              if (dId < 0 || static_cast<size_t>(dId) >= groups[gId].datasets.size())
                return;

              // Build updated dataset from the authoritative source
              auto dataset          = groups[gId].datasets[dId];
              dataset.transformCode = code;
              pm.updateDataset(gId, dId, dataset, false);

              // Update the current selection if it matches
              if (m_selectedDataset.groupId == gId && m_selectedDataset.datasetId == dId)
                m_selectedDataset.transformCode = code;

              // Keep the tree-item cache in sync
              for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
                if (it.value().groupId == gId && it.value().datasetId == dId) {
                  it.value().transformCode = code;
                  break;
                }
              }

              // Re-sync FrameBuilder to recompile transform engines
              DataModel::FrameBuilder::instance().syncFromProjectModel();
            });
  }

  m_transformEditor->displayDialog(m_selectedDataset.title,
                                   m_selectedDataset.transformCode,
                                   lang,
                                   m_selectedDataset.groupId,
                                   m_selectedDataset.datasetId);
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

/**
 * @brief Returns the type enum value of the currently selected output widget.
 */
int DataModel::ProjectEditor::outputWidgetType() const noexcept
{
  return static_cast<int>(m_selectedOutputWidget.type);
}

/**
 * @brief Returns true when the selected group is an OutputPanel.
 */
bool DataModel::ProjectEditor::currentGroupIsOutputPanel() const
{
  if (m_currentView == GroupView || m_currentView == OutputWidgetView)
    return m_selectedGroup.groupType == DataModel::GroupType::Output;

  return false;
}

/**
 * @brief Returns the form model for the currently selected output widget.
 */
DataModel::CustomModel* DataModel::ProjectEditor::outputWidgetModel() const
{
  return m_outputWidgetModel;
}

/**
 * @brief Returns a const ref to the currently selected output widget.
 */
const DataModel::OutputWidget& DataModel::ProjectEditor::selectedOutputWidget() const noexcept
{
  return m_selectedOutputWidget;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects the Frame Parser item for source 0 in the tree after a short delay.
 *
 * The delay allows any pending tree rebuild to complete before the selection
 * is applied, ensuring the item exists in the model.
 */
void DataModel::ProjectEditor::displayFrameParserView(int sourceId)
{
  QTimer::singleShot(100, this, [this, sourceId] {
    if (!m_selectionModel)
      return;

    for (auto it = m_sourceParserItems.begin(); it != m_sourceParserItems.end(); ++it) {
      if (it.value().sourceId != sourceId)
        continue;

      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      break;
    }
  });
}

//--------------------------------------------------------------------------------------------------
// Model builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Coalesces rapid ProjectModel mutation bursts into a single rebuild.
 *
 * Many ProjectModel signals (groupsChanged, actionsChanged, tablesChanged,
 * workspacesChanged) fire once per keystroke during batch edits. Connecting
 * them directly to buildTreeModel() enqueues one rebuild per emission, even
 * when they all resolve on the same event-loop tick. Routing through this
 * single-shot zero-interval timer collapses the burst into one rebuild.
 */
void DataModel::ProjectEditor::scheduleTreeRebuild()
{
  if (!m_rebuildTimer.isActive())
    m_rebuildTimer.start();
}

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
  // Clear all item maps for the fresh rebuild
  m_rootItems.clear();
  m_groupItems.clear();
  m_sourceItems.clear();
  m_actionItems.clear();
  m_datasetItems.clear();
  m_outputWidgetItems.clear();
  m_sourceParserItems.clear();
  m_userTableItems.clear();
  m_workspaceItems.clear();
  m_groupsRootItem     = nullptr;
  m_tablesRootItem     = nullptr;
  m_systemDatasetsItem = nullptr;
  m_workspacesRootItem = nullptr;

  // Save expanded state before destroying the old model
  QHash<QString, bool> expandedStates;
  if (m_treeModel)
    saveExpandedStateMap(m_treeModel->invisibleRootItem(), expandedStates, "");

  // Disconnect only our captured lambda/slot connection, not every signal
  if (m_currentSelectionConnection) {
    QObject::disconnect(m_currentSelectionConnection);
    m_currentSelectionConnection = QMetaObject::Connection();
  }

  // Dispose of the previous selection model
  if (m_selectionModel) {
    m_selectionModel->deleteLater();
    m_selectionModel = nullptr;
  }

  if (m_treeModel) {
    disconnect(m_treeModel);
    m_treeModel->deleteLater();
    m_treeModel = nullptr;
  }

  // Create fresh model and root item
  m_treeModel = new CustomModel(this);

  const auto& pm = DataModel::ProjectModel::instance();
  auto* root     = new QStandardItem(pm.title());
  root->setData(root->text(), TreeViewText);
  root->setData("qrc:/rcc/icons/project-editor/treeview/project-setup.svg", TreeViewIcon);
  root->setData(true, TreeViewExpanded);

  m_treeModel->appendRow(root);
  m_rootItems.insert(root, kRootItem);

  // Populate sources, actions, groups, datasets, output widgets
  buildTreeItems(root, expandedStates);

  // Connect the selection model and emit the new tree
  m_selectionModel             = new QItemSelectionModel(m_treeModel);
  m_currentSelectionConnection = connect(m_selectionModel,
                                         &QItemSelectionModel::currentChanged,
                                         this,
                                         &DataModel::ProjectEditor::onCurrentSelectionChanged);

  Q_EMIT treeModelChanged();

  // Restore the previously selected item by matching IDs
  restoreTreeSelection();
}

/**
 * @brief Populates the tree model with source, action, group, dataset, and
 * output widget items under the given @p root.
 */
void DataModel::ProjectEditor::buildTreeItems(QStandardItem* root,
                                              QHash<QString, bool>& expandedStates)
{
  Q_ASSERT(root != nullptr);

  const auto& pm         = DataModel::ProjectModel::instance();
  const auto& groups     = pm.groups();
  const auto& actions    = pm.actions();
  const auto& sources    = pm.sources();
  const bool multiSource = sources.size() > 1;

  // Tree search query — when non-empty, limit items to those whose title (or
  // one of their descendants' titles) matches the query (case-insensitive).
  const QString q         = m_treeSearchQuery.trimmed();
  const bool filterActive = !q.isEmpty();
  const auto matches      = [&q](const QString& s) {
    return s.contains(q, Qt::CaseInsensitive);
  };

  // Add source items with their frame parser children (skip when filtering —
  // search only spans actions, groups, and datasets)
  if (!filterActive) {
    for (const auto& source : sources) {
      auto* sourceItem = new QStandardItem(source.title);
      sourceItem->setData(-1, TreeViewFrameIndex);
      sourceItem->setData(busTypeIcon(source.busType), TreeViewIcon);
      sourceItem->setData(source.title, TreeViewText);
      sourceItem->setData(source.sourceId, TreeViewSourceId);
      sourceItem->setData(multiSource ? source.title : QString(), TreeViewSourceName);
      sourceItem->setData(true, TreeViewExpanded);

      auto* parserItem = new QStandardItem(tr("Frame Parser"));
      parserItem->setData(-1, TreeViewFrameIndex);
      parserItem->setData("qrc:/rcc/icons/project-editor/treeview/code.svg", TreeViewIcon);
      parserItem->setData(tr("Frame Parser"), TreeViewText);
      sourceItem->appendRow(parserItem);
      m_sourceParserItems.insert(parserItem, source);

      root->appendRow(sourceItem);
      m_sourceItems.insert(sourceItem, source);
    }
  }

  // Add action items (filtered by title when search is active)
  for (const auto& action : actions) {
    if (filterActive && !matches(action.title))
      continue;

    auto* actionItem = new QStandardItem(action.title);
    actionItem->setData(-1, TreeViewFrameIndex);
    actionItem->setData("qrc:/rcc/icons/project-editor/treeview/action.svg", TreeViewIcon);
    actionItem->setData(action.title, TreeViewText);
    root->appendRow(actionItem);
    m_actionItems.insert(actionItem, action);
  }

  // Build the "Groups" category parent — collects every group item as a child
  // so the whole section can be collapsed/expanded as a unit. Created lazily
  // so that an empty project (or a filter that hides all groups) doesn't leave
  // a dangling empty parent node in the tree.
  QStandardItem* groupsRoot   = nullptr;
  const auto ensureGroupsRoot = [&]() {
    if (groupsRoot)
      return;

    groupsRoot = new QStandardItem(tr("Groups"));
    groupsRoot->setData(tr("Groups"), TreeViewText);
    groupsRoot->setData("qrc:/rcc/icons/project-editor/treeview/group.svg", TreeViewIcon);
    groupsRoot->setData(-1, TreeViewFrameIndex);
    groupsRoot->setData(true, TreeViewExpanded);
  };

  // Add group items with their dataset and output widget children
  for (const auto& group : groups) {
    // Determine which datasets in this group match the query — a group is
    // included if its own title matches OR at least one of its datasets matches.
    const bool groupMatches = !filterActive || matches(group.title);
    bool anyDatasetMatches  = false;
    if (filterActive && !groupMatches) {
      for (const auto& ds : group.datasets) {
        if (matches(ds.title)) {
          anyDatasetMatches = true;
          break;
        }
      }

      if (!anyDatasetMatches)
        continue;
    }

    auto* groupItem = new QStandardItem(group.title);
    auto icon = SerialStudio::dashboardWidgetIcon(SerialStudio::getDashboardWidget(group), false);

    groupItem->setData(icon, TreeViewIcon);
    groupItem->setData(-1, TreeViewFrameIndex);
    groupItem->setData(group.title, TreeViewText);
    groupItem->setData(QString(), TreeViewSourceName);
    groupItem->setData(group.sourceId, TreeViewSourceId);

    // Force-expand groups that matched via a descendant when filtering
    if (filterActive)
      groupItem->setData(true, TreeViewExpanded);

    // Add dataset children — if the group itself matched, show all datasets;
    // otherwise only show the datasets that individually match the query.
    for (const auto& dataset : group.datasets) {
      if (filterActive && !groupMatches && !matches(dataset.title))
        continue;

      auto* datasetItem = new QStandardItem(dataset.title);
      auto widgets      = SerialStudio::getDashboardWidgets(dataset);
      QString dIcon     = "qrc:/rcc/icons/project-editor/treeview/dataset.svg";
      if (widgets.count() > 0)
        dIcon = SerialStudio::dashboardWidgetIcon(widgets.first(), false);

      datasetItem->setData(dIcon, TreeViewIcon);
      datasetItem->setData(dataset.title, TreeViewText);
      datasetItem->setData(dataset.index, TreeViewFrameIndex);
      datasetItem->setData(dataset.sourceId, TreeViewSourceId);
      datasetItem->setData(QString(), TreeViewSourceName);
      groupItem->appendRow(datasetItem);
      m_datasetItems.insert(datasetItem, dataset);
    }

    // Add output widget children (skip when filtering — search is scoped to
    // actions, groups, and datasets)
    for (const auto& ow : group.outputWidgets) {
      if (filterActive)
        break;

      auto* owItem = new QStandardItem(ow.title);

      QString owIcon;
      switch (ow.type) {
        case DataModel::OutputWidgetType::Button:
          owIcon = QStringLiteral("qrc:/rcc/icons/project-editor/treeview/output-button.svg");
          break;
        case DataModel::OutputWidgetType::Slider:
          owIcon = QStringLiteral("qrc:/rcc/icons/project-editor/treeview/output-slider.svg");
          break;
        case DataModel::OutputWidgetType::Toggle:
          owIcon = QStringLiteral("qrc:/rcc/icons/project-editor/treeview/output-toggle.svg");
          break;
        case DataModel::OutputWidgetType::TextField:
          owIcon = QStringLiteral("qrc:/rcc/icons/project-editor/treeview/output-textfield.svg");
          break;
        case DataModel::OutputWidgetType::Knob:
          owIcon = QStringLiteral("qrc:/rcc/icons/project-editor/treeview/output-knob.svg");
          break;
        case DataModel::OutputWidgetType::RampGenerator:
          owIcon = QStringLiteral("qrc:/rcc/icons/project-editor/treeview/output-ramp.svg");
          break;
        default:
          owIcon = QStringLiteral("qrc:/rcc/icons/project-editor/treeview/output-widget.svg");
          break;
      }

      owItem->setData(owIcon, TreeViewIcon);
      owItem->setData(ow.title, TreeViewText);
      owItem->setData(-2, TreeViewFrameIndex);
      owItem->setData(ow.sourceId, TreeViewSourceId);
      owItem->setData(QString(), TreeViewSourceName);
      groupItem->appendRow(owItem);
      m_outputWidgetItems.insert(owItem, ow);
    }

    ensureGroupsRoot();
    const QString gPath = root->text() + "/" + tr("Groups") + "/" + group.title;
    restoreExpandedStateMap(groupItem, expandedStates, gPath);
    groupsRoot->appendRow(groupItem);
    m_groupItems.insert(groupItem, group);
  }

  if (groupsRoot) {
    restoreExpandedStateMap(groupsRoot, expandedStates, root->text() + "/" + groupsRoot->text());
    root->appendRow(groupsRoot);
    m_groupsRootItem = groupsRoot;
  }

  // Add "Shared Memory" category root (Pro only). When a tree search is
  // active, include it only if the root name, "Dataset Values", or any
  // user-table name matches.
#ifdef BUILD_COMMERCIAL
  const auto& userTables = pm.tables();
  bool includeSharedRoot =
    !filterActive || matches(tr("Shared Memory")) || matches(tr("Dataset Values"));
  if (!includeSharedRoot) {
    for (const auto& t : userTables) {
      if (matches(t.name)) {
        includeSharedRoot = true;
        break;
      }
    }
  }

  if (includeSharedRoot) {
    auto* tablesRoot = new QStandardItem(tr("Shared Memory"));
    tablesRoot->setData(tr("Shared Memory"), TreeViewText);
    tablesRoot->setData("qrc:/rcc/icons/project-editor/treeview/shared-memory.svg", TreeViewIcon);
    tablesRoot->setData(-1, TreeViewFrameIndex);
    tablesRoot->setData(true, TreeViewExpanded);

    // Add the read-only "Dataset Values" entry (auto-generated from the project)
    auto* sysDsItem = new QStandardItem(tr("Dataset Values"));
    sysDsItem->setData(tr("Dataset Values"), TreeViewText);
    sysDsItem->setData("qrc:/rcc/icons/project-editor/treeview/dataset-values.svg", TreeViewIcon);
    sysDsItem->setData(-1, TreeViewFrameIndex);
    tablesRoot->appendRow(sysDsItem);

    // Add user-defined shared tables as children (filtered by search query)
    for (const auto& table : userTables) {
      if (filterActive && !matches(table.name))
        continue;

      auto* tableItem = new QStandardItem(table.name);
      tableItem->setData(table.name, TreeViewText);
      tableItem->setData("qrc:/rcc/icons/project-editor/treeview/shared-table.svg", TreeViewIcon);
      tableItem->setData(-1, TreeViewFrameIndex);
      tablesRoot->appendRow(tableItem);
      m_userTableItems.insert(tableItem, table.name);
    }

    restoreExpandedStateMap(tablesRoot, expandedStates, root->text() + "/" + tablesRoot->text());
    root->appendRow(tablesRoot);
    m_tablesRootItem     = tablesRoot;
    m_systemDatasetsItem = sysDsItem;
  }
#endif  // BUILD_COMMERCIAL

  // Add "Workspaces" category root. Hidden when a tree search is active
  // unless it or one of its children matches.
  const auto& workspaces = pm.workspaces();
  bool includeWorkspaces = !filterActive || matches(tr("Workspaces"));
  if (!includeWorkspaces) {
    for (const auto& ws : workspaces) {
      if (matches(ws.title)) {
        includeWorkspaces = true;
        break;
      }
    }
  }

  if (includeWorkspaces) {
    auto* wsRoot = new QStandardItem(tr("Workspaces"));
    wsRoot->setData(tr("Workspaces"), TreeViewText);
    wsRoot->setData("qrc:/rcc/icons/project-editor/treeview/datagrid.svg", TreeViewIcon);
    wsRoot->setData(-1, TreeViewFrameIndex);
    wsRoot->setData(true, TreeViewExpanded);

    for (const auto& ws : workspaces) {
      if (filterActive && !matches(ws.title))
        continue;

      auto* wsItem = new QStandardItem(ws.title);
      wsItem->setData(ws.title, TreeViewText);
      wsItem->setData(ws.icon.isEmpty() ? "qrc:/rcc/icons/project-editor/treeview/group.svg"
                                        : ws.icon,
                      TreeViewIcon);
      wsItem->setData(-1, TreeViewFrameIndex);
      wsRoot->appendRow(wsItem);
      m_workspaceItems.insert(wsItem, ws.workspaceId);
    }

    restoreExpandedStateMap(wsRoot, expandedStates, root->text() + "/" + wsRoot->text());
    root->appendRow(wsRoot);
    m_workspacesRootItem = wsRoot;
  }

  // Add spacer item at the end of the tree
  auto* spacer = new QStandardItem(" ");
  spacer->setData(" ", TreeViewText);
  spacer->setData("", TreeViewIcon);
  spacer->setData(-1, TreeViewFrameIndex);
  spacer->setEnabled(false);
  spacer->setSelectable(false);
  root->appendRow(spacer);
}

/**
 * @brief Restores the tree selection to the previously active item by matching
 * IDs against the current view state.
 */
void DataModel::ProjectEditor::restoreTreeSelection()
{
  QStandardItem* toSelect = nullptr;

  // Match the current view's selected item against the rebuilt tree
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
  } else if (m_currentView == SourceView) {
    const auto sid = m_selectedSource.sourceId;
    for (auto it = m_sourceItems.begin(); it != m_sourceItems.end(); ++it) {
      if (it.value().sourceId == sid) {
        toSelect = it.key();
        break;
      }
    }
  } else if (m_currentView == OutputWidgetView) {
    const auto gid = m_selectedOutputWidget.groupId;
    const auto wid = m_selectedOutputWidget.widgetId;
    for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
      if (it.value().groupId == gid && it.value().widgetId == wid) {
        toSelect = it.key();
        break;
      }
    }
  } else if (m_currentView == DataTablesView) {
    toSelect = m_tablesRootItem;
  } else if (m_currentView == SystemDatasetsView) {
    toSelect = m_systemDatasetsItem;
  } else if (m_currentView == UserTableView) {
    for (auto it = m_userTableItems.begin(); it != m_userTableItems.end(); ++it) {
      if (it.value() == m_selectedUserTable) {
        toSelect = it.key();
        break;
      }
    }

    // Table was deleted — fall back to its parent category rather than the
    // project root.
    if (!toSelect)
      toSelect = m_tablesRootItem;
  } else if (m_currentView == WorkspacesView) {
    toSelect = m_workspacesRootItem;
  } else if (m_currentView == WorkspaceView) {
    for (auto it = m_workspaceItems.begin(); it != m_workspaceItems.end(); ++it) {
      if (it.value() == m_selectedWorkspaceId) {
        toSelect = it.key();
        break;
      }
    }

    // Workspace was deleted — fall back to the Workspaces category.
    if (!toSelect)
      toSelect = m_workspacesRootItem;
  }

  // Fall back to root project item when no match found
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
  // Dispose of the previous model and create a fresh one
  if (m_projectModel) {
    disconnect(m_projectModel);
    m_projectModel->deleteLater();
  }

  m_projectModel = new CustomModel(this);
  const auto& pm = DataModel::ProjectModel::instance();

  // Project Information section
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
  // Dispose of the previous model and create a fresh one
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

  const auto& sources    = DataModel::ProjectModel::instance().sources();
  const bool multiSource = sources.size() > 1;

  if (multiSource) {
    QStringList sourceLabels;
    for (const auto& src : sources)
      sourceLabels.append(src.title.isEmpty() ? tr("Device %1").arg(QChar('A' + src.sourceId))
                                              : src.title);

    int sourceIndex = 0;
    for (int i = 0; i < static_cast<int>(sources.size()); ++i) {
      if (sources[i].sourceId == group.sourceId) {
        sourceIndex = i;
        break;
      }
    }

    auto* sourceItem = new QStandardItem();
    sourceItem->setEditable(true);
    sourceItem->setData(true, Active);
    sourceItem->setData(ComboBox, WidgetType);
    sourceItem->setData(sourceLabels, ComboBoxData);
    sourceItem->setData(sourceIndex, EditableValue);
    sourceItem->setData(kGroupView_Source, ParameterType);
    sourceItem->setData(tr("Input Device"), ParameterName);
    sourceItem->setData(tr("Select which connected device provides data for this group"),
                        ParameterDescription);
    m_groupModel->appendRow(sourceItem);
  }

  // Composite widget selector (hidden for output groups)
  if (group.groupType != DataModel::GroupType::Output) {
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
  }

  // Image View configuration fields (pro only, shown when widget == "image")
#ifdef BUILD_COMMERCIAL
  if (group.widget == QLatin1String("image")) {
    auto* imgHdr = new QStandardItem();
    imgHdr->setData(SectionHeader, WidgetType);
    imgHdr->setData(tr("Image Configuration"), PlaceholderValue);
    imgHdr->setData("qrc:/rcc/icons/project-editor/model/image.svg", ParameterIcon);
    m_groupModel->appendRow(imgHdr);

    static const QStringList kImgModeValues = {QStringLiteral("autodetect"),
                                               QStringLiteral("manual")};

    int modeIndex = group.imgDetectionMode == QLatin1String("manual") ? 1 : 0;

    auto* modeItem = new QStandardItem();
    modeItem->setEditable(true);
    modeItem->setData(true, Active);
    modeItem->setData(ComboBox, WidgetType);
    modeItem->setData(m_imgDetectionModes, ComboBoxData);
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
    startItem->setData(tr("Hex bytes marking the start of an image frame"), ParameterDescription);
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
 * @brief Rebuilds the source-settings form model for @p source.
 *
 * Queries the live driver via SourceManager::driverForEditing() to get a
 * flat list of DriverProperty descriptors, then populates m_sourceModel with
 * one row per property (plus a bus-type ComboBox at the top). QML renders this
 * through the same TableDelegate used by group/dataset/action views.
 *
 * @param source The source whose properties should be displayed.
 */
void DataModel::ProjectEditor::buildSourceModel(const DataModel::Source& source)
{
  // Dispose of the previous model and create a fresh one
  if (m_sourceModel) {
    disconnect(m_sourceModel);
    m_sourceModel->deleteLater();
  }

  m_selectedSource = source;
  m_sourceModel    = new CustomModel(this);

  auto* identHdr = new QStandardItem();
  identHdr->setData(SectionHeader, WidgetType);
  identHdr->setData(tr("Identity"), PlaceholderValue);
  identHdr->setData("qrc:/rcc/icons/project-editor/model/project.svg", ParameterIcon);
  m_sourceModel->appendRow(identHdr);

  auto* titleItem = new QStandardItem();
  titleItem->setEditable(true);
  titleItem->setData(true, Active);
  titleItem->setData(TextField, WidgetType);
  titleItem->setData(source.title, EditableValue);
  titleItem->setData(kSourceView_Title, ParameterType);
  titleItem->setData(tr("Device Name"), ParameterName);
  titleItem->setData(tr("Device 1"), PlaceholderValue);
  titleItem->setData(tr("Human-readable name for this input device"), ParameterDescription);
  m_sourceModel->appendRow(titleItem);

  auto* hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Input Device"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/project.svg", ParameterIcon);
  m_sourceModel->appendRow(hdr);

  auto* busItem = new QStandardItem();
  busItem->setEditable(true);
  busItem->setData(true, Active);
  busItem->setData(ComboBox, WidgetType);
  busItem->setData(kSourceView_BusType, ParameterType);
  busItem->setData(source.busType, EditableValue);
  busItem->setData(tr("Bus Type"), ParameterName);
  busItem->setData(tr("Select the hardware interface for this input device"), ParameterDescription);

  QStringList busTypes = {tr("Serial Port"), tr("Network Socket"), tr("Bluetooth LE")};
#ifdef BUILD_COMMERCIAL
  busTypes << tr("Audio Input") << tr("Modbus") << tr("CAN Bus") << tr("Raw USB")
           << tr("HID Device") << tr("Process");
#endif

  busItem->setData(busTypes, ComboBoxData);
  m_sourceModel->appendRow(busItem);

  // BLE connection is configured at runtime via the Setup panel, not the editor
  if (source.busType != static_cast<int>(SerialStudio::BusType::BluetoothLE)) {
    auto* driverHdr = new QStandardItem();
    driverHdr->setData(SectionHeader, WidgetType);
    driverHdr->setData(tr("Connection Settings"), PlaceholderValue);
    driverHdr->setData(busTypeIcon(source.busType), ParameterIcon);
    m_sourceModel->appendRow(driverHdr);

    IO::HAL_Driver* driver = IO::ConnectionManager::instance().driverForEditing(source.sourceId);
    if (driver) {
      const auto props = driver->driverProperties();
      for (const auto& prop : props) {
        auto* item = new QStandardItem();
        item->setEditable(true);
        item->setData(true, Active);
        item->setData(prop.key, ParameterKey);
        item->setData(kSourceView_Property, ParameterType);
        item->setData(prop.label, ParameterName);

        if (!prop.description.isEmpty())
          item->setData(prop.description, ParameterDescription);

        switch (prop.type) {
          case IO::DriverProperty::Text:
            item->setData(TextField, WidgetType);
            break;
          case IO::DriverProperty::HexText:
            item->setData(HexTextField, WidgetType);
            break;
          case IO::DriverProperty::IntField:
            item->setData(IntField, WidgetType);
            break;
          case IO::DriverProperty::FloatField:
            item->setData(FloatField, WidgetType);
            break;
          case IO::DriverProperty::CheckBox:
            item->setData(CheckBox, WidgetType);
            break;
          case IO::DriverProperty::ComboBox:
            item->setData(ComboBox, WidgetType);
            item->setData(prop.options, ComboBoxData);
            break;
        }

        item->setData(prop.value, EditableValue);
        m_sourceModel->appendRow(item);
      }
    }
  }

  // Frame Detection section
  auto* fdHdr = new QStandardItem();
  fdHdr->setData(SectionHeader, WidgetType);
  fdHdr->setData(tr("Frame Detection"), PlaceholderValue);
  fdHdr->setData("qrc:/rcc/icons/project-editor/model/frame-detection.svg", ParameterIcon);
  m_sourceModel->appendRow(fdHdr);

  const auto detection     = static_cast<SerialStudio::FrameDetection>(source.frameDetection);
  const bool hexDelimiters = source.hexadecimalDelimiters;
  const auto frameStart    = source.frameStart;
  const auto frameEnd      = source.frameEnd;

  auto* frameDetectionItem = new QStandardItem();
  frameDetectionItem->setEditable(true);
  frameDetectionItem->setData(true, Active);
  frameDetectionItem->setData(ComboBox, WidgetType);
  frameDetectionItem->setData(m_frameDetectionMethods, ComboBoxData);
  frameDetectionItem->setData(m_frameDetectionMethodsValues.indexOf(detection), EditableValue);
  frameDetectionItem->setData(kSourceView_FrameDetection, ParameterType);
  frameDetectionItem->setData(tr("Frame Detection Method"), ParameterName);
  frameDetectionItem->setData(tr("Select how incoming data frames are identified"),
                              ParameterDescription);
  m_sourceModel->appendRow(frameDetectionItem);

  auto* hexSeqItem = new QStandardItem();
  hexSeqItem->setEditable(true);
  hexSeqItem->setData(true, Active);
  hexSeqItem->setData(CheckBox, WidgetType);
  hexSeqItem->setData(hexDelimiters, EditableValue);
  hexSeqItem->setData(kSourceView_HexadecimalSequence, ParameterType);
  hexSeqItem->setData(tr("Hexadecimal Delimiters"), ParameterName);
  hexSeqItem->setData(tr("Enter frame start/end sequences as hexadecimal values"),
                      ParameterDescription);
  m_sourceModel->appendRow(hexSeqItem);

  const bool showStart = (detection == SerialStudio::StartDelimiterOnly
                          || detection == SerialStudio::StartAndEndDelimiter);
  const bool showEnd   = (detection == SerialStudio::EndDelimiterOnly
                        || detection == SerialStudio::StartAndEndDelimiter);

  if (showStart) {
    auto* startSeqItem = new QStandardItem();
    startSeqItem->setEditable(true);
    startSeqItem->setData(true, Active);
    startSeqItem->setData(hexDelimiters ? HexTextField : TextField, WidgetType);
    startSeqItem->setData(frameStart, EditableValue);
    startSeqItem->setData(kSourceView_FrameStartSequence, ParameterType);
    startSeqItem->setData(tr("Frame Start Delimiter"), ParameterName);
    startSeqItem->setData(tr("e.g. /*"), PlaceholderValue);
    startSeqItem->setData(tr("Sequence that marks the beginning of a data frame"),
                          ParameterDescription);
    m_sourceModel->appendRow(startSeqItem);
  }

  if (showEnd) {
    auto* endSeqItem = new QStandardItem();
    endSeqItem->setEditable(true);
    endSeqItem->setData(true, Active);
    endSeqItem->setData(hexDelimiters ? HexTextField : TextField, WidgetType);
    endSeqItem->setData(frameEnd, EditableValue);
    endSeqItem->setData(kSourceView_FrameEndSequence, ParameterType);
    endSeqItem->setData(tr("Frame End Delimiter"), ParameterName);
    endSeqItem->setData(tr("e.g. */"), PlaceholderValue);
    endSeqItem->setData(tr("Sequence that marks the end of a data frame"), ParameterDescription);
    m_sourceModel->appendRow(endSeqItem);
  }

  // Payload Processing section
  auto* ppHdr = new QStandardItem();
  ppHdr->setData(SectionHeader, WidgetType);
  ppHdr->setData(tr("Payload Processing & Validation"), PlaceholderValue);
  ppHdr->setData("qrc:/rcc/icons/project-editor/model/data-conversion.svg", ParameterIcon);
  m_sourceModel->appendRow(ppHdr);

  auto* decoderItem = new QStandardItem();
  decoderItem->setEditable(true);
  decoderItem->setData(true, Active);
  decoderItem->setData(ComboBox, WidgetType);
  decoderItem->setData(m_decoderOptions, ComboBoxData);
  decoderItem->setData(source.decoderMethod, EditableValue);
  decoderItem->setData(kSourceView_FrameDecoder, ParameterType);
  decoderItem->setData(tr("Data Conversion Method"), ParameterName);
  decoderItem->setData(tr("Select how incoming binary data is decoded before parsing"),
                       ParameterDescription);
  m_sourceModel->appendRow(decoderItem);

  const auto availableChecksums = IO::availableChecksums();
  int checksumIdx               = availableChecksums.indexOf(source.checksumAlgorithm);
  if (checksumIdx < 0)
    checksumIdx = 0;

  auto* checksumItem = new QStandardItem();
  checksumItem->setEditable(true);
  checksumItem->setData(true, Active);
  checksumItem->setData(ComboBox, WidgetType);
  checksumItem->setData(m_checksumMethods, ComboBoxData);
  checksumItem->setData(checksumIdx, EditableValue);
  checksumItem->setData(kSourceView_ChecksumFunction, ParameterType);
  checksumItem->setData(tr("Checksum Algorithm"), ParameterName);
  checksumItem->setData(tr("Select the checksum algorithm used to validate frames"),
                        ParameterDescription);
  m_sourceModel->appendRow(checksumItem);

  connect(
    m_sourceModel, &CustomModel::itemChanged, this, &DataModel::ProjectEditor::onSourceItemChanged);

  // Rebuild source form when device lists change so ComboBox options stay current
  if (m_deviceListConn)
    disconnect(m_deviceListConn);

  m_deviceListConn = connect(
    &IO::ConnectionManager::instance(),
    &IO::ConnectionManager::deviceListRefreshed,
    this,
    [this]() { buildSourceModel(m_selectedSource); },
    Qt::QueuedConnection);

  Q_EMIT sourceModelChanged();
}

/**
 * @brief Handles edits from the source settings form model.
 *
 * Dispatches changes to either ProjectModel (for bus-type changes) or the live
 * driver via setDriverProperty() (for all other connection parameters). After a
 * driver property change the updated values are captured back into
 * Source::connectionSettings so they survive project save/load.
 *
 * @param item The QStandardItem whose data was changed.
 */
void DataModel::ProjectEditor::onSourceItemChanged(QStandardItem* item)
{
  // Validate item and read the parameter type
  if (!item)
    return;

  const int id = item->data(ParameterType).toInt();

  if (id == kSourceView_Title) {
    const QString newTitle = item->data(EditableValue).toString();
    if (m_selectedSource.title == newTitle)
      return;

    m_selectedSource.title = newTitle;
    DataModel::ProjectModel::instance().updateSourceTitle(m_selectedSource.sourceId, newTitle);

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
    return;
  }

  if (id == kSourceView_BusType) {
    const int busType = item->data(EditableValue).toInt();
    DataModel::ProjectModel::instance().updateSourceBusType(m_selectedSource.sourceId, busType);
    m_selectedSource.busType = busType;
    auto conn                = std::make_shared<QMetaObject::Connection>();
    *conn                    = connect(
      &IO::ConnectionManager::instance(),
      &IO::ConnectionManager::contextsRebuilt,
      this,
      [this, conn] {
        disconnect(*conn);
        buildSourceModel(m_selectedSource);
      },
      Qt::QueuedConnection);
    return;
  }

  if (id == kSourceView_Property) {
    const QString key  = item->data(ParameterKey).toString();
    const QVariant val = item->data(EditableValue);
    IO::HAL_Driver* drv =
      IO::ConnectionManager::instance().driverForEditing(m_selectedSource.sourceId);
    if (drv)
      drv->setDriverProperty(key, val);

    DataModel::ProjectModel::instance().captureSourceSettings(m_selectedSource.sourceId);

    // Transport mode change affects which properties are visible, rebuild the form
    static const QStringList kModeKeys = {
      QStringLiteral("socketTypeIndex"),
      QStringLiteral("protocolIndex"),
    };
    if (kModeKeys.contains(key))
      buildSourceModel(m_selectedSource);

    return;
  }

  const auto& sources = DataModel::ProjectModel::instance().sources();
  const int sid       = m_selectedSource.sourceId;

  if (sid < 0 || sid >= static_cast<int>(sources.size()))
    return;

  DataModel::Source updated = sources[sid];

  switch (static_cast<SourceItem>(id)) {
    case kSourceView_FrameDetection: {
      const int idx = item->data(EditableValue).toInt();
      if (idx < 0 || idx >= m_frameDetectionMethodsValues.size())
        return;

      updated.frameDetection = static_cast<int>(m_frameDetectionMethodsValues.at(idx));
      DataModel::ProjectModel::instance().updateSource(sid, updated);
      m_selectedSource = updated;
      buildSourceModel(m_selectedSource);
      break;
    }
    case kSourceView_HexadecimalSequence:
      updated.hexadecimalDelimiters = item->data(EditableValue).toBool();
      DataModel::ProjectModel::instance().updateSource(sid, updated);
      m_selectedSource = updated;
      buildSourceModel(m_selectedSource);
      break;
    case kSourceView_FrameStartSequence:
      updated.frameStart = item->data(EditableValue).toString();
      DataModel::ProjectModel::instance().updateSource(sid, updated);
      m_selectedSource = updated;
      break;
    case kSourceView_FrameEndSequence:
      updated.frameEnd = item->data(EditableValue).toString();
      DataModel::ProjectModel::instance().updateSource(sid, updated);
      m_selectedSource = updated;
      break;
    case kSourceView_FrameDecoder:
      updated.decoderMethod = item->data(EditableValue).toInt();
      DataModel::ProjectModel::instance().updateSource(sid, updated);
      m_selectedSource = updated;
      break;
    case kSourceView_ChecksumFunction: {
      const auto checksums = IO::availableChecksums();
      const int checksumId = item->data(EditableValue).toInt();
      if (checksumId < 0 || checksumId >= checksums.size())
        return;

      updated.checksumAlgorithm = checksums.at(checksumId);
      DataModel::ProjectModel::instance().updateSource(sid, updated);
      m_selectedSource = updated;
      break;
    }
    default:
      break;
  }
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
  // Dispose of the previous model and create a fresh one
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

  // Target device (only shown when multiple sources exist)
  const auto& sources = DataModel::ProjectModel::instance().sources();
  if (sources.size() > 1) {
    QStringList sourceLabels;
    for (const auto& src : sources)
      sourceLabels.append(src.title.isEmpty() ? tr("Device %1").arg(QChar('A' + src.sourceId))
                                              : src.title);

    int sourceIndex = 0;
    for (int i = 0; i < static_cast<int>(sources.size()); ++i) {
      if (sources[i].sourceId == action.sourceId) {
        sourceIndex = i;
        break;
      }
    }

    auto* sourceItem = new QStandardItem();
    sourceItem->setEditable(true);
    sourceItem->setData(true, Active);
    sourceItem->setData(ComboBox, WidgetType);
    sourceItem->setData(sourceLabels, ComboBoxData);
    sourceItem->setData(sourceIndex, EditableValue);
    sourceItem->setData(kActionView_SourceId, ParameterType);
    sourceItem->setData(tr("Target Device"), ParameterName);
    sourceItem->setData(tr("Select which connected device this action sends data to"),
                        ParameterDescription);
    m_actionModel->appendRow(sourceItem);
  }

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

    auto* encodingItem = new QStandardItem();
    encodingItem->setEditable(true);
    encodingItem->setData(true, Active);
    encodingItem->setData(ComboBox, WidgetType);
    encodingItem->setData(SerialStudio::textEncodings(), ComboBoxData);
    encodingItem->setData(action.txEncoding, EditableValue);
    encodingItem->setData(kActionView_TxEncoding, ParameterType);
    encodingItem->setData(tr("Text Encoding"), ParameterName);
    encodingItem->setData(tr("Character encoding used to serialize the text payload"),
                          ParameterDescription);
    m_actionModel->appendRow(encodingItem);
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

  auto* repeatCount = new QStandardItem();
  repeatCount->setData(IntField, WidgetType);
  repeatCount->setEditable(action.timerMode == DataModel::TimerMode::RepeatNTimes);
  repeatCount->setData(tr("Repeat Count"), ParameterName);
  repeatCount->setData(repeatCount->isEditable(), Active);
  repeatCount->setData(action.repeatCount, EditableValue);
  repeatCount->setData(kActionView_RepeatCount, ParameterType);
  repeatCount->setData(tr("Repeat Count"), PlaceholderValue);
  repeatCount->setData(tr("Number of times to send the command on each trigger"),
                       ParameterDescription);
  m_actionModel->appendRow(repeatCount);

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
  // Dispose of the previous model and create a fresh one
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
  // Section header
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

  auto* virtualItem = new QStandardItem();
  virtualItem->setEditable(true);
  virtualItem->setData(true, Active);
  virtualItem->setData(CheckBox, WidgetType);
  virtualItem->setData(dataset.virtual_, EditableValue);
  virtualItem->setData(kDatasetView_Virtual, ParameterType);
  virtualItem->setData(tr("Virtual Dataset"), ParameterName);
  virtualItem->setData(tr("Virtual datasets compute their value from transforms and data tables, "
                          "they do not require a frame index"),
                       ParameterDescription);
  model->appendRow(virtualItem);

  auto* indexItem = new QStandardItem();
  indexItem->setEditable(!dataset.virtual_);
  indexItem->setData(!dataset.virtual_, Active);
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
  // Section header
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
  // Section header
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
  // Determine editability and build widget type selector
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
  // Determine editability based on widget type
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
  // Section header
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
  // FFT window sizes
  m_fftSamples.clear();
  m_fftSamples << "8" << "16" << "32" << "64" << "128" << "256" << "512"
               << "1024" << "2048" << "4096" << "8192" << "16384";

  // Timer modes
  m_timerModes.clear();
  m_timerModes << tr("Off") << tr("Auto Start") << tr("Start on Trigger") << tr("Toggle on Trigger")
               << tr("Repeat N Times");

  // Decoder options
  m_decoderOptions.clear();
  m_decoderOptions << tr("Plain Text (UTF8)") << tr("Hexadecimal") << tr("Base64")
                   << tr("Binary (Direct)");

  // Checksum methods
  m_checksumMethods.clear();
  m_checksumMethods         = IO::availableChecksums();
  const int noChecksumIndex = m_checksumMethods.indexOf(QLatin1String(""));
  if (noChecksumIndex >= 0)
    m_checksumMethods[noChecksumIndex] = tr("No Checksum");

  // Frame detection methods
  m_frameDetectionMethods.clear();
  m_frameDetectionMethodsValues.clear();
  m_frameDetectionMethods << tr("End Delimiter Only") << tr("Start Delimiter Only")
                          << tr("Start + End Delimiter") << tr("No Delimiters");
  m_frameDetectionMethodsValues << SerialStudio::EndDelimiterOnly
                                << SerialStudio::StartDelimiterOnly
                                << SerialStudio::StartAndEndDelimiter << SerialStudio::NoDelimiters;

#ifdef BUILD_COMMERCIAL
  m_imgDetectionModes.clear();
  m_imgDetectionModes << tr("Auto-detect") << tr("Manual Delimiters");

  m_outputWidgetTypes.clear();
  m_outputWidgetTypes << tr("Button") << tr("Slider") << tr("Toggle") << tr("Text Field")
                      << tr("Knob") << tr("Ramp Generator");
#endif

  // Group composite widgets
  m_groupWidgets.clear();
  m_groupWidgets.insert(QStringLiteral("datagrid"), tr("Data Grid"));
  m_groupWidgets.insert(QStringLiteral("map"), tr("GPS Map"));
  m_groupWidgets.insert(QStringLiteral("gyro"), tr("Gyroscope"));
  m_groupWidgets.insert(QStringLiteral("multiplot"), tr("Multiple Plot"));
  m_groupWidgets.insert(QStringLiteral("accelerometer"), tr("Accelerometer"));
  m_groupWidgets.insert(QStringLiteral("plot3d"), tr("3D Plot"));
  m_groupWidgets.insert(QStringLiteral("image"), tr("Image View"));
  m_groupWidgets.insert(QLatin1String(""), tr("None"));

  // Dataset widgets
  m_datasetWidgets.clear();
  m_datasetWidgets.insert(QLatin1String(""), tr("None"));
  m_datasetWidgets.insert(QStringLiteral("bar"), tr("Bar"));
  m_datasetWidgets.insert(QStringLiteral("gauge"), tr("Gauge"));
  m_datasetWidgets.insert(QStringLiteral("compass"), tr("Compass"));

  // End-of-line sequences
  m_eolSequences.clear();
  m_eolSequences.insert(QLatin1String(""), tr("None"));
  m_eolSequences.insert(QStringLiteral("\n"), tr("New Line (\\n)"));
  m_eolSequences.insert(QStringLiteral("\r"), tr("Carriage Return (\\r)"));
  m_eolSequences.insert(QStringLiteral("\r\n"), tr("CRLF (\\r\\n)"));

  // Plot options
  m_plotOptions.clear();
  m_plotOptions.insert(qMakePair(false, false), tr("No"));
  m_plotOptions.insert(qMakePair(true, false), tr("Yes"));
}

//--------------------------------------------------------------------------------------------------
// Private slot: view management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Transitions the editor to @p view.
 *
 * Emits currentViewChanged() and selectedTextChanged() after the transition.
 *
 * @param view  The target editor view.
 */
void DataModel::ProjectEditor::setCurrentView(const DataModel::ProjectEditor::CurrentView view)
{
  if (m_currentView == view)
    return;

  m_currentView = view;
  Q_EMIT currentViewChanged();
  Q_EMIT selectedTextChanged();
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
  // Validate item and extract the changed parameter
  if (!item)
    return;

  const auto id      = static_cast<GroupItem>(item->data(ParameterType).toInt());
  const auto value   = item->data(EditableValue);
  auto& pm           = DataModel::ProjectModel::instance();
  const auto groupId = m_selectedGroup.groupId;

  if (id == kGroupView_Title) {
    const auto newTitle = value.toString();
    if (m_selectedGroup.title == newTitle)
      return;

    m_selectedGroup.title = newTitle;
    pm.updateGroup(groupId, m_selectedGroup, false);

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
  } else if (id == kGroupView_Source) {
    const auto& sources = DataModel::ProjectModel::instance().sources();
    const int srcIdx    = value.toInt();
    if (srcIdx >= 0 && srcIdx < static_cast<int>(sources.size())) {
      m_selectedGroup.sourceId = sources[srcIdx].sourceId;
      for (auto& ds : m_selectedGroup.datasets)
        ds.sourceId = m_selectedGroup.sourceId;

      pm.updateGroup(groupId, m_selectedGroup, true);
    }
  } else if (id == kGroupView_Widget) {
    const auto keys     = m_groupWidgets.keys();
    const int widgetIdx = value.toInt();
    if (widgetIdx < 0 || widgetIdx >= keys.size())
      return;

    const auto widgetStr = keys.at(widgetIdx);

    static const QMap<QString, SerialStudio::GroupWidget> kWidgetEnumMap = {
      {"accelerometer", SerialStudio::Accelerometer},
      {    "multiplot",     SerialStudio::MultiPlot},
      {         "gyro",     SerialStudio::Gyroscope},
      {          "map",           SerialStudio::GPS},
      {     "datagrid",      SerialStudio::DataGrid},
      {       "plot3d",        SerialStudio::Plot3D},
      {        "image",     SerialStudio::ImageView},
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
  // Validate item and lazy-initialize EOL key list
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
      const auto& sources = DataModel::ProjectModel::instance().sources();
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

  // Persist the change to the project model
  auto& pm            = DataModel::ProjectModel::instance();
  const auto actionId = m_selectedAction.actionId;
  pm.setSelectedAction(m_selectedAction);
  pm.updateAction(actionId, m_selectedAction, false);

  // Update tree item text in-place for title changes
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
  }
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
  // Validate item and dispatch the change
  if (!item)
    return;

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);
  auto& pm         = DataModel::ProjectModel::instance();

  switch (static_cast<ProjectItem>(id.toInt())) {
    case kProjectView_Title:
      pm.setTitle(value.toString());
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
  // Validate item and lazy-initialize key lists
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
    case kDatasetView_Widget: {
      const int widgetIdx = value.toInt();
      if (widgetIdx < 0 || widgetIdx >= datasetWidgetKeys.size())
        return;

      m_selectedDataset.widget = datasetWidgetKeys.at(widgetIdx);
      if (m_selectedDataset.widget == "compass") {
        m_selectedDataset.wgtMin    = 0;
        m_selectedDataset.wgtMax    = 360;
        m_selectedDataset.alarmLow  = 0;
        m_selectedDataset.alarmHigh = 0;
      }
      buildDatasetModel(m_selectedDataset);
      break;
    }
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
      // Deprecated — overview option removed in v3.3. Kept in enum to avoid
      // renumbering downstream cases; never reached because the row is no
      // longer added to the editor model.
      break;
    case kDatasetView_Plot: {
      const int plotIdx = value.toInt();
      if (plotIdx < 0 || plotIdx >= plotOptionKeys.size())
        return;

      m_selectedDataset.plt = plotOptionKeys.at(plotIdx).first;
      m_selectedDataset.log = plotOptionKeys.at(plotIdx).second;
      buildDatasetModel(m_selectedDataset);
      break;
    }
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
    case kDatasetView_FFT_Samples: {
      const int sampleIdx = value.toInt();
      if (sampleIdx < 0 || sampleIdx >= m_fftSamples.size())
        return;

      m_selectedDataset.fftSamples = m_fftSamples.at(sampleIdx).toInt();
      break;
    }
    case kDatasetView_FFT_SamplingRate:
      m_selectedDataset.fftSamplingRate = value.toInt();
      break;
    case kDatasetView_TransformCode:
      m_selectedDataset.transformCode = value.toString();
      break;
    case kDatasetView_Virtual: {
      m_selectedDataset.virtual_ = value.toBool();

      // Defer the rebuild so we don't mutate the model mid-item-changed signal
      // emission. Guard against the user switching selection before the lambda
      // fires — if a different dataset is now selected, skip the rebuild.
      const int uid = m_selectedDataset.uniqueId;
      QTimer::singleShot(0, this, [this, uid] {
        if (m_selectedDataset.uniqueId == uid)
          buildDatasetModel(m_selectedDataset);
      });

      break;
    }
    default:
      break;
  }

  auto& pm             = DataModel::ProjectModel::instance();
  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;
  const auto idInt     = static_cast<DatasetItem>(id.toInt());

  // Title updates tree item in-place; index changes require a full rebuild
  if (idInt == kDatasetView_Title) {
    const auto newTitle = m_selectedDataset.title;
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
  } else {
    const bool rebuildTree = (idInt == kDatasetView_Index);
    pm.updateDataset(groupId, datasetId, m_selectedDataset, rebuildTree);
  }

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

  if (m_sourceParserItems.contains(item)) {
    const auto source = m_sourceParserItems.value(item);
    m_selectedSource  = source;
    setCurrentView(SourceFrameParserView);
    Q_EMIT selectedSourceFrameParserCodeChanged();
    Q_EMIT sourceModelChanged();
  } else if (m_sourceItems.contains(item)) {
    const auto source = m_sourceItems.value(item);
    if (m_currentView == SourceView && source.sourceId == m_selectedSource.sourceId)
      return;

    setCurrentView(SourceView);
    buildSourceModel(source);
  } else if (m_groupItems.contains(item)) {
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
  } else if (m_outputWidgetItems.contains(item)) {
    const auto ow = m_outputWidgetItems.value(item);
    DataModel::ProjectModel::instance().setSelectedOutputWidget(ow);
    setCurrentView(OutputWidgetView);
    buildOutputWidgetModel(ow);
  } else if (item == m_tablesRootItem) {
    setCurrentView(DataTablesView);
  } else if (item == m_systemDatasetsItem) {
    setCurrentView(SystemDatasetsView);
  } else if (m_userTableItems.contains(item)) {
    const auto name = m_userTableItems.value(item);
    if (m_selectedUserTable != name) {
      m_selectedUserTable = name;
      Q_EMIT selectedUserTableChanged();
    }

    setCurrentView(UserTableView);
  } else if (item == m_workspacesRootItem) {
    setCurrentView(WorkspacesView);
  } else if (m_workspaceItems.contains(item)) {
    const int wid = m_workspaceItems.value(item);
    if (m_selectedWorkspaceId != wid) {
      m_selectedWorkspaceId = wid;
      Q_EMIT selectedWorkspaceIdChanged();
    }

    setCurrentView(WorkspaceView);
  } else if (m_rootItems.contains(item)) {
    setCurrentView(ProjectView);
    buildProjectModel();
  }
}

//--------------------------------------------------------------------------------------------------
// Private helpers: expanded state persistence
/**
 * @brief Selects the source item with the given @p sourceId in the tree.
 * @param sourceId The source identifier to navigate to.
 */
void DataModel::ProjectEditor::selectSource(int sourceId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_sourceItems.begin(); it != m_sourceItems.end(); ++it) {
    if (it.value().sourceId == sourceId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the group item with the given @p groupId in the tree.
 * @param groupId The group identifier to navigate to.
 */
void DataModel::ProjectEditor::selectGroup(int groupId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
    if (it.value().groupId == groupId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the dataset item identified by @p groupId and @p datasetId in the tree.
 * @param groupId   The owning group identifier.
 * @param datasetId The dataset identifier within that group.
 */
void DataModel::ProjectEditor::selectDataset(int groupId, int datasetId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
    if (it.value().groupId == groupId && it.value().datasetId == datasetId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the action item with the given @p actionId in the tree.
 * @param actionId The action identifier to navigate to.
 */
void DataModel::ProjectEditor::selectAction(int actionId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_actionItems.begin(); it != m_actionItems.end(); ++it) {
    if (it.value().actionId == actionId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the Frame Parser Code item in the tree.
 */
void DataModel::ProjectEditor::selectFrameParser(int sourceId)
{
  displayFrameParserView(sourceId);
}

/**
 * @brief Programmatically selects an output widget in the tree.
 */
void DataModel::ProjectEditor::selectOutputWidget(int groupId, int widgetId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
    if (it.value().groupId == groupId && it.value().widgetId == widgetId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Output widget form model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the form model for editing an output widget's properties.
 *
 * Creates form rows for title, type, value range, labels, and the JavaScript
 * transmit function. Connects itemChanged to onOutputWidgetItemChanged.
 */
void DataModel::ProjectEditor::buildOutputWidgetModel(const DataModel::OutputWidget& widget)
{
  // Store selection and dispose of the previous model
  m_selectedOutputWidget = widget;

  if (m_outputWidgetModel) {
    disconnect(m_outputWidgetModel);
    m_outputWidgetModel->deleteLater();
  }

  m_outputWidgetModel = new CustomModel(this);

  // General information section
  auto* hdr = new QStandardItem();
  hdr->setData(true, Active);
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("General Information"), PlaceholderValue);
  hdr->setData("qrc:/rcc/icons/project-editor/model/output-widget.svg", ParameterIcon);
  m_outputWidgetModel->appendRow(hdr);

  // Label
  auto* titleItem = new QStandardItem();
  titleItem->setEditable(true);
  titleItem->setData(true, Active);
  titleItem->setData(TextField, WidgetType);
  titleItem->setData(widget.title, EditableValue);
  titleItem->setData(kOutputWidget_Title, ParameterType);
  titleItem->setData(tr("Label"), ParameterName);
  titleItem->setData(tr("Display label"), PlaceholderValue);
  m_outputWidgetModel->appendRow(titleItem);

  // Icon (for Button type)
  if (widget.type == DataModel::OutputWidgetType::Button) {
    auto* iconItem = new QStandardItem();
    iconItem->setEditable(true);
    iconItem->setData(true, Active);
    iconItem->setData(IconPicker, WidgetType);
    iconItem->setData(widget.icon, EditableValue);
    iconItem->setData(kOutputWidget_Icon, ParameterType);
    iconItem->setData(tr("Button Icon"), ParameterName);
    m_outputWidgetModel->appendRow(iconItem);

    // Mono icon checkbox
    auto* monoItem = new QStandardItem();
    monoItem->setEditable(true);
    monoItem->setData(true, Active);
    monoItem->setData(CheckBox, WidgetType);
    monoItem->setData(widget.monoIcon, EditableValue);
    monoItem->setData(kOutputWidget_MonoIcon, ParameterType);
    monoItem->setData(tr("Colorize Icon"), ParameterName);
    monoItem->setData(tr("Tint the icon with the button color"), ParameterDescription);
    m_outputWidgetModel->appendRow(monoItem);
  }

  // Initial value (not applicable for buttons)
  if (widget.type != DataModel::OutputWidgetType::Button) {
    auto* initItem = new QStandardItem();
    initItem->setEditable(true);
    initItem->setData(true, Active);
    initItem->setData(FloatField, WidgetType);
    initItem->setData(widget.initialValue, EditableValue);
    initItem->setData(kOutputWidget_InitialValue, ParameterType);
    initItem->setData(tr("Initial Value"), ParameterName);
    m_outputWidgetModel->appendRow(initItem);
  }

  // Text encoding picker for string results from transmit(value)
  auto* encodingItem = new QStandardItem();
  encodingItem->setEditable(true);
  encodingItem->setData(true, Active);
  encodingItem->setData(ComboBox, WidgetType);
  encodingItem->setData(SerialStudio::textEncodings(), ComboBoxData);
  encodingItem->setData(widget.txEncoding, EditableValue);
  encodingItem->setData(kOutputWidget_TxEncoding, ParameterType);
  encodingItem->setData(tr("Text Encoding"), ParameterName);
  encodingItem->setData(tr("Character encoding used when transmit() returns a string value"),
                        ParameterDescription);
  m_outputWidgetModel->appendRow(encodingItem);

  // Value range section (for sliders/knobs)
  const bool isNumeric = widget.type == DataModel::OutputWidgetType::Slider
                      || widget.type == DataModel::OutputWidgetType::Knob;

  if (isNumeric) {
    auto* rangeHdr = new QStandardItem();
    rangeHdr->setData(true, Active);
    rangeHdr->setData(SectionHeader, WidgetType);
    rangeHdr->setData(tr("Value Range"), PlaceholderValue);
    rangeHdr->setData("qrc:/rcc/icons/project-editor/model/output-range.svg", ParameterIcon);
    m_outputWidgetModel->appendRow(rangeHdr);

    // Min value
    auto* minItem = new QStandardItem();
    minItem->setEditable(true);
    minItem->setData(true, Active);
    minItem->setData(FloatField, WidgetType);
    minItem->setData(widget.minValue, EditableValue);
    minItem->setData(kOutputWidget_MinValue, ParameterType);
    minItem->setData(tr("Minimum Value"), ParameterName);
    m_outputWidgetModel->appendRow(minItem);

    // Max value
    auto* maxItem = new QStandardItem();
    maxItem->setEditable(true);
    maxItem->setData(true, Active);
    maxItem->setData(FloatField, WidgetType);
    maxItem->setData(widget.maxValue, EditableValue);
    maxItem->setData(kOutputWidget_MaxValue, ParameterType);
    maxItem->setData(tr("Maximum Value"), ParameterName);
    m_outputWidgetModel->appendRow(maxItem);

    // Step size
    auto* stepItem = new QStandardItem();
    stepItem->setEditable(true);
    stepItem->setData(true, Active);
    stepItem->setData(FloatField, WidgetType);
    stepItem->setData(widget.stepSize, EditableValue);
    stepItem->setData(kOutputWidget_StepSize, ParameterType);
    stepItem->setData(tr("Step Size"), ParameterName);
    m_outputWidgetModel->appendRow(stepItem);
  }

  connect(m_outputWidgetModel,
          &CustomModel::itemChanged,
          this,
          &DataModel::ProjectEditor::onOutputWidgetItemChanged);

  Q_EMIT outputWidgetModelChanged();
}

/**
 * @brief Handles changes to output widget form fields.
 */
void DataModel::ProjectEditor::onOutputWidgetItemChanged(QStandardItem* item)
{
  // Validate item and dispatch the change by parameter type
  if (!item)
    return;

  const auto id    = item->data(ParameterType);
  const auto value = item->data(EditableValue);

  switch (static_cast<OutputWidgetItem>(id.toInt())) {
    case kOutputWidget_Title:
      m_selectedOutputWidget.title = value.toString();
      break;
    case kOutputWidget_Icon:
      m_selectedOutputWidget.icon = value.toString();
      break;
    case kOutputWidget_MonoIcon:
      m_selectedOutputWidget.monoIcon = value.toBool();
      break;
    case kOutputWidget_Type: {
      const auto newType = static_cast<DataModel::OutputWidgetType>(value.toInt());
      if (m_selectedOutputWidget.type != newType) {
        m_selectedOutputWidget.type = newType;
        buildOutputWidgetModel(m_selectedOutputWidget);
      }
      break;
    }
    case kOutputWidget_MinValue:
      m_selectedOutputWidget.minValue = value.toDouble();
      break;
    case kOutputWidget_MaxValue:
      m_selectedOutputWidget.maxValue = value.toDouble();
      break;
    case kOutputWidget_StepSize:
      m_selectedOutputWidget.stepSize = value.toDouble();
      break;
    case kOutputWidget_InitialValue:
      m_selectedOutputWidget.initialValue = value.toDouble();
      break;
    case kOutputWidget_TransmitFunction:
      m_selectedOutputWidget.transmitFunction = value.toString();
      break;
    case kOutputWidget_TxEncoding:
      m_selectedOutputWidget.txEncoding = value.toInt();
      break;
  }

  // Update the tree item title
  if (static_cast<OutputWidgetItem>(id.toInt()) == kOutputWidget_Title) {
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
  }

  // Persist to ProjectModel
  DataModel::ProjectModel::instance().updateOutputWidget(
    m_selectedOutputWidget.groupId, m_selectedOutputWidget.widgetId, m_selectedOutputWidget, false);
}

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

//--------------------------------------------------------------------------------------------------
// Data tables — read-only summaries for QML views
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a read-only summary of all user-defined data tables and the
 *        auto-generated __datasets__ system table for display in QML.
 *
 * Each list entry is a QVariantMap with keys:
 *   - "name":         Table name (QString)
 *   - "isSystem":     True for the __datasets__ system table (bool)
 *   - "registerCount": Number of registers in the table (int)
 */
QVariantList DataModel::ProjectEditor::tablesSummary() const
{
  // Always list the auto-generated "Dataset Values" entry first
  QVariantList result;
  const auto& groups = DataModel::ProjectModel::instance().groups();

  int datasetCount = 0;
  for (const auto& g : groups)
    datasetCount += static_cast<int>(g.datasets.size());

  QVariantMap sysRow;
  sysRow["name"]        = tr("Dataset Values");
  sysRow["description"] = tr("Raw and transformed values for every dataset (read-only)");
  sysRow["isSystem"]    = true;
  sysRow["entryCount"]  = datasetCount;
  result.append(sysRow);

  // Append user-defined shared tables
  const auto& tables = DataModel::ProjectModel::instance().tables();
  for (const auto& table : tables) {
    QVariantMap row;
    row["name"]        = table.name;
    row["description"] = tr("Shared table defined in this project");
    row["isSystem"]    = false;
    row["entryCount"]  = static_cast<int>(table.registers.size());
    result.append(row);
  }

  return result;
}

/**
 * @brief Returns the name of the currently selected user table (empty if none).
 */
QString DataModel::ProjectEditor::selectedUserTable() const
{
  return m_selectedUserTable;
}

/**
 * @brief Selects the given user table and switches to the UserTableView.
 */
void DataModel::ProjectEditor::selectUserTable(const QString& tableName)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_userTableItems.begin(); it != m_userTableItems.end(); ++it) {
    if (it.value() == tableName) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Returns the id of the workspace currently displayed in the editor.
 */
int DataModel::ProjectEditor::selectedWorkspaceId() const noexcept
{
  return m_selectedWorkspaceId;
}

/**
 * @brief Selects the workspace with the given id and switches to its editor.
 */
void DataModel::ProjectEditor::selectWorkspace(int workspaceId)
{
  if (!m_selectionModel)
    return;

  for (auto it = m_workspaceItems.begin(); it != m_workspaceItems.end(); ++it) {
    if (it.value() == workspaceId) {
      m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Returns the current tree search query (empty = no filter).
 */
const QString& DataModel::ProjectEditor::treeSearchQuery() const noexcept
{
  return m_treeSearchQuery;
}

/**
 * @brief Updates the tree search query and rebuilds the tree to apply it.
 *
 * Matching is case-insensitive substring — rows whose title (or any ancestor's
 * title) contains the query survive. Empty string restores the full tree.
 */
void DataModel::ProjectEditor::setTreeSearchQuery(const QString& query)
{
  if (m_treeSearchQuery == query)
    return;

  m_treeSearchQuery = query;
  Q_EMIT treeSearchQueryChanged();

  // Defer the rebuild so rapid typing doesn't rebuild on every keystroke
  // mid-QML binding evaluation. Check the query is still current — if the
  // user kept typing, a later setter invocation queued its own rebuild.
  const auto current = m_treeSearchQuery;
  QTimer::singleShot(0, this, [this, current] {
    if (m_treeSearchQuery == current)
      buildTreeModel();
  });
}

/**
 * @brief Returns a read-only summary of the system __datasets__ table.
 *
 * Each list entry corresponds to one dataset and is a QVariantMap with keys:
 *   - "uniqueId":   Dataset unique identifier (int)
 *   - "groupTitle": Owning group title (QString)
 *   - "title":      Dataset title (QString)
 *   - "units":      Dataset units (QString)
 *   - "rawReg":     Register name for the raw value (QString)
 *   - "finalReg":   Register name for the final value (QString)
 *   - "isVirtual":  True if this is a virtual dataset (bool)
 */
QVariantList DataModel::ProjectEditor::systemDatasetsSummary() const
{
  // Recompute uniqueId using the finalize_frame encoding — ProjectModel's
  // copy of groups is not finalized: sourceId*1000000 + groupId*10000 + datasetId
  QVariantList result;
  const auto& groups = DataModel::ProjectModel::instance().groups();

  for (const auto& group : groups) {
    for (const auto& ds : group.datasets) {
      const int uid = group.sourceId * 1000000 + ds.groupId * 10000 + ds.datasetId;

      QVariantMap row;
      row["uniqueId"]   = uid;
      row["groupTitle"] = group.title;
      row["title"]      = ds.title;
      row["units"]      = ds.units;
      row["rawReg"]     = QStringLiteral("raw:") + QString::number(uid);
      row["finalReg"]   = QStringLiteral("final:") + QString::number(uid);
      row["isVirtual"]  = ds.virtual_;
      result.append(row);
    }
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Workspaces — read-only summaries for QML views
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a summary of every workspace defined in the project.
 *
 * Each entry: { id (int), title (QString), icon (QString), widgetCount (int) }.
 */
QVariantList DataModel::ProjectEditor::workspacesSummary() const
{
  QVariantList result;
  const auto& workspaces = DataModel::ProjectModel::instance().workspaces();

  for (const auto& ws : workspaces) {
    QVariantMap row;
    row["id"]          = ws.workspaceId;
    row["title"]       = ws.title;
    row["icon"]        = ws.icon;
    row["widgetCount"] = static_cast<int>(ws.widgetRefs.size());
    result.append(row);
  }

  return result;
}

/**
 * @brief Returns the list of widget references attached to a workspace.
 *
 * Each entry: { widgetType (int), groupId (int), relativeIndex (int),
 *               groupTitle (QString), datasetTitle (QString) }.
 *
 * groupTitle/datasetTitle are resolved from the project so the editor can
 * display human-readable labels rather than raw numeric refs.
 */
QVariantList DataModel::ProjectEditor::widgetsForWorkspace(int workspaceId) const
{
  QVariantList result;
  const auto& pm     = DataModel::ProjectModel::instance();
  const auto& wsList = pm.workspaces();

  // Locate the workspace
  auto wsIt = std::find_if(wsList.begin(), wsList.end(), [workspaceId](const auto& w) {
    return w.workspaceId == workspaceId;
  });

  if (wsIt == wsList.end())
    return result;

  // Mirror buildAutoWorkspaces()'s walk — relativeIndex is a per-widget-type
  // project-wide counter, NOT a dataset array index within the group.
  struct ResolvedWidget {
    QString groupTitle;
    QString datasetTitle;
  };

  QHash<qint64, ResolvedWidget> lookup;
  const auto makeKey = [](int widgetType, int groupId, int relIdx) {
    return (static_cast<qint64>(widgetType) << 40) | (static_cast<qint64>(groupId) << 20)
         | static_cast<qint64>(relIdx);
  };

  const auto& groups = pm.groups();
  const bool pro     = SerialStudio::proWidgetsEnabled();
  QHash<int, int> groupRunning;
  QHash<int, int> datasetRunning;

  for (const auto& g : groups) {
    if (!SerialStudio::groupEligibleForWorkspace(g))
      continue;

    auto groupKey = SerialStudio::getDashboardWidget(g);
    if (groupKey == SerialStudio::DashboardPlot3D && !pro)
      groupKey = SerialStudio::DashboardMultiPlot;

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey)) {
      const int typeKey = static_cast<int>(groupKey);
      const int relIdx  = groupRunning.value(typeKey, 0);
      groupRunning.insert(typeKey, relIdx + 1);

      ResolvedWidget entry;
      entry.groupTitle   = g.title;
      entry.datasetTitle = QString();
      lookup.insert(makeKey(typeKey, g.groupId, relIdx), entry);
    }

    for (const auto& ds : g.datasets) {
      const auto keys = SerialStudio::getDashboardWidgets(ds);
      for (const auto& k : keys) {
        if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
          continue;

        const int typeKey = static_cast<int>(k);
        const int relIdx  = datasetRunning.value(typeKey, 0);
        datasetRunning.insert(typeKey, relIdx + 1);

        ResolvedWidget entry;
        entry.groupTitle   = g.title;
        entry.datasetTitle = ds.title;
        lookup.insert(makeKey(typeKey, g.groupId, relIdx), entry);
      }
    }
  }

  for (const auto& ref : wsIt->widgetRefs) {
    QVariantMap row;
    row["widgetType"]     = ref.widgetType;
    row["widgetTypeName"] = SerialStudio::dashboardWidgetTitle(
      static_cast<SerialStudio::DashboardWidget>(ref.widgetType));
    row["groupId"]       = ref.groupId;
    row["relativeIndex"] = ref.relativeIndex;
    row["groupTitle"]    = QString();
    row["datasetTitle"]  = QString();

    const auto it = lookup.constFind(makeKey(ref.widgetType, ref.groupId, ref.relativeIndex));
    if (it != lookup.constEnd()) {
      row["groupTitle"]   = it->groupTitle;
      row["datasetTitle"] = it->datasetTitle;
    }

    result.append(row);
  }

  return result;
}

/**
 * @brief Returns every widget the project can show, with its (widgetType,
 *        groupId, relativeIndex) triple.
 *
 * Mirrors the walk performed by ProjectModel::autoGenerateWorkspaces() so the
 * per-type relativeIndex matches what Dashboard produces at runtime. Each
 * entry: { widgetType, groupId, relativeIndex, groupTitle, datasetTitle,
 *          isGroupWidget, widgetLabel (human-readable) }.
 *
 * Used by the "Add Widget" picker in WorkspaceView.
 */
QVariantList DataModel::ProjectEditor::allWidgetsSummary() const
{
  QVariantList result;
  QMap<SerialStudio::DashboardWidget, int> groupIdx;
  QMap<SerialStudio::DashboardWidget, int> datasetIdx;

  const auto& groups = DataModel::ProjectModel::instance().groups();
  const bool pro     = SerialStudio::proWidgetsEnabled();
  for (const auto& group : groups) {
    // Skip groups that don't contribute to Dashboard's widget walker
    if (!SerialStudio::groupEligibleForWorkspace(group))
      continue;

    // Group-level widget — mirror Dashboard's non-Pro Plot3D fallback so the
    // picker advertises the widget that will actually render.
    auto groupKey = SerialStudio::getDashboardWidget(group);
    if (groupKey == SerialStudio::DashboardPlot3D && !pro)
      groupKey = SerialStudio::DashboardMultiPlot;

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey)) {
      QVariantMap row;
      row["widgetType"]    = static_cast<int>(groupKey);
      row["groupId"]       = group.groupId;
      row["relativeIndex"] = groupIdx.value(groupKey, 0);
      row["groupTitle"]    = group.title;
      row["datasetTitle"]  = QString();
      row["isGroupWidget"] = true;
      row["widgetLabel"]   = SerialStudio::dashboardWidgetTitle(groupKey);
      groupIdx[groupKey]   = row["relativeIndex"].toInt() + 1;
      result.append(row);
    }

    // Dataset widgets
    for (const auto& ds : group.datasets) {
      const auto keys = SerialStudio::getDashboardWidgets(ds);
      for (const auto& k : keys) {
        if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
          continue;

        QVariantMap row;
        row["widgetType"]    = static_cast<int>(k);
        row["groupId"]       = group.groupId;
        row["relativeIndex"] = datasetIdx.value(k, 0);
        row["groupTitle"]    = group.title;
        row["datasetTitle"]  = ds.title;
        row["isGroupWidget"] = false;
        row["widgetLabel"]   = SerialStudio::dashboardWidgetTitle(k);
        datasetIdx[k]        = row["relativeIndex"].toInt() + 1;
        result.append(row);
      }
    }
  }

  return result;
}
