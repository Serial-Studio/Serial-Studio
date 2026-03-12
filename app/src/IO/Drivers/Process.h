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

#pragma once
#ifdef BUILD_COMMERCIAL

#  include <atomic>
#  include <QList>
#  include <QObject>
#  include <QProcess>
#  include <QProcessEnvironment>
#  include <QSettings>
#  include <QString>
#  include <QStringList>
#  include <QThread>

#  include "IO/HAL_Driver.h"

#  ifdef Q_OS_WIN
#    ifndef WIN32_LEAN_AND_MEAN
#      define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef NOMINMAX
#      define NOMINMAX
#    endif
// clang-format off
#    include <windows.h>
// clang-format on
#  else
#    include <sys/types.h>
#  endif

namespace IO {
namespace Drivers {

/**
 * @brief Process I/O driver supporting two modes: child-process and named pipe.
 *
 * In Launch mode, Serial Studio spawns a child process under a PTY (Unix) or
 * ConPTY (Windows) so interactive CLI tools (claude, gemini, python REPL, etc.)
 * see a real terminal and produce output normally.  Falls back to QProcess
 * anonymous pipes when PTY creation fails.
 *
 * In NamedPipe mode, Serial Studio opens a named pipe / FIFO written by an
 * external process and reads from it.
 *
 * Threading:
 *   Launch mode PTY: m_ptyThread runs ptyReadLoop() which polls the master fd.
 *   Launch mode pipe fallback: m_process runs on the main thread.
 *   Pipe mode: m_pipeThread runs pipeReadLoop().
 *   Fatal errors from worker threads are marshalled to the main thread via
 *   QMetaObject::invokeMethod().
 */
class Process : public HAL_Driver {
  // clang-format off
  Q_OBJECT

  Q_PROPERTY(int mode
             READ  mode
             WRITE setMode
             NOTIFY modeChanged)
  Q_PROPERTY(int outputCapture
             READ  outputCapture
             WRITE setOutputCapture
             NOTIFY outputCaptureChanged)
  Q_PROPERTY(QString pipePath
             READ  pipePath
             WRITE setPipePath
             NOTIFY pipePathChanged)
  Q_PROPERTY(QString workingDir
             READ  workingDir
             WRITE setWorkingDir
             NOTIFY workingDirChanged)
  Q_PROPERTY(QString arguments
             READ  arguments
             WRITE setArguments
             NOTIFY argumentsChanged)
  Q_PROPERTY(QString executable
             READ  executable
             WRITE setExecutable
             NOTIFY executableChanged)
  Q_PROPERTY(QStringList runningProcesses
             READ  runningProcesses
             NOTIFY runningProcessesChanged)
  // clang-format on

signals:
  void modeChanged();
  void pipePathChanged();
  void argumentsChanged();
  void workingDirChanged();
  void executableChanged();
  void outputCaptureChanged();
  void runningProcessesChanged();

public:
  explicit Process();
  ~Process();

  Process(Process&&)                 = delete;
  Process(const Process&)            = delete;
  Process& operator=(Process&&)      = delete;
  Process& operator=(const Process&) = delete;

  enum class Mode {
    Launch    = 0, /**< Spawn a child process; read stdout, write stdin */
    NamedPipe = 1  /**< Open an existing named pipe / FIFO */
  };
  Q_ENUM(Mode)

  enum class OutputCapture {
    StdOut = 0, /**< Capture only stdout (pipe fallback only) */
    StdErr = 1, /**< Capture only stderr (pipe fallback only) */
    Both   = 2  /**< Merge stdout and stderr */
  };
  Q_ENUM(OutputCapture)

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] int mode() const;
  [[nodiscard]] int outputCapture() const;
  [[nodiscard]] QString pipePath() const;
  [[nodiscard]] QString workingDir() const;
  [[nodiscard]] QString arguments() const;
  [[nodiscard]] QString executable() const;
  [[nodiscard]] QStringList runningProcesses() const;

public slots:
  void setMode(int mode);
  void setOutputCapture(int capture);
  void setPipePath(const QString& path);
  void setWorkingDir(const QString& dir);
  void setArguments(const QString& args);
  void setExecutable(const QString& path);
  void refreshProcessList();
  void setTerminalSize(int columns, int rows);
  void setDriverProperty(const QString& key, const QVariant& value) override;

private slots:
  void onPipeError();
  void onPtyStopped();
  void onReadyRead();
  void onProcessFinished(int exitCode, QProcess::ExitStatus status);
  void onProcessError(QProcess::ProcessError error);

private:
  void doClose();
  void pipeReadLoop();
  void ptyReadLoop();

  bool openWithPty(const QString& resolved,
                   const QStringList& args,
                   const QProcessEnvironment& env);
  bool openWithQProcess(const QString& resolved,
                        const QStringList& args,
                        const QProcessEnvironment& env);

  static const QProcessEnvironment& shellEnvironment();
  static QString resolveExecutable(const QString& name, const QProcessEnvironment& env);

private:
  Mode m_mode;
  OutputCapture m_outputCapture;

  QProcess* m_process;

  QThread m_ptyThread;
  std::atomic<bool> m_ptyRunning{false};

#  ifdef Q_OS_WIN
  HANDLE m_conPtyIn{INVALID_HANDLE_VALUE};
  HANDLE m_conPtyOut{INVALID_HANDLE_VALUE};
  HANDLE m_hPseudoConsole{nullptr};
  HANDLE m_hProcess{INVALID_HANDLE_VALUE};
  HANDLE m_hThread{INVALID_HANDLE_VALUE};
#  else
  int m_ptyMasterFd{-1};
  pid_t m_childPid{-1};
#  endif

  QThread m_pipeThread;
  std::atomic<bool> m_pipeRunning{false};

  QSettings m_settings;
  QString m_executable;
  QString m_arguments;
  QString m_workingDir;
  QString m_pipePath;
  QStringList m_runningProcesses;
};

}  // namespace Drivers
}  // namespace IO

#endif  // BUILD_COMMERCIAL
