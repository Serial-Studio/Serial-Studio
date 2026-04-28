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

#include <algorithm>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/FrameParser.h"
#include "DataModel/OutputCodeEditor.h"
#include "DataModel/ProjectEditor.h"
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
 * @brief Constructs the ProjectModel singleton and seeds an empty project.
 */
DataModel::ProjectModel::ProjectModel()
  : m_title("")
  , m_frameEndSequence("")
  , m_checksumAlgorithm("")
  , m_frameStartSequence("")
  , m_writerVersionAtCreation("")
  , m_hexadecimalDelimiters(false)
  , m_frameDecoder(SerialStudio::PlainText)
  , m_frameDetection(SerialStudio::EndDelimiterOnly)
  , m_pointCount(100)
  , m_modified(false)
  , m_silentReload(false)
  , m_filePath("")
  , m_suppressMessageBoxes(false)
  , m_customizeWorkspaces(false)
{
  // newJsonFile() must run before the auto-regen hook is wired — it emits
  // groupsChanged, and regenerateAutoWorkspaces() reaches into AppState,
  // which is itself mid-init on the very first ProjectModel::instance() call.
  newJsonFile();

  // Any structural change rebuilds the auto-generated workspace list. When
  // the user has opted into manual editing, merge new auto refs into the
  // hand-edited list instead so additions still propagate.
  connect(this, &ProjectModel::groupsChanged, this, [this] {
    if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
      return;

    if (m_customizeWorkspaces) {
      if (mergeAutoWorkspaceUpdates()) {
        Q_EMIT editorWorkspacesChanged();
        Q_EMIT activeWorkspacesChanged();
      }
      return;
    }

    regenerateAutoWorkspaces();
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
  });
}

/**
 * @brief Returns the singleton ProjectModel instance.
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
 * @brief Returns true when the project has unsaved edits.
 */
bool DataModel::ProjectModel::modified() const noexcept
{
  return m_modified;
}

/**
 * @brief Returns the project-wide payload decoder method.
 */
SerialStudio::DecoderMethod DataModel::ProjectModel::decoderMethod() const noexcept
{
  return m_frameDecoder;
}

/**
 * @brief Returns the project-wide frame detection strategy.
 */
SerialStudio::FrameDetection DataModel::ProjectModel::frameDetection() const noexcept
{
  return m_frameDetection;
}

//--------------------------------------------------------------------------------------------------
// Document information
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the project filename, or "New Project" when none is loaded.
 */
QString DataModel::ProjectModel::jsonFileName() const
{
  if (!m_filePath.isEmpty())
    return QFileInfo(m_filePath).fileName();

  return tr("New Project");
}

/**
 * @brief Returns the workspace folder used for project files.
 */
QString DataModel::ProjectModel::jsonProjectsPath() const
{
  return Misc::WorkspaceManager::instance().path("JSON Projects");
}

/**
 * @brief Returns "Samples" plus every dataset label, sorted by frame index.
 */
