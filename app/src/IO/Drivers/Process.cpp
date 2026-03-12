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

#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QMetaObject>
#  include <QProcessEnvironment>
#  include <QStandardPaths>

#  include "IO/ConnectionManager.h"
#  include "Misc/Utilities.h"

#  ifdef Q_OS_WIN
// clang-format off
#    include <windows.h>
#    include <tlhelp32.h>
// clang-format on
#  else
#    include <fcntl.h>
#    include <poll.h>
#    include <signal.h>
#    include <termios.h>
#    include <unistd.h>

#    include <sys/ioctl.h>
#    include <sys/stat.h>
#    include <sys/wait.h>
#    ifdef Q_OS_MACOS
#      include <util.h>
#    else
#      include <pty.h>
#    endif
#  endif

//--------------------------------------------------------------------------------------------------
// Constructor / destructor / singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Process driver singleton.
 *
 * Restores all persisted settings from QSettings and connects the thread
 * started() signals to their respective read-loop slots.
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
  connect(&m_ptyThread, &QThread::started, this, &Process::ptyReadLoop, Qt::DirectConnection);
}

/**
 * @brief Destructor — ensures both worker threads are stopped before the
 *        QThread members are destroyed.
 *
 * Qt calls qFatal() if a QThread is destroyed while still running, which
 * crashes the process at exit when the singleton is torn down.  Calling
 * doClose() (non-virtual) here guarantees the threads are joined first.
 */
IO::Drivers::Process::~Process()
{
  doClose();
}

//--------------------------------------------------------------------------------------------------
// HAL_Driver interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the active connection and releases all associated resources.
 *
 * Delegates to doClose() so the destructor can share the same cleanup path
 * without invoking a virtual function during teardown.
 */
void IO::Drivers::Process::close()
{
  doClose();
}

/**
 * @brief Non-virtual cleanup implementation shared by close() and ~Process().
 *
 * Launch mode (PTY): sets m_ptyRunning to false, then closes the master fd /
 * ConPTY output pipe to unblock any blocking read in ptyReadLoop().  On
 * Windows a local copy of the pipe handle is taken before zeroing the member
 * so that any in-flight GetOverlappedResult() in ptyReadLoop() can finish
 * cleanly against the still-valid local copy.  Waits for m_ptyThread to exit,
 * then terminates the child process and releases all remaining handles.
 *
 * Launch mode (pipe fallback): terminates the QProcess gracefully (SIGTERM),
 * waits up to 2 s, then kills it forcefully if still running.
 *
 * NamedPipe mode: signals pipeReadLoop() to exit and joins m_pipeThread.
 */
