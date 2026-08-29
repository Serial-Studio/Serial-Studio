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
#include <QSet>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/IconRegistry.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#  include "MQTT/PublisherScriptEditor.h"
#endif
#include "DataModel/Project/ProjectEditorIcons.h"
#include "ProjectEditorItemIds.h"

/**
 * @brief Serializes alarm bands into the QVariantList shape the AlarmBandsEditor dialog consumes.
 */
[[nodiscard]] static QVariantList bandsToVariantList(const std::vector<DataModel::AlarmBand>& bands)
{
  QVariantList out;
  out.reserve(static_cast<int>(bands.size()));
  for (const auto& b : bands) {
    QVariantMap entry;
    entry.insert(QStringLiteral("min"), qMin(b.min, b.max));
    entry.insert(QStringLiteral("max"), qMax(b.min, b.max));
    entry.insert(QStringLiteral("severity"), static_cast<int>(b.severity));
    entry.insert(QStringLiteral("color"), b.color);
    entry.insert(QStringLiteral("label"), b.label);
    entry.insert(QStringLiteral("blink"), b.blink);
    out.append(entry);
  }

  return out;
}

/**
 * @brief Field-wise equality of two alarm-band lists, used to detect a shared multi-selection set.
 */
[[nodiscard]] static bool alarmBandsEqual(const std::vector<DataModel::AlarmBand>& a,
                                          const std::vector<DataModel::AlarmBand>& b)
{
  if (a.size() != b.size())
    return false;

  for (size_t i = 0; i < a.size(); ++i)
    if (a[i].min != b[i].min || a[i].max != b[i].max || a[i].severity != b[i].severity
        || a[i].blink != b[i].blink || a[i].color != b[i].color || a[i].label != b[i].label)
      return false;

  return true;
}

/**
 * @brief Rebuilds the project-level settings form model.
 */
void DataModel::ProjectEditor::buildProjectModel()
{
  if (m_projectModel) {
    m_projectModel->disconnect(this);
    m_projectModel->deleteLater();
  }

  m_projectModel = new CustomModel(this);
  const auto& pm = m_projectModelRef;

  static auto& registry = Misc::IconRegistry::instance();
  auto* hdr             = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Project Information"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("project"), 16),
               ParameterIcon);
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
 * @brief Appends the title field and (when applicable) the input-device selector.
 */
void DataModel::ProjectEditor::buildGroupGeneralSection(const DataModel::Group& group)
{
  static auto& registry = Misc::IconRegistry::instance();
  auto* hdr             = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Group Information"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("widgets"), QStringLiteral("group"), 16),
               ParameterIcon);
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
}

/**
 * @brief Appends the multi-source input-device combo for the given group.
 */
void DataModel::ProjectEditor::buildGroupSourceSection(const DataModel::Group& group)
{
  const auto& sources = m_projectModelRef.sources();
  if (sources.size() <= 1)
    return;

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

/**
 * @brief Appends the Image-View configuration fields for groups using the image widget.
 */
void DataModel::ProjectEditor::buildGroupImageSection(const DataModel::Group& group)
{
#ifdef BUILD_COMMERCIAL
  if (group.widget != QLatin1String("image"))
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* imgHdr          = new QStandardItem();
  imgHdr->setData(SectionHeader, WidgetType);
  imgHdr->setData(tr("Image Configuration"), PlaceholderValue);
  imgHdr->setData(registry.icon(QStringLiteral("widgets"), QStringLiteral("image"), 16),
                  ParameterIcon);
  m_groupModel->appendRow(imgHdr);

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
#else
  Q_UNUSED(group);
#endif
}

/**
 * @brief Appends a "Datasets" section header plus one navigable row per dataset, so the group form
 *        carries the folder-style navigation down to its datasets without leaving the model.
 */
void DataModel::ProjectEditor::buildGroupDatasetsSection(const DataModel::Group& group)
{
  if (group.datasets.empty())
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* hdr             = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Datasets"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("dataset"), 16),
               ParameterIcon);
  m_groupModel->appendRow(hdr);

  for (const auto& dataset : group.datasets) {
    const auto widgets = SerialStudio::getDashboardWidgets(dataset);
    QString icon       = registry.icon(QStringLiteral("editor"), QStringLiteral("dataset"), 16);
    if (widgets.count() > 0)
      icon = SerialStudio::dashboardWidgetIcon(widgets.first(), false);

    auto* item = new QStandardItem();
    item->setEditable(false);
    item->setData(true, Active);
    item->setData(NavRow, WidgetType);
    item->setData(icon, ParameterIcon);
    item->setData(dataset.datasetId, ParameterKey);
    item->setData(kGroupView_Dataset, ParameterType);
    item->setData(dataset.title, ParameterName);
    m_groupModel->appendRow(item);
  }
}

