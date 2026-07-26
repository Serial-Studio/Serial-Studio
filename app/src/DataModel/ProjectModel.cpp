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

#include "DataModel/ProjectModel.h"

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
#include "Project/ProjectModelShared.h"
#include "SessionContext.h"
#include "UI/Dashboard.h"
#include "UI/WidgetExtensions.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/LemonSqueezy.h"
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
  , m_plotTimeRange(10.0)
  , m_frozen(false)
  , m_changeDrivenTransforms(false)
  , m_nextUniqueId(1)
  , m_modified(false)
  , m_initialized(false)
  , m_silentReload(false)
  , m_filePath("")
  , m_suppressMessageBoxes(false)
  , m_customizeWorkspaces(false)
  , m_passwordHash("")
  , m_locked(false)
  , m_autoSaveTimer(new QTimer(this))
  , m_autoSaveSuspended(false)
  , m_runtimeDirty(false)
  , m_mutationEpoch(0)
  , m_fileWatcher(new QFileSystemWatcher(this))
  , m_diskCheckPending(false)
  , m_diskPromptActive(false)
{
  m_autoSaveTimer->setSingleShot(true);
  m_autoSaveTimer->setInterval(1500);
  connect(m_autoSaveTimer, &QTimer::timeout, this, &ProjectModel::autoSave);

  connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [this] {
    if (m_diskCheckPending)
      return;

    m_diskCheckPending = true;
    QTimer::singleShot(500, this, &ProjectModel::resolveDiskFileChange);
  });

  const auto bumpEpoch = [this] {
    ++m_mutationEpoch;
  };
  connect(this, &ProjectModel::groupAdded, this, bumpEpoch);
  connect(this, &ProjectModel::groupDeleted, this, bumpEpoch);
  connect(this, &ProjectModel::datasetAdded, this, bumpEpoch);
  connect(this, &ProjectModel::datasetDeleted, this, bumpEpoch);
  connect(this, &ProjectModel::sourceAdded, this, bumpEpoch);
  connect(this, &ProjectModel::sourceDeleted, this, bumpEpoch);
  connect(this, &ProjectModel::sourceStructureChanged, this, bumpEpoch);
  connect(this, &ProjectModel::groupsChanged, this, bumpEpoch);
  connect(this, &ProjectModel::actionsChanged, this, bumpEpoch);
  connect(this, &ProjectModel::sourceChanged, this, bumpEpoch);
  connect(this, &ProjectModel::sourcesChanged, this, bumpEpoch);
  connect(this, &ProjectModel::sourceFrameParserCodeChanged, this, bumpEpoch);
  connect(this, &ProjectModel::frameDetectionChanged, this, bumpEpoch);
  connect(this, &ProjectModel::editorWorkspacesChanged, this, bumpEpoch);
  connect(this, &ProjectModel::tablesChanged, this, bumpEpoch);
  connect(this, &ProjectModel::titleChanged, this, bumpEpoch);

  const auto markDirty = [this] {
    m_runtimeDirty = true;
    scheduleAutoSave();
  };
  connect(this, &ProjectModel::groupsChanged, this, markDirty);
  connect(this, &ProjectModel::groupDataChanged, this, markDirty);
  connect(this, &ProjectModel::actionsChanged, this, markDirty);
  connect(this, &ProjectModel::sourcesChanged, this, markDirty);
  connect(this, &ProjectModel::sourceChanged, this, markDirty);
  connect(this, &ProjectModel::sourceStructureChanged, this, markDirty);
  connect(this, &ProjectModel::frameDetectionChanged, this, markDirty);
  connect(this, &ProjectModel::sourceFrameParserCodeChanged, this, markDirty);
  connect(this, &ProjectModel::tablesChanged, this, markDirty);

  connect(this, &ProjectModel::titleChanged, this, &ProjectModel::saveStatusChanged);
  connect(this, &ProjectModel::groupsChanged, this, &ProjectModel::saveStatusChanged);

  connect(this, &ProjectModel::widgetSettingsChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::widgetDisplayChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::sourceChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::sourcesChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::sourceFrameParserCodeChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::frameDetectionChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::editorWorkspacesChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::tablesChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::titleChanged, this, &ProjectModel::scheduleAutoSave);

  // code-verify off
  // Must run before the groupsChanged auto-regen connect below: newJsonFile()
  // emits groupsChanged while AppState is still mid-init, so wiring first would
  // fire the regen handler against half-initialized state.
  newJsonFile();
  // code-verify on

  connect(this, &ProjectModel::groupsChanged, this, [this] {
    static auto& appState = AppState::instance();
    if (appState.operationMode() != SerialStudio::ProjectFile)
      return;

    if (m_customizeWorkspaces) {
      if (mergeAutoWorkspaceUpdates()) {
        Q_EMIT editorWorkspacesChanged();
        Q_EMIT activeWorkspacesChanged();
      }
      return;
    }

    regenerateAutoWorkspacesUnnotified();
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
  });

  m_initialized = true;
  m_history.setEnabled(true);
}

/**
 * @brief Returns this session's project document. The object is owned by the SessionContext and
 *        built by the composition root, so a reach before adoption is a named fatal instead of
 *        an out-of-order lazy construction (spec 0039 M2, wave C2).
 */
DataModel::ProjectModel& DataModel::ProjectModel::instance()
{
  return SessionContext::current().projectModel();
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

/**
 * @brief Returns @c true if the project configuration is sufficient to generate a valid
 *        dashboard configuration
 */
bool DataModel::ProjectModel::validateProject(const bool silent)
{
  if (m_title.isEmpty()) {
    if (!silent) {
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("Project title cannot be empty!"), QMessageBox::Warning);
    }

    return false;
  }

  if (groupCount() <= 0) {
    if (!silent) {
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("You need to add at least one group!"), QMessageBox::Warning);
    }

    return false;
  }

  const bool hasImageGroup = std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
    return g.widget == QLatin1String("image");
  });

  if (datasetCount() <= 0 && !hasImageGroup) {
    if (!silent) {
      Misc::Utilities::showMessageBox(
        tr("Project error"), tr("You need to add at least one dataset!"), QMessageBox::Warning);
    }

    return false;
  }

  return true;
}

