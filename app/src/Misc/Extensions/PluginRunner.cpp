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

#include "Misc/Extensions/PluginRunner.h"

#include <QDir>
#include <QFile>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include "SSAssert.h"
#include "UI/Dashboard.h"

/**
 * @brief Output ring cap. A chatty plugin left running for hours would otherwise grow its
 *        buffer without bound; on overflow the newest half is kept.
 */
static constexpr int kMaxOutputChars  = 65536;
static constexpr int kKeptOutputChars = 32768;

/**
 * @brief Milliseconds a terminating process is given before it is killed.
 */
static constexpr int kTerminateGraceMs = 3000;

/**
 * @brief Constructs an idle runner holding no processes.
 */
Misc::PluginRunner::PluginRunner(QObject* parent) : QObject(parent) {}

/**
 * @brief Returns whether the plugin with the given ID has a live process.
 */
bool Misc::PluginRunner::isRunning(const QString& id) const
{
  return m_processes.contains(id);
}

/**
 * @brief Returns the captured stdout/stderr output for the given plugin.
 */
QString Misc::PluginRunner::output(const QString& id) const
{
  return m_output.value(id, QString());
}

/**
 * @brief Returns whether the user closed this plugin themselves, which is what stops the
 *        session restore from relaunching it behind their back.
 */
bool Misc::PluginRunner::userClosed(const QString& id) const
{
  return m_userClosed.contains(id);
}

/**
 * @brief Returns the running-plugin list in the shape the dashboard consumes.
 */
const QVariantList& Misc::PluginRunner::running() const noexcept
{
  return m_running;
}

/**
 * @brief Returns the plugin IDs that were running when the previous session ended.
 */
QStringList Misc::PluginRunner::restorableIds() const
{
  return m_settings.value("RunningPlugins").toStringList();
}

/**
 * @brief Appends a line to a plugin's log. Public because ExtensionManager reports its own
 *        validation failures ("not installed", "no entry point") into the same log the user
 *        reads, and that log lives here.
 */
void Misc::PluginRunner::appendOutput(const QString& id, const QString& text)
{
  m_output[id] += text;
  Q_EMIT outputChanged(id);
}

/**
 * @brief Clears the user-closed mark so an explicit launch overrides an earlier close.
 */
void Misc::PluginRunner::clearUserClosed(const QString& id)
{
  m_userClosed.remove(id);
}

/**
 * @brief Spawns the plugin process and records it. Returns false when the process refuses to
 *        start, in which case the failure is already in the plugin's log.
 */
bool Misc::PluginRunner::start(const QString& id,
                               const QString& pluginDir,
                               const QString& runtime,
                               const QString& entryPath,
                               const QString& title,
                               const bool terminal,
                               const bool hasPipDeps)
{
  SS_ASSERT(!id.isEmpty(), return false);
  SS_ASSERT(!isRunning(id), return false);

  auto* process = new QProcess(this);
  process->setWorkingDirectory(pluginDir);
  process->setProcessEnvironment(buildEnvironment());

  if (terminal)
    process->setProcessChannelMode(QProcess::ForwardedChannels);
  else {
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setStandardInputFile(QProcess::nullDevice());
  }

  m_output.insert(id, QString());
  wireProcess(process, id);
  startProcess(process, runtime, entryPath, terminal);

  if (!process->waitForStarted(kTerminateGraceMs)) {
    appendOutput(id, QStringLiteral("[Error] Failed to start: ") + process->errorString() + "\n");
    delete process;
    return false;
  }

  const auto mode = terminal ? QStringLiteral(" (terminal)") : QString();
  appendOutput(
    id, QStringLiteral("[Started] PID ") + QString::number(process->processId()) + mode + "\n");

  if (hasPipDeps && !QDir(pluginDir + "/venv").exists())
    appendOutput(
      id, QStringLiteral("[Setup] Installing required packages -- this may take a moment...\n"));

  m_processes.insert(id, process);

  QVariantMap entry;
  entry.insert("id", id);
  entry.insert("title", title);
  m_running.append(entry);
  Q_EMIT runningChanged();
  return true;
}

/**
 * @brief Stops a running plugin by ID.
 */
void Misc::PluginRunner::stop(const QString& id)
{
  auto it = m_processes.find(id);
  if (it == m_processes.end())
    return;

  static auto& dashboard = UI::Dashboard::instance();
  if (dashboard.available())
    m_userClosed.insert(id);

  auto* process = it.value();
  appendOutput(id, QStringLiteral("[Stopping...]\n"));

  process->disconnect(this);
  m_processes.erase(it);

  process->terminate();
  if (!process->waitForFinished(kTerminateGraceMs))
    process->kill();

  const auto remaining = QString::fromUtf8(process->readAll());
  if (!remaining.isEmpty())
    appendOutput(id, remaining);

  appendOutput(id, QStringLiteral("[Stopped]\n"));
  delete process;

  forget(id);
  Q_EMIT runningChanged();
}

