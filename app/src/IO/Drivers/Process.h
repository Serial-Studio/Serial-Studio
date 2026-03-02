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
#  include <QSettings>
#  include <QString>
#  include <QStringList>
#  include <QThread>

#  include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief Process I/O driver supporting two modes: child-process and named pipe.
 *
 * In Launch mode, Serial Studio spawns a child process and reads its stdout
 * while writing to its stdin. In NamedPipe mode, Serial Studio opens a named
 * pipe or FIFO written by an external process and reads from it.
 *
 * No additional third-party libraries are required — QProcess and QFile are
 * used from Qt Core.
 *
 * Threading:
 *   Launch mode: m_process runs on the main thread; readyRead() signals are
 *   handled synchronously.
 *   Pipe mode: m_pipeThread runs pipeReadLoop() with blocking waitForReadyRead()
 *   calls. Fatal errors are marshalled back to the main thread via
 *   QMetaObject::invokeMethod().
 */
class Process : public HAL_Driver {
  // clang-format off
  Q_OBJECT

  Q_PROPERTY(int mode
             READ  mode
             WRITE setMode
             NOTIFY modeChanged)
  Q_PROPERTY(QString executable
             READ  executable
             WRITE setExecutable
             NOTIFY executableChanged)
  Q_PROPERTY(QString arguments
             READ  arguments
             WRITE setArguments
             NOTIFY argumentsChanged)
  Q_PROPERTY(QString workingDir
             READ  workingDir
             WRITE setWorkingDir
             NOTIFY workingDirChanged)
  Q_PROPERTY(QString pipePath
             READ  pipePath
             WRITE setPipePath
             NOTIFY pipePathChanged)
  Q_PROPERTY(QStringList runningProcesses
             READ  runningProcesses
             NOTIFY runningProcessesChanged)
  Q_PROPERTY(int outputCapture
             READ  outputCapture
             WRITE setOutputCapture
             NOTIFY outputCaptureChanged)
  // clang-format on

public:
  enum class Mode {
    Launch    = 0, /**< Spawn a child process; read stdout, write stdin */
    NamedPipe = 1  /**< Open an existing named pipe / FIFO */
  };
  Q_ENUM(Mode)

  enum class OutputCapture {
    StdOut = 0, /**< Capture only stdout */
    StdErr = 1, /**< Capture only stderr */
    Both   = 2  /**< Merge stdout and stderr */
  };
  Q_ENUM(OutputCapture)

  static Process& instance();

  void close() override;
  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] int mode() const;
  [[nodiscard]] QString executable() const;
  [[nodiscard]] QString arguments() const;
  [[nodiscard]] QString workingDir() const;
  [[nodiscard]] QString pipePath() const;
  [[nodiscard]] QStringList runningProcesses() const;
  [[nodiscard]] int outputCapture() const;

public slots:
  void setMode(int mode);
  void setExecutable(const QString& path);
  void setArguments(const QString& args);
  void setWorkingDir(const QString& dir);
  void setPipePath(const QString& path);
  void setOutputCapture(int capture);
  void refreshProcessList();

signals:
  void modeChanged();
  void executableChanged();
  void argumentsChanged();
  void workingDirChanged();
  void pipePathChanged();
  void outputCaptureChanged();
  void runningProcessesChanged();

private:
  explicit Process();
  ~Process() = default;

  Process(Process&&)                 = delete;
  Process(const Process&)            = delete;
  Process& operator=(Process&&)      = delete;
  Process& operator=(const Process&) = delete;

  // Called on main thread (via queued invokeMethod) when pipe open fails
  Q_SLOT void onPipeError();

  void onProcessFinished(int exitCode, QProcess::ExitStatus status);
  void onProcessError(QProcess::ProcessError error);
  void onReadyRead();
  void pipeReadLoop();

  Mode m_mode;
  OutputCapture m_outputCapture;
  QProcess* m_process;
  QThread m_pipeThread;
  QSettings m_settings;

  std::atomic<bool> m_pipeRunning{false};

  QString m_executable;
  QString m_arguments;
  QString m_workingDir;
  QString m_pipePath;
  QStringList m_runningProcesses;
};

}  // namespace Drivers
}  // namespace IO

#endif  // BUILD_COMMERCIAL
