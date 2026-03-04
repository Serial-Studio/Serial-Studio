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

#ifdef BUILD_COMMERCIAL

#  include "IO/Drivers/Process.h"

#  include <QFile>
#  include <QFileInfo>
#  include <QMetaObject>

#  include "IO/Manager.h"
#  include "Misc/Utilities.h"

#  ifdef Q_OS_WIN
#    define WIN32_LEAN_AND_MEAN
#    ifndef NOMINMAX
#      define NOMINMAX
#    endif
// clang-format off
#    include <windows.h>
#    include <tlhelp32.h>
// clang-format on
#    include <QDir>
#  else
#    include <fcntl.h>
#    include <poll.h>
#    include <sys/stat.h>
#    include <unistd.h>
#  endif

//--------------------------------------------------------------------------------------------------
// Constructor / destructor / singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Process driver singleton.
 *
 * Loads all persisted settings from QSettings and initialises the process
 * pointer to nullptr.
 */
IO::Drivers::Process::Process()
  : m_mode(Mode::Launch), m_outputCapture(OutputCapture::StdOut), m_process(nullptr)
{
  const int savedMode = m_settings.value("ProcessDriver/mode", 0).toInt();
  m_mode = (savedMode == static_cast<int>(Mode::NamedPipe)) ? Mode::NamedPipe : Mode::Launch;

  const int savedCapture = m_settings.value("ProcessDriver/outputCapture", 0).toInt();
  if (savedCapture == static_cast<int>(OutputCapture::StdErr))
    m_outputCapture = OutputCapture::StdErr;
  else if (savedCapture == static_cast<int>(OutputCapture::Both))
    m_outputCapture = OutputCapture::Both;
  else
    m_outputCapture = OutputCapture::StdOut;

  m_executable = m_settings.value("ProcessDriver/executable", QString()).toString();
  m_arguments  = m_settings.value("ProcessDriver/arguments", QString()).toString();
  m_workingDir = m_settings.value("ProcessDriver/workingDir", QString()).toString();
  m_pipePath   = m_settings.value("ProcessDriver/pipePath", QString()).toString();

  connect(&m_pipeThread, &QThread::started, this, &Process::pipeReadLoop, Qt::DirectConnection);
}

/**
 * @brief Returns the singleton Process driver instance.
 */
IO::Drivers::Process& IO::Drivers::Process::instance()
{
  static Process instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// HAL_Driver interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the active connection.
 *
 * In Launch mode: terminates the child process gracefully (SIGTERM), waits up
 * to 2 s, then kills it forcefully if still running.
 * In Pipe mode: signals the read loop to exit and waits for the pipe thread.
 */
void IO::Drivers::Process::close()
{
  if (m_mode == Mode::Launch) {
    if (m_process) {
      m_process->disconnect();
      m_process->terminate();
      if (!m_process->waitForFinished(2000))
        m_process->kill();
      m_process->deleteLater();
      m_process = nullptr;
    }
  } else {
    m_pipeRunning = false;
    if (m_pipeThread.isRunning()) {
      m_pipeThread.quit();
      m_pipeThread.wait();
    }
  }
}

/**
 * @brief Returns true when the driver has an active data channel.
 */
bool IO::Drivers::Process::isOpen() const noexcept
{
  if (m_mode == Mode::Launch)
    return m_process && m_process->state() == QProcess::Running;

  return m_pipeRunning.load();
}

/**
 * @brief Returns true — both modes support reading.
 */
bool IO::Drivers::Process::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true only in Launch mode (stdin is writable).
 *
 * Named pipes opened read-only do not support writes.
 */
bool IO::Drivers::Process::isWritable() const noexcept
{
  return m_mode == Mode::Launch && isOpen();
}

/**
 * @brief Returns true when the user has supplied enough configuration to open.
 *
 * Launch mode: executable path must be non-empty and point to an existing file.
 * Pipe mode: pipe path must be non-empty.
 */
bool IO::Drivers::Process::configurationOk() const noexcept
{
  if (m_mode == Mode::Launch)
    return !m_executable.isEmpty() && QFileInfo::exists(m_executable);

  return !m_pipePath.isEmpty();
}

/**
 * @brief Writes @p data to the child process stdin (Launch mode only).
 *
 * @return Number of bytes written, or -1 if the driver is not open or is in
 *         Pipe mode.
 */
qint64 IO::Drivers::Process::write(const QByteArray& data)
{
  if (m_mode == Mode::Launch && m_process)
    return m_process->write(data);

  return -1;
}

/**
 * @brief Opens the data channel.
 *
 * Launch mode: spawns the configured executable with the parsed argument list.
 * Returns false if the process does not start within 3 s.
 *
 * Pipe mode: starts the dedicated pipe-read thread and returns true
 * immediately. A failed pipe open inside pipeReadLoop() will call close()
 * on the main thread via a queued invocation.
 *
 * @param mode  Ignored — open mode is determined by the driver's Mode setting.
 */
bool IO::Drivers::Process::open(const QIODevice::OpenMode mode)
{
  (void)mode;

  if (m_mode == Mode::Launch) {
    m_process = new QProcess(this);

    if (m_outputCapture == OutputCapture::Both)
      m_process->setProcessChannelMode(QProcess::MergedChannels);
    else
      m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::readyRead, this, &Process::onReadyRead, Qt::DirectConnection);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &Process::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &Process::onProcessError);

    if (!m_workingDir.isEmpty())
      m_process->setWorkingDirectory(m_workingDir);

    const QStringList args = QProcess::splitCommand(m_arguments);
    m_process->start(m_executable, args);

    if (!m_process->waitForStarted(3000)) {
      const QString detail = m_process->errorString();
      m_process->deleteLater();
      m_process = nullptr;

      Misc::Utilities::showMessageBox(tr("Failed to start process"), detail, QMessageBox::Warning);
      return false;
    }

    return true;
  }

  m_pipeRunning = true;
  m_pipeThread.start();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current driver mode as an integer.
 *
 * The integer maps to the Mode enum: 0 = Launch, 1 = NamedPipe. Exposed as
 * int so QML ComboBox indices bind without conversion.
 *
 * @return Current Mode cast to int.
 */
