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

#include "DataModel/Scripting/MacroRunner.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
}
// clang-format on

#include <QCoreApplication>
#include <QDeadlineTimer>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJSEngine>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <stdexcept>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/DataTable.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Scripting/ControlScriptWorker.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "DataModel/Scripting/MacroWorker.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "DataModel/Scripting/ScriptDryRun.h"
#include "IO/PipelineHost.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kLuaDeadlineMs            = 30000;
static constexpr int kLuaHookInstructions      = 10000;
static constexpr int kMacroShutdownWaitSlices  = 140;
static constexpr int kMacroShutdownWaitSliceMs = 50;

//--------------------------------------------------------------------------------------------------
// Lua glue (GUI thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deadline + owner handles the Lua watchdog hook and print capture read back through
 *        the Lua registry.
 */
struct MacroLuaContext {
  DataModel::MacroRunner* runner;
  QDeadlineTimer deadline;
};

/**
 * @brief Removes the ffi and jit modules LuaJIT's luaL_openlibs installs: ffi is a full
 *        sandbox escape and jit is engine control, and neither is exposed to user scripts in
 *        any execution mode (spec 0051 constraint) -- macros included.
 */
static void stripJitOnlyLibs(lua_State* L)
{
  for (const char* name : {"ffi", "jit"}) {
    lua_pushnil(L);
    lua_setglobal(L, name);
    lua_getglobal(L, "package");
    if (lua_istable(L, -1)) {
      lua_getfield(L, -1, "loaded");
      if (lua_istable(L, -1)) {
        lua_pushnil(L);
        lua_setfield(L, -2, name);
      }
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  }
}

/**
 * @brief Lua atpanic handler that throws so abort() is never reached.
 */
static int macroLuaPanic(lua_State* L)
{
  const char* msg = lua_tostring(L, -1);
  throw std::runtime_error(msg ? msg : "lua panic");
}

/**
 * @brief Reads the MacroLuaContext pointer stored in the Lua registry.
 */
static MacroLuaContext* macroLuaContext(lua_State* L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_macro_ctx__");
  auto* ctx = static_cast<MacroLuaContext*>(lua_touserdata(L, -1));
  lua_pop(L, 1);
  return ctx;
}

/**
 * @brief Lua debug hook that aborts the macro when the safety deadline expires.
 */
static void macroLuaHook(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)

  auto* ctx = macroLuaContext(L);
  if (!ctx) [[unlikely]]
    return;

  if (ctx->deadline.hasExpired()) [[unlikely]]
    luaL_error(L, "macro exceeded the %d ms safety deadline", kLuaDeadlineMs);
}

/**
 * @brief Replacement print() that streams each line into the terminal scrollback.
 */
static int macroLuaPrint(lua_State* L)
{
  auto* ctx = macroLuaContext(L);
  if (!ctx || !ctx->runner)
    return 0;

  QStringList parts;
  const int argc = lua_gettop(L);
  for (int i = 1; i <= argc; ++i) {
    const char* text = luaL_tolstring(L, i, nullptr);
    parts.append(QString::fromUtf8(text ? text : ""));
    lua_pop(L, 1);
  }

  ctx->runner->postLog(parts.join(QStringLiteral("\t")));
  return 0;
}

//--------------------------------------------------------------------------------------------------
// Construction & teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the marshaller + worker pair and starts the macro worker thread; constructed
 *        by QML long after the composition root, so every singleton it captures exists.
 */
DataModel::MacroRunner::MacroRunner(QObject* parent)
  : QObject(parent)
  , m_busy(false)
  , m_jsRunning(false)
  , m_thread(nullptr)
  , m_worker(nullptr)
  , m_marshaller(nullptr)
  , m_frameBuilder(DataModel::FrameBuilder::instance())
{
  m_thread     = new QThread;
  m_marshaller = new ControlApiMarshaller(this);
  m_worker     = new MacroWorker(m_marshaller);
  m_worker->moveToThread(m_thread);

  connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
  connect(m_worker, &MacroWorker::finished, this, &MacroRunner::onWorkerFinished);
  connect(m_worker, &MacroWorker::scriptError, this, &MacroRunner::onWorkerError);
  connect(m_worker, &MacroWorker::logMessage, this, &MacroRunner::logMessage);

  m_thread->start();
}