/**
 * @brief Appends the multiplot X-Axis selector (Time or Samples) and the log-scale
 *        toggles to the group model.
 */
void DataModel::ProjectEditor::buildGroupXAxisRow(const DataModel::Group& group)
{
  QStringList options;
  options << tr("Time") << tr("Samples");

  const bool samples = SerialStudio::groupXAxisMode(group) == SerialStudio::XAxisMode::Samples;

  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(ComboBox, WidgetType);
  item->setData(options, ComboBoxData);
  item->setData(samples ? 1 : 0, EditableValue);
  item->setData(kGroupView_xAxis, ParameterType);
  item->setData(tr("X-Axis Source"), ParameterName);
  item->setData(tr("Plot every curve against time or against the sample number"),
                ParameterDescription);
  m_groupModel->appendRow(item);

  const bool has_datasets = !group.datasets.empty();
  const bool log_x        = has_datasets && group.datasets.front().pltLogX;
  const bool log_y        = has_datasets && group.datasets.front().pltLogY;

  auto* logXItem = new QStandardItem();
  logXItem->setEditable(samples && has_datasets);
  logXItem->setData(0, PlaceholderValue);
  logXItem->setData(CheckBox, WidgetType);
  logXItem->setData(logXItem->isEditable(), Active);
  logXItem->setData(log_x, EditableValue);
  logXItem->setData(kGroupView_LogX, ParameterType);
  logXItem->setData(tr("Logarithmic X Axis"), ParameterName);
  logXItem->setData(tr("Scale the X axis in decades; available when the X-Axis source is "
                       "Samples (not Time)"),
                    ParameterDescription);
  m_groupModel->appendRow(logXItem);

  auto* logYItem = new QStandardItem();
  logYItem->setEditable(has_datasets);
  logYItem->setData(0, PlaceholderValue);
  logYItem->setData(CheckBox, WidgetType);
  logYItem->setData(logYItem->isEditable(), Active);
  logYItem->setData(log_y, EditableValue);
  logYItem->setData(kGroupView_LogY, ParameterType);
  logYItem->setData(tr("Logarithmic Y Axis"), ParameterName);
  logYItem->setData(tr("Scale the shared Y axis in decades; values at or below zero are "
                       "clamped"),
                    ParameterDescription);
  m_groupModel->appendRow(logYItem);
}

/**
 * @brief Appends the URL row used by the web-view group widget.
 */
void DataModel::ProjectEditor::buildGroupWebViewRow(const DataModel::Group& group)
{
  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(TextField, WidgetType);
  item->setData(group.webViewUrl, EditableValue);
  item->setData(kGroupView_WebUrl, ParameterType);
  item->setData(tr("URL"), ParameterName);
  item->setData(QStringLiteral("https://"), PlaceholderValue);
  item->setData(tr("Web address to load in this widget"), ParameterDescription);
  m_groupModel->appendRow(item);
}

/**
 * @brief Adds the bar-panel orientation combo (Auto / Horizontal / Vertical, spec 0052).
 */
