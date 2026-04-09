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

#include "DataModel/ProjectModel.h"

#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/FrameParser.h"
#include "DataModel/OutputCodeEditor.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Client.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton instance access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructor for DataModel::ProjectModel.
 *
 * Sets default values and loads the current JSON map file, or starts a new
 * project if none is loaded.
 */
DataModel::ProjectModel::ProjectModel()
  : m_title("")
  , m_frameEndSequence("")
  , m_checksumAlgorithm("")
  , m_frameStartSequence("")
  , m_hexadecimalDelimiters(false)
  , m_frameDecoder(SerialStudio::PlainText)
  , m_frameDetection(SerialStudio::EndDelimiterOnly)
  , m_pointCount(100)
  , m_modified(false)
  , m_silentReload(false)
  , m_filePath("")
  , m_suppressMessageBoxes(false)
{
  // Debounce timer for plugin state saves (plugins may save frequently)
  m_pluginSaveTimer.setSingleShot(true);
  m_pluginSaveTimer.setInterval(1000);
  connect(&m_pluginSaveTimer, &QTimer::timeout, this, [this]() {
    QFile file(m_filePath);
    if (!file.open(QFile::WriteOnly))
      return;

    const auto json = serializeToJson();
    file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
    file.close();
  });

  newJsonFile();
}

/**
 * @brief Returns the singleton instance of DataModel::ProjectModel.
 */
DataModel::ProjectModel& DataModel::ProjectModel::instance()
{
  static ProjectModel singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Document status
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if the project has unsaved modifications.
 */
bool DataModel::ProjectModel::modified() const noexcept
{
  return m_modified;
}

/**
 * @brief Returns the current frame decoder method.
 */
SerialStudio::DecoderMethod DataModel::ProjectModel::decoderMethod() const noexcept
{
  return m_frameDecoder;
}

/**
 * @brief Returns the current frame detection method.
 */
SerialStudio::FrameDetection DataModel::ProjectModel::frameDetection() const noexcept
{
  return m_frameDetection;
}

//--------------------------------------------------------------------------------------------------
// Document information
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the filename portion of the project file path.
 *
 * Returns "New Project" when no file is associated with the project.
 */
QString DataModel::ProjectModel::jsonFileName() const
{
  if (!m_filePath.isEmpty())
    return QFileInfo(m_filePath).fileName();

  return tr("New Project");
}

/**
 * @brief Returns the default directory for JSON project files.
 */
QString DataModel::ProjectModel::jsonProjectsPath() const
{
  return Misc::WorkspaceManager::instance().path("JSON Projects");
}

/**
 * @brief Returns all available X-axis data source names.
 *
 * Includes "Samples" plus every registered dataset, sorted by frame index.
 */
QStringList DataModel::ProjectModel::xDataSources() const
{
  // Build list of available X-axis data sources from all datasets
  QStringList list;
  list.append(tr("Samples"));

  QMap<int, QString> datasets;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto index = dataset.index;
      if (!datasets.contains(index))
        datasets.insert(index, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
}

/**
 * @brief Suppresses modal dialogs when true (API/headless mode).
 */
void DataModel::ProjectModel::setSuppressMessageBoxes(const bool suppress)
{
  m_suppressMessageBoxes = suppress;
}

/**
 * @brief Returns whether modal dialogs are suppressed.
 */
bool DataModel::ProjectModel::suppressMessageBoxes() const noexcept
{
  return m_suppressMessageBoxes;
}

/**
 * @brief Returns the project title.
 */
const QString& DataModel::ProjectModel::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the file path of the current project file.
 */
const QString& DataModel::ProjectModel::jsonFilePath() const noexcept
{
  return m_filePath;
}

/**
 * @brief Returns the frame parser JavaScript source code for the primary source.
 *
 * The code lives exclusively in sources[0].frameParserCode. Returns an empty
 * string when no sources exist.
 */
QString DataModel::ProjectModel::frameParserCode() const
{
  if (m_sources.empty())
    return QString();

  return m_sources[0].frameParserCode;
}

/**
 * @brief Returns the active group ID for the dashboard tab bar.
 *
 * Returns -1 if none is persisted.
 */
int DataModel::ProjectModel::activeGroupId() const
{
  return m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
}

/**
 * @brief Returns the persisted layout for the given group ID.
 * @param groupId The group ID as used by the dashboard.
 * @return Saved layout object, or an empty object if none.
 */
QJsonObject DataModel::ProjectModel::groupLayout(int groupId) const
{
  return m_widgetSettings.value(Keys::layoutKey(groupId)).toObject().value("data").toObject();
}

/**
 * @brief Returns the persisted settings object for a specific widget.
 * @param widgetId Decimal string of the WidgetID.
 * @return Settings QJsonObject, or empty if none saved.
 */
QJsonObject DataModel::ProjectModel::widgetSettings(const QString& widgetId) const
{
  return m_widgetSettings.value(widgetId).toObject();
}

/**
 * @brief Returns the persisted state object for a plugin.
 *
 * Plugin states are stored under "plugin:<pluginId>" keys
 * in the widgetSettings section of the project file.
 *
 * @param pluginId The extension ID (e.g. "digital-indicator").
 * @return State QJsonObject, or empty if none saved.
 */
QJsonObject DataModel::ProjectModel::pluginState(const QString& pluginId) const
{
  return m_widgetSettings.value(QStringLiteral("plugin:") + pluginId).toObject();
}

/**
 * Returns @c true if the project uses features that require a commercial
 * license (e.g. the 3D plot widget).
 */
bool DataModel::ProjectModel::containsCommercialFeatures() const
{
  return SerialStudio::commercialCfg(m_groups);
}

/**
 * @brief Returns the project's dashboard point count (0 = use global default).
 */
int DataModel::ProjectModel::pointCount() const noexcept
{
  return m_pointCount;
}

/**
 * @brief Returns the total number of groups in the project.
 */
int DataModel::ProjectModel::groupCount() const noexcept
{
  return static_cast<int>(m_groups.size());
}

/**
 * @brief Returns the total number of datasets across all groups.
 */
int DataModel::ProjectModel::datasetCount() const
{
  // Sum datasets across all groups
  int count = 0;
  for (const auto& group : m_groups)
    count += static_cast<int>(group.datasets.size());

  return count;
}

/**
 * @brief Returns a const reference to the vector of groups.
 */
const std::vector<DataModel::Group>& DataModel::ProjectModel::groups() const noexcept
{
  return m_groups;
}

/**
 * @brief Returns a const reference to the vector of actions.
 */
const std::vector<DataModel::Action>& DataModel::ProjectModel::actions() const noexcept
{
  return m_actions;
}

/**
 * @brief Returns a const reference to the vector of sources.
 */
const std::vector<DataModel::Source>& DataModel::ProjectModel::sources() const noexcept
{
  return m_sources;
}

/**
 * @brief Returns the number of sources in the project.
 */
int DataModel::ProjectModel::sourceCount() const noexcept
{
  return static_cast<int>(m_sources.size());
}

/**
 * @brief Returns a const reference to the vector of user-defined workspaces.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::workspaces() const noexcept
{
  return m_workspaces;
}

/**
 * @brief Returns the set of auto-generated group IDs hidden by the user.
 */
const QSet<int>& DataModel::ProjectModel::hiddenGroupIds() const noexcept
{
  return m_hiddenGroupIds;
}

/**
 * @brief Returns the number of user-defined workspaces in the project.
 */
int DataModel::ProjectModel::workspaceCount() const noexcept
{
  return static_cast<int>(m_workspaces.size());
}

/**
 * @brief Returns true if the given group ID is hidden from the workspace list.
 */
bool DataModel::ProjectModel::isGroupHidden(int groupId) const
{
  return m_hiddenGroupIds.contains(groupId);
}

/**
 * @brief Persists a single key/value setting for a widget to the project file.
 *
 * No-op when the operation mode is not ProjectFile, when no file path is set,
 * or when no I/O connection or MQTT subscription is active.
 *
 * @param widgetId  Decimal string of the WidgetID.
 * @param key       Setting key (e.g. "interpolate").
 * @param value     Value to persist.
 */
void DataModel::ProjectModel::saveWidgetSetting(const QString& widgetId,
                                                const QString& key,
                                                const QVariant& value)
{
  // Abort if no project file is loaded
  if (m_filePath.isEmpty())
    return;

  // Update in-memory settings and write to disk
  auto obj            = m_widgetSettings.value(widgetId).toObject();
  const auto newValue = QJsonValue::fromVariant(value);
  if (obj.value(key) == newValue)
    return;

  obj.insert(key, newValue);
  m_widgetSettings.insert(widgetId, obj);

  QFile file(m_filePath);
  if (!file.open(QFile::WriteOnly))
    return;

  const auto json = serializeToJson();
  file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
  file.close();
}

/**
 * @brief Persists a plugin's entire state to the project file.
 *
 * Saves the state object under "plugin:<pluginId>" in the
 * widgetSettings section. The project file is written to disk
 * immediately.
 *
 * @param pluginId The extension ID.
 * @param state    JSON object containing the plugin's state.
 */
void DataModel::ProjectModel::savePluginState(const QString& pluginId, const QJsonObject& state)
{
  // Abort if no project file is loaded
  if (m_filePath.isEmpty())
    return;

  // Check for changes before scheduling a debounced write
  const auto key = QStringLiteral("plugin:") + pluginId;
  if (m_widgetSettings.value(key).toObject() == state)
    return;

  // Update in-memory state immediately, debounce disk write
  m_widgetSettings.insert(key, state);
  m_pluginSaveTimer.start();
}

/**
 * @brief Adds a new source to the project with default settings.
 *
 * In GPL builds, a project may have at most one source. Attempting to add a
 * second source shows an informational message and is otherwise a no-op.
 */
void DataModel::ProjectModel::addSource()
{
  // GPL builds only allow a single data source
#ifndef BUILD_COMMERCIAL
  if (!m_sources.empty()) {
    if (!m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(
        tr("Multiple data sources require a Pro license"),
        tr("Serial Studio Pro allows connecting to multiple devices simultaneously. "
           "Please upgrade to unlock this feature."),
        QMessageBox::Information);

    return;
  }
#endif

  const int newId = static_cast<int>(m_sources.size());

  DataModel::Source source;
  source.sourceId              = newId;
  source.title                 = tr("Device %1").arg(QChar('A' + newId));
  source.busType               = static_cast<int>(SerialStudio::BusType::UART);
  source.frameStart            = m_frameStartSequence;
  source.frameEnd              = m_frameEndSequence;
  source.checksumAlgorithm     = m_checksumAlgorithm;
  source.frameDetection        = static_cast<int>(m_frameDetection);
  source.decoderMethod         = static_cast<int>(m_frameDecoder);
  source.hexadecimalDelimiters = m_hexadecimalDelimiters;
  source.frameParserCode       = FrameParser::defaultTemplateCode();

  m_sources.push_back(source);
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceAdded(newId);
}

/**
 * @brief Deletes the source with the given @p sourceId.
 *
 * The default source (id == 0) cannot be deleted.
 * Any groups referencing the deleted source are reassigned to source 0.
 *
 * @param sourceId The id of the source to delete.
 */
void DataModel::ProjectModel::deleteSource(int sourceId)
{
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  return;
#else
  if (sourceId <= 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources.erase(m_sources.begin() + sourceId);

  for (auto& group : m_groups)
    if (group.sourceId == sourceId)
      group.sourceId = 0;
    else if (group.sourceId > sourceId)
      --group.sourceId;

  for (size_t i = 0; i < m_sources.size(); ++i)
    m_sources[i].sourceId = static_cast<int>(i);

  setModified(true);
  Q_EMIT groupsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceDeleted();
#endif
}

/**
 * @brief Duplicates the source with the given @p sourceId.
 * @param sourceId The id of the source to duplicate.
 */
void DataModel::ProjectModel::duplicateSource(int sourceId)
{
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  return;
#else
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  DataModel::Source copy  = m_sources[sourceId];
  copy.sourceId           = static_cast<int>(m_sources.size());
  copy.title              = copy.title + tr(" (Copy)");
  copy.connectionSettings = QJsonObject();

  m_sources.push_back(copy);
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceAdded(copy.sourceId);
#endif
}

/**
 * @brief Updates the source with the given @p sourceId.
 * @param sourceId The id of the source to update.
 * @param source   New source data.
 */
void DataModel::ProjectModel::updateSource(int sourceId, const DataModel::Source& source)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId]          = source;
  m_sources[sourceId].sourceId = sourceId;

  if (sourceId == 0) {
    m_frameStartSequence    = source.frameStart;
    m_frameEndSequence      = source.frameEnd;
    m_checksumAlgorithm     = source.checksumAlgorithm;
    m_hexadecimalDelimiters = source.hexadecimalDelimiters;
    m_frameDetection        = static_cast<SerialStudio::FrameDetection>(source.frameDetection);
    m_frameDecoder          = static_cast<SerialStudio::DecoderMethod>(source.decoderMethod);
    Q_EMIT frameDetectionChanged();
  }

  setModified(true);
  Q_EMIT sourcesChanged();
}

/**
 * @brief Updates the title of the source with the given @p sourceId.
 * @param sourceId The id of the source to update.
 * @param title    New title string.
 */
void DataModel::ProjectModel::updateSourceTitle(int sourceId, const QString& title)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].title = title.simplified();
  setModified(true);
  Q_EMIT sourcesChanged();
}