void IO::Drivers::Process::doClose()
{
  if (m_mode == Mode::Launch) {
    m_ptyRunning = false;

#  ifdef Q_OS_WIN
    // Take a local copy and zero the member *before* closing so that any
    // in-flight GetOverlappedResult() in ptyReadLoop() uses the valid handle.
    HANDLE outPipe = m_conPtyOut;
    m_conPtyOut    = INVALID_HANDLE_VALUE;
    if (outPipe != INVALID_HANDLE_VALUE)
      CloseHandle(outPipe);
#  else
    // Close master fd first — unblocks poll() in ptyReadLoop immediately
    if (m_ptyMasterFd >= 0) {
      ::close(m_ptyMasterFd);
      m_ptyMasterFd = -1;
    }
#  endif

    if (m_ptyThread.isRunning()) {
      m_ptyThread.quit();
      m_ptyThread.wait();
    }

#  ifdef Q_OS_WIN
    if (m_hProcess != INVALID_HANDLE_VALUE) {
      TerminateProcess(m_hProcess, 0);
      WaitForSingleObject(m_hProcess, 2000);
      CloseHandle(m_hProcess);
      m_hProcess = INVALID_HANDLE_VALUE;
    }

    if (m_hThread != INVALID_HANDLE_VALUE) {
      CloseHandle(m_hThread);
      m_hThread = INVALID_HANDLE_VALUE;
    }

    if (m_hPseudoConsole) {
      using FnClosePseudoConsole = void(WINAPI*)(HPCON);
      auto fn                    = reinterpret_cast<FnClosePseudoConsole>(
        GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "ClosePseudoConsole"));
      if (fn)
        fn(static_cast<HPCON>(m_hPseudoConsole));
      m_hPseudoConsole = nullptr;
    }

    if (m_conPtyIn != INVALID_HANDLE_VALUE) {
      CloseHandle(m_conPtyIn);
      m_conPtyIn = INVALID_HANDLE_VALUE;
    }
#  else
    if (m_childPid > 0) {
      ::kill(m_childPid, SIGTERM);
      int status = 0;
      if (::waitpid(m_childPid, &status, WNOHANG) == 0) {
        ::usleep(200000);
        ::waitpid(m_childPid, &status, WNOHANG);
      }
      m_childPid = -1;
    }
#  endif

    // Clean up QProcess fallback if PTY was not used
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
 *
 * In Launch mode, true if either the PTY read loop is running or the QProcess
 * fallback is in the Running state.  In NamedPipe mode, true while
 * pipeReadLoop() is executing.
 */
bool IO::Drivers::Process::isOpen() const noexcept
{
  if (m_mode == Mode::Launch) {
    if (m_ptyRunning.load())
      return true;

    return m_process && m_process->state() == QProcess::Running;
  }

  return m_pipeRunning.load();
}

/**
 * @brief Returns true — both Launch and NamedPipe modes support reading.
 */
bool IO::Drivers::Process::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true only in Launch mode while the channel is open.
 *
 * NamedPipe mode opens the pipe read-only and does not support writes.
 */
bool IO::Drivers::Process::isWritable() const noexcept
{
  return m_mode == Mode::Launch && isOpen();
}

/**
 * @brief Returns true when enough configuration is present to call open().
 *
 * Launch mode: the executable field must be non-empty and either point to an
 * existing file or be resolvable to a full path via the shell PATH.
 * NamedPipe mode: the pipe path must be non-empty.
 */
bool IO::Drivers::Process::configurationOk() const noexcept
{
  if (m_mode == Mode::Launch) {
    if (m_executable.isEmpty())
      return false;

    if (QFileInfo::exists(m_executable))
      return true;

    return !resolveExecutable(m_executable, shellEnvironment()).isEmpty();
  }

  return !m_pipePath.isEmpty();
}

/**
 * @brief Writes @p data to the child process stdin.
 *
 * In PTY mode the bytes are written directly to the master fd (Unix) or the
 * ConPTY input pipe (Windows).  Falls back to QProcess::write() when the PTY
 * is not active.
 *
 * @return Number of bytes written, or -1 if the driver is not in Launch mode
 *         or no channel is open.
 */
qint64 IO::Drivers::Process::write(const QByteArray& data)
{
  if (m_mode != Mode::Launch)
    return -1;

#  ifdef Q_OS_WIN
  if (m_conPtyIn != INVALID_HANDLE_VALUE) {
    DWORD written = 0;
    WriteFile(m_conPtyIn, data.constData(), static_cast<DWORD>(data.size()), &written, nullptr);
    return static_cast<qint64>(written);
  }
#  else
  if (m_ptyMasterFd >= 0)
    return ::write(m_ptyMasterFd, data.constData(), static_cast<size_t>(data.size()));
#  endif

  if (m_process)
    return m_process->write(data);

  return -1;
}

/**
 * @brief Opens the data channel.
 *
 * Launch mode: resolves the executable against the shell PATH, then attempts
 * to spawn it under a PTY via openWithPty().  If PTY creation fails (e.g. on
 * a platform without ConPTY support) it falls back to openWithQProcess() with
 * anonymous pipes.
 *
 * NamedPipe mode: starts m_pipeThread which runs pipeReadLoop() and returns
 * immediately; pipe-open failures are reported asynchronously via onPipeError().
 *
 * @param mode Ignored — open mode is determined by the driver's Mode setting.
 * @return false if the executable cannot be resolved or the process fails to start.
 */
bool IO::Drivers::Process::open(const QIODevice::OpenMode mode)
{
  (void)mode;

  if (m_mode == Mode::Launch) {
    const QProcessEnvironment env = shellEnvironment();
    const QString resolved        = resolveExecutable(m_executable, env);

    if (resolved.isEmpty()) {
      Misc::Utilities::showMessageBox(tr("Failed to start process"),
                                      tr("Executable \"%1\" not found in PATH.").arg(m_executable),
                                      QMessageBox::Warning);
      return false;
    }

    const QStringList args = QProcess::splitCommand(m_arguments);

    if (openWithPty(resolved, args, env))
      return true;

    return openWithQProcess(resolved, args, env);
  }

  m_pipeRunning = true;
  m_pipeThread.start();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current driver mode as an integer (0 = Launch, 1 = NamedPipe).
 */
int IO::Drivers::Process::mode() const
{
  return static_cast<int>(m_mode);
}

/**
 * @brief Returns the configured executable path or bare name for Launch mode.
 */
QString IO::Drivers::Process::executable() const
{
  return m_executable;
}

/**
 * @brief Returns the command-line argument string for Launch mode.
 *
 * The string is split by QProcess::splitCommand() when the process starts,
 * so standard shell quoting rules apply.
 */
QString IO::Drivers::Process::arguments() const
{
  return m_arguments;
}

/**
 * @brief Returns the working directory for Launch mode.
 *
 * An empty string means the child inherits the application's current directory.
 */
QString IO::Drivers::Process::workingDir() const
{
  return m_workingDir;
}

/**
 * @brief Returns the named-pipe / FIFO path for NamedPipe mode.
 */
QString IO::Drivers::Process::pipePath() const
{
  return m_pipePath;
}

/**
 * @brief Returns the snapshot of running processes populated by refreshProcessList().
 *
 * Each entry is formatted as "name [PID]".
 */
QStringList IO::Drivers::Process::runningProcesses() const
{
  return m_runningProcesses;
}

/**
 * @brief Returns the output capture mode as an integer (0 = StdOut, 1 = StdErr, 2 = Both).
 *
 * Only meaningful for the QProcess pipe fallback; PTY mode always captures both
 * stdout and stderr through the single master fd.
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
 * @brief Sets the executable path or bare name for Launch mode.
 *
 * Persists the value and emits executableChanged() and configurationChanged().
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
  }
}

/**
 * @brief Sets the working directory for Launch mode.
 *
 * Pass an empty string to inherit the application's current directory.
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
 * @brief Sets the output capture mode for the QProcess pipe fallback.
 *
 * In Both mode QProcess merges channels at the OS level.  Has no effect when
 * the process is running under a PTY, which always merges stdout and stderr.
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
 * @brief Sets the named-pipe / FIFO path for NamedPipe mode.
 *
 * Persists the value and emits pipePathChanged() and configurationChanged().
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
 * @brief Enumerates running processes and updates the runningProcesses list.
 *
 * On Unix: runs `ps -eo pid,comm` and formats entries as "name [PID]".
 * On Windows: uses the Toolhelp32 snapshot API.
 *
 * Call this before opening the ProcessPicker dialog.  Emits
 * runningProcessesChanged() if the list differs from the previous snapshot.
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

/**
 * @brief Resizes the PTY to match the given terminal dimensions.
 *
 * On Unix, sends TIOCSWINSZ to the master fd so the child process receives
 * a SIGWINCH and can reflow its output to the new dimensions.
 * On Windows, calls ResizePseudoConsole() on the ConPTY handle.
 * Has no effect when running under the QProcess pipe fallback or when no
 * PTY is active.
 *
 * @param columns  Number of character columns visible in the terminal widget.
 * @param rows     Number of character rows visible in the terminal widget.
 */
void IO::Drivers::Process::setTerminalSize(int columns, int rows)
{
  if (columns <= 0 || rows <= 0)
    return;

#  ifdef Q_OS_WIN
  if (m_hPseudoConsole) {
    using FnResizePseudoConsole = HRESULT(WINAPI*)(HPCON, COORD);
    auto fn                     = reinterpret_cast<FnResizePseudoConsole>(
      GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "ResizePseudoConsole"));
    if (fn) {
      COORD size{static_cast<SHORT>(columns), static_cast<SHORT>(rows)};
      fn(static_cast<HPCON>(m_hPseudoConsole), size);
    }
  }
#  else
  if (m_ptyMasterFd >= 0) {
    struct winsize ws{};
    ws.ws_col = static_cast<unsigned short>(columns);
    ws.ws_row = static_cast<unsigned short>(rows);
    ::ioctl(m_ptyMasterFd, TIOCSWINSZ, &ws);
  }
#  endif
}

//--------------------------------------------------------------------------------------------------
// Private: PTY launch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Spawns the child under a PTY so it sees a real terminal.
 *
 * Unix: openpty() allocates a master/slave pair; fork() + execvpe() runs the
 * child with the slave as its controlling terminal.  The parent retains the
 * master fd and starts ptyReadLoop() on m_ptyThread.
 *
 * Windows: CreatePseudoConsole (ConPTY) is loaded dynamically so the binary
 * still runs on older Windows versions; if the API is absent we return false
 * and the caller falls back to QProcess.
 *
 * @return true if the child started successfully, false to trigger fallback.
 */
bool IO::Drivers::Process::openWithPty(const QString& resolved,
                                       const QStringList& args,
                                       const QProcessEnvironment& env)
{
#  ifdef Q_OS_WIN
  using FnCreatePseudoConsole = HRESULT(WINAPI*)(COORD, HANDLE, HANDLE, DWORD, HPCON*);
  using FnClosePseudoConsole  = void(WINAPI*)(HPCON);

  HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
  auto fnCreate =
    reinterpret_cast<FnCreatePseudoConsole>(GetProcAddress(kernel32, "CreatePseudoConsole"));
  if (!fnCreate)
    return false;

  // Create two pipe pairs: one for stdin, one for stdout
  HANDLE hPipeInRead = INVALID_HANDLE_VALUE, hPipeInWrite = INVALID_HANDLE_VALUE;
  HANDLE hPipeOutRead = INVALID_HANDLE_VALUE, hPipeOutWrite = INVALID_HANDLE_VALUE;

  if (!CreatePipe(&hPipeInRead, &hPipeInWrite, nullptr, 0))
    return false;

  if (!CreatePipe(&hPipeOutRead, &hPipeOutWrite, nullptr, 0)) {
    CloseHandle(hPipeInRead);
    CloseHandle(hPipeInWrite);
    return false;
  }

  COORD consoleSize{220, 50};
  HPCON hpc  = nullptr;
  HRESULT hr = fnCreate(consoleSize, hPipeInRead, hPipeOutWrite, 0, &hpc);

  CloseHandle(hPipeInRead);
  CloseHandle(hPipeOutWrite);

  if (FAILED(hr)) {
    CloseHandle(hPipeInWrite);
    CloseHandle(hPipeOutRead);
    return false;
  }

  // Build STARTUPINFOEX with the pseudo-console attribute
  SIZE_T attrSize = 0;
  InitializeProcThreadAttributeList(nullptr, 1, 0, &attrSize);
  auto attrBuf   = std::make_unique<char[]>(attrSize);
  auto pAttrList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrBuf.get());
  InitializeProcThreadAttributeList(pAttrList, 1, 0, &attrSize);
  UpdateProcThreadAttribute(
    pAttrList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hpc, sizeof(hpc), nullptr, nullptr);

  STARTUPINFOEXW si{};
  si.StartupInfo.cb      = sizeof(si);
  si.lpAttributeList     = pAttrList;
  si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;

  // Build command line string
  QString cmdLine = "\"" + resolved + "\"";
  for (const QString& a : args)
    cmdLine += " \"" + a + "\"";

  // Build environment block
  QString envBlock;
  for (const QString& key : env.keys())
    envBlock += key + "=" + env.value(key) + '\0';
  envBlock += '\0';

  const std::wstring wCmdLine = cmdLine.toStdWString();
  const std::wstring wWorkDir =
    m_workingDir.isEmpty() ? std::wstring() : m_workingDir.toStdWString();
  const LPCWSTR pWorkDir = wWorkDir.empty() ? nullptr : wWorkDir.c_str();

  std::wstring wEnvBlock(envBlock.toStdWString());

  PROCESS_INFORMATION pi{};
  BOOL ok = CreateProcessW(nullptr,
                           const_cast<LPWSTR>(wCmdLine.c_str()),
                           nullptr,
                           nullptr,
                           FALSE,
                           EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT,
                           wEnvBlock.data(),
                           pWorkDir,
                           &si.StartupInfo,
                           &pi);

  DeleteProcThreadAttributeList(pAttrList);

  if (!ok) {
    auto fn =
      reinterpret_cast<FnClosePseudoConsole>(GetProcAddress(kernel32, "ClosePseudoConsole"));
    if (fn)
      fn(hpc);
    CloseHandle(hPipeInWrite);
    CloseHandle(hPipeOutRead);
    return false;
  }

  m_hPseudoConsole = hpc;
  m_conPtyIn       = hPipeInWrite;
  m_conPtyOut      = hPipeOutRead;
  m_hProcess       = pi.hProcess;
  m_hThread        = pi.hThread;

  m_ptyRunning = true;
  m_ptyThread.start();
  return true;

#  else
  // Unix: openpty + fork + execvpe
  //
  // Disable PTY line-discipline echo (ECHO/ECHOE/ECHOK/ECHONL).  The shell's
  // own line editor (readline / ZLE) echoes every character it receives, so
  // leaving kernel echo enabled causes every keystroke to appear twice.
  struct termios pts{};
  ::cfmakeraw(&pts);
  pts.c_lflag &= ~static_cast<tcflag_t>(ECHO | ECHOE | ECHOK | ECHONL);
  pts.c_oflag |= OPOST | ONLCR;
  pts.c_cc[VMIN]  = 1;
  pts.c_cc[VTIME] = 0;

  int master = -1, slave = -1;
  if (::openpty(&master, &slave, nullptr, &pts, nullptr) < 0)
    return false;

  // Build argv
  const QByteArray resolvedBytes = resolved.toLocal8Bit();
  std::vector<QByteArray> argBytes;
  argBytes.push_back(resolvedBytes);
  for (const QString& a : args)
    argBytes.push_back(a.toLocal8Bit());

  std::vector<char*> argv;
  for (auto& b : argBytes)
    argv.push_back(b.data());
  argv.push_back(nullptr);

  // Build envp — ensure TERM is set and suppress zsh's partial-line marker
  QProcessEnvironment childEnv = env;
  if (!childEnv.contains("TERM"))
    childEnv.insert("TERM", "xterm-256color");

  childEnv.insert("PROMPT_EOL_MARK", "");
  childEnv.insert("PROMPT_SP", "");

  const QStringList keys = childEnv.keys();
  std::vector<QByteArray> envBytes;
  for (const QString& key : keys)
    envBytes.push_back((key + "=" + childEnv.value(key)).toLocal8Bit());

  std::vector<char*> envp;
  for (auto& b : envBytes)
    envp.push_back(b.data());
  envp.push_back(nullptr);

  // Set a reasonable terminal window size on the master before forking
  struct winsize ws{};
  ws.ws_col = 220;
  ws.ws_row = 50;
  ::ioctl(master, TIOCSWINSZ, &ws);

  const QByteArray wdBytes = m_workingDir.toLocal8Bit();

  const pid_t pid = ::fork();

  if (pid < 0) {
    ::close(master);
    ::close(slave);
    return false;
  }

  if (pid == 0) {
    // Child: make slave the controlling terminal
    ::close(master);
    ::setsid();

#    ifdef TIOCSCTTY
    ::ioctl(slave, TIOCSCTTY, 0);
#    endif

    ::dup2(slave, STDIN_FILENO);
    ::dup2(slave, STDOUT_FILENO);
    ::dup2(slave, STDERR_FILENO);

    if (slave > STDERR_FILENO)
      ::close(slave);

    if (!wdBytes.isEmpty())
      ::chdir(wdBytes.constData());

    ::execve(resolvedBytes.constData(), argv.data(), envp.data());
    ::_exit(127);
  }

  // Parent
  ::close(slave);
  m_ptyMasterFd = master;
  m_childPid    = pid;

  m_ptyRunning = true;
  m_ptyThread.start();
  return true;
#  endif
}