void DataModel::ProjectEditor::buildGroupBarPanelStyleRow(const DataModel::Group& group)
{
  int index = 0;
  if (group.barPanelStyle == QLatin1String("horizontal"))
    index = 1;
  else if (group.barPanelStyle == QLatin1String("vertical"))
    index = 2;

  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(ComboBox, WidgetType);
  item->setData(QStringList{tr("Auto"), tr("Horizontal"), tr("Vertical")}, ComboBoxData);
  item->setData(index, EditableValue);
  item->setData(kGroupView_BarPanelStyle, ParameterType);
  item->setData(tr("Bar Style"), ParameterName);
  item->setData(tr("Bar orientation: automatic, horizontal rows, or vertical columns"),
                ParameterDescription);
  m_groupModel->appendRow(item);
}

/**
 * @brief Rebuilds the group-settings form model for the given group.
 */
void DataModel::ProjectEditor::buildGroupModel(const DataModel::Group& group)
{
  if (m_groupModel) {
    m_groupModel->disconnect(this);
    m_groupModel->deleteLater();
  }

  m_selectedGroup = group;
  m_groupModel    = new CustomModel(this);

  buildGroupGeneralSection(group);
  buildGroupSourceSection(group);

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

  if (group.widget == QStringLiteral("multiplot"))
    buildGroupXAxisRow(group);

  if (group.widget == QStringLiteral("webview"))
    buildGroupWebViewRow(group);

  if (group.widget == QStringLiteral("barpanel"))
    buildGroupBarPanelStyleRow(group);

  buildGroupImageSection(group);
  buildGroupDatasetsSection(group);

  connect(
    m_groupModel, &CustomModel::itemChanged, this, &DataModel::ProjectEditor::onGroupItemChanged);

  Q_EMIT groupModelChanged();
}

/**
 * @brief Appends Identity (title) and Input Device (bus type) rows to the source model.
 */
void DataModel::ProjectEditor::buildSourceCommonRows(const DataModel::Source& source)
{
  static auto& registry = Misc::IconRegistry::instance();
  auto* identHdr        = new QStandardItem();
  identHdr->setData(SectionHeader, WidgetType);
  identHdr->setData(tr("Input Device"), PlaceholderValue);
  identHdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("project"), 16),
                    ParameterIcon);
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

  auto* busItem = new QStandardItem();
  busItem->setEditable(true);
  busItem->setData(true, Active);
  busItem->setData(ComboBox, WidgetType);
  busItem->setData(kSourceView_BusType, ParameterType);
  busItem->setData(source.busType, EditableValue);
  busItem->setData(tr("Bus Type"), ParameterName);
  busItem->setData(tr("Select the hardware interface for this input device"), ParameterDescription);

  QStringList busTypes = {tr("Serial Port"), tr("Network"), tr("Bluetooth LE")};
#ifdef BUILD_COMMERCIAL
  busTypes << tr("Audio Input") << tr("Modbus") << tr("CAN Bus") << tr("Raw USB")
           << tr("HID Device") << tr("Process") << tr("MQTT Subscriber") << tr("OPC UA")
           << tr("Siemens S7") << tr("EtherNet/IP") << tr("IEC 60870-5-104");
#endif

  busItem->setData(busTypes, ComboBoxData);
  m_sourceModel->appendRow(busItem);
}

/**
 * @brief Resolves a 16 px editor-category form icon through the icon registry.
 */
static QString formIcon(const char* id)
{
  static auto& registry = Misc::IconRegistry::instance();
  return registry.iconById(QLatin1String(id), 16);
}

/**
 * @brief Appends the Frame Detection + Payload Processing rows to the source form model.
 */