/**
 * @brief Identifies which (if any) save prerequisite is currently missing.
 */
DataModel::ProjectModel::SaveBlocker DataModel::ProjectModel::saveBlockerCode() const
{
  if (m_title.isEmpty())
    return SaveBlocker::MissingTitle;

  if (groupCount() <= 0)
    return SaveBlocker::MissingGroup;

  const bool hasImageGroup = std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
    return g.widget == QLatin1String("image");
  });

  if (datasetCount() <= 0 && !hasImageGroup)
    return SaveBlocker::MissingDataset;

  return SaveBlocker::None;
}

/**
 * @brief Returns true when the project has everything saveJsonFile() needs.
 */
bool DataModel::ProjectModel::canSave() const
{
  return saveBlockerCode() == SaveBlocker::None;
}

/**
 * @brief Returns a short, HIG-style heading describing the current save blocker.
 */
QString DataModel::ProjectModel::saveBlockerTitle() const
{
  switch (saveBlockerCode()) {
    case SaveBlocker::None:
      return QString();
    case SaveBlocker::MissingTitle:
      return tr("Your project needs a title");
    case SaveBlocker::MissingGroup:
      return tr("Add a group to get started");
    case SaveBlocker::MissingDataset:
      return tr("Add a dataset to a group");
  }
  return QString();
}

/**
 * @brief Returns one or two sentences of HIG-style guidance on how to fix the blocker.
 */
QString DataModel::ProjectModel::saveBlockerDetail() const
{
  switch (saveBlockerCode()) {
    case SaveBlocker::None:
      return QString();
    case SaveBlocker::MissingTitle:
      return tr("Open the Project view at the top of the tree and enter "
                "a name. You can rename the project at any time.");
    case SaveBlocker::MissingGroup:
      return tr("Groups organize datasets into dashboard widgets. Use the "
                "Group button in the toolbar above to create one, then add "
                "datasets to it.");
    case SaveBlocker::MissingDataset:
      return tr("Datasets are the values that appear on the dashboard. "
                "Select a group in the tree and use the Dataset button in "
                "the toolbar to add one.");
  }
  return QString();
}

//--------------------------------------------------------------------------------------------------
// Project lock: UX read-only flag (plain MD5, not crypto)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the editor body is gated behind the lock screen.
 */
bool DataModel::ProjectModel::locked() const noexcept
{
  return m_locked;
}

/**
 * @brief Monotonic counter bumped on every structural project mutation.
 */
qint64 DataModel::ProjectModel::mutationEpoch() const noexcept
{
  return m_mutationEpoch;
}

//--------------------------------------------------------------------------------------------------
// Undo/redo history
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when a project-document step can be undone.
 */
bool DataModel::ProjectModel::canUndo() const noexcept
{
  return m_history.canUndo();
}

/**
 * @brief Returns true when an undone project-document step can be replayed.
 */
bool DataModel::ProjectModel::canRedo() const noexcept
{
  return m_history.canRedo();
}

/**
 * @brief Returns the label of the operation undo would revert.
 */
QString DataModel::ProjectModel::undoText() const
{
  return m_history.undoText();
}

/**
 * @brief Returns the label of the operation redo would replay.
 */
QString DataModel::ProjectModel::redoText() const
{
  return m_history.redoText();
}

/**
 * @brief Returns the undo history for scope/frame plumbing (API layer, editor bulk ops).
 */
DataModel::ProjectHistory& DataModel::ProjectModel::history() noexcept
{
  return m_history;
}

/**
 * @brief Reverts the most recent project-document step; the position moves only after the
 *        snapshot applied cleanly, so a failed apply leaves history consistent.
 */
bool DataModel::ProjectModel::undo()
{
  if (!m_history.canUndo() || m_history.applying())
    return false;

  const auto current = QJsonDocument(serializeToJson()).toJson(QJsonDocument::Compact);
  const auto state   = m_history.peekUndoState();
  if (!applyHistorySnapshot(state)) {
    qWarning() << "[ProjectModel] Undo failed to apply the stored snapshot";
    return false;
  }

  m_history.confirmUndo(current);
  setModified(!m_history.isAtSavePoint());
  Q_EMIT projectHistoryChanged();
  return true;
}

/**
 * @brief Replays the most recently undone project-document step; position moves only on a
 *        clean apply.
 */
bool DataModel::ProjectModel::redo()
{
  if (!m_history.canRedo() || m_history.applying())
    return false;

  const auto state = m_history.peekRedoState();
  if (!applyHistorySnapshot(state)) {
    qWarning() << "[ProjectModel] Redo failed to apply the stored snapshot";
    return false;
  }

  m_history.confirmRedo();
  setModified(!m_history.isAtSavePoint());
  Q_EMIT projectHistoryChanged();
  return true;
}

/**
 * @brief Stores the label/coalesce hint the editor sets right before a keystroke commit.
 */
void DataModel::ProjectModel::setNextUndoHint(const QString& label, const QString& coalesceKey)
{
  m_history.setNextHint(label, coalesceKey);
}

/**
 * @brief Prompts for a password, hashes it, and locks the editor.
 */
