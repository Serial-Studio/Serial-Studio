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

#include "DataModel/Scripting/ControlScriptWorker.h"

#include <atomic>
#include <QFile>
#include <QJSEngine>
#include <QJsonObject>
#include <QThread>
#include <QTimer>

#include "Core/Api/CommandProtocol.h"
#include "Core/Api/ICommandExecutor.h"
#include "Core/Api/UserWaitScope.h"
#include "Core/IO/IDeviceWriter.h"
#include "Core/SSAssert.h"
#include "DataModel/Scripting/ChecksumApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/JsWatchdog.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "DataModel/Scripting/ScriptDeviceWait.h"
#include "DataModel/Scripting/ScriptResult.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kLoopRearmMs       = 1;
static constexpr int kDelaySliceMs      = 50;
static constexpr int kControlMaxDelayMs = 3600000;
static constexpr int kRuntimeWatchdogMs = 2000;

static std::atomic<bool> s_shutdownRequested{false};

/**
 * @brief Whether this run ends because it was stopped or the app is quitting: an error it raised
 *        on the way out (an apiCall throwing at the stop) is no script error, so it stops quietly.
 */
static bool endedByStop(const DataModel::ControlApiMarshaller& marshaller)
{
  return marshaller.runStopped() || s_shutdownRequested.load(std::memory_order_acquire);
}

/**
 * @brief Throws into @p bridge's engine that its run was stopped, so the script ends at this call
 *        instead of carrying on with a refused one (spec 0120 R5).
 */
static QVariantMap stoppedCall(QObject* bridge)
{
  const auto message = QStringLiteral("apiCall: the control script was stopped");
  if (auto* engine = qjsEngine(bridge))
    engine->throwError(message);

  return DataModel::ScriptResult::makeError(message);
}

//--------------------------------------------------------------------------------------------------
// GUI-thread API marshaller
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the GUI-thread API marshaller.
 */
DataModel::ControlApiMarshaller::ControlApiMarshaller(QObject* parent)
  : QObject(parent), m_activeRun(0), m_stops(0)
{}

/**
 * @brief Whether the owner stopped the run now active; any thread, after the run began.
 */
bool DataModel::ControlApiMarshaller::runStopped() const noexcept
{
  return m_activeRun.load(std::memory_order_acquire) != m_stops.load(std::memory_order_acquire);
}

/**
 * @brief The id a run queued now runs as; read on the GUI thread when the owner queues a start, so
 *        a stop between that queue and the start still reaches the run.
 */
quint64 DataModel::ControlApiMarshaller::nextRun() const noexcept
{
  return m_stops.load(std::memory_order_acquire);
}

/**
 * @brief Marks @p run (from nextRun()) as the run now active, on whichever thread starts it.
 */
void DataModel::ControlApiMarshaller::beginRun(quint64 run) noexcept
{
  m_activeRun.store(run, std::memory_order_release);
}

/**
 * @brief Ends the run now active (GUI thread): its pending and later apiCalls are refused or throw.
 */
void DataModel::ControlApiMarshaller::stopRun() noexcept
{
  m_stops.fetch_add(1, std::memory_order_acq_rel);
}

/**
 * @brief Runs an apiCall on the GUI thread; fails fast once shutdown is requested or the calling
 *        run was stopped, so a call posted before a stop never opens a box after it. The control
 *        script and macros may wait on the user (a modal message box), granted by this marshaller:
 *        their workers own no acquisition work, so parking one behind a dialog stalls nothing else.
 */
QVariantMap DataModel::ControlApiMarshaller::dispatch(const QString& method,
                                                      const QVariantMap& params)
{
  static std::atomic<quint64> s_requestSeq{0};

  if (s_shutdownRequested.load(std::memory_order_acquire))
    return ScriptResult::makeError(QStringLiteral("apiCall: application is shutting down"));

  if (runStopped())
    return ScriptResult::makeError(QStringLiteral("apiCall: the script was stopped"));

  API::CommandRequest request;
  request.id      = QStringLiteral("cs-%1").arg(s_requestSeq.fetch_add(1) + 1);
  request.command = method;
  request.params  = QJsonObject::fromVariantMap(params);

  const API::UserWaitScope userWait(true, this);
  const auto response = DataModel::requireCommandExecutor().execute(request);

  QVariantMap out;
  out.insert(QStringLiteral("ok"), response.success);
  if (response.success) {
    if (!response.result.isEmpty())
      out.insert(QStringLiteral("result"), response.result.toVariantMap());
  } else {
    out.insert(QStringLiteral("error"), response.errorMessage);
    out.insert(QStringLiteral("errorCode"), response.errorCode);
    if (!response.errorData.isEmpty())
      out.insert(QStringLiteral("errorData"), response.errorData.toVariantMap());
  }
  return out;
}