int IO::Drivers::Process::mode() const
{
  return static_cast<int>(m_mode);
}

/**
 * @brief Returns the configured executable path for Launch mode.
 *
 * @return Absolute or relative path to the executable, or an empty string if
 *         not yet configured.
 */
QString IO::Drivers::Process::executable() const
{
  return m_executable;
}

/**
 * @brief Returns the command-line argument string for Launch mode.
 *
 * The string is split by QProcess::splitCommand() when the process is started,
 * so standard shell quoting rules apply.
 *
 * @return Argument string, or an empty string if none were configured.
 */
QString IO::Drivers::Process::arguments() const
{
  return m_arguments;
}

/**
 * @brief Returns the working directory for Launch mode.
 *
 * If empty, the child process inherits the application's current directory.
 *
 * @return Working directory path, or an empty string if not configured.
 */
QString IO::Drivers::Process::workingDir() const
{
  return m_workingDir;
}

/**
 * @brief Returns the named-pipe / FIFO path for Pipe mode.
 *
 * @return File-system path of the pipe, or an empty string if not configured.
 */
QString IO::Drivers::Process::pipePath() const
{
  return m_pipePath;
}

/**
 * @brief Returns the snapshot of running processes populated by refreshProcessList().
 *
 * Each entry is formatted as "name [PID]". The list is empty until
 * refreshProcessList() has been called at least once.
 *
 * @return List of "name [PID]" strings for all running processes.
 */
QStringList IO::Drivers::Process::runningProcesses() const
{
  return m_runningProcesses;
}

/**
 * @brief Returns the current output capture mode as an integer.
 *
 * Maps to the OutputCapture enum: 0 = StdOut, 1 = StdErr, 2 = Both. Exposed
 * as int for QML ComboBox binding.
 *
 * @return Current OutputCapture cast to int.
 */
