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

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class QProcess;

namespace API {
/**
 * @brief Owns helper processes spawned by the project control script. Every launched process
 *        is terminated on device disconnect, project change, and application quit so a project
 *        never leaves orphaned helpers (e.g. a Python data generator) running.
 */
class ProcessLauncher : public QObject {
  Q_OBJECT

signals:
  void processStarted(int processId);
  void processFinished(int processId, int exitCode);

private:
  explicit ProcessLauncher(QObject* parent = nullptr);
  ProcessLauncher(ProcessLauncher&&)                 = delete;
  ProcessLauncher(const ProcessLauncher&)            = delete;
  ProcessLauncher& operator=(ProcessLauncher&&)      = delete;
  ProcessLauncher& operator=(const ProcessLauncher&) = delete;

public:
  [[nodiscard]] static ProcessLauncher& instance();

  void setupExternalConnections();

  [[nodiscard]] int launch(const QString& program,
                           const QStringList& arguments,
                           const QString& workingDirectory,
                           QString& error);
  [[nodiscard]] bool kill(int processId);
  void killAll();

  [[nodiscard]] QVariantList runningProcesses() const;

private slots:
  void onConnectedChanged();
  void onProjectFileChanged();
  void onAboutToQuit();

private:
  [[nodiscard]] static QStringList extraSearchPaths();
  [[nodiscard]] static QString resolveExecutable(const QString& name);

  void logLine(int id, const QString& name, const QString& message);
  void logProcessOutput(int id, const QString& name, const QByteArray& data);

private:
  int m_nextId;
  bool m_wasConnected;
  QString m_lastProjectPath;
  QHash<int, QProcess*> m_processes;
};
}  // namespace API
