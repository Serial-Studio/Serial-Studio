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

#include "DataModel/Scripting/ControlScriptWorker.h"

#include <atomic>
#include <QFile>
#include <QJSEngine>
#include <QJsonObject>
#include <QThread>
#include <QTimer>

#include "API/CommandHandler.h"
#include "API/CommandProtocol.h"
#include "API/CommandRegistry.h"
#include "DataModel/Scripting/JsWatchdog.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kLoopRearmMs       = 1;
static constexpr int kDelaySliceMs      = 50;
static constexpr int kMaxDelayMs        = 3600000;
static constexpr int kRuntimeWatchdogMs = 2000;
static constexpr int kReplyPollSliceMs  = 5;
static constexpr int kMaxReplyWaitMs    = 30000;

static std::atomic<bool> s_shutdownRequested{false};

//--------------------------------------------------------------------------------------------------
// GUI-thread API marshaller
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the GUI-thread API marshaller.
 */
DataModel::ControlApiMarshaller::ControlApiMarshaller(QObject* parent) : QObject(parent) {}

/**
 * @brief Runs an apiCall on the GUI thread so handlers never touch GUI objects off-thread.
 *        Fails fast once shutdown is requested: a call drained late in teardown must never
 *        execute a command against modules that are already torn down.
 */
QVariantMap DataModel::ControlApiMarshaller::dispatch(const QString& method,
                                                      const QVariantMap& params)
{
  static std::atomic<quint64> s_requestSeq{0};

  if (s_shutdownRequested.load(std::memory_order_acquire)) {
    QVariantMap out;
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: application is shutting down"));
    return out;
  }

  API::CommandRequest request;
  request.id      = QStringLiteral("cs-%1").arg(s_requestSeq.fetch_add(1) + 1);
  request.command = method;
  request.params  = QJsonObject::fromVariantMap(params);

  const auto response = API::CommandHandler::instance().processCommand(request);

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

  return IO::ConnectionManager::instance().writeAndArmReply(sourceId, data);
}

/**
 * @brief GUI-thread entry that returns the bytes captured for @p sourceId so far.
 */
QByteArray DataModel::ControlApiMarshaller::pollReply(int sourceId)
{
  return IO::ConnectionManager::instance().pollReplyBuffer(sourceId);
}

/**
 * @brief GUI-thread entry that releases the capture buffer for @p sourceId.
 */
void DataModel::ControlApiMarshaller::disarmReply(int sourceId)
{
  IO::ConnectionManager::instance().disarmReplyCapture(sourceId);
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
 *        fast during shutdown (the GUI loop stops dispatching) and re-arms the watchdog after
 *        the call so GUI-side time is not billed against the script budget.
 */
QVariantMap DataModel::ControlApiBridge::call(const QString& method, const QVariantMap& params)
{
  QVariantMap out;
  if (method.isEmpty()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: method must not be empty"));
    return out;
  }

  if (s_shutdownRequested.load(std::memory_order_acquire)) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: application is shutting down"));
    return out;
  }

  const bool ok = QMetaObject::invokeMethod(m_marshaller,
                                            "dispatch",
                                            Qt::BlockingQueuedConnection,
                                            Q_RETURN_ARG(QVariantMap, out),
                                            Q_ARG(QString, method),
                                            Q_ARG(QVariantMap, params));
  if (m_watchdog && !s_shutdownRequested.load(std::memory_order_acquire))
    m_watchdog->arm();

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
  const auto& commands = API::CommandRegistry::instance().commands();
  result.reserve(commands.size());
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it)
    result.append(it.key());

  return result;
}

/**
 * @brief Writes @p data to @p sourceId then blocks the worker (never the GUI) until the reply
 *        satisfies @p until (terminator, byte length, or undefined for first non-empty) or
 *        @p timeoutMs elapses, capped at kMaxReplyWaitMs. The C++ wait re-arms the watchdog after
 *        (like delay()), so device latency is never billed to the JS budget. Shutdown-sliced.
 */
