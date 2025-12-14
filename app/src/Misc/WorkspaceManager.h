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

#pragma once

#include <QObject>
#include <QSettings>

namespace Misc
{
/**
 * @class Misc::WorkspaceManager
 * @brief Manages the application's workspace directory for user data and
 *        generated files.
 *
 * The WorkspaceManager class provides centralized management of the
 * application's workspace directory, which serves as the default location for:
 * - Exported CSV files
 * - Console logs
 * - Project files
 * - Screenshots and captures
 * - Other user-generated content
 *
 * Key Features:
 * - **Persistent Configuration**: Workspace path is stored in QSettings and
 *   persists across application sessions
 * - **Default Path**: Automatically selects an appropriate default location
 *   based on platform conventions
 * - **User Selection**: Allows users to choose a custom workspace location
 * - **Subdirectory Management**: Creates and manages organized subdirectories
 *   within the workspace
 * - **Path Formatting**: Provides both full and shortened path representations
 *   for UI display
 *
 * Default Workspace Locations:
 * - **Windows**: %USERPROFILE%/Documents/Serial Studio
 * - **macOS**: ~/Documents/Serial Studio
 * - **Linux**: ~/Documents/Serial Studio (or ~/serial-studio if Documents
 *   doesn't exist)
 *
 * Typical Usage:
 * @code
 * // Get the base workspace path
 * QString workspace = Misc::WorkspaceManager::instance().path();
 *
 * // Get path to a specific subdirectory (auto-created if needed)
 * QString csvPath = Misc::WorkspaceManager::instance().path("CSV");
 *
 * // Let user select a new workspace location
 * Misc::WorkspaceManager::instance().selectPath();
 * @endcode
 *
 * @note The workspace directory and requested subdirectories are automatically
 *       created if they don't exist.
 */
class WorkspaceManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString path READ path NOTIFY pathChanged)
  Q_PROPERTY(QString shortPath READ shortPath NOTIFY pathChanged)

signals:
  void pathChanged();

private:
  explicit WorkspaceManager();
  WorkspaceManager(WorkspaceManager &&) = delete;
  WorkspaceManager(const WorkspaceManager &) = delete;
  WorkspaceManager &operator=(WorkspaceManager &&) = delete;
  WorkspaceManager &operator=(const WorkspaceManager &) = delete;

public:
  static WorkspaceManager &instance();

  [[nodiscard]] QString path() const;
  [[nodiscard]] QString shortPath() const;

  [[nodiscard]] QString path(const QString &subdirectory) const;

public slots:
  void reset();
  void selectPath();

private:
  QString m_path;
  QSettings m_settings;
};
} // namespace Misc