/**
 * @brief GUI-thread entry that arms reply capture and writes to the device in one step.
 */
qint64 DataModel::ControlApiMarshaller::writeAndArm(int sourceId, const QByteArray& data)
{
  if (s_shutdownRequested.load(std::memory_order_acquire))
    return -1;

  return DataModel::requireDeviceWriter().writeAndArmReply(sourceId, data);
}

/**
 * @brief GUI-thread entry that returns the bytes captured for @p sourceId so far.
 */
QByteArray DataModel::ControlApiMarshaller::pollReply(int sourceId)
{
  return DataModel::requireDeviceWriter().pollReplyBuffer(sourceId);
}

/**
 * @brief GUI-thread entry that releases the capture buffer for @p sourceId.
 */
void DataModel::ControlApiMarshaller::disarmReply(int sourceId)
{
  DataModel::requireDeviceWriter().disarmReplyCapture(sourceId);
}

//--------------------------------------------------------------------------------------------------
// JS-facing apiCall bridge
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the JS bridge bound to a GUI-thread marshaller.
 */
DataModel::ControlApiBridge::ControlApiBridge(ControlApiMarshaller* marshaller, QObject* parent)
  : QObject(parent), m_marshaller(marshaller), m_watchdog(nullptr)
{}

/**
 * @brief Binds the watchdog so delay() can extend its deadline while sleeping.
 */
void DataModel::ControlApiBridge::setWatchdog(DataModel::JsWatchdog* watchdog)
{
  m_watchdog = watchdog;
}

/**
 * @brief Implements apiCall(method, params) by blocking onto the GUI-thread marshaller. Fails
 *        fast during shutdown (the GUI loop stops dispatching). The watchdog is disarmed for the
 *        call and re-armed after it, so GUI-side time (a message box waiting on the user) is not
 *        billed, and no interrupt from a deadline that passed mid-wait survives the re-arm.
 */
QVariantMap DataModel::ControlApiBridge::call(const QString& method, const QVariantMap& params)
{
  QVariantMap out;
  if (method.isEmpty())
    return ScriptResult::makeError(QStringLiteral("apiCall: method must not be empty"));

  if (s_shutdownRequested.load(std::memory_order_acquire))
    return ScriptResult::makeError(QStringLiteral("apiCall: application is shutting down"));

  if (m_marshaller->runStopped())
    return stoppedCall(this);

  if (m_watchdog)
    m_watchdog->disarm();

  const bool ok = QMetaObject::invokeMethod(m_marshaller,
                                            "dispatch",
                                            Qt::BlockingQueuedConnection,
                                            Q_RETURN_ARG(QVariantMap, out),
                                            Q_ARG(QString, method),
                                            Q_ARG(QVariantMap, params));
  if (m_watchdog && !s_shutdownRequested.load(std::memory_order_acquire))
    m_watchdog->arm();

  if (endedByStop(*m_marshaller))
    return stoppedCall(this);

  if (!ok) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: GUI dispatch failed"));
  }

  return out;
}

/**
 * @brief Returns the array of registered API command names for discovery.
 */
QVariantList DataModel::ControlApiBridge::listCommands()
{
  QVariantList result;
  const auto names = DataModel::requireCommandExecutor().commandNames();
  result.reserve(names.size());
  for (const auto& name : names)
    result.append(name);

  return result;
}

/**
 * @brief Writes @p data to @p sourceId then blocks the worker (never the GUI) until the reply
 *        satisfies @p until or @p timeoutMs elapses; a stop of this run or shutdown aborts it.
 */
