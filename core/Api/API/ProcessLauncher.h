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

#pragma once

#include <QHash>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "API/Server/ConsentGate.h"

class QProcess;

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace API {
/**
 * @brief Owns helper processes spawned by the project control script, terminating every one on
 *        device disconnect, project change, and application quit so no helper is orphaned.
 *        Launching is gated by a per-project user consent: a script runs programs only once
 *        the user said yes for THIS project file (session-only for a project without one).
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

  static constexpr int kLaunchDenied          = -3;
  static constexpr int kLaunchConsentRequired = -2;

  void setupExternalConnections();

  [[nodiscard]] int launch(const QString& program,
                           const QStringList& arguments,
                           const QString& workingDirectory,
                           QString& error);
  [[nodiscard]] bool kill(int processId);
  void killAll();

  [[nodiscard]] QVariantList runningProcesses() const;

private slots:
  void onAboutToQuit();
  void onProjectFileChanged();

private:
  [[nodiscard]] static QStringList extraSearchPaths();
  [[nodiscard]] static QString resolveExecutable(const QString& name);
  [[nodiscard]] static QString consentKeyFor(const QString& projectPath);

  void logLine(int id, const QString& name, const QString& message);
  void logProcessOutput(int id, const QString& name, const QByteArray& data);
  void reapAsync(QProcess* process);

private:
  int m_nextId;
  QSettings m_settings;
  ConsentGate m_consent;
  QString m_lastProjectPath;
  QHash<int, QProcess*> m_processes;
  IO::ConnectionManager* m_connectionManager;
  DataModel::ProjectModel* m_projectModel;
};
}  // namespace API