void DataModel::ProjectEditor::buildSourceFrameDetectionRows(const DataModel::Source& source)
{
  auto* fdHdr = new QStandardItem();
  fdHdr->setData(SectionHeader, WidgetType);
  fdHdr->setData(tr("Frame Detection"), PlaceholderValue);
  fdHdr->setData(formIcon("editor/frame-detection"), ParameterIcon);
  m_sourceModel->appendRow(fdHdr);

  const auto detection     = static_cast<SerialStudio::FrameDetection>(source.frameDetection);
  const bool hexDelimiters = source.hexadecimalDelimiters;

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
    startSeqItem->setData(source.frameStart, EditableValue);
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
    endSeqItem->setData(source.frameEnd, EditableValue);
    endSeqItem->setData(kSourceView_FrameEndSequence, ParameterType);
    endSeqItem->setData(tr("Frame End Delimiter"), ParameterName);
    endSeqItem->setData(tr("e.g. */"), PlaceholderValue);
    endSeqItem->setData(tr("Sequence that marks the end of a data frame"), ParameterDescription);
    m_sourceModel->appendRow(endSeqItem);
  }

  auto* ppHdr = new QStandardItem();
  ppHdr->setData(SectionHeader, WidgetType);
  ppHdr->setData(tr("Payload Processing & Validation"), PlaceholderValue);
  ppHdr->setData(formIcon("editor/data-conversion"), ParameterIcon);
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
}

/**
 * @brief Rebuilds the source-settings form model from the live driver props.
 */
void DataModel::ProjectEditor::buildSourceModel(const DataModel::Source& source)
{
  if (m_sourceModel) {
    m_sourceModel->disconnect(this);
    m_sourceModel->deleteLater();
  }

  m_selectedSource = source;
  m_sourceModel    = new CustomModel(this);

  buildSourceCommonRows(source);

  if (source.busType != static_cast<int>(SerialStudio::BusType::BluetoothLE))
    appendDriverPropertyRows(source);

  buildSourceFrameDetectionRows(source);

  connect(
    m_sourceModel, &CustomModel::itemChanged, this, &DataModel::ProjectEditor::onSourceItemChanged);

  if (m_deviceListConn)
    disconnect(m_deviceListConn);

  m_deviceListConn = connect(
    &m_connectionManager,
    &IO::ConnectionManager::deviceListRefreshed,
    this,
    [this]() { buildSourceModel(m_selectedSource); },
    Qt::QueuedConnection);

  Q_EMIT sourceModelChanged();
}

/**
 * @brief Appends the Connection Settings header and one row per live driver property.
 */
void DataModel::ProjectEditor::appendDriverPropertyRows(const DataModel::Source& source)
{
  auto* driverHdr = new QStandardItem();
  driverHdr->setData(SectionHeader, WidgetType);
  driverHdr->setData(tr("Connection Settings"), PlaceholderValue);
  driverHdr->setData(busTypeIcon(source.busType), ParameterIcon);
  m_sourceModel->appendRow(driverHdr);

  IO::HAL_Driver* driver = m_connectionManager.driverForEditing(source.sourceId);
  if (!driver)
    return;

  const auto widgetForProperty = [](IO::DriverProperty::Type t) -> EditorWidget {
    switch (t) {
      case IO::DriverProperty::Text:
        return TextField;
      case IO::DriverProperty::HexText:
        return HexTextField;
      case IO::DriverProperty::IntField:
        return IntField;
      case IO::DriverProperty::FloatField:
        return FloatField;
      case IO::DriverProperty::CheckBox:
        return CheckBox;
      case IO::DriverProperty::ComboBox:
        return ComboBox;
      case IO::DriverProperty::Password:
        return PasswordField;
    }
    return TextField;
  };

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

    item->setData(widgetForProperty(prop.type), WidgetType);
    if (prop.type == IO::DriverProperty::ComboBox)
      item->setData(prop.options, ComboBoxData);

    item->setData(prop.value, EditableValue);
    m_sourceModel->appendRow(item);
  }
}

/**
 * @brief Appends General Information rows (header, title, icon, target device).
 */
void DataModel::ProjectEditor::buildActionGeneralRows(const DataModel::Action& action)
{
  static auto& registry = Misc::IconRegistry::instance();
  auto* hdr             = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("General Information"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("action"), 16),
               ParameterIcon);
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

  const auto& sources = m_projectModelRef.sources();
  if (sources.size() <= 1)
    return;

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

/**
 * @brief Appends Data Payload rows (binary toggle, payload, encoding, EOL sequence).
 */
