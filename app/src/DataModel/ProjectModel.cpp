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
#include <QFileInfo>
#include <QInputDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTimer>

#include "AppState.h"
#include "DataModel/Scripting/ControlScript.h"
#include "IO/ConnectionManager.h"
#include "Misc/PasswordHash.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "SessionContext.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/LemonSqueezy.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton instance access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ProjectModel singleton and seeds an empty project. The sub-objects are
 *        value members built before the body runs, and none of them reaches another module's
 *        instance() at construction, so the protected ctor closure stays free of the Meyers-guard
 *        recursion that shipped and crashed once (2026-07-07).
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
  , m_luaFastMode(false)
  , m_nextUniqueId(1)
  , m_modified(false)
  , m_initialized(false)
  , m_silentReload(false)
  , m_filePath("")
  , m_suppressMessageBoxes(false)
  , m_passwordHash("")
  , m_locked(false)
  , m_mutationEpoch(0)
  , m_presentation(*this)
  , m_persistence(*this)
  , m_folders(*this)
  , m_workspaces(*this)
  , m_tables(*this)
  , m_loader(*this)
  , m_sourceOps(*this)
  , m_entities(*this)
  , m_outputWidgets(*this)
  , m_bulk(*this)
{
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
    m_persistence.setRuntimeDirty(true);
    m_persistence.scheduleAutoSave();
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

    if (m_workspaces.customizeWorkspaces()) {
      if (m_workspaces.mergeAutoWorkspaceUpdates()) {
        Q_EMIT editorWorkspacesChanged();
        Q_EMIT activeWorkspacesChanged();
      }
      return;
    }

    m_workspaces.regenerateAutoWorkspacesUnnotified();
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

  const bool hasDatasetlessGroup =
    std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
      return g.widget == QLatin1String("image") || g.widget == QLatin1String("painter");
    });

  if (datasetCount() <= 0 && !hasDatasetlessGroup) {
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

  const bool hasDatasetlessGroup =
    std::any_of(m_groups.begin(), m_groups.end(), [](const Group& g) {
      return g.widget == QLatin1String("image") || g.widget == QLatin1String("painter");
    });

  if (datasetCount() <= 0 && !hasDatasetlessGroup)
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
  if (!m_loader.applyHistorySnapshot(state)) {
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
  if (!m_loader.applyHistorySnapshot(state)) {
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
  return m_presentation.xDataSources();
}

/**
 * @brief Parallel to xDataSources(): the dataset uniqueId at each combo position.
 */
QList<int> DataModel::ProjectModel::xDataSourceUniqueIds() const
{
  return m_presentation.xDataSourceUniqueIds();
}

/**
 * @brief Returns "Time" plus every dataset label, sorted by uniqueId.
 */
QStringList DataModel::ProjectModel::yWaterfallSources() const
{
  return m_presentation.yWaterfallSources();
}

/**
 * @brief Parallel to yWaterfallSources(): the dataset uniqueId at each combo position.
 */
QList<int> DataModel::ProjectModel::yWaterfallSourceUniqueIds() const
{
  return m_presentation.yWaterfallSourceUniqueIds();
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
 * @brief Returns whether Fast Lua execution (JIT on, watchdog off) is enabled for this project.
 */
bool DataModel::ProjectModel::luaFastMode() const noexcept
{
  return m_luaFastMode;
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
 * @brief Returns the editor-owned workspace list.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::editorWorkspaces() const noexcept
{
  return m_workspaces.list();
}

/**
 * @brief Returns the editor-owned workspace folder list.
 */
const std::vector<DataModel::WorkspaceFolder>& DataModel::ProjectModel::editorWorkspaceFolders()
  const noexcept
{
  return m_folders.workspaceFolders();
}

/**
 * @brief Returns the editor-owned group folder list.
 */
const std::vector<DataModel::GroupFolder>& DataModel::ProjectModel::editorGroupFolders()
  const noexcept
{
  return m_folders.groupFolders();
}

/**
 * @brief Returns the editor-owned table folder list.
 */
const std::vector<DataModel::TableFolder>& DataModel::ProjectModel::editorTableFolders()
  const noexcept
{
  return m_folders.tableFolders();
}

/**
 * @brief Returns the workspace list currently rendered by the dashboard.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectModel::activeWorkspaces() const
{
  return m_workspaces.activeList();
}

/**
 * @brief Returns the set of hidden auto-generated group IDs.
 */
const QSet<int>& DataModel::ProjectModel::hiddenGroupIds() const noexcept
{
  return m_workspaces.hiddenGroupIds();
}

/**
 * @brief Returns the number of workspaces defined in the project.
 */
int DataModel::ProjectModel::workspaceCount() const noexcept
{
  return m_workspaces.count();
}

/**
 * @brief Returns true when the auto-generated group workspace is hidden.
 */
bool DataModel::ProjectModel::isGroupHidden(int groupId) const
{
  return m_workspaces.isGroupHidden(groupId);
}

/**
 * @brief Returns the number of user-defined tables in the project.
 */
int DataModel::ProjectModel::tableCount() const noexcept
{
  return m_tables.count();
}

/**
 * @brief Returns the project's user-defined data table list.
 */
const std::vector<DataModel::TableDef>& DataModel::ProjectModel::tables() const noexcept
{
  return m_tables.list();
}

/**
 * @brief Returns whether the user has opted in to customising workspaces.
 */
bool DataModel::ProjectModel::customizeWorkspaces() const noexcept
{
  return m_workspaces.customizeWorkspaces();
}

//--------------------------------------------------------------------------------------------------
// Sink configuration
//--------------------------------------------------------------------------------------------------

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
 * @brief Returns the project's InfluxDB sink configuration (empty when unset, i.e. disabled).
 */
const QJsonObject& DataModel::ProjectModel::influxSink() const noexcept
{
  return m_influxSink;
}

/**
 * @brief Replaces the InfluxDB sink configuration and marks the project as modified. The API
 *        token is never part of @p config: it stays in the machine-bound credential vault.
 */
void DataModel::ProjectModel::setInfluxSink(const QJsonObject& config)
{
  if (m_influxSink == config)
    return;

  const ProjectUndoScope undo_scope{*this, tr("Change InfluxDB Sink")};

  m_influxSink = config;
  setModified(true);
  Q_EMIT influxSinkChanged();
}

/**
 * @brief Notifies the sink modules that a document reset cleared their configuration, each one
 *        only when it actually had one, so a fresh document does not churn every sink.
 */
void DataModel::ProjectModel::emitSinkConfigResets(bool hadMqttPublisher, bool hadInfluxSink)
{
  if (hadMqttPublisher)
    Q_EMIT mqttPublisherChanged();

  if (hadInfluxSink)
    Q_EMIT influxSinkChanged();
}

//--------------------------------------------------------------------------------------------------
// Presentation forwarding: widget settings, display titles, layouts, tree/diagram state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the active group ID for the dashboard tab bar, or -1.
 */
int DataModel::ProjectModel::activeGroupId() const
{
  return m_presentation.activeGroupId();
}

/**
 * @brief Stages the active dashboard tab group ID.
 */
void DataModel::ProjectModel::setActiveGroupId(const int groupId)
{
  m_presentation.setActiveGroupId(groupId);
}

/**
 * @brief Returns the persisted layout for the given group ID.
 */
QJsonObject DataModel::ProjectModel::groupLayout(int groupId) const
{
  return m_presentation.groupLayout(groupId);
}

/**
 * @brief Returns the persisted layout for the given group within a window scope.
 */
QJsonObject DataModel::ProjectModel::groupLayout(const QString& scope, int groupId) const
{
  return m_presentation.groupLayout(scope, groupId);
}

/**
 * @brief Returns the auto-layout pattern and split ratio for a group or workspace.
 */
QJsonObject DataModel::ProjectModel::layoutChoice(const QString& scope, int groupId) const
{
  return m_presentation.layoutChoice(scope, groupId);
}

/**
 * @brief Stores the auto-layout pattern and split ratio for a group or workspace.
 */
void DataModel::ProjectModel::setLayoutChoice(const QString& scope,
                                              int groupId,
                                              const QString& pattern,
                                              int ratio)
{
  m_presentation.setLayoutChoice(scope, groupId, pattern, ratio);
}

/**
 * @brief Stages the widget layout for a specific group.
 */
void DataModel::ProjectModel::setGroupLayout(const int groupId, const QJsonObject& layout)
{
  m_presentation.setGroupLayout(groupId, layout);
}

/**
 * @brief Returns the persisted external-window records (workspace, geometry, state).
 */
QJsonArray DataModel::ProjectModel::externalWindows() const
{
  return m_presentation.externalWindows();
}

/**
 * @brief Persists the external dashboard windows and prunes layouts of closed windows.
 */
void DataModel::ProjectModel::setExternalWindows(const QJsonArray& windows)
{
  m_presentation.setExternalWindows(windows);
}

/**
 * @brief Returns the persisted settings object for the given widget.
 */
QJsonObject DataModel::ProjectModel::widgetSettings(const QString& widgetId) const
{
  return m_presentation.widgetSettings(widgetId);
}

/**
 * @brief Stages a single widget setting and marks the project dirty.
 */
void DataModel::ProjectModel::saveWidgetSetting(const QString& widgetId,
                                                const QString& key,
                                                const QVariant& value)
{
  m_presentation.saveWidgetSetting(widgetId, key, value);
}

/**
 * @brief Returns the persisted state object for the given plugin.
 */
QJsonObject DataModel::ProjectModel::pluginState(const QString& pluginId) const
{
  return m_presentation.pluginState(pluginId);
}

/**
 * @brief Stages a plugin's state in the project and marks it dirty.
 */
void DataModel::ProjectModel::savePluginState(const QString& pluginId, const QJsonObject& state)
{
  m_presentation.savePluginState(pluginId, state);
}

/**
 * @brief Returns the entity-level display-title override for the given unique ID.
 */
QString DataModel::ProjectModel::displayTitle(int uniqueId) const
{
  return m_presentation.displayTitle(uniqueId);
}

/**
 * @brief Returns the widget-level display-title override for (widgetType, uniqueId).
 */
QString DataModel::ProjectModel::widgetDisplayTitle(int widgetType, int uniqueId) const
{
  return m_presentation.widgetDisplayTitle(widgetType, uniqueId);
}

/**
 * @brief Returns the freeze-title mode ("bar", "painted" or "hidden") for the given widget.
 */
QString DataModel::ProjectModel::freezeTitleMode(int widgetType, int uniqueId) const
{
  return m_presentation.freezeTitleMode(widgetType, uniqueId);
}

/**
 * @brief Returns the full display-title override map (uniqueId -> title).
 */
QJsonObject DataModel::ProjectModel::displayTitles() const
{
  return m_presentation.displayTitles();
}

/**
 * @brief Stages an entity-level display-title override.
 */
void DataModel::ProjectModel::setDisplayTitle(int uniqueId, const QString& title)
{
  m_presentation.setDisplayTitle(uniqueId, title);
}

/**
 * @brief Stages a widget-level display-title override for (widgetType, uniqueId).
 */
void DataModel::ProjectModel::setWidgetDisplayTitle(int widgetType,
                                                    int uniqueId,
                                                    const QString& title)
{
  m_presentation.setWidgetDisplayTitle(widgetType, uniqueId, title);
}

/**
 * @brief Prompts for a widget display title and stages it as a widget-level override.
 */
void DataModel::ProjectModel::promptRenameWidget(int widgetType,
                                                 int uniqueId,
                                                 const QString& currentTitle)
{
  m_presentation.promptRenameWidget(widgetType, uniqueId, currentTitle);
}

/**
 * @brief Stages the freeze-title mode for the given widget.
 */
void DataModel::ProjectModel::setFreezeTitleMode(int widgetType, int uniqueId, const QString& mode)
{
  m_presentation.setFreezeTitleMode(widgetType, uniqueId, mode);
}

/**
 * @brief Path-keyed Project Editor tree node expansion map (persisted in the file).
 */
const QJsonObject& DataModel::ProjectModel::treeExpansion() const noexcept
{
  return m_presentation.treeExpansion();
}

/**
 * @brief Stores the editor tree expansion map, marking the project dirty when it changed.
 */
void DataModel::ProjectModel::setTreeExpansion(const QJsonObject& expansion)
{
  m_presentation.setTreeExpansion(expansion);
}

/**
 * @brief Stable-id keyed Project Overview diagram node collapse map (persisted in the file).
 */
const QJsonObject& DataModel::ProjectModel::diagramCollapse() const noexcept
{
  return m_presentation.diagramCollapse();
}

/**
 * @brief Stores the diagram collapse map, marking the project dirty when it changed.
 */
void DataModel::ProjectModel::setDiagramCollapse(const QJsonObject& state)
{
  m_presentation.setDiagramCollapse(state);
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

    if (!m_persistence.writeProjectFile(m_filePath))
      return;

    Q_EMIT pointCountChanged();
  });

  connect(&UI::Dashboard::instance(), &UI::Dashboard::widgetCountChanged, this, [this] {
    if (AppState::instance().operationMode() != SerialStudio::QuickPlot)
      return;

    m_workspaces.rebuildSessionWorkspaces();
    Q_EMIT activeWorkspacesChanged();
  });

  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    const auto opMode = AppState::instance().operationMode();
    if (opMode == SerialStudio::ProjectFile)
      m_workspaces.clearSessionWorkspaces();
    else
      m_workspaces.rebuildSessionWorkspaces();

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
        m_workspaces.rebuildSessionWorkspaces();
        Q_EMIT activeWorkspacesChanged();
        return;
      }

      if (m_workspaces.customizeWorkspaces()) {
        if (m_workspaces.mergeAutoWorkspaceUpdates()) {
          Q_EMIT editorWorkspacesChanged();
          Q_EMIT activeWorkspacesChanged();
        }

        return;
      }

      m_workspaces.regenerateAutoWorkspacesUnnotified();
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
  m_workspaces.resetDocument();
  m_tables.clear();
  m_folders.mutableGroupFolders().clear();
  m_folders.mutableTableFolders().clear();

  const bool hadMqttPublisher = !m_mqttPublisher.isEmpty();
  const bool hadInfluxSink    = !m_influxSink.isEmpty();
  m_mqttPublisher             = QJsonObject();
  m_influxSink                = QJsonObject();

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
  m_luaFastMode              = false;
  m_nextUniqueId             = 1;
  m_controlScriptCode        = "";
  static auto& controlScript = DataModel::ControlScript::instance();
  controlScript.setCode(m_controlScriptCode);
  m_frameDecoder   = SerialStudio::PlainText;
  m_frameDetection = SerialStudio::EndDelimiterOnly;
  m_presentation.resetDocument();

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
  ProjectSources::seedDefaultFrameParser(defaultSource);
  m_sources.push_back(defaultSource);

  m_filePath = "";
  m_persistence.watchProjectFile();

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
  Q_EMIT luaFastModeChanged();

  if (wasLocked)
    Q_EMIT lockedChanged();

  emitSinkConfigResets(hadMqttPublisher, hadInfluxSink);

  if (!m_silentReload)
    Q_EMIT sourceStructureChanged();

  m_history.clear();
  Q_EMIT projectHistoryChanged();
  setModified(false);
}

/**
 * @brief Clears workspaces and widget settings for QuickPlot/ConsoleOnly sessions that have no
 * backing project file.
 */
void DataModel::ProjectModel::clearTransientState()
{
  static auto& appState = AppState::instance();
  const auto opMode     = appState.operationMode();
  if (opMode == SerialStudio::ProjectFile || !m_filePath.isEmpty()
      || m_workspaces.customizeWorkspaces())
    return;

  if (m_workspaces.clearTransientState()) {
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
  }

  if (m_presentation.clearWidgetSettings())
    Q_EMIT widgetSettingsChanged();

  if (!m_mqttPublisher.isEmpty()) {
    m_mqttPublisher = QJsonObject();
    Q_EMIT mqttPublisherChanged();
  }

  if (!m_influxSink.isEmpty()) {
    m_influxSink = QJsonObject();
    Q_EMIT influxSinkChanged();
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

//--------------------------------------------------------------------------------------------------
// Scalar property setters
//--------------------------------------------------------------------------------------------------

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
 * @brief Toggles Fast Lua execution (spec 0051 R20): one mode switch, because JIT-compiled
 *        traces never fire the count hook -- a "watchdog" alongside an active JIT would be a
 *        silently dead safety control. Engines re-read the flag on their next (re)compile.
 */
void DataModel::ProjectModel::setLuaFastMode(const bool enabled)
{
  if (m_luaFastMode == enabled)
    return;

  const ProjectUndoScope undo_scope{*this, tr("Toggle Fast Lua Execution")};

  m_luaFastMode = enabled;
  setModified(true);
  Q_EMIT luaFastModeChanged();
}

/**
 * @brief UI entry point for the Fast-mode toggle: enabling walks through the native consent
 *        box that names the traded-away watchdog (spec 0051 R20) before the property flips.
 *        Programmatic callers (API handlers, tests) use setLuaFastMode directly and never see
 *        a modal.
 */
void DataModel::ProjectModel::requestLuaFastMode(const bool enabled)
{
  if (!enabled || m_luaFastMode == enabled) {
    setLuaFastMode(enabled);
    return;
  }

  const int choice = Misc::Utilities::showMessageBox(
    tr("Enable Fast Lua Execution?"),
    tr("Fast mode runs Lua parsers and transforms through the JIT compiler (up to ~40x "
       "faster), but the runaway-script watchdog cannot operate: a script stuck in an "
       "infinite loop will stall its data source until you disconnect.\n\n"
       "Enable it only for scripts you trust and have tested in Safe mode first."),
    QMessageBox::Warning,
    tr("Fast Lua Execution"),
    QMessageBox::Ok | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Ok)
    setLuaFastMode(true);
  else
    Q_EMIT luaFastModeChanged();
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

  if (modified && m_groups.empty() && m_actions.empty() && m_tables.list().empty()
      && m_workspaces.list().empty() && !m_workspaces.customizeWorkspaces() && !m_locked
      && m_workspaces.hiddenGroupIds().isEmpty()) {
    Q_EMIT contentTouched();
    return;
  }

  m_modified = modified;
  Q_EMIT modifiedChanged();
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
 * @brief Stores the editor's currently selected output widget.
 */
void DataModel::ProjectModel::setSelectedOutputWidget(const DataModel::OutputWidget& widget)
{
  m_selectedOutputWidget = widget;
}

//--------------------------------------------------------------------------------------------------
// QML diagram snapshots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a snapshot of all sources suitable for QML diagram consumption.
 */
QVariantList DataModel::ProjectModel::sourcesForDiagram() const
{
  return m_presentation.sourcesForDiagram();
}

/**
 * @brief Returns a snapshot of all groups (with their datasets) for QML diagram consumption.
 */
QVariantList DataModel::ProjectModel::groupsForDiagram() const
{
  return m_presentation.groupsForDiagram();
}

/**
 * @brief Returns a snapshot of all actions suitable for QML diagram consumption.
 */
QVariantList DataModel::ProjectModel::actionsForDiagram() const
{
  return m_presentation.actionsForDiagram();
}

/**
 * @brief Returns a snapshot of project data tables (name + register count) for the diagram.
 */
QVariantList DataModel::ProjectModel::tablesForDiagram() const
{
  return m_presentation.tablesForDiagram();
}

//--------------------------------------------------------------------------------------------------
// Persistence forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the complete project state to a QJsonObject.
 */
QJsonObject DataModel::ProjectModel::serializeToJson() const
{
  return m_persistence.serializeToJson();
}

/**
 * @brief Prompts to save changes, returning false only on cancel.
 */
bool DataModel::ProjectModel::askSave()
{
  return m_persistence.askSave();
}

/**
 * @brief Validates and saves the project, optionally prompting for a path.
 */
bool DataModel::ProjectModel::saveJsonFile(const bool askPath)
{
  return m_persistence.saveJsonFile(askPath);
}

/**
 * @brief Headless save to the given path (no file dialog).
 */
bool DataModel::ProjectModel::apiSaveJsonFile(const QString& path)
{
  return m_persistence.apiSaveJsonFile(path);
}

/**
 * @brief Flushes any pending debounced autosave synchronously (called on app quit).
 */
void DataModel::ProjectModel::flushAutoSave()
{
  m_persistence.flushAutoSave();
}

/**
 * @brief Starts the debounced autosave when saving is currently permitted.
 */
void DataModel::ProjectModel::scheduleAutoSave()
{
  m_persistence.scheduleAutoSave();
}

/**
 * @brief Suspends or resumes the debounced autosave (used by the API batch endpoint).
 */
void DataModel::ProjectModel::setAutoSaveSuspended(bool suspend)
{
  m_persistence.setAutoSaveSuspended(suspend);
}

//--------------------------------------------------------------------------------------------------
// Loading forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shows a file-open dialog and loads the selected project.
 */
void DataModel::ProjectModel::openJsonFile()
{
  m_loader.openJsonFile();
}

/**
 * @brief Loads a project from the given .ssproj/.json path.
 */
bool DataModel::ProjectModel::openJsonFile(const QString& path)
{
  return m_loader.openJsonFile(path);
}

/**
 * @brief Deserialises a project from an in-memory QJsonDocument.
 */
bool DataModel::ProjectModel::loadFromJsonDocument(const QJsonDocument& document,
                                                   const QString& sourcePath)
{
  return m_loader.loadFromJsonDocument(document, sourcePath);
}

/**
 * @brief Prompts for a save path, writes the imported project, then opens it.
 */
void DataModel::ProjectModel::importProjectFromJson(const QJsonObject& project,
                                                    const QString& suggestedFileName)
{
  m_loader.importProjectFromJson(project, suggestedFileName);
}

//--------------------------------------------------------------------------------------------------
// Entity forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fills every empty dataset alias from its title; returns the number seeded.
 */
int DataModel::ProjectModel::seedDatasetAliases()
{
  return m_entities.seedDatasetAliases();
}

/**
 * @brief Replaces the group at groupId and emits groupsChanged.
 */
void DataModel::ProjectModel::updateGroup(const int groupId,
                                          const DataModel::Group& group,
                                          const bool rebuildTree)
{
  m_entities.updateGroup(groupId, group, rebuildTree);
}

/**
 * @brief Replaces the action at actionId and emits actionsChanged.
 */
void DataModel::ProjectModel::updateAction(const int actionId,
                                           const DataModel::Action& action,
                                           const bool rebuildTree)
{
  m_entities.updateAction(actionId, action, rebuildTree);
}

/**
 * @brief Replaces the dataset at (groupId, datasetId).
 */
void DataModel::ProjectModel::updateDataset(const int groupId,
                                            const int datasetId,
                                            const DataModel::Dataset& dataset,
                                            const bool rebuildTree)
{
  m_entities.updateDataset(groupId, datasetId, dataset, rebuildTree);
}

/**
 * @brief Enables or disables a group and refreshes the runtime frame.
 */
void DataModel::ProjectModel::setGroupEnabled(int groupId, bool enabled)
{
  m_entities.setGroupEnabled(groupId, enabled);
}

/**
 * @brief Enables or disables a single dataset and refreshes the runtime frame.
 */
void DataModel::ProjectModel::setDatasetEnabled(int groupId, int datasetId, bool enabled)
{
  m_entities.setDatasetEnabled(groupId, datasetId, enabled);
}

/**
 * @brief Deletes the currently selected group after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentGroup()
{
  m_entities.deleteCurrentGroup();
}

/**
 * @brief Deletes the currently selected action after user confirmation.
 */
void DataModel::ProjectModel::deleteCurrentAction()
{
  m_entities.deleteCurrentAction();
}

/**
 * @brief Deletes the selected dataset, removing the group if it becomes empty.
 */
void DataModel::ProjectModel::deleteCurrentDataset()
{
  m_entities.deleteCurrentDataset();
}

/**
 * @brief Appends a copy of the currently selected group to the project.
 */
void DataModel::ProjectModel::duplicateCurrentGroup()
{
  m_entities.duplicateCurrentGroup();
}

/**
 * @brief Appends a copy of the currently selected action to the project.
 */
void DataModel::ProjectModel::duplicateCurrentAction()
{
  m_entities.duplicateCurrentAction();
}

/**
 * @brief Appends a copy of the currently selected dataset to its parent group.
 */
void DataModel::ProjectModel::duplicateCurrentDataset()
{
  m_entities.duplicateCurrentDataset();
}

/**
 * @brief Deletes the group at @p groupId.
 */
void DataModel::ProjectModel::deleteGroup(int groupId, bool confirm)
{
  m_entities.deleteGroup(groupId, confirm);
}

/**
 * @brief Duplicates the group at @p groupId.
 */
void DataModel::ProjectModel::duplicateGroup(int groupId)
{
  m_entities.duplicateGroup(groupId);
}

/**
 * @brief Deletes the dataset at (groupId, datasetId).
 */
void DataModel::ProjectModel::deleteDataset(int groupId, int datasetId, bool confirm)
{
  m_entities.deleteDataset(groupId, datasetId, confirm);
}

/**
 * @brief Duplicates the dataset at (groupId, datasetId).
 */
void DataModel::ProjectModel::duplicateDataset(int groupId, int datasetId)
{
  m_entities.duplicateDataset(groupId, datasetId);
}

/**
 * @brief Deletes the action at @p actionId.
 */
void DataModel::ProjectModel::deleteAction(int actionId, bool confirm)
{
  m_entities.deleteAction(actionId, confirm);
}

/**
 * @brief Duplicates the action at @p actionId.
 */
void DataModel::ProjectModel::duplicateAction(int actionId)
{
  m_entities.duplicateAction(actionId);
}

/**
 * @brief Moves a group from one position to another.
 */
void DataModel::ProjectModel::moveGroup(int fromGroupId, int toGroupId)
{
  m_entities.moveGroup(fromGroupId, toGroupId);
}

/**
 * @brief Moves a dataset within its group.
 */
void DataModel::ProjectModel::moveDataset(int groupId, int fromDatasetId, int toDatasetId)
{
  m_entities.moveDataset(groupId, fromDatasetId, toDatasetId);
}

/**
 * @brief Moves an action within the project actions list.
 */
void DataModel::ProjectModel::moveAction(int fromActionId, int toActionId)
{
  m_entities.moveAction(fromActionId, toActionId);
}

/**
 * @brief Ensures a compatible group is selected before adding a dataset.
 */
void DataModel::ProjectModel::ensureValidGroup(int sourceId)
{
  m_entities.ensureValidGroup(sourceId);
}

/**
 * @brief Adds a new dataset of the given type to the selected group.
 */
void DataModel::ProjectModel::addDataset(const SerialStudio::DatasetOption options, int sourceId)
{
  m_entities.addDataset(options, sourceId);
}

/**
 * @brief Appends template-defined datasets to a canvas group.
 */
void DataModel::ProjectModel::ensurePainterDatasets(int groupId, const QVariantList& specs)
{
  m_entities.ensurePainterDatasets(groupId, specs);
}

/**
 * @brief Toggles a dataset option flag on the currently selected dataset.
 */
void DataModel::ProjectModel::changeDatasetOption(const SerialStudio::DatasetOption option,
                                                  const bool checked)
{
  m_entities.changeDatasetOption(option, checked);
}

/**
 * @brief Adds a new action with a unique title to the project.
 */
void DataModel::ProjectModel::addAction(int sourceId)
{
  m_entities.addAction(sourceId);
}

/**
 * @brief Adds a new group with a unique title and the given widget type.
 */
void DataModel::ProjectModel::addGroup(const QString& title,
                                       const SerialStudio::GroupWidget widget,
                                       int sourceId,
                                       int parentFolderId)
{
  m_entities.addGroup(title, widget, sourceId, parentFolderId);
}

/**
 * @brief Assigns a widget type to the group, replacing fixed-layout datasets.
 */
bool DataModel::ProjectModel::setGroupWidget(const int group,
                                             const SerialStudio::GroupWidget widget)
{
  return m_entities.setGroupWidget(group, widget);
}

/**
 * @brief Prompts for a new title and applies it to the group at @p groupId.
 */
void DataModel::ProjectModel::promptRenameGroup(int groupId)
{
  m_entities.promptRenameGroup(groupId);
}

/**
 * @brief Prompts for a new title and applies it to the dataset at (groupId, datasetId).
 */
void DataModel::ProjectModel::promptRenameDataset(int groupId, int datasetId)
{
  m_entities.promptRenameDataset(groupId, datasetId);
}

/**
 * @brief Prompts for a new title and applies it to the action at @p actionId.
 */
void DataModel::ProjectModel::promptRenameAction(int actionId)
{
  m_entities.promptRenameAction(actionId);
}

//--------------------------------------------------------------------------------------------------
// Source forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a new source to the project (GPL: capped to one source).
 */
void DataModel::ProjectModel::addSource()
{
  m_sourceOps.addSource();
}

/**
 * @brief Deletes the source and reassigns dependent groups to source 0.
 */
void DataModel::ProjectModel::deleteSource(int sourceId, bool confirm)
{
  m_sourceOps.deleteSource(sourceId, confirm);
}

/**
 * @brief Duplicates the source with the given @p sourceId.
 */
void DataModel::ProjectModel::duplicateSource(int sourceId)
{
  m_sourceOps.duplicateSource(sourceId);
}

/**
 * @brief Updates the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSource(int sourceId,
                                           const DataModel::Source& source,
                                           bool rebuildTree)
{
  m_sourceOps.updateSource(sourceId, source, rebuildTree);
}

/**
 * @brief Updates the title of the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSourceTitle(int sourceId,
                                                const QString& title,
                                                bool rebuildTree)
{
  m_sourceOps.updateSourceTitle(sourceId, title, rebuildTree);
}

/**
 * @brief Updates the bus type of the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSourceBusType(int sourceId, int busType)
{
  m_sourceOps.updateSourceBusType(sourceId, busType);
}

/**
 * @brief Prompts for a new title and applies it to the source at @p sourceId.
 */
void DataModel::ProjectModel::promptRenameSource(int sourceId)
{
  m_sourceOps.promptRenameSource(sourceId);
}

/**
 * @brief Updates the per-source frame parser code and reloads the engine.
 */
void DataModel::ProjectModel::updateSourceFrameParser(int sourceId, const QString& code)
{
  m_sourceOps.updateSourceFrameParser(sourceId, code);
}

/**
 * @brief Snapshots the current driver settings for source @p sourceId.
 */
void DataModel::ProjectModel::captureSourceSettings(int sourceId)
{
  m_sourceOps.captureSourceSettings(sourceId);
}

/**
 * @brief Applies the source's saved connectionSettings to its live driver.
 */
void DataModel::ProjectModel::restoreSourceSettings(int sourceId)
{
  m_sourceOps.restoreSourceSettings(sourceId);
}

/**
 * @brief Sets source[0].busType without emitting sourceStructureChanged.
 */
void DataModel::ProjectModel::setSource0BusType(int busType)
{
  m_sourceOps.setSource0BusType(busType);
}

/**
 * @brief Overwrites source[0].connectionSettings without emitting sourcesChanged.
 */
void DataModel::ProjectModel::setSource0ConnectionSettings(const QJsonObject& settings)
{
  m_sourceOps.setSource0ConnectionSettings(settings);
}

/**
 * @brief Sets source[0].frameParserCode and emits frameParserCodeChanged.
 */
void DataModel::ProjectModel::setFrameParserCode(const QString& code)
{
  m_sourceOps.setFrameParserCode(code);
}

/**
 * @brief Sets the scripting language for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserLanguage(int language)
{
  m_sourceOps.setFrameParserLanguage(language);
}

/**
 * @brief Sets the scripting language for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserLanguage(int sourceId, int language)
{
  m_sourceOps.updateSourceFrameParserLanguage(sourceId, language);
}

/**
 * @brief Stores frame parser code without emitting signals or reloading the engine.
 */
void DataModel::ProjectModel::storeFrameParserCode(int sourceId, const QString& code)
{
  m_sourceOps.storeFrameParserCode(sourceId, code);
}

/**
 * @brief Sets the native parser template id for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserTemplate(const QString& templateId)
{
  m_sourceOps.setFrameParserTemplate(templateId);
}

/**
 * @brief Sets the native parser template params for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserParams(const QJsonObject& params)
{
  m_sourceOps.setFrameParserParams(params);
}

/**
 * @brief Sets the native parser template id for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserTemplate(int sourceId,
                                                              const QString& templateId)
{
  m_sourceOps.updateSourceFrameParserTemplate(sourceId, templateId);
}

/**
 * @brief Sets the native parser template params for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserParams(int sourceId, const QJsonObject& params)
{
  m_sourceOps.updateSourceFrameParserParams(sourceId, params);
}

/**
 * @brief Sets the stream-lane override for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceStreamLane(int sourceId, const QString& lane)
{
  m_sourceOps.updateSourceStreamLane(sourceId, lane);
}

//--------------------------------------------------------------------------------------------------
// Output widget forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds an output control, creating a new output group if needed.
 */
void DataModel::ProjectModel::addOutputControl(const SerialStudio::OutputWidgetType type,
                                               int sourceId)
{
  m_outputWidgets.addOutputControl(type, sourceId);
}

/**
 * @brief Creates a new output group with a default button control.
 */
void DataModel::ProjectModel::addOutputPanel(int sourceId)
{
  m_outputWidgets.addOutputPanel(sourceId);
}

/**
 * @brief Changes the type of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetType(int type)
{
  m_outputWidgets.setOutputWidgetType(type);
}

/**
 * @brief Sets the icon of the currently selected output widget.
 */
void DataModel::ProjectModel::setOutputWidgetIcon(const QString& icon)
{
  m_outputWidgets.setOutputWidgetIcon(icon);
}

/**
 * @brief Deletes the currently selected output widget after confirmation.
 */
void DataModel::ProjectModel::deleteCurrentOutputWidget()
{
  m_outputWidgets.deleteCurrentOutputWidget();
}

/**
 * @brief Duplicates the currently selected output widget.
 */
void DataModel::ProjectModel::duplicateCurrentOutputWidget()
{
  m_outputWidgets.duplicateCurrentOutputWidget();
}

/**
 * @brief Deletes the output widget at (groupId, widgetId).
 */
void DataModel::ProjectModel::deleteOutputWidget(int groupId, int widgetId, bool confirm)
{
  m_outputWidgets.deleteOutputWidget(groupId, widgetId, confirm);
}

/**
 * @brief Duplicates the output widget at (groupId, widgetId).
 */
void DataModel::ProjectModel::duplicateOutputWidget(int groupId, int widgetId)
{
  m_outputWidgets.duplicateOutputWidget(groupId, widgetId);
}

/**
 * @brief Updates an output widget in place.
 */
void DataModel::ProjectModel::updateOutputWidget(int groupId,
                                                 int widgetId,
                                                 const DataModel::OutputWidget& widget,
                                                 bool rebuildTree)
{
  m_outputWidgets.updateOutputWidget(groupId, widgetId, widget, rebuildTree);
}

/**
 * @brief Moves an output widget within its group's outputWidgets list.
 */
void DataModel::ProjectModel::moveOutputWidget(int groupId, int fromWidgetId, int toWidgetId)
{
  m_outputWidgets.moveOutputWidget(groupId, fromWidgetId, toWidgetId);
}

//--------------------------------------------------------------------------------------------------
// Table forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a new empty data table; returns its full folder-qualified path.
 */
QString DataModel::ProjectModel::addTable(const QString& name, int parentFolderId)
{
  return m_tables.addTable(name, parentFolderId);
}

/**
 * @brief Returns the register list of a table as a QVariantList for QML.
 */
QVariantList DataModel::ProjectModel::registersForTable(const QString& table) const
{
  return m_tables.registersForTable(table);
}

/**
 * @brief Deletes the table with the given full path.
 */
void DataModel::ProjectModel::deleteTable(const QString& name)
{
  m_tables.deleteTable(name);
}

/**
 * @brief Renames a table's leaf name.
 */
void DataModel::ProjectModel::renameTable(const QString& oldName, const QString& newName)
{
  m_tables.renameTable(oldName, newName);
}

/**
 * @brief Appends a register to @p table with a unique name.
 */
void DataModel::ProjectModel::addRegister(const QString& table,
                                          const QString& registerName,
                                          bool computed,
                                          const QVariant& defaultValue)
{
  m_tables.addRegister(table, registerName, computed, defaultValue);
}

/**
 * @brief Removes a register from the specified table.
 */
void DataModel::ProjectModel::deleteRegister(const QString& table, const QString& registerName)
{
  m_tables.deleteRegister(table, registerName);
}

/**
 * @brief Updates an existing register -- rename, retype, and/or default value.
 */
bool DataModel::ProjectModel::updateRegister(const QString& table,
                                             const QString& registerName,
                                             const QString& newName,
                                             bool computed,
                                             const QVariant& defaultValue)
{
  return m_tables.updateRegister(table, registerName, newName, computed, defaultValue);
}

/**
 * @brief Prompts for a new shared table name and appends it on accept.
 */
void DataModel::ProjectModel::promptAddTable()
{
  m_tables.promptAddTable();
}

/**
 * @brief Prompts for a new name for an existing table.
 */
void DataModel::ProjectModel::promptRenameTable(const QString& oldName)
{
  m_tables.promptRenameTable(oldName);
}

/**
 * @brief Prompts for a register name and type, then appends the register.
 */
void DataModel::ProjectModel::promptAddRegister(const QString& table)
{
  m_tables.promptAddRegister(table);
}

/**
 * @brief Prompts for a new name for an existing register.
 */
void DataModel::ProjectModel::promptRenameRegister(const QString& table,
                                                   const QString& registerName)
{
  m_tables.promptRenameRegister(table, registerName);
}

/**
 * @brief Asks the user to confirm before deleting a table.
 */
void DataModel::ProjectModel::confirmDeleteTable(const QString& name)
{
  m_tables.confirmDeleteTable(name);
}

/**
 * @brief Asks the user to confirm before deleting a register.
 */
void DataModel::ProjectModel::confirmDeleteRegister(const QString& table,
                                                    const QString& registerName)
{
  m_tables.confirmDeleteRegister(table, registerName);
}

/**
 * @brief Imports registers from a CSV file into an existing table.
 */
void DataModel::ProjectModel::importTableFromCsv(const QString& tableName)
{
  m_tables.importTableFromCsv(tableName);
}

/**
 * @brief Exports a table's registers to a CSV file chosen by the user.
 */
void DataModel::ProjectModel::exportTableToCsv(const QString& tableName)
{
  m_tables.exportTableToCsv(tableName);
}

//--------------------------------------------------------------------------------------------------
// Workspace forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flips the customize switch (Off->On seeds auto layout, On->Off re-seeds it).
 */
void DataModel::ProjectModel::setCustomizeWorkspaces(const bool enabled)
{
  m_workspaces.setCustomizeWorkspaces(enabled);
}

/**
 * @brief Creates a new user-defined workspace with the given title.
 */
int DataModel::ProjectModel::addWorkspace(const QString& title)
{
  return m_workspaces.addWorkspace(title);
}

/**
 * @brief Materialises the synthetic workspace list and flips into customize mode.
 */
int DataModel::ProjectModel::autoGenerateWorkspaces()
{
  return m_workspaces.autoGenerateWorkspaces();
}

/**
 * @brief Returns the title of a workspace, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceTitle(int workspaceId) const
{
  return m_workspaces.workspaceTitle(workspaceId);
}

/**
 * @brief Returns the icon of a workspace, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceIcon(int workspaceId) const
{
  return m_workspaces.workspaceIcon(workspaceId);
}

/**
 * @brief Returns {id, title} entries for every hidden auto-group, in group order.
 */
QVariantList DataModel::ProjectModel::hiddenGroupsSummary() const
{
  return m_workspaces.hiddenGroupsSummary();
}

/**
 * @brief Drops user customisations and returns the project to the auto layout.
 */
void DataModel::ProjectModel::resetWorkspacesToAuto()
{
  m_workspaces.resetWorkspacesToAuto();
}

/**
 * @brief Asks the user to confirm before discarding workspace customisations.
 */
void DataModel::ProjectModel::confirmResetWorkspacesToAuto()
{
  m_workspaces.confirmResetWorkspacesToAuto();
}

/**
 * @brief Restores every hidden auto-group in one shot.
 */
void DataModel::ProjectModel::showAllHiddenGroups()
{
  m_workspaces.showAllHiddenGroups();
}

/**
 * @brief Deletes the workspace with the given ID.
 */
void DataModel::ProjectModel::deleteWorkspace(int workspaceId)
{
  m_workspaces.deleteWorkspace(workspaceId);
}

/**
 * @brief Wipes every workspace, leaving an empty customised list.
 */
void DataModel::ProjectModel::clearAllWorkspaces()
{
  m_workspaces.clearAllWorkspaces();
}

/**
 * @brief Renames the workspace with the given ID.
 */
void DataModel::ProjectModel::renameWorkspace(int workspaceId, const QString& title)
{
  m_workspaces.renameWorkspace(workspaceId, title);
}

/**
 * @brief Patches title, icon, and/or description on the workspace with the given ID.
 */
void DataModel::ProjectModel::updateWorkspace(int workspaceId,
                                              const QString& title,
                                              const QString& icon,
                                              const QString& description,
                                              bool setTitle,
                                              bool setIcon,
                                              bool setDescription)
{
  m_workspaces.updateWorkspace(
    workspaceId, title, icon, description, setTitle, setIcon, setDescription);
}

/**
 * @brief Convenience slot that sets only the icon of a workspace.
 */
void DataModel::ProjectModel::setWorkspaceIcon(int workspaceId, const QString& icon)
{
  m_workspaces.setWorkspaceIcon(workspaceId, icon);
}

/**
 * @brief Reorders user-defined workspaces by the given id sequence.
 */
void DataModel::ProjectModel::reorderWorkspaces(const QList<int>& userWorkspaceIds)
{
  m_workspaces.reorderWorkspaces(userWorkspaceIds);
}

/**
 * @brief Moves a workspace to the given index in the editor list.
 */
void DataModel::ProjectModel::moveWorkspace(int workspaceId, int targetIndex)
{
  m_workspaces.moveWorkspace(workspaceId, targetIndex);
}

/**
 * @brief Appends a widget reference to the specified workspace.
 */
void DataModel::ProjectModel::addWidgetToWorkspace(int workspaceId,
                                                   int widgetType,
                                                   int groupUniqueId,
                                                   int relativeIndex)
{
  m_workspaces.addWidgetToWorkspace(workspaceId, widgetType, groupUniqueId, relativeIndex);
}

/**
 * @brief Removes a widget reference matching (widgetType, groupId, relativeIndex).
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId,
                                                        int widgetType,
                                                        int groupUniqueId,
                                                        int relativeIndex)
{
  m_workspaces.removeWidgetFromWorkspace(workspaceId, widgetType, groupUniqueId, relativeIndex);
}

/**
 * @brief Removes a widget reference from the specified workspace by index.
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId, int index)
{
  m_workspaces.removeWidgetFromWorkspace(workspaceId, index);
}

/**
 * @brief Drops every workspace widget ref whose encoded key isn't in validKeys.
 */
int DataModel::ProjectModel::cleanupWorkspaceWidgetRefs(const QSet<qint64>& validKeys)
{
  return m_workspaces.cleanupWorkspaceWidgetRefs(validKeys);
}

/**
 * @brief Prompts for a new workspace name and creates it.
 */
void DataModel::ProjectModel::promptAddWorkspace()
{
  m_workspaces.promptAddWorkspace();
}

/**
 * @brief Prompts for a new title for the given workspace.
 */
void DataModel::ProjectModel::promptRenameWorkspace(int workspaceId)
{
  m_workspaces.promptRenameWorkspace(workspaceId);
}

/**
 * @brief Asks the user to confirm before deleting a workspace.
 */
void DataModel::ProjectModel::confirmDeleteWorkspace(int workspaceId)
{
  m_workspaces.confirmDeleteWorkspace(workspaceId);
}

/**
 * @brief Hides an auto-generated group workspace from the workspace list.
 */
void DataModel::ProjectModel::hideGroup(int groupId)
{
  m_workspaces.hideGroup(groupId);
}

/**
 * @brief Restores a previously hidden auto-generated group workspace.
 */
void DataModel::ProjectModel::showGroup(int groupId)
{
  m_workspaces.showGroup(groupId);
}

//--------------------------------------------------------------------------------------------------
// Folder forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a workspace folder under @p parentFolderId; returns its new id.
 */
int DataModel::ProjectModel::addWorkspaceFolder(int parentFolderId, const QString& title)
{
  return m_folders.addWorkspaceFolder(parentFolderId, title);
}

/**
 * @brief Returns the title of a workspace folder, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceFolderTitle(int folderId) const
{
  return m_folders.workspaceFolderTitle(folderId);
}

/**
 * @brief Renames a workspace folder; ignores empty names.
 */
void DataModel::ProjectModel::renameWorkspaceFolder(int folderId, const QString& title)
{
  m_folders.renameWorkspaceFolder(folderId, title);
}

/**
 * @brief Deletes a workspace folder, promoting its contents to its parent.
 */
void DataModel::ProjectModel::deleteWorkspaceFolder(int folderId)
{
  m_folders.deleteWorkspaceFolder(folderId);
}

/**
 * @brief Files a workspace into folder @p parentFolderId (-1 = top level).
 */
void DataModel::ProjectModel::moveWorkspaceToFolder(int workspaceId, int parentFolderId)
{
  m_folders.moveWorkspaceToFolder(workspaceId, parentFolderId);
}

/**
 * @brief Re-parents a workspace folder, rejecting a cyclic move.
 */
void DataModel::ProjectModel::moveFolderToFolder(int folderId, int parentFolderId)
{
  m_folders.moveFolderToFolder(folderId, parentFolderId);
}

/**
 * @brief Reorders a workspace among its siblings in the same folder.
 */
void DataModel::ProjectModel::moveWorkspaceInFolder(int workspaceId, int direction)
{
  m_folders.moveWorkspaceInFolder(workspaceId, direction);
}

/**
 * @brief Reorders a workspace folder among its sibling folders.
 */
void DataModel::ProjectModel::moveWorkspaceFolderInParent(int folderId, int direction)
{
  m_folders.moveWorkspaceFolderInParent(folderId, direction);
}

/**
 * @brief Prompts for a name and creates a workspace folder under @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddWorkspaceFolder(int parentFolderId)
{
  m_folders.promptAddWorkspaceFolder(parentFolderId);
}

/**
 * @brief Prompts for a name, creates a workspace, and files it into @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddWorkspaceInFolder(int parentFolderId)
{
  m_folders.promptAddWorkspaceInFolder(parentFolderId);
}

/**
 * @brief Prompts for a new title for the given workspace folder.
 */
void DataModel::ProjectModel::promptRenameWorkspaceFolder(int folderId)
{
  m_folders.promptRenameWorkspaceFolder(folderId);
}

/**
 * @brief Confirms before deleting a workspace folder.
 */
void DataModel::ProjectModel::confirmDeleteWorkspaceFolder(int folderId)
{
  m_folders.confirmDeleteWorkspaceFolder(folderId);
}

/**
 * @brief Adds a group folder under @p parentFolderId; returns its new id.
 */
int DataModel::ProjectModel::addGroupFolder(int parentFolderId, const QString& title)
{
  return m_folders.addGroupFolder(parentFolderId, title);
}

/**
 * @brief Returns the title of a group folder, or empty if not found.
 */
QString DataModel::ProjectModel::groupFolderTitle(int folderId) const
{
  return m_folders.groupFolderTitle(folderId);
}

/**
 * @brief Renames a group folder; ignores empty names.
 */
void DataModel::ProjectModel::renameGroupFolder(int folderId, const QString& title)
{
  m_folders.renameGroupFolder(folderId, title);
}

/**
 * @brief Deletes a group folder, promoting its contents to its parent.
 */
void DataModel::ProjectModel::deleteGroupFolder(int folderId)
{
  m_folders.deleteGroupFolder(folderId);
}

/**
 * @brief Files a group into folder @p parentFolderId (-1 = top level).
 */
void DataModel::ProjectModel::moveGroupToFolder(int groupId, int parentFolderId)
{
  m_folders.moveGroupToFolder(groupId, parentFolderId);
}

/**
 * @brief Re-parents a group folder, rejecting a cyclic move.
 */
void DataModel::ProjectModel::moveGroupFolderToFolder(int folderId, int parentFolderId)
{
  m_folders.moveGroupFolderToFolder(folderId, parentFolderId);
}

/**
 * @brief Reorders a group folder among its sibling folders.
 */
void DataModel::ProjectModel::moveGroupFolderInParent(int folderId, int direction)
{
  m_folders.moveGroupFolderInParent(folderId, direction);
}

/**
 * @brief Prompts for a name and creates a group folder under @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddGroupFolder(int parentFolderId)
{
  m_folders.promptAddGroupFolder(parentFolderId);
}

/**
 * @brief Prompts for a new title for the given group folder.
 */
void DataModel::ProjectModel::promptRenameGroupFolder(int folderId)
{
  m_folders.promptRenameGroupFolder(folderId);
}

/**
 * @brief Confirms before deleting a group folder.
 */
void DataModel::ProjectModel::confirmDeleteGroupFolder(int folderId)
{
  m_folders.confirmDeleteGroupFolder(folderId);
}

/**
 * @brief Adds a table folder under @p parentFolderId; returns its new id.
 */
int DataModel::ProjectModel::addTableFolder(int parentFolderId, const QString& title)
{
  return m_folders.addTableFolder(parentFolderId, title);
}

/**
 * @brief Returns the title of a table folder, or empty if not found.
 */
QString DataModel::ProjectModel::tableFolderTitle(int folderId) const
{
  return m_folders.tableFolderTitle(folderId);
}

/**
 * @brief Renames a table folder (slashes stripped); ignores empty names.
 */
void DataModel::ProjectModel::renameTableFolder(int folderId, const QString& title)
{
  m_folders.renameTableFolder(folderId, title);
}

/**
 * @brief Deletes a table folder, promoting its contents to its parent.
 */
void DataModel::ProjectModel::deleteTableFolder(int folderId)
{
  m_folders.deleteTableFolder(folderId);
}

/**
 * @brief Files a table into folder @p parentFolderId.
 */
void DataModel::ProjectModel::moveTableToFolder(const QString& tablePath, int parentFolderId)
{
  m_folders.moveTableToFolder(tablePath, parentFolderId);
}

/**
 * @brief Re-parents a table folder, rejecting a cyclic move.
 */
void DataModel::ProjectModel::moveTableFolderToFolder(int folderId, int parentFolderId)
{
  m_folders.moveTableFolderToFolder(folderId, parentFolderId);
}

/**
 * @brief Reorders a table folder among its sibling folders.
 */
void DataModel::ProjectModel::moveTableFolderInParent(int folderId, int direction)
{
  m_folders.moveTableFolderInParent(folderId, direction);
}

/**
 * @brief Prompts for a name and creates a table folder under @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddTableFolder(int parentFolderId)
{
  m_folders.promptAddTableFolder(parentFolderId);
}

/**
 * @brief Prompts for a name, creates a table, and files it into @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddTableInFolder(int parentFolderId)
{
  m_folders.promptAddTableInFolder(parentFolderId);
}

/**
 * @brief Prompts for a new title for the given table folder.
 */
void DataModel::ProjectModel::promptRenameTableFolder(int folderId)
{
  m_folders.promptRenameTableFolder(folderId);
}

/**
 * @brief Confirms before deleting a table folder.
 */
void DataModel::ProjectModel::confirmDeleteTableFolder(int folderId)
{
  m_folders.confirmDeleteTableFolder(folderId);
}

//--------------------------------------------------------------------------------------------------
// Bulk-operation forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Duplicates every item in @p items, folder subtrees first.
 */
void DataModel::ProjectModel::duplicateSelectedItems(const QVariantList& items)
{
  m_bulk.duplicateSelectedItems(items);
}

/**
 * @brief Deletes every item in @p items, children first, descending ids within a kind.
 */
void DataModel::ProjectModel::deleteSelectedItems(const QVariantList& items)
{
  m_bulk.deleteSelectedItems(items);
}

/**
 * @brief Prompts before deleting a multi-selection, then deletes it.
 */
void DataModel::ProjectModel::confirmDeleteSelectedItems(const QVariantList& items)
{
  m_bulk.confirmDeleteSelectedItems(items);
}

/**
 * @brief Files every item in @p items into folder @p folderId via its per-kind move.
 */
void DataModel::ProjectModel::moveSelectedItemsToFolder(const QVariantList& items, int folderId)
{
  m_bulk.moveSelectedItemsToFolder(items, folderId);
}

/**
 * @brief Enables or disables every applicable item in a tree multi-selection.
 */
void DataModel::ProjectModel::setItemsEnabled(const QVariantList& items, bool enabled)
{
  m_bulk.setItemsEnabled(items, enabled);
}
