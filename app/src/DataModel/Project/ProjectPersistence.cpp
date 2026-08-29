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

#include "DataModel/Project/ProjectPersistence.h"

#include <algorithm>
#include <QApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSaveFile>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/Project/ProjectLoader.h"
#include "DataModel/Project/ProjectPresentation.h"
#include "DataModel/Project/ProjectTables.h"
#include "DataModel/Project/ProjectWorkspaces.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"
#include "SSAssert.h"

namespace DataModel {

/**
 * @brief Serializes any folder vector to a JSON array.
 */
template<typename Folder>
static QJsonArray serializeFolders(const std::vector<Folder>& folders)
{
  QJsonArray arr;
  for (const auto& f : std::as_const(folders))
    arr.append(DataModel::serialize(f));

  return arr;
}

}  // namespace DataModel

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the autosave timer and the on-disk watcher for @p model. Runs inside ProjectModel's
 *        ctor closure, so it touches nothing but its own children.
 */
DataModel::ProjectPersistence::ProjectPersistence(ProjectModel& model)
  : m_model(model)
  , m_autoSaveTimer(new QTimer(this))
  , m_autoSaveSuspended(false)
  , m_runtimeDirty(false)
  , m_fileWatcher(new QFileSystemWatcher(this))
  , m_diskCheckPending(false)
  , m_diskPromptActive(false)
{
  m_autoSaveTimer->setSingleShot(true);
  m_autoSaveTimer->setInterval(1500);
  connect(m_autoSaveTimer, &QTimer::timeout, this, &ProjectPersistence::autoSave);

  connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [this] {
    if (m_diskCheckPending)
      return;

    m_diskCheckPending = true;
    QTimer::singleShot(500, this, &ProjectPersistence::resolveDiskFileChange);
  });
}

//--------------------------------------------------------------------------------------------------
// Document serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the complete project state to a QJsonObject.
 */
