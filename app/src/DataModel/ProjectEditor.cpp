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

#include "DataModel/ProjectEditor.h"

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
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
#include "Project/ProjectEditorItemIds.h"
#include "Project/ProjectEditorShared.h"

/**
 * @brief Constructs the ProjectEditor singleton and wires its ProjectModel signals.
 */
DataModel::ProjectEditor::ProjectEditor()
  : m_projectModelRef(DataModel::ProjectModel::instance())
  , m_connectionManager(IO::ConnectionManager::instance())
  , m_translator(Misc::Translator::instance())
  , m_frameBuilder(DataModel::FrameBuilder::instance())
#ifdef BUILD_COMMERCIAL
  , m_mqttPublisher(MQTT::Publisher::instance())
#endif
  , m_currentView(ProjectView)
  , m_suppressViewChange(false)
  , m_navCursor(-1)
  , m_navDirection(0)
  , m_navigatingHistory(false)
  , m_batchKind(KindNone)
  , m_batchApplying(false)
  , m_groupsRootItem(nullptr)
  , m_selectedGroupFolderId(-1)
  , m_tablesRootItem(nullptr)
  , m_systemDatasetsItem(nullptr)
  , m_selectedTableFolderId(-1)
  , m_workspacesRootItem(nullptr)
  , m_selectedWorkspaceId(-1)
  , m_selectedFolderId(-1)
  , m_mqttPublisherItem(nullptr)
  , m_controlScriptItem(nullptr)
  , m_seedExpansionFromModel(true)
  , m_treeModel(nullptr)
  , m_selectionModel(nullptr)
  , m_groupModel(nullptr)
  , m_sourceModel(nullptr)
  , m_actionModel(nullptr)
  , m_projectModel(nullptr)
  , m_datasetModel(nullptr)
  , m_outputWidgetModel(nullptr)
  , m_mqttPublisherModel(nullptr)
  , m_transformEditor(nullptr)
#ifdef BUILD_COMMERCIAL
  , m_mqttScriptEditor(nullptr)
#endif
  , m_pendingSelectionKind(PendingSelectionKind::None)
  , m_pendingSelectionGroupId(-1)
  , m_pendingSelectionItemId(-1)
{
  generateComboBoxModels();

  m_rebuildTimer.setSingleShot(true);
  m_rebuildTimer.setInterval(0);
  connect(&m_rebuildTimer, &QTimer::timeout, this, &DataModel::ProjectEditor::buildTreeModel);

  wireProjectModelRebuilds();
  wireGroupSignals();
  wireDatasetSignals();
  wireActionSignals();
  wireOutputWidgetSignals();
  wireSourceSignals();
  wireEditorSelfSignals();
  wireExternalSignals();

  buildTreeModel();
  buildProjectModel();
}

/**
 * @brief Releases the lazily-opened editor dialogs. They are constructed parentless because
 *        this class is a QObject and they are QWidgets, so no Qt parent chain ever reaches
 *        them and the destructor is their only release path.
 */
DataModel::ProjectEditor::~ProjectEditor()
{
  delete m_transformEditor;
#ifdef BUILD_COMMERCIAL
  delete m_mqttScriptEditor;
#endif
}

/**
 * @brief Returns the singleton ProjectEditor instance.
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
 * @brief Returns the editor's currently active view.
 */
DataModel::ProjectEditor::CurrentView DataModel::ProjectEditor::currentView() const
{
  return m_currentView;
}

/**
 * @brief Returns the ItemKind shared by a homogeneous multi-selection (KindNone otherwise).
 */
int DataModel::ProjectEditor::multiSelectionKind() const noexcept
{
  return static_cast<int>(m_batchKind);
}

/**
 * @brief Returns the number of items in the current homogeneous multi-selection.
 */
