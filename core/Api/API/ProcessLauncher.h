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

#include "API/LaunchConsent.h"
#include "Core/IO/LaunchAuthorizer.h"

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
 *        It is also the launch authorizer every program start asks (spec 0098), so a script's
 *        system.exec and a Process source answer to one per-project consent.
 */
class ProcessLauncher
  : public QObject
  , public IO::ILaunchAuthorizer {
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
  [[nodiscard]] IO::LaunchVerdict authorizeLaunch(const QString& program,
                                                  const QStringList& arguments,
                                                  const QString& workingDir) override;
  [[nodiscard]] bool kill(int processId);
  void killAll();

  [[nodiscard]] QVariantList runningProcesses() const;

public slots:
  void onRemoteProgramChanged();

private slots:
  void onAboutToQuit();
  void onConsentGranted();
  void onProjectFileChanged();

private:
  [[nodiscard]] static QStringList extraSearchPaths();
  [[nodiscard]] static QString resolveExecutable(const QString& name);

  void logLine(int id, const QString& name, const QString& message);
  void logProcessOutput(int id, const QString& name, const QByteArray& data);
  void reapAsync(QProcess* process);

private:
  int m_nextId;
  QSettings m_settings;
  LaunchConsent m_consent;
  bool m_reconnectOnGrant;
  QString m_lastProjectPath;
  QHash<int, QProcess*> m_processes;
  IO::ConnectionManager* m_connectionManager;
  DataModel::ProjectModel* m_projectModel;
};
}  // namespace API