/**
 * @brief Updates the bus type of the source with the given @p sourceId.
 * @param sourceId The id of the source to update.
 * @param busType  New bus type as int (cast to SerialStudio::BusType at use).
 */
void DataModel::ProjectModel::updateSourceBusType(int sourceId, int busType)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].busType = busType;
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
}

/**
 * @brief Updates the per-source JavaScript frame parser code.
 * @param sourceId The id of the source to update.
 * @param code     The new JavaScript source string.
 */
void DataModel::ProjectModel::updateSourceFrameParser(int sourceId, const QString& code)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].frameParserCode = code;
  DataModel::FrameParser::instance().setSourceCode(sourceId, code);
  setModified(true);
}

/**
 * @brief Snapshots the current driver settings for source @p sourceId into
 *        Source::connectionSettings.
 *
 * Calls driverProperties() on the live driver to get the current values and
 * serialises them to JSON.
 *
 * Uses uiDriverForBusType() instead of driverForEditing() because the caller
 * has already put the shared UI driver into the desired state via
 * setDriverProperty(). Calling driverForEditing() would re-apply the old
 * saved connectionSettings and overwrite the edit that was just made.
 *
 * @param sourceId The source whose connectionSettings should be updated.
 */
void DataModel::ProjectModel::captureSourceSettings(int sourceId)
{
  // Validate source index
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  // Snapshot the UI driver's current properties
  const auto busType     = static_cast<SerialStudio::BusType>(m_sources[sourceId].busType);
  IO::HAL_Driver* driver = IO::ConnectionManager::instance().uiDriverForBusType(busType);
  if (!driver)
    return;

  QJsonObject settings;
  for (const auto& prop : driver->driverProperties())
    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));

  // Save stable hardware identifiers for cross-platform device matching
  const auto deviceId = driver->deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  m_sources[sourceId].connectionSettings = settings;
  setModified(true);
}

/**
 * @brief Restores driver settings from Source::connectionSettings for source @p sourceId.
 *
 * Calls setDriverProperty() on the live driver for every key stored in the JSON
 * snapshot. Works for both source 0 (singleton) and secondary sources.
 *
 * @param sourceId The source whose connectionSettings should be applied.
 */
void DataModel::ProjectModel::restoreSourceSettings(int sourceId)
{
  // Validate source index
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  // Apply saved connection settings to the driver
  const auto& source = m_sources[sourceId];
  if (source.connectionSettings.isEmpty())
    return;

  IO::HAL_Driver* driver = IO::ConnectionManager::instance().driverForEditing(sourceId);
  if (!driver)
    return;

  for (auto it = source.connectionSettings.constBegin(); it != source.connectionSettings.constEnd();
       ++it)
    driver->setDriverProperty(it.key(), it.value().toVariant());

  // Try to match saved hardware identifiers to currently available devices
  const auto deviceIdVal = source.connectionSettings.value(QStringLiteral("deviceId"));
  if (deviceIdVal.isObject())
    driver->selectByIdentifier(deviceIdVal.toObject());
}

/**
 * @brief Directly overwrites source[0].connectionSettings without triggering a
 *        rebuildDevices() cycle.
 *
 * Called by ConnectionManager::onUiDriverConfigurationChanged() to capture the
 * UI-config driver state back into the project model without emitting sourcesChanged()
 * (which would cause an infinite sync loop).
 *
 * @param settings Serialized driver properties to store.
 */
void DataModel::ProjectModel::setSource0ConnectionSettings(const QJsonObject& settings)
{
  if (m_sources.empty())
    return;

  m_sources[0].connectionSettings = settings;
  setModified(true);
}

/**
 * @brief Directly sets source[0].busType without emitting sourceStructureChanged.
 *
 * Used by ConnectionManager to keep the in-memory model in sync with the UI bus type
 * selection without triggering a full device rebuild cycle.
 *
 * @param busType New bus type as int.
 */
void DataModel::ProjectModel::setSource0BusType(int busType)
{
  if (m_sources.empty())
    return;

  m_sources[0].busType = busType;
  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Document saving / export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to save unsaved changes.
 *
 * In API mode, changes are silently discarded. Returns false only if the user
 * explicitly cancels the dialog.
 *
 * @return true if the caller may proceed, false if the user cancelled.
 */
bool DataModel::ProjectModel::askSave()
{
  // Nothing to save if project is unmodified
  if (!modified())
    return true;

  // Nothing to save if not in ProjectFile mode and no file loaded
  const auto opMode = AppState::instance().operationMode();
  if (opMode != SerialStudio::ProjectFile && m_filePath.isEmpty())
    return true;

  // In API mode, silently discard changes
  if (m_suppressMessageBoxes) {
    qWarning() << "[ProjectModel] Discarding unsaved changes (API mode)";
    if (jsonFilePath().isEmpty())
      newJsonFile();
    else {
      const auto path = m_filePath;
      m_silentReload  = true;
      m_filePath.clear();
      openJsonFile(path);
      m_silentReload = false;
    }

    return true;
  }

  auto ret =
    Misc::Utilities::showMessageBox(tr("Do you want to save your changes?"),
                                    tr("You have unsaved modifications in this project!"),
                                    QMessageBox::Question,
                                    APP_NAME,
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Cancel)
    return false;

  if (ret == QMessageBox::Discard) {
    if (jsonFilePath().isEmpty())
      newJsonFile();
    else {
      const auto path = m_filePath;
      m_silentReload  = true;
      m_filePath.clear();
      openJsonFile(path);
      m_silentReload = false;
    }

    return true;
  }

  return saveJsonFile(false);
}

/**
 * @brief Saves the project to disk.
 *
 * Validates the project (title, groups, datasets) and saves the current state.
 * When @p askPath is true or no file path is set, a save-as dialog is shown.
 *
 * @param askPath  When true, always prompt for a save location.
 * @return true if the file was written successfully.
 */
bool DataModel::ProjectModel::saveJsonFile(const bool askPath)
{
  // Validate required project fields before saving
  if (m_title.isEmpty()) {
    if (m_suppressMessageBoxes)
      qWarning() << "[ProjectModel] Project title cannot be empty";
    else
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("Project title cannot be empty!"), QMessageBox::Warning);

    return false;
  }

  if (groupCount() <= 0) {
    if (m_suppressMessageBoxes)
      qWarning() << "[ProjectModel] Project needs at least one group";
    else
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("You need to add at least one group!"), QMessageBox::Warning);

    return false;
  }

  const bool hasImageGroup = std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
    return g.widget == QLatin1String("image");
  });

  if (datasetCount() <= 0 && !hasImageGroup) {
    if (m_suppressMessageBoxes)
      qWarning() << "[ProjectModel] Project needs at least one dataset";
    else
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("You need to add at least one dataset!"), QMessageBox::Warning);

    return false;
  }

  if (jsonFilePath().isEmpty() || askPath) {
    auto* dialog = new QFileDialog(nullptr,
                                   tr("Save Serial Studio Project"),
                                   jsonProjectsPath() + "/" + title() + ".ssproj",
                                   tr("Serial Studio Project Files (*.ssproj)"));

    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setOption(QFileDialog::DontUseNativeDialog);

    connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
      dialog->deleteLater();

      if (path.isEmpty())
        return;

      m_filePath = path;
      (void)finalizeProjectSave();
    });

    dialog->open();
    return false;
  }

  // File already on disk, just write new data to it
  return finalizeProjectSave();
}