void DataModel::ProjectModel::lockProject()
{
  bool ok          = false;
  const auto first = QInputDialog::getText(nullptr,
                                           tr("Lock Project"),
                                           tr("Choose a password to lock the project:"),
                                           QLineEdit::Password,
                                           QString(),
                                           &ok);
  if (!ok || first.isEmpty())
    return;

  const auto second = QInputDialog::getText(
    nullptr, tr("Lock Project"), tr("Confirm the password:"), QLineEdit::Password, QString(), &ok);

  if (first != second || !ok) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Passwords do not match"),
        tr("The two passwords you entered do not match. The project was not locked."),
        QMessageBox::Warning);
    });
    return;
  }

  m_passwordHash = Misc::PasswordHash::hashPassword(first);

  if (!m_locked) {
    m_locked = true;
    Q_EMIT lockedChanged();
  }

  m_history.clear();
  Q_EMIT projectHistoryChanged();

  if (validateProject(true)) {
    setModified(true);
    (void)saveJsonFile(false);
  }
}

/**
 * @brief Prompts for the password, verifies it, and clears the lock on success.
 */
void DataModel::ProjectModel::unlockProject()
{
  if (m_passwordHash.isEmpty()) {
    if (m_locked) {
      m_locked = false;
      Q_EMIT lockedChanged();
    }
    return;
  }

  bool ok        = false;
  const auto pwd = QInputDialog::getText(nullptr,
                                         tr("Unlock Project"),
                                         tr("Enter the project password:"),
                                         QLineEdit::Password,
                                         QString(),
                                         &ok);
  if (!ok)
    return;

  if (!Misc::PasswordHash::verifyPassword(pwd, m_passwordHash)) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Incorrect password"),
        tr("The password you entered does not match the one stored in the project file."),
        QMessageBox::Warning);
    });
    return;
  }

  m_passwordHash.clear();

  if (m_locked) {
    m_locked = false;
    Q_EMIT lockedChanged();
  }

  m_history.clear();
  Q_EMIT projectHistoryChanged();

  if (validateProject(true)) {
    setModified(true);
    (void)saveJsonFile(false);
  }
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
  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  return workspaceManager.path("Projects");
}

/**
 * @brief Allocates the next persistent uniqueId for a new group or dataset.
 */
int DataModel::ProjectModel::allocateUniqueId()
{
  return m_nextUniqueId++;
}

/**
 * @brief Resolves a Group.uniqueId to its current positional groupId; returns -1 if absent.
 */
int DataModel::ProjectModel::groupIdForUniqueId(int uniqueId) const
{
  if (uniqueId < 0)
    return -1;

  for (const auto& group : m_groups)
    if (group.uniqueId == uniqueId)
      return group.groupId;

  return -1;
}

/**
 * @brief Resolves a positional groupId to its Group.uniqueId; returns -1 if out of range.
 */
int DataModel::ProjectModel::groupUniqueIdForGroupId(int groupId) const
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return -1;

  return m_groups[static_cast<size_t>(groupId)].uniqueId;
}

/**
 * @brief Returns "Time", "Samples", then every dataset label sorted by uniqueId.
 */
QStringList DataModel::ProjectModel::xDataSources() const
{
  QStringList list;
  list.append(tr("Time"));
  list.append(tr("Samples"));

  QMap<int, QString> datasets;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!datasets.contains(uid))
        datasets.insert(uid, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
}

/**
 * @brief Parallel to xDataSources(): the dataset uniqueId at each combo position
 *        (position 0 -> -2 "Time", 1 -> -1 "Samples", then dataset uniqueIds).
 */
QList<int> DataModel::ProjectModel::xDataSourceUniqueIds() const
{
  QList<int> out;
  out.append(kXAxisTime);
  out.append(kXAxisSamples);

  QMap<int, bool> seen;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!seen.contains(uid))
        seen.insert(uid, true);
    }
  }

  for (auto it = seen.cbegin(); it != seen.cend(); ++it)
    out.append(it.key());

  return out;
}

/**
 * @brief Returns "Time" plus every dataset label, sorted by uniqueId.
 */
QStringList DataModel::ProjectModel::yWaterfallSources() const
{
  QStringList list;
  list.append(tr("Time"));

  QMap<int, QString> datasets;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!datasets.contains(uid))
        datasets.insert(uid, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
}

/**
 * @brief Parallel to yWaterfallSources(): the dataset uniqueId at each combo position
 *        (position 0 -> 0, the "Time" sentinel).
 */
QList<int> DataModel::ProjectModel::yWaterfallSourceUniqueIds() const
{
  QList<int> out;
  out.append(0);

  QMap<int, bool> seen;
  for (const auto& group : m_groups) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!seen.contains(uid))
        seen.insert(uid, true);
    }
  }

  for (auto it = seen.cbegin(); it != seen.cend(); ++it)
    out.append(it.key());

  return out;
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
 * @brief Returns the native parser template id for the global frame parser (source 0).
 */
QString DataModel::ProjectModel::frameParserTemplate() const
{
  if (m_sources.empty())
    return QString();

  return m_sources[0].frameParserTemplate;
}

/**
 * @brief Returns the native parser template id for the source, or source 0's.
 */
QString DataModel::ProjectModel::frameParserTemplate(int sourceId) const
{
  for (const auto& src : m_sources)
    if (src.sourceId == sourceId)
      return src.frameParserTemplate;

  return frameParserTemplate();
}

/**
 * @brief Returns the native parser template params for the global frame parser (source 0).
 */
QJsonObject DataModel::ProjectModel::frameParserParams() const
{
  if (m_sources.empty())
    return QJsonObject();

  return m_sources[0].frameParserParams;
}

/**
 * @brief Returns the native parser template params for the source, or source 0's.
 */