void DataModel::ProjectEditor::buildActionPayloadRows(const DataModel::Action& action)
{
  static auto& registry = Misc::IconRegistry::instance();
  auto* hdr             = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Data Payload"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("tx-data"), 16),
               ParameterIcon);
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
}

/**
 * @brief Appends Execution Behavior and Timer Behavior rows for the action form.
 */
void DataModel::ProjectEditor::buildActionTimingRows(const DataModel::Action& action)
{
  static auto& registry = Misc::IconRegistry::instance();
  auto* hdr             = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Execution Behavior"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("output-range"), 16),
               ParameterIcon);
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

  hdr = new QStandardItem();
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("Timer Behavior"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("timer"), 16), ParameterIcon);
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
}

/**
 * @brief Rebuilds the action-settings form model for the given action.
 */
void DataModel::ProjectEditor::buildActionModel(const DataModel::Action& action)
{
  if (m_actionModel) {
    m_actionModel->disconnect(this);
    m_actionModel->deleteLater();
  }

  m_selectedAction = action;
  m_actionModel    = new CustomModel(this);

  buildActionGeneralRows(action);
  buildActionPayloadRows(action);
  buildActionTimingRows(action);

  connect(
    m_actionModel, &CustomModel::itemChanged, this, &DataModel::ProjectEditor::onActionItemChanged);

  Q_EMIT actionModelChanged();
}

/**
 * @brief Rebuilds the dataset-settings form model for the given dataset.
 */
