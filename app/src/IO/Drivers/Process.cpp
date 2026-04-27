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

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

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

#  include <sys/stat.h>
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Process driver and restores persisted launch/pipe settings.
 */
IO::Drivers::Process::Process() : m_mode(Mode::Launch), m_process(nullptr), m_pipeRunning(false)
{
  // Restore persisted settings
  const int saved = m_settings.value("ProcessDriver/mode", 0).toInt();
  m_mode          = (saved == static_cast<int>(Mode::NamedPipe)) ? Mode::NamedPipe : Mode::Launch;

  m_executable = m_settings.value("ProcessDriver/executable", QString()).toString();
  m_arguments  = m_settings.value("ProcessDriver/arguments", QString()).toString();
  m_workingDir = m_settings.value("ProcessDriver/workingDir", QString()).toString();
  m_pipePath   = m_settings.value("ProcessDriver/pipePath", QString()).toString();

  // Connect pipe thread to read loop
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
  // Clean up Launch mode resources
  if (m_process) {
    m_process->disconnect();
    m_process->terminate();
    if (!m_process->waitForFinished(2000))
      m_process->kill();

    m_process->deleteLater();
    m_process = nullptr;
  }

  // Clean up NamedPipe mode resources
  m_pipeRunning = false;
  if (m_pipeThread.isRunning()) {
    m_pipeThread.quit();
    m_pipeThread.wait();
  }
}

/**
 * @brief Returns true when the process or pipe channel is open.
 */
bool IO::Drivers::Process::isOpen() const noexcept
{
  if (m_mode == Mode::Launch)
    return m_process && m_process->state() == QProcess::Running;

  return m_pipeRunning.load();
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

  // Close any previous connection
  doClose();

  // Launch mode: spawn process
  if (m_mode == Mode::Launch) {
    // Resolve executable path
    const QString resolved = resolveExecutable(m_executable);
    if (resolved.isEmpty()) {
      Misc::Utilities::showMessageBox(tr("Failed to start process"),
                                      tr("Executable \"%1\" not found in PATH.").arg(m_executable),
                                      QMessageBox::Warning);
      return false;
    }

    // Set up arguments and augment PATH with platform-specific directories
    const QStringList args = QProcess::splitCommand(m_arguments);

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

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

    // Connect process signals
    connect(m_process, &QProcess::readyRead, this, &Process::onReadyRead);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &Process::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &Process::onProcessError);

    // Set working directory and start the process
    if (!m_workingDir.isEmpty())
      m_process->setWorkingDirectory(m_workingDir);

    m_process->start(resolved, args);

    if (!m_process->waitForStarted(3000)) {
      const QString detail = m_process->errorString();
      m_process->deleteLater();
      m_process = nullptr;

      Misc::Utilities::showMessageBox(tr("Failed to start process"), detail, QMessageBox::Warning);
      return false;
    }

    return true;
  }

  // NamedPipe mode: start read thread
  m_pipeRunning = true;
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

  auto* dialog = new QFileDialog(nullptr, tr("Select Executable"), start);

  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      setExecutable(path);

    dialog->deleteLater();
  });

  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);

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

  auto* dialog = new QFileDialog(nullptr, tr("Select Working Directory"), start);

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      setWorkingDir(path);

    dialog->deleteLater();
  });

  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);

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

  auto* dialog = new QFileDialog(nullptr, tr("Select Named Pipe / FIFO"), start);

  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      setPipePath(path);

    dialog->deleteLater();
  });

  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);

  dialog->open();
}

/**
 * @brief Enumerates running processes and updates the runningProcesses list.
 */