int DataModel::ProjectEditor::multiSelectionCount() const noexcept
{
  return static_cast<int>(m_batchItems.size());
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
 * @brief Returns the icon path of the currently selected tree item.
 */
QString DataModel::ProjectEditor::selectedIcon() const
{
  if (!m_selectionModel || !m_treeModel)
    return "";

  const auto index = m_selectionModel->currentIndex();
  return m_treeModel->data(index, TreeViewIcon).toString();
}

/**
 * @brief Returns the icon path of the currently selected action.
 */
const QString DataModel::ProjectEditor::actionIcon() const
{
  return m_selectedAction.icon;
}

/**
 * @brief Returns the icon path of the currently selected output widget.
 */
const QString DataModel::ProjectEditor::outputWidgetIcon() const
{
  return m_selectedOutputWidget.icon;
}

/**
 * @brief Returns the cached list of action icon base-names from resources.
 */
const QStringList& DataModel::ProjectEditor::availableActionIcons() const
{
  static QStringList icons;

  if (icons.isEmpty()) {
    const auto path = QStringLiteral(":/actions/");
    QDirIterator it(path, QStringList() << "*.svg", QDir::Files);
    while (it.hasNext()) {
      const auto filePath = it.next();
      icons.append(QFileInfo(filePath).baseName());
    }
  }

  return icons;
}

/**
 * @brief Returns true when the selected group's dataset list is freely editable.
 */
bool DataModel::ProjectEditor::currentGroupIsEditable() const
{
  if (m_currentView == GroupView) {
    const auto& widget = m_selectedGroup.widget;
    if (widget != "" && widget != "multiplot" && widget != "datagrid" && widget != "painter"
        && widget != "barpanel")
      return false;
  }

  return true;
}

/**
 * @brief Returns true when the selected dataset's parent group is editable.
 */
bool DataModel::ProjectEditor::currentDatasetIsEditable() const
{
  if (m_currentView == DatasetView || m_currentView == MultiSelectionView)
    return datasetWidgetEditable(m_selectedDataset);

  return true;
}

/**
 * @brief Returns true when @p dataset's own parent group leaves its widget freely selectable;
 *        view-independent so the multi-select aggregate form gates each member like the
 *        single-dataset editor would.
 */
bool DataModel::ProjectEditor::datasetWidgetEditable(const DataModel::Dataset& dataset) const
{
  const auto& groups = m_projectModelRef.groups();
  const auto groupId = dataset.groupId;
  if (groupId >= 0 && static_cast<size_t>(groupId) < groups.size()) {
    const auto& widget = groups[groupId].widget;
    if (widget != "" && widget != "multiplot" && widget != "datagrid" && widget != "painter"
        && widget != "barpanel")
      return false;
  }

  return true;
}

/**
 * @brief Returns whether the selected group is enabled (false dims and locks its editor form).
 * Reads the live ProjectModel so a context-menu toggle of the open group reflects immediately.
 */
bool DataModel::ProjectEditor::selectedGroupEnabled() const
{
  const auto& groups = m_projectModelRef.groups();
  const auto groupId = m_selectedGroup.groupId;
  if (groupId >= 0 && static_cast<size_t>(groupId) < groups.size())
    return groups[groupId].enabled;

  return true;
}

/**
 * @brief Returns the selected dataset's effective enablement: false when the dataset or its parent
 *        group is disabled, so a dataset under a disabled group reads as inactive. Reads live
 * state.
 */
bool DataModel::ProjectEditor::selectedDatasetEnabled() const
{
  const auto& groups   = m_projectModelRef.groups();
  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return true;

  const auto& group = groups[groupId];
  if (!group.enabled)
    return false;

  if (datasetId >= 0 && static_cast<size_t>(datasetId) < group.datasets.size())
    return group.datasets[datasetId].enabled;

  return true;
}

/**
 * @brief Returns the DatasetOption bitmask for the selected dataset.
 */
quint16 DataModel::ProjectEditor::datasetOptions() const
{
  quint16 option = SerialStudio::DatasetGeneric;

  if (m_selectedDataset.plt)
    option |= SerialStudio::DatasetPlot;

  if (m_selectedDataset.fft)
    option |= SerialStudio::DatasetFFT;

  if (m_selectedDataset.led)
    option |= SerialStudio::DatasetLED;

  if (m_selectedDataset.waterfall)
    option |= SerialStudio::DatasetWaterfall;

  static const QHash<QString, quint16> kWidgetFlags = {
    {    QStringLiteral("bar"),     SerialStudio::DatasetBar},
    {  QStringLiteral("gauge"),   SerialStudio::DatasetGauge},
    {QStringLiteral("compass"), SerialStudio::DatasetCompass},
    {  QStringLiteral("meter"),   SerialStudio::DatasetMeter},
  };
  option |= kWidgetFlags.value(m_selectedDataset.widget, 0);

  return option;
}

//--------------------------------------------------------------------------------------------------
// Model accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the tree model exposed to QML.
 */
DataModel::CustomModel* DataModel::ProjectEditor::treeModel() const
{
  return m_treeModel;
}

/**
 * @brief Returns the QML-bound selection model for the project tree.
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
 * @brief Returns the source ID of the currently selected source.
 */
int DataModel::ProjectEditor::selectedSourceId() const noexcept
{
  return m_selectedSource.sourceId;
}

/**
 * @brief Returns the bus type of the currently selected source.
 */
int DataModel::ProjectEditor::selectedSourceBusType() const noexcept
{
  return m_selectedSource.busType;
}

/**
 * @brief Returns the frame parser code of the currently selected source.
 */
QString DataModel::ProjectEditor::selectedSourceFrameParserCode() const
{
  return m_selectedSource.frameParserCode;
}

/**
 * @brief Updates the frame parser code of the selected source.
 */
void DataModel::ProjectEditor::setSelectedSourceFrameParserCode(const QString& code)
{
  if (m_selectedSource.frameParserCode == code)
    return;

  m_selectedSource.frameParserCode = code;
  m_projectModelRef.updateSourceFrameParser(m_selectedSource.sourceId, code);
  Q_EMIT selectedSourceFrameParserCodeChanged();
}

/**
 * @brief Returns the JS code attached to the selected painter group.
 */
QString DataModel::ProjectEditor::currentGroupPainterCode() const
{
  const int gid      = m_selectedGroup.groupId;
  const auto& groups = m_projectModelRef.groups();
  if (gid >= 0 && static_cast<size_t>(gid) < groups.size())
    return groups[gid].painterCode;

  return m_selectedGroup.painterCode;
}

/**
 * @brief Returns true when the selected group is a painter widget.
 */
bool DataModel::ProjectEditor::currentGroupIsPainter() const
{
  return m_selectedGroup.widget == QLatin1String("painter");
}

/**
 * @brief Returns the current group's ID, or -1 when no group is selected.
 */
int DataModel::ProjectEditor::currentGroupId() const
{
  return m_selectedGroup.groupId;
}

/**
 * @brief Builds a QVariantList describing the current group's datasets so QML preview tooling can
 * seed simulated values that mirror the real configuration (titles, units, min/max bounds).
 */
QVariantList DataModel::ProjectEditor::currentGroupDatasetsForPreview() const
{
  QVariantList out;
  out.reserve(static_cast<int>(m_selectedGroup.datasets.size()));
  for (const auto& ds : m_selectedGroup.datasets) {
    QVariantMap m;
    m.insert(QStringLiteral("title"), ds.title);
    m.insert(QStringLiteral("units"), ds.units);
    m.insert(QStringLiteral("min"), ds.wgtMin);
    m.insert(QStringLiteral("max"), ds.wgtMax);
    const double mid = (ds.wgtMax + ds.wgtMin) * 0.5;
    m.insert(QStringLiteral("value"), std::isfinite(mid) ? mid : 0.0);
    out.append(m);
  }
  return out;
}

/**
 * @brief Replaces the painter code on the selected group.
 */
void DataModel::ProjectEditor::setCurrentGroupPainterCode(const QString& code)
{
  if (m_selectedGroup.painterCode == code)
    return;

  m_selectedGroup.painterCode = code;
  m_projectModelRef.setNextUndoHint(tr("Edit Canvas Code"),
                                    QStringLiteral("painter-code:%1").arg(m_selectedGroup.groupId));
  m_projectModelRef.updateGroup(m_selectedGroup.groupId, m_selectedGroup, false);
  Q_EMIT currentGroupPainterCodeChanged();
}

/**
 * @brief Updates the transmit function of the selected output widget.
 */
void DataModel::ProjectEditor::setSelectedOutputWidgetTransmitFunction(const QString& code)
{
  if (m_selectedOutputWidget.transmitFunction == code)
    return;

  m_selectedOutputWidget.transmitFunction = code;

  for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
    if (it.value().groupId == m_selectedOutputWidget.groupId
        && it.value().widgetId == m_selectedOutputWidget.widgetId) {
      m_outputWidgetItems[it.key()].transmitFunction = code;
      break;
    }
  }

  m_projectModelRef.setNextUndoHint(tr("Edit Transmit Function"),
                                    QStringLiteral("owidget-tx:%1:%2")
                                      .arg(QString::number(m_selectedOutputWidget.groupId),
                                           QString::number(m_selectedOutputWidget.widgetId)));
  m_projectModelRef.updateOutputWidget(
    m_selectedOutputWidget.groupId, m_selectedOutputWidget.widgetId, m_selectedOutputWidget, false);
}

