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

#include "ProjectEditor/ProjectEditor.h"

#include <cmath>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>

#include "API/HandlerContext.h"
#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "ProjectEditorItemIds.h"
#include "UI/WidgetExtensions.h"

static_assert(static_cast<int>(DataModel::ProjectEditor::KindNone) == DataModel::KindNone);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindGroup) == DataModel::KindGroup);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindDataset) == DataModel::KindDataset);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindWorkspace)
              == DataModel::KindWorkspace);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindWorkspaceFolder)
              == DataModel::KindWorkspaceFolder);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindAction) == DataModel::KindAction);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindOutputWidget)
              == DataModel::KindOutputWidget);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindMqttPublisher)
              == DataModel::KindMqttPublisher);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindControlScript)
              == DataModel::KindControlScript);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindTransformLibrary)
              == DataModel::KindTransformLibrary);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindScriptsRoot)
              == DataModel::KindScriptsRoot);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindJsLibrary)
              == DataModel::KindJsLibrary);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindExportRoot)
              == DataModel::KindExportRoot);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindGroupFolder)
              == DataModel::KindGroupFolder);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindUserTable)
              == DataModel::KindUserTable);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindTableFolder)
              == DataModel::KindTableFolder);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindSource) == DataModel::KindSource);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindProjectRoot)
              == DataModel::KindProjectRoot);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindFrameParser)
              == DataModel::KindFrameParser);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindGroupsRoot)
              == DataModel::KindGroupsRoot);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindTablesRoot)
              == DataModel::KindTablesRoot);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindSystemDatasets)
              == DataModel::KindSystemDatasets);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindWorkspacesRoot)
              == DataModel::KindWorkspacesRoot);
static_assert(static_cast<int>(DataModel::ProjectEditor::KindInfluxSink)
              == DataModel::KindInfluxSink);
static_assert(static_cast<int>(DataModel::ProjectEditor::SectionHeader)
              == DataModel::RowSectionHeader);
static_assert(static_cast<int>(DataModel::ProjectEditor::ColorPicker) == DataModel::RowColorPicker);

/**
 * @brief Constructs the ProjectEditor singleton: the shared state first, then the eight
 *        sub-objects (declared last so every reference they capture is already bound), then the
 *        wiring and the first tree and project-form build.
 */
DataModel::ProjectEditor::ProjectEditor()
  : m_projectModelRef(DataModel::pipelineModules().projectModel)
  , m_connectionManager(API::handlerContext().connectionManager)
  , m_currentView(ProjectView)
  , m_suppressViewChange(false)
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
  , m_influxSinkItem(nullptr)
  , m_controlScriptItem(nullptr)
  , m_transformLibraryItem(nullptr)
  , m_jsLibraryItem(nullptr)
  , m_scriptsRootItem(nullptr)
  , m_exportRootItem(nullptr)
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
  , m_influxSinkModel(nullptr)
  , m_pendingSelectionKind(PendingSelectionKind::None)
  , m_pendingSelectionGroupId(-1)
  , m_pendingSelectionItemId(-1)
  , m_wiring(*this, m_projectModelRef)
  , m_selection(*this, m_projectModelRef)
  , m_tree(*this, m_projectModelRef)
  , m_forms(*this, m_projectModelRef)
  , m_commit(*this, m_projectModelRef)
  , m_summaries(*this, m_projectModelRef)
  , m_multiSelect(*this, m_projectModelRef)
  , m_mqtt(*this)
  , m_influx(*this)
{
  generateComboBoxModels();

  m_wiring.wireProjectModelRebuilds();
  m_wiring.wireGroupSignals();
  m_wiring.wireDatasetSignals();
  m_wiring.wireActionSignals();
  m_wiring.wireOutputWidgetSignals();
  m_wiring.wireSourceSignals();
  m_wiring.wireSelectionRequests();
  m_wiring.wireEditorSelfSignals();
  m_wiring.wireExternalSignals();

  buildTreeModel();
  buildProjectModel();
}

/**
 * @brief The sub-objects own the lazily-opened editor dialogs and release them themselves.
 */
