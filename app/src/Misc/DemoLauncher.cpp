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

#include "Misc/DemoLauncher.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>

#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the demo launcher.
 */
Misc::DemoLauncher::DemoLauncher() {}

/**
 * @brief Returns the singleton instance of the demo launcher.
 */
Misc::DemoLauncher& Misc::DemoLauncher::instance()
{
  static DemoLauncher singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stage a writable copy, open it under a demo session (never persisted as the last
 *        project), gate the Pro 3D group, then connect so the simulation runs. A failed load
 *        restores the prior mode and session; when the demo is already loaded it re-arms the
 *        session, re-applies mode and gating, and reconnects instead of re-staging.
 */
bool Misc::DemoLauncher::startDemo()
{
  auto& state = AppState::instance();
  if (state.ephemeralSession())
    return false;

  auto& io          = IO::ConnectionManager::instance();
  const auto& model = DataModel::ProjectModel::instance();

  if (model.jsonFilePath() == demoProjectPath()) {
    state.setDemoSession(demoDirectory());
    state.setOperationMode(SerialStudio::ProjectFile);
    applyProGating();

    if (io.isConnected())
      io.disconnectDevice();

    io.connectDevice();
    return true;
  }

  QString projectPath;
  if (!stageDemoProject(projectPath)) {
    showStartFailure();
    return false;
  }

  const auto previousMode = state.operationMode();
  state.setDemoSession(demoDirectory());

  if (!DataModel::ProjectModel::instance().openJsonFile(projectPath)) {
    state.setDemoSession(QString());
    state.setOperationMode(previousMode);
    qWarning() << "[Demo] Failed to open staged demo project:" << projectPath;
    showStartFailure();
    return false;
  }

  applyProGating();

  if (io.isConnected())
    io.disconnectDevice();

  io.connectDevice();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the writable directory that holds the staged demo project.
 */
QString Misc::DemoLauncher::demoDirectory() const
{
  const auto base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return base + QStringLiteral("/demo");
}

/**
 * @brief Returns the staged demo project path; the single construction point that the
 *        already-loaded fast path and the AppState prefix guard both rely on byte-for-byte.
 */
QString Misc::DemoLauncher::demoProjectPath() const
{
  return demoDirectory() + QStringLiteral("/RocketLaunch.ssproj");
}

/**
 * @brief Copies the bundled demo project into a writable, deterministic staging copy and
 *        clears the read-only permissions resource copies carry (autosave and the watcher
 *        need write access). A leftover read-only copy from a torn previous staging gets its
 *        permissions fixed and the removal retried; Windows cannot delete read-only files.
 */
bool Misc::DemoLauncher::stageDemoProject(QString& projectPath) const
{
  const QString dir = demoDirectory();
  if (!QDir().mkpath(dir)) {
    qWarning() << "[Demo] Cannot create demo directory:" << dir;
    return false;
  }

  const auto permissions = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup
                         | QFileDevice::ReadOther;

  projectPath = demoProjectPath();
  if (QFile::exists(projectPath) && !QFile::remove(projectPath)) {
    QFile::setPermissions(projectPath, permissions);
    if (!QFile::remove(projectPath)) {
      qWarning() << "[Demo] Cannot replace staged demo project:" << projectPath;
      return false;
    }
  }

  if (!QFile::copy(QStringLiteral(":/demo/RocketLaunch.ssproj"), projectPath)) {
    qWarning() << "[Demo] Cannot stage demo project:" << projectPath;
    return false;
  }

  if (!QFile::setPermissions(projectPath, permissions)) {
    qWarning() << "[Demo] Cannot make staged demo project writable:" << projectPath;
    return false;
  }

  return true;
}

/**
 * @brief Enables the Pro-only groups (the Mission View painter, and any 3D plot) when Pro
 *        widgets are available; they ship disabled in the bundled project so GPL and
 *        unlicensed builds never render them.
 */
void Misc::DemoLauncher::applyProGating() const
{
  auto& model        = DataModel::ProjectModel::instance();
  const auto& groups = model.groups();

  for (size_t i = 0; i < groups.size(); ++i)
    if (groups[i].widget == QStringLiteral("painter")
        || groups[i].widget == QStringLiteral("plot3d"))
      model.setGroupEnabled(static_cast<int>(i), SerialStudio::proWidgetsEnabled());
}

/**
 * @brief Tells the user the demo could not start; qWarning alone would leave the UI silent
 *        because the QML entry points do not inspect the return value.
 */
void Misc::DemoLauncher::showStartFailure() const
{
  Misc::Utilities::showMessageBox(tr("Cannot start the demo"),
                                  tr("The demo project could not be staged or opened. Check "
                                     "the application data directory permissions."),
                                  QMessageBox::Critical);
}