void DataModel::ProjectEditor::buildDatasetModel(const DataModel::Dataset& dataset)
{
  if (m_datasetModel) {
    m_datasetModel->disconnect(this);
    m_datasetModel->deleteLater();
  }

  m_selectedDataset = dataset;
  m_datasetModel    = new CustomModel(this);

  addGeneralSection(m_datasetModel, dataset);
  addPlotSection(m_datasetModel, dataset);
  addFFTSection(m_datasetModel, dataset);
  addWidgetSection(m_datasetModel, dataset);
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
 * @brief Appends the FFT section rows to the dataset form model.
 */
void DataModel::ProjectEditor::addFFTSection(CustomModel* model, const DataModel::Dataset& dataset)
{
  buildFftGeneralRows(model, dataset);
  buildFftRangeRows(model, dataset);
}

/**
 * @brief Emits openAlarmBandsEditor for the currently-selected dataset. LED-only datasets
 *        often leave the widget range unset, so the scale falls back to the plot range and
 *        then to 0-100; an LED dataset with no bands is pre-filled from its ledHigh threshold.
 */
void DataModel::ProjectEditor::openAlarmBandsEditorForSelection()
{
  if (m_currentView == MultiSelectionView && m_batchKind == KindDataset) {
    openAlarmBandsEditorForMultiSelection();
    return;
  }

  double range_min = qMin(m_selectedDataset.wgtMin, m_selectedDataset.wgtMax);
  double range_max = qMax(m_selectedDataset.wgtMin, m_selectedDataset.wgtMax);
  if (range_max <= range_min) {
    range_min = qMin(m_selectedDataset.pltMin, m_selectedDataset.pltMax);
    range_max = qMax(m_selectedDataset.pltMin, m_selectedDataset.pltMax);
  }

  if (range_max <= range_min) {
    range_min = 0;
    range_max = 100;
  }

  QVariantList bands = bandsToVariantList(m_selectedDataset.alarmBands);
  if (bands.isEmpty() && m_selectedDataset.led && m_selectedDataset.ledHigh < range_max) {
    QVariantMap entry;
    entry.insert(QStringLiteral("min"), qMax(range_min, m_selectedDataset.ledHigh));
    entry.insert(QStringLiteral("max"), range_max);
    entry.insert(QStringLiteral("severity"), 1);
    entry.insert(QStringLiteral("color"), QString());
    entry.insert(QStringLiteral("label"), tr("On"));
    entry.insert(QStringLiteral("blink"), false);
    bands.append(entry);
  }

  Q_EMIT openAlarmBandsEditor(
    m_selectedDataset.groupId, m_selectedDataset.datasetId, range_min, range_max, bands);
}

/**
 * @brief Emits openAlarmBandsEditor for a dataset multi-selection: the scale is the union of each
 *        member's effective range, and bands are prefilled only when every dataset already agrees
 *        (otherwise the editor starts empty and Apply writes one common set to all).
 */
void DataModel::ProjectEditor::openAlarmBandsEditorForMultiSelection()
{
  auto& pm = m_projectModelRef;

  QVector<DataModel::Dataset> sel;
  {
    const auto& groups = pm.groups();
    for (const auto& pr : m_batchItems) {
      const int gid = pr.first, dsid = pr.second;
      if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
        continue;

      for (const auto& d : groups[gid].datasets)
        if (d.datasetId == dsid) {
          sel.append(d);
          break;
        }
    }
  }

  if (sel.isEmpty())
    return;

  bool haveRange   = false;
  double range_min = 0, range_max = 100;
  for (const auto& d : sel) {
    double lo = qMin(d.wgtMin, d.wgtMax);
    double hi = qMax(d.wgtMin, d.wgtMax);
    if (hi <= lo) {
      lo = qMin(d.pltMin, d.pltMax);
      hi = qMax(d.pltMin, d.pltMax);
    }

    if (hi <= lo)
      continue;

    range_min = haveRange ? qMin(range_min, lo) : lo;
    range_max = haveRange ? qMax(range_max, hi) : hi;
    haveRange = true;
  }

  if (!haveRange) {
    range_min = 0;
    range_max = 100;
  }

  bool shared            = true;
  const auto& firstBands = sel.first().alarmBands;
  for (int i = 1; i < sel.size() && shared; ++i)
    shared = alarmBandsEqual(sel[i].alarmBands, firstBands);

  const QVariantList bands = shared ? bandsToVariantList(firstBands) : QVariantList();
  Q_EMIT openAlarmBandsEditor(-1, -1, range_min, range_max, bands);
}

/**
 * @brief Emits openFrequencyMarkersEditor for the currently-selected dataset; the editable
 *        frequency range is 0 to Nyquist from the dataset's configured FFT sampling rate.
 */
void DataModel::ProjectEditor::openFrequencyMarkersEditorForSelection()
{
  const double nyquist = qMax(1, m_selectedDataset.fftSamplingRate) * 0.5;

  QVariantList markers;
  markers.reserve(static_cast<int>(m_selectedDataset.fftMarkers.size()));
  for (const auto& m : m_selectedDataset.fftMarkers) {
    QVariantMap entry;
    entry.insert(QStringLiteral("freq"), m.frequency);
    entry.insert(QStringLiteral("endFreq"), m.endFrequency);
    entry.insert(QStringLiteral("label"), m.label);
    entry.insert(QStringLiteral("color"), m.color);
    entry.insert(QStringLiteral("warningDb"),
                 std::isfinite(m.warningDb) ? QVariant(m.warningDb) : QVariant());
    entry.insert(QStringLiteral("alarmDb"),
                 std::isfinite(m.alarmDb) ? QVariant(m.alarmDb) : QVariant());
    markers.append(entry);
  }

  Q_EMIT openFrequencyMarkersEditor(
    m_selectedDataset.groupId, m_selectedDataset.datasetId, nyquist, markers);
}

//--------------------------------------------------------------------------------------------------
// Output widget form model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends General Information rows (header, label, icon, mono toggle, encoding).
 */
void DataModel::ProjectEditor::buildOutputWidgetCommonRows(const DataModel::OutputWidget& widget)
{
  static auto& registry = Misc::IconRegistry::instance();
  auto* hdr             = new QStandardItem();
  hdr->setData(true, Active);
  hdr->setData(SectionHeader, WidgetType);
  hdr->setData(tr("General Information"), PlaceholderValue);
  hdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("widget"), 16),
               ParameterIcon);
  m_outputWidgetModel->appendRow(hdr);

  auto* titleItem = new QStandardItem();
  titleItem->setEditable(true);
  titleItem->setData(true, Active);
  titleItem->setData(TextField, WidgetType);
  titleItem->setData(widget.title, EditableValue);
  titleItem->setData(kOutputWidget_Title, ParameterType);
  titleItem->setData(tr("Label"), ParameterName);
  titleItem->setData(tr("Display label"), PlaceholderValue);
  m_outputWidgetModel->appendRow(titleItem);

  if (widget.type == DataModel::OutputWidgetType::Button) {
    auto* iconItem = new QStandardItem();
    iconItem->setEditable(true);
    iconItem->setData(true, Active);
    iconItem->setData(IconPicker, WidgetType);
    iconItem->setData(widget.icon, EditableValue);
    iconItem->setData(kOutputWidget_Icon, ParameterType);
    iconItem->setData(tr("Button Icon"), ParameterName);
    m_outputWidgetModel->appendRow(iconItem);

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
}