/**
 * @brief Joins the worker: latched teardown + sliced wait sized above the JS watchdog budget,
 *        draining the blocking marshaller each slice (ControlScript shutdown pattern). A
 *        worker that refuses to finish is abandoned with a warning; the thread and marshaller
 *        are deliberately leaked because destroying a running QThread aborts the process.
 */
DataModel::MacroRunner::~MacroRunner()
{
  if (m_worker)
    m_worker->requestTeardown();

  m_thread->quit();
  for (int i = 0; i < kMacroShutdownWaitSlices; ++i) {
    if (m_thread->wait(kMacroShutdownWaitSliceMs))
      break;

    if (qApp)
      QCoreApplication::sendPostedEvents(m_marshaller);
  }

  if (m_thread->isRunning()) {
    qWarning() << "[MacroRunner] Worker thread did not stop within the shutdown budget";
    m_marshaller->setParent(nullptr);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);
    return;
  }

  delete m_thread;
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a macro is currently executing.
 */
bool DataModel::MacroRunner::busy() const noexcept
{
  return m_busy;
}

/**
 * @brief Returns whether Stop can interrupt the current run (JS runs only; Lua v1 blocks).
 */
bool DataModel::MacroRunner::canStop() const noexcept
{
  return m_busy && m_jsRunning;
}

/**
 * @brief Streams a Lua print() line into the terminal scrollback (called by the Lua glue,
 *        which cannot emit the protected signal itself).
 */
void DataModel::MacroRunner::postLog(const QString& message)
{
  Q_EMIT logMessage(message);
}

//--------------------------------------------------------------------------------------------------
// Execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Queues a JS macro onto the worker thread; the GUI never blocks on the run. The stop
 *        flag is reset here (GUI side, before the queue) so a Stop pressed after busyChanged
 *        can never be erased by the worker's entry.
 */
void DataModel::MacroRunner::runJs(const QString& source)
{
  if (m_busy)
    return;

  m_worker->resetStop();
  m_busy      = true;
  m_jsRunning = true;
  Q_EMIT busyChanged();

  const bool queued =
    QMetaObject::invokeMethod(m_worker, "run", Qt::QueuedConnection, Q_ARG(QString, source));
  SS_ASSERT(queued, {
    m_busy      = false;
    m_jsRunning = false;
    Q_EMIT busyChanged();
    Q_EMIT scriptError(tr("Failed to queue the macro onto the worker thread"));
    return;
  });
}

/**
 * @brief Runs a Lua macro GUI-synchronously (documented v1 behavior): fresh state, full SDK
 *        via the GUI-thread bridges, MASKCOUNT hook + deadline as the anti-hang bound.
 */
