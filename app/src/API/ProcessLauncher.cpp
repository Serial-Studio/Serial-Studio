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

#include "API/ProcessLauncher.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPointer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTimer>
#include <QVariantMap>

#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "SSAssert.h"

static constexpr int kTerminateGraceMs = 2000;

/**
 * @brief Constructs the launcher with an empty process table.
 */
API::ProcessLauncher::ProcessLauncher(QObject* parent)
  : QObject(parent), m_nextId(1), m_connectionManager(nullptr), m_projectModel(nullptr)
{}

/**
 * @brief Returns the singleton instance.
 */
API::ProcessLauncher& API::ProcessLauncher::instance()
{
  static ProcessLauncher singleton;
  return singleton;
}

/**
 * @brief Wires lifecycle signals so every spawned helper is reaped on disconnect, project
 *        change, and application quit.
 */
void API::ProcessLauncher::setupExternalConnections()
{
  m_connectionManager = &IO::ConnectionManager::instance();
  m_projectModel      = &DataModel::ProjectModel::instance();

  m_lastProjectPath = m_projectModel->jsonFilePath();

  connect(m_connectionManager,
          &IO::ConnectionManager::sessionClosed,
          this,
          &API::ProcessLauncher::killAll);
  connect(m_projectModel,
          &DataModel::ProjectModel::jsonFileChanged,
          this,
          &API::ProcessLauncher::onProjectFileChanged);
  connect(qApp, &QCoreApplication::aboutToQuit, this, &API::ProcessLauncher::onAboutToQuit);
}

/**
 * @brief Spawns a helper process; returns its id, or -1 with @p error set on failure. A helper
 *        identical to one still running (same executable, arguments, and working directory) is
 *        not spawned again: reconnects re-run the script's onConnect() hook, and a duplicate
 *        server helper only steals the first one's port and dies on bind.
 */
int API::ProcessLauncher::launch(const QString& program,
                                 const QStringList& arguments,
                                 const QString& workingDirectory,
                                 QString& error)
{
  if (program.isEmpty()) {
    error = tr("No program specified");
    return -1;
  }

  const QString resolved = resolveExecutable(program);
  if (resolved.isEmpty()) {
    error = tr("Program \"%1\" not found in PATH").arg(program);
    return -1;
  }

  for (auto it = m_processes.constBegin(); it != m_processes.constEnd(); ++it) {
    auto* existing = it.value();
    if (existing && existing->state() != QProcess::NotRunning && existing->program() == resolved
        && existing->arguments() == arguments && existing->workingDirectory() == workingDirectory) {
      logLine(it.key(), QFileInfo(program).fileName(), QStringLiteral("already running"));
      return it.key();
    }
  }

  auto* process = new QProcess(this);
  process->setProcessChannelMode(QProcess::MergedChannels);
  if (!workingDirectory.isEmpty())
    process->setWorkingDirectory(workingDirectory);

  auto env                = QProcessEnvironment::systemEnvironment();
  const QStringList extra = extraSearchPaths();
  if (!extra.isEmpty()) {
#ifdef Q_OS_WIN
    const QChar sep = QLatin1Char(';');
#else
    const QChar sep = QLatin1Char(':');
#endif
    const QString current = env.value(QStringLiteral("PATH"));
    env.insert(QStringLiteral("PATH"), extra.join(sep) + sep + current);
    process->setProcessEnvironment(env);
  }

  const int id          = m_nextId++;
  const QString logName = QFileInfo(program).fileName();

  connect(process, &QProcess::readyReadStandardOutput, this, [this, id, logName, process]() {
    logProcessOutput(id, logName, process->readAllStandardOutput());
  });
  connect(
    process, &QProcess::finished, this, [this, id, logName](int exitCode, QProcess::ExitStatus) {
      auto* proc = m_processes.take(id);
      if (proc)
        proc->deleteLater();

      logLine(id, logName, QStringLiteral("exited with code %1").arg(exitCode));
      Q_EMIT processFinished(id, exitCode);
    });

  process->start(resolved, arguments);
  if (!process->waitForStarted(kTerminateGraceMs)) {
    error = process->errorString();
    process->deleteLater();
    return -1;
  }

  m_processes.insert(id, process);
  logLine(id, logName, QStringLiteral("started"));
  Q_EMIT processStarted(id);
  return id;
}

/**
 * @brief Writes one tagged log line to the console/terminal via qInfo.
 */
void API::ProcessLauncher::logLine(int id, const QString& name, const QString& message)
{
  qInfo().noquote()
    << QStringLiteral("[process %1: %2] %3").arg(QString::number(id), name, message);
}

/**
 * @brief Emits the child's output one line at a time so it shows in the console/terminal.
 */
void API::ProcessLauncher::logProcessOutput(int id, const QString& name, const QByteArray& data)
{
  const auto text  = QString::fromUtf8(data);
  const auto lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
  for (const auto& line : lines) {
    const auto trimmed = line.trimmed();
    if (!trimmed.isEmpty())
      logLine(id, name, trimmed);
  }
}

