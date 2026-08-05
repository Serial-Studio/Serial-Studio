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

#include <algorithm>
#include <QApplication>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "ProjectModelShared.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

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
// Document saving / export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts to save changes, returning false only on cancel.
 */
bool DataModel::ProjectModel::askSave()
{
  if (!modified())
    return true;

  static auto& appState = AppState::instance();

  const auto opMode = appState.operationMode();
  if (opMode != SerialStudio::ProjectFile && m_filePath.isEmpty())
    return true;

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
      if (opMode != SerialStudio::ProjectFile)
        appState.setOperationMode(opMode);
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
bool DataModel::ProjectModel::saveJsonFile(const bool askPath)
{
  if (!validateProject(m_suppressMessageBoxes))
    return false;

  if (jsonFilePath().isEmpty() || askPath) {
    auto* dialog = new QFileDialog(qApp->activeWindow(),
                                   tr("Save Serial Studio Project"),
                                   jsonProjectsPath() + "/" + title() + ".ssproj",
                                   tr("Serial Studio Project Files (*.ssproj)"));

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
          if (m_title == tr("Untitled Project") && !chosenTitle.isEmpty()
              && chosenTitle != m_title) {
            m_title = chosenTitle;
            Q_EMIT titleChanged();
          }

          m_filePath = finalPath;
          (void)finalizeProjectSave();
        },
        Qt::QueuedConnection);
    });

    connect(dialog, &QFileDialog::finished, this, [this, accepted](int) {
      Q_EMIT saveDialogCompleted(*accepted);
    });

    dialog->open();
    return false;
  }

  return finalizeProjectSave();
}

/**
 * @brief Headless save to the given path (no file dialog).
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

  const bool hasDatasetlessGroup =
    std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
      return g.widget == QLatin1String("image") || g.widget == QLatin1String("painter");
    });

  if (datasetCount() <= 0 && !hasDatasetlessGroup) {
    qWarning() << "[ProjectModel] Project needs at least one dataset";
    return false;
  }

  QString finalPath = path;
  if (!finalPath.endsWith(QStringLiteral(".ssproj"), Qt::CaseInsensitive))
    finalPath += QStringLiteral(".ssproj");

  m_filePath = finalPath;
  return finalizeProjectSave();
}

/**
 * @brief Serializes the complete project state to a QJsonObject.
 */