QJsonObject DataModel::ProjectPersistence::serializeToJson() const
{
  QJsonObject json;
  json.insert(Keys::Title, m_model.m_title);
  json.insert(Keys::PointCount, m_model.m_pointCount);
  json.insert(Keys::PlotTimeRange, m_model.m_plotTimeRange);
  json.insert(Keys::Frozen, m_model.m_frozen);
  json.insert(Keys::ChangeDrivenTransforms, m_model.m_changeDrivenTransforms);
  json.insert(Keys::LuaFastMode, m_model.m_luaFastMode);
  json.insert(Keys::HexadecimalDelimiters, m_model.m_hexadecimalDelimiters);

  const QString writer = DataModel::current_writer_version();
  const QString creator =
    m_model.m_writerVersionAtCreation.isEmpty() ? writer : m_model.m_writerVersionAtCreation;
  json.insert(Keys::SchemaVersion, DataModel::kSchemaVersion);
  json.insert(Keys::WriterVersion, writer);
  json.insert(Keys::WriterVersionAtCreation, creator);
  json.insert(Keys::NextUniqueId, m_model.m_nextUniqueId);

  if (!m_model.m_passwordHash.isEmpty())
    json.insert(Keys::PasswordHash, m_model.m_passwordHash);

  if (!m_model.m_controlScriptCode.isEmpty())
    json.insert(Keys::ControlScriptCode, m_model.m_controlScriptCode);

  QJsonArray groupArray;
  for (const auto& group : std::as_const(m_model.m_groups))
    groupArray.append(DataModel::serialize(group));

  json.insert(Keys::Groups, groupArray);

  const auto& groupFolders = m_model.m_folders.groupFolders();
  if (!groupFolders.empty())
    json.insert(Keys::GroupFolders, serializeFolders(groupFolders));

  QJsonArray actionsArray;
  for (const auto& action : std::as_const(m_model.m_actions))
    actionsArray.append(DataModel::serialize(action));

  json.insert(Keys::Actions, actionsArray);

  QJsonArray sourcesArray;
  for (const auto& source : std::as_const(m_model.m_sources))
    sourcesArray.append(DataModel::serialize(source));

  json.insert(Keys::Sources, sourcesArray);

  if (m_model.m_workspaces.customizeWorkspaces()) {
    json.insert(Keys::CustomizeWorkspaces, true);

    QJsonArray workspacesArray;
    for (const auto& ws : std::as_const(m_model.m_workspaces.list()))
      workspacesArray.append(DataModel::serialize(ws));

    json.insert(Keys::Workspaces, workspacesArray);

    QJsonArray foldersArray;
    for (const auto& folder : std::as_const(m_model.m_folders.workspaceFolders()))
      foldersArray.append(DataModel::serialize(folder));

    if (!foldersArray.isEmpty())
      json.insert(Keys::WorkspaceFolders, foldersArray);
  }

  const auto& hiddenGroupIds = m_model.m_workspaces.hiddenGroupIds();
  if (!hiddenGroupIds.isEmpty()) {
    QJsonArray hiddenArray;
    for (const int id : std::as_const(hiddenGroupIds))
      hiddenArray.append(id);

    json.insert(Keys::HiddenGroups, hiddenArray);
  }

  const auto& tables = m_model.m_tables.list();
  if (!tables.empty()) {
    QJsonArray tablesArray;
    for (const auto& table : std::as_const(tables))
      tablesArray.append(DataModel::serialize(table));

    json.insert(Keys::Tables, tablesArray);
  }

  const auto& tableFolders = m_model.m_folders.tableFolders();
  if (!tableFolders.empty())
    json.insert(Keys::TableFolders, serializeFolders(tableFolders));

  const auto& presentation = m_model.m_presentation;
  if (!presentation.widgetSettingsBlob().isEmpty())
    json.insert(Keys::WidgetSettings, presentation.widgetSettingsBlob());

  if (!presentation.widgetDisplayBlob().isEmpty())
    json.insert(Keys::WidgetDisplay, presentation.widgetDisplayBlob());

  if (!presentation.treeExpansion().isEmpty())
    json.insert(Keys::TreeExpansion, presentation.treeExpansion());

  if (!presentation.diagramCollapse().isEmpty())
    json.insert(Keys::DiagramCollapse, presentation.diagramCollapse());

  if (!m_model.m_mqttPublisher.isEmpty())
    json.insert(Keys::MqttPublisher, m_model.m_mqttPublisher);

  if (!m_model.m_influxSink.isEmpty())
    json.insert(Keys::InfluxSink, m_model.m_influxSink);

  return json;
}

//--------------------------------------------------------------------------------------------------
// Document saving / export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts to save changes, returning false only on cancel.
 */
bool DataModel::ProjectPersistence::askSave()
{
  if (!m_model.modified())
    return true;

  static auto& appState = AppState::instance();

  const auto opMode = appState.operationMode();
  if (opMode != SerialStudio::ProjectFile && m_model.m_filePath.isEmpty())
    return true;

  if (m_model.m_suppressMessageBoxes) {
    qWarning() << "[ProjectModel] Discarding unsaved changes (API mode)";
    if (m_model.jsonFilePath().isEmpty())
      m_model.newJsonFile();
    else {
      const auto path        = m_model.m_filePath;
      m_model.m_silentReload = true;
      m_model.m_filePath.clear();
      m_model.openJsonFile(path);
      m_model.m_silentReload = false;
      if (opMode != SerialStudio::ProjectFile)
        appState.setOperationMode(opMode);
    }

    return true;
  }

  auto ret = Misc::Utilities::showMessageBox(
    ProjectModel::tr("Do you want to save your changes?"),
    ProjectModel::tr("You have unsaved modifications in this project!"),
    QMessageBox::Question,
    APP_NAME,
    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Cancel)
    return false;

  if (ret == QMessageBox::Discard) {
    if (m_model.jsonFilePath().isEmpty())
      m_model.newJsonFile();
    else {
      const auto path        = m_model.m_filePath;
      m_model.m_silentReload = true;
      m_model.m_filePath.clear();
      m_model.openJsonFile(path);
      m_model.m_silentReload = false;
      if (opMode != SerialStudio::ProjectFile)
        appState.setOperationMode(opMode);
    }

    return true;
  }

  return saveJsonFile(false);
}