void DataModel::MacroRunner::runLua(const QString& source)
{
  if (m_busy)
    return;

  if (source.trimmed().isEmpty()) {
    Q_EMIT scriptError(tr("Macro is empty"));
    return;
  }

  m_busy      = true;
  m_jsRunning = false;
  Q_EMIT busyChanged();

  lua_State* state = luaL_newstate();
  SS_ASSERT(state != nullptr, {
    m_busy = false;
    Q_EMIT busyChanged();
    Q_EMIT scriptError(tr("Failed to create the Lua state"));
    return;
  });

  lua_atpanic(state, macroLuaPanic);

  MacroLuaContext context;
  context.runner   = this;
  context.deadline = QDeadlineTimer(kLuaDeadlineMs);

  QString error;
  QString result;
  bool ok = false;
  try {
    luaL_openlibs(state);
    stripJitOnlyLibs(state);
    luaJIT_setmode(state, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
    DataModel::installLuaConsole(state);
    DataModel::installLuaCompat(state);
    DataModel::ScriptApiCall::installAll(state, 0);

    lua_pushlightuserdata(state, &context);
    lua_setfield(state, LUA_REGISTRYINDEX, "__ss_macro_ctx__");
    lua_pushcfunction(state, macroLuaPrint);
    lua_setglobal(state, "print");
    lua_sethook(state, macroLuaHook, LUA_MASKCOUNT, kLuaHookInstructions);

    const QByteArray utf8 = source.toUtf8();
    if (luaL_loadbuffer(state, utf8.constData(), static_cast<size_t>(utf8.size()), "macro")
        != LUA_OK) {
      const char* msg = lua_tostring(state, -1);
      error           = QString::fromUtf8(msg ? msg : "syntax error");
    } else if (lua_pcall(state, 0, LUA_MULTRET, 0) != LUA_OK) {
      const char* msg = lua_tostring(state, -1);
      error           = QString::fromUtf8(msg ? msg : "runtime error");
    } else {
      ok = true;
    }

    if (ok && lua_gettop(state) > 0) {
      const char* text = luaL_tolstring(state, 1, nullptr);
      result           = QString::fromUtf8(text ? text : "");
      lua_pop(state, 1);
    }
  } catch (const std::exception& e) {
    ok    = false;
    error = QString::fromUtf8(e.what());
  } catch (...) {
    ok    = false;
    error = tr("Unknown Lua error");
  }

  closeMacroLuaState(state);

  m_busy = false;
  Q_EMIT busyChanged();
  if (ok)
    Q_EMIT finished(result);
  else
    Q_EMIT scriptError(error);
}

/**
 * @brief Requests interruption of the running JS macro; bridge touch-points abort promptly
 *        and the watchdog bounds a pure-JS spin.
 */
void DataModel::MacroRunner::stop()
{
  if (m_worker && canStop())
    m_worker->requestStop();
}

//--------------------------------------------------------------------------------------------------
// Verification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compile-only check (SerialStudio::ScriptLanguage encoding): parses in a throwaway
 *        engine, never executes.
 */
QVariantMap DataModel::MacroRunner::verify(const QString& source, int language) const
{
  return (language == SerialStudio::Lua) ? verifyLua(source) : verifyJs(source);
}

/**
 * @brief Parses JS by evaluating a function-expression wrapper, so the macro body is compiled
 *        but never run; the wrapper adds one line, subtracted from reported line numbers, and the
 *        error text carries the line in the same "Line N:" shape as a run-time error. The
 *        evaluation is deadline-guarded because a source that closes the wrapper early does run.
 */
QVariantMap DataModel::MacroRunner::verifyJs(const QString& source) const
{
  ScriptDryRun session(ScriptDryRun::Language::JavaScript, kScriptDryRunBudgetMs, "macro.verify");

  QVariantMap out;
  if (!session.valid()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), tr("Failed to create the JavaScript engine"));
    return out;
  }

  const QJSValue result =
    session.evaluate(QStringLiteral("(function() {\n%1\n})").arg(source), QStringLiteral("macro"));
  if (session.timedOut()) {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"),
               tr("The macro did not finish compiling within %1 ms").arg(session.budgetMs()));
    return out;
  }

  out.insert(QStringLiteral("ok"), !result.isError());
  if (result.isError()) {
    const int source_lines = static_cast<int>(source.count(QLatin1Char('\n'))) + 1;
    const int line =
      qBound(1, result.property(QStringLiteral("lineNumber")).toInt() - 1, source_lines);
    out.insert(QStringLiteral("line"), line);
    out.insert(QStringLiteral("error"),
               tr("Line %1: %2").arg(QString::number(line), result.toString()));
  }

  return out;
}

/**
 * @brief Parses Lua with luaL_loadbuffer (loads, never calls) in a bare throwaway state.
 */