QJsonObject DataModel::ProjectModel::frameParserParams(int sourceId) const
{
  for (const auto& src : m_sources)
    if (src.sourceId == sourceId)
      return src.frameParserParams;

  return frameParserParams();
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
 * @brief Returns the persisted layout for the given group within a window scope.
 */
QJsonObject DataModel::ProjectModel::groupLayout(const QString& scope, int groupId) const
{
  const auto key = Keys::layoutKey(scope, groupId);
  return m_widgetSettings.value(key).toObject().value("data").toObject();
}

/**
 * @brief Returns the persisted external-window records (workspace, geometry, state).
 */
QJsonArray DataModel::ProjectModel::externalWindows() const
{
  return m_widgetSettings.value(Keys::kDashboardWindowsSubKey).toArray();
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
 * @brief Builds the widget-scoped subkey ("<type>:<uid>") used by the titles and
 *        freezeTitle maps.
 */
static QString widget_scope_key(int widgetType, int uniqueId)
{
  return QString::number(widgetType) + QLatin1Char(':') + QString::number(uniqueId);
}

/**
 * @brief Builds the widget-scoped subkey for a widget-extension slot
 * ("ext:&lt;id&gt;:&lt;uid&gt;"). Extension widgets share one enum value, so the numeric key cannot
 * address one; the package id comes from the entity's own widget string (the dashboard's
 * title-resolution key), falling back to the numeric key when the entity names no package.
 */
static QString extension_scope_key(int widgetType,
                                   int uniqueId,
                                   const std::vector<DataModel::Group>& groups)
{
  if (widgetType != static_cast<int>(SerialStudio::DashboardExtension))
    return widget_scope_key(widgetType, uniqueId);

  QString package;
  for (const auto& group : groups) {
    if (group.uniqueId == uniqueId)
      package = group.widget;

    for (const auto& dataset : group.datasets)
      if (dataset.uniqueId == uniqueId)
        package = dataset.widget;
  }

  if (package.isEmpty())
    return widget_scope_key(widgetType, uniqueId);

  return UI::WidgetExtensions::persistedTypeToken(package) + QLatin1Char(':')
       + QString::number(uniqueId);
}

/**
 * @brief Returns the entity-level display-title override for the given unique ID (empty =
 *        no entity-level override; widget-level entries may still exist).
 */
QString DataModel::ProjectModel::displayTitle(int uniqueId) const
{
  const auto titles = m_widgetDisplay.value(Keys::Titles).toObject();
  return titles.value(QString::number(uniqueId)).toString();
}

/**
 * @brief Returns the widget-level display-title override for (widgetType, uniqueId); this
 *        scope wins over the entity-level override during resolution.
 */
QString DataModel::ProjectModel::widgetDisplayTitle(int widgetType, int uniqueId) const
{
  const auto titles = m_widgetDisplay.value(Keys::Titles).toObject();
  return titles.value(extension_scope_key(widgetType, uniqueId, m_groups)).toString();
}

/**
 * @brief Returns the freeze-title mode ("bar", "painted" or "hidden") for the given widget;
 *        unset widgets resolve to "painted" for title-painting instruments, "bar" otherwise.
 */
QString DataModel::ProjectModel::freezeTitleMode(int widgetType, int uniqueId) const
{
  const auto modes = m_widgetDisplay.value(Keys::FreezeTitle).toObject();
  const auto mode  = modes.value(extension_scope_key(widgetType, uniqueId, m_groups)).toString();
  if (!mode.isEmpty())
    return mode;

  const auto widget = static_cast<SerialStudio::DashboardWidget>(widgetType);
  return SerialStudio::dashboardWidgetPaintsTitle(widget) ? QStringLiteral("painted")
                                                          : QStringLiteral("bar");
}

/**
 * @brief Returns the full display-title override map (uniqueId -> title).
 */
QJsonObject DataModel::ProjectModel::displayTitles() const
{
  return m_widgetDisplay.value(Keys::Titles).toObject();
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
 * @brief Returns the project's plot time range in seconds (visible window for time-axis plots).
 */
double DataModel::ProjectModel::plotTimeRange() const noexcept
{
  return m_plotTimeRange;
}

/**
 * @brief Returns whether the dashboard is frozen into an operator panel (chrome hidden,
 *        layout locked); the stored flag, independent of license state.
 */
bool DataModel::ProjectModel::frozen() const noexcept
{
  return m_frozen;
}

/**
 * @brief Returns whether change-driven transform execution is enabled for this project.
 */
bool DataModel::ProjectModel::changeDrivenTransforms() const noexcept
{
  return m_changeDrivenTransforms;
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
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::editorWorkspaces() const noexcept
{
  return m_workspaces;
}

/**
 * @brief Returns the editor-owned workspace folder list.
 */
const std::vector<DataModel::WorkspaceFolder>& DataModel::ProjectModel::editorWorkspaceFolders()
  const noexcept
{
  return m_workspaceFolders;
}

/**
 * @brief Returns the editor-owned group folder list.
 */
const std::vector<DataModel::GroupFolder>& DataModel::ProjectModel::editorGroupFolders()
  const noexcept
{
  return m_groupFolders;
}

/**
 * @brief Returns the editor-owned table folder list.
 */
const std::vector<DataModel::TableFolder>& DataModel::ProjectModel::editorTableFolders()
  const noexcept
{
  return m_tableFolders;
}

/**
 * @brief Returns the workspace list currently rendered by the dashboard.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::activeWorkspaces() const
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
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
 * @brief Returns the project's MQTT publisher configuration (empty when unset).
 */
const QJsonObject& DataModel::ProjectModel::mqttPublisher() const noexcept
{
  return m_mqttPublisher;
}

/**
 * @brief Replaces the MQTT publisher configuration and marks the project as modified.
 */
void DataModel::ProjectModel::setMqttPublisher(const QJsonObject& config)
{
  if (m_mqttPublisher == config)
    return;

  const ProjectUndoScope undo_scope{*this, tr("Change MQTT Publisher")};

  m_mqttPublisher = config;
  setModified(true);
  Q_EMIT mqttPublisherChanged();
}

/**
 * @brief Stages a single widget setting and marks the project dirty. QML callers pass JS
 *        arrays/objects as QJSValue-wrapped variants, which QJsonValue::fromVariant silently
 *        turns into null, so they are unwrapped first.
 */
void DataModel::ProjectModel::saveWidgetSetting(const QString& widgetId,
                                                const QString& key,
                                                const QVariant& value)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  auto normalized = value;
  if (normalized.userType() == qMetaTypeId<QJSValue>())
    normalized = normalized.value<QJSValue>().toVariant();

  auto obj            = m_widgetSettings.value(widgetId).toObject();
  const auto newValue = QJsonValue::fromVariant(normalized);
  if (obj.value(key) == newValue)
    return;

  obj.insert(key, newValue);
  m_widgetSettings.insert(widgetId, obj);

  setModified(true);
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Stages one titles-map entry under @p key; an empty title removes the entry.
 *        Display-only: canonical dataset/group titles are never touched.
 */
void DataModel::ProjectModel::stageDisplayTitle(const QString& key, const QString& title)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  auto titles    = m_widgetDisplay.value(Keys::Titles).toObject();
  const auto old = titles.value(key).toString();
  if (old == title)
    return;

  if (title.isEmpty())
    titles.remove(key);
  else
    titles.insert(key, title);

  if (titles.isEmpty())
    m_widgetDisplay.remove(Keys::Titles);
  else
    m_widgetDisplay.insert(Keys::Titles, titles);

  setModified(true);
  Q_EMIT widgetDisplayChanged();
}

/**
 * @brief Stages an entity-level display-title override (every widget of the dataset/group
 *        with the given unique ID); an empty title removes the entry.
 */
void DataModel::ProjectModel::setDisplayTitle(int uniqueId, const QString& title)
{
  stageDisplayTitle(QString::number(uniqueId), title);
}

/**
 * @brief Stages a widget-level display-title override for (widgetType, uniqueId); wins over
 *        the entity-level entry, an empty title removes it.
 */
void DataModel::ProjectModel::setWidgetDisplayTitle(int widgetType,
                                                    int uniqueId,
                                                    const QString& title)
{
  stageDisplayTitle(extension_scope_key(widgetType, uniqueId, m_groups), title);
}

/**
 * @brief Prompts for a widget display title and stages it as a widget-level override;
 *        clearing the field restores the entity-level or canonical title.
 */
void DataModel::ProjectModel::promptRenameWidget(int widgetType,
                                                 int uniqueId,
                                                 const QString& currentTitle)
{
  if (widgetType < 0 || uniqueId < 0)
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(nullptr,
                                             tr("Rename Widget"),
                                             tr("Display title (empty restores the original):"),
                                             QLineEdit::Normal,
                                             currentTitle,
                                             &ok);

  if (!ok || name.trimmed() == currentTitle)
    return;

  stageDisplayTitle(extension_scope_key(widgetType, uniqueId, m_groups), name.trimmed());
}

/**
 * @brief Stages the freeze-title mode ("bar", "painted" or "hidden") for the given widget;
 *        "painted" is only valid for title-painting instruments, and writing a widget's
 *        per-type default removes the stored entry instead of recording it.
 */
void DataModel::ProjectModel::setFreezeTitleMode(int widgetType, int uniqueId, const QString& mode)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  const auto widget = static_cast<SerialStudio::DashboardWidget>(widgetType);
  const bool paints = SerialStudio::dashboardWidgetPaintsTitle(widget);
  if (mode != QLatin1String("bar") && mode != QLatin1String("hidden")
      && !(mode == QLatin1String("painted") && paints))
    return;

  const auto fallback = paints ? QLatin1String("painted") : QLatin1String("bar");
  const auto key      = extension_scope_key(widgetType, uniqueId, m_groups);
  auto modes          = m_widgetDisplay.value(Keys::FreezeTitle).toObject();
  const auto old      = modes.value(key).toString();
  const auto now      = (mode == fallback) ? QString() : mode;
  if (old == now)
    return;

  if (now.isEmpty())
    modes.remove(key);
  else
    modes.insert(key, now);

  if (modes.isEmpty())
    m_widgetDisplay.remove(Keys::FreezeTitle);
  else
    m_widgetDisplay.insert(Keys::FreezeTitle, modes);

  setModified(true);
  Q_EMIT widgetDisplayChanged();
}

/**
 * @brief Persists the external dashboard windows and prunes layouts of closed windows.
 */
void DataModel::ProjectModel::setExternalWindows(const QJsonArray& windows)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_widgetSettings.value(Keys::kDashboardWindowsSubKey).toArray() == windows)
    return;

  QSet<QString> liveScopes;
  for (const auto& value : windows) {
    const auto id = value.toObject().value(QStringLiteral("id")).toString();
    if (!id.isEmpty())
      liveScopes.insert(id);
  }

  const auto keys = m_widgetSettings.keys();
  for (const auto& key : keys) {
    const auto parts = key.split(QLatin1Char(':'));
    if (parts.size() == 3 && parts.first() == QLatin1String("layout")
        && !liveScopes.contains(parts.at(1)))
      m_widgetSettings.remove(key);
  }

  if (windows.isEmpty())
    m_widgetSettings.remove(Keys::kDashboardWindowsSubKey);
  else
    m_widgetSettings.insert(Keys::kDashboardWindowsSubKey, windows);

  setModified(true);
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Path-keyed Project Editor tree node expansion map (persisted in the file).
 */
const QJsonObject& DataModel::ProjectModel::treeExpansion() const noexcept
{
  return m_treeExpansion;
}

/**
 * @brief Stores the editor tree expansion map, marking the project dirty when it changed.
 */
void DataModel::ProjectModel::setTreeExpansion(const QJsonObject& expansion)
{
  if (m_treeExpansion == expansion)
    return;

  m_treeExpansion = expansion;
  setModified(true);
  scheduleAutoSave();
}

/**
 * @brief Stable-id keyed Project Overview diagram node collapse map (persisted in the file).
 */
const QJsonObject& DataModel::ProjectModel::diagramCollapse() const noexcept
{
  return m_diagramCollapse;
}

/**
 * @brief Stores the diagram collapse map, marking the project dirty when it changed.
 */
void DataModel::ProjectModel::setDiagramCollapse(const QJsonObject& state)
{
  if (m_diagramCollapse == state)
    return;

  m_diagramCollapse = state;
  Q_EMIT diagramCollapseChanged();
  setModified(true);
  scheduleAutoSave();
}

/**
 * @brief Stages a plugin's state in the project and marks it dirty.
 */
void DataModel::ProjectModel::savePluginState(const QString& pluginId, const QJsonObject& state)
{
  const auto key = QStringLiteral("plugin:") + pluginId;
  if (m_widgetSettings.value(key).toObject() == state)
    return;

  m_widgetSettings.insert(key, state);
  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile)
    setModified(true);

  Q_EMIT widgetSettingsChanged();
}