/**
 * @brief Stops every running plugin, recording the set so the next session can restore it.
 *        Called on shutdown and whenever the dashboard goes away.
 */
void Misc::PluginRunner::stopAll()
{
  const auto ids = m_processes.keys();
  m_settings.setValue("RunningPlugins", QStringList(ids));

  for (const auto& id : ids) {
    auto* process = m_processes.value(id);
    if (!process)
      continue;

    process->disconnect(this);
    process->terminate();
    if (!process->waitForFinished(kTerminateGraceMs))
      process->kill();

    delete process;
  }

  m_processes.clear();
  m_running.clear();
  Q_EMIT runningChanged();
}

/**
 * @brief Handles a plugin process exiting on its own.
 */
void Misc::PluginRunner::onFinished(const QString& id)
{
  auto it = m_processes.find(id);
  if (it == m_processes.end())
    return;

  static auto& dashboard = UI::Dashboard::instance();
  if (dashboard.available())
    m_userClosed.insert(id);

  auto* process        = it.value();
  const auto remaining = QString::fromUtf8(process->readAll());
  if (!remaining.isEmpty())
    appendOutput(id, remaining);

  appendOutput(id, QStringLiteral("[Exited with code %1]\n").arg(process->exitCode()));

  m_processes.erase(it);
  process->deleteLater();

  forget(id);
  Q_EMIT runningChanged();
}

/**
 * @brief Drops a plugin from the running list.
 */
void Misc::PluginRunner::forget(const QString& id)
{
  for (int i = 0; i < m_running.count(); ++i) {
    if (m_running.at(i).toMap().value("id").toString() == id) {
      m_running.removeAt(i);
      return;
    }
  }
}

/**
 * @brief Connects readyRead/finished/errorOccurred handlers for a plugin's QProcess.
 */
void Misc::PluginRunner::wireProcess(QProcess* process, const QString& id)
{
  SS_ASSERT(process != nullptr, return);

  connect(process, &QProcess::readyRead, this, [this, id, process]() {
    m_output[id] += QString::fromUtf8(process->readAll());

    auto& buf = m_output[id];
    if (buf.size() > kMaxOutputChars)
      buf = buf.right(kKeptOutputChars);

    Q_EMIT outputChanged(id);
  });

  connect(process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this,
          [this, id]() { onFinished(id); });

  connect(process, &QProcess::errorOccurred, this, [this, id](QProcess::ProcessError err) {
    Q_UNUSED(err)
    appendOutput(id, QStringLiteral("[Error] Plugin failed to start or crashed.\n"));
  });
}

/**
 * @brief Builds the QProcessEnvironment for plugin processes, prepending common tool paths.
 */
QProcessEnvironment Misc::PluginRunner::buildEnvironment() const
{
  auto env  = QProcessEnvironment::systemEnvironment();
  auto path = env.value("PATH");
#ifdef Q_OS_MACOS
  const QStringList extraPaths = {
    "/opt/homebrew/bin",
    "/opt/homebrew/sbin",
    "/usr/local/bin",
    QDir::homePath() + "/.local/bin",
  };

  for (const auto& p : extraPaths)
    if (!path.contains(p) && QDir(p).exists())
      path = p + ":" + path;
#endif
  env.insert("PATH", path);
  env.insert("TK_SILENCE_DEPRECATION", "1");
  return env;
}

/**
 * @brief Starts a plugin process directly or via the platform-native terminal emulator.
 */
void Misc::PluginRunner::startProcess(QProcess* process,
                                      const QString& runtime,
                                      const QString& entryPath,
                                      const bool terminal)
{
  const bool isNative = runtime.isEmpty();
#ifndef Q_OS_WIN
  if (isNative)
    QFile::setPermissions(
      entryPath, QFile::permissions(entryPath) | QFileDevice::ExeUser | QFileDevice::ExeGroup);
#endif

  if (!terminal) {
    if (isNative)
      process->start(entryPath);
    else
      process->start(runtime, {entryPath});

    return;
  }

#ifdef Q_OS_MACOS
  Q_UNUSED(isNative)
  process->start("open", {"-a", "Terminal", entryPath});
#elif defined(Q_OS_WIN)
  if (isNative)
    process->start("cmd.exe", {"/c", "start", "cmd.exe", "/k", entryPath});
  else
    process->start("cmd.exe", {"/c", "start", "cmd.exe", "/k", runtime, entryPath});
#else
  const QStringList terms = {
    "x-terminal-emulator", "gnome-terminal", "konsole", "xfce4-terminal", "xterm"};
  for (const auto& term : terms) {
    if (QStandardPaths::findExecutable(term).isEmpty())
      continue;

    if (isNative)
      process->start(term, {"--", entryPath});
    else
      process->start(term, {"--", runtime, entryPath});

    return;
  }

  if (isNative)
    process->start(entryPath);
  else
    process->start(runtime, {entryPath});
#endif
}