int IO::Drivers::Process::outputCapture() const
{
  return static_cast<int>(m_outputCapture);
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the driver mode (Launch or NamedPipe).
 *
 * Persists the choice and emits modeChanged() and configurationChanged().
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
 * @brief Sets the executable path for Launch mode.
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
 * @brief Sets the command-line arguments for Launch mode.
 */
void IO::Drivers::Process::setArguments(const QString& args)
{
  if (m_arguments != args) {
    m_arguments = args;
    m_settings.setValue("ProcessDriver/arguments", args);
    Q_EMIT argumentsChanged();
  }
}

/**
 * @brief Sets the working directory for Launch mode (optional).
 */
void IO::Drivers::Process::setWorkingDir(const QString& dir)
{
  if (m_workingDir != dir) {
    m_workingDir = dir;
    m_settings.setValue("ProcessDriver/workingDir", dir);
    Q_EMIT workingDirChanged();
  }
}

/**
 * @brief Sets the output capture mode (stdout / stderr / both).
 *
 * In Both mode QProcess merges the channels at the OS level so stdout and
 * stderr arrive interleaved through the single readAll() call.
 */
void IO::Drivers::Process::setOutputCapture(int capture)
{
  OutputCapture newCapture = OutputCapture::StdOut;
  if (capture == static_cast<int>(OutputCapture::StdErr))
    newCapture = OutputCapture::StdErr;
  else if (capture == static_cast<int>(OutputCapture::Both))
    newCapture = OutputCapture::Both;

  if (m_outputCapture != newCapture) {
    m_outputCapture = newCapture;
    m_settings.setValue("ProcessDriver/outputCapture", capture);
    Q_EMIT outputCaptureChanged();
  }
}

/**
 * @brief Sets the named-pipe / FIFO path for Pipe mode.
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
 * @brief Enumerates running processes and populates the runningProcesses list.
 *
 * On Unix: runs `ps -eo pid,comm` and parses the output into
 * "name [PID]" strings.
 * On Windows: uses the Toolhelp32 snapshot API.
 *
 * Call this before opening the ProcessPicker dialog.
 */
void IO::Drivers::Process::refreshProcessList()
{
  QStringList list;

#  ifdef Q_OS_WIN
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
#  else
  QProcess ps;
  ps.start("ps", QStringList{"-eo", "pid,comm"});
  ps.waitForFinished(3000);

  const QString output    = QString::fromUtf8(ps.readAllStandardOutput());
  const QStringList lines = output.split('\n', Qt::SkipEmptyParts);

  // Skip header line
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
#  endif

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
 * @brief Handles data available from the child process.
 *
 * Reads the configured output channel(s) and emits dataReceived().
 *
 * - StdOut: reads only stdout (default).
 * - StdErr: reads only stderr.
 * - Both:   channels are merged at the QProcess level, so readAll() returns
 *           the interleaved stream.
 */
void IO::Drivers::Process::onReadyRead()
{
  if (!m_process)
    return;

  QByteArray data;
  if (m_outputCapture == OutputCapture::StdErr)
    data = m_process->readAllStandardError();
  else
    data = m_process->readAllStandardOutput();

  if (!data.isEmpty())
    Q_EMIT dataReceived(makeByteArray(data));
}

/**
 * @brief Handles child-process termination.
 *
 * Shows an informational message box and requests disconnection via a queued
 * call to IO::Manager so the UI is updated on the main thread.
 */
void IO::Drivers::Process::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
  const QString reason = (status == QProcess::CrashExit) ? tr("The process crashed.")
                                                         : tr("Exit code: %1").arg(exitCode);

  Misc::Utilities::showMessageBox(
    tr("Process \"%1\" stopped").arg(QFileInfo(m_executable).fileName()),
    reason,
    QMessageBox::Warning);

  QMetaObject::invokeMethod(&IO::Manager::instance(), "disconnectDevice", Qt::QueuedConnection);
}

/**
 * @brief Handles a QProcess-level error before or during execution.
 *
 * Shows a warning message box and requests disconnection.
 */
void IO::Drivers::Process::onProcessError(QProcess::ProcessError error)
{
  if (error == QProcess::FailedToStart)
    return;

  const QString detail = m_process ? m_process->errorString() : tr("Unknown error");
  Misc::Utilities::showMessageBox(tr("Process Error"), detail, QMessageBox::Warning);

  QMetaObject::invokeMethod(&IO::Manager::instance(), "disconnectDevice", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Private: pipe read loop (runs on m_pipeThread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Called on the main thread when the pipe fails to open.
 *
 * Shows a warning and triggers a full disconnect so the UI reflects the
 * closed state. This slot exists because pipeReadLoop() runs on m_pipeThread
 * and cannot call close() directly — it marshals the call here via a queued
 * QMetaObject::invokeMethod().
 */
void IO::Drivers::Process::onPipeError()
{
  Misc::Utilities::showMessageBox(
    tr("Pipe Error"), tr("Could not open named pipe: %1").arg(m_pipePath), QMessageBox::Warning);

  QMetaObject::invokeMethod(&IO::Manager::instance(), "disconnectDevice", Qt::QueuedConnection);
}

/**
 * @brief Blocking read loop for Pipe mode, executed on m_pipeThread.
 *
 * Opens the named pipe / FIFO for reading and loops, forwarding data via
 * dataReceived(). If the pipe cannot be opened the loop exits and
 * onPipeError() is scheduled on the main thread via a queued invocation.
 *
 * QFile::waitForReadyRead() is not implemented for FIFOs/pipes and always
 * returns false. We use native blocking I/O instead: poll() on Unix and
 * ReadFile() on Windows, both with a 100 ms timeout so m_pipeRunning can be
 * checked between iterations.
 */
void IO::Drivers::Process::pipeReadLoop()
{
#  ifdef Q_OS_WIN
  const QString dosPath = QDir::toNativeSeparators(m_pipePath);
  HANDLE hPipe = CreateNamedPipeW(reinterpret_cast<LPCWSTR>(dosPath.utf16()),
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

  // Wait for a writer to connect, checking m_pipeRunning every 100 ms
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
    const BOOL ok = ReadFile(hPipe, buf, sizeof(buf), &bytesRead, nullptr);
    if (!ok || bytesRead == 0)
      break;

    Q_EMIT dataReceived(makeByteArray(QByteArray(buf, static_cast<int>(bytesRead))));
  }

  CloseHandle(hPipe);
#  else
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

    Q_EMIT dataReceived(makeByteArray(QByteArray(buf, static_cast<int>(n))));
  }

  ::close(fd);
#  endif
}

#endif  // BUILD_COMMERCIAL
