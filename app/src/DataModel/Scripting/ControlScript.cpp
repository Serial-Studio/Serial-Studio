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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/ControlScript.h"

#include "AppState.h"
#include "DataModel/Scripting/ControlScriptWorker.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the worker on its own thread and tracks the connection lifecycle.
 */
DataModel::ControlScript::ControlScript() : m_ready(false), m_running(false), m_worker(nullptr)
{
  m_worker = new ControlScriptWorker();
  m_worker->moveToThread(&m_thread);

  connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
  connect(m_worker, &ControlScriptWorker::scriptError, this, &ControlScript::onWorkerError);

  m_thread.start();
}

/**
 * @brief Wires the lifecycle signals after singletons exist, avoiding recursive static init.
 */
void DataModel::ControlScript::setupExternalConnections()
{
  m_ready = true;

  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &ControlScript::onConnectedChanged);
  connect(&AppState::instance(),
          &AppState::operationModeChanged,
          this,
          &ControlScript::onConnectedChanged);

  onConnectedChanged();
}

/**
 * @brief Stops the worker thread cleanly before destruction.
 */
DataModel::ControlScript::~ControlScript()
{
  stopWorker();
  m_thread.quit();
  m_thread.wait();
}

/**
 * @brief Returns the singleton instance.
 */
DataModel::ControlScript& DataModel::ControlScript::instance()
{
  static ControlScript singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current control-script source.
 */
QString DataModel::ControlScript::code() const
{
  return m_code;
}

/**
 * @brief Returns whether the control script is currently running.
 */
bool DataModel::ControlScript::running() const
{
  return m_running;
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the control-script source; restarts the worker if connected.
 */
void DataModel::ControlScript::setCode(const QString& code)
{
  if (m_code == code)
    return;

  m_code = code;
  Q_EMIT codeChanged();

  if (shouldRun()) {
    stopWorker();
    startWorker();
  }
}

//--------------------------------------------------------------------------------------------------
// Lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief True only when connected and in ProjectFile mode (a project owns the script).
 */
bool DataModel::ControlScript::shouldRun() const
{
  if (!m_ready)
    return false;

  return IO::ConnectionManager::instance().isConnected()
      && AppState::instance().operationMode() == SerialStudio::ProjectFile;
}

/**
 * @brief Starts or stops the control script in step with the connection and operation mode.
 */
void DataModel::ControlScript::onConnectedChanged()
{
  if (shouldRun())
    startWorker();
  else
    stopWorker();
}

/**
 * @brief Forwards a worker-thread script error to the GUI and marks the script stopped.
 */
void DataModel::ControlScript::onWorkerError(const QString& message)
{
  if (m_running) {
    m_running = false;
    Q_EMIT runningChanged();
  }

  Q_EMIT error(message);
}

/**
 * @brief Asks the worker thread to compile and run the current script.
 */
void DataModel::ControlScript::startWorker()
{
  if (m_running || m_code.trimmed().isEmpty() || !shouldRun())
    return;

  QMetaObject::invokeMethod(m_worker, "start", Qt::QueuedConnection, Q_ARG(QString, m_code));
  m_running = true;
  Q_EMIT runningChanged();
}

/**
 * @brief Asks the worker thread to stop the running script.
 */
void DataModel::ControlScript::stopWorker()
{
  if (!m_running)
    return;

  QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);
  m_running = false;
  Q_EMIT runningChanged();
}