DataModel::ProjectEditor::~ProjectEditor() = default;

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
 * @brief Returns the InfluxDB Sink form model exposed to QML.
 */
DataModel::CustomModel* DataModel::ProjectEditor::influxSinkModel() const
{
  return m_influxSinkModel;
}

/**
 * @brief Returns the currently selected output widget descriptor.
 */
const DataModel::OutputWidget& DataModel::ProjectEditor::selectedOutputWidget() const noexcept
{
  return m_selectedOutputWidget;
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

//--------------------------------------------------------------------------------------------------
// View state: the facade's own view latch (the enum is nested here, a sub-object header cannot
// name it)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Transitions the editor to the given view.
 */
void DataModel::ProjectEditor::setCurrentView(const DataModel::ProjectEditor::CurrentView view)
{
  if (m_suppressViewChange) [[unlikely]]
    return;

  if (m_currentView == view)
    return;

  m_currentView = view;
  Q_EMIT currentViewChanged();
  Q_EMIT selectedTextChanged();
}

/**
 * @brief Toggles the suppression latch used by the diagram's context-menu actions.
 */
void DataModel::ProjectEditor::setSuppressViewChange(bool suppress) noexcept
{
  m_suppressViewChange = suppress;
}

//--------------------------------------------------------------------------------------------------
// Bodies kept on the facade: the generated dataset-form family and trivial state readers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the FFT section rows to the dataset form model.
 */
void DataModel::ProjectEditor::addFFTSection(CustomModel* model, const DataModel::Dataset& dataset)
{
  buildFftGeneralRows(model, dataset);
  buildFftRangeRows(model, dataset);
}

/**
 * @brief Returns the name of the currently selected user-defined table.
 */
QString DataModel::ProjectEditor::selectedUserTable() const
{
  return m_selectedUserTable;
}

/**
 * @brief Returns the ID of the currently selected workspace, or -1.
 */
int DataModel::ProjectEditor::selectedWorkspaceId() const noexcept
{
  return m_selectedWorkspaceId;
}

/**
 * @brief Returns the id of the currently selected workspace folder.
 */
int DataModel::ProjectEditor::selectedFolderId() const noexcept
{
  return m_selectedFolderId;
}

/**
 * @brief Returns the currently selected group folder id (-1 when none).
 */
int DataModel::ProjectEditor::selectedGroupFolderId() const noexcept
{
  return m_selectedGroupFolderId;
}

/**
 * @brief Returns the currently selected table folder id (-1 when none).
 */
int DataModel::ProjectEditor::selectedTableFolderId() const noexcept
{
  return m_selectedTableFolderId;
}

/**
 * @brief Returns the active tree search query (empty when no filter is set).
 */
const QString& DataModel::ProjectEditor::treeSearchQuery() const noexcept
{
  return m_treeSearchQuery;
}

//--------------------------------------------------------------------------------------------------
// Forwarders to the sub-objects (declared on the facade so the QML surface is unchanged)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forwards to EditorSelection.
 */
bool DataModel::ProjectEditor::canGoBack() const noexcept
{
  return m_selection.canGoBack();
}

/**
 * @brief Forwards to EditorSelection.
 */
bool DataModel::ProjectEditor::canGoForward() const noexcept
{
  return m_selection.canGoForward();
}

/**
 * @brief Forwards to EditorSelection.
 */
int DataModel::ProjectEditor::navDirection() const noexcept
{
  return m_selection.navDirection();
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::navigateBack()
{
  m_selection.navigateBack();
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::navigateForward()
{
  m_selection.navigateForward();
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::selectSource(int sourceId)
{
  m_selection.selectSource(sourceId);
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::selectGroup(int groupId)
{
  m_selection.selectGroup(groupId);
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::selectDataset(int groupId, int datasetId)
{
  m_selection.selectDataset(groupId, datasetId);
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::selectAction(int actionId)
{
  m_selection.selectAction(actionId);
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::selectOutputWidget(int groupId, int widgetId)
{
  m_selection.selectOutputWidget(groupId, widgetId);
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::selectFrameParser(int sourceId)
{
  m_selection.selectFrameParser(sourceId);
}

/**
 * @brief Forwards to EditorSelection.
 */
void DataModel::ProjectEditor::displayFrameParserView(int sourceId)
{
  m_selection.displayFrameParserView(sourceId);
}

/**
 * @brief Forwards to EditorTree.
 */
bool DataModel::ProjectEditor::treeIndexHasChildren(const QModelIndex& index) const
{
  return m_tree.treeIndexHasChildren(index);
}

/**
 * @brief Forwards to EditorTree.
 */
bool DataModel::ProjectEditor::treeIndexExpanded(const QModelIndex& index) const
{
  return m_tree.treeIndexExpanded(index);
}

/**
 * @brief Forwards to EditorTree.
 */
void DataModel::ProjectEditor::buildTreeModel()
{
  m_tree.buildTreeModel();
}

/**
 * @brief Forwards to EditorTree.
 */
void DataModel::ProjectEditor::persistTreeExpansion()
{
  m_tree.persistTreeExpansion();
}

/**
 * @brief Forwards to EditorTree.
 */
void DataModel::ProjectEditor::expandAllTreeItems()
{
  m_tree.expandAllTreeItems();
}

/**
 * @brief Forwards to EditorTree.
 */
void DataModel::ProjectEditor::collapseTreeToOverview()
{
  m_tree.collapseTreeToOverview();
}

/**
 * @brief Forwards to EditorTree.
 */
void DataModel::ProjectEditor::expandTreeToIndex(const QModelIndex& index)
{
  m_tree.expandTreeToIndex(index);
}

/**
 * @brief Forwards to EditorTree.
 */
void DataModel::ProjectEditor::setTreeIndexExpanded(const QModelIndex& index, bool expanded)
{
  m_tree.setTreeIndexExpanded(index, expanded);
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::buildProjectModel()
{
  m_forms.buildProjectModel();
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::buildGroupModel(const DataModel::Group& group)
{
  m_forms.buildGroupModel(group);
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::buildSourceModel(const DataModel::Source& source)
{
  m_forms.buildSourceModel(source);
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::buildActionModel(const DataModel::Action& action)
{
  m_forms.buildActionModel(action);
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::buildDatasetModel(const DataModel::Dataset& dataset)
{
  m_forms.buildDatasetModel(dataset);
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::buildOutputWidgetModel(const DataModel::OutputWidget& widget)
{
  m_forms.buildOutputWidgetModel(widget);
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::openTransformEditor()
{
  m_forms.openTransformEditor();
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::openTransformEditorFor(int groupId, int datasetId)
{
  m_forms.openTransformEditorFor(groupId, datasetId);
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::openAlarmBandsEditorForSelection()
{
  m_forms.openAlarmBandsEditorForSelection();
}

/**
 * @brief Forwards to EditorForms.
 */
void DataModel::ProjectEditor::openFrequencyMarkersEditorForSelection()
{
  m_forms.openFrequencyMarkersEditorForSelection();
}

/**
 * @brief Forwards to EditorCommit.
 */
void DataModel::ProjectEditor::commitAlarmBands(const QVariantList& bands)
{
  m_commit.commitAlarmBands(bands);
}

/**
 * @brief Forwards to EditorCommit.
 */
void DataModel::ProjectEditor::commitFrequencyMarkers(const QVariantList& markers)
{
  m_commit.commitFrequencyMarkers(markers);
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::selectedTreeItems() const
{
  return m_summaries.selectedTreeItems();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::tablesSummary() const
{
  return m_summaries.tablesSummary();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::systemDatasetsSummary() const
{
  return m_summaries.systemDatasetsSummary();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::workspacesSummary() const
{
  return m_summaries.workspacesSummary();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::widgetsForWorkspace(int workspaceId) const
{
  return m_summaries.widgetsForWorkspace(workspaceId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::allWidgetsSummary() const
{
  return m_summaries.allWidgetsSummary();
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::workspaceHasUnresolvedRefs(int workspaceId) const
{
  return m_summaries.workspaceHasUnresolvedRefs(workspaceId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
int DataModel::ProjectEditor::unresolvedWorkspaceWidgetCount() const
{
  return m_summaries.unresolvedWorkspaceWidgetCount();
}

/**
 * @brief Forwards to EditorSummaries.
 */
int DataModel::ProjectEditor::cleanupUnresolvedWorkspaceWidgets()
{
  return m_summaries.cleanupUnresolvedWorkspaceWidgets();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::workspaceFolderTree() const
{
  return m_summaries.workspaceFolderTree();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::workspaceFolderContents(int parentFolderId) const
{
  return m_summaries.workspaceFolderContents(parentFolderId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::groupFolderTree() const
{
  return m_summaries.groupFolderTree();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::groupFolderContents(int parentFolderId) const
{
  return m_summaries.groupFolderContents(parentFolderId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantMap DataModel::ProjectEditor::groupFolderPaths() const
{
  return m_summaries.groupFolderPaths();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::tableFolderTree() const
{
  return m_summaries.tableFolderTree();
}

/**
 * @brief Forwards to EditorSummaries.
 */
QVariantList DataModel::ProjectEditor::tableFolderContents(int parentFolderId) const
{
  return m_summaries.tableFolderContents(parentFolderId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectUserTable(const QString& tableName)
{
  m_summaries.selectUserTable(tableName);
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectWorkspace(int workspaceId)
{
  m_summaries.selectWorkspace(workspaceId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectWorkspaceFolder(int folderId)
{
  m_summaries.selectWorkspaceFolder(folderId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectGroupFolder(int folderId)
{
  m_summaries.selectGroupFolder(folderId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectTableFolder(int folderId)
{
  m_summaries.selectTableFolder(folderId);
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectMqttPublisher()
{
  m_summaries.selectMqttPublisher();
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectInfluxSink()
{
  m_summaries.selectInfluxSink();
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectControlScript()
{
  m_summaries.selectControlScript();
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectTransformLibrary()
{
  m_summaries.selectTransformLibrary();
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectJsLibrary()
{
  m_summaries.selectJsLibrary();
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectProjectScripts()
{
  m_summaries.selectProjectScripts();
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::selectDataExport()
{
  m_summaries.selectDataExport();
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::setTreeSearchQuery(const QString& query)
{
  m_summaries.setTreeSearchQuery(query);
}

/**
 * @brief Forwards to EditorSummaries.
 */
void DataModel::ProjectEditor::confirmCleanupUnresolvedWorkspaceWidgets()
{
  m_summaries.confirmCleanupUnresolvedWorkspaceWidgets();
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::canMoveCurrentUp() const
{
  return m_summaries.canMoveCurrentUp();
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::canMoveCurrentDown() const
{
  return m_summaries.canMoveCurrentDown();
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::moveCurrentGroup(int direction)
{
  return m_summaries.moveCurrentGroup(direction);
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::moveCurrentDataset(int direction)
{
  return m_summaries.moveCurrentDataset(direction);
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::moveCurrentAction(int direction)
{
  return m_summaries.moveCurrentAction(direction);
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::moveCurrentOutputWidget(int direction)
{
  return m_summaries.moveCurrentOutputWidget(direction);
}

/**
 * @brief Forwards to EditorSummaries.
 */
bool DataModel::ProjectEditor::moveWorkspace(int workspaceId, int direction)
{
  return m_summaries.moveWorkspace(workspaceId, direction);
}

/**
 * @brief Forwards to EditorMultiSelect.
 */
void DataModel::ProjectEditor::changeDatasetOptionForSelection(int option, bool checked)
{
  m_multiSelect.changeDatasetOptionForSelection(option, checked);
}

/**
 * @brief Forwards to EditorMqtt.
 */
void DataModel::ProjectEditor::openMqttScriptEditor()
{
  m_mqtt.openMqttScriptEditor();
}

/**
 * @brief Forwards to EditorMqtt.
 */
void DataModel::ProjectEditor::buildMqttPublisherModel()
{
  m_mqtt.buildMqttPublisherModel();
}
