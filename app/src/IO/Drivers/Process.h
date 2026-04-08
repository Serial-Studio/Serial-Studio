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

#include <atomic>
#include <QFileDialog>
#include <QList>
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QThread>

#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief Process I/O driver supporting two modes: child-process and named pipe.
 *
 * In Launch mode, Serial Studio spawns a child process via QProcess and reads
 * its stdout+stderr (merged).
 *
 * In NamedPipe mode, Serial Studio opens a named pipe / FIFO written by an
 * external process and reads from it.
 */
class Process : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int mode
             READ  mode
             WRITE setMode
             NOTIFY modeChanged)
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
  void runningProcessesChanged();

public:
  explicit Process();
  ~Process();

  Process(Process&&)                 = delete;
  Process(const Process&)            = delete;
  Process& operator=(Process&&)      = delete;
  Process& operator=(const Process&) = delete;

  enum class Mode {
    Launch    = 0,
    NamedPipe = 1
  };
  Q_ENUM(Mode)

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] int mode() const noexcept;
  [[nodiscard]] QString pipePath() const;
  [[nodiscard]] QString workingDir() const;
  [[nodiscard]] QString arguments() const;
  [[nodiscard]] QString executable() const;
  [[nodiscard]] QStringList runningProcesses() const;

public slots:
  void setMode(int mode);
  void setPipePath(const QString& path);
  void setWorkingDir(const QString& dir);
  void setArguments(const QString& args);
  void setExecutable(const QString& path);
  void browseExecutable();
  void browseWorkingDir();
  void browsePipePath();
  void refreshProcessList();
  void setDriverProperty(const QString& key, const QVariant& value) override;

private slots:
  void onPipeError();
  void onReadyRead();
  void onProcessFinished(int exitCode, QProcess::ExitStatus status);
  void onProcessError(QProcess::ProcessError error);

private:
  void doClose();
  void pipeReadLoop();

  static QStringList extraSearchPaths();
  static QString resolveExecutable(const QString& name);

private:
  Mode m_mode;
  QProcess* m_process;

  QThread m_pipeThread;
  std::atomic<bool> m_pipeRunning;

  QSettings m_settings;
  QString m_executable;
  QString m_arguments;
  QString m_workingDir;
  QString m_pipePath;
  QStringList m_runningProcesses;
};

}  // namespace Drivers
}  // namespace IO