/**
 * @brief Opens the dataset value transform editor for the selected dataset.
 */
void DataModel::ProjectEditor::openTransformEditor()
{
  openTransformEditorFor(m_selectedDataset.groupId, m_selectedDataset.datasetId);
}

/**
 * @brief Opens the transform editor for an explicit (groupId, datasetId) -- no selection change.
 */
void DataModel::ProjectEditor::openTransformEditorFor(int groupId, int datasetId)
{
  auto& pm           = m_projectModelRef;
  const auto& groups = pm.groups();

  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return;

  const auto& dataset = groups[groupId].datasets[datasetId];

  int lang = dataset.transformLanguage;
  if (lang < 0 || dataset.transformCode.isEmpty()) {
    for (const auto& src : pm.sources())
      if (src.sourceId == dataset.sourceId) {
        lang = src.frameParserLanguage;
        break;
      }
  }

  if (!m_transformEditor) {
    m_transformEditor = new DatasetTransformEditor(nullptr);

    connect(m_transformEditor,
            &DatasetTransformEditor::transformApplied,
            this,
            [this](const QString& code, int langId, int gId, int dId) {
              auto& model     = m_projectModelRef;
              auto& groupList = model.groups();
              if (gId < 0 || static_cast<size_t>(gId) >= groupList.size())
                return;

              if (dId < 0 || static_cast<size_t>(dId) >= groupList[gId].datasets.size())
                return;

              auto updated              = groupList[gId].datasets[dId];
              updated.transformCode     = code;
              updated.transformLanguage = code.isEmpty() ? -1 : langId;
              model.updateDataset(gId, dId, updated, false);

              if (m_selectedDataset.groupId == gId && m_selectedDataset.datasetId == dId) {
                m_selectedDataset.transformCode     = code;
                m_selectedDataset.transformLanguage = updated.transformLanguage;
              }

              for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
                if (it.value().groupId == gId && it.value().datasetId == dId) {
                  it.value().transformCode     = code;
                  it.value().transformLanguage = updated.transformLanguage;
                  break;
                }
              }

              m_frameBuilder.syncFromProjectModel();
            });
  }

  m_transformEditor->displayDialog(dataset.title, dataset.transformCode, lang, groupId, datasetId);
}