/**
 * @brief Validates and saves the project, optionally prompting for a path; the
 * path-accepted handler defers via a queued invoke because the macOS NSSavePanel
 * KVO callback must unwind before re-entering the model.
 */
bool DataModel::ProjectPersistence::saveJsonFile(const bool askPath)
{
  if (!m_model.validateProject(m_model.m_suppressMessageBoxes))
    return false;

  if (m_model.jsonFilePath().isEmpty() || askPath) {
    auto* dialog = new QFileDialog(qApp->activeWindow(),
                                   ProjectModel::tr("Save Serial Studio Project"),
                                   m_model.jsonProjectsPath() + "/" + m_model.title() + ".ssproj",
                                   ProjectModel::tr("Serial Studio Project Files (*.ssproj)"));

    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    auto accepted = std::make_shared<bool>(false);
    connect(dialog, &QFileDialog::fileSelected, this, [this, accepted](const QString& path) {
      if (path.isEmpty())
        return;

      *accepted = true;

      QMetaObject::invokeMethod(
        this,
        [this, path]() {
          QString finalPath = path;
          if (!finalPath.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
            finalPath += QStringLiteral(".ssproj");

          const QString chosenTitle = QFileInfo(finalPath).completeBaseName();
          if (m_model.m_title == ProjectModel::tr("Untitled Project") && !chosenTitle.isEmpty()
              && chosenTitle != m_model.m_title) {
            m_model.m_title = chosenTitle;
            Q_EMIT m_model.titleChanged();
          }

          m_model.m_filePath = finalPath;
          (void)finalizeProjectSave();
        },
        Qt::QueuedConnection);
    });

    connect(dialog, &QFileDialog::finished, this, [this, accepted](int) {
      Q_EMIT m_model.saveDialogCompleted(*accepted);
    });

    dialog->open();
    return false;
  }

  return finalizeProjectSave();
}

/**
 * @brief Headless save to the given path (no file dialog).
 */
bool DataModel::ProjectPersistence::apiSaveJsonFile(const QString& path)
{
  if (path.isEmpty())
    return false;

  if (m_model.m_title.isEmpty()) {
    qWarning() << "[ProjectModel] Project title cannot be empty";
    return false;
  }

  if (m_model.groupCount() <= 0) {
    qWarning() << "[ProjectModel] Project needs at least one group";
    return false;
  }

  const bool hasDatasetlessGroup =
    std::any_of(m_model.m_groups.begin(), m_model.m_groups.end(), [](const Group& g) {
      return g.widget == QLatin1String("image") || g.widget == QLatin1String("painter");
    });

  if (m_model.datasetCount() <= 0 && !hasDatasetlessGroup) {
    qWarning() << "[ProjectModel] Project needs at least one dataset";
    return false;
  }

  QString finalPath = path;
  if (!finalPath.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
    finalPath += QStringLiteral(".ssproj");

  m_model.m_filePath = finalPath;
  return finalizeProjectSave();
}

/**
 * @brief Writes the current project to the model's file path and reloads it.
 */
bool DataModel::ProjectPersistence::finalizeProjectSave()
{
  m_model.m_loader.resolveDatasetTransformLanguages();
  m_model.m_loader.resolveDatasetVirtualFlags();

  if (!writeProjectFile(m_model.m_filePath)) {
    if (!m_model.m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(
        ProjectModel::tr("File save error"), m_model.m_filePath, QMessageBox::Critical);

    return false;
  }

  static auto& appState = AppState::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);
  m_model.setModified(false);
  m_model.m_history.markSaved();
  Q_EMIT m_model.jsonFileChanged();
  return true;
}

/**
 * @brief Atomically serializes the current project to @p path.
 */
bool DataModel::ProjectPersistence::writeProjectFile(const QString& path)
{
  SS_ASSERT(!path.isEmpty(), return false);

  QSaveFile file(path);
  if (!file.open(QFile::WriteOnly)) {
    qWarning() << "[ProjectModel] File open error:" << file.errorString();
    return false;
  }

  const QByteArray payload = QJsonDocument(serializeToJson()).toJson(QJsonDocument::Indented);
  if (file.write(payload) != payload.size()) {
    qWarning() << "[ProjectModel] Short write:" << file.errorString();
    file.cancelWriting();
    return false;
  }

  if (!file.commit()) {
    qWarning() << "[ProjectModel] Commit failed:" << file.errorString();
    return false;
  }

  watchProjectFile();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Debounced autosave
//--------------------------------------------------------------------------------------------------

/**
 * @brief Silently writes the current project to disk; called from the debounce timer.
 */
void DataModel::ProjectPersistence::autoSave()
{
  if (m_autoSaveSuspended)
    return;

  if (m_model.m_filePath.isEmpty() || m_model.m_locked || !m_model.m_modified)
    return;

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!writeProjectFile(m_model.m_filePath)) {
    qWarning() << "[ProjectModel] Auto-save failed";
    return;
  }

  m_model.setModified(false);
  m_model.m_history.markSaved();

  if (m_runtimeDirty)
    syncRuntime();
}

/**
 * @brief Rebuilds the live frame pipeline from the current project and clears the runtime-dirty
 *        flag. Resets the dashboard the same way the enable/disable toggle does.
 */
void DataModel::ProjectPersistence::syncRuntime()
{
  m_runtimeDirty            = false;
  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.syncFromProjectModel();
}

/**
 * @brief Flushes any pending debounced autosave synchronously (called on app quit).
 */
void DataModel::ProjectPersistence::flushAutoSave()
{
  stopAutoSaveTimer();
  autoSave();
}

/**
 * @brief Starts the debounced autosave when saving is currently permitted; the API mutation
 *        path calls this so programmatic edits persist without an explicit save.
 */
void DataModel::ProjectPersistence::scheduleAutoSave()
{
  if (m_autoSaveSuspended || m_model.m_filePath.isEmpty() || m_model.m_locked)
    return;

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  m_autoSaveTimer->start();
}

/**
 * @brief Suspends or resumes the debounced autosave (used by the API batch endpoint).
 */
void DataModel::ProjectPersistence::setAutoSaveSuspended(bool suspend)
{
  if (m_autoSaveSuspended == suspend)
    return;

  m_autoSaveSuspended = suspend;
  if (suspend)
    stopAutoSaveTimer();
}

/**
 * @brief Stops a pending debounce without running the save; the load paths use it to hold the
 *        timer down while they suspend autosave around a document swap.
 */
void DataModel::ProjectPersistence::stopAutoSaveTimer()
{
  if (m_autoSaveTimer && m_autoSaveTimer->isActive())
    m_autoSaveTimer->stop();
}

/**
 * @brief Returns true while the debounced autosave is held down.
 */
bool DataModel::ProjectPersistence::autoSaveSuspended() const noexcept
{
  return m_autoSaveSuspended;
}

/**
 * @brief Returns true when the live frame pipeline is behind the document.
 */
bool DataModel::ProjectPersistence::runtimeDirty() const noexcept
{
  return m_runtimeDirty;
}

/**
 * @brief Flags the live frame pipeline as behind (or level with) the document.
 */
void DataModel::ProjectPersistence::setRuntimeDirty(bool dirty) noexcept
{
  m_runtimeDirty = dirty;
}

//--------------------------------------------------------------------------------------------------
// On-disk change detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the SHA-256 of the file at @p path, or an empty array when unreadable.
 */
QByteArray DataModel::ProjectPersistence::hashProjectFile(const QString& path)
{
  QFile file(path);
  if (!file.open(QFile::ReadOnly))
    return {};

  return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256);
}

/**
 * @brief Re-arms the filesystem watcher on the project path and caches the on-disk content hash;
 *        called after every successful write or load so self-saves are recognized as such
 *        (QSaveFile's atomic rename drops the previous watch on some platforms).
 */
void DataModel::ProjectPersistence::watchProjectFile()
{
  const auto watched = m_fileWatcher->files();
  if (!watched.isEmpty())
    m_fileWatcher->removePaths(watched);

  m_diskFileHash.clear();
  if (m_model.m_filePath.isEmpty() || !QFile::exists(m_model.m_filePath))
    return;

  m_fileWatcher->addPath(m_model.m_filePath);
  m_diskFileHash = hashProjectFile(m_model.m_filePath);
}

/**
 * @brief Debounced watcher handler: ignores self-saves (hash unchanged), flags deletion,
 *        and prompts to reload when another program modified the project file.
 */
void DataModel::ProjectPersistence::resolveDiskFileChange()
{
  m_diskCheckPending = false;
  if (m_diskPromptActive || m_model.m_filePath.isEmpty())
    return;

  if (!QFile::exists(m_model.m_filePath)) {
    m_diskFileHash.clear();
    m_model.m_modified = true;
    Q_EMIT m_model.modifiedChanged();
    Q_EMIT m_model.projectFileChangedOnDisk();
    static auto& nc = DataModel::NotificationCenter::instance();
    nc.postWarning(QStringLiteral("ProjectModel"),
                   ProjectModel::tr("Project file removed from disk"),
                   ProjectModel::tr("%1 was deleted or renamed by another program. Save the "
                                    "project to recreate it.")
                     .arg(m_model.jsonFileName()));
    return;
  }

  if (!m_fileWatcher->files().contains(m_model.m_filePath))
    m_fileWatcher->addPath(m_model.m_filePath);

  const auto hash = hashProjectFile(m_model.m_filePath);
  if (hash.isEmpty() || hash == m_diskFileHash)
    return;

  m_diskFileHash = hash;
  Q_EMIT m_model.projectFileChangedOnDisk();

  if (m_model.m_suppressMessageBoxes) {
    qWarning() << "[ProjectModel] Project file changed on disk; keeping in-memory state";
    m_model.m_modified = true;
    Q_EMIT m_model.modifiedChanged();
    static auto& nc = DataModel::NotificationCenter::instance();
    nc.postWarning(
      QStringLiteral("ProjectModel"),
      ProjectModel::tr("Project file changed on disk"),
      ProjectModel::tr("%1 was modified by another program. The in-memory project was kept; "
                       "reopen the file to load the external changes.")
        .arg(m_model.jsonFileName()));
    return;
  }

  promptDiskFileReload();
}

/**
 * @brief Asks whether to reload the externally-modified project file; declining keeps the
 *        in-memory state and marks it modified so the divergence is saveable.
 */
void DataModel::ProjectPersistence::promptDiskFileReload()
{
  m_diskPromptActive = true;
  const auto question =
    m_model.m_modified
      ? ProjectModel::tr("The project file was modified by another program.\n\n"
                         "Reload it and discard your unsaved changes?")
      : ProjectModel::tr("The project file was modified by another program.\n\nReload it?");
  const auto ret = Misc::Utilities::showMessageBox(ProjectModel::tr("Project file changed on disk"),
                                                   question,
                                                   QMessageBox::Question,
                                                   APP_NAME,
                                                   QMessageBox::Yes | QMessageBox::No);
  m_diskPromptActive = false;

  if (ret != QMessageBox::Yes) {
    m_model.m_modified = true;
    Q_EMIT m_model.modifiedChanged();
    return;
  }

  const auto path = m_model.m_filePath;
  m_model.m_filePath.clear();
  if (!m_model.openJsonFile(path))
    qWarning() << "[ProjectModel] Reload after on-disk change failed:" << path;
}
