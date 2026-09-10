/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/Process.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimer>

#include "IO/ConnectionManager.h"

// Grace the terminated child gets before it is killed, off the GUI thread
static constexpr int kProcessTerminateGraceMs = 1000;

#ifdef Q_OS_WIN
// clang-format off
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <tlhelp32.h>
// clang-format on
#else
#  include <fcntl.h>
#  include <poll.h>
#  include <unistd.h>

#  include <cerrno>
#  include <sys/stat.h>
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Process driver and restores persisted launch/pipe settings.
 */
IO::Drivers::Process::Process()
  : m_mode(Mode::Launch)
  , m_dropReported(false)
  , m_process(nullptr)
  , m_listProbe(nullptr)
  , m_pipeRunning(false)
  , m_pipeConnected(false)
{
  const int saved = m_settings.value("ProcessDriver/mode", 0).toInt();
  m_mode          = (saved == static_cast<int>(Mode::NamedPipe)) ? Mode::NamedPipe : Mode::Launch;

  m_executable = m_settings.value("ProcessDriver/executable", QString()).toString();
  m_arguments  = m_settings.value("ProcessDriver/arguments", QString()).toString();
  m_workingDir = m_settings.value("ProcessDriver/workingDir", QString()).toString();
  m_pipePath   = m_settings.value("ProcessDriver/pipePath", QString()).toString();

  connect(&m_pipeThread, &QThread::started, this, &Process::pipeReadLoop, Qt::DirectConnection);
}

/**
 * @brief Tears down the spawned process or pipe reader thread.
 */
IO::Drivers::Process::~Process()
{
  doClose();
}

//--------------------------------------------------------------------------------------------------
// HAL_Driver interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the active process or pipe channel.
 */
void IO::Drivers::Process::close()
{
  doClose();
}

/**
 * @brief Non-virtual cleanup implementation shared by close() and ~Process().
 */
void IO::Drivers::Process::doClose()
{
  retireProcess();

  m_pipeRunning   = false;
  m_pipeConnected = false;
  if (m_pipeThread.isRunning()) {
    m_pipeThread.quit();
    m_pipeThread.wait();
  }
}

/**
 * @brief Retires the spawned process without blocking the GUI. The child is asked to terminate and
 *        killed by a timer if it will not: waiting two seconds here froze the window on every
 *        disconnect of a process that ignores SIGTERM. The QProcess is parented to this driver, so
 *        a child still running at teardown is reaped by Qt's own destructor.
 */
void IO::Drivers::Process::retireProcess()
{
  if (!m_process)
    return;

  auto* dying = m_process;
  m_process   = nullptr;
  dying->disconnect();

  if (dying->state() == QProcess::NotRunning) {
    dying->deleteLater();
    return;
  }

  connect(dying, &QProcess::finished, dying, &QProcess::deleteLater);
  dying->terminate();

  QTimer::singleShot(kProcessTerminateGraceMs, dying, [dying] {
    if (dying->state() != QProcess::NotRunning)
      dying->kill();

    dying->deleteLater();
  });
}

/**
 * @brief Queues the manager teardown at most once per session: a crashing process reports through
 *        BOTH finished() and errorOccurred(), and each queued a disconnect of its own.
 */
void IO::Drivers::Process::reportDropOnce()
{
  if (m_dropReported)
    return;

  m_dropReported = true;

  static auto& connectionManager = IO::ConnectionManager::instance();
  QMetaObject::invokeMethod(
    &connectionManager, [this] { connectionManager.disconnectDevice(this); }, Qt::QueuedConnection);
}

/**
 * @brief Returns true when the process is running or the pipe endpoint is actually live: the
 *        launch phase and the pipe's wait-for-writer window report through isConnecting()
 *        instead of faking an open channel.
 */
bool IO::Drivers::Process::isOpen() const noexcept
{
  if (m_mode == Mode::Launch)
    return m_process && m_process->state() == QProcess::Running;

  return m_pipeRunning.load() && m_pipeConnected.load();
}

/**
 * @brief Returns true while the launched process is still starting or the pipe read thread has
 *        not yet opened its endpoint.
 */
bool IO::Drivers::Process::isConnecting() const noexcept
{
  if (m_mode == Mode::Launch)
    return m_process && m_process->state() == QProcess::Starting;

  return m_pipeRunning.load() && !m_pipeConnected.load();
}