/**
 * @brief Returns the form model for the currently selected action.
 */
DataModel::CustomModel* DataModel::ProjectEditor::actionModel() const
{
  return m_actionModel;
}

/**
 * @brief Returns the form model for the project-level settings view.
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
 * @brief Returns the type integer of the currently selected output widget.
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
 * @brief Returns the MQTT Publisher form model exposed to QML.
 */
DataModel::CustomModel* DataModel::ProjectEditor::mqttPublisherModel() const
{
  return m_mqttPublisherModel;
}

/**
 * @brief Returns the currently selected output widget descriptor.
 */
const DataModel::OutputWidget& DataModel::ProjectEditor::selectedOutputWidget() const noexcept
{
  return m_selectedOutputWidget;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects the Frame Parser tree item for the given source, deferred.
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
// Private slot: combobox initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Populates the combobox string lists used by the form models.
 */
void DataModel::ProjectEditor::generateComboBoxModels()
{
  m_fftSamples.clear();
  m_fftSamples << "8" << "16" << "32" << "64" << "128" << "256" << "512" << "1024" << "2048"
               << "4096" << "8192" << "16384" << "32768" << "65536" << "131072" << "262144";

  m_fftWindows.clear();
  m_fftWindowValues.clear();
  m_fftWindows << tr("Rectangular (None)") << tr("Bartlett (Triangular)") << tr("Hann")
               << tr("Hamming") << tr("Blackman") << tr("Blackman-Harris") << tr("Nuttall")
               << tr("Blackman-Nuttall") << tr("Flat Top") << tr("Welch") << tr("Bartlett-Hann")
               << tr("Bohman") << tr("Cosine (Sine)") << tr("Lanczos") << tr("Parzen");
  m_fftWindowValues << SerialStudio::FFTWindowRectangular << SerialStudio::FFTWindowBartlett
                    << SerialStudio::FFTWindowHann << SerialStudio::FFTWindowHamming
                    << SerialStudio::FFTWindowBlackman << SerialStudio::FFTWindowBlackmanHarris
                    << SerialStudio::FFTWindowNuttall << SerialStudio::FFTWindowBlackmanNuttall
                    << SerialStudio::FFTWindowFlatTop << SerialStudio::FFTWindowWelch
                    << SerialStudio::FFTWindowBartlettHann << SerialStudio::FFTWindowBohman
                    << SerialStudio::FFTWindowCosine << SerialStudio::FFTWindowLanczos
                    << SerialStudio::FFTWindowParzen;

  m_timerModes.clear();
  m_timerModes << tr("Off") << tr("Auto Start") << tr("Start on Trigger") << tr("Toggle on Trigger")
               << tr("Repeat N Times");

  m_decoderOptions.clear();
  m_decoderOptions << tr("Plain Text (UTF8)") << tr("Hexadecimal") << tr("Base64")
                   << tr("Binary (Direct)");

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

#ifdef BUILD_COMMERCIAL
  m_imgDetectionModes.clear();
  m_imgDetectionModes << tr("Auto-detect") << tr("Manual Delimiters");

  m_outputWidgetTypes.clear();
  m_outputWidgetTypes << tr("Button") << tr("Slider") << tr("Toggle") << tr("Text Field")
                      << tr("Knob");
#endif

  m_groupWidgets.clear();
  m_groupWidgets.insert(QStringLiteral("barpanel"), tr("Bar Panel"));
  m_groupWidgets.insert(QStringLiteral("datagrid"), tr("Data Grid"));
  m_groupWidgets.insert(QStringLiteral("map"), tr("GPS Map"));
  m_groupWidgets.insert(QStringLiteral("gyro"), tr("Gyroscope"));
  m_groupWidgets.insert(QStringLiteral("multiplot"), tr("Multi-Plot"));
  m_groupWidgets.insert(QStringLiteral("accelerometer"), tr("Accelerometer"));
  m_groupWidgets.insert(QStringLiteral("plot3d"), tr("3D Plot"));
  m_groupWidgets.insert(QStringLiteral("image"), tr("Image View"));
  m_groupWidgets.insert(QStringLiteral("painter"), tr("Canvas Widget"));
  m_groupWidgets.insert(QStringLiteral("webview"), tr("Web View"));
  m_groupWidgets.insert(QLatin1String(""), tr("None"));

  m_datasetWidgets.clear();
  m_datasetWidgets.insert(QLatin1String(""), tr("None"));
  m_datasetWidgets.insert(QStringLiteral("bar"), tr("Bar"));
  m_datasetWidgets.insert(QStringLiteral("gauge"), tr("Gauge"));
  m_datasetWidgets.insert(QStringLiteral("compass"), tr("Compass"));
  m_datasetWidgets.insert(QStringLiteral("meter"), tr("Meter"));

  appendExtensionWidgets();

  m_displayFormats.clear();
  m_displayFormats.insert(QLatin1String(""), tr("Auto"));
  m_displayFormats.insert(QStringLiteral("0d"), tr("Integer (0 decimals)"));
  m_displayFormats.insert(QStringLiteral("1d"), tr("1 decimal"));
  m_displayFormats.insert(QStringLiteral("2d"), tr("2 decimals"));
  m_displayFormats.insert(QStringLiteral("3d"), tr("3 decimals"));
  m_displayFormats.insert(QStringLiteral("sci"), tr("Scientific"));

  m_eolSequences.clear();
  m_eolSequences.insert(QLatin1String(""), tr("None"));
  m_eolSequences.insert(QStringLiteral("\n"), tr("New Line (\\n)"));
  m_eolSequences.insert(QStringLiteral("\r"), tr("Carriage Return (\\r)"));
  m_eolSequences.insert(QStringLiteral("\r\n"), tr("CRLF (\\r\\n)"));

  m_plotOptions.clear();
  m_plotOptions.insert(qMakePair(false, false), tr("No"));
  m_plotOptions.insert(qMakePair(true, false), tr("Yes"));
}

/**
 * @brief Adds one widget-picker entry per installed extension package, in the picker matching the
 *        scope it declared. A bundled package that ships as a built-in implementation is skipped:
 *        its built-in entry already exists and keeps its own label.
 */
void DataModel::ProjectEditor::appendExtensionWidgets()
{
  static auto& catalog = UI::WidgetExtensions::instance();

  const auto packages = catalog.ids();
  for (const auto& id : packages) {
    const auto& package = catalog.descriptor(id);
    if (!package.replaces.isEmpty() || package.title.isEmpty())
      continue;

    if (package.scope == UI::WidgetExtensions::GroupScope)
      m_groupWidgets.insert(id, package.title);

    else
      m_datasetWidgets.insert(id, package.title);
  }
}
