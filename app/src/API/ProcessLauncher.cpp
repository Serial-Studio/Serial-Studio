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

#include "API/ProcessLauncher.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QVariantMap>

#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"

static constexpr int kTerminateGraceMs = 2000;

/**
 * @brief Constructs the launcher with an empty process table.
 */
API::ProcessLauncher::ProcessLauncher(QObject* parent)
  : QObject(parent), m_nextId(1), m_wasConnected(false)
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
  m_lastProjectPath = DataModel::ProjectModel::instance().jsonFilePath();

  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &API::ProcessLauncher::onConnectedChanged);
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::jsonFileChanged,
          this,
          &API::ProcessLauncher::onProjectFileChanged);
  connect(qApp, &QCoreApplication::aboutToQuit, this, &API::ProcessLauncher::onAboutToQuit);
}

/**
 * @brief Spawns a helper process; returns its id, or -1 with @p error set on failure.
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
 * @brief Terminates a single managed process; returns false when the id is unknown.
 */
bool API::ProcessLauncher::kill(int processId)
{
  auto* process = m_processes.take(processId);
  if (!process)
    return false;

  process->terminate();
  if (!process->waitForFinished(kTerminateGraceMs))
    process->kill();

  process->deleteLater();
  return true;
}

/**
 * @brief Terminates every managed process (terminate, then kill on grace-period timeout).
 */
void API::ProcessLauncher::killAll()
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
 * @brief Reaps every helper only on a real connected -> disconnected transition. A bare
 *        "not connected" signal must not reap: onConnect() launches a server while still
 *        disconnected, and the in-progress connect emits not-connected signals before it
 *        succeeds; killing then would terminate the server the connection is about to reach.
 */
void API::ProcessLauncher::onConnectedChanged()
{
  const bool connected = IO::ConnectionManager::instance().isConnected();
  if (m_wasConnected && !connected)
    killAll();

  m_wasConnected = connected;
}

/**
 * @brief Reaps every helper when the loaded project's file path actually changes.
 */
void API::ProcessLauncher::onProjectFileChanged()
{
  const auto path = DataModel::ProjectModel::instance().jsonFilePath();
  if (path == m_lastProjectPath)
    return;

  m_lastProjectPath = path;
  killAll();
}

/**
 * @brief Reaps every helper during application teardown.
 */
void API::ProcessLauncher::onAboutToQuit()
{
  killAll();
}
