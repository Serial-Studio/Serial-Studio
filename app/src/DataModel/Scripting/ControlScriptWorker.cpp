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

#include <QCoreApplication>
#include <QFile>
#include <QJSEngine>
#include <QJsonObject>
#include <QThread>
#include <QTimer>
#include <QUuid>

#include "API/CommandHandler.h"
#include "API/CommandProtocol.h"
#include "API/CommandRegistry.h"
#include "DataModel/Scripting/JsWatchdog.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kRuntimeWatchdogMs = 2000;

//--------------------------------------------------------------------------------------------------
// GUI-thread API marshaller
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the GUI-thread API marshaller.
 */
DataModel::ControlApiMarshaller::ControlApiMarshaller(QObject* parent) : QObject(parent) {}

/**
 * @brief Runs an apiCall on the GUI thread so handlers never touch GUI objects off-thread.
 */
QVariantMap DataModel::ControlApiMarshaller::dispatch(const QString& method,
                                                      const QVariantMap& params)
{
  API::CommandRequest request;
  request.id      = QUuid::createUuid().toString(QUuid::WithoutBraces);
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
 * @brief Implements apiCall(method, params) by blocking onto the GUI-thread marshaller.
 */
QVariantMap DataModel::ControlApiBridge::call(const QString& method, const QVariantMap& params)
{
  QVariantMap out;
  if (method.isEmpty()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), QStringLiteral("apiCall: method must not be empty"));
    return out;
  }

  const bool ok = QMetaObject::invokeMethod(m_marshaller,
                                            "dispatch",
                                            Qt::BlockingQueuedConnection,
                                            Q_RETURN_ARG(QVariantMap, out),
                                            Q_ARG(QString, method),
                                            Q_ARG(QVariantMap, params));
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
 * @brief Blocks the worker thread for the given time, deferring the watchdog deadline.
 */
void DataModel::ControlApiBridge::delay(int milliseconds)
{
  if (milliseconds <= 0)
    return;

  QThread::msleep(static_cast<unsigned long>(milliseconds));

  if (m_watchdog)
    m_watchdog->arm();
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the worker and the GUI-thread marshaller it dispatches through.
 */
DataModel::ControlScriptWorker::ControlScriptWorker(QObject* parent)
  : QObject(parent), m_loaded(false), m_loopTimer(nullptr), m_bridge(nullptr), m_marshaller(nullptr)
{
  m_marshaller = new ControlApiMarshaller();
  m_marshaller->moveToThread(QCoreApplication::instance()->thread());
  m_marshaller->setParent(QCoreApplication::instance());
}

/**
 * @brief Destructor; engine and timer are torn down on the worker thread.
 */
DataModel::ControlScriptWorker::~ControlScriptWorker()
{
  stop();
  if (m_marshaller)
    m_marshaller->deleteLater();
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

    m_loopTimer->start(0);
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
  m_bridge  = nullptr;
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
    m_loopTimer->start(0);
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

  if (result.isError())
    Q_EMIT scriptError(QStringLiteral("setup() line %1: %2")
                         .arg(result.property(QStringLiteral("lineNumber")).toInt())
                         .arg(result.toString()));
}

//--------------------------------------------------------------------------------------------------
// Compilation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the engine, installs the marshalling apiCall, and grabs setup()/loop().
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

  QFile sdkFile(QStringLiteral(":/api/SerialStudio.js"));
  if (sdkFile.open(QFile::ReadOnly))
    m_engine->evaluate(QString::fromUtf8(sdkFile.readAll()));

  const auto result = m_engine->evaluate(source, QStringLiteral("control-script.js"));
  if (result.isError()) {
    errorOut = QStringLiteral("Line %1: %2")
                 .arg(result.property(QStringLiteral("lineNumber")).toInt())
                 .arg(result.toString());
    m_engine.reset();
    m_watchdog.reset();
    return false;
  }

  m_setupFn = m_engine->globalObject().property(QStringLiteral("setup"));
  m_loopFn  = m_engine->globalObject().property(QStringLiteral("loop"));

  if (!m_setupFn.isCallable() && !m_loopFn.isCallable()) {
    errorOut = QStringLiteral("Script must define setup() and/or loop().");
    m_engine.reset();
    m_watchdog.reset();
    return false;
  }

  m_loaded = true;
  return true;
}