QVariantMap DataModel::ControlApiBridge::writeAndWait(const QJSValue& data,
                                                      int timeoutMs,
                                                      const QJSValue& until,
                                                      int sourceId)
{
  const auto* marshaller = m_marshaller;
  const auto aborted     = [marshaller] {
    return endedByStop(*marshaller);
  };

  return ScriptDeviceWait::writeAndWait(m_marshaller,
                                        m_watchdog,
                                        aborted,
                                        QStringLiteral("deviceWriteAndWait: stopped"),
                                        data,
                                        timeoutMs,
                                        until,
                                        sourceId);
}

/**
 * @brief Blocks the worker thread for the given time (capped at one hour), sleeping in slices
 *        so a stop of this run or shutdown can interrupt the wait, then defers the watchdog.
 */
void DataModel::ControlApiBridge::delay(int milliseconds)
{
  const int total = qBound(0, milliseconds, kControlMaxDelayMs);
  for (int slept = 0; slept < total; slept += kDelaySliceMs) {
    if (endedByStop(*m_marshaller))
      return;

    QThread::msleep(static_cast<unsigned long>(qMin(kDelaySliceMs, total - slept)));
  }

  if (m_watchdog)
    m_watchdog->arm();
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the worker around a GUI-thread marshaller owned by the caller, which must
 *        outlive the worker thread.
 */
DataModel::ControlScriptWorker::ControlScriptWorker(ControlApiMarshaller* marshaller,
                                                    QObject* parent)
  : QObject(parent)
  , m_loaded(false)
  , m_loopTimer(nullptr)
  , m_bridge(nullptr)
  , m_marshaller(marshaller)
{
  SS_ASSERT_LOG(marshaller != nullptr);
}

/**
 * @brief Destructor; engine and timer are torn down on the worker thread.
 */
DataModel::ControlScriptWorker::~ControlScriptWorker()
{
  stop();
}

/**
 * @brief Marks the process as shutting down so apiCall() and delay() fail fast instead of
 *        blocking on a GUI event loop that no longer dispatches.
 */
void DataModel::ControlScriptWorker::requestShutdown() noexcept
{
  s_shutdownRequested.store(true, std::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
// Lifecycle slots (run on the worker thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compiles the script, runs setup(), and arms the loop scheduler, as @p run (the
 *        marshaller's nextRun() when the owner queued it): a stop since then cancels it unrun.
 */
void DataModel::ControlScriptWorker::start(const QString& source, quint64 run)
{
  stop();
  m_marshaller->beginRun(run);

  if (source.trimmed().isEmpty() || m_marshaller->runStopped())
    return;

  QString errorOut;
  const bool compiled = compile(source, errorOut);
  if (endedByStop(*m_marshaller)) {
    stop();
    return;
  }

  if (!compiled) {
    Q_EMIT scriptError(errorOut);
    return;
  }

  runSetup();

  if (m_loaded && m_loopFn.isCallable()) {
    if (!m_loopTimer) {
      m_loopTimer = new QTimer(this);
      m_loopTimer->setSingleShot(true);
      m_loopTimer->setTimerType(Qt::PreciseTimer);
      connect(m_loopTimer, &QTimer::timeout, this, &ControlScriptWorker::runLoopTick);
    }

    m_loopTimer->start(kLoopRearmMs);
  }
}

/**
 * @brief Stops the loop timer and releases the engine.
 */
void DataModel::ControlScriptWorker::stop()
{
  if (m_loopTimer)
    m_loopTimer->stop();

  m_loaded  = false;
  m_setupFn = QJSValue();
  m_loopFn  = QJSValue();
  releaseEngine();
}

/**
 * @brief Drops the bridge, watchdog, and engine in that order; the watchdog must unregister
 *        before the engine it monitors is destroyed.
 */
void DataModel::ControlScriptWorker::releaseEngine()
{
  m_bridge = nullptr;
  m_watchdog.reset();
  m_engine.reset();
}

//--------------------------------------------------------------------------------------------------
// Execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs one loop() iteration and re-arms the single-shot timer.
 */
void DataModel::ControlScriptWorker::runLoopTick()
{
  if (!m_loaded || !m_loopFn.isCallable() || !m_watchdog)
    return;

  QJSValueList args;
  const QJSValue result = m_watchdog->call(m_loopFn, args);

  if (endedByStop(*m_marshaller)) {
    stop();
    return;
  }

  if (m_watchdog->lastCallTimedOut()) {
    Q_EMIT scriptError(QStringLiteral("Control script loop() exceeded %1 ms budget; stopped.")
                         .arg(kRuntimeWatchdogMs));
    stop();
    return;
  }

  if (result.isError()) {
    Q_EMIT scriptError(QStringLiteral("loop() line %1: %2")
                         .arg(result.property(QStringLiteral("lineNumber")).toInt())
                         .arg(result.toString()));
    stop();
    return;
  }

  if (m_loopTimer)
    m_loopTimer->start(kLoopRearmMs);
}

/**
 * @brief Runs the one-shot setup() function if the script defines it.
 */
void DataModel::ControlScriptWorker::runSetup()
{
  if (!m_loaded || !m_setupFn.isCallable() || !m_watchdog)
    return;

  QJSValueList args;
  const QJSValue result = m_watchdog->call(m_setupFn, args);

  if (endedByStop(*m_marshaller)) {
    stop();
    return;
  }

  if (m_watchdog->lastCallTimedOut()) {
    Q_EMIT scriptError(
      QStringLiteral("Control script setup() exceeded %1 ms budget.").arg(kRuntimeWatchdogMs));
    stop();
    return;
  }

  if (result.isError()) {
    Q_EMIT scriptError(QStringLiteral("setup() line %1: %2")
                         .arg(result.property(QStringLiteral("lineNumber")).toInt())
                         .arg(result.toString()));
    stop();
  }
}

//--------------------------------------------------------------------------------------------------
// Compilation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the engine, evals the SDK, and grabs setup()/loop(). Only __ss_bridge goes
 *        in: the direct helper bridges wrap main-thread singletons and must never execute on
 *        this worker thread (__ss_control tells the prelude to marshal instead). User source
 *        is evaluated under the armed watchdog so a top-level loop cannot wedge the worker.
 */
bool DataModel::ControlScriptWorker::compile(const QString& source, QString& errorOut)
{
  m_engine = std::make_unique<QJSEngine>();
  m_engine->installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
  m_watchdog = std::make_unique<DataModel::JsWatchdog>(
    m_engine.get(), kRuntimeWatchdogMs, QStringLiteral("Control script"));

  m_bridge = new ControlApiBridge(m_marshaller, m_engine.get());
  m_bridge->setWatchdog(m_watchdog.get());
  auto bridgeVal = m_engine->newQObject(m_bridge);
  m_engine->globalObject().setProperty(QStringLiteral("__ss_bridge"), bridgeVal);
  m_engine->globalObject().setProperty(QStringLiteral("__ss_control"), QJSValue(true));
  DataModel::ChecksumApi::installJS(m_engine.get());

  QFile sdkFile(QStringLiteral(":/api/SerialStudio.js"));
  if (sdkFile.open(QFile::ReadOnly))
    m_engine->evaluate(QString::fromUtf8(sdkFile.readAll()));

  m_watchdog->arm();
  const auto result = m_engine->evaluate(source, QStringLiteral("control-script.js"));
  m_watchdog->disarm();

  if (m_engine->isInterrupted()) {
    m_engine->setInterrupted(false);
    errorOut = QStringLiteral("Script did not finish evaluating within %1 ms and was "
                              "interrupted (infinite loop at the top level?).")
                 .arg(kRuntimeWatchdogMs);
    releaseEngine();
    return false;
  }

  if (result.isError()) {
    errorOut = QStringLiteral("Line %1: %2")
                 .arg(result.property(QStringLiteral("lineNumber")).toInt())
                 .arg(result.toString());
    releaseEngine();
    return false;
  }

  m_setupFn = m_engine->globalObject().property(QStringLiteral("setup"));
  m_loopFn  = m_engine->globalObject().property(QStringLiteral("loop"));

  if (!m_setupFn.isCallable() && !m_loopFn.isCallable()) {
    errorOut = QStringLiteral("Script must define setup() and/or loop().");
    releaseEngine();
    return false;
  }

  m_loaded = true;
  return true;
}
