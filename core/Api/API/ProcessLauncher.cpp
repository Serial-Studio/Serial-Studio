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
#include <QThread>
#include <QTimer>
#include <QVariantMap>

#include "API/HandlerContext.h"
#include "API/Server.h"
#include "Core/Runtime.h"
#include "Core/SSAssert.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "IO/ConnectionManager.h"

static constexpr int kTerminateGraceMs = 2000;

/**
 * @brief Constructs the launcher with an empty process table and an unbound consent (no project
 *        yet); LaunchConsent applies the headless override.
 */
API::ProcessLauncher::ProcessLauncher(QObject* parent)
  : QObject(parent)
  , m_nextId(1)
  , m_consent(m_settings, this)
  , m_reconnectOnGrant(false)
  , m_connectionManager(nullptr)
  , m_projectModel(nullptr)
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
 *        change, and application quit, and becomes the launch authorizer every program start
 *        asks (spec 0098). A root that never wires the launcher refuses every launch.
 */
void API::ProcessLauncher::setupExternalConnections()
{
  m_connectionManager = &API::handlerContext().connectionManager;
  m_projectModel      = &DataModel::pipelineModules().projectModel;

  m_lastProjectPath = m_projectModel->jsonFilePath();
  m_consent.setOperatorConsentedPath(Core::Runtime::operatorProject());
  m_consent.bindProject(m_lastProjectPath);
  IO::setLaunchAuthorizer(this);

  connect(&m_consent,
          &LaunchConsent::granted,
          &DataModel::pipelineModules().controlScript,
          &DataModel::ControlScript::restart);
  connect(&m_consent, &LaunchConsent::granted, this, &API::ProcessLauncher::onConsentGranted);
  connect(&API::handlerContext().server,
          &API::Server::remoteProgramChanged,
          this,
          &API::ProcessLauncher::onRemoteProgramChanged);
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
 * @brief Spawns a helper; returns its id, or with @p error set: -1 on failure,
 *        kLaunchConsentRequired while the per-project prompt is pending, kLaunchDenied after a
 *        "no". An identical helper still running is not spawned again: reconnects re-run
 *        onConnect(), and a duplicate server only steals the first one's port and dies on bind.
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

  const auto request = tr("This project's script wants to run:\n%1")
                         .arg((QStringList(program) + arguments).join(QLatin1Char(' ')));
  const auto verdict = m_consent.authorize(request);
  if (verdict == ConsentVerdict::ConsentRequired) {
    error = tr("Launching programs needs the user's consent; a prompt was shown");
    return kLaunchConsentRequired;
  }

  if (verdict == ConsentVerdict::Denied) {
    error = tr("Launching programs was denied by the user");
    return kLaunchDenied;
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
 * @brief The launch gate a Process I/O source asks before it starts a program (spec 0098 R8):
 *        the same per-project consent as system.exec. A start refused while the prompt is
 *        pending reconnects once the user says yes.
 */
IO::LaunchVerdict API::ProcessLauncher::authorizeLaunch(const QString& program,
                                                        const QStringList& arguments,
                                                        const QString& workingDir)
{
  SS_ASSERT(QThread::currentThread() == thread(), return IO::LaunchVerdict::Denied);

  auto request = tr("This project's Process I/O source wants to run:\n%1")
                   .arg((QStringList(program) + arguments).join(QLatin1Char(' ')));
  if (!workingDir.isEmpty())
    request += QStringLiteral("\n") + tr("in %1").arg(workingDir);

  const auto verdict = m_consent.authorize(request);
  if (verdict == ConsentVerdict::Allowed)
    return IO::LaunchVerdict::Allowed;

  if (verdict == ConsentVerdict::Denied) {
    m_reconnectOnGrant = false;
    return IO::LaunchVerdict::Denied;
  }

  m_reconnectOnGrant = true;
  return IO::LaunchVerdict::ConsentRequired;
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
 * @brief A remote client changed what the project runs (spec 0098 R13): its launch consent is
 *        asked again, once this session.
 */
void API::ProcessLauncher::onRemoteProgramChanged()
{
  m_consent.noteRemoteChange();
}

/**
 * @brief Retries the connect a Process source's consent prompt refused, once, queued so it never
 *        runs under the prompt's own stack.
 */
void API::ProcessLauncher::onConsentGranted()
{
  if (!m_reconnectOnGrant || m_connectionManager == nullptr)
    return;

  m_reconnectOnGrant = false;
  QMetaObject::invokeMethod(
    this,
    [this] {
      if (!m_connectionManager->isConnected())
        m_connectionManager->connectDevice();
    },
    Qt::QueuedConnection);
}

/**
 * @brief Reaps every helper when the loaded project's file path actually changes, and rebinds
 *        the launch consent to the new file: a yes given to one project never carries to another.
 *        A pending reconnect belonged to the old project and is dropped. The operator project is
 *        applied first, so a command-line project the session had already restored still counts.
 */
void API::ProcessLauncher::onProjectFileChanged()
{
  SS_ASSERT(m_projectModel != nullptr, return);
  m_consent.setOperatorConsentedPath(Core::Runtime::operatorProject());
  const auto path = m_projectModel->jsonFilePath();
  if (path == m_lastProjectPath)
    return;

  if (!m_processes.isEmpty())
    qInfo() << "[ProcessLauncher] project changed to" << path << ", stopping" << m_processes.size()
            << "helper process(es)";

  m_lastProjectPath  = path;
  m_reconnectOnGrant = false;
  killAll();

  m_consent.bindProject(path);
}
