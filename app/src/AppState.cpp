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
AppState::AppState() : m_operationMode(SerialStudio::QuickPlot)
{
  const int saved = m_settings.value("operation_mode", SerialStudio::QuickPlot).toInt();
  m_operationMode = static_cast<SerialStudio::OperationMode>(saved);
  m_frameConfig   = deriveFrameConfig();
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
  // Connect project model signals to update frame config reactively
  auto& pm = DataModel::ProjectModel::instance();

  connect(&pm, &DataModel::ProjectModel::jsonFileChanged, this, &AppState::onProjectLoaded);
  connect(&pm, &DataModel::ProjectModel::frameDetectionChanged, this, &AppState::onProjectLoaded);
}

/**
 * @brief Restores the last opened project file after all modules are wired up.
 *
 * Called by ModuleManager after all setupExternalConnections() calls so that
 * FrameBuilder and Dashboard signals are connected before the project fires.
 */
void AppState::restoreLastProject()
{
  // Load the last opened project file if it still exists
  auto& pm = DataModel::ProjectModel::instance();

  const QString saved_path = m_settings.value("project_file_path", "").toString();
  if (!saved_path.isEmpty() && QFileInfo::exists(saved_path))
    pm.openJsonFile(saved_path);
  else
    pm.newJsonFile();
}

/**
 * @brief Sets the operation mode, persists it, and emits derived config changes.
 * @param mode The new operation mode.
 */
void AppState::setOperationMode(SerialStudio::OperationMode mode)
{
  // Guard against duplicate assignments
  if (m_operationMode == mode)
    return;

  m_operationMode = mode;
  m_settings.setValue("operation_mode", static_cast<int>(mode));
  Q_EMIT operationModeChanged();

  m_frameConfig = deriveFrameConfig();
  Q_EMIT frameConfigChanged(m_frameConfig);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Synchronises all state after a project load or clear.
 *
 * Called whenever ProjectModel emits jsonFileChanged or frameDetectionChanged.
 * Execution order is intentionally explicit:
 *  1. Update the stored path and notify QML bindings.
 *  2. Sync FrameBuilder's frame structure from the already-parsed ProjectModel data.
 *  3. Re-derive the IO FrameConfig and notify ConnectionManager.
 */
void AppState::onProjectLoaded()
{
  // Update stored path and notify QML bindings
  const QString path = DataModel::ProjectModel::instance().jsonFilePath();
  if (m_projectFilePath != path) {
    m_projectFilePath = path;
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
  // Nothing to do if already cleared
  if (m_projectFilePath.isEmpty())
    return;

  m_projectFilePath.clear();
  m_settings.remove("project_file_path");
  Q_EMIT projectFileChanged();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the FrameConfig for device 0 from current state.
 *
 * In ProjectFile mode source[0] delimiters are used when available.
 * In other modes, default delimiters and no checksum are applied.
 */
IO::FrameConfig AppState::deriveFrameConfig() const
{
  // Build config from current mode and project source[0]
  IO::FrameConfig cfg;
  cfg.operationMode = m_operationMode;

  if (m_operationMode != SerialStudio::ProjectFile) {
    cfg.startSequence     = QByteArray("/*");
    cfg.finishSequence    = QByteArray("*/");
    cfg.checksumAlgorithm = QString();
    cfg.frameDetection    = SerialStudio::EndDelimiterOnly;
    return cfg;
  }

  const auto& sources = DataModel::ProjectModel::instance().sources();
  if (sources.empty()) {
    cfg.startSequence     = QByteArray("/*");
    cfg.finishSequence    = QByteArray("*/");
    cfg.checksumAlgorithm = QString();
    cfg.frameDetection    = DataModel::ProjectModel::instance().frameDetection();
    return cfg;
  }

  const auto& src0 = sources[0];
  QByteArray start, end;
  QString checksum;
  DataModel::read_io_settings(start, end, checksum, DataModel::serialize(src0));

  cfg.startSequence     = start;
  cfg.finishSequence    = end;
  cfg.checksumAlgorithm = checksum;
  cfg.frameDetection    = static_cast<SerialStudio::FrameDetection>(src0.frameDetection);
  return cfg;
}