//--------------------------------------------------------------------------------------------------
// Signal/slot setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires Dashboard, ConnectionManager, AppState and licensing signals to this model.
 *        The activation hook re-derives the workspace list because the auto layout bakes the
 *        Pro/fallback widget choice (Plot3D vs MultiPlot) in at build time, so a license flip
 *        after project load would otherwise leave stale fallback refs on screen.
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

    if (!writeProjectFile(m_filePath))
      return;

    Q_EMIT pointCountChanged();
  });

  connect(&UI::Dashboard::instance(), &UI::Dashboard::widgetCountChanged, this, [this] {
    if (AppState::instance().operationMode() != SerialStudio::QuickPlot)
      return;

    m_sessionWorkspaces = buildAutoWorkspaces();
    Q_EMIT activeWorkspacesChanged();
  });

  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    const auto opMode = AppState::instance().operationMode();
    if (opMode == SerialStudio::ProjectFile)
      m_sessionWorkspaces.clear();
    else
      m_sessionWorkspaces = buildAutoWorkspaces();

    Q_EMIT activeWorkspacesChanged();
  });

  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (!IO::ConnectionManager::instance().isConnected())
        clearTransientState();
    });

#ifdef BUILD_COMMERCIAL
  connect(
    &Licensing::LemonSqueezy::instance(), &Licensing::LemonSqueezy::activatedChanged, this, [this] {
      if (AppState::instance().operationMode() != SerialStudio::ProjectFile) {
        m_sessionWorkspaces = buildAutoWorkspaces();
        Q_EMIT activeWorkspacesChanged();
        return;
      }

      if (m_customizeWorkspaces) {
        if (mergeAutoWorkspaceUpdates()) {
          Q_EMIT editorWorkspacesChanged();
          Q_EMIT activeWorkspacesChanged();
        }

        return;
      }

      regenerateAutoWorkspacesUnnotified();
      Q_EMIT editorWorkspacesChanged();
      Q_EMIT activeWorkspacesChanged();
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
  m_groups.clear();
  m_actions.clear();
  m_sources.clear();
  m_workspaces.clear();
  m_autoSnapshot.clear();
  m_tables.clear();
  m_groupFolders.clear();
  m_tableFolders.clear();
  m_hiddenGroupIds.clear();
  m_customizeWorkspaces = false;

  const bool hadMqttPublisher = !m_mqttPublisher.isEmpty();
  m_mqttPublisher             = QJsonObject();

  const bool wasLocked = m_locked;
  m_passwordHash.clear();
  m_locked = false;

  m_frameEndSequence         = "\\n";
  m_checksumAlgorithm        = "";
  m_frameStartSequence       = "$";
  m_writerVersionAtCreation  = "";
  m_hexadecimalDelimiters    = false;
  m_title                    = tr("Untitled Project");
  m_pointCount               = 100;
  m_plotTimeRange            = 10.0;
  m_frozen                   = false;
  m_changeDrivenTransforms   = false;
  m_nextUniqueId             = 1;
  m_controlScriptCode        = "";
  static auto& controlScript = DataModel::ControlScript::instance();
  controlScript.setCode(m_controlScriptCode);
  m_frameDecoder    = SerialStudio::PlainText;
  m_frameDetection  = SerialStudio::EndDelimiterOnly;
  m_widgetSettings  = QJsonObject();
  m_widgetDisplay   = QJsonObject();
  m_treeExpansion   = QJsonObject();
  m_diagramCollapse = QJsonObject();

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
  seedDefaultFrameParser(defaultSource);
  m_sources.push_back(defaultSource);

  m_filePath = "";
  watchProjectFile();

  if (m_initialized) {
    static auto& appState = AppState::instance();
    if (appState.operationMode() == SerialStudio::ProjectFile) {
      static auto& dashboard = UI::Dashboard::instance();
      dashboard.setPoints(m_pointCount);
      dashboard.setPlotTimeRange(m_plotTimeRange);
    }
  }

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
  Q_EMIT controlScriptChanged();
  Q_EMIT pointCountChanged();
  Q_EMIT plotTimeRangeChanged();
  Q_EMIT frozenChanged();
  Q_EMIT changeDrivenTransformsChanged();

  if (wasLocked)
    Q_EMIT lockedChanged();

  if (hadMqttPublisher)
    Q_EMIT mqttPublisherChanged();

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  m_history.clear();
  Q_EMIT projectHistoryChanged();
  setModified(false);
}

/**
 * @brief Updates the project title and emits titleChanged.
 */
void DataModel::ProjectModel::setTitle(const QString& title)
{
  if (m_title != title) {
    const ProjectUndoScope undo_scope{*this, tr("Change Project Title")};
    m_title = title;
    setModified(true);
    Q_EMIT titleChanged();
  }
}

/**
 * @brief Returns the project setup()/loop() control script source.
 */
QString DataModel::ProjectModel::controlScriptCode() const
{
  return m_controlScriptCode;
}

/**
 * @brief Stages a new control script and pushes it to the live runtime.
 */
void DataModel::ProjectModel::setControlScriptCode(const QString& code)
{
  if (m_controlScriptCode == code)
    return;

  const ProjectUndoScope undo_scope{
    *this, tr("Edit Control Script"), QStringLiteral("control-script")};

  m_controlScriptCode        = code;
  static auto& controlScript = DataModel::ControlScript::instance();
  controlScript.setCode(code);
  setModified(true);
  Q_EMIT controlScriptChanged();
}

/**
 * @brief Sets the dashboard point count and syncs it to the Dashboard.
 */
void DataModel::ProjectModel::setPointCount(const int points)
{
  if (m_pointCount == points)
    return;

  const ProjectUndoScope undo_scope{*this, tr("Change Point Count")};

  m_pointCount = points;

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    static auto& dashboard = UI::Dashboard::instance();
    dashboard.setPoints(points);
  }

  setModified(true);
  Q_EMIT pointCountChanged();
}

