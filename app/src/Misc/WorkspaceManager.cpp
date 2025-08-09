/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "WorkspaceManager.h"

#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

/**
 * @brief Constructs the WorkspaceManager and initializes the workspace path.
 *
 * Loads the saved workspace path from settings, or defaults to
 * <Documents>/Serial Studio if none is set.
 */
Misc::WorkspaceManager::WorkspaceManager()
{
  auto def = QString("%1/%2").arg(
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
      QStringLiteral("Serial Studio"));
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
 * Sets the path to <Documents>/Serial Studio, updates settings,
 * and emits pathChanged().
 */
void Misc::WorkspaceManager::reset()
{
  m_path = QString("%1/%2").arg(
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
      QStringLiteral("Serial Studio"));
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
  auto *dialog
      = new QFileDialog(nullptr, tr("Select Workspace Location"), m_path);

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            dialog->deleteLater();

            if (path.isEmpty())
              return;

            m_path = path;
            m_settings.setValue(QStringLiteral("Workspace"), path);

            emit pathChanged();
          });

  dialog->open();
}
