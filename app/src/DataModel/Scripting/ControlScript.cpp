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

#include <QCoreApplication>
#include <QDebug>

#include "AppState.h"
#include "DataModel/Scripting/ControlScriptWorker.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kShutdownWaitSlices  = 50;
static constexpr int kShutdownWaitSliceMs = 100;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the worker on its own thread and tracks the connection lifecycle. The worker
 *        thread is joined at the start of ModuleManager::onQuit() while every module is still
 *        alive (aboutToQuit stays connected as an idempotent fallback); the marshaller is
 *        parented to this singleton so it outlives the worker thread.
 */
DataModel::ControlScript::ControlScript()
  : m_ready(false)
  , m_running(false)
  , m_shouldRun(false)
  , m_shutdown(false)
  , m_worker(nullptr)
  , m_marshaller(nullptr)
{
  m_marshaller = new ControlApiMarshaller(this);
  m_worker     = new ControlScriptWorker(m_marshaller);
  m_worker->moveToThread(&m_thread);

  connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
  connect(m_worker, &ControlScriptWorker::scriptError, this, &ControlScript::onWorkerError);
  connect(
    QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &ControlScript::shutdown);

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
 * @brief Joins the worker thread; normally a no-op because shutdown() already ran on
 *        aboutToQuit, while the GUI event loop was still alive.
 */
DataModel::ControlScript::~ControlScript()
{
  shutdown();
}

/**
 * @brief Stops the worker and joins its thread while the GUI thread can still drain the
 *        blocking apiCall marshaller; a worker parked inside an apiCall dispatch is released
 *        by sendPostedEvents() and then fails fast on the shutdown flag.
 */
void DataModel::ControlScript::shutdown()
{
  if (m_shutdown)
    return;

  m_shutdown = true;
  ControlScriptWorker::requestShutdown();
  QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);

  if (m_running) {
    m_running = false;
    Q_EMIT runningChanged();
  }

  m_thread.quit();
  for (int i = 0; i < kShutdownWaitSlices; ++i) {
    if (m_thread.wait(kShutdownWaitSliceMs))
      break;

    if (QCoreApplication::instance())
      QCoreApplication::sendPostedEvents(m_marshaller);
  }

  if (m_thread.isRunning())
    qWarning() << "[ControlScript] Worker thread did not stop within the shutdown budget";
  else
    m_worker = nullptr;
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
  if (!m_ready || m_shutdown)
    return false;

  return IO::ConnectionManager::instance().isConnected()
      && AppState::instance().operationMode() == SerialStudio::ProjectFile;
}

/**
 * @brief Starts or stops the control script in step with the connection and operation mode.
 *        Every rising edge force-restarts the worker so each connection gets a fresh engine
 *        (setup() re-runs, top-level script state resets) even if a stale worker survived.
 */
void DataModel::ControlScript::onConnectedChanged()
{
  const bool run = shouldRun();
  if (run == m_shouldRun)
    return;

  m_shouldRun = run;
  stopWorker();
  if (run)
    startWorker();
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
 * @brief Asks the worker thread to stop the running script. Always queues the (idempotent)
 *        stop so a worker that desynced from m_running can never keep an old engine alive
 *        across a connection cycle. No-op after shutdown(): the joined worker thread has
 *        already deleted m_worker, so the pointer dangles.
 */
void DataModel::ControlScript::stopWorker()
{
  if (m_shutdown)
    return;

  QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);

  if (m_running) {
    m_running = false;
    Q_EMIT runningChanged();
  }
}