QJsonObject DataModel::ProjectModel::serializeToJson() const
{
  QJsonObject json;
  json.insert(Keys::Title, m_title);
  json.insert(Keys::PointCount, m_pointCount);
  json.insert(Keys::PlotTimeRange, m_plotTimeRange);
  json.insert(Keys::Frozen, m_frozen);
  json.insert(Keys::ChangeDrivenTransforms, m_changeDrivenTransforms);
  json.insert(Keys::HexadecimalDelimiters, m_hexadecimalDelimiters);

  const QString writer  = DataModel::current_writer_version();
  const QString creator = m_writerVersionAtCreation.isEmpty() ? writer : m_writerVersionAtCreation;
  json.insert(Keys::SchemaVersion, DataModel::kSchemaVersion);
  json.insert(Keys::WriterVersion, writer);
  json.insert(Keys::WriterVersionAtCreation, creator);
  json.insert(Keys::NextUniqueId, m_nextUniqueId);

  if (!m_passwordHash.isEmpty())
    json.insert(Keys::PasswordHash, m_passwordHash);

  if (!m_controlScriptCode.isEmpty())
    json.insert(Keys::ControlScriptCode, m_controlScriptCode);

  QJsonArray groupArray;
  for (const auto& group : std::as_const(m_groups))
    groupArray.append(DataModel::serialize(group));

  json.insert(Keys::Groups, groupArray);

  if (!m_groupFolders.empty())
    json.insert(Keys::GroupFolders, serializeFolders(m_groupFolders));

  QJsonArray actionsArray;
  for (const auto& action : std::as_const(m_actions))
    actionsArray.append(DataModel::serialize(action));

  json.insert(Keys::Actions, actionsArray);

  QJsonArray sourcesArray;
  for (const auto& source : std::as_const(m_sources))
    sourcesArray.append(DataModel::serialize(source));

  json.insert(Keys::Sources, sourcesArray);

  if (m_customizeWorkspaces) {
    json.insert(Keys::CustomizeWorkspaces, true);

    QJsonArray workspacesArray;
    for (const auto& ws : std::as_const(m_workspaces))
      workspacesArray.append(DataModel::serialize(ws));

    json.insert(Keys::Workspaces, workspacesArray);

    QJsonArray foldersArray;
    for (const auto& folder : std::as_const(m_workspaceFolders))
      foldersArray.append(DataModel::serialize(folder));

    if (!foldersArray.isEmpty())
      json.insert(Keys::WorkspaceFolders, foldersArray);
  }

  if (!m_hiddenGroupIds.isEmpty()) {
    QJsonArray hiddenArray;
    for (const int id : std::as_const(m_hiddenGroupIds))
      hiddenArray.append(id);

    json.insert(Keys::HiddenGroups, hiddenArray);
  }

  if (!m_tables.empty()) {
    QJsonArray tablesArray;
    for (const auto& table : std::as_const(m_tables))
      tablesArray.append(DataModel::serialize(table));

    json.insert(Keys::Tables, tablesArray);
  }

  if (!m_tableFolders.empty())
    json.insert(Keys::TableFolders, serializeFolders(m_tableFolders));

  if (!m_widgetSettings.isEmpty())
    json.insert(Keys::WidgetSettings, m_widgetSettings);

  if (!m_widgetDisplay.isEmpty())
    json.insert(Keys::WidgetDisplay, m_widgetDisplay);

  if (!m_treeExpansion.isEmpty())
    json.insert(Keys::TreeExpansion, m_treeExpansion);

  if (!m_diagramCollapse.isEmpty())
    json.insert(Keys::DiagramCollapse, m_diagramCollapse);

  if (!m_mqttPublisher.isEmpty())
    json.insert(Keys::MqttPublisher, m_mqttPublisher);

  return json;
}

/**
 * @brief Silently writes the current project to disk; called from the debounce timer.
 */
void DataModel::ProjectModel::autoSave()
{
  if (m_autoSaveSuspended)
    return;

  if (m_filePath.isEmpty() || m_locked || !m_modified)
    return;

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!writeProjectFile(m_filePath)) {
    qWarning() << "[ProjectModel] Auto-save failed";
    return;
  }

  setModified(false);
  m_history.markSaved();

  if (m_runtimeDirty)
    syncRuntime();
}

/**
 * @brief Rebuilds the live frame pipeline from the current project and clears the runtime-dirty
 *        flag. Resets the dashboard the same way the enable/disable toggle does.
 */
void DataModel::ProjectModel::syncRuntime()
{
  m_runtimeDirty            = false;
  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.syncFromProjectModel();
}

/**
 * @brief Flushes any pending debounced autosave synchronously (called on app quit).
 */
void DataModel::ProjectModel::flushAutoSave()
{
  if (m_autoSaveTimer && m_autoSaveTimer->isActive())
    m_autoSaveTimer->stop();

  autoSave();
}

/**
 * @brief Starts the debounced autosave when saving is currently permitted; the API mutation
 *        path calls this so programmatic edits persist without an explicit save.
 */
void DataModel::ProjectModel::scheduleAutoSave()
{
  if (m_autoSaveSuspended || m_filePath.isEmpty() || m_locked)
    return;

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  m_autoSaveTimer->start();
}

/**
 * @brief Suspends or resumes the debounced autosave (used by the API batch endpoint).
 */
void DataModel::ProjectModel::setAutoSaveSuspended(bool suspend)
{
  if (m_autoSaveSuspended == suspend)
    return;

  m_autoSaveSuspended = suspend;
  if (suspend && m_autoSaveTimer)
    m_autoSaveTimer->stop();
}

/**
 * @brief Atomically serializes the current project to @p path.
 */
bool DataModel::ProjectModel::writeProjectFile(const QString& path)
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

/**
 * @brief Returns the SHA-256 of the file at @p path, or an empty array when unreadable.
 */
