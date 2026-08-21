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

#include "DataModel/Scripting/MacroWorker.h"

#include <QFile>
#include <QJSEngine>
#include <QThread>

#include "API/CommandRegistry.h"
#include "DataModel/Scripting/ControlScriptWorker.h"
#include "DataModel/Scripting/JsWatchdog.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kMacroWatchdogMs       = 5000;
static constexpr int kMacroDelaySliceMs     = 50;
static constexpr int kMacroMaxDelayMs       = 3600000;
static constexpr int kMacroReplyPollSliceMs = 5;
static constexpr int kMacroMaxReplyWaitMs   = 30000;

//--------------------------------------------------------------------------------------------------
// Macro apiCall bridge (worker thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the macro bridge bound to a GUI-thread marshaller and the worker's stop flag.
 */
DataModel::MacroApiBridge::MacroApiBridge(ControlApiMarshaller* marshaller,
                                          const std::atomic<bool>* stopFlag,
                                          QObject* parent)
  : QObject(parent)
  , m_marshaller(marshaller)
  , m_watchdog(nullptr)
  , m_stop(stopFlag)
  , m_registry(API::CommandRegistry::instance())
{
  SS_ASSERT_LOG(marshaller != nullptr);
  SS_ASSERT_LOG(stopFlag != nullptr);
}

/**
 * @brief Binds the watchdog so GUI-side time (apiCall, delay, device waits) is not billed
 *        against the macro's JS budget.
 */
void DataModel::MacroApiBridge::setWatchdog(DataModel::JsWatchdog* watchdog)
{
  m_watchdog = watchdog;
}

/**
 * @brief Returns whether the user pressed Stop; checked at every bridge touch-point so a
 *        macro sleeping or waiting on the device aborts promptly.
 */
bool DataModel::MacroApiBridge::stopRequested() const noexcept
{
  return m_stop && m_stop->load(std::memory_order_acquire);
}

/**
 * @brief Implements apiCall(method, params) by blocking onto the GUI-thread marshaller;
 *        refuses once Stop was pressed and re-arms the watchdog after the call.
 */
QVariantMap DataModel::MacroApiBridge::call(const QString& method, const QVariantMap& params)
{
  QVariantMap out;
  if (method.isEmpty()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: method must not be empty"));
    return out;
  }

  if (stopRequested()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: macro stop requested"));
    return out;
  }

  const bool ok = QMetaObject::invokeMethod(m_marshaller,
                                            "dispatch",
                                            Qt::BlockingQueuedConnection,
                                            Q_RETURN_ARG(QVariantMap, out),
                                            Q_ARG(QString, method),
                                            Q_ARG(QVariantMap, params));
  if (m_watchdog && !stopRequested())
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
QVariantList DataModel::MacroApiBridge::listCommands()
{
  QVariantList result;
  const auto& commands = m_registry.commands();
  result.reserve(commands.size());
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it)
    result.append(it.key());

  return result;
}

/**
 * @brief Writes @p data to @p sourceId then blocks the worker (never the GUI) until the reply
 *        satisfies @p until or @p timeoutMs elapses; Stop-sliced, watchdog re-armed after.
 */
QVariantMap DataModel::MacroApiBridge::writeAndWait(const QJSValue& data,
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

  if (stopRequested()) {
    out.insert(QStringLiteral("error"), QStringLiteral("deviceWriteAndWait: macro stop requested"));
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
    if (m_watchdog && !stopRequested())
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

  const int budget = qBound(0, timeoutMs, kMacroMaxReplyWaitMs);
  QByteArray reply;
  bool satisfied = false;
  for (int waited = 0; waited <= budget && !satisfied; waited += kMacroReplyPollSliceMs) {
    if (stopRequested())
      break;

    QThread::msleep(static_cast<unsigned long>(kMacroReplyPollSliceMs));
    QMetaObject::invokeMethod(m_marshaller,
                              "pollReply",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QByteArray, reply),
                              Q_ARG(int, sourceId));
    satisfied = satisfies(reply);
  }

  QMetaObject::invokeMethod(
    m_marshaller, "disarmReply", Qt::BlockingQueuedConnection, Q_ARG(int, sourceId));

  if (m_watchdog && !stopRequested())
    m_watchdog->arm();

  out.insert(QStringLiteral("ok"), satisfied);
  out.insert(QStringLiteral("data"), QString::fromUtf8(reply));
  out.insert(QStringLiteral("bytesRead"), reply.size());
  out.insert(QStringLiteral("timedOut"), !satisfied);
  return out;
}

/**
 * @brief Streams a print()/console.log() line back to the GUI scrollback.
 */
void DataModel::MacroApiBridge::log(const QString& message)
{
  Q_EMIT logMessage(message);
}

/**
 * @brief Blocks the worker thread for the given time (capped at one hour), sleeping in slices
 *        so Stop can interrupt the wait, then defers the watchdog deadline.
 */
void DataModel::MacroApiBridge::delay(int milliseconds)
{
  const int total = qBound(0, milliseconds, kMacroMaxDelayMs);
  for (int slept = 0; slept < total; slept += kMacroDelaySliceMs) {
    if (stopRequested())
      return;

    QThread::msleep(static_cast<unsigned long>(qMin(kMacroDelaySliceMs, total - slept)));
  }

  if (m_watchdog && !stopRequested())
    m_watchdog->arm();
}

