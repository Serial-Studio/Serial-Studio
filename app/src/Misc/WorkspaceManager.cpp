/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "AppInfo.h"
#include "WorkspaceManager.h"

#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

/**
 * @brief Constructs the WorkspaceManager and initializes the workspace path.
 *
 * Loads the saved workspace path from settings, or defaults to
 * <Documents>/<APP_NAME> if none is set.
 */
Misc::WorkspaceManager::WorkspaceManager()
{
  auto def = QString("%1/%2").arg(
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
      APP_NAME);
  m_path = m_settings.value(QStringLiteral("Workspace"), def).toString();
}

/**
 * @brief Returns the singleton instance of WorkspaceManager.
 *
 * @return Reference to the WorkspaceManager instance.
 */
Misc::WorkspaceManager &Misc::WorkspaceManager::instance()
{
  static WorkspaceManager instance;
  return instance;
}

/**
 * @brief Returns the base workspace path.
 *
 * @return Current workspace path.
 */
QString Misc::WorkspaceManager::path() const
{
  return m_path;
}

/**
 * @brief Returns the user friendly workspace path.
 *
 * @return Current workspace path.
 */
QString Misc::WorkspaceManager::shortPath() const
{
  return path().replace(QDir::homePath(), QStringLiteral("~"));
}

/**
 * @brief Returns the full path to a subdirectory within the workspace.
 *
 * Ensures the subdirectory exists by creating it if necessary.
 *
 * @param subdirectory Name of the subdirectory under the workspace.
 * @return Full path to the subdirectory.
 */
QString Misc::WorkspaceManager::path(const QString &subdirectory) const
{
  QString path = QStringLiteral("%1/%2").arg(m_path, subdirectory);

  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(".");

  return path;
}

/**
 * @brief Resets the workspace path to the default location.
 *
 * Sets the path to <Documents>/<APP_NAME>, updates settings,
 * and emits pathChanged().
 */
void Misc::WorkspaceManager::reset()
{
  m_path = QString("%1/%2").arg(
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
      APP_NAME);
  m_settings.setValue(QStringLiteral("Workspace"), m_path);

  Q_EMIT pathChanged();
}

/**
 * @brief Opens a dialog for the user to select a new workspace path.
 *
 * If a new path is selected, updates internal state and emits pathChanged().
 */
void Misc::WorkspaceManager::selectPath()
{
  auto path = QFileDialog::getExistingDirectory(
      nullptr, tr("Select Workspace Location"), m_path);

  if (!path.isEmpty())
  {
    m_path = path;
    m_settings.setValue(QStringLiteral("Workspace"), path);

    Q_EMIT pathChanged();
  }
}