/**
 * @brief Sets the project's plot time range (seconds) and syncs it to the Dashboard.
 */
void DataModel::ProjectModel::setPlotTimeRange(const double seconds)
{
  const double clamped = qMax(0.001, seconds);
  if (qFuzzyCompare(m_plotTimeRange, clamped))
    return;

  const ProjectUndoScope undo_scope{*this, tr("Change Plot Time Range")};

  m_plotTimeRange = clamped;

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    static auto& dashboard = UI::Dashboard::instance();
    dashboard.setPlotTimeRange(clamped);
  }

  setModified(true);
  Q_EMIT plotTimeRangeChanged();
}

/**
 * @brief Sets the dashboard freeze flag; enabling is license-gated, disabling always works
 *        so an expired license can never trap a project frozen.
 */
void DataModel::ProjectModel::setFrozen(const bool frozen)
{
  if (m_frozen == frozen)
    return;

  if (frozen && !SerialStudio::activated())
    return;

  const ProjectUndoScope undo_scope{*this, tr("Toggle Freeze")};

  m_frozen = frozen;
  setModified(true);
  Q_EMIT frozenChanged();
}

/**
 * @brief Toggles change-driven transform execution; the FrameBuilder refreshes its cached flag.
 */
void DataModel::ProjectModel::setChangeDrivenTransforms(const bool enabled)
{
  if (m_changeDrivenTransforms == enabled)
    return;

  const ProjectUndoScope undo_scope{*this, tr("Toggle Change-Driven Transforms")};

  m_changeDrivenTransforms = enabled;
  setModified(true);
  Q_EMIT changeDrivenTransformsChanged();
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
  if (m_frameStartSequence == sequence)
    return;

  const ProjectUndoScope undo_scope{*this, tr("Change Frame Start Sequence")};

  m_frameStartSequence = sequence;

  if (!m_sources.empty())
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

  const ProjectUndoScope undo_scope{*this, tr("Change Frame End Sequence")};

  m_frameEndSequence = sequence;

  if (!m_sources.empty())
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

  const ProjectUndoScope undo_scope{*this, tr("Change Checksum")};

  m_checksumAlgorithm = algorithm;

  if (!m_sources.empty())
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

  const ProjectUndoScope undo_scope{*this, tr("Change Frame Detection")};

  m_frameDetection = detection;

  if (!m_sources.empty())
    m_sources[0].frameDetection = static_cast<int>(detection);

  setModified(true);
  Q_EMIT frameDetectionChanged();
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

  const ProjectUndoScope undo_scope{*this, tr("Change Decoder")};

  m_frameDecoder = method;

  if (!m_sources.empty())
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

  const ProjectUndoScope undo_scope{*this, tr("Toggle Hex Delimiters")};

  m_hexadecimalDelimiters = hexadecimal;

  if (!m_sources.empty())
    m_sources[0].hexadecimalDelimiters = hexadecimal;

  Q_EMIT frameDetectionChanged();
  setModified(true);
}

