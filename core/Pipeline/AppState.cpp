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

#include "AppState.h"

#include <QFileInfo>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/DataModel/Frame.h"
#include "Core/ModuleConstruction.h"
#include "Core/Runtime.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"

AppState* AppState::s_instance = nullptr;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs AppState and restores the saved operation mode.
 */
AppState::AppState(Core::Bus::MessageBus& bus)
  : m_bus(bus)
  , m_ephemeralSession(false)
  , m_operationMode(SerialStudio::QuickPlot)
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_frameBuilder(nullptr)
  , m_hasPendingMode(false)
  , m_pendingMode(SerialStudio::QuickPlot)
{
  const int saved   = m_settings.value("operation_mode", SerialStudio::QuickPlot).toInt();
  const int clamped = (saved >= SerialStudio::ProjectFile && saved <= SerialStudio::QuickPlot)
                      ? saved
                      : SerialStudio::QuickPlot;
  m_operationMode   = static_cast<SerialStudio::OperationMode>(clamped);
  m_frameConfig     = deriveFrameConfig();
  publishState();

  m_linkState = m_bus.subscribe<Core::Bus::ConnectionStateChanged>(
    this, [this](const std::shared_ptr<const Core::Bus::ConnectionStateChanged>& link) {
      if (!link->connected && !link->connecting)
        applyPendingMode();
    });
}

/**
 * @brief Returns this session's application state, bound by the session context right after
 * adoption; a reach before that is a named fatal (spec 0039 M2, spec 0077 T66).
 */
AppState& AppState::instance()
{
  SS_ASSERT(s_instance != nullptr, Core::ModuleConstruction::reportUnavailable(staticMetaObject));
  return *s_instance;
}

//--------------------------------------------------------------------------------------------------
// Public accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the base filename of the loaded project, or an empty string.
 */
QString AppState::projectFileName() const
{
  if (m_projectFilePath.isEmpty())
    return QString();

  return QFileInfo(m_projectFilePath).fileName();
}

/**
 * @brief Returns the absolute path of the loaded project file.
 */
const QString& AppState::projectFilePath() const noexcept
{
  return m_projectFilePath;
}

/**
 * @brief Returns the current operation mode.
 */
SerialStudio::OperationMode AppState::operationMode() const noexcept
{
  return m_operationMode;
}

/**
 * @brief Returns the mode the app is heading to: the deferred mode while one waits for a
 *        disconnect, otherwise the current mode.
 */
SerialStudio::OperationMode AppState::targetOperationMode() const noexcept
{
  return m_hasPendingMode ? m_pendingMode : m_operationMode;
}

/**
 * @brief Returns the current derived FrameConfig for device 0.
 */
IO::FrameConfig AppState::frameConfig() const noexcept
{
  return m_frameConfig;
}

/**
 * @brief Returns true while the session is ephemeral (operator/runtime deployments).
 */