/**
 * @brief Common executable directories a GUI app does not inherit on PATH (notably the
 *        minimal PATH a macOS app gets when launched from Finder), including the usual
 *        Homebrew, pyenv, and user-local Python install locations.
 */
QStringList API::ProcessLauncher::extraSearchPaths()
{
  QStringList paths;
  const QString home = QDir::homePath();

#ifdef Q_OS_MACOS
  paths << QStringLiteral("/opt/homebrew/bin") << QStringLiteral("/opt/homebrew/sbin")
        << QStringLiteral("/usr/local/bin") << QStringLiteral("/usr/local/sbin")
        << QStringLiteral("/Library/Frameworks/Python.framework/Versions/Current/bin");
  if (!home.isEmpty())
    paths << home + QStringLiteral("/.pyenv/shims") << home + QStringLiteral("/.local/bin");
#elif defined(Q_OS_LINUX)
  paths << QStringLiteral("/usr/local/bin") << QStringLiteral("/usr/local/sbin")
        << QStringLiteral("/snap/bin");
  if (!home.isEmpty())
    paths << home + QStringLiteral("/.pyenv/shims") << home + QStringLiteral("/.local/bin");
#elif defined(Q_OS_WIN)
  const QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
  if (!localAppData.isEmpty()) {
    paths << localAppData + QStringLiteral("\\Microsoft\\WindowsApps")
          << localAppData + QStringLiteral("\\Programs\\Python");
  }
#endif

  return paths;
}

/**
 * @brief Resolves a bare program name to an absolute path. The common user-install directories
 *        (Homebrew, pyenv, user-local) are searched BEFORE the bare system PATH so the user's
 *        real interpreter with their pip packages wins over the package-less /usr/bin fallback
 *        that a GUI app sees on its minimal PATH.
 */
QString API::ProcessLauncher::resolveExecutable(const QString& name)
{
  if (QFileInfo::exists(name))
    return name;

  const QString preferred = QStandardPaths::findExecutable(name, extraSearchPaths());
  if (!preferred.isEmpty())
    return preferred;

  return QStandardPaths::findExecutable(name);
}

/**
 * @brief Terminates @p process without blocking the caller: SIGTERM now, a hard kill() scheduled
 *        for the grace deadline, and deleteLater() once it exits. A hung helper ignoring SIGTERM
 *        can no longer stall the GUI thread (frame ingest, API server) during a routine
 *        disconnect or project switch.
 */
void API::ProcessLauncher::reapAsync(QProcess* process)
{
  if (!process)
    return;

  connect(process, &QProcess::finished, process, &QObject::deleteLater);
  process->terminate();

  QPointer<QProcess> guard(process);
  QTimer::singleShot(kTerminateGraceMs, process, [guard] {
    if (guard && guard->state() != QProcess::NotRunning)
      guard->kill();
  });
}

/**
 * @brief Terminates a single managed process; returns false when the id is unknown.
 */
bool API::ProcessLauncher::kill(int processId)
{
  auto* process = m_processes.take(processId);
  if (!process)
    return false;

  reapAsync(process);
  return true;
}

/**
 * @brief Terminates every managed process asynchronously (terminate now, hard-kill on timeout).
 */
void API::ProcessLauncher::killAll()
{
  const auto processes = m_processes;
  m_processes.clear();

  for (auto* process : processes)
    reapAsync(process);
}

/**
 * @brief Returns the id, program, and arguments of every running managed process.
 */
QVariantList API::ProcessLauncher::runningProcesses() const
{
  QVariantList list;
  for (auto it = m_processes.constBegin(); it != m_processes.constEnd(); ++it) {
    const auto* process = it.value();
    if (!process)
      continue;

    QVariantMap entry;
    entry.insert(QStringLiteral("processId"), it.key());
    entry.insert(QStringLiteral("program"), process->program());
    entry.insert(QStringLiteral("arguments"), process->arguments());
    entry.insert(QStringLiteral("workingDir"), process->workingDirectory());
    list.append(entry);
  }

  return list;
}

/**
 * @brief Reaps every helper during application teardown. This path blocks (bounded): the event
 *        loop is stopping, so the async timer/deleteLater machinery would never run and a
 *        SIGTERM'd child could be orphaned. The GUI is already gone, so the wait is harmless.
 */
void API::ProcessLauncher::onAboutToQuit()
{
  const auto processes = m_processes;
  m_processes.clear();

  for (auto* process : processes) {
    if (!process)
      continue;

    process->terminate();
    if (!process->waitForFinished(kTerminateGraceMs))
      process->kill();

    process->deleteLater();
  }
}

/**
 * @brief Reaps every helper when the loaded project's file path actually changes.
 */
void API::ProcessLauncher::onProjectFileChanged()
{
  SS_ASSERT(m_projectModel != nullptr, return);
  const auto path = m_projectModel->jsonFilePath();
  if (path == m_lastProjectPath)
    return;

  if (!m_processes.isEmpty())
    qInfo() << "[ProcessLauncher] project changed to" << path << ", stopping" << m_processes.size()
            << "helper process(es)";

  m_lastProjectPath = path;
  killAll();
}