QStringList DataModel::ProjectModel::xDataSources() const
{
  // Start with the implicit "Samples" option
  QStringList list;
  list.append(tr("Samples"));

  // Collect unique datasets sorted by frame index
  QMap<int, QString> datasets;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto index = dataset.index;
      if (!datasets.contains(index))
        datasets.insert(index, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  // Append dataset labels in index order
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
 * @brief Returns true when modal dialogs are suppressed (API/headless mode).
 */
bool DataModel::ProjectModel::suppressMessageBoxes() const noexcept
{
  return m_suppressMessageBoxes;
}

/**
 * @brief Returns the current project title.
 */
const QString& DataModel::ProjectModel::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the absolute path of the loaded project file, or empty.
 */
const QString& DataModel::ProjectModel::jsonFilePath() const noexcept
{
  return m_filePath;
}

/**
 * @brief Returns the frame parser source code from source 0.
 */
QString DataModel::ProjectModel::frameParserCode() const
{
  if (m_sources.empty())
    return QString();

  return m_sources[0].frameParserCode;
}

/**
 * @brief Returns the scripting language for the global frame parser (source 0).
 */
int DataModel::ProjectModel::frameParserLanguage() const
{
  if (m_sources.empty())
    return 0;

  return m_sources[0].frameParserLanguage;
}

/**
 * @brief Returns the scripting language for the source, or source 0's.
 */
int DataModel::ProjectModel::frameParserLanguage(int sourceId) const
{
  for (const auto& src : m_sources)
    if (src.sourceId == sourceId)
      return src.frameParserLanguage;

  return frameParserLanguage();
}

/**
 * @brief Returns the active group ID for the dashboard tab bar, or -1.
 */
int DataModel::ProjectModel::activeGroupId() const
{
  return m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
}

/**
 * @brief Returns the persisted layout for the given group ID.
 */
QJsonObject DataModel::ProjectModel::groupLayout(int groupId) const
{
  return m_widgetSettings.value(Keys::layoutKey(groupId)).toObject().value("data").toObject();
}

/**
 * @brief Returns the persisted settings object for the given widget.
 */
QJsonObject DataModel::ProjectModel::widgetSettings(const QString& widgetId) const
{
  return m_widgetSettings.value(widgetId).toObject();
}

/**
 * @brief Returns the persisted state object for the given plugin.
 */
QJsonObject DataModel::ProjectModel::pluginState(const QString& pluginId) const
{
  return m_widgetSettings.value(QStringLiteral("plugin:") + pluginId).toObject();
}

/**
 * @brief Returns true if the project uses any commercial-only features.
 */
bool DataModel::ProjectModel::containsCommercialFeatures() const
{
  return SerialStudio::commercialCfg(m_groups);
}

/**
 * @brief Returns the dashboard point count (0 = use global default).
 */
int DataModel::ProjectModel::pointCount() const noexcept
{
  return m_pointCount;
}

/**
 * @brief Returns the number of groups in the project.
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
  int count = 0;
  for (const auto& group : m_groups)
    count += static_cast<int>(group.datasets.size());

  return count;
}

/**
 * @brief Returns the project's group list.
 */
const std::vector<DataModel::Group>& DataModel::ProjectModel::groups() const noexcept
{
  return m_groups;
}

/**
 * @brief Returns the project's action list.
 */
const std::vector<DataModel::Action>& DataModel::ProjectModel::actions() const noexcept
{
  return m_actions;
}

/**
 * @brief Returns the project's data-source list.
 */
const std::vector<DataModel::Source>& DataModel::ProjectModel::sources() const noexcept
{
  return m_sources;
}

/**
 * @brief Returns the number of configured data sources.
 */
int DataModel::ProjectModel::sourceCount() const noexcept
{
  return static_cast<int>(m_sources.size());
}

/**
 * @brief Returns the editor-owned workspace list (always m_workspaces).
 */
const std::vector<DataModel::Workspace>&
DataModel::ProjectModel::editorWorkspaces() const noexcept
{
  return m_workspaces;
}

/**
 * @brief Returns the workspace list currently rendered by the dashboard.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::activeWorkspaces() const
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return m_sessionWorkspaces;

  return m_workspaces;
}

/**
 * @brief Returns the set of hidden auto-generated group IDs.
 */
const QSet<int>& DataModel::ProjectModel::hiddenGroupIds() const noexcept
{
  return m_hiddenGroupIds;
}

/**
 * @brief Returns the number of workspaces defined in the project.
 */
int DataModel::ProjectModel::workspaceCount() const noexcept
{
  return static_cast<int>(m_workspaces.size());
}

/**
 * @brief Returns true when the auto-generated group workspace is hidden.
 */
bool DataModel::ProjectModel::isGroupHidden(int groupId) const
{
  return m_hiddenGroupIds.contains(groupId);
}

/**
 * @brief Returns the number of user-defined tables in the project.
 */
int DataModel::ProjectModel::tableCount() const noexcept
{
  return static_cast<int>(m_tables.size());
}

/**
 * @brief Returns the project's user-defined data table list.
 */
const std::vector<DataModel::TableDef>& DataModel::ProjectModel::tables() const noexcept
{
  return m_tables;
}

/**
 * @brief Stages a single widget setting and marks the project dirty.
 */
void DataModel::ProjectModel::saveWidgetSetting(const QString& widgetId,
                                                const QString& key,
                                                const QVariant& value)
{
  // Skip if the value hasn't changed
  auto obj            = m_widgetSettings.value(widgetId).toObject();
  const auto newValue = QJsonValue::fromVariant(value);
  if (obj.value(key) == newValue)
    return;

  // Update in-memory store
  obj.insert(key, newValue);
  m_widgetSettings.insert(widgetId, obj);

  // Mark the project dirty and notify listeners
  setModified(true);
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Stages a plugin's state in the project and marks it dirty.
 */
void DataModel::ProjectModel::savePluginState(const QString& pluginId, const QJsonObject& state)
{
  // Skip if the state hasn't changed
  const auto key = QStringLiteral("plugin:") + pluginId;
  if (m_widgetSettings.value(key).toObject() == state)
    return;

  // Update in-memory store and mark the project dirty
  m_widgetSettings.insert(key, state);
  setModified(true);
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Adds a new source to the project (GPL: capped to one source).
 */
void DataModel::ProjectModel::addSource()
{
  // Enforce single-source limit in GPL builds
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

  // Build a new source with defaults inherited from the project
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
  source.frameParserLanguage   = static_cast<int>(SerialStudio::Lua);
  source.frameParserCode       = FrameParser::defaultTemplateCode(SerialStudio::Lua);

  // Append and notify
  m_sources.push_back(source);
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceAdded(newId);
}

/**
 * @brief Deletes the source and reassigns dependent groups to source 0.
 */
void DataModel::ProjectModel::deleteSource(int sourceId)
{
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  return;
#else
  // Cannot delete the default source (id 0)
  if (sourceId <= 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  // Remove the source entry
  m_sources.erase(m_sources.begin() + sourceId);

  // Reassign groups that referenced the deleted source
  for (auto& group : m_groups)
    if (group.sourceId == sourceId)
      group.sourceId = 0;
    else if (group.sourceId > sourceId)
      --group.sourceId;

  // Renumber remaining source IDs
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

  // Clone the source, reset connection settings for the new copy
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

  // Replace the source data, preserving the canonical ID
  m_sources[sourceId]          = source;
  m_sources[sourceId].sourceId = sourceId;

  // Source 0 drives the project-level frame detection settings
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

  // Targeted per-source notification so JsCodeEditor instances can refresh
  // without reacting to unrelated sources-changed storms
  Q_EMIT sourceFrameParserCodeChanged(sourceId);
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
 * @brief Applies the source's saved connectionSettings to its live driver.
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
 * @brief Overwrites source[0].connectionSettings without emitting sourcesChanged.
 */
void DataModel::ProjectModel::setSource0ConnectionSettings(const QJsonObject& settings)
{
  if (m_sources.empty())
    return;

  m_sources[0].connectionSettings = settings;
  setModified(true);
}

/**
 * @brief Sets source[0].busType without emitting sourceStructureChanged.
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
 * @brief Prompts to save changes, returning false only on cancel.
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

  // Ask the user whether to save, discard, or cancel
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
 * @brief Validates and saves the project, optionally prompting for a path.
 */
bool DataModel::ProjectModel::saveJsonFile(const bool askPath)
{
  // Title is required
  if (m_title.isEmpty()) {
    if (m_suppressMessageBoxes)
      qWarning() << "[ProjectModel] Project title cannot be empty";
    else
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("Project title cannot be empty!"), QMessageBox::Warning);

    return false;
  }

  // At least one group is required
  if (groupCount() <= 0) {
    if (m_suppressMessageBoxes)
      qWarning() << "[ProjectModel] Project needs at least one group";
    else
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("You need to add at least one group!"), QMessageBox::Warning);

    return false;
  }

  // At least one dataset is required (image groups are exempt)
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

  // Prompt for a save location if no file path is set
  if (jsonFilePath().isEmpty() || askPath) {
    auto* dialog = new QFileDialog(nullptr,
                                   tr("Save Serial Studio Project"),
                                   jsonProjectsPath() + "/" + title() + ".ssproj",
                                   tr("Serial Studio Project Files (*.ssproj)"));

    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);

    connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
      dialog->deleteLater();

      if (path.isEmpty())
        return;

      // Enforce .ssproj extension — append when missing
      QString finalPath = path;
      if (!finalPath.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
        finalPath += QStringLiteral(".ssproj");

      m_filePath = finalPath;
      (void)finalizeProjectSave();
    });

    dialog->open();
    return false;
  }

  // File already on disk, just write new data to it
  return finalizeProjectSave();
}

/**
 * @brief Headless save to the given path (no file dialog).
 */
bool DataModel::ProjectModel::apiSaveJsonFile(const QString& path)
{
  if (path.isEmpty())
    return false;

  // Validate required project fields
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

  // Enforce .ssproj extension — append when missing
  QString finalPath = path;
  if (!finalPath.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
    finalPath += QStringLiteral(".ssproj");

  // Write directly to the specified path
  m_filePath = finalPath;
  return finalizeProjectSave();
}

/**
 * @brief Serializes the complete project state to a QJsonObject.
 */
QJsonObject DataModel::ProjectModel::serializeToJson() const
{
  // Project metadata
  QJsonObject json;
  json.insert(Keys::Title, m_title);
  json.insert(Keys::PointCount, m_pointCount);
  json.insert(Keys::HexadecimalDelimiters, m_hexadecimalDelimiters);

  // Stamp file with current schema version + writer app version. Preserve the
  // original creator stamp if it was loaded from disk.
  const QString writer  = DataModel::current_writer_version();
  const QString creator = m_writerVersionAtCreation.isEmpty() ? writer : m_writerVersionAtCreation;
  json.insert(Keys::SchemaVersion, DataModel::kSchemaVersion);
  json.insert(Keys::WriterVersion, writer);
  json.insert(Keys::WriterVersionAtCreation, creator);

  // Groups
  QJsonArray groupArray;
  for (const auto& group : std::as_const(m_groups))
    groupArray.append(DataModel::serialize(group));

  json.insert(Keys::Groups, groupArray);

  // Actions
  QJsonArray actionsArray;
  for (const auto& action : std::as_const(m_actions))
    actionsArray.append(DataModel::serialize(action));

  json.insert(Keys::Actions, actionsArray);

  // Data sources
  QJsonArray sourcesArray;
  for (const auto& source : std::as_const(m_sources))
    sourcesArray.append(DataModel::serialize(source));

  json.insert(Keys::Sources, sourcesArray);

  // Persist the workspace list on every save. When customizeWorkspaces is
  // off the list is auto-generated, but we still write it so a project
  // opened on a different machine — or an older build — sees the same
  // layout without having to rebuild it.
  json.insert(QStringLiteral("customizeWorkspaces"), m_customizeWorkspaces);

  QJsonArray workspacesArray;
  for (const auto& ws : std::as_const(m_workspaces))
    workspacesArray.append(DataModel::serialize(ws));

  json.insert(Keys::Workspaces, workspacesArray);

  // Hidden auto-generated group IDs
  if (!m_hiddenGroupIds.isEmpty()) {
    QJsonArray hiddenArray;
    for (const int id : std::as_const(m_hiddenGroupIds))
      hiddenArray.append(id);

    json.insert(Keys::HiddenGroups, hiddenArray);
  }

  // Data tables (skip if empty for backward compat)
  if (!m_tables.empty()) {
    QJsonArray tablesArray;
    for (const auto& table : std::as_const(m_tables))
      tablesArray.append(DataModel::serialize(table));

    json.insert(Keys::Tables, tablesArray);
  }

  if (!m_widgetSettings.isEmpty())
    json.insert(Keys::WidgetSettings, m_widgetSettings);

  return json;
}

//--------------------------------------------------------------------------------------------------
// Signal/slot setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires Dashboard, ConnectionManager, and AppState signals to this model.
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

  // In QuickPlot mode the session workspace list is driven by the dashboard
  // frame, not m_groups — rebuild it whenever Dashboard's widget count
  // changes. ConsoleOnly has no dashboard, so the list stays empty there.
  connect(&UI::Dashboard::instance(), &UI::Dashboard::widgetCountChanged, this, [this] {
    if (AppState::instance().operationMode() != SerialStudio::QuickPlot)
      return;

    m_sessionWorkspaces = buildAutoWorkspaces();
    Q_EMIT activeWorkspacesChanged();
  });

  // Mode switch: rebuild the session list for the new mode and republish so
  // the taskbar drops the previous mode's workspaces immediately (without
  // waiting for a frame or a groupsChanged).
  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    const auto opMode = AppState::instance().operationMode();
    if (opMode == SerialStudio::ProjectFile)
      m_sessionWorkspaces.clear();
    else
      m_sessionWorkspaces = buildAutoWorkspaces();

    Q_EMIT activeWorkspacesChanged();
  });

  // Clear ephemeral workspace data on disconnect in non-project modes
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
 * @brief Resets all project state to factory defaults.
 */
void DataModel::ProjectModel::newJsonFile()
{
  // Clear all project data
  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();
  m_autoSnapshot.clear();
  m_tables.clear();
  m_customizeWorkspaces = false;

  // Reset to factory defaults
  m_frameEndSequence        = "\\n";
  m_checksumAlgorithm       = "";
  m_frameStartSequence      = "$";
  m_writerVersionAtCreation = "";
  m_hexadecimalDelimiters   = false;
  m_title                   = tr("Untitled Project");
  m_pointCount              = 100;
  m_frameDecoder            = SerialStudio::PlainText;
  m_frameDetection          = SerialStudio::EndDelimiterOnly;
  m_widgetSettings          = QJsonObject();

  // Create the default data source (source 0)
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
  defaultSource.frameParserLanguage   = static_cast<int>(SerialStudio::Lua);
  defaultSource.frameParserCode       = FrameParser::defaultTemplateCode(SerialStudio::Lua);
  m_sources.push_back(defaultSource);

  // Clear file path and notify all listeners
  m_filePath = "";

  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT tablesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  setModified(false);
}

/**
 * @brief Updates the project title and emits titleChanged.
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

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      openJsonFile(path);

    dialog->deleteLater();
  });

  dialog->open();
}

/**
 * @brief Loads a project from the given .ssproj/.json path.
 */
bool DataModel::ProjectModel::openJsonFile(const QString& path)
{
  // Validate path and skip redundant reloads
  if (path.isEmpty())
    return false;

  if (m_filePath == path && !m_groups.empty())
    return true;

  // Read and validate the JSON file
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

  return loadFromJsonDocument(document, path);
}

/**
 * @brief Deserialises a project from an in-memory QJsonDocument.
 */
bool DataModel::ProjectModel::loadFromJsonDocument(const QJsonDocument& document,
                                                   const QString& sourcePath)
{
  if (document.isEmpty())
    return false;

  // Clear internal data without emitting intermediate signals
  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();
  m_widgetSettings = QJsonObject();

  // Record the source path (empty = in-memory load, no file association)
  m_filePath = sourcePath;

  auto json                      = document.object();
  m_title                        = json.value(Keys::Title).toString();
  m_frameEndSequence             = json.value(Keys::FrameEnd).toString();
  m_frameStartSequence           = json.value(Keys::FrameStart).toString();
  const QString legacyParserCode = json.value(QLatin1StringView("frameParser")).toString();
  m_hexadecimalDelimiters        = json.value(Keys::HexadecimalDelimiters).toBool();
  m_frameDetection =
    static_cast<SerialStudio::FrameDetection>(json.value(Keys::FrameDetection).toInt());

  // Prefer canonical "checksumAlgorithm" / "decoderMethod" keys, fall back to
  // legacy "checksum" / "decoder" written by older Serial Studio versions
  if (json.contains(Keys::ChecksumAlgorithm))
    m_checksumAlgorithm = json.value(Keys::ChecksumAlgorithm).toString();
  else
    m_checksumAlgorithm = json.value(Keys::Checksum).toString();

  if (json.contains(Keys::DecoderMethod))
    m_frameDecoder =
      static_cast<SerialStudio::DecoderMethod>(json.value(Keys::DecoderMethod).toInt());
  else
    m_frameDecoder = static_cast<SerialStudio::DecoderMethod>(json.value(Keys::Decoder).toInt());

  // Capture creator stamp so we round-trip it on save without overwriting
  m_writerVersionAtCreation = json.value(Keys::WriterVersionAtCreation).toString();

  if (!json.contains(Keys::FrameDetection))
    m_frameDetection = SerialStudio::StartAndEndDelimiter;

  // Deserialize groups
  auto groups = json.value(Keys::Groups).toArray();
  for (int g = 0; g < groups.count(); ++g) {
    DataModel::Group group;
    group.groupId = g;
    if (DataModel::read(group, groups.at(g).toObject()))
      m_groups.push_back(group);
  }

  // Deserialize actions
  auto actions = json.value(Keys::Actions).toArray();
  for (int a = 0; a < actions.count(); ++a) {
    DataModel::Action action;
    action.actionId = a;
    if (DataModel::read(action, actions.at(a).toObject()))
      m_actions.push_back(action);
  }

  // Deserialize data sources
  m_sources.clear();
  if (json.contains(Keys::Sources)) {
    auto sourcesArr = json.value(Keys::Sources).toArray();
    for (int s = 0; s < sourcesArr.count(); ++s) {
      DataModel::Source source;
      if (DataModel::read(source, sourcesArr.at(s).toObject()))
        m_sources.push_back(source);
    }
  }

  // Create a default source if none were loaded (legacy or empty file)
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

    // Snapshot UI driver settings so the migrated source matches the setup panel
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

  // Truncate to single source in GPL builds
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

  // Resolve unset transformLanguage (-1) so FrameBuilder hotpath never sees it
  for (auto& group : m_groups) {
    for (auto& dataset : group.datasets) {
      if (dataset.transformLanguage >= 0 || dataset.transformCode.isEmpty())
        continue;

      int resolved = 0;
      for (const auto& src : m_sources)
        if (src.sourceId == dataset.sourceId) {
          resolved = src.frameParserLanguage;
          break;
        }

      dataset.transformLanguage = resolved;
    }
  }

  // Load widget settings
  m_widgetSettings = json.value(Keys::WidgetSettings).toObject();

  // Workspaces. When the project opted in to customising workspaces, load
  // the saved list verbatim; otherwise the list will be regenerated below
  // once all groups have been parsed.
  m_workspaces.clear();
  m_customizeWorkspaces = json.value(QStringLiteral("customizeWorkspaces")).toBool(false);

  // Migrate pre-v3.3 projects: if a workspaces array is present but the
  // customize flag wasn't persisted, auto-promote so the saved list isn't lost
  if (!m_customizeWorkspaces && json.contains(Keys::Workspaces)
      && !json.value(Keys::Workspaces).toArray().isEmpty())
    m_customizeWorkspaces = true;

  if (m_customizeWorkspaces && json.contains(Keys::Workspaces)) {
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

  // Deserialize data tables (missing key = empty, backward compatible)
  m_tables.clear();
  if (json.contains(Keys::Tables)) {
    const auto tablesArray = json.value(Keys::Tables).toArray();
    for (const auto& val : tablesArray) {
      DataModel::TableDef table;
      if (DataModel::read(table, val.toObject()))
        m_tables.push_back(table);
    }
  }

  // Read point count from root level or legacy widgetSettings key
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

  // Migrate legacy layout keys to canonical "layout:N" format
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

  // Migrate legacy dashboardLayout/activeGroupId into widgetSettings
  if (json.contains(QStringLiteral("dashboardLayout"))) {
    const int legacy_group_id = json.value(QStringLiteral("activeGroupId")).toInt(-1);
    const auto layout         = json.value(QStringLiteral("dashboardLayout")).toObject();
    if (legacy_group_id >= 0 && !layout.isEmpty())
      m_widgetSettings.insert(Keys::layoutKey(legacy_group_id), layout);

    if (legacy_group_id >= 0)
      m_widgetSettings.insert(Keys::kActiveGroupSubKey, legacy_group_id);
  }

  // m_workspaces is regenerated by the groupsChanged handler emitted below,
  // so nothing to seed explicitly here when customizeWorkspaces is off.

  setModified(false);

  // Migrate legacy "separator" field into the frame parser function
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

      // Persist the migration only when we came from a real file — an
      // in-memory load has no destination and saveJsonFile(false) would
      // prompt the user for a path, which is wrong on the replay path.
      if (!m_filePath.isEmpty())
        (void)saveJsonFile(false);

      return true;
    }
  }

  m_autoSnapshot = buildAutoWorkspaces();

  // Notify all listeners that the project has been fully loaded
  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT tablesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  if (m_widgetSettings.contains(Keys::kActiveGroupSubKey))
    Q_EMIT activeGroupIdChanged();

  if (!m_widgetSettings.isEmpty())
    Q_EMIT widgetSettingsChanged();

  // Auto-save migration from legacy single-source format — skip for
  // in-memory loads (empty m_filePath) since they have no file to rewrite.
  if (legacyFormat && !m_filePath.isEmpty()) {
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
 * @brief Stores the editor's currently selected group.
 */
void DataModel::ProjectModel::setSelectedGroup(const DataModel::Group& group)
{
  m_selectedGroup = group;
}

/**
 * @brief Stores the editor's currently selected action.
 */
void DataModel::ProjectModel::setSelectedAction(const DataModel::Action& action)
{
  m_selectedAction = action;
}

/**
 * @brief Stores the editor's currently selected dataset.
 */
void DataModel::ProjectModel::setSelectedDataset(const DataModel::Dataset& dataset)
{
  m_selectedDataset = dataset;
}

/**
 * @brief Sets the frame decoder method and emits frameDetectionChanged.
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
 * @brief Toggles hexadecimal delimiter mode.
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
 * @brief Replaces the group at groupId and emits groupsChanged.
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

  // Resolve any unset transform language (-1) to the source's parser language
  DataModel::Dataset resolved = dataset;
  if (resolved.transformLanguage < 0 && !resolved.transformCode.isEmpty()) {
    for (const auto& src : m_sources)
      if (src.sourceId == resolved.sourceId) {
        resolved.transformLanguage = src.frameParserLanguage;
        break;
      }

    if (resolved.transformLanguage < 0)
      resolved.transformLanguage = 0;
  }

  m_groups[groupId].datasets[datasetId] = resolved;
  m_selectedDataset                     = resolved;

  if (rebuildTree)
    Q_EMIT groupsChanged();

  setModified(true);
}

/**
 * @brief Replaces the action at actionId and emits actionsChanged.
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
  // Confirm deletion with the user
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

  // Validate and erase the group
  const auto gid = m_selectedGroup.groupId;
  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  // Count widgets-of-each-type the deleted group contributed BEFORE erasing
  // so we can decrement relativeIndex on any ref that came after this group.
  QMap<int, int> deletedTypeCounts;
  if (m_customizeWorkspaces)
    deletedTypeCounts = widgetTypeCountsForGroup(m_groups[gid]);

  m_groups.erase(m_groups.begin() + gid);

  // Renumber group and dataset IDs to stay contiguous
  int id = 0;
  for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
    g->groupId = id;
    for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
      d->groupId = id;
  }

  // If the user has customised workspaces, rewrite widgetRefs so both groupId
  // and relativeIndex match the post-delete layout — relativeIndex is a
  // per-widget-type running counter in project order and shifts for every
  // widget that came after the deleted group.
  if (m_customizeWorkspaces)
    shiftWorkspaceRefsAfterGroupDelete(gid, deletedTypeCounts);

  Q_EMIT groupsChanged();
  Q_EMIT groupDeleted();
  setModified(true);
}

/**
 * @brief Deletes the currently selected action after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentAction()
{
  // Confirm deletion with the user
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

  // Validate and erase the action
  const auto aid = m_selectedAction.actionId;
  if (aid < 0 || static_cast<size_t>(aid) >= m_actions.size())
    return;

  m_actions.erase(m_actions.begin() + aid);

  // Renumber remaining action IDs
  int id = 0;
  for (auto a = m_actions.begin(); a != m_actions.end(); ++a, ++id)
    a->actionId = id;

  Q_EMIT actionsChanged();
  Q_EMIT actionDeleted();
  setModified(true);
}

/**
 * @brief Deletes the selected dataset, removing the group if it becomes empty.
 */
void DataModel::ProjectModel::deleteCurrentDataset()
{
  // Confirm deletion with the user
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

  // Validate group and dataset indices
  const auto groupId   = m_selectedDataset.groupId;
  const auto datasetId = m_selectedDataset.datasetId;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  // Snapshot the group's full widget-type counts BEFORE we touch anything so
  // the post-erase ref rebuild has accurate "count removed" data if the group
  // ends up being dropped entirely below.
  QMap<int, int> deletedTypeCounts;
  if (m_customizeWorkspaces)
    deletedTypeCounts = widgetTypeCountsForGroup(m_groups[groupId]);

  // Snapshot the deleted dataset's own widget-type contribution — if the
  // group survives, workspace refs pointing at the same widget type in later
  // slots need their relativeIndex shifted by exactly this much.
  QMap<int, int> datasetTypeCounts;
  if (m_customizeWorkspaces) {
    const auto& ds  = m_groups[groupId].datasets[datasetId];
    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys)
      if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
        datasetTypeCounts[static_cast<int>(k)] += 1;
  }

  // Erase the dataset
  m_groups[groupId].datasets.erase(m_groups[groupId].datasets.begin() + datasetId);

  // If the group is now empty, remove it and renumber all groups
  if (m_groups[groupId].datasets.empty()) {
    m_groups.erase(m_groups.begin() + groupId);

    int id = 0;
    for (auto g = m_groups.begin(); g != m_groups.end(); ++g, ++id) {
      g->groupId = id;
      for (auto d = g->datasets.begin(); d != g->datasets.end(); ++d)
        d->groupId = id;
    }

    // Group became empty and was removed — shift refs using the pre-delete
    // per-type widget counts we captured above.
    if (m_customizeWorkspaces)
      shiftWorkspaceRefsAfterGroupDelete(groupId, deletedTypeCounts);

    Q_EMIT groupsChanged();
    Q_EMIT datasetDeleted(-1);
    setModified(true);
    return;
  }

  // Renumber remaining dataset IDs within the group
  int id     = 0;
  auto begin = m_groups[groupId].datasets.begin();
  auto end   = m_groups[groupId].datasets.end();
  for (auto dataset = begin; dataset != end; ++dataset, ++id)
    dataset->datasetId = id;

  // Group survives — shift workspace refs of the same widget type whose
  // running counter lands at or above the removed dataset's slot.
  if (m_customizeWorkspaces)
    shiftWorkspaceRefsAfterDatasetDelete(groupId, datasetTypeCounts);

  Q_EMIT groupsChanged();
  Q_EMIT datasetDeleted(groupId);
  setModified(true);
}

/**
 * @brief Appends a copy of the currently selected group to the project.
 */
void DataModel::ProjectModel::duplicateCurrentGroup()
{
  // Build the cloned group with reassigned dataset indices
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
 * @brief Stores the editor's currently selected output widget.
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
    qBound(0, type, static_cast<int>(DataModel::OutputWidgetType::Knob)));

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
 * @brief Adds an output control, creating a new output group if needed.
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

  // Validate indices
  const auto gid = m_selectedOutputWidget.groupId;
  const auto wid = m_selectedOutputWidget.widgetId;

  if (gid < 0 || static_cast<size_t>(gid) >= m_groups.size())
    return;

  auto& widgets = m_groups[gid].outputWidgets;
  if (wid < 0 || static_cast<size_t>(wid) >= widgets.size())
    return;

  // Erase the widget
  widgets.erase(widgets.begin() + wid);

  // If group is now empty, remove the group and renumber
  if (widgets.empty()) {
    m_groups.erase(m_groups.begin() + gid);

    for (int i = 0; i < static_cast<int>(m_groups.size()); ++i)
      m_groups[i].groupId = i;

    Q_EMIT groupsChanged();
    Q_EMIT groupDeleted();
    setModified(true);
    return;
  }

  // Renumber remaining widget IDs
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
  // A compatible group accepts user-added datasets
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

  // Use the current selection if it's already compatible
  const auto selId      = m_selectedGroup.groupId;
  const bool selInRange = selId >= 0 && static_cast<size_t>(selId) < m_groups.size();

  if (selInRange && isValidGroup(m_groups[selId])) {
    m_selectedGroup = m_groups[selId];
    return;
  }

  // Fall back to the first compatible group
  for (const auto& group : std::as_const(m_groups)) {
    if (!isValidGroup(group))
      continue;

    m_selectedGroup = group;
    return;
  }

  // No compatible group exists, create one
  addGroup(tr("Group"), SerialStudio::NoGroupWidget);
  m_selectedGroup = m_groups.back();
}

