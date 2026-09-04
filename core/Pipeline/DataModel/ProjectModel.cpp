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
  , m_workspaceRegenPending(false)
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
  connect(this, &ProjectModel::editorWorkspacesChanged, this, &ProjectModel::scheduleAutoSave);
  connect(this, &ProjectModel::titleChanged, this, &ProjectModel::scheduleAutoSave);

  // code-verify off
  // Must run before the groupsChanged auto-regen connect below: newJsonFile()
  // emits groupsChanged while AppState is still mid-init, so wiring first would
  // fire the regen handler against half-initialized state.
  newJsonFile();
  // code-verify on

  connect(this, &ProjectModel::groupsChanged, this, &ProjectModel::scheduleWorkspaceRegen);

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
 * @brief Returns the total number of datasets across all groups.
 */
int DataModel::ProjectModel::datasetCount() const
{
  int count = 0;
  for (const auto& group : m_groups)
    count += static_cast<int>(group.datasets.size());

  return count;
}

//--------------------------------------------------------------------------------------------------
// Sink configuration
//--------------------------------------------------------------------------------------------------

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
    if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
      return;

    setPointCount(UI::Dashboard::instance().points());
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
 * @brief Queues one auto-workspace regeneration for the end of the current event-loop turn: a
 *        bulk delete emits groupsChanged per removed group and each pass rebuilt the whole
 *        workspace list. groupsChanged itself stays synchronous, so the mutation epoch, the
 *        dirty flag and the queued tree rebuild keep their ordering.
 */
void DataModel::ProjectModel::scheduleWorkspaceRegen()
{
  if (m_workspaceRegenPending)
    return;

  m_workspaceRegenPending = true;
  QTimer::singleShot(0, this, &ProjectModel::flushWorkspaceRegen);
}

/**
 * @brief Runs a queued auto-workspace regeneration now. Every disk write calls this first, so a
 *        save can never serialize the workspace list the queued pass was about to replace.
 */
void DataModel::ProjectModel::flushWorkspaceRegen()
{
  if (!m_workspaceRegenPending)
    return;

  m_workspaceRegenPending = false;

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