/**
 * @brief Returns true when the channel can be read.
 */
bool IO::Drivers::Process::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true only in Launch mode while the channel is open.
 */
bool IO::Drivers::Process::isWritable() const noexcept
{
  return m_mode == Mode::Launch && isOpen();
}

/**
 * @brief Returns true when enough configuration is present to call open().
 */
bool IO::Drivers::Process::configurationOk() const noexcept
{
  if (m_mode == Mode::Launch) {
    if (m_executable.isEmpty())
      return false;

    if (QFileInfo::exists(m_executable))
      return true;

    return !resolveExecutable(m_executable).isEmpty();
  }

  return !m_pipePath.isEmpty();
}

/**
 * @brief Writes @p data to the child process stdin.
 */
qint64 IO::Drivers::Process::write(const QByteArray& data)
{
  if (m_mode != Mode::Launch)
    return -1;

  if (m_process)
    return m_process->write(data);

  return -1;
}

/**
 * @brief Opens the data channel (spawns process or starts pipe reader thread).
 */
bool IO::Drivers::Process::open(const QIODevice::OpenMode mode)
{
  (void)mode;

  doClose();

  if (m_mode == Mode::Launch) {
    const QString resolved = resolveExecutable(m_executable);
    if (resolved.isEmpty()) {
      logDriverError(tr("Failed to start process"),
                     tr("Executable \"%1\" not found in PATH.").arg(m_executable));
      return false;
    }

    const QStringList args = QProcess::splitCommand(m_arguments);

    m_dropReported = false;
    m_process      = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    auto env                = QProcessEnvironment::systemEnvironment();
    const QStringList extra = extraSearchPaths();
    if (!extra.isEmpty()) {
#ifdef Q_OS_WIN
      const QChar sep = QLatin1Char(';');
#else
      const QChar sep = QLatin1Char(':');
#endif
      const QString current = env.value(QStringLiteral("PATH"));
      env.insert(QStringLiteral("PATH"), current + sep + extra.join(sep));
      m_process->setProcessEnvironment(env);
    }

    connect(m_process, &QProcess::readyReadStandardOutput, this, &Process::onReadyRead);
    connect(m_process, &QProcess::readyReadStandardError, this, &Process::onReadyReadStandardError);
    connect(m_process, &QProcess::started, this, [this] { reportOpenFinished(true); });
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &Process::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &Process::onProcessError);

    if (!m_workingDir.isEmpty())
      m_process->setWorkingDirectory(m_workingDir);

    m_process->start(resolved, args);
    return m_process->state() != QProcess::NotRunning;
  }

  m_pipeConnected = false;
  m_pipeRunning   = true;
  m_pipeThread.start();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current driver mode (0 = Launch, 1 = NamedPipe).
 */
int IO::Drivers::Process::mode() const noexcept
{
  return static_cast<int>(m_mode);
}

/**
 * @brief Returns the configured executable path.
 */
QString IO::Drivers::Process::executable() const
{
  return m_executable;
}

/**
 * @brief Returns the configured command-line arguments.
 */
QString IO::Drivers::Process::arguments() const
{
  return m_arguments;
}

/**
 * @brief Returns the working directory for Launch mode.
 */
QString IO::Drivers::Process::workingDir() const
{
  return m_workingDir;
}

/**
 * @brief Returns the configured named pipe / FIFO path.
 */
QString IO::Drivers::Process::pipePath() const
{
  return m_pipePath;
}

/**
 * @brief Returns the snapshot of running processes populated by refreshProcessList().
 */