/**
 * @brief Adds a new dataset of the given type to the selected group.
 */
void DataModel::ProjectModel::addDataset(const SerialStudio::DatasetOption option)
{
  ensureValidGroup();

  // Initialize dataset with type-specific defaults
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

  // Generate a unique title within the group
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

  // Assign the title and the next available frame index
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
 */
void DataModel::ProjectModel::changeDatasetOption(const SerialStudio::DatasetOption option,
                                                  const bool checked)
{
  // Update the option flag on the cached dataset
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

  // Write the updated dataset back into the group
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
  // Generate a unique title
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
 * @brief Adds a new group with a unique title and the given widget type.
 */
void DataModel::ProjectModel::addGroup(const QString& title, const SerialStudio::GroupWidget widget)
{
  // Generate a unique title
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

  // Create the group and assign its widget type
  DataModel::Group group;
  group.title   = newTitle;
  group.groupId = m_groups.size();

  m_groups.push_back(group);
  setGroupWidget(static_cast<int>(m_groups.size()) - 1, widget);

  Q_EMIT groupAdded(static_cast<int>(m_groups.size()) - 1);
  setModified(true);
}

/**
 * @brief Assigns a widget type to the group, replacing fixed-layout datasets.
 */
bool DataModel::ProjectModel::setGroupWidget(const int group,
                                             const SerialStudio::GroupWidget widget)
{
  if (group < 0 || group >= static_cast<int>(m_groups.size())) [[unlikely]]
    return false;

  auto& grp          = m_groups[group];
  const auto groupId = grp.groupId;

  // Handle existing datasets: compatible types keep them, others require confirmation
  if (!grp.datasets.empty()) {
    if ((widget == SerialStudio::DataGrid || widget == SerialStudio::MultiPlot
         || widget == SerialStudio::NoGroupWidget)
        && (grp.widget == "multiplot" || grp.widget == "datagrid" || grp.widget == "")) {
      grp.widget = "";
    } else {
      auto ret =
        Misc::Utilities::showMessageBox(tr("Are you sure you want to change the group-level "
                                           "widget?"),
                                        tr("Existing datasets for this group are deleted"),
                                        QMessageBox::Question,
                                        APP_NAME,
                                        QMessageBox::Yes | QMessageBox::No);

      if (ret == QMessageBox::No)
        return false;

      grp.datasets.clear();
    }
  }

  // Assign the widget string and populate canonical datasets for fixed-layout types
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
 * @brief Updates the project's modification flag and emits modifiedChanged.
 */
void DataModel::ProjectModel::setModified(const bool modified)
{
  // Keep a truly empty project clean
  if (modified && m_groups.empty() && m_actions.empty() && m_tables.empty() && m_workspaces.empty())
    return;

  m_modified = modified;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Sets source[0].frameParserCode and emits frameParserCodeChanged.
 */
void DataModel::ProjectModel::setFrameParserCode(const QString& code)
{
  if (m_sources.empty() || code == m_sources[0].frameParserCode)
    return;

  m_sources[0].frameParserCode = code;
  setModified(true);
  Q_EMIT frameParserCodeChanged();
  Q_EMIT sourceFrameParserCodeChanged(0);
}

/**
 * @brief Sets the scripting language for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserLanguage(int language)
{
  if (m_sources.empty() || language == m_sources[0].frameParserLanguage)
    return;

  m_sources[0].frameParserLanguage = language;
  setModified(true);
  Q_EMIT frameParserLanguageChanged();
  Q_EMIT sourceFrameParserLanguageChanged(0);
}

/**
 * @brief Sets the scripting language for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserLanguage(int sourceId, int language)
{
  // Find the source by logical ID
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  // Unknown source → nothing to do
  if (it == m_sources.end())
    return;

  // Skip if the language is already set to the requested value
  if (it->frameParserLanguage == language)
    return;

  // Flip the language and notify
  it->frameParserLanguage = language;
  setModified(true);

  // Source 0 is also exposed via the legacy Q_PROPERTY; emit for QML bindings
  if (sourceId == 0)
    Q_EMIT frameParserLanguageChanged();

  // Per-source targeted signal so JsCodeEditor instances can filter by their
  // own sourceId instead of reacting to every unrelated sourcesChanged()
  Q_EMIT sourceFrameParserLanguageChanged(sourceId);
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
 * @brief Stages the active dashboard tab group ID.
 *
 * Stored in m_widgetSettings so it survives a save/load cycle and marks
 * the project as modified. No disk write happens here — the standard
 * save workflow is responsible for flushing. No-op outside ProjectFile
 * mode so QuickPlot/ConsoleOnly can't pollute the loaded project.
 */
void DataModel::ProjectModel::setActiveGroupId(const int groupId)
{
  // Only persist tab selection for the active project file
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  // Skip if unchanged
  const int current = m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
  if (current == groupId)
    return;

  // Update in-memory store
  if (groupId >= 0)
    m_widgetSettings.insert(Keys::kActiveGroupSubKey, groupId);
  else
    m_widgetSettings.remove(Keys::kActiveGroupSubKey);

  // Mark the project dirty and notify listeners
  setModified(true);
  Q_EMIT activeGroupIdChanged();
}

/**
 * @brief Stages the widget layout for a specific group.
 *
 * Stores the layout under the canonical key produced by Keys::layoutKey()
 * and marks the project as modified. No disk write happens here — the
 * standard save workflow is responsible for flushing. No-op outside
 * ProjectFile mode so QuickPlot/ConsoleOnly can't pollute the loaded project.
 *
 * @param groupId  The group whose layout is being saved.
 * @param layout   The layout QJsonObject to store.
 */
void DataModel::ProjectModel::setGroupLayout(const int groupId, const QJsonObject& layout)
{
  // Only persist layouts for the active project file
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  // Update in-memory store
  QJsonObject entry;
  entry[QStringLiteral("data")] = layout;
  m_widgetSettings.insert(Keys::layoutKey(groupId), entry);

  // Mark the project dirty and notify listeners
  setModified(true);
  Q_EMIT widgetSettingsChanged();
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
int DataModel::ProjectModel::addWorkspace(const QString& title)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return -1;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  int maxId = 999;
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId > maxId)
      maxId = ws.workspaceId;

  DataModel::Workspace ws;
  ws.workspaceId = maxId + 1;
  ws.title       = title.simplified().isEmpty() ? tr("Workspace") : title.simplified();
  m_workspaces.push_back(ws);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return ws.workspaceId;
}

/**
 * @brief Deletes the workspace with the given ID.
 */
void DataModel::ProjectModel::deleteWorkspace(int workspaceId)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(), [workspaceId](const auto& ws) {
    return ws.workspaceId == workspaceId;
  });

  if (it == m_workspaces.end())
    return;

  m_workspaces.erase(it);
  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Renames the workspace with the given ID.
 */
void DataModel::ProjectModel::renameWorkspace(int workspaceId, const QString& title)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId == workspaceId) {
      ws.title = title.simplified();
      setModified(true);
      Q_EMIT editorWorkspacesChanged();
      Q_EMIT activeWorkspacesChanged();
      return;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Data-table CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a new empty data table with a unique name derived from @p name.
 *
 * @return The actual name used after uniquifying (callers use this to select
 *         the newly created table in the editor tree).
 */
QString DataModel::ProjectModel::addTable(const QString& name)
{
  // Derive a unique table name so we never collide with an existing one
  QString base = name.simplified();
  if (base.isEmpty())
    base = tr("Shared Table");

  QString unique     = base;
  int suffix         = 2;
  const auto hasName = [this](const QString& n) {
    for (const auto& t : m_tables)
      if (t.name == n)
        return true;

    return false;
  };

  while (hasName(unique))
    unique = QStringLiteral("%1 %2").arg(base).arg(suffix++);

  // Append and notify
  DataModel::TableDef table;
  table.name = unique;
  m_tables.push_back(table);
  setModified(true);
  Q_EMIT tablesChanged();
  return unique;
}

/**
 * @brief Deletes the table with the given @p name.
 */
void DataModel::ProjectModel::deleteTable(const QString& name)
{
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&name](const auto& t) { return t.name == name; });

  if (it == m_tables.end())
    return;

  m_tables.erase(it);
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Renames a table (no-op if target name already exists).
 */
void DataModel::ProjectModel::renameTable(const QString& oldName, const QString& newName)
{
  const QString n = newName.simplified();
  if (n.isEmpty())
    return;

  // Reject rename if another table already owns the target name
  for (const auto& t : m_tables)
    if (t.name == n && t.name != oldName)
      return;

  for (auto& t : m_tables) {
    if (t.name == oldName) {
      t.name = n;
      setModified(true);
      Q_EMIT tablesChanged();
      return;
    }
  }
}

/**
 * @brief Appends a register to @p table with a unique name.
 */
void DataModel::ProjectModel::addRegister(const QString& table,
                                          const QString& registerName,
                                          bool computed,
                                          const QVariant& defaultValue)
{
  // Locate the target table
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return;

  // Derive a unique register name within this table
  QString base = registerName.simplified();
  if (base.isEmpty())
    base = tr("register");

  QString unique     = base;
  int suffix         = 2;
  const auto hasName = [it](const QString& n) {
    for (const auto& r : it->registers)
      if (r.name == n)
        return true;

    return false;
  };

  while (hasName(unique))
    unique = QStringLiteral("%1_%2").arg(base).arg(suffix++);

  // Build the register def and append
  DataModel::RegisterDef reg;
  reg.name         = unique;
  reg.type         = computed ? RegisterType::Computed : RegisterType::Constant;
  reg.defaultValue = defaultValue.isValid() ? defaultValue : QVariant(0.0);
  it->registers.push_back(reg);

  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Removes a register from the specified table.
 */
void DataModel::ProjectModel::deleteRegister(const QString& table, const QString& registerName)
{
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return;

  auto rit = std::find_if(it->registers.begin(),
                          it->registers.end(),
                          [&registerName](const auto& r) { return r.name == registerName; });

  if (rit == it->registers.end())
    return;

  it->registers.erase(rit);
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Updates an existing register — rename, retype, and/or default value.
 */
void DataModel::ProjectModel::updateRegister(const QString& table,
                                             const QString& registerName,
                                             const QString& newName,
                                             bool computed,
                                             const QVariant& defaultValue)
{
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return;

  // Reject rename collisions
  const QString n = newName.simplified();
  if (n.isEmpty())
    return;

  if (n != registerName) {
    for (const auto& r : it->registers)
      if (r.name == n)
        return;
  }

  // Apply the update in place
  for (auto& r : it->registers) {
    if (r.name == registerName) {
      r.name         = n;
      r.type         = computed ? RegisterType::Computed : RegisterType::Constant;
      r.defaultValue = defaultValue.isValid() ? defaultValue : r.defaultValue;
      setModified(true);
      Q_EMIT tablesChanged();
      return;
    }
  }
}

/**
 * @brief Returns the register list of a table as a QVariantList for QML.
 *
 * Each entry: { name, type ("constant"|"computed"), value, valueType ("number"|"string") }
 */
QVariantList DataModel::ProjectModel::registersForTable(const QString& table) const
{
  QVariantList result;
  auto it = std::find_if(
    m_tables.begin(), m_tables.end(), [&table](const auto& t) { return t.name == table; });

  if (it == m_tables.end())
    return result;

  for (const auto& r : it->registers) {
    QVariantMap row;
    row["name"] = r.name;
    row["type"] =
      r.type == RegisterType::Computed ? QStringLiteral("computed") : QStringLiteral("constant");
    row["value"]     = r.defaultValue;
    row["valueType"] = r.defaultValue.typeId() == QMetaType::Double ? QStringLiteral("number")
                                                                    : QStringLiteral("string");
    result.append(row);
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// QInputDialog wrappers — native prompts invoked from QML
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a new shared-memory table name and appends it on accept.
 */
void DataModel::ProjectModel::promptAddTable()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Shared Table"), tr("Name:"), QLineEdit::Normal, tr("Shared Table"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  // Create the table, then defer selection so the editor's tree rebuild
  // (driven by tablesChanged over a queued connection) has finished first.
  const QString added = addTable(name.trimmed());
  QTimer::singleShot(
    0, this, [added] { DataModel::ProjectEditor::instance().selectUserTable(added); });
}

/**
 * @brief Prompts for a new name for an existing table.
 */
void DataModel::ProjectModel::promptRenameTable(const QString& oldName)
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Table"), tr("Name:"), QLineEdit::Normal, oldName, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == oldName)
    return;

  renameTable(oldName, name.trimmed());
}

/**
 * @brief Prompts for a register name and type, then appends the register with a
 *        zero default (numeric). Users can edit the value inline afterwards.
 */
void DataModel::ProjectModel::promptAddRegister(const QString& table)
{
  if (table.isEmpty())
    return;

  // Ask for the register name
  bool okName           = false;
  const QString regName = QInputDialog::getText(nullptr,
                                                tr("New Register"),
                                                tr("Name:"),
                                                QLineEdit::Normal,
                                                QStringLiteral("register"),
                                                &okName);

  if (!okName || regName.trimmed().isEmpty())
    return;

  // New registers default to Read/Write (transforms can update them). Users
  // can switch a register to Read-Only from the Permissions column.
  addRegister(table, regName.trimmed(), true, QVariant(0.0));
}

/**
 * @brief Prompts for a new name for an existing register.
 */
void DataModel::ProjectModel::promptRenameRegister(const QString& table,
                                                   const QString& registerName)
{
  if (table.isEmpty() || registerName.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Register"), tr("Name:"), QLineEdit::Normal, registerName, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == registerName)
    return;

  // Look up the register to preserve its type and default value
  for (const auto& t : m_tables) {
    if (t.name != table)
      continue;

    for (const auto& r : t.registers) {
      if (r.name == registerName) {
        updateRegister(
          table, registerName, name.trimmed(), r.type == RegisterType::Computed, r.defaultValue);
        return;
      }
    }
  }
}

/**
 * @brief Asks the user to confirm before deleting a table.
 */
void DataModel::ProjectModel::confirmDeleteTable(const QString& name)
{
  if (name.isEmpty())
    return;

  // Count registers so the informative text can mention them
  int registerCount = 0;
  for (const auto& t : m_tables) {
    if (t.name == name) {
      registerCount = static_cast<int>(t.registers.size());
      break;
    }
  }

  const QString informative =
    registerCount == 0
      ? tr("This action cannot be undone.")
      : tr("This removes %1 register(s) along with the table. This action cannot be undone.")
          .arg(registerCount);

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(name),
                                                     informative,
                                                     QMessageBox::Warning,
                                                     tr("Delete Table"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteTable(name);
}

/**
 * @brief Asks the user to confirm before deleting a register.
 */
void DataModel::ProjectModel::confirmDeleteRegister(const QString& table,
                                                    const QString& registerName)
{
  if (table.isEmpty() || registerName.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(registerName),
                                                     tr("This action cannot be undone."),
                                                     QMessageBox::Warning,
                                                     tr("Delete Register"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteRegister(table, registerName);
}

/**
 * @brief Exports a table's registers to a CSV file chosen by the user.
 *
 * Format: name,type,value — one row per register, no header row.
 */
void DataModel::ProjectModel::exportTableToCsv(const QString& tableName)
{
  // Find the table
  const auto it = std::find_if(m_tables.begin(), m_tables.end(), [&](const DataModel::TableDef& t) {
    return t.name == tableName;
  });

  if (it == m_tables.end())
    return;

  // Ask for output path
  const auto path = QFileDialog::getSaveFileName(
    nullptr,
    tr("Export Table"),
    QStringLiteral("%1/%2.csv").arg(Misc::WorkspaceManager::instance().path("CSV"), tableName),
    tr("CSV files (*.csv)"));

  if (path.isEmpty())
    return;

  // Write CSV
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);
  out << "name,type,value\n";
  for (const auto& reg : it->registers) {
    const auto type = (reg.type == DataModel::RegisterType::Computed) ? QStringLiteral("computed")
                                                                      : QStringLiteral("constant");

    // CSV-escape the value
    auto val = reg.defaultValue.toString();
    if (val.contains(',') || val.contains('"') || val.contains('\n'))
      val = QStringLiteral("\"%1\"").arg(val.replace('"', "\"\""));

    out << reg.name << ',' << type << ',' << val << '\n';
  }

  file.close();
}

/**
 * @brief Imports registers from a CSV file into an existing table.
 *
 * Expects format: name,type,value (with a header row). Creates registers
 * that don't exist; updates the type and value of registers that do.
 */
void DataModel::ProjectModel::importTableFromCsv(const QString& tableName)
{
  // Find the table
  auto it = std::find_if(m_tables.begin(), m_tables.end(), [&](const DataModel::TableDef& t) {
    return t.name == tableName;
  });

  if (it == m_tables.end())
    return;

  // Ask for input file
  const auto path = QFileDialog::getOpenFileName(nullptr,
                                                 tr("Import Table"),
                                                 Misc::WorkspaceManager::instance().path("CSV"),
                                                 tr("CSV files (*.csv)"));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream in(&file);

  // Skip header row
  if (!in.atEnd())
    in.readLine();

  // Hard cap on imported rows to bound this loop per NASA PoT rule 2
  constexpr int kMaxImportRows = 1'000'000;

  int imported = 0;
  int rowsRead = 0;
  while (!in.atEnd() && rowsRead < kMaxImportRows) {
    ++rowsRead;
    const auto line = in.readLine().trimmed();
    if (line.isEmpty())
      continue;

    // Parse CSV row: name,type,value
    const auto parts = line.split(',');
    if (parts.size() < 3)
      continue;

    // Re-join every field past the second comma so commas embedded in a
    // quoted value survive. Only strip the surrounding quote pair added by
    // exportTableToCsv(); preserve any quotes inside the value.
    const auto name    = parts[0].trimmed();
    const auto typeStr = parts[1].trimmed().toLower();
    auto valStr        = parts.mid(2).join(',').trimmed();
    if (valStr.size() >= 2 && valStr.startsWith('"') && valStr.endsWith('"')) {
      valStr = valStr.mid(1, valStr.size() - 2);
      valStr.replace(QStringLiteral("\"\""), QStringLiteral("\""));
    }

    if (name.isEmpty())
      continue;

    const bool computed = (typeStr == QStringLiteral("computed"));

    // Determine if the value is numeric
    bool isNumeric              = false;
    const double dval           = valStr.toDouble(&isNumeric);
    const QVariant defaultValue = isNumeric ? QVariant(dval) : QVariant(valStr);

    // Check if the register already exists
    auto regIt = std::find_if(it->registers.begin(),
                              it->registers.end(),
                              [&](const DataModel::RegisterDef& r) { return r.name == name; });

    if (regIt != it->registers.end()) {
      // Update existing register
      regIt->type =
        computed ? DataModel::RegisterType::Computed : DataModel::RegisterType::Constant;
      regIt->defaultValue = defaultValue;
    } else {
      // Create new register
      DataModel::RegisterDef reg;
      reg.name = name;
      reg.type = computed ? DataModel::RegisterType::Computed : DataModel::RegisterType::Constant;
      reg.defaultValue = defaultValue;
      it->registers.push_back(reg);
    }

    ++imported;
  }

  file.close();

  if (imported > 0) {
    setModified(true);
    Q_EMIT tablesChanged();
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
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

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
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference from the specified workspace by index.
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId, int index)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    if (index < 0 || static_cast<size_t>(index) >= ws.widgetRefs.size())
      return;

    ws.widgetRefs.erase(ws.widgetRefs.begin() + index);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference matching (widgetType, groupId, relativeIndex).
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId,
                                                        int widgetType,
                                                        int groupId,
                                                        int relativeIndex)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    auto it = std::find_if(ws.widgetRefs.begin(), ws.widgetRefs.end(), [=](const auto& r) {
      return r.widgetType == widgetType && r.groupId == groupId && r.relativeIndex == relativeIndex;
    });

    if (it == ws.widgetRefs.end())
      return;

    ws.widgetRefs.erase(it);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Returns the title of a workspace, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceTitle(int workspaceId) const
{
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId == workspaceId)
      return ws.title;

  return QString();
}

/**
 * @brief Prompts for a new workspace name and creates it. The new workspace
 *        is then selected in the project editor.
 */
void DataModel::ProjectModel::promptAddWorkspace()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Workspace"), tr("Name:"), QLineEdit::Normal, tr("Workspace"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addWorkspace(name.trimmed());
  QTimer::singleShot(
    0, this, [newId] { DataModel::ProjectEditor::instance().selectWorkspace(newId); });
}

/**
 * @brief Prompts for a new title for the given workspace.
 */
void DataModel::ProjectModel::promptRenameWorkspace(int workspaceId)
{
  const QString current = workspaceTitle(workspaceId);
  if (current.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Workspace"), tr("Name:"), QLineEdit::Normal, current, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == current)
    return;

  renameWorkspace(workspaceId, name.trimmed());
}

/**
 * @brief Returns whether the user has opted in to customising workspaces.
 */
bool DataModel::ProjectModel::customizeWorkspaces() const noexcept
{
  return m_customizeWorkspaces;
}

/**
 * @brief Flips the customize switch.
 *
 * Off → On: m_workspaces is already the auto layout; simply stop
 *   regenerating it so the user's subsequent edits stick.
 *
 * On → Off: discard the hand-edited list and re-seed from the current
 *   project structure.
 */
void DataModel::ProjectModel::setCustomizeWorkspaces(const bool enabled)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_customizeWorkspaces == enabled)
    return;

  m_customizeWorkspaces = enabled;

  if (!enabled)
    regenerateAutoWorkspaces();

  setModified(true);
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Synthesises the default workspace layout for a project.
 *
 * Walks every non-image / non-output group once, computing per-widget-type
 * running relativeIndex values that match Dashboard::buildWidgetGroups()'s
 * runtime assignment. Produces: Overview (one group widget per group), All
 * Data (every widget flattened), then one workspace per group. Mirrors the
 * legacy Taskbar auto-workspace behaviour so existing projects "just work"
 * without manual workspace configuration.
 */
std::vector<DataModel::Workspace> DataModel::ProjectModel::buildAutoWorkspaces() const
{
  std::vector<DataModel::Workspace> result;

  // QuickPlot groups live in FrameBuilder's quick-plot frame; ProjectFile
  // groups live in m_groups
  const auto mode    = AppState::instance().operationMode();
  const auto& groups = (mode == SerialStudio::QuickPlot)
                       ? DataModel::FrameBuilder::instance().quickPlotFrame().groups
                       : m_groups;

  // Running per-type indices, mirroring Dashboard::buildWidgetGroups()
  QMap<SerialStudio::DashboardWidget, int> groupIdx;
  QMap<SerialStudio::DashboardWidget, int> datasetIdx;

  // Dashboard remaps Plot3D → MultiPlot on non-Pro builds (see
  // UI::Dashboard::buildWidgetGroups). Mirror that here so workspace refs
  // resolve to the widget Dashboard actually instantiates.
  const bool pro = SerialStudio::proWidgetsEnabled();

  // Refs collected across the walk. Reused to populate all three
  // workspace categories without re-walking the project.
  std::vector<DataModel::WidgetRef> allRefs;
  std::vector<DataModel::WidgetRef> overviewRefs;
  QMap<int, std::vector<DataModel::WidgetRef>> perGroupRefs;

  int eligibleGroups = 0;

  for (const auto& group : groups) {
    if (!SerialStudio::groupEligibleForWorkspace(group))
      continue;

    std::vector<DataModel::WidgetRef> groupRefs;

    // Group-level widget. Plot3D falls back to MultiPlot on non-Pro so the
    // workspace ref matches the widget Dashboard actually registers.
    auto groupKey = SerialStudio::getDashboardWidget(group);
    if (groupKey == SerialStudio::DashboardPlot3D && !pro)
      groupKey = SerialStudio::DashboardMultiPlot;

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey)) {
      DataModel::WidgetRef r;
      r.widgetType       = static_cast<int>(groupKey);
      r.groupId          = group.groupId;
      r.relativeIndex    = groupIdx.value(groupKey, 0);
      groupIdx[groupKey] = r.relativeIndex + 1;

      groupRefs.push_back(r);
      allRefs.push_back(r);
      overviewRefs.push_back(r);
    }

    // Dataset widgets
    for (const auto& ds : group.datasets) {
      const auto keys = SerialStudio::getDashboardWidgets(ds);
      for (const auto& k : keys) {
        if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
          continue;

        DataModel::WidgetRef r;
        r.widgetType    = static_cast<int>(k);
        r.groupId       = group.groupId;
        r.relativeIndex = datasetIdx.value(k, 0);
        datasetIdx[k]   = r.relativeIndex + 1;

        groupRefs.push_back(r);
        allRefs.push_back(r);
      }
    }

    if (groupRefs.empty())
      continue;

    perGroupRefs.insert(group.groupId, std::move(groupRefs));
    ++eligibleGroups;
  }

  if (eligibleGroups == 0)
    return result;

  // Fixed workspace ID slots — stable across group add/remove so a persisted
  // m_selectedWorkspaceId can't silently rebind to the wrong workspace.
  static constexpr int kOverviewId      = 1000;
  static constexpr int kAllDataId       = 1001;
  static constexpr int kPerGroupIdStart = 1002;

  // "Overview" — one group-level widget per group. Always first so users
  // land on the project overview by default.
  if (overviewRefs.size() >= 2) {
    DataModel::Workspace ws;
    ws.workspaceId = kOverviewId;
    ws.title       = tr("Overview");
    ws.icon        = QStringLiteral("qrc:/rcc/icons/panes/overview.svg");
    ws.widgetRefs  = overviewRefs;
    result.push_back(std::move(ws));
  }

  // "All Data" — every widget across every group.
  if (eligibleGroups >= 2) {
    DataModel::Workspace ws;
    ws.workspaceId = kAllDataId;
    ws.title       = tr("All Data");
    ws.icon        = QStringLiteral("qrc:/rcc/icons/panes/dashboard.svg");
    ws.widgetRefs  = allRefs;
    result.push_back(std::move(ws));
  }

  // One workspace per group, in project order. groupId is always contiguous
  // from 0, so kPerGroupIdStart + groupId cannot collide with the reserved
  // Overview/All Data slots.
  for (const auto& group : groups) {
    const auto it = perGroupRefs.constFind(group.groupId);
    if (it == perGroupRefs.constEnd())
      continue;

    DataModel::Workspace ws;
    ws.workspaceId = kPerGroupIdStart + group.groupId;
    ws.title       = group.title;
    ws.widgetRefs  = it.value();
    result.push_back(std::move(ws));
  }

  return result;
}

/**
 * @brief Refreshes m_workspaces from the current project structure.
 *
 * Called on project load and on every groupsChanged signal while
 * customizeWorkspaces is off. Does not touch m_customizeWorkspaces and does
 * not call setModified() — auto-regeneration is not a user-visible edit.
 * Caller is responsible for emitting editor/activeWorkspacesChanged() when appropriate.
 */
void DataModel::ProjectModel::regenerateAutoWorkspaces()
{
  if (m_customizeWorkspaces)
    return;

  m_workspaces   = buildAutoWorkspaces();
  m_autoSnapshot = m_workspaces;
}

/**
 * @brief Merges newly-eligible auto refs into the user-customised workspace list.
 *
 * In customize mode the user owns m_workspaces, so we cannot wholesale
 * regenerate it. Instead we diff the current auto layout against the last
 * snapshot — refs and per-group workspaces that appear for the first time are
 * merged in (preserving user-removed refs from prior generations), while
 * existing per-group workspaces inherit the latest group title. Returns true
 * when m_workspaces was modified.
 */
bool DataModel::ProjectModel::mergeAutoWorkspaceUpdates()
{
  if (!m_customizeWorkspaces)
    return false;

  const auto current = buildAutoWorkspaces();
  bool dirty         = false;

  const auto refsEqual = [](const WidgetRef& a, const WidgetRef& b) {
    return a.widgetType == b.widgetType && a.groupId == b.groupId
        && a.relativeIndex == b.relativeIndex;
  };

  const auto findById = [](std::vector<DataModel::Workspace>& list, int id) {
    return std::find_if(
      list.begin(), list.end(), [id](const auto& w) { return w.workspaceId == id; });
  };

  const auto findByIdConst = [](const std::vector<DataModel::Workspace>& list, int id) {
    return std::find_if(
      list.begin(), list.end(), [id](const auto& w) { return w.workspaceId == id; });
  };

  for (const auto& cur : current) {
    auto userIt = findById(m_workspaces, cur.workspaceId);
    auto snapIt = findByIdConst(m_autoSnapshot, cur.workspaceId);

    if (userIt == m_workspaces.end()) {
      m_workspaces.push_back(cur);
      dirty = true;
      continue;
    }

    if (cur.workspaceId >= 1002 && userIt->title != cur.title) {
      userIt->title = cur.title;
      dirty         = true;
    }

    for (const auto& r : cur.widgetRefs) {
      const bool inSnap = snapIt != m_autoSnapshot.end()
                       && std::any_of(snapIt->widgetRefs.begin(),
                                      snapIt->widgetRefs.end(),
                                      [&](const auto& s) { return refsEqual(s, r); });
      if (inSnap)
        continue;

      const bool inUser = std::any_of(userIt->widgetRefs.begin(),
                                      userIt->widgetRefs.end(),
                                      [&](const auto& s) { return refsEqual(s, r); });
      if (inUser)
        continue;

      userIt->widgetRefs.push_back(r);
      dirty = true;
    }
  }

  m_autoSnapshot = current;

  if (dirty)
    setModified(true);

  return dirty;
}

/**
 * @brief Materialises the synthetic workspace list into m_workspaces and flips
 *        the project into customize mode so the user can edit it from there.
 *
 * @return id of the first workspace created, or -1 if the project has no
 *         eligible widgets.
 */
int DataModel::ProjectModel::autoGenerateWorkspaces()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return -1;

  // Refuse to clobber a hand-curated workspace list — callers must first
  // toggle customizeWorkspaces off if they really want to regenerate.
  if (m_customizeWorkspaces && !m_workspaces.empty())
    return m_workspaces.front().workspaceId;

  // Bail out if nothing eligible — don't flip customize mode on an empty list
  auto seed = buildAutoWorkspaces();
  if (seed.empty())
    return -1;

  // Promote into customize mode so subsequent groupsChanged signals don't
  // overwrite the freshly materialised list.
  m_workspaces           = std::move(seed);
  m_autoSnapshot         = m_workspaces;
  const bool flagChanged = !m_customizeWorkspaces;
  m_customizeWorkspaces  = true;

  // Post-condition checks: buildAutoWorkspaces returned a non-empty vector so
  // the move must have produced a non-empty m_workspaces, and ids start at 1000
  Q_ASSERT(!m_workspaces.empty());
  Q_ASSERT(m_workspaces.front().workspaceId >= 1000);

  setModified(true);
  if (flagChanged)
    Q_EMIT customizeWorkspacesChanged();

  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return m_workspaces.front().workspaceId;
}

/**
 * @brief Counts, per dashboard-widget type, how many widgets the given group
 *        contributes to Dashboard::buildWidgetGroups's running type counter.
 */
QMap<int, int> DataModel::ProjectModel::widgetTypeCountsForGroup(const Group& g) const
{
  QMap<int, int> counts;

  // Skip groups filtered out by Dashboard
  if (!SerialStudio::groupEligibleForWorkspace(g))
    return counts;

  // Group-level widget (one per group at most). Mirror buildAutoWorkspaces's
  // non-Pro Plot3D → MultiPlot remap so shift math stays consistent with
  // what the refs were actually assigned.
  auto groupKey = SerialStudio::getDashboardWidget(g);
  if (groupKey == SerialStudio::DashboardPlot3D && !SerialStudio::proWidgetsEnabled())
    groupKey = SerialStudio::DashboardMultiPlot;

  if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey))
    counts[static_cast<int>(groupKey)] += 1;

  // Dataset-level widgets — filter identical to buildAutoWorkspaces
  for (const auto& ds : g.datasets) {
    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys) {
      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      counts[static_cast<int>(k)] += 1;
    }
  }

  return counts;
}