QByteArray DataModel::ProjectModel::hashProjectFile(const QString& path)
{
  QFile file(path);
  if (!file.open(QFile::ReadOnly))
    return {};

  return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256);
}

/**
 * @brief Re-arms the filesystem watcher on m_filePath and caches the on-disk content hash;
 *        called after every successful write or load so self-saves are recognized as such
 *        (QSaveFile's atomic rename drops the previous watch on some platforms).
 */
void DataModel::ProjectModel::watchProjectFile()
{
  const auto watched = m_fileWatcher->files();
  if (!watched.isEmpty())
    m_fileWatcher->removePaths(watched);

  m_diskFileHash.clear();
  if (m_filePath.isEmpty() || !QFile::exists(m_filePath))
    return;

  m_fileWatcher->addPath(m_filePath);
  m_diskFileHash = hashProjectFile(m_filePath);
}

/**
 * @brief Debounced watcher handler: ignores self-saves (hash unchanged), flags deletion,
 *        and prompts to reload when another program modified the project file.
 */
void DataModel::ProjectModel::resolveDiskFileChange()
{
  m_diskCheckPending = false;
  if (m_diskPromptActive || m_filePath.isEmpty())
    return;

  if (!QFile::exists(m_filePath)) {
    m_diskFileHash.clear();
    m_modified = true;
    Q_EMIT modifiedChanged();
    Q_EMIT projectFileChangedOnDisk();
    static auto& nc = DataModel::NotificationCenter::instance();
    nc.postWarning(
      QStringLiteral("ProjectModel"),
      tr("Project file removed from disk"),
      tr("%1 was deleted or renamed by another program. Save the project to recreate it.")
        .arg(jsonFileName()));
    return;
  }

  if (!m_fileWatcher->files().contains(m_filePath))
    m_fileWatcher->addPath(m_filePath);

  const auto hash = hashProjectFile(m_filePath);
  if (hash.isEmpty() || hash == m_diskFileHash)
    return;

  m_diskFileHash = hash;
  Q_EMIT projectFileChangedOnDisk();

  if (m_suppressMessageBoxes) {
    qWarning() << "[ProjectModel] Project file changed on disk; keeping in-memory state";
    m_modified = true;
    Q_EMIT modifiedChanged();
    static auto& nc = DataModel::NotificationCenter::instance();
    nc.postWarning(
      QStringLiteral("ProjectModel"),
      tr("Project file changed on disk"),
      tr("%1 was modified by another program. The in-memory project was kept; reopen the "
         "file to load the external changes.")
        .arg(jsonFileName()));
    return;
  }

  promptDiskFileReload();
}

/**
 * @brief Asks whether to reload the externally-modified project file; declining keeps the
 *        in-memory state and marks it modified so the divergence is saveable.
 */
void DataModel::ProjectModel::promptDiskFileReload()
{
  m_diskPromptActive  = true;
  const auto question = m_modified ? tr("The project file was modified by another program.\n\n"
                                        "Reload it and discard your unsaved changes?")
                                   : tr("The project file was modified by another program.\n\n"
                                        "Reload it?");
  const auto ret      = Misc::Utilities::showMessageBox(tr("Project file changed on disk"),
                                                   question,
                                                   QMessageBox::Question,
                                                   APP_NAME,
                                                   QMessageBox::Yes | QMessageBox::No);
  m_diskPromptActive  = false;

  if (ret != QMessageBox::Yes) {
    m_modified = true;
    Q_EMIT modifiedChanged();
    return;
  }

  const auto path = m_filePath;
  m_filePath.clear();
  if (!openJsonFile(path))
    qWarning() << "[ProjectModel] Reload after on-disk change failed:" << path;
}

/**
 * @brief Writes the current project to m_filePath and reloads it.
 */
bool DataModel::ProjectModel::finalizeProjectSave()
{
  resolveDatasetTransformLanguages();
  resolveDatasetVirtualFlags();

  if (!writeProjectFile(m_filePath)) {
    if (!m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(tr("File save error"), m_filePath, QMessageBox::Critical);

    return false;
  }

  static auto& appState = AppState::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);
  setModified(false);
  m_history.markSaved();
  Q_EMIT jsonFileChanged();
  return true;
}