/**
 * @brief Falls back to QProcess (anonymous pipes) when PTY creation fails.
 */
bool IO::Drivers::Process::openWithQProcess(const QString& resolved,
                                            const QStringList& args,
                                            const QProcessEnvironment& env)
{
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

  m_process->setProcessEnvironment(env);
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

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles data available from the QProcess pipe fallback.
 *
 * Reads the configured output channel and emits dataReceived().
 * Not used when running under a PTY — data arrives via ptyReadLoop() instead.
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
 * @brief Handles QProcess pipe-fallback termination.
 *
 * Shows an informational message box and requests disconnection via a queued
 * call so the UI is updated on the main thread. The isConnected() guard
 * prevents a spurious dialog when doClose() already initiated the disconnect.
 */
void IO::Drivers::Process::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
  if (!IO::ConnectionManager::instance().isConnected())
    return;

  const QString reason = (status == QProcess::CrashExit) ? tr("The process crashed.")
                                                         : tr("Exit code: %1").arg(exitCode);

  Misc::Utilities::showMessageBox(
    tr("Process \"%1\" stopped").arg(QFileInfo(m_executable).fileName()),
    reason,
    QMessageBox::Warning);

  QMetaObject::invokeMethod(
    &IO::ConnectionManager::instance(), "disconnectDevice", Qt::QueuedConnection);
}

