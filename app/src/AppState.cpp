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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "AppState.h"

#include <QFileInfo>

#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs AppState and restores the saved operation mode.
 */
AppState::AppState() : m_ephemeralSession(false), m_operationMode(SerialStudio::QuickPlot)
{
  const int saved   = m_settings.value("operation_mode", SerialStudio::QuickPlot).toInt();
  const int clamped = (saved >= SerialStudio::ProjectFile && saved <= SerialStudio::QuickPlot)
                      ? saved
                      : SerialStudio::QuickPlot;
  m_operationMode   = static_cast<SerialStudio::OperationMode>(clamped);
  m_frameConfig     = deriveFrameConfig();
}

/**
 * @brief Returns the singleton instance of AppState.
 */
AppState& AppState::instance()
{
  static AppState singleton;
  return singleton;
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
 * @brief Returns the current derived FrameConfig for device 0.
 */
IO::FrameConfig AppState::frameConfig() const noexcept
{
  return m_frameConfig;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires AppState to ProjectModel so frame config and file path update reactively.
 */
void AppState::setupExternalConnections()
{
  auto& pm = DataModel::ProjectModel::instance();
  connect(&pm, &DataModel::ProjectModel::jsonFileChanged, this, &AppState::onProjectLoaded);
  connect(&pm, &DataModel::ProjectModel::frameDetectionChanged, this, &AppState::onProjectLoaded);
}

/**
 * @brief Restores the last opened project file after all modules are wired up.
 */
void AppState::restoreLastProject()
{
  auto& pm                 = DataModel::ProjectModel::instance();
  const QString saved_path = m_settings.value("project_file_path", "").toString();
  const auto persistedMode = m_operationMode;

  if (!saved_path.isEmpty() && QFileInfo::exists(saved_path))
    pm.openJsonFile(saved_path);
  else
    pm.newJsonFile();

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
 * @brief Sets the operation mode, persists it, and emits derived config changes.
 */
void AppState::setOperationMode(SerialStudio::OperationMode mode)
{
  if (m_operationMode == mode)
    return;

  m_operationMode = mode;
  if (!m_ephemeralSession)
    m_settings.setValue("operation_mode", static_cast<int>(mode));

  m_frameConfig = deriveFrameConfig();

  Q_EMIT operationModeChanged();
  Q_EMIT frameConfigChanged(m_frameConfig);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Synchronises all state after a project load or clear.
 */
void AppState::onProjectLoaded()
{
  const QString path = DataModel::ProjectModel::instance().jsonFilePath();
  if (m_projectFilePath != path) {
    m_projectFilePath = path;
    if (!m_ephemeralSession)
      m_settings.setValue("project_file_path", path);

    Q_EMIT projectFileChanged();
  }

  DataModel::FrameBuilder::instance().syncFromProjectModel();

  m_frameConfig = deriveFrameConfig();
  Q_EMIT frameConfigChanged(m_frameConfig);
}

/**
 * @brief Clears the stored project file path and notifies listeners.
 */
void AppState::onProjectFileCleared()
{
  if (m_projectFilePath.isEmpty())
    return;

  m_projectFilePath.clear();
  if (!m_ephemeralSession)
    m_settings.remove("project_file_path");

  Q_EMIT projectFileChanged();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

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

  const auto& sources = DataModel::ProjectModel::instance().sources();
  if (sources.empty()) {
    cfg.startSequences    = {QByteArray("/*")};
    cfg.finishSequences   = {QByteArray("*/")};
    cfg.checksumAlgorithm = QString();
    cfg.frameDetection    = DataModel::ProjectModel::instance().frameDetection();
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
