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
#include "DataModel/FrameBuilder.h"
#include "DataModel/FrameParser.h"
#include "IO/Checksum.h"
#include "IO/Manager.h"
#include "Misc/JsonValidator.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"

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
  , m_frameParserCode("")
  , m_frameEndSequence("")
  , m_checksumAlgorithm("")
  , m_frameStartSequence("")
  , m_hexadecimalDelimiters(false)
  , m_frameDecoder(SerialStudio::PlainText)
  , m_frameDetection(SerialStudio::EndDelimiterOnly)
  , m_modified(false)
  , m_filePath("")
  , m_suppressMessageBoxes(false)
{
  if (!DataModel::FrameBuilder::instance().jsonMapFilepath().isEmpty())
    onJsonLoaded();
  else
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
bool DataModel::ProjectModel::modified() const
{
  bool parserModified = false;
  if (DataModel::FrameBuilder::instance().frameParser())
    parserModified = DataModel::FrameBuilder::instance().frameParser()->isModified();

  return m_modified || parserModified;
}

/**
 * @brief Returns the current frame decoder method.
 */
SerialStudio::DecoderMethod DataModel::ProjectModel::decoderMethod() const
{
  return m_frameDecoder;
}

/**
 * @brief Returns the current frame detection method.
 */
SerialStudio::FrameDetection DataModel::ProjectModel::frameDetection() const
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
bool DataModel::ProjectModel::suppressMessageBoxes() const
{
  return m_suppressMessageBoxes;
}

/**
 * @brief Returns the project title.
 */
const QString& DataModel::ProjectModel::title() const
{
  return m_title;
}

/**
 * @brief Returns the file path of the current project file.
 */
const QString& DataModel::ProjectModel::jsonFilePath() const
{
  return m_filePath;
}

/**
 * @brief Returns the frame parser JavaScript source code.
 */
const QString& DataModel::ProjectModel::frameParserCode() const
{
  return m_frameParserCode;
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
  const auto opMode = DataModel::FrameBuilder::instance().operationMode();
  if (opMode != SerialStudio::ProjectFile || m_filePath.isEmpty())
    return;

  const bool ioConnected = IO::Manager::instance().isConnected();
#ifdef BUILD_COMMERCIAL
  const bool mqttSubscribed =
    MQTT::Client::instance().isConnected() && MQTT::Client::instance().isSubscriber();
#else
  const bool mqttSubscribed = false;
#endif
  if (!ioConnected && !mqttSubscribed)
    return;

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
 * Returns @c true if the project uses features that require a commercial
 * license (e.g. the 3D plot widget).
 */
bool DataModel::ProjectModel::containsCommercialFeatures() const
{
  return SerialStudio::commercialCfg(m_groups);
}

/**
 * @brief Returns the total number of groups in the project.
 */
int DataModel::ProjectModel::groupCount() const
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
 * @brief Returns a const reference to the vector of groups.
 */
const std::vector<DataModel::Group>& DataModel::ProjectModel::groups() const
{
  return m_groups;
}

/**
 * @brief Returns a const reference to the vector of actions.
 */
const std::vector<DataModel::Action>& DataModel::ProjectModel::actions() const
{
  return m_actions;
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
  if (!modified())
    return true;

  if (m_suppressMessageBoxes) {
    qWarning() << "[ProjectModel] Discarding unsaved changes (API mode)";
    if (jsonFilePath().isEmpty())
      newJsonFile();
    else
      openJsonFile(jsonFilePath());

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
    else
      openJsonFile(jsonFilePath());

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

  auto* parser = DataModel::FrameBuilder::instance().frameParser();
  if (parser && parser->isModified()) {
    if (!parser->save(true))
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
 * @brief Serializes the complete project state to a QJsonObject.
 *
 * Used for both file saving and API export.
 *
 * @return QJsonObject containing the full project configuration.
 */
QJsonObject DataModel::ProjectModel::serializeToJson() const
{
  QJsonObject json;

  json.insert("title", m_title);
  json.insert("decoder", m_frameDecoder);
  json.insert("frameEnd", m_frameEndSequence);
  json.insert("frameParser", m_frameParserCode);
  json.insert("checksum", m_checksumAlgorithm);
  json.insert("frameDetection", m_frameDetection);
  json.insert("frameStart", m_frameStartSequence);
  json.insert("hexadecimalDelimiters", m_hexadecimalDelimiters);

  QJsonArray groupArray;
  for (const auto& group : std::as_const(m_groups))
    groupArray.append(DataModel::serialize(group));

  json.insert("groups", groupArray);

  QJsonArray actionsArray;
  for (const auto& action : std::as_const(m_actions))
    actionsArray.append(DataModel::serialize(action));

  json.insert("actions", actionsArray);

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
 * Connects the FrameBuilder's jsonFileMapChanged signal so that the project is
 * reloaded when the underlying JSON map file changes. Language-change rebuilds
 * are handled by ProjectEditor.
 */
void DataModel::ProjectModel::setupExternalConnections()
{
  connect(&DataModel::FrameBuilder::instance(),
          &DataModel::FrameBuilder::jsonFileMapChanged,
          this,
          &DataModel::ProjectModel::onJsonLoaded);
}

//--------------------------------------------------------------------------------------------------
// Document initialisation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resets all project state to factory defaults and emits change signals.
 */
void DataModel::ProjectModel::newJsonFile()
{
  m_groups.clear();
  m_actions.clear();

  m_frameEndSequence      = "\\n";
  m_checksumAlgorithm     = "";
  m_frameStartSequence    = "$";
  m_hexadecimalDelimiters = false;
  m_title                 = tr("Untitled Project");
  m_frameDecoder          = SerialStudio::PlainText;
  m_frameDetection        = SerialStudio::EndDelimiterOnly;
  m_frameParserCode       = FrameParser::defaultTemplateCode();
  m_widgetSettings        = QJsonObject();

  auto* parser = DataModel::FrameBuilder::instance().frameParser();
  if (parser) {
    parser->readCode();
    parser->loadDefaultTemplate(false);
  }

  m_filePath = "";

  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();

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
 */
void DataModel::ProjectModel::setFrameStartSequence(const QString& sequence)
{
  if (m_frameStartSequence != sequence) {
    m_frameStartSequence = sequence;
    Q_EMIT frameDetectionChanged();
    setModified(true);
  }
}

/**
 * @brief Sets the frame end delimiter sequence.
 */
void DataModel::ProjectModel::setFrameEndSequence(const QString& sequence)
{
  if (m_frameEndSequence != sequence) {
    m_frameEndSequence = sequence;
    Q_EMIT frameDetectionChanged();
    setModified(true);
  }
}

/**
 * @brief Sets the checksum algorithm name.
 */
void DataModel::ProjectModel::setChecksumAlgorithm(const QString& algorithm)
{
  if (m_checksumAlgorithm != algorithm) {
    m_checksumAlgorithm = algorithm;
    Q_EMIT frameDetectionChanged();
    setModified(true);
  }
}

/**
 * @brief Sets the frame detection strategy.
 */
void DataModel::ProjectModel::setFrameDetection(const SerialStudio::FrameDetection detection)
{
  if (m_frameDetection != detection) {
    m_frameDetection = detection;
    setModified(true);
    Q_EMIT frameDetectionChanged();
  }
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
 */
void DataModel::ProjectModel::openJsonFile(const QString& path)
{
  if (path.isEmpty())
    return;

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

      return;
    }

    document = result.document;
    file.close();
  }

  if (document.isEmpty())
    return;

  newJsonFile();

  m_filePath = path;

  auto json               = document.object();
  m_title                 = json.value("title").toString();
  m_frameEndSequence      = json.value("frameEnd").toString();
  m_checksumAlgorithm     = json.value("checksum").toString();
  m_frameParserCode       = json.value("frameParser").toString();
  m_frameStartSequence    = json.value("frameStart").toString();
  m_hexadecimalDelimiters = json.value("hexadecimalDelimiters").toBool();
  m_frameDecoder          = static_cast<SerialStudio::DecoderMethod>(json.value("decoder").toInt());
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

  m_widgetSettings = json.value(Keys::WidgetSettings).toObject();

  for (const auto& key : m_widgetSettings.keys()) {
    if (!key.startsWith(QStringLiteral("__layout__:")))
      continue;

    auto entry = m_widgetSettings.value(key).toObject();
    if (!entry.contains(QStringLiteral("data")))
      continue;

    QJsonObject cleaned;
    cleaned[QStringLiteral("data")] = entry[QStringLiteral("data")];
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

    if (legacyRegex.match(m_frameParserCode).hasMatch()) {
      if (separator.length() > 1)
        m_frameParserCode =
          QStringLiteral("/**\n * Automatically migrated frame parser function.\n"
                         " */\nfunction parse(frame) {\n    return frame.split(\"%1\");\n}")
            .arg(separator);
      else
        m_frameParserCode =
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

      saveJsonFile(false);
      return;
    }
  }

  if (DataModel::FrameBuilder::instance().jsonMapFilepath() != path)
    DataModel::FrameBuilder::instance().loadJsonMap(path, !m_suppressMessageBoxes);

  Q_EMIT groupsChanged();
  Q_EMIT actionsChanged();
  Q_EMIT titleChanged();
  Q_EMIT jsonFileChanged();
  Q_EMIT frameDetectionChanged();
  Q_EMIT frameParserCodeChanged();

  if (m_widgetSettings.contains(Keys::kActiveGroupSubKey))
    Q_EMIT activeGroupIdChanged();

  if (!m_widgetSettings.isEmpty())
    Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Ensures Serial Studio is in Project File mode.
 *
 * Prompts the user (or silently switches in API mode) when the current
 * operation mode is not ProjectFile.
 */
void DataModel::ProjectModel::enableProjectMode()
{
  const auto opMode = DataModel::FrameBuilder::instance().operationMode();
  if (opMode == SerialStudio::ProjectFile)
    return;

  if (!m_suppressMessageBoxes) {
    auto answ =
      Misc::Utilities::showMessageBox(tr("Switch Serial Studio to Project Mode?"),
                                      tr("This operation mode is required to load and display "
                                         "dashboards from project files."),
                                      QMessageBox::Question,
                                      qApp->applicationDisplayName(),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::Yes);

    if (answ == QMessageBox::Yes)
      DataModel::FrameBuilder::instance().setOperationMode(SerialStudio::ProjectFile);
  } else {
    qWarning() << "[ProjectModel] Automatically switching to ProjectFile mode";
    DataModel::FrameBuilder::instance().setOperationMode(SerialStudio::ProjectFile);
  }
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
 */
void DataModel::ProjectModel::setDecoderMethod(const SerialStudio::DecoderMethod method)
{
  if (m_frameDecoder == method)
    return;

  m_frameDecoder = method;
  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Toggles hexadecimal delimiter mode and converts existing sequences.
 */
void DataModel::ProjectModel::setHexadecimalDelimiters(const bool hexadecimal)
{
  if (m_hexadecimalDelimiters == hexadecimal)
    return;

  m_hexadecimalDelimiters = hexadecimal;
  Q_EMIT frameDetectionChanged();
  setModified(true);
}

/**
 * @brief Replaces the group at @p groupId with @p group and emits groupsChanged.
 */
void DataModel::ProjectModel::updateGroup(const int groupId, const DataModel::Group& group)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  m_groups[groupId] = group;
  Q_EMIT groupsChanged();
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
void DataModel::ProjectModel::updateAction(const int actionId, const DataModel::Action& action)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  m_actions[actionId] = action;
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
  action.eolSequence          = m_selectedAction.eolSequence;
  action.timerIntervalMs      = m_selectedAction.timerIntervalMs;
  action.title                = tr("%1 (Copy)").arg(m_selectedAction.title);
  action.autoExecuteOnConnect = m_selectedAction.autoExecuteOnConnect;

  m_actions.push_back(action);

  Q_EMIT actionsChanged();
  Q_EMIT actionAdded(static_cast<int>(m_actions.size()) - 1);
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
  const auto isValidGroup = [](const QString& widgetId) -> bool {
    switch (SerialStudio::groupWidgetFromId(widgetId)) {
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

  if (selInRange && isValidGroup(m_groups[selId].widget)) {
    m_selectedGroup = m_groups[selId];
    return;
  }

  for (const auto& group : std::as_const(m_groups)) {
    if (!isValidGroup(group.widget))
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

#ifdef BUILD_COMMERCIAL
  else if (widget == SerialStudio::ImageView)
    grp.widget = "image";
#endif

  m_groups[group] = grp;

  Q_EMIT groupsChanged();
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
  m_modified = modified;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Sets the frame parser JavaScript source and marks the project modified.
 */
void DataModel::ProjectModel::setFrameParserCode(const QString& code)
{
  if (code != m_frameParserCode) {
    m_frameParserCode = code;
    setModified(true);
    Q_EMIT frameParserCodeChanged();
  }
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
// Internal slot
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reloads the project from the path reported by FrameBuilder.
 */
void DataModel::ProjectModel::onJsonLoaded()
{
  openJsonFile(DataModel::FrameBuilder::instance().jsonMapFilepath());
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

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
 * @brief Writes the current project to m_filePath and reloads it.
 *
 * Calls enableProjectMode() and openJsonFile() after a successful write.
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

  enableProjectMode();

  openJsonFile(file.fileName());
  DataModel::FrameBuilder::instance().loadJsonMap(file.fileName(), !m_suppressMessageBoxes);
  return true;
}