QVariantMap DataModel::MacroRunner::verifyLua(const QString& source) const
{
  QVariantMap out;
  lua_State* state = luaL_newstate();
  SS_ASSERT(state != nullptr, {
    out.insert(QStringLiteral("ok"), false);
    out.insert(QStringLiteral("error"), tr("Failed to create the Lua state"));
    return out;
  });

  const QByteArray utf8 = source.toUtf8();
  const int status =
    luaL_loadbuffer(state, utf8.constData(), static_cast<size_t>(utf8.size()), "macro");

  out.insert(QStringLiteral("ok"), status == LUA_OK);
  if (status != LUA_OK) {
    const char* msg     = lua_tostring(state, -1);
    const QString error = QString::fromUtf8(msg ? msg : "syntax error");
    out.insert(QStringLiteral("error"), error);

    static const QRegularExpression line_re(QStringLiteral(":(\\d+):"));
    const auto match = line_re.match(error);
    if (match.hasMatch())
      out.insert(QStringLiteral("line"), match.captured(1).toInt());
  }

  lua_close(state);
  return out;
}

//--------------------------------------------------------------------------------------------------
// Macro files
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the default macro directory (<AppData>/Macros), created on first use.
 */
QString DataModel::MacroRunner::macrosDirectory() const
{
  const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  const QString dir  = base + QStringLiteral("/Macros");
  QDir().mkpath(dir);
  return dir;
}

/**
 * @brief Opens a macro file; the read is deferred out of fileSelected so the dialog's done()
 *        unwinds first (macOS reentrancy rule), and the language follows the file suffix.
 */
void DataModel::MacroRunner::loadMacro()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select macro to load"), macrosDirectory(), QStringLiteral("*.js *.lua"));
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() {
        QFile file(path);
        if (!file.open(QFile::ReadOnly)) {
          Q_EMIT scriptError(tr("Cannot open %1").arg(path));
          return;
        }

        const int language = path.endsWith(QStringLiteral(".lua"), Qt::CaseInsensitive)
                             ? SerialStudio::Lua
                             : SerialStudio::JavaScript;
        Q_EMIT macroLoaded(QString::fromUtf8(file.readAll()), language, QFileInfo(path).fileName());
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Saves the macro text; the write is deferred out of fileSelected (macOS reentrancy
 *        rule) and the default suffix follows the selected language.
 */
void DataModel::MacroRunner::saveMacro(const QString& source, int language)
{
  const bool lua = (language == SerialStudio::Lua);
  auto* dialog   = new QFileDialog(nullptr,
                                 tr("Save macro"),
                                 macrosDirectory(),
                                 lua ? QStringLiteral("*.lua") : QStringLiteral("*.js"));
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setDefaultSuffix(lua ? QStringLiteral("lua") : QStringLiteral("js"));
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this, source](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, source, path]() {
        QSaveFile file(path);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
          Q_EMIT scriptError(tr("Cannot write %1").arg(path));
          return;
        }

        const QByteArray payload = source.toUtf8();
        if (file.write(payload) != payload.size() || !file.commit()) {
          Q_EMIT scriptError(tr("Failed to save %1: %2").arg(path, file.errorString()));
          return;
        }

        Q_EMIT macroSaved(QFileInfo(path).fileName());
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

//--------------------------------------------------------------------------------------------------
// Worker results
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes a successful JS run and clears the busy state.
 */
void DataModel::MacroRunner::onWorkerFinished(const QString& result)
{
  m_busy      = false;
  m_jsRunning = false;
  Q_EMIT busyChanged();
  Q_EMIT finished(result);
}

/**
 * @brief Publishes a failed or interrupted JS run and clears the busy state.
 */
void DataModel::MacroRunner::onWorkerError(const QString& message)
{
  m_busy      = false;
  m_jsRunning = false;
  Q_EMIT busyChanged();
  Q_EMIT scriptError(message);
}

/**
 * @brief Closes a macro Lua state after invalidating the table-store lookup cache the
 *        injected table API may have populated (the closeLuaState rule). Skipped during
 *        teardown, when no event loop is left to carry the marshal.
 */
void DataModel::MacroRunner::closeMacroLuaState(lua_State* state)
{
  if (!state)
    return;

  if (!IO::PipelineHost::tearingDown())
    m_frameBuilder.invokeOnBuilderThreadBlocking(
      [this] { m_frameBuilder.tableStore().clearLookupCache(); });

  lua_close(state);
}