/**
 * @brief Saves the project to the given @p path without showing a dialog.
 *
 * Runs the same validation as apiSaveJsonFile(bool), then writes directly to
 * @p path. Designed for headless / API automation where a file dialog is
 * not available.
 *
 * @param path  Absolute destination path for the project file.
 * @return true if the file was written successfully.
 */
bool DataModel::ProjectModel::apiSaveJsonFile(const QString& path)
{
  if (path.isEmpty())
    return false;

  if (m_title.isEmpty()) {
    qWarning() << "[ProjectModel] Project title cannot be empty";
    return false;
  }

  if (groupCount() <= 0) {
    qWarning() << "[ProjectModel] Project needs at least one group";
    return false;
  }

  const bool hasImageGroup = std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
    return g.widget == QLatin1String("image");
  });

  if (datasetCount() <= 0 && !hasImageGroup) {
    qWarning() << "[ProjectModel] Project needs at least one dataset";
    return false;
  }

  m_filePath = path;
  return finalizeProjectSave();
}

/**
 * @brief Serializes the complete project state to a QJsonObject.
 *
 * Used for both file saving and API export.
 *
 * @return QJsonObject containing the full project configuration.
 */
QJsonObject DataModel::ProjectModel::serializeToJson() const
{
  // Build the root JSON object with project metadata
  QJsonObject json;

  json.insert("title", m_title);
  json.insert("pointCount", m_pointCount);
  json.insert("hexadecimalDelimiters", m_hexadecimalDelimiters);

  QJsonArray groupArray;
  for (const auto& group : std::as_const(m_groups))
    groupArray.append(DataModel::serialize(group));

  json.insert("groups", groupArray);

  QJsonArray actionsArray;
  for (const auto& action : std::as_const(m_actions))
    actionsArray.append(DataModel::serialize(action));

  json.insert("actions", actionsArray);

  QJsonArray sourcesArray;
  for (const auto& source : std::as_const(m_sources))
    sourcesArray.append(DataModel::serialize(source));

  json.insert(Keys::Sources, sourcesArray);

  // Serialize user-defined workspaces
  if (!m_workspaces.empty()) {
    QJsonArray workspacesArray;
    for (const auto& ws : std::as_const(m_workspaces))
      workspacesArray.append(DataModel::serialize(ws));

    json.insert(Keys::Workspaces, workspacesArray);
  }

  // Serialize hidden auto-generated group IDs
  if (!m_hiddenGroupIds.isEmpty()) {
    QJsonArray hiddenArray;
    for (const int id : std::as_const(m_hiddenGroupIds))
      hiddenArray.append(id);

    json.insert(Keys::HiddenGroups, hiddenArray);
  }

  if (!m_widgetSettings.isEmpty())
    json.insert(Keys::WidgetSettings, m_widgetSettings);

  return json;
}

//--------------------------------------------------------------------------------------------------
// Signal/slot setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Connects external signals to this model.
 *
 * Wires Dashboard::pointsChanged so the new point count is persisted into the
 * project file whenever the user changes it.
 */
void DataModel::ProjectModel::setupExternalConnections()
{
  connect(&UI::Dashboard::instance(), &UI::Dashboard::pointsChanged, this, [this]() {
    const auto opMode = AppState::instance().operationMode();
    if (opMode != SerialStudio::ProjectFile || m_filePath.isEmpty())
      return;

    const int points = UI::Dashboard::instance().points();
    if (m_pointCount == points)
      return;

    m_pointCount = points;

    QFile file(m_filePath);
    if (!file.open(QFile::WriteOnly))
      return;

    file.write(QJsonDocument(serializeToJson()).toJson(QJsonDocument::Indented));
    file.close();

    Q_EMIT pointCountChanged();
  });

  // Clear transient workspaces and widget settings on disconnect in
  // non-project modes (QuickPlot, DeviceSendsJSON). These modes have no
  // backing file, so workspace data is ephemeral and should not accumulate
  // across sessions or trigger "save changes?" prompts.
  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (!IO::ConnectionManager::instance().isConnected())
        clearTransientState();
    });

#ifdef BUILD_COMMERCIAL
  connect(&MQTT::Client::instance(), &MQTT::Client::connectedChanged, this, [this] {
    if (!MQTT::Client::instance().isConnected())
      clearTransientState();
  });
#endif
}

//--------------------------------------------------------------------------------------------------
// Document initialisation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resets all project state to factory defaults and emits change signals.
 */
void DataModel::ProjectModel::newJsonFile()
{
  // Clear all project data
  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();

  m_frameEndSequence      = "\\n";
  m_checksumAlgorithm     = "";
  m_frameStartSequence    = "$";
  m_hexadecimalDelimiters = false;
  m_title                 = tr("Untitled Project");
  m_pointCount            = 100;
  m_frameDecoder          = SerialStudio::PlainText;
  m_frameDetection        = SerialStudio::EndDelimiterOnly;
  m_widgetSettings        = QJsonObject();

  DataModel::Source defaultSource;
  defaultSource.sourceId              = 0;
  defaultSource.title                 = tr("Device A");
  defaultSource.busType               = static_cast<int>(SerialStudio::BusType::UART);
  defaultSource.frameStart            = m_frameStartSequence;
  defaultSource.frameEnd              = m_frameEndSequence;
  defaultSource.checksumAlgorithm     = m_checksumAlgorithm;
  defaultSource.frameDetection        = static_cast<int>(m_frameDetection);
  defaultSource.decoderMethod         = static_cast<int>(m_frameDecoder);
  defaultSource.hexadecimalDelimiters = m_hexadecimalDelimiters;
  defaultSource.frameParserCode       = FrameParser::defaultTemplateCode();
  m_sources.push_back(defaultSource);

  m_filePath = "";

  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT workspacesChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  setModified(false);
}

/**
 * @brief Sets the project title and marks the project as modified.
 */
void DataModel::ProjectModel::setTitle(const QString& title)
{
  if (m_title != title) {
    m_title = title;
    setModified(true);
    Q_EMIT titleChanged();
  }
}

/**
 * @brief Sets the dashboard point count and syncs it to the Dashboard.
 */
void DataModel::ProjectModel::setPointCount(const int points)
{
  if (m_pointCount == points)
    return;

  m_pointCount = points;

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    UI::Dashboard::instance().setPoints(points);

  setModified(true);
  Q_EMIT pointCountChanged();
}

/**
 * @brief Clears the project file path without changing project data.
 */
void DataModel::ProjectModel::clearJsonFilePath()
{
  if (!m_filePath.isEmpty()) {
    m_filePath.clear();
    Q_EMIT jsonFileChanged();
  }
}

/**
 * @brief Sets the frame start delimiter sequence.
 *
 * Also syncs into sources[0].frameStart in single-source mode so that
 * buildFrameConfig() uses the correct delimiter for the FrameReader.
 */
void DataModel::ProjectModel::setFrameStartSequence(const QString& sequence)
{
  if (m_frameStartSequence == sequence)
    return;

  m_frameStartSequence = sequence;

  if (m_sources.size() == 1)
    m_sources[0].frameStart = sequence;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Sets the frame end delimiter sequence.
 *
 * Also syncs into sources[0].frameEnd in single-source mode so that
 * buildFrameConfig() uses the correct delimiter for the FrameReader.
 */
void DataModel::ProjectModel::setFrameEndSequence(const QString& sequence)
{
  if (m_frameEndSequence == sequence)
    return;

  m_frameEndSequence = sequence;

  if (m_sources.size() == 1)
    m_sources[0].frameEnd = sequence;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Sets the checksum algorithm name.
 *
 * Also syncs into sources[0].checksumAlgorithm in single-source mode so that
 * buildFrameConfig() uses the correct checksum for the FrameReader.
 */
void DataModel::ProjectModel::setChecksumAlgorithm(const QString& algorithm)
{
  if (m_checksumAlgorithm == algorithm)
    return;

  m_checksumAlgorithm = algorithm;

  if (m_sources.size() == 1)
    m_sources[0].checksumAlgorithm = algorithm;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Sets the frame detection strategy.
 *
 * Also syncs into sources[0].frameDetection in single-source mode so that
 * buildFrameConfig() uses the correct detection mode for the FrameReader.
 */
void DataModel::ProjectModel::setFrameDetection(const SerialStudio::FrameDetection detection)
{
  if (m_frameDetection == detection)
    return;

  m_frameDetection = detection;

  if (m_sources.size() == 1)
    m_sources[0].frameDetection = static_cast<int>(detection);

  setModified(true);
  Q_EMIT frameDetectionChanged();
}

//--------------------------------------------------------------------------------------------------
// Document loading / import
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shows a file-open dialog and loads the selected project.
 */
void DataModel::ProjectModel::openJsonFile()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select Project File"), jsonProjectsPath(), tr("Project Files (*.json *.ssproj)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      openJsonFile(path);

    dialog->deleteLater();
  });

  dialog->open();
}