void IO::Drivers::Process::refreshProcessList()
{
  QStringList list;

#ifdef Q_OS_WIN
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
#else
  QProcess ps;
  ps.start("ps", QStringList{"-eo", "pid,comm"});
  ps.waitForFinished(3000);

  const QString output    = QString::fromUtf8(ps.readAllStandardOutput());
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
#endif

  list.sort(Qt::CaseInsensitive);

  if (m_runningProcesses != list) {
    m_runningProcesses = list;
    Q_EMIT runningProcessesChanged();
  }
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
 * @brief Handles QProcess termination.
 */
void IO::Drivers::Process::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
  // Drain any buffered stdout before tearing down
  if (m_process) {
    const QByteArray remaining = m_process->readAllStandardOutput();
    if (!remaining.isEmpty())
      publishReceivedData(remaining);
  }

  const QString reason = (status == QProcess::CrashExit) ? tr("The process crashed.")
                                                         : tr("Exit code: %1").arg(exitCode);

  Misc::Utilities::showMessageBox(
    tr("Process \"%1\" stopped").arg(QFileInfo(m_executable).fileName()),
    reason,
    QMessageBox::Warning);

  QMetaObject::invokeMethod(&IO::ConnectionManager::instance(), "disconnectDevice");
}

/**
 * @brief Handles a QProcess-level error during execution.
 */
void IO::Drivers::Process::onProcessError(QProcess::ProcessError error)
{
  // FailedToStart is already reported by open() via waitForStarted()
  if (error == QProcess::FailedToStart)
    return;

  const QString detail = m_process ? m_process->errorString() : tr("Unknown error");
  Misc::Utilities::showMessageBox(tr("Process Error"), detail, QMessageBox::Warning);

  QMetaObject::invokeMethod(&IO::ConnectionManager::instance(), "disconnectDevice");
}

/**
 * @brief Called on the main thread when pipeReadLoop() fails to open the pipe.
 */
void IO::Drivers::Process::onPipeError()
{
  Misc::Utilities::showMessageBox(
    tr("Pipe Error"), tr("Could not open named pipe: %1").arg(m_pipePath), QMessageBox::Warning);

  QMetaObject::invokeMethod(&IO::ConnectionManager::instance(), "disconnectDevice");
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
#else
  const QByteArray pathBytes = m_pipePath.toLocal8Bit();
  struct stat st{};
  const bool exists = (::stat(pathBytes.constData(), &st) == 0);
  if (!exists)
    ::mkfifo(pathBytes.constData(), 0600);
  else if (!S_ISFIFO(st.st_mode)) {
    QMetaObject::invokeMethod(this, "onPipeError", Qt::QueuedConnection);
    return;
  }

  const int fd = ::open(pathBytes.constData(), O_RDONLY | O_NONBLOCK);
  if (fd < 0) {
    QMetaObject::invokeMethod(this, "onPipeError", Qt::QueuedConnection);
    return;
  }

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

  // Search the system PATH
  const QString found = QStandardPaths::findExecutable(name);
  if (!found.isEmpty())
    return found;

  // Fall back to platform-specific directories
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
  props.append(exe);

  IO::DriverProperty args;
  args.key   = QStringLiteral("arguments");
  args.label = tr("Arguments");
  args.type  = IO::DriverProperty::Text;
  args.value = m_arguments;
  props.append(args);

  IO::DriverProperty dir;
  dir.key   = QStringLiteral("workingDir");
  dir.label = tr("Working Directory");
  dir.type  = IO::DriverProperty::Text;
  dir.value = m_workingDir;
  props.append(dir);

  IO::DriverProperty pipe;
  pipe.key   = QStringLiteral("pipePath");
  pipe.label = tr("Pipe Path");
  pipe.type  = IO::DriverProperty::Text;
  pipe.value = m_pipePath;
  props.append(pipe);

  return props;
}

/**
 * @brief Applies a single Process configuration change by key.
 */
void IO::Drivers::Process::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("mode"))
    setMode(value.toInt());

  else if (key == QLatin1String("executable"))
    setExecutable(value.toString());

  else if (key == QLatin1String("arguments"))
    setArguments(value.toString());

  else if (key == QLatin1String("workingDir"))
    setWorkingDir(value.toString());

  else if (key == QLatin1String("pipePath"))
    setPipePath(value.toString());
}