/**
 * @brief Handles a QProcess-level error during execution.
 *
 * FailedToStart is ignored — it is already reported by open() via
 * waitForStarted(). Crashed is ignored when not connected (it fires after
 * doClose() calls terminate()). All other errors show a warning and trigger
 * disconnect.
 */
void IO::Drivers::Process::onProcessError(QProcess::ProcessError error)
{
  if (error == QProcess::FailedToStart)
    return;

  if (!IO::ConnectionManager::instance().isConnected())
    return;

  const QString detail = m_process ? m_process->errorString() : tr("Unknown error");
  Misc::Utilities::showMessageBox(tr("Process Error"), detail, QMessageBox::Warning);

  QMetaObject::invokeMethod(
    &IO::ConnectionManager::instance(), "disconnectDevice", Qt::QueuedConnection);
}

/**
 * @brief Called on the main thread when pipeReadLoop() fails to open the pipe.
 *
 * Shows a warning and triggers a full disconnect so the UI reflects the closed
 * state.  Marshalled here from m_pipeThread via a queued invokeMethod() because
 * UI operations must not be performed on worker threads.
 */
void IO::Drivers::Process::onPipeError()
{
  Misc::Utilities::showMessageBox(
    tr("Pipe Error"), tr("Could not open named pipe: %1").arg(m_pipePath), QMessageBox::Warning);

  QMetaObject::invokeMethod(
    &IO::ConnectionManager::instance(), "disconnectDevice", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Private: PTY read loop (runs on m_ptyThread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads data from the PTY master fd and forwards it via dataReceived().
 *
 * On Unix: polls the master fd with a 100 ms timeout so m_ptyRunning can be
 * checked between iterations.  EIO on the master means the child exited.
 *
 * On Windows: uses ReadFile on the ConPTY output pipe with overlapped I/O so
 * we can respect m_ptyRunning without blocking forever.
 */
void IO::Drivers::Process::ptyReadLoop()
{
#  ifdef Q_OS_WIN
  // Capture the pipe handle locally. doClose() zeros m_conPtyOut before
  // closing the real HANDLE so that we never pass INVALID_HANDLE_VALUE to
  // GetOverlappedResult() while I/O is still completing.
  const HANDLE pipe = m_conPtyOut;

  char buf[4096];
  OVERLAPPED ov{};
  ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

  while (m_ptyRunning.load()) {
    DWORD bytesRead = 0;
    ResetEvent(ov.hEvent);
    const BOOL ok = ReadFile(pipe, buf, sizeof(buf), &bytesRead, &ov);

    if (!ok && GetLastError() == ERROR_IO_PENDING) {
      const DWORD rc = WaitForSingleObject(ov.hEvent, 100);
      if (rc == WAIT_TIMEOUT)
        continue;

      if (rc != WAIT_OBJECT_0)
        break;

      if (!GetOverlappedResult(pipe, &ov, &bytesRead, FALSE))
        break;
    } else if (!ok) {
      break;
    }

    if (bytesRead > 0)
      Q_EMIT dataReceived(makeByteArray(QByteArray(buf, static_cast<int>(bytesRead))));
  }

  CloseHandle(ov.hEvent);
#  else
  char buf[4096];
  struct pollfd pfd{};
  pfd.fd     = m_ptyMasterFd;
  pfd.events = POLLIN;

  while (m_ptyRunning.load()) {
    const int rc = ::poll(&pfd, 1, 100);

    if (rc < 0)
      break;

    if (rc == 0)
      continue;

    if (pfd.revents & (POLLERR | POLLNVAL))
      break;

    if (pfd.revents & POLLIN) {
      const ssize_t n = ::read(m_ptyMasterFd, buf, sizeof(buf));

      if (n > 0)
        Q_EMIT dataReceived(makeByteArray(QByteArray(buf, static_cast<int>(n))));
      else if (n == 0 || (n < 0 && errno != EAGAIN && errno != EINTR))
        break;
    }

    if (pfd.revents & POLLHUP)
      break;
  }
#  endif

  m_ptyRunning = false;

  QMetaObject::invokeMethod(this, "onPtyStopped", Qt::QueuedConnection);
}

/**
 * @brief Called on the main thread when ptyReadLoop() detects the child has exited.
 *
 * Shows a warning dialog and disconnects the device.  The guard on isConnected()
 * prevents a duplicate dialog when the user initiates disconnect manually —
 * close() sets m_ptyRunning to false before the queued invocation fires.
 */
void IO::Drivers::Process::onPtyStopped()
{
  if (!IO::ConnectionManager::instance().isConnected())
    return;

  const QString name = QFileInfo(m_executable).fileName();
  Misc::Utilities::showMessageBox(
    tr("Process \"%1\" stopped").arg(name), tr("The process has exited."), QMessageBox::Warning);

  IO::ConnectionManager::instance().disconnectDevice();
}

//--------------------------------------------------------------------------------------------------
// Private: pipe read loop (runs on m_pipeThread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Blocking read loop for NamedPipe mode, executed on m_pipeThread.
 *
 * On Unix: creates the FIFO if it does not exist, opens it O_RDONLY|O_NONBLOCK,
 * then polls with a 100 ms timeout so m_pipeRunning can be checked between
 * iterations.  POLLHUP with n == 0 means the writer closed its end.
 *
 * On Windows: creates a named pipe server, waits for a writer to connect using
 * overlapped I/O, then reads with PeekNamedPipe / ReadFile in a polling loop.
 *
 * If the pipe cannot be opened, onPipeError() is scheduled on the main thread
 * via a queued QMetaObject::invokeMethod() because UI dialogs must not be
 * created on worker threads.
 */
void IO::Drivers::Process::pipeReadLoop()
{
#  ifdef Q_OS_WIN
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

//--------------------------------------------------------------------------------------------------
// Private: environment and executable helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the full login-shell environment, cached after the first call.
 *
 * GUI apps on macOS/Linux inherit a stripped environment from the window
 * manager.  Spawning the user's login shell with -l -c env gives the same
 * PATH seen in a terminal (Homebrew, nvm, pyenv, mise, etc.).
 *
 * On Windows, cmd.exe /c set expands the full user PATH from the registry
 * (winget, scoop, chocolatey, etc.).
 */
const QProcessEnvironment& IO::Drivers::Process::shellEnvironment()
{
  static QProcessEnvironment env = []() -> QProcessEnvironment {
#  ifdef Q_OS_WIN
    QProcess cmd;
    cmd.start("cmd.exe", QStringList{"/c", "set"});
    if (!cmd.waitForFinished(5000))
      return QProcessEnvironment::systemEnvironment();

    QProcessEnvironment result;
    const QString output = QString::fromLocal8Bit(cmd.readAllStandardOutput());
    for (const QString& line : output.split('\n', Qt::SkipEmptyParts)) {
      const int eq = line.indexOf('=');
      if (eq > 0)
        result.insert(line.left(eq).trimmed(), line.mid(eq + 1).trimmed());
    }

    return result.isEmpty() ? QProcessEnvironment::systemEnvironment() : result;
#  else
    const QByteArray shell = qgetenv("SHELL");
    if (shell.isEmpty())
      return QProcessEnvironment::systemEnvironment();

    QProcess sh;
    sh.start(QString::fromLocal8Bit(shell), QStringList{"-l", "-c", "env"});
    if (!sh.waitForFinished(5000))
      return QProcessEnvironment::systemEnvironment();

    QProcessEnvironment result;
    const QString output = QString::fromLocal8Bit(sh.readAllStandardOutput());
    for (const QString& line : output.split('\n', Qt::SkipEmptyParts)) {
      const int eq = line.indexOf('=');
      if (eq > 0)
        result.insert(line.left(eq), line.mid(eq + 1));
    }

    return result.isEmpty() ? QProcessEnvironment::systemEnvironment() : result;
#  endif
  }();

  return env;
}

/**
 * @brief Resolves a bare executable name against the shell PATH.
 *
 * Returns the full path, or an empty string when nothing is found.
 */
QString IO::Drivers::Process::resolveExecutable(const QString& name, const QProcessEnvironment& env)
{
  if (QFileInfo::exists(name))
    return name;

  const QString pathVar = env.value("PATH");
#  ifdef Q_OS_WIN
  const QStringList dirs = pathVar.split(';', Qt::SkipEmptyParts);
#  else
  const QStringList dirs = pathVar.split(':', Qt::SkipEmptyParts);
#  endif

  return QStandardPaths::findExecutable(name, dirs);
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Process configuration as a flat list of editable properties.
 * @return List of DriverProperty descriptors with current values.
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

  IO::DriverProperty capture;
  capture.key     = QStringLiteral("outputCapture");
  capture.label   = tr("Output Capture");
  capture.type    = IO::DriverProperty::ComboBox;
  capture.value   = static_cast<int>(m_outputCapture);
  capture.options = {tr("Stdout"), tr("Stderr"), tr("Both")};
  props.append(capture);

  return props;
}

/**
 * @brief Applies a single Process configuration change by key.
 * @param key   The DriverProperty::key that was edited.
 * @param value The new value chosen by the user.
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

  else if (key == QLatin1String("outputCapture"))
    setOutputCapture(value.toInt());
}

#endif  // BUILD_COMMERCIAL