QStringList IO::Drivers::Process::runningProcesses() const
{
  return m_runningProcesses;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the driver mode (Launch or NamedPipe).
 */
void IO::Drivers::Process::setMode(int mode)
{
  const Mode newMode = (mode == static_cast<int>(Mode::NamedPipe)) ? Mode::NamedPipe : Mode::Launch;

  if (m_mode != newMode) {
    m_mode = newMode;
    m_settings.setValue("ProcessDriver/mode", mode);
    Q_EMIT modeChanged();
    Q_EMIT configurationChanged();
  }
}

/**
 * @brief Sets the executable path or bare name for Launch mode.
 */
void IO::Drivers::Process::setExecutable(const QString& path)
{
  if (m_executable != path) {
    m_executable = path;
    m_settings.setValue("ProcessDriver/executable", path);
    Q_EMIT executableChanged();
    Q_EMIT configurationChanged();
  }
}

/**
 * @brief Sets the command-line argument string for Launch mode.
 */
void IO::Drivers::Process::setArguments(const QString& args)
{
  if (m_arguments != args) {
    m_arguments = args;
    m_settings.setValue("ProcessDriver/arguments", args);
    Q_EMIT argumentsChanged();
    Q_EMIT configurationChanged();
  }
}

/**
 * @brief Sets the working directory for Launch mode.
 */
void IO::Drivers::Process::setWorkingDir(const QString& dir)
{
  if (m_workingDir != dir) {
    m_workingDir = dir;
    m_settings.setValue("ProcessDriver/workingDir", dir);
    Q_EMIT workingDirChanged();
    Q_EMIT configurationChanged();
  }
}

/**
 * @brief Sets the named-pipe / FIFO path for NamedPipe mode.
 */
void IO::Drivers::Process::setPipePath(const QString& path)
{
  if (m_pipePath != path) {
    m_pipePath = path;
    m_settings.setValue("ProcessDriver/pipePath", path);
    Q_EMIT pipePathChanged();
    Q_EMIT configurationChanged();
  }
}

/**
 * @brief Opens a file dialog to select the executable for Launch mode.
 */
void IO::Drivers::Process::browseExecutable()
{
  const auto start = m_executable.isEmpty()
                     ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                     : QFileInfo(m_executable).absolutePath();

  auto* dialog = new QFileDialog(qApp->activeWindow(), tr("Select Executable"), start);

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { setExecutable(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a folder dialog to select the working directory for Launch mode.
 */
void IO::Drivers::Process::browseWorkingDir()
{
  const auto start = m_workingDir.isEmpty()
                     ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                     : m_workingDir;

  auto* dialog = new QFileDialog(qApp->activeWindow(), tr("Select Working Directory"), start);

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { setWorkingDir(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a file dialog to select the named pipe / FIFO path.
 */
void IO::Drivers::Process::browsePipePath()
{
  const auto start = m_pipePath.isEmpty()
                     ? QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                     : QFileInfo(m_pipePath).absolutePath();

  auto* dialog = new QFileDialog(qApp->activeWindow(), tr("Select Named Pipe / FIFO"), start);

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { setPipePath(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Enumerates running processes and updates the runningProcesses list.
 */
void IO::Drivers::Process::refreshProcessList()
{
#ifdef Q_OS_WIN
  QStringList list;
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap != INVALID_HANDLE_VALUE) {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snap, &entry)) {
      do {
        const QString name = QString::fromWCharArray(entry.szExeFile);
        const DWORD pid    = entry.th32ProcessID;
        list.append(name + " [" + QString::number(pid) + "]");
      } while (Process32NextW(snap, &entry));
    }

    CloseHandle(snap);
  }

  list.sort(Qt::CaseInsensitive);
  publishProcessList(list);
#else
  if (m_listProbe)
    return;

  m_listProbe = new QProcess(this);
  connect(m_listProbe, &QProcess::finished, this, &Process::onProcessListReady);
  connect(m_listProbe, &QProcess::errorOccurred, this, &Process::onProcessListReady);
  m_listProbe->start(QStringLiteral("ps"), QStringList{"-eo", "pid,comm"});
#endif
}

/**
 * @brief Publishes a sorted enumeration when it differs from the one the pane already shows.
 */
void IO::Drivers::Process::publishProcessList(const QStringList& list)
{
  if (m_runningProcesses == list)
    return;

  m_runningProcesses = list;
  Q_EMIT runningProcessesChanged();
}

/**
 * @brief Parses the asynchronous "ps" enumeration. The probe runs off the call: waiting three
 *        seconds for it inside the pane's refresh froze the window on a loaded machine.
 */
void IO::Drivers::Process::onProcessListReady()
{
  if (!m_listProbe)
    return;

  auto* probe = m_listProbe;
  m_listProbe = nullptr;
  probe->disconnect(this);
  probe->deleteLater();

  QStringList list;
  const QString output    = QString::fromUtf8(probe->readAllStandardOutput());
  const QStringList lines = output.split('\n', Qt::SkipEmptyParts);

  for (int i = 1; i < lines.size(); ++i) {
    const QString line = lines.at(i).trimmed();
    if (line.isEmpty())
      continue;

    const int spaceIdx = line.indexOf(' ');
    if (spaceIdx < 0)
      continue;

    const QString pid  = line.left(spaceIdx).trimmed();
    const QString name = line.mid(spaceIdx + 1).trimmed();
    if (!pid.isEmpty() && !name.isEmpty())
      list.append(name + " [" + pid + "]");
  }

  list.sort(Qt::CaseInsensitive);
  publishProcessList(list);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles data available from the QProcess.
 */
void IO::Drivers::Process::onReadyRead()
{
  if (!m_process)
    return;

  const QByteArray data = m_process->readAllStandardOutput();
  if (!data.isEmpty())
    publishReceivedData(data);
}

/**
 * @brief Publishes the process's stderr to the terminal ALONE. Merging it into stdout put log
 *        lines into the frame stream, where the parser saw them as telemetry.
 */
void IO::Drivers::Process::onReadyReadStandardError()
{
  if (!m_process)
    return;

  QByteArray data = m_process->readAllStandardError();
  if (!data.isEmpty())
    publishConsoleData(std::move(data));
}

/**
 * @brief Handles QProcess termination. The teardown is queued before the box so the UI never
 *        claims a dead process is a connected device while the modal is up.
 */
void IO::Drivers::Process::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
  if (m_process) {
    const QByteArray remaining = m_process->readAllStandardOutput();
    if (!remaining.isEmpty())
      publishReceivedData(remaining);
  }

  const QString reason = (status == QProcess::CrashExit) ? tr("The process crashed.")
                                                         : tr("Exit code: %1").arg(exitCode);

  reportOpenFinished(false, reason);
  reportDropOnce();

  logDriverError(tr("Process \"%1\" stopped").arg(QFileInfo(m_executable).fileName()), reason);
}

/**
 * @brief Handles a QProcess-level error during execution. The teardown is queued before the box
 *        so the UI never claims a failed process is a connected device while the modal is up.
 */
void IO::Drivers::Process::onProcessError(QProcess::ProcessError error)
{
  if (error == QProcess::FailedToStart) {
    reportOpenFinished(false, m_process ? m_process->errorString() : tr("Failed to start"));
    return;
  }

  const QString detail = m_process ? m_process->errorString() : tr("Unknown error");
  reportOpenFinished(false, detail);
  reportDropOnce();

  logDriverError(tr("Process Error"), detail);
}

/**
 * @brief Called on the main thread when the pipe peer closed or the read loop died mid-stream.
 *        The flag drops first so isOpen() turns false immediately: without this the UI kept
 *        showing a connected device that could never produce data again.
 */
void IO::Drivers::Process::onPipeClosed()
{
  if (!m_pipeRunning.load())
    return;

  m_pipeRunning = false;
  reportOpenFinished(false, tr("The pipe closed before the peer attached."));
  queuePipeTeardown();

  logDriverError(tr("Pipe Closed"),
                 tr("The named pipe \"%1\" was closed on the other end.").arg(m_pipePath));
}

/**
 * @brief Called on the main thread (queued from the pipe thread) when the peer attaches: the
 *        dial verdict is reported here, never from the pipe thread.
 */
void IO::Drivers::Process::onPipeAttached()
{
  reportOpenFinished(true);
}

/**
 * @brief Called on the main thread when pipeReadLoop() fails to open the pipe. The teardown is
 *        queued before the box so the UI never claims a dead pipe is a connected device.
 */
void IO::Drivers::Process::onPipeError()
{
  reportOpenFinished(false, tr("Could not open named pipe: %1").arg(m_pipePath));
  queuePipeTeardown();

  logDriverError(tr("Pipe Error"), tr("Could not open named pipe: %1").arg(m_pipePath));
}

/**
 * @brief Queues the device teardown shared by the pipe failure paths.
 */
void IO::Drivers::Process::queuePipeTeardown()
{
  static auto& connectionManager = IO::ConnectionManager::instance();
  QMetaObject::invokeMethod(
    &connectionManager, [this] { connectionManager.disconnectDevice(this); }, Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Private: pipe read loop (runs on m_pipeThread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Blocking read loop for NamedPipe mode, executed on m_pipeThread.
 */
void IO::Drivers::Process::pipeReadLoop()
{
#ifdef Q_OS_WIN
  pipeReadLoopWindows();
#else
  pipeReadLoopPosix();
#endif
}

/**
 * @brief Windows implementation of the named-pipe read loop.
 */
void IO::Drivers::Process::pipeReadLoopWindows()
{
#ifdef Q_OS_WIN
  const QString dosPath = QDir::toNativeSeparators(m_pipePath);
  HANDLE hPipe          = CreateNamedPipeW(reinterpret_cast<LPCWSTR>(dosPath.utf16()),
                                  PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                                  PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                  1,
                                  0,
                                  4096,
                                  0,
                                  nullptr);

  if (hPipe == INVALID_HANDLE_VALUE) {
    QMetaObject::invokeMethod(this, "onPipeError", Qt::QueuedConnection);
    return;
  }

  OVERLAPPED ov{};
  ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  ConnectNamedPipe(hPipe, &ov);

  bool connected = (GetLastError() == ERROR_PIPE_CONNECTED);
  while (!connected && m_pipeRunning.load()) {
    const DWORD rc = WaitForSingleObject(ov.hEvent, 100);
    if (rc == WAIT_OBJECT_0) {
      connected = true;
      break;
    }
  }

  CloseHandle(ov.hEvent);

  if (!connected) {
    CloseHandle(hPipe);
    return;
  }

  m_pipeConnected = true;
  QMetaObject::invokeMethod(this, &Process::onPipeAttached, Qt::QueuedConnection);

  char buf[4096];
  while (m_pipeRunning.load()) {
    DWORD avail = 0;
    if (!PeekNamedPipe(hPipe, nullptr, 0, nullptr, &avail, nullptr))
      break;

    if (avail == 0) {
      ::Sleep(10);
      continue;
    }

    DWORD bytesRead = 0;
    const BOOL ok   = ReadFile(hPipe, buf, sizeof(buf), &bytesRead, nullptr);
    if (!ok || bytesRead == 0)
      break;

    publishReceivedData(QByteArray(buf, static_cast<int>(bytesRead)));
  }

  CloseHandle(hPipe);
  m_pipeConnected = false;

  if (m_pipeRunning.load())
    QMetaObject::invokeMethod(this, "onPipeClosed", Qt::QueuedConnection);
#endif
}

/**
 * @brief POSIX (mkfifo + poll) implementation of the named-pipe read loop.
 */
void IO::Drivers::Process::pipeReadLoopPosix()
{
#ifndef Q_OS_WIN
  const QByteArray pathBytes = m_pipePath.toLocal8Bit();
  struct stat st{};
  const bool exists = (::stat(pathBytes.constData(), &st) == 0);
  if (exists && !S_ISFIFO(st.st_mode)) {
    QMetaObject::invokeMethod(this, "onPipeError", Qt::QueuedConnection);
    return;
  }

  if (!exists && ::mkfifo(pathBytes.constData(), 0600) != 0 && errno != EEXIST) {
    QMetaObject::invokeMethod(this, "onPipeError", Qt::QueuedConnection);
    return;
  }

  const int fd = ::open(pathBytes.constData(), O_RDONLY | O_NONBLOCK);
  if (fd < 0) {
    QMetaObject::invokeMethod(this, "onPipeError", Qt::QueuedConnection);
    return;
  }

  struct stat opened{};
  if (::fstat(fd, &opened) != 0 || !S_ISFIFO(opened.st_mode)) {
    ::close(fd);
    QMetaObject::invokeMethod(this, "onPipeError", Qt::QueuedConnection);
    return;
  }

  m_pipeConnected = true;
  QMetaObject::invokeMethod(this, &Process::onPipeAttached, Qt::QueuedConnection);

  char buf[4096];
  struct pollfd pfd{};
  pfd.fd     = fd;
  pfd.events = POLLIN;

  while (m_pipeRunning.load()) {
    const int rc = ::poll(&pfd, 1, 100);
    if (rc <= 0)
      continue;

    if (!(pfd.revents & POLLIN)) {
      if (pfd.revents & (POLLERR | POLLNVAL))
        break;

      continue;
    }

    const ssize_t n = ::read(fd, buf, sizeof(buf));
    if (n < 0)
      break;

    if (n == 0) {
      if (pfd.revents & POLLHUP)
        break;

      continue;
    }

    publishReceivedData(QByteArray(buf, static_cast<int>(n)));
  }

  ::close(fd);
  m_pipeConnected = false;

  if (m_pipeRunning.load())
    QMetaObject::invokeMethod(this, "onPipeClosed", Qt::QueuedConnection);
#endif
}

//--------------------------------------------------------------------------------------------------
// Private: executable helper
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns common executable directories not always present in PATH.
 */
QStringList IO::Drivers::Process::extraSearchPaths()
{
  QStringList paths;

#ifdef Q_OS_MACOS
  paths << QStringLiteral("/opt/homebrew/bin") << QStringLiteral("/opt/homebrew/sbin")
        << QStringLiteral("/usr/local/bin") << QStringLiteral("/usr/local/sbin");
#elif defined(Q_OS_LINUX)
  paths << QStringLiteral("/usr/local/bin") << QStringLiteral("/usr/local/sbin")
        << QStringLiteral("/snap/bin") << QStringLiteral("/var/lib/flatpak/exports/bin");

  const QString home = QDir::homePath();
  if (!home.isEmpty()) {
    paths << home + QStringLiteral("/.local/bin")
          << home + QStringLiteral("/.local/share/flatpak/exports/bin");
  }
#elif defined(Q_OS_WIN)
  const QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
  const QString userProfile  = qEnvironmentVariable("USERPROFILE");
  if (!localAppData.isEmpty())
    paths << localAppData + QStringLiteral("\\Microsoft\\WindowsApps");

  if (!userProfile.isEmpty()) {
    paths << userProfile + QStringLiteral("\\scoop\\shims")
          << userProfile + QStringLiteral("\\AppData\\Local\\Programs\\Python\\Python3\\Scripts");
  }
#endif

  return paths;
}

/**
 * @brief Resolves a bare executable name against the system PATH and common platform-specific
 * directories.
 */
QString IO::Drivers::Process::resolveExecutable(const QString& name)
{
  if (QFileInfo::exists(name))
    return name;

  QString found = QStandardPaths::findExecutable(name);
  if (!found.isEmpty())
    return found;

  return QStandardPaths::findExecutable(name, extraSearchPaths());
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Process configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::Process::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty procMode;
  procMode.key     = QStringLiteral("mode");
  procMode.label   = tr("Mode");
  procMode.type    = IO::DriverProperty::ComboBox;
  procMode.value   = static_cast<int>(m_mode);
  procMode.options = {tr("Launch Process"), tr("Named Pipe")};
  props.append(procMode);

  IO::DriverProperty exe;
  exe.key   = QStringLiteral("executable");
  exe.label = tr("Executable");
  exe.type  = IO::DriverProperty::Text;
  exe.value = m_executable;
  exe.showWhen(QStringLiteral("mode"), {static_cast<int>(Mode::Launch)});
  props.append(exe);

  IO::DriverProperty args;
  args.key   = QStringLiteral("arguments");
  args.label = tr("Arguments");
  args.type  = IO::DriverProperty::Text;
  args.value = m_arguments;
  args.showWhen(QStringLiteral("mode"), {static_cast<int>(Mode::Launch)});
  props.append(args);

  IO::DriverProperty dir;
  dir.key   = QStringLiteral("workingDir");
  dir.label = tr("Working Directory");
  dir.type  = IO::DriverProperty::Text;
  dir.value = m_workingDir;
  dir.showWhen(QStringLiteral("mode"), {static_cast<int>(Mode::Launch)});
  props.append(dir);

  IO::DriverProperty pipe;
  pipe.key   = QStringLiteral("pipePath");
  pipe.label = tr("Pipe Path");
  pipe.type  = IO::DriverProperty::Text;
  pipe.value = m_pipePath;
  pipe.showWhen(QStringLiteral("mode"), {static_cast<int>(Mode::NamedPipe)});
  props.append(pipe);

  return props;
}

/**
 * @brief Applies a single Process configuration change by key.
 */
void IO::Drivers::Process::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("mode")) {
    setMode(value.toInt());
    return;
  }

  if (key == QLatin1String("executable")) {
    setExecutable(value.toString());
    return;
  }

  if (key == QLatin1String("arguments")) {
    setArguments(value.toString());
    return;
  }

  if (key == QLatin1String("workingDir")) {
    setWorkingDir(value.toString());
    return;
  }

  if (key == QLatin1String("pipePath"))
    setPipePath(value.toString());
}