/**
 * @brief Opens and loads a project file from the given path.
 *
 * Reads all project settings, groups, actions, and widget settings from the
 * file. Emits groupsChanged(), actionsChanged(), titleChanged(),
 * jsonFileChanged(), frameDetectionChanged(), and frameParserCodeChanged()
 * after a successful load.
 *
 * @param path Absolute path to the .ssproj or .json project file.
 * @return true if the project was loaded successfully, false on error.
 */
bool DataModel::ProjectModel::openJsonFile(const QString& path)
{
  // Validate path and check for redundant reload
  if (path.isEmpty())
    return false;

  if (m_filePath == path && !m_groups.empty())
    return true;

  QFile file(path);
  QJsonDocument document;
  if (file.open(QFile::ReadOnly)) {
    auto result = Misc::JsonValidator::parseAndValidate(file.readAll());
    if (!result.valid) [[unlikely]] {
      if (m_suppressMessageBoxes)
        qWarning() << "[ProjectModel] JSON validation error:" << result.errorMessage;
      else
        Misc::Utilities::showMessageBox(
          tr("JSON validation error"), result.errorMessage, QMessageBox::Critical);

      return false;
    }

    document = result.document;
    file.close();
  }

  if (document.isEmpty())
    return false;

  // During a silent reload, reset state without emitting signals so the
  // Clear internal data without emitting intermediate signals, so the
  // dashboard doesn't briefly flash empty while we repopulate from disk.
  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();
  m_widgetSettings = QJsonObject();

  m_filePath = path;

  auto json                      = document.object();
  m_title                        = json.value("title").toString();
  m_frameEndSequence             = json.value("frameEnd").toString();
  m_checksumAlgorithm            = json.value("checksum").toString();
  m_frameStartSequence           = json.value("frameStart").toString();
  const QString legacyParserCode = json.value("frameParser").toString();
  m_hexadecimalDelimiters        = json.value("hexadecimalDelimiters").toBool();
  m_frameDecoder = static_cast<SerialStudio::DecoderMethod>(json.value("decoder").toInt());
  m_frameDetection =
    static_cast<SerialStudio::FrameDetection>(json.value("frameDetection").toInt());

  if (!json.contains("frameDetection"))
    m_frameDetection = SerialStudio::StartAndEndDelimiter;

  auto groups = json.value("groups").toArray();
  for (int g = 0; g < groups.count(); ++g) {
    DataModel::Group group;
    group.groupId = g;
    if (DataModel::read(group, groups.at(g).toObject()))
      m_groups.push_back(group);
  }

  auto actions = json.value("actions").toArray();
  for (int a = 0; a < actions.count(); ++a) {
    DataModel::Action action;
    action.actionId = a;
    if (DataModel::read(action, actions.at(a).toObject()))
      m_actions.push_back(action);
  }

  m_sources.clear();
  if (json.contains(Keys::Sources)) {
    auto sourcesArr = json.value(Keys::Sources).toArray();
    for (int s = 0; s < sourcesArr.count(); ++s) {
      DataModel::Source source;
      if (DataModel::read(source, sourcesArr.at(s).toObject()))
        m_sources.push_back(source);
    }
  }

  const bool legacyFormat = !json.contains(Keys::Sources);

  if (m_sources.empty()) {
    DataModel::Source defaultSource;
    defaultSource.sourceId              = 0;
    defaultSource.title                 = tr("Device A");
    auto& cm                            = IO::ConnectionManager::instance();
    defaultSource.busType               = static_cast<int>(cm.busType());
    defaultSource.frameStart            = m_frameStartSequence;
    defaultSource.frameEnd              = m_frameEndSequence;
    defaultSource.checksumAlgorithm     = m_checksumAlgorithm;
    defaultSource.frameDetection        = static_cast<int>(m_frameDetection);
    defaultSource.decoderMethod         = static_cast<int>(m_frameDecoder);
    defaultSource.hexadecimalDelimiters = m_hexadecimalDelimiters;
    defaultSource.frameParserCode =
      legacyParserCode.isEmpty() ? FrameParser::defaultTemplateCode() : legacyParserCode;

    // Capture current UI driver settings so the migrated source matches the setup panel
    IO::HAL_Driver* uiDriver = cm.uiDriverForBusType(cm.busType());
    if (uiDriver) {
      QJsonObject settings;
      for (const auto& prop : uiDriver->driverProperties())
        settings.insert(prop.key, QJsonValue::fromVariant(prop.value));

      const auto deviceId = uiDriver->deviceIdentifier();
      if (!deviceId.isEmpty())
        settings.insert(QStringLiteral("deviceId"), deviceId);

      defaultSource.connectionSettings = settings;
    }

    m_sources.push_back(defaultSource);
  } else if (!m_sources.empty() && m_sources[0].frameParserCode.isEmpty()) {
    m_sources[0].frameParserCode =
      legacyParserCode.isEmpty() ? FrameParser::defaultTemplateCode() : legacyParserCode;
  }

#ifndef BUILD_COMMERCIAL
  if (m_sources.size() > 1) {
    m_sources.resize(1);
    for (auto& g : m_groups)
      if (g.sourceId > 0)
        g.sourceId = 0;

    if (!m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(
        tr("Multi-source projects require a Pro license"),
        tr("This project contains multiple data sources. Only the first source "
           "has been loaded. A Serial Studio Pro license is required to use "
           "multi-source projects."),
        QMessageBox::Information);
    else
      qWarning() << "[ProjectModel] Multi-source project truncated to 1 source (GPL build)";
  }
#endif

  m_widgetSettings = json.value(Keys::WidgetSettings).toObject();

  // Deserialize user-defined workspaces (backward-compat: missing key = empty)
  m_workspaces.clear();
  if (json.contains(Keys::Workspaces)) {
    const auto wsArray = json.value(Keys::Workspaces).toArray();
    for (const auto& val : wsArray) {
      DataModel::Workspace ws;
      if (DataModel::read(ws, val.toObject()))
        m_workspaces.push_back(ws);
    }
  }

  // Deserialize hidden auto-generated group IDs
  m_hiddenGroupIds.clear();
  if (json.contains(Keys::HiddenGroups)) {
    const auto hiddenArray = json.value(Keys::HiddenGroups).toArray();
    for (const auto& val : hiddenArray)
      m_hiddenGroupIds.insert(val.toInt());
  }

  // Read point count from root level (new format) or legacy widgetSettings key
  m_pointCount = UI::Dashboard::instance().points();
  if (json.contains(Keys::PointCount)) {
    const int pts = json.value(Keys::PointCount).toInt();
    if (pts > 0)
      m_pointCount = pts;
  } else if (m_widgetSettings.contains(QStringLiteral("__pointCount__"))) {
    const int pts = m_widgetSettings.value(QStringLiteral("__pointCount__")).toInt();
    if (pts > 0)
      m_pointCount = pts;

    m_widgetSettings.remove(QStringLiteral("__pointCount__"));
  }

  if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
    UI::Dashboard::instance().setPoints(m_pointCount);

  // Migrate legacy "__layout__:N__" keys to "layout:N"
  const auto keys = m_widgetSettings.keys();
  for (const auto& key : keys) {
    bool isOldFormat = key.startsWith(QStringLiteral("__layout__:"));
    bool isNewFormat = key.startsWith(QStringLiteral("layout:"));
    if (!isOldFormat && !isNewFormat)
      continue;

    auto entry = m_widgetSettings.value(key).toObject();
    if (!entry.contains(QStringLiteral("data")))
      continue;

    QJsonObject cleaned;
    cleaned[QStringLiteral("data")] = entry[QStringLiteral("data")];

    if (isOldFormat) {
      m_widgetSettings.remove(key);
      auto id = key.mid(11);
      id.chop(2);
      m_widgetSettings.insert(QStringLiteral("layout:") + id, cleaned);
    } else
      m_widgetSettings.insert(key, cleaned);
  }

  if (json.contains(QStringLiteral("dashboardLayout"))) {
    const int legacy_group_id = json.value(QStringLiteral("activeGroupId")).toInt(-1);
    const auto layout         = json.value(QStringLiteral("dashboardLayout")).toObject();
    if (legacy_group_id >= 0 && !layout.isEmpty())
      m_widgetSettings.insert(Keys::layoutKey(legacy_group_id), layout);

    if (legacy_group_id >= 0)
      m_widgetSettings.insert(Keys::kActiveGroupSubKey, legacy_group_id);
  }

  setModified(false);

  if (json.contains("separator")) {
    const auto separator = json.value("separator").toString();
    static QRegularExpression legacyRegex(
      R"(function\s+parse\s*\(\s*frame\s*,\s*separator\s*\)\s*\{\s*return\s+frame\.split\(separator\);\s*\})");

    if (!m_sources.empty() && legacyRegex.match(m_sources[0].frameParserCode).hasMatch()) {
      if (separator.length() > 1)
        m_sources[0].frameParserCode =
          QStringLiteral("/**\n * Automatically migrated frame parser function.\n"
                         " */\nfunction parse(frame) {\n    return frame.split(\"%1\");\n}")
            .arg(separator);
      else
        m_sources[0].frameParserCode =
          QStringLiteral("/**\n * Automatically migrated frame parser function.\n"
                         " */\nfunction parse(frame) {\n    return frame.split(\'%1\');\n}")
            .arg(separator);

      if (!m_suppressMessageBoxes)
        Misc::Utilities::showMessageBox(
          tr("Legacy frame parser function updated"),
          tr("Your project used a legacy frame parser function with a 'separator' argument. "
             "It has been automatically migrated to the new format."),
          QMessageBox::Information);
      else
        qWarning() << "[ProjectModel] Legacy frame parser function automatically migrated";

      (void)saveJsonFile(false);
      return true;
    }
  }

  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT workspacesChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  if (m_widgetSettings.contains(Keys::kActiveGroupSubKey))
    Q_EMIT activeGroupIdChanged();

  if (!m_widgetSettings.isEmpty())
    Q_EMIT widgetSettingsChanged();

  if (legacyFormat) {
    qInfo() << "[ProjectModel] Migrating legacy project to multi-source format, saving...";
    QFile f(m_filePath);
    if (f.open(QFile::WriteOnly)) {
      f.write(QJsonDocument(serializeToJson()).toJson(QJsonDocument::Indented));
      f.close();
    }
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Selection state (used internally by group/dataset/action operations)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records the currently selected group for data operations.
 */
void DataModel::ProjectModel::setSelectedGroup(const DataModel::Group& group)
{
  m_selectedGroup = group;
}

/**
 * @brief Records the currently selected action for data operations.
 */
void DataModel::ProjectModel::setSelectedAction(const DataModel::Action& action)
{
  m_selectedAction = action;
}

/**
 * @brief Records the currently selected dataset for data operations.
 */
void DataModel::ProjectModel::setSelectedDataset(const DataModel::Dataset& dataset)
{
  m_selectedDataset = dataset;
}

/**
 * @brief Sets the frame decoder method and emits frameDetectionChanged.
 *
 * Also syncs into sources[0].decoderMethod in single-source mode.
 */
void DataModel::ProjectModel::setDecoderMethod(const SerialStudio::DecoderMethod method)
{
  if (m_frameDecoder == method)
    return;

  m_frameDecoder = method;

  if (m_sources.size() == 1)
    m_sources[0].decoderMethod = static_cast<int>(method);

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Toggles hexadecimal delimiter mode and converts existing sequences.
 *
 * Also syncs into sources[0].hexadecimalDelimiters in single-source mode.
 */
void DataModel::ProjectModel::setHexadecimalDelimiters(const bool hexadecimal)
{
  if (m_hexadecimalDelimiters == hexadecimal)
    return;

  m_hexadecimalDelimiters = hexadecimal;

  if (m_sources.size() == 1)
    m_sources[0].hexadecimalDelimiters = hexadecimal;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Replaces the group at @p groupId with @p group and emits groupsChanged.
 */
void DataModel::ProjectModel::updateGroup(const int groupId,
                                          const DataModel::Group& group,
                                          const bool rebuildTree)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  m_groups[groupId] = group;

  if (rebuildTree)
    Q_EMIT groupsChanged();
  else
    Q_EMIT groupDataChanged();

  setModified(true);
}

/**
 * @brief Replaces the dataset at @p groupId/@p datasetId.
 *
 * Emits groupsChanged() only when @p rebuildTree is true (e.g. title/icon change).
 * For scalar-only edits, just updates in-place and marks the project modified.
 */
void DataModel::ProjectModel::updateDataset(const int groupId,
                                            const int datasetId,
                                            const DataModel::Dataset& dataset,
                                            const bool rebuildTree)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  m_groups[groupId].datasets[datasetId] = dataset;
  m_selectedDataset                     = dataset;

  if (rebuildTree)
    Q_EMIT groupsChanged();

  setModified(true);
}

/**
 * @brief Replaces the action at @p actionId with @p action and emits actionsChanged.
 */
void DataModel::ProjectModel::updateAction(const int actionId,
                                           const DataModel::Action& action,
                                           const bool rebuildTree)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  m_actions[actionId] = action;

  if (rebuildTree)
    Q_EMIT actionsChanged();

  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Group / dataset / action mutation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes the currently selected group after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentGroup()
{
  // Confirm deletion with the user unless in API mode
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete group \"%1\"?").arg(m_selectedGroup.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_selectedGroup.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  m_groups.erase(m_groups.begin() + gid);

  int id = 0;
  for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
    g->groupId = id;
    for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
      d->groupId = id;
  }

  Q_EMIT groupsChanged();
  Q_EMIT groupDeleted();
  setModified(true);
}

/**
 * @brief Deletes the currently selected action after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentAction()
{
  // Confirm deletion with the user unless in API mode
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete action \"%1\"?").arg(m_selectedAction.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto aid = m_selectedAction.actionId;
  if (aid < 0 || static_cast<size_t>(aid) >= m_actions.size())
    return;

  m_actions.erase(m_actions.begin() + aid);

  int id = 0;
  for (auto a = m_actions.begin(); a != m_actions.end(); ++a, ++id)
    a->actionId = id;

  Q_EMIT actionsChanged();
  Q_EMIT actionDeleted();
  setModified(true);
}

/**
 * @brief Deletes the currently selected dataset after user confirmation.
 *
 * Removes the parent group too when it becomes empty, then renumbers all
 * group and dataset IDs.
 */
void DataModel::ProjectModel::deleteCurrentDataset()
{
  // Confirm deletion with the user unless in API mode
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete dataset \"%1\"?").arg(m_selectedDataset.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  m_groups[groupId].datasets.erase(m_groups[groupId].datasets.begin() + datasetId);

  if (m_groups[groupId].datasets.empty()) {
    m_groups.erase(m_groups.begin() + groupId);

    int id = 0;
    for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
      g->groupId = id;
      for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
        d->groupId = id;
    }

    Q_EMIT groupsChanged();
    Q_EMIT datasetDeleted(-1);
    setModified(true);
    return;
  }

  int id     = 0;
  auto begin = m_groups[groupId].datasets.begin();
  auto end   = m_groups[groupId].datasets.end();
  for (auto dataset = begin; dataset != end; ++dataset, ++id)
    dataset->datasetId = id;

  Q_EMIT groupsChanged();
  Q_EMIT datasetDeleted(groupId);
  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected group to the project.
 */
void DataModel::ProjectModel::duplicateCurrentGroup()
{
  // Create a copy of the selected group with a new ID
  DataModel::Group group;
  group.groupId = m_groups.size();
  group.widget  = m_selectedGroup.widget;
  group.title   = tr("%1 (Copy)").arg(m_selectedGroup.title);

  for (size_t i = 0; i < m_selectedGroup.datasets.size(); ++i) {
    auto dataset    = m_selectedGroup.datasets[i];
    dataset.groupId = group.groupId;
    dataset.index   = nextDatasetIndex() + static_cast<int>(i);
    group.datasets.push_back(dataset);
  }

  m_groups.push_back(group);

  Q_EMIT groupsChanged();
  Q_EMIT groupAdded(static_cast<int>(m_groups.size()) - 1);
  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected action to the project.
 */
void DataModel::ProjectModel::duplicateCurrentAction()
{
  // Create a copy of the selected action with a new ID
  DataModel::Action action;
  action.actionId             = m_actions.size();
  action.icon                 = m_selectedAction.icon;
  action.txData               = m_selectedAction.txData;
  action.timerMode            = m_selectedAction.timerMode;
  action.repeatCount          = m_selectedAction.repeatCount;
  action.eolSequence          = m_selectedAction.eolSequence;
  action.timerIntervalMs      = m_selectedAction.timerIntervalMs;
  action.title                = tr("%1 (Copy)").arg(m_selectedAction.title);
  action.autoExecuteOnConnect = m_selectedAction.autoExecuteOnConnect;

  m_actions.push_back(action);

  Q_EMIT actionsChanged();
  Q_EMIT actionAdded(static_cast<int>(m_actions.size()) - 1);
  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Output widget CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the given output widget as the current selection.
 */
void DataModel::ProjectModel::setSelectedOutputWidget(const DataModel::OutputWidget& widget)
{
  m_selectedOutputWidget = widget;
}

/**
 * @brief Changes the type of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetType(int type)
{
  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  const auto newType = static_cast<DataModel::OutputWidgetType>(
    qBound(0, type, static_cast<int>(DataModel::OutputWidgetType::RampGenerator)));

  if (widgets[wid].type == newType)
    return;

  widgets[wid].type           = newType;
  m_selectedOutputWidget.type = newType;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(gid, wid);
  setModified(true);
}

/**
 * @brief Sets the icon of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetIcon(const QString& icon)
{
  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  widgets[wid].icon           = icon;
  m_selectedOutputWidget.icon = icon;

  Q_EMIT groupDataChanged();
  Q_EMIT outputWidgetAdded(gid, wid);
  setModified(true);
}

/**
 * @brief Creates a new output group with a default button control.
 */
void DataModel::ProjectModel::addOutputPanel()
{
  const auto saved = m_selectedGroup;
  m_selectedGroup  = DataModel::Group();
  addOutputControl(SerialStudio::OutputButton);
  m_selectedGroup = saved;
}

/**
 * @brief Adds an output control to the project, just like addDataset().
 *
 * If the selected group is an output group, adds to it. Otherwise
 * creates a new output group automatically.
 */
void DataModel::ProjectModel::addOutputControl(const SerialStudio::OutputWidgetType type)
{
  // Use selected group if it's an output group, otherwise create a new one
  int groupId    = -1;
  const auto sel = m_selectedGroup.groupId;
  if (sel >= 0 && static_cast<size_t>(sel) < m_groups.size()
      && m_groups[sel].groupType == DataModel::GroupType::Output)
    groupId = sel;

  // Create new group if needed
  if (groupId < 0) {
    addGroup(tr("Output Controls"), SerialStudio::NoGroupWidget);
    auto& group     = m_groups.back();
    group.groupType = DataModel::GroupType::Output;
    groupId         = group.groupId;
  }

  auto& group = m_groups[groupId];

  // Default title based on type
  QString title;
  switch (type) {
    case SerialStudio::OutputButton:
      title = tr("New Button");
      break;
    case SerialStudio::OutputSlider:
      title = tr("New Slider");
      break;
    case SerialStudio::OutputToggle:
      title = tr("New Toggle");
      break;
    case SerialStudio::OutputTextField:
      title = tr("New Text Field");
      break;
    case SerialStudio::OutputKnob:
      title = tr("New Knob");
      break;
    case SerialStudio::OutputRampGenerator:
      title = tr("New Ramp Generator");
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

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(groupId, ow.widgetId);
  setModified(true);
}

/**
 * @brief Deletes the currently selected output widget after confirmation.
 */
void DataModel::ProjectModel::deleteCurrentOutputWidget()
{
  if (!m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete output widget \"%1\"?").arg(m_selectedOutputWidget.title),
      tr("This action cannot be undone. Do you wish to proceed?"),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes)
      return;
  }

  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  widgets.erase(widgets.begin() + wid);

  // If group is now empty, delete the group too
  if (widgets.empty()) {
    m_groups.erase(m_groups.begin() + gid);

    // Renumber group IDs
    for (int i = 0; i < static_cast<int>(m_groups.size()); ++i)
      m_groups[i].groupId = i;

    Q_EMIT groupsChanged();
    Q_EMIT groupDeleted();
    setModified(true);
    return;
  }

  // Renumber widget IDs
  for (int i = 0; i < static_cast<int>(widgets.size()); ++i)
    widgets[i].widgetId = i;

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetDeleted(gid);
  setModified(true);
}

/**
 * @brief Duplicates the currently selected output widget.
 */
void DataModel::ProjectModel::duplicateCurrentOutputWidget()
{
  const auto gid = m_selectedOutputWidget.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;

  DataModel::OutputWidget ow = m_selectedOutputWidget;
  ow.widgetId                = static_cast<int>(widgets.size());
  ow.title                   = tr("%1 (Copy)").arg(ow.title);

  widgets.push_back(ow);

  Q_EMIT groupsChanged();
  Q_EMIT outputWidgetAdded(gid, ow.widgetId);
  setModified(true);
}

/**
 * @brief Updates an output widget in place.
 */
void DataModel::ProjectModel::updateOutputWidget(int groupId,
                                                 int widgetId,
                                                 const DataModel::OutputWidget& widget,
                                                 bool rebuildTree)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& widgets = m_groups[groupId].outputWidgets;
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= widgets.size())
    return;

  widgets[widgetId] = widget;

  if (rebuildTree)
    Q_EMIT groupsChanged();
  else
    Q_EMIT groupDataChanged();

  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected dataset to its parent group.
 */
void DataModel::ProjectModel::duplicateCurrentDataset()
{
  auto dataset = m_selectedDataset;

  if (dataset.groupId < 0 || static_cast<size_t>(dataset.groupId) >= m_groups.size())
    return;

  dataset.index     = nextDatasetIndex();
  dataset.title     = tr("%1 (Copy)").arg(dataset.title);
  dataset.datasetId = m_groups[dataset.groupId].datasets.size();

  m_groups[dataset.groupId].datasets.push_back(dataset);

  Q_EMIT groupsChanged();
  Q_EMIT datasetAdded(dataset.groupId,
                      static_cast<int>(m_groups[dataset.groupId].datasets.size()) - 1);
  setModified(true);
}

/**
 * @brief Ensures a compatible group is selected before adding a dataset.
 *
 * A compatible group is one whose widget is NoGroupWidget, MultiPlot, or
 * DataGrid. If the current selection is already such a group (or a dataset
 * within one), it is used. Otherwise, the first compatible group is selected.
 * When no compatible group exists, a new one is created.
 */
void DataModel::ProjectModel::ensureValidGroup()
{
  const auto isValidGroup = [](const DataModel::Group& g) -> bool {
    if (g.groupType == DataModel::GroupType::Output)
      return false;

    switch (SerialStudio::groupWidgetFromId(g.widget)) {
      case SerialStudio::MultiPlot:
      case SerialStudio::DataGrid:
      case SerialStudio::NoGroupWidget:
        return true;
      default:
        return false;
    }
  };

  const auto selId      = m_selectedGroup.groupId;
  const bool selInRange = selId >= 0 && static_cast<size_t>(selId) < m_groups.size();

  if (selInRange && isValidGroup(m_groups[selId])) {
    m_selectedGroup = m_groups[selId];
    return;
  }

  for (const auto& group : std::as_const(m_groups)) {
    if (!isValidGroup(group))
      continue;

    m_selectedGroup = group;
    return;
  }

  addGroup(tr("Group"), SerialStudio::NoGroupWidget);
  m_selectedGroup = m_groups.back();
}

/**
 * @brief Adds a new dataset of the given type to the currently selected group.
 *
 * Calls ensureValidGroup() first to guarantee a compatible target group.
 * The dataset title is made unique within the group.
 *
 * @param option Type of dataset to create (Generic, Plot, FFT, Bar, etc.).
 */
void DataModel::ProjectModel::addDataset(const SerialStudio::DatasetOption option)
{
  ensureValidGroup();

  const auto groupId = m_selectedGroup.groupId;
  DataModel::Dataset dataset;
  dataset.groupId = groupId;

  QString title;
  switch (option) {
    case SerialStudio::DatasetGeneric:
      title = tr("New Dataset");
      break;
    case SerialStudio::DatasetPlot:
      title       = tr("New Plot");
      dataset.plt = true;
      break;
    case SerialStudio::DatasetFFT:
      title       = tr("New FFT Plot");
      dataset.fft = true;
      break;
    case SerialStudio::DatasetBar:
      title          = tr("New Level Indicator");
      dataset.widget = QStringLiteral("bar");
      break;
    case SerialStudio::DatasetGauge:
      title          = tr("New Gauge");
      dataset.widget = QStringLiteral("gauge");
      break;
    case SerialStudio::DatasetCompass:
      title             = tr("New Compass");
      dataset.wgtMin    = 0;
      dataset.wgtMax    = 360;
      dataset.alarmLow  = 0;
      dataset.alarmHigh = 0;
      dataset.widget    = QStringLiteral("compass");
      break;
    case SerialStudio::DatasetLED:
      title       = tr("New LED Indicator");
      dataset.led = true;
      break;
    default:
      break;
  }

  int count        = 1;
  QString newTitle = title;
  for (const auto& d : std::as_const(m_groups[groupId].datasets)) {
    if (d.title == newTitle) {
      count++;
      newTitle = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& d : std::as_const(m_groups[groupId].datasets)) {
      if (d.title != newTitle)
        continue;

      count++;
      newTitle    = QString("%1 (%2)").arg(title).arg(count);
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  dataset.title     = newTitle;
  dataset.index     = nextDatasetIndex();
  dataset.datasetId = m_groups[groupId].datasets.size();

  m_groups[groupId].datasets.push_back(dataset);

  Q_EMIT groupsChanged();
  Q_EMIT datasetAdded(groupId, static_cast<int>(m_groups[groupId].datasets.size()) - 1);
  setModified(true);
}

/**
 * @brief Toggles a dataset option flag on the currently selected dataset.
 *
 * Updates the dataset in its parent group and emits groupsChanged().
 *
 * @param option  The option to modify (Plot, FFT, Bar, Gauge, Compass, LED).
 * @param checked Whether to enable (true) or disable (false) the option.
 */
void DataModel::ProjectModel::changeDatasetOption(const SerialStudio::DatasetOption option,
                                                  const bool checked)
{
  switch (option) {
    case SerialStudio::DatasetPlot:
      m_selectedDataset.plt = checked;
      break;
    case SerialStudio::DatasetFFT:
      m_selectedDataset.fft = checked;
      break;
    case SerialStudio::DatasetBar:
      m_selectedDataset.widget = checked ? QStringLiteral("bar") : "";
      break;
    case SerialStudio::DatasetGauge:
      m_selectedDataset.widget = checked ? QStringLiteral("gauge") : "";
      break;
    case SerialStudio::DatasetCompass:
      m_selectedDataset.widget = checked ? QStringLiteral("compass") : "";
      break;
    case SerialStudio::DatasetLED:
      m_selectedDataset.led = checked;
      break;
    default:
      break;
  }

  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  m_groups[groupId].datasets[datasetId] = m_selectedDataset;

  Q_EMIT groupsChanged();
  setModified(true);
}

/**
 * @brief Adds a new action with a unique title to the project.
 */
void DataModel::ProjectModel::addAction()
{
  int count     = 1;
  QString title = tr("New Action");
  for (const auto& action : std::as_const(m_actions)) {
    if (action.title == title) {
      count++;
      title = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& action : std::as_const(m_actions)) {
      if (action.title != title)
        continue;

      count++;
      title       = QString("%1 (%2)").arg(title).arg(count);
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  DataModel::Action action;
  action.title    = title;
  action.actionId = m_actions.size();

  m_actions.push_back(action);

  Q_EMIT actionsChanged();
  Q_EMIT actionAdded(static_cast<int>(m_actions.size()) - 1);
  setModified(true);
}

/**
 * @brief Adds a new group with the given title and widget type.
 *
 * The title is made unique within the project. setGroupWidget() is called
 * to populate default datasets for complex widget types.
 *
 * @param title   Desired group title.
 * @param widget  Widget type to assign.
 */
void DataModel::ProjectModel::addGroup(const QString& title, const SerialStudio::GroupWidget widget)
{
  int count        = 1;
  QString newTitle = title;
  for (const auto& group : std::as_const(m_groups)) {
    if (group.title == newTitle) {
      count++;
      newTitle = QString("%1 (%2)").arg(title).arg(count);
    }
  }

  while (count > 1) {
    bool titleExists = false;
    for (const auto& group : std::as_const(m_groups)) {
      if (group.title != newTitle)
        continue;

      count++;
      newTitle    = QString("%1 (%2)").arg(title).arg(count);
      titleExists = true;
      break;
    }

    if (!titleExists)
      break;
  }

  DataModel::Group group;
  group.title   = newTitle;
  group.groupId = m_groups.size();

  m_groups.push_back(group);
  setGroupWidget(static_cast<int>(m_groups.size()) - 1, widget);

  Q_EMIT groupAdded(static_cast<int>(m_groups.size()) - 1);
  setModified(true);
}

/**
 * @brief Assigns a widget type to the specified group index.
 *
 * For widget types that carry a fixed dataset layout (Accelerometer, Gyroscope,
 * GPS, Plot3D), the existing datasets are replaced by a canonical set.
 *
 * When the group already has datasets and the new widget would replace them,
 * the user is asked to confirm. Returns false if the user declines.
 *
 * @param group   Index into m_groups.
 * @param widget  Widget type to assign.
 * @return true on success, false if the operation was cancelled.
 */
bool DataModel::ProjectModel::setGroupWidget(const int group,
                                             const SerialStudio::GroupWidget widget)
{
  if (group < 0 || group >= m_groups.size()) [[unlikely]]
    return false;

  auto& grp          = m_groups[group];
  const auto groupId = grp.groupId;

  if (!grp.datasets.empty()) {
    if ((widget == SerialStudio::DataGrid || widget == SerialStudio::MultiPlot
         || widget == SerialStudio::NoGroupWidget)
        && (grp.widget == "multiplot" || grp.widget == "datagrid" || grp.widget == "")) {
      grp.widget = "";
    } else {
      auto ret =
        Misc::Utilities::showMessageBox(tr("Are you sure you want to change the group-level "
                                           "widget?"),
                                        tr("Existing datasets for this group will be deleted"),
                                        QMessageBox::Question,
                                        APP_NAME,
                                        QMessageBox::Yes | QMessageBox::No);

      if (ret == QMessageBox::No)
        return false;

      grp.datasets.clear();
    }
  }

  if (widget == SerialStudio::NoGroupWidget)
    grp.widget = "";

  if (widget == SerialStudio::DataGrid)
    grp.widget = "datagrid";

  else if (widget == SerialStudio::MultiPlot)
    grp.widget = "multiplot";

  else if (widget == SerialStudio::Accelerometer) {
    grp.widget = "accelerometer";

    DataModel::Dataset x, y, z;

    x.datasetId = 0;
    y.datasetId = 1;
    z.datasetId = 2;

    x.groupId = groupId;
    y.groupId = groupId;
    z.groupId = groupId;

    x.index = nextDatasetIndex();
    y.index = nextDatasetIndex() + 1;
    z.index = nextDatasetIndex() + 2;

    x.units = "m/s²";
    y.units = "m/s²";
    z.units = "m/s²";

    x.wgtMin    = 0;
    x.wgtMax    = 0;
    y.wgtMin    = 0;
    y.wgtMax    = 0;
    z.wgtMin    = 0;
    z.wgtMax    = 0;
    x.plt       = true;
    y.plt       = true;
    z.plt       = true;
    x.widget    = "x";
    y.widget    = "y";
    z.widget    = "z";
    x.alarmLow  = 0;
    y.alarmLow  = 0;
    z.alarmLow  = 0;
    x.alarmHigh = 0;
    y.alarmHigh = 0;
    z.alarmHigh = 0;
    x.title     = tr("Accelerometer %1").arg("X");
    y.title     = tr("Accelerometer %1").arg("Y");
    z.title     = tr("Accelerometer %1").arg("Z");

    grp.datasets.push_back(x);
    grp.datasets.push_back(y);
    grp.datasets.push_back(z);
  }

  else if (widget == SerialStudio::Gyroscope) {
    grp.widget = "gyro";

    DataModel::Dataset x, y, z;

    x.datasetId = 0;
    y.datasetId = 1;
    z.datasetId = 2;

    x.groupId = groupId;
    y.groupId = groupId;
    z.groupId = groupId;

    x.index = nextDatasetIndex();
    y.index = nextDatasetIndex() + 1;
    z.index = nextDatasetIndex() + 2;

    x.units = "deg/s";
    y.units = "deg/s";
    z.units = "deg/s";

    x.wgtMin    = 0;
    x.wgtMax    = 0;
    y.wgtMin    = 0;
    y.wgtMax    = 0;
    z.wgtMin    = 0;
    z.wgtMax    = 0;
    x.plt       = true;
    y.plt       = true;
    z.plt       = true;
    x.widget    = "x";
    y.widget    = "y";
    z.widget    = "z";
    x.alarmLow  = 0;
    y.alarmLow  = 0;
    z.alarmLow  = 0;
    x.alarmHigh = 0;
    y.alarmHigh = 0;
    z.alarmHigh = 0;
    x.title     = tr("Gyro %1").arg("X");
    y.title     = tr("Gyro %1").arg("Y");
    z.title     = tr("Gyro %1").arg("Z");

    grp.datasets.push_back(x);
    grp.datasets.push_back(y);
    grp.datasets.push_back(z);
  }

  else if (widget == SerialStudio::GPS) {
    grp.widget = "map";

    DataModel::Dataset lat, lon, alt;

    lat.datasetId = 0;
    lon.datasetId = 1;
    alt.datasetId = 2;

    lat.groupId = groupId;
    lon.groupId = groupId;
    alt.groupId = groupId;

    lat.index = nextDatasetIndex();
    lon.index = nextDatasetIndex() + 1;
    alt.index = nextDatasetIndex() + 2;

    lat.units = "°";
    lon.units = "°";
    alt.units = "m";

    lat.widget    = "lat";
    lon.widget    = "lon";
    alt.widget    = "alt";
    lat.alarmLow  = 0;
    lon.alarmLow  = 0;
    alt.alarmLow  = 0;
    lat.alarmHigh = 0;
    lon.alarmHigh = 0;
    alt.alarmHigh = 0;
    lat.wgtMax    = 90.0;
    lat.wgtMin    = -90.0;
    lon.wgtMax    = 180.0;
    lon.wgtMin    = -180.0;
    alt.wgtMin    = -500.0;
    alt.wgtMax    = 1000000.0;
    lat.title     = tr("Latitude");
    lon.title     = tr("Longitude");
    alt.title     = tr("Altitude");

    grp.datasets.push_back(lat);
    grp.datasets.push_back(lon);
    grp.datasets.push_back(alt);
  }

  else if (widget == SerialStudio::Plot3D) {
    grp.widget = "plot3d";

    DataModel::Dataset x, y, z;

    x.datasetId = 0;
    y.datasetId = 1;
    z.datasetId = 2;

    x.groupId = groupId;
    y.groupId = groupId;
    z.groupId = groupId;

    x.index = nextDatasetIndex();
    y.index = nextDatasetIndex() + 1;
    z.index = nextDatasetIndex() + 2;

    x.wgtMin    = 0;
    x.wgtMax    = 0;
    y.wgtMin    = 0;
    y.wgtMax    = 0;
    z.wgtMin    = 0;
    z.wgtMax    = 0;
    x.widget    = "x";
    y.widget    = "y";
    z.widget    = "z";
    x.alarmLow  = 0;
    y.alarmLow  = 0;
    z.alarmLow  = 0;
    x.alarmHigh = 0;
    y.alarmHigh = 0;
    z.alarmHigh = 0;
    x.title     = tr("X");
    y.title     = tr("Y");
    z.title     = tr("Z");

    grp.datasets.push_back(x);
    grp.datasets.push_back(y);
    grp.datasets.push_back(z);
  }

  else if (widget == SerialStudio::ImageView)
    grp.widget = "image";

  m_groups[group] = grp;

  Q_EMIT groupsChanged();
  setModified(true);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Scalar property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the project's modification flag and emits modifiedChanged().
 */
void DataModel::ProjectModel::setModified(const bool modified)
{
  // Don't mark empty projects (no groups) as modified
  if (modified && m_groups.empty())
    return;

  m_modified = modified;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Sets the frame parser JavaScript source code.
 *
 * The code is stored directly in sources[0].frameParserCode, which is the
 * single authoritative location. Emits frameParserCodeChanged() so that
 * FrameParser::readCode() and the JsCodeEditor are notified.
 */
void DataModel::ProjectModel::setFrameParserCode(const QString& code)
{
  if (m_sources.empty() || code == m_sources[0].frameParserCode)
    return;

  m_sources[0].frameParserCode = code;
  setModified(true);
  Q_EMIT frameParserCodeChanged();
}

/**
 * @brief Stores frame parser code for the given source without emitting
 *        signals or reloading the JS engine.
 *
 * Used by the JsCodeEditor on every keystroke so that the project model
 * always has the latest code. The expensive FrameParser engine reload
 * only happens when the user explicitly clicks Evaluate/Apply.
 *
 * @param sourceId The source to update.
 * @param code     The new JavaScript source string.
 */
void DataModel::ProjectModel::storeFrameParserCode(int sourceId, const QString& code)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  if (m_sources[sourceId].frameParserCode == code)
    return;

  m_sources[sourceId].frameParserCode = code;
  setModified(true);
}

/**
 * @brief Persists the active dashboard tab group ID.
 *
 * Written into m_widgetSettings so it survives a save/load cycle.
 */
void DataModel::ProjectModel::setActiveGroupId(const int groupId)
{
  const int current = m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
  if (current == groupId)
    return;

  if (groupId >= 0)
    m_widgetSettings.insert(Keys::kActiveGroupSubKey, groupId);
  else
    m_widgetSettings.remove(Keys::kActiveGroupSubKey);

  // Silent disk write — no signals, no reload
  if (!m_filePath.isEmpty()) {
    QFile file(m_filePath);
    if (file.open(QFile::WriteOnly)) {
      file.write(QJsonDocument(serializeToJson()).toJson(QJsonDocument::Indented));
      file.close();
    }
  }

  Q_EMIT activeGroupIdChanged();
}

/**
 * @brief Persists the widget layout for a specific group.
 *
 * Stores the layout under the canonical key produced by Keys::layoutKey().
 * When a file path is set the project file is updated immediately.
 *
 * @param groupId  The group whose layout is being saved.
 * @param layout   The layout QJsonObject to store.
 */
void DataModel::ProjectModel::setGroupLayout(const int groupId, const QJsonObject& layout)
{
  QJsonObject entry;
  entry[QStringLiteral("data")] = layout;
  m_widgetSettings.insert(Keys::layoutKey(groupId), entry);

  if (m_filePath.isEmpty())
    return;

  QFile file(m_filePath);
  if (!file.open(QFile::WriteOnly))
    return;

  file.write(QJsonDocument(serializeToJson()).toJson(QJsonDocument::Indented));
  file.close();
}

//--------------------------------------------------------------------------------------------------
// Workspace CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new user-defined workspace with the given title.
 *
 * Assigns the next available workspace ID (max existing + 1, starting at 1000
 * to avoid collision with auto-generated group-based workspace IDs).
 */
void DataModel::ProjectModel::addWorkspace(const QString& title)
{
  int maxId = 999;
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId > maxId)
      maxId = ws.workspaceId;

  DataModel::Workspace ws;
  ws.workspaceId = maxId + 1;
  ws.title       = title.simplified();
  m_workspaces.push_back(ws);

  setModified(true);
  Q_EMIT workspacesChanged();
  writeProjectFile();
}

/**
 * @brief Deletes the workspace with the given ID.
 */
void DataModel::ProjectModel::deleteWorkspace(int workspaceId)
{
  auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(), [workspaceId](const auto& ws) {
    return ws.workspaceId == workspaceId;
  });

  if (it == m_workspaces.end())
    return;

  m_workspaces.erase(it);
  setModified(true);
  Q_EMIT workspacesChanged();
  writeProjectFile();
}

/**
 * @brief Renames the workspace with the given ID.
 */
void DataModel::ProjectModel::renameWorkspace(int workspaceId, const QString& title)
{
  for (auto& ws : m_workspaces) {
    if (ws.workspaceId == workspaceId) {
      ws.title = title.simplified();
      setModified(true);
      Q_EMIT workspacesChanged();
      writeProjectFile();
      return;
    }
  }
}

/**
 * @brief Appends a widget reference to the specified workspace.
 */
void DataModel::ProjectModel::addWidgetToWorkspace(int workspaceId,
                                                   int widgetType,
                                                   int groupId,
                                                   int relativeIndex)
{
  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    // Avoid duplicates
    for (const auto& ref : ws.widgetRefs)
      if (ref.widgetType == widgetType && ref.groupId == groupId
          && ref.relativeIndex == relativeIndex)
        return;

    DataModel::WidgetRef ref;
    ref.widgetType    = widgetType;
    ref.groupId       = groupId;
    ref.relativeIndex = relativeIndex;
    ws.widgetRefs.push_back(ref);

    setModified(true);
    Q_EMIT workspacesChanged();
    writeProjectFile();
    return;
  }
}

/**
 * @brief Removes a widget reference from the specified workspace by index.
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId, int index)
{
  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    if (index < 0 || static_cast<size_t>(index) >= ws.widgetRefs.size())
      return;

    ws.widgetRefs.erase(ws.widgetRefs.begin() + index);
    setModified(true);
    Q_EMIT workspacesChanged();
    writeProjectFile();
    return;
  }
}

/**
 * @brief Hides an auto-generated group workspace from the workspace list.
 *
 * Does not affect the Overview workspace (id < 0). The hidden state is
 * persisted in the project file.
 */
void DataModel::ProjectModel::hideGroup(int groupId)
{
  if (groupId < 0)
    return;

  if (m_hiddenGroupIds.contains(groupId))
    return;

  m_hiddenGroupIds.insert(groupId);
  setModified(true);
  Q_EMIT workspacesChanged();
  writeProjectFile();
}

/**
 * @brief Restores a previously hidden auto-generated group workspace.
 */
void DataModel::ProjectModel::showGroup(int groupId)
{
  if (!m_hiddenGroupIds.remove(groupId))
    return;

  setModified(true);
  Q_EMIT workspacesChanged();
  writeProjectFile();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the current project state to disk if a file path is set.
 *
 * Convenience wrapper used by workspace CRUD methods to persist changes
 * immediately rather than waiting for an explicit save.
 */
void DataModel::ProjectModel::writeProjectFile()
{
  if (m_filePath.isEmpty())
    return;

  QFile file(m_filePath);
  if (!file.open(QFile::WriteOnly))
    return;

  file.write(QJsonDocument(serializeToJson()).toJson(QJsonDocument::Indented));
  file.close();
}

/**
 * @brief Clears workspaces and widget settings when disconnecting in
 *        non-project modes (QuickPlot / DeviceSendsJSON).
 *
 * In these modes there is no backing project file, so any workspaces or
 * widget settings created during a session are transient and should not
 * survive across connection cycles.
 */
void DataModel::ProjectModel::clearTransientState()
{
  const auto opMode = AppState::instance().operationMode();
  if (opMode == SerialStudio::ProjectFile)
    return;

  m_hiddenGroupIds.clear();

  if (!m_workspaces.empty()) {
    m_workspaces.clear();
    Q_EMIT workspacesChanged();
  }

  if (!m_widgetSettings.isEmpty()) {
    m_widgetSettings = QJsonObject();
    Q_EMIT widgetSettingsChanged();
  }

  setModified(false);
}

/**
 * @brief Returns the next available dataset frame index.
 *
 * Scans all datasets for the maximum index and returns max + 1.
 */
int DataModel::ProjectModel::nextDatasetIndex()
{
  int maxIndex = 1;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets)
      if (dataset.index >= maxIndex)
        maxIndex = dataset.index + 1;
  }

  return maxIndex;
}

/**
 * @brief Returns a snapshot of all sources suitable for QML diagram consumption.
 * @return QVariantList of QVariantMaps with keys: sourceId, busType, title.
 */
QVariantList DataModel::ProjectModel::sourcesForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_sources.size()));

  for (const auto& src : m_sources) {
    QVariantMap map;
    map[QStringLiteral("sourceId")] = src.sourceId;
    map[QStringLiteral("busType")]  = src.busType;
    map[QStringLiteral("title")]    = src.title;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all groups (with their datasets) for QML diagram consumption.
 * @return QVariantList of QVariantMaps with keys: groupId, sourceId, title, widget, datasets.
 */
QVariantList DataModel::ProjectModel::groupsForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_groups.size()));

  for (const auto& grp : m_groups) {
    QVariantList datasets;
    datasets.reserve(static_cast<qsizetype>(grp.datasets.size()));

    for (const auto& ds : grp.datasets) {
      QVariantMap dsMap;
      dsMap[QStringLiteral("datasetId")] = ds.datasetId;
      dsMap[QStringLiteral("title")]     = ds.title;
      dsMap[QStringLiteral("units")]     = ds.units;
      dsMap[QStringLiteral("widget")]    = ds.widget;
      datasets.append(dsMap);
    }

    QVariantMap map;
    map[QStringLiteral("groupId")] = grp.groupId;
    QVariantList outputWidgets;
    outputWidgets.reserve(static_cast<qsizetype>(grp.outputWidgets.size()));
    for (const auto& ow : grp.outputWidgets) {
      QVariantMap owMap;
      owMap[QStringLiteral("title")] = ow.title;
      owMap[QStringLiteral("type")]  = static_cast<int>(ow.type);
      outputWidgets.append(owMap);
    }

    map[QStringLiteral("sourceId")]      = grp.sourceId;
    map[QStringLiteral("title")]         = grp.title;
    map[QStringLiteral("widget")]        = grp.widget;
    map[QStringLiteral("groupType")]     = static_cast<int>(grp.groupType);
    map[QStringLiteral("datasets")]      = datasets;
    map[QStringLiteral("outputWidgets")] = outputWidgets;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all actions suitable for QML diagram consumption.
 * @return QVariantList of QVariantMaps with keys: actionId, title, icon.
 */
QVariantList DataModel::ProjectModel::actionsForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_actions.size()));

  for (const auto& act : m_actions) {
    QVariantMap map;
    map[QStringLiteral("actionId")] = act.actionId;
    map[QStringLiteral("sourceId")] = act.sourceId;
    map[QStringLiteral("title")]    = act.title;
    map[QStringLiteral("icon")]     = Misc::IconEngine::resolveActionIconSource(act.icon);
    result.append(map);
  }

  return result;
}

/**
 * @brief Writes the current project to m_filePath and reloads it.
 *
 * Switches to ProjectFile mode via AppState and emits jsonFileChanged.
 *
 * @return true if the file was written successfully.
 */
bool DataModel::ProjectModel::finalizeProjectSave()
{
  QFile file(m_filePath);
  if (!file.open(QFile::WriteOnly)) {
    if (m_suppressMessageBoxes)
      qWarning() << "[ProjectModel] File open error:" << file.errorString();
    else
      Misc::Utilities::showMessageBox(
        tr("File open error"), file.errorString(), QMessageBox::Critical);

    return false;
  }

  const QJsonObject json = serializeToJson();
  file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
  file.close();

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  setModified(false);
  Q_EMIT jsonFileChanged();
  Q_EMIT sourceStructureChanged();
  return true;
}