//--------------------------------------------------------------------------------------------------
// Scalar property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the project's modification flag and emits modifiedChanged; a
 * request to dirty a truly empty project intentionally leaves the flag clean for
 * the dirty-flag UX but still emits contentTouched so the backup layer (whose
 * snapshot hash decides) gets nudged.
 */
void DataModel::ProjectModel::setModified(const bool modified)
{
  if (modified)
    m_history.commitPending();

  if (modified && m_groups.empty() && m_actions.empty() && m_tables.empty() && m_workspaces.empty()
      && !m_customizeWorkspaces && !m_locked && m_hiddenGroupIds.isEmpty()) {
    Q_EMIT contentTouched();
    return;
  }

  m_modified = modified;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Stages the active dashboard tab group ID.
 */
void DataModel::ProjectModel::setActiveGroupId(const int groupId)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  const int current = m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
  if (current == groupId)
    return;

  if (groupId >= 0)
    m_widgetSettings.insert(Keys::kActiveGroupSubKey, groupId);
  else
    m_widgetSettings.remove(Keys::kActiveGroupSubKey);

  setModified(true);
  Q_EMIT activeGroupIdChanged();
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Stages the widget layout for a specific group.
 */
void DataModel::ProjectModel::setGroupLayout(const int groupId, const QJsonObject& layout)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  QJsonObject entry;
  entry[QStringLiteral("data")] = layout;
  m_widgetSettings.insert(Keys::layoutKey(groupId), entry);

  setModified(true);
  Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Prompts for a new title and applies it to the group at @p groupId.
 */
void DataModel::ProjectModel::promptRenameGroup(int groupId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  bool ok        = false;
  const auto old = m_groups[groupId].title;
  const auto fresh =
    QInputDialog::getText(nullptr, tr("Rename Group"), tr("Name:"), QLineEdit::Normal, old, &ok)
      .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto group  = m_groups[groupId];
  group.title = fresh;
  updateGroup(groupId, group, true);
}

/**
 * @brief Prompts for a new title and applies it to the dataset at (groupId, datasetId).
 */
void DataModel::ProjectModel::promptRenameDataset(int groupId, int datasetId)
{
  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= m_groups[groupId].datasets.size())
    return;

  bool ok        = false;
  const auto old = m_groups[groupId].datasets[datasetId].title;
  const auto fresh =
    QInputDialog::getText(nullptr, tr("Rename Dataset"), tr("Name:"), QLineEdit::Normal, old, &ok)
      .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto dataset  = m_groups[groupId].datasets[datasetId];
  dataset.title = fresh;
  updateDataset(groupId, datasetId, dataset, true);
}