/**
 * @brief Appends initial value (when applicable) and text encoding rows for the output widget.
 */
void DataModel::ProjectEditor::buildOutputWidgetTransmitRow(const DataModel::OutputWidget& widget)
{
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
}

/**
 * @brief Appends min/max/step rows for slider and knob output widgets.
 */
void DataModel::ProjectEditor::buildOutputWidgetValueRows(const DataModel::OutputWidget& widget)
{
  const bool isNumeric = widget.type == DataModel::OutputWidgetType::Slider
                      || widget.type == DataModel::OutputWidgetType::Knob;
  if (!isNumeric)
    return;

  static auto& registry = Misc::IconRegistry::instance();
  auto* rangeHdr        = new QStandardItem();
  rangeHdr->setData(true, Active);
  rangeHdr->setData(SectionHeader, WidgetType);
  rangeHdr->setData(tr("Value Range"), PlaceholderValue);
  rangeHdr->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("output-range"), 16),
                    ParameterIcon);
  m_outputWidgetModel->appendRow(rangeHdr);

  auto* minItem = new QStandardItem();
  minItem->setEditable(true);
  minItem->setData(true, Active);
  minItem->setData(FloatField, WidgetType);
  minItem->setData(widget.minValue, EditableValue);
  minItem->setData(kOutputWidget_MinValue, ParameterType);
  minItem->setData(tr("Minimum Value"), ParameterName);
  m_outputWidgetModel->appendRow(minItem);

  auto* maxItem = new QStandardItem();
  maxItem->setEditable(true);
  maxItem->setData(true, Active);
  maxItem->setData(FloatField, WidgetType);
  maxItem->setData(widget.maxValue, EditableValue);
  maxItem->setData(kOutputWidget_MaxValue, ParameterType);
  maxItem->setData(tr("Maximum Value"), ParameterName);
  m_outputWidgetModel->appendRow(maxItem);

  auto* stepItem = new QStandardItem();
  stepItem->setEditable(true);
  stepItem->setData(true, Active);
  stepItem->setData(FloatField, WidgetType);
  stepItem->setData(widget.stepSize, EditableValue);
  stepItem->setData(kOutputWidget_StepSize, ParameterType);
  stepItem->setData(tr("Step Size"), ParameterName);
  m_outputWidgetModel->appendRow(stepItem);
}

/**
 * @brief Builds the form model for editing an output widget's properties.
 */
void DataModel::ProjectEditor::buildOutputWidgetModel(const DataModel::OutputWidget& widget)
{
  m_selectedOutputWidget = widget;

  if (m_outputWidgetModel) {
    m_outputWidgetModel->disconnect(this);
    m_outputWidgetModel->deleteLater();
  }

  m_outputWidgetModel = new CustomModel(this);

  buildOutputWidgetCommonRows(widget);
  buildOutputWidgetTransmitRow(widget);
  buildOutputWidgetValueRows(widget);

  connect(m_outputWidgetModel,
          &CustomModel::itemChanged,
          this,
          &DataModel::ProjectEditor::onOutputWidgetItemChanged);

  Q_EMIT outputWidgetModelChanged();
}