/**
 * @brief Shifts or drops user-customised widget refs after a group delete.
 *
 * Refs with groupId == deletedGid are dropped. Refs with groupId > deletedGid
 * have their groupId decremented and their relativeIndex reduced by the number
 * of same-type widgets the deleted group contributed.
 */
void DataModel::ProjectModel::shiftWorkspaceRefsAfterGroupDelete(
  int deletedGid, const QMap<int, int>& deletedTypeCounts)
{
  Q_ASSERT(deletedGid >= 0);
  Q_ASSERT(m_customizeWorkspaces);

  for (auto& ws : m_workspaces) {
    // Drop refs pointing at the deleted group
    ws.widgetRefs.erase(
      std::remove_if(ws.widgetRefs.begin(),
                     ws.widgetRefs.end(),
                     [deletedGid](const WidgetRef& r) { return r.groupId == deletedGid; }),
      ws.widgetRefs.end());

    // Shift surviving refs
    for (auto& r : ws.widgetRefs) {
      if (r.groupId <= deletedGid)
        continue;

      r.groupId -= 1;
      const int lost  = deletedTypeCounts.value(r.widgetType, 0);
      r.relativeIndex = std::max(0, r.relativeIndex - lost);

      // Post-condition: relativeIndex must remain non-negative
      Q_ASSERT(r.relativeIndex >= 0);
    }
  }
}