QVariantMap DataModel::ControlApiBridge::writeAndWait(const QJSValue& data,
                                                      int timeoutMs,
                                                      const QJSValue& until,
                                                      int sourceId)
{
  QVariantMap out;
  out.insert(QStringLiteral("ok"), false);
  out.insert(QStringLiteral("data"), QString());
  out.insert(QStringLiteral("bytesRead"), 0);
  out.insert(QStringLiteral("timedOut"), false);

  QByteArray payload;
  if (data.isString()) {
    payload = data.toString().toUtf8();
  } else if (data.isArray()) {
    const int len = data.property(QStringLiteral("length")).toInt();
    payload.reserve(len);
    for (int i = 0; i < len; ++i)
      payload.append(static_cast<char>(data.property(static_cast<quint32>(i)).toInt() & 0xff));
  }

  if (sourceId < 0 || payload.isEmpty()) {
    out.insert(QStringLiteral("error"), QStringLiteral("deviceWriteAndWait: bad source or data"));
    return out;
  }

  if (s_shutdownRequested.load(std::memory_order_acquire)) {
    out.insert(QStringLiteral("error"), QStringLiteral("deviceWriteAndWait: shutting down"));
    return out;
  }

  QByteArray terminator;
  int expectedLen = 0;
  if (until.isString())
    terminator = until.toString().toUtf8();
  else if (until.isNumber())
    expectedLen = until.toInt();

  qint64 written = -1;
  QMetaObject::invokeMethod(m_marshaller,
                            "writeAndArm",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(qint64, written),
                            Q_ARG(int, sourceId),
                            Q_ARG(QByteArray, payload));

  if (written <= 0) {
    QMetaObject::invokeMethod(
      m_marshaller, "disarmReply", Qt::BlockingQueuedConnection, Q_ARG(int, sourceId));
    out.insert(QStringLiteral("error"), QStringLiteral("deviceWriteAndWait: write failed"));
    if (m_watchdog && !s_shutdownRequested.load(std::memory_order_acquire))
      m_watchdog->arm();

    return out;
  }

  const auto satisfies = [&](const QByteArray& reply) {
    if (!terminator.isEmpty())
      return reply.contains(terminator);

    if (expectedLen > 0)
      return reply.size() >= expectedLen;

    return !reply.isEmpty();
  };

  const int budget = qBound(0, timeoutMs, kMaxReplyWaitMs);
  QByteArray reply;
  bool satisfied = false;
  for (int waited = 0; waited <= budget && !satisfied; waited += kReplyPollSliceMs) {
    if (s_shutdownRequested.load(std::memory_order_acquire))
      break;

    QThread::msleep(static_cast<unsigned long>(kReplyPollSliceMs));
    QMetaObject::invokeMethod(m_marshaller,
                              "pollReply",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QByteArray, reply),
                              Q_ARG(int, sourceId));
    satisfied = satisfies(reply);
  }

  QMetaObject::invokeMethod(
    m_marshaller, "disarmReply", Qt::BlockingQueuedConnection, Q_ARG(int, sourceId));

  if (m_watchdog && !s_shutdownRequested.load(std::memory_order_acquire))
    m_watchdog->arm();

  out.insert(QStringLiteral("ok"), satisfied);
  out.insert(QStringLiteral("data"), QString::fromUtf8(reply));
  out.insert(QStringLiteral("bytesRead"), reply.size());
  out.insert(QStringLiteral("timedOut"), !satisfied);
  return out;
}

/**
 * @brief Blocks the worker thread for the given time (capped at one hour), sleeping in slices
 *        so shutdown can interrupt the wait, then defers the watchdog deadline.
 */
void DataModel::ControlApiBridge::delay(int milliseconds)
{
  const int total = qBound(0, milliseconds, kMaxDelayMs);
  for (int slept = 0; slept < total; slept += kDelaySliceMs) {
    if (s_shutdownRequested.load(std::memory_order_acquire))
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
  Q_ASSERT(marshaller);
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
 * @brief Compiles the script, runs setup(), and arms the loop scheduler.
 */
void DataModel::ControlScriptWorker::start(const QString& source)
{
  stop();

  if (source.trimmed().isEmpty())
    return;

  QString errorOut;
  if (!compile(source, errorOut)) {
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