bool AppState::ephemeralSession() const noexcept
{
  return m_ephemeralSession;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires AppState to ProjectModel so frame config and file path update reactively.
 */
void AppState::setupExternalConnections()
{
  m_frameBuilder = &DataModel::FrameBuilder::instance();

  connect(
    &m_projectModel, &DataModel::ProjectModel::jsonFileChanged, this, &AppState::onProjectLoaded);
  connect(&m_projectModel,
          &DataModel::ProjectModel::frameDetectionChanged,
          this,
          &AppState::onProjectLoaded);
}

/**
 * @brief Restores the last opened project file after all modules are wired up. A sealed process
 *        restores nothing: the package's project must be the one document it ever loads.
 */
void AppState::restoreLastProject()
{
  if (Core::Runtime::sealed())
    return;

  const QString saved_path = m_settings.value("project_file_path", "").toString();
  const auto persistedMode = m_operationMode;

  if (!saved_path.isEmpty() && QFileInfo::exists(saved_path))
    m_projectModel.openJsonFile(saved_path);
  else
    m_projectModel.newJsonFile();

  if (m_operationMode != persistedMode)
    setOperationMode(persistedMode);
}

/**
 * @brief Marks the session as ephemeral (operator runs) so QSettings writes are skipped,
 *        keeping the main application's last project and operation mode untouched.
 */
void AppState::setEphemeralSession(bool ephemeral)
{
  m_ephemeralSession = ephemeral;
}

/**
 * @brief Sets the operation mode, persists it, and announces the derived config: the retained
 *        topics go out first, mode before framing, then the Qt signals in the same order. An
 *        explicit choice supersedes any implicit switch still waiting for a disconnect.
 */
void AppState::setOperationMode(SerialStudio::OperationMode mode)
{
  m_hasPendingMode = false;
  if (m_operationMode == mode)
    return;

  m_operationMode = mode;
  if (!m_ephemeralSession)
    m_settings.setValue("operation_mode", static_cast<int>(mode));

  m_frameConfig = deriveFrameConfig();
  publishState();

  Q_EMIT operationModeChanged();
  Q_EMIT frameConfigChanged(m_frameConfig);
}

/**
 * @brief Requests an implicit mode switch (spec 0100 R7): a project load, save or generation asks
 *        for Project File mode. Applied at once while disconnected; while a link is live it waits
 *        for the disconnect, is persisted so a quit keeps it, and the user is told. Explicit mode
 *        choices keep calling setOperationMode().
 */
void AppState::requestOperationMode(SerialStudio::OperationMode mode)
{
  const auto link = m_bus.latest<Core::Bus::ConnectionStateChanged>();
  const bool live = link && (link->connected || link->connecting);
  if (!live) {
    m_hasPendingMode = false;
    setOperationMode(mode);
    return;
  }

  m_pendingMode    = mode;
  m_hasPendingMode = (mode != m_operationMode);
  if (!m_ephemeralSession)
    m_settings.setValue("operation_mode", static_cast<int>(targetOperationMode()));

  if (!m_hasPendingMode)
    return;

  DataModel::pipelineModules().notifications.postInfo(
    QStringLiteral("OperationMode"),
    tr("Mode changes after you disconnect"),
    tr("The project is ready. Serial Studio switches to Project File mode when the current "
       "connection ends, so the session in progress is not interrupted."));
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Synchronises all state after a project load or clear.
 */
void AppState::onProjectLoaded()
{
  const QString path = m_projectModel.jsonFilePath();
  if (m_projectFilePath != path) {
    m_projectFilePath = path;
    if (!m_ephemeralSession)
      m_settings.setValue("project_file_path", path);

    Q_EMIT projectFileChanged();
  }

  SS_ASSERT_LOG(m_frameBuilder != nullptr);
  if (m_frameBuilder)
    m_frameBuilder->syncFromProjectModel();

  m_frameConfig = deriveFrameConfig();
  m_bus.publishState<Core::Bus::FrameConfigChanged>(m_frameConfig);
  Q_EMIT frameConfigChanged(m_frameConfig);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the mode a live session deferred, once the link is down.
 */
void AppState::applyPendingMode()
{
  if (!m_hasPendingMode)
    return;

  setOperationMode(m_pendingMode);
}

/**
 * @brief Retains the mode and its framing on the bus, mode first, so a subscriber replaying the
 *        latest state or a library below reading latest<>() sees both at any point after
 *        construction (the headless roots never run a wiring pass).
 */
void AppState::publishState()
{
  m_bus.publishState<Core::Bus::OperationModeChanged>(static_cast<int>(m_operationMode));
  m_bus.publishState<Core::Bus::FrameConfigChanged>(m_frameConfig);
}

/**
 * @brief Computes the FrameConfig for device 0 from current state.
 */
IO::FrameConfig AppState::deriveFrameConfig() const
{
  IO::FrameConfig cfg;
  cfg.operationMode = m_operationMode;

  if (m_operationMode == SerialStudio::QuickPlot) {
    cfg.startSequences    = {};
    cfg.finishSequences   = {QByteArray("\n"), QByteArray("\r\n"), QByteArray("\r")};
    cfg.checksumAlgorithm = QString();
    cfg.frameDetection    = SerialStudio::EndDelimiterOnly;
    return cfg;
  }

  if (m_operationMode == SerialStudio::ConsoleOnly) {
    cfg.startSequences    = {};
    cfg.finishSequences   = {};
    cfg.checksumAlgorithm = QString();
    cfg.frameDetection    = SerialStudio::NoDelimiters;
    return cfg;
  }

  const auto& sources = m_projectModel.sources();
  if (sources.empty()) {
    cfg.startSequences    = {QByteArray("/*")};
    cfg.finishSequences   = {QByteArray("*/")};
    cfg.checksumAlgorithm = QString();
    cfg.frameDetection    = m_projectModel.frameDetection();
    return cfg;
  }

  cfg.frameDetection = static_cast<SerialStudio::FrameDetection>(sources[0].frameDetection);

  QByteArray startSeq;
  QByteArray finishSeq;
  DataModel::read_io_settings(
    startSeq, finishSeq, cfg.checksumAlgorithm, DataModel::serialize(sources[0]));

  cfg.startSequences  = startSeq.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{startSeq};
  cfg.finishSequences = finishSeq.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{finishSeq};

  if ((cfg.frameDetection == SerialStudio::StartDelimiterOnly
       || cfg.frameDetection == SerialStudio::StartAndEndDelimiter)
      && cfg.startSequences.isEmpty()) [[unlikely]]
    cfg.frameDetection =
      cfg.finishSequences.isEmpty() ? SerialStudio::NoDelimiters : SerialStudio::EndDelimiterOnly;

  if (cfg.frameDetection == SerialStudio::EndDelimiterOnly && cfg.finishSequences.isEmpty())
    [[unlikely]]
    cfg.frameDetection = SerialStudio::NoDelimiters;

  return cfg;
}