/**
 * @brief Shifts user-customised widget refs after a single dataset is deleted
 *        from a surviving group.
 *
 * The running per-widget-type counter used by Dashboard walks groups in order,
 * then datasets inside each group. A dataset removed from groupId "steals" N
 * positions from the counter for each widget type the dataset contributed —
 * refs whose relativeIndex is at or above that position need to shift down.
 * Refs pointing at the removed dataset itself (same groupId, relativeIndex in
 * the stolen range) are dropped.
 */
void DataModel::ProjectModel::shiftWorkspaceRefsAfterDatasetDelete(
  int groupId, const QMap<int, int>& datasetTypeCounts)
{
  Q_ASSERT(groupId >= 0);
  Q_ASSERT(m_customizeWorkspaces);

  if (datasetTypeCounts.isEmpty())
    return;

  // Compute, for each widget type, the running counter value at the start of
  // the deleted group's dataset block. Everything >= this counter is "later"
  // in project order.
  QMap<int, int> runningAtGroup;
  for (const auto& g : m_groups) {
    if (!SerialStudio::groupEligibleForWorkspace(g))
      continue;

    if (g.groupId == groupId)
      break;

    const auto groupKey = SerialStudio::getDashboardWidget(g);
    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey))
      runningAtGroup[static_cast<int>(groupKey)] += 1;

    for (const auto& ds : g.datasets) {
      const auto keys = SerialStudio::getDashboardWidgets(ds);
      for (const auto& k : keys)
        if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
          runningAtGroup[static_cast<int>(k)] += 1;
    }
  }

  for (auto& ws : m_workspaces) {
    // Drop refs that point into the removed dataset's slice
    ws.widgetRefs.erase(std::remove_if(ws.widgetRefs.begin(),
                                       ws.widgetRefs.end(),
                                       [&](const WidgetRef& r) {
                                         const int lost = datasetTypeCounts.value(r.widgetType, 0);
                                         if (lost == 0 || r.groupId != groupId)
                                           return false;

                                         const int base = runningAtGroup.value(r.widgetType, 0);
                                         return r.relativeIndex >= base
                                             && r.relativeIndex < base + lost;
                                       }),
                        ws.widgetRefs.end());

    // Shift surviving refs whose relativeIndex is past the removed slice
    for (auto& r : ws.widgetRefs) {
      const int lost = datasetTypeCounts.value(r.widgetType, 0);
      if (lost == 0)
        continue;

      const int base = runningAtGroup.value(r.widgetType, 0);
      if (r.relativeIndex >= base + lost) {
        r.relativeIndex -= lost;
        Q_ASSERT(r.relativeIndex >= 0);
      }
    }
  }
}