//--------------------------------------------------------------------------------------------------
// Macro worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the worker around a GUI-thread marshaller owned by the caller, which must
 *        outlive the worker thread.
 */
DataModel::MacroWorker::MacroWorker(ControlApiMarshaller* marshaller, QObject* parent)
  : QObject(parent)
  , m_teardown(false)
  , m_bridge(nullptr)
  , m_stopRequested(false)
  , m_marshaller(marshaller)
{
  SS_ASSERT_LOG(marshaller != nullptr);
}

/**
 * @brief Destructor; engine and watchdog are torn down on the worker thread.
 */
DataModel::MacroWorker::~MacroWorker()
{
  releaseEngine();
}

/**
 * @brief Clears the per-run stop flag; called from the GUI thread before queueing a run so a
 *        Stop pressed after busyChanged is never erased by the worker (the flag is reset by
 *        the producer of both writes, making their order program order).
 */
void DataModel::MacroWorker::resetStop() noexcept
{
  m_stopRequested.store(false, std::memory_order_release);
}

/**
 * @brief Requests a stop from any thread: bridge touch-points abort promptly, and the armed
 *        watchdog bounds a pure-JS spin (worst-case stop latency is the JS slice budget).
 */
void DataModel::MacroWorker::requestStop() noexcept
{
  m_stopRequested.store(true, std::memory_order_release);
}

/**
 * @brief Latches teardown from the destructor: never cleared, so a run() already queued when
 *        the owner dies bails at entry instead of erasing the stop request.
 */
void DataModel::MacroWorker::requestTeardown() noexcept
{
  m_teardown.store(true, std::memory_order_release);
  m_stopRequested.store(true, std::memory_order_release);
}

/**
 * @brief Evaluates one macro in a fresh engine: SDK prelude + the marshal bridge ONLY (direct
 *        helper bridges wrap main-thread singletons and must never execute on this thread);
 *        console output is routed through the bridge log() shim, and the engine is released
 *        after every run so no state survives into the next one.
 */
void DataModel::MacroWorker::run(const QString& source)
{
  if (m_teardown.load(std::memory_order_acquire))
    return;

  if (source.trimmed().isEmpty()) {
    Q_EMIT scriptError(QStringLiteral("Macro is empty"));
    return;
  }

  m_engine = std::make_unique<QJSEngine>();
  m_engine->installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
  m_watchdog = std::make_unique<DataModel::JsWatchdog>(
    m_engine.get(), kMacroWatchdogMs, QStringLiteral("Macro"));

  m_bridge = new MacroApiBridge(m_marshaller, &m_stopRequested, m_engine.get());
  m_bridge->setWatchdog(m_watchdog.get());
  connect(m_bridge, &MacroApiBridge::logMessage, this, &MacroWorker::logMessage);

  auto bridge_value = m_engine->newQObject(m_bridge);
  m_engine->globalObject().setProperty(QStringLiteral("__ss_bridge"), bridge_value);
  m_engine->globalObject().setProperty(QStringLiteral("__ss_control"), QJSValue(true));

  QFile sdk_file(QStringLiteral(":/api/SerialStudio.js"));
  if (sdk_file.open(QFile::ReadOnly))
    m_engine->evaluate(QString::fromUtf8(sdk_file.readAll()));

  m_engine->evaluate(QStringLiteral("(function() {"
                                    "  var ssLog = function() {"
                                    "    var parts = [];"
                                    "    for (var i = 0; i < arguments.length; ++i)"
                                    "      parts.push(String(arguments[i]));"
                                    "    __ss_bridge.log(parts.join(' '));"
                                    "  };"
                                    "  console.log = ssLog;"
                                    "  console.info = ssLog;"
                                    "  console.warn = ssLog;"
                                    "  console.error = ssLog;"
                                    "  globalThis.print = ssLog;"
                                    "})();"));

  m_watchdog->arm();
  const QJSValue result = m_engine->evaluate(source, QStringLiteral("macro.js"));
  m_watchdog->disarm();

  if (m_engine->isInterrupted()) {
    m_engine->setInterrupted(false);
    const bool stopped = m_stopRequested.load(std::memory_order_acquire);
    Q_EMIT scriptError(stopped
                         ? QStringLiteral("Macro stopped by user")
                         : QStringLiteral("Macro exceeded the %1 ms script budget between API "
                                          "calls and was interrupted")
                             .arg(kMacroWatchdogMs));
    releaseEngine();
    return;
  }

  if (result.isError()) {
    Q_EMIT scriptError(QStringLiteral("Line %1: %2")
                         .arg(result.property(QStringLiteral("lineNumber")).toInt())
                         .arg(result.toString()));
    releaseEngine();
    return;
  }

  Q_EMIT finished(result.isUndefined() || result.isNull() ? QString() : result.toString());
  releaseEngine();
}

/**
 * @brief Drops the bridge, watchdog, and engine in that order; the watchdog must unregister
 *        before the engine it monitors is destroyed.
 */
void DataModel::MacroWorker::releaseEngine()
{
  m_bridge = nullptr;
  m_watchdog.reset();
  m_engine.reset();
}