/**
 * @brief Prompts for a new title and applies it to the source at @p sourceId.
 */
void DataModel::ProjectModel::promptRenameSource(int sourceId)
{
  const Source* src = nullptr;
  for (const auto& s : m_sources)
    if (s.sourceId == sourceId) {
      src = &s;
      break;
    }
  if (!src)
    return;

  bool ok          = false;
  const auto old   = src->title;
  const auto fresh = QInputDialog::getText(
                       nullptr, tr("Rename Data Source"), tr("Name:"), QLineEdit::Normal, old, &ok)
                       .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  updateSourceTitle(sourceId, fresh);
}

/**
 * @brief Prompts for a new title and applies it to the action at @p actionId.
 */
void DataModel::ProjectModel::promptRenameAction(int actionId)
{
  if (actionId < 0 || static_cast<size_t>(actionId) >= m_actions.size())
    return;

  bool ok        = false;
  const auto old = m_actions[actionId].title;
  const auto fresh =
    QInputDialog::getText(nullptr, tr("Rename Action"), tr("Name:"), QLineEdit::Normal, old, &ok)
      .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  auto action  = m_actions[actionId];
  action.title = fresh;
  updateAction(actionId, action);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears workspaces and widget settings for QuickPlot/ConsoleOnly sessions that have no
 * backing project file.
 */
void DataModel::ProjectModel::clearTransientState()
{
  static auto& appState = AppState::instance();
  const auto opMode     = appState.operationMode();
  if (opMode == SerialStudio::ProjectFile || !m_filePath.isEmpty() || m_customizeWorkspaces)
    return;

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

  if (!m_mqttPublisher.isEmpty()) {
    m_mqttPublisher = QJsonObject();
    Q_EMIT mqttPublisherChanged();
  }

  setModified(false);
}

/**
 * @brief Returns the next available dataset frame index.
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
 */
QVariantList DataModel::ProjectModel::sourcesForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_sources.size()));

  for (const auto& src : m_sources) {
    QVariantMap map;
    map[Keys::SourceId] = src.sourceId;
    map[Keys::BusType]  = src.busType;
    map[Keys::Title]    = src.title;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all groups (with their datasets) for QML diagram consumption.
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
      dsMap[Keys::DatasetId]                = ds.datasetId;
      dsMap[Keys::Title]                    = ds.title;
      dsMap[Keys::Units]                    = ds.units;
      dsMap[Keys::Widget]                   = ds.widget;
      dsMap[QStringLiteral("hasTransform")] = !ds.transformCode.trimmed().isEmpty();
      datasets.append(dsMap);
    }

    QVariantMap map;
    map[Keys::GroupId] = grp.groupId;
    QVariantList outputWidgets;
    outputWidgets.reserve(static_cast<qsizetype>(grp.outputWidgets.size()));
    for (const auto& ow : grp.outputWidgets) {
      QVariantMap owMap;
      owMap[Keys::Title]      = ow.title;
      owMap[Keys::OutputType] = static_cast<int>(ow.type);
      outputWidgets.append(owMap);
    }

    map[Keys::SourceId]       = grp.sourceId;
    map[Keys::Title]          = grp.title;
    map[Keys::Widget]         = grp.widget;
    map[Keys::GroupType]      = static_cast<int>(grp.groupType);
    map[Keys::ParentFolderId] = grp.parentFolderId;
    map[Keys::Datasets]       = datasets;
    map[Keys::OutputWidgets]  = outputWidgets;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all actions suitable for QML diagram consumption.
 */
QVariantList DataModel::ProjectModel::actionsForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_actions.size()));

  for (const auto& act : m_actions) {
    QVariantMap map;
    map[Keys::ActionId] = act.actionId;
    map[Keys::SourceId] = act.sourceId;
    map[Keys::Title]    = act.title;
    map[Keys::Icon]     = Misc::IconEngine::resolveActionIconSource(act.icon);
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of project data tables (name + register count) for the diagram.
 */
QVariantList DataModel::ProjectModel::tablesForDiagram() const
{
  QVariantList result;
  result.reserve(static_cast<qsizetype>(m_tables.size()));

  for (const auto& tbl : m_tables) {
    QVariantMap map;
    map[Keys::Name]                      = tbl.name;
    map[Keys::ParentFolderId]            = tbl.parentFolderId;
    map[QStringLiteral("registerCount")] = static_cast<int>(tbl.registers.size());
    result.append(map);
  }

  return result;
}