/**
 * @brief Asks the user to confirm before deleting a workspace.
 */
void DataModel::ProjectModel::confirmDeleteWorkspace(int workspaceId)
{
  const QString name = workspaceTitle(workspaceId);
  if (name.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(name),
                                                     tr("This action cannot be undone."),
                                                     QMessageBox::Warning,
                                                     tr("Delete Workspace"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteWorkspace(workspaceId);
}

/**
 * @brief Hides an auto-generated group workspace from the workspace list.
 *
 * Does not affect the Overview workspace (id < 0). The hidden state is
 * persisted in the project file.
 */
void DataModel::ProjectModel::hideGroup(int groupId)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (groupId < 0)
    return;

  if (m_hiddenGroupIds.contains(groupId))
    return;

  m_hiddenGroupIds.insert(groupId);
  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Restores a previously hidden auto-generated group workspace.
 */
void DataModel::ProjectModel::showGroup(int groupId)
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_hiddenGroupIds.remove(groupId))
    return;

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears workspaces and widget settings for QuickPlot/ConsoleOnly
 *        sessions that have no backing project file.
 *
 * Skips when a file is loaded so a project's state survives across mode
 * switches and connection cycles — even when the user runs QuickPlot on
 * top of an open project.
 */
void DataModel::ProjectModel::clearTransientState()
{
  // Preserve project-owned state whenever a file is loaded
  const auto opMode = AppState::instance().operationMode();
  if (opMode == SerialStudio::ProjectFile || !m_filePath.isEmpty())
    return;

  // Discard ephemeral workspace and widget state
  m_hiddenGroupIds.clear();

  if (!m_workspaces.empty()) {
    m_workspaces.clear();
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
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
    // Serialize child datasets
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

    // Serialize output widgets
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

    // Assemble the group entry
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
  // Open the file for writing
  QFile file(m_filePath);
  if (!file.open(QFile::WriteOnly)) {
    if (m_suppressMessageBoxes)
      qWarning() << "[ProjectModel] File open error:" << file.errorString();
    else
      Misc::Utilities::showMessageBox(
        tr("File open error"), file.errorString(), QMessageBox::Critical);

    return false;
  }

  // Serialize and write
  const QJsonObject json = serializeToJson();
  file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
  file.close();

  // Switch to project-file mode and notify listeners
  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  setModified(false);
  Q_EMIT jsonFileChanged();
  Q_EMIT sourceStructureChanged();
  return true;
}
