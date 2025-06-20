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

#pragma once

#include <QObject>
#include <QSettings>

namespace Misc
{
/**
 * @brief Manages the application's workspace directory.
 *
 * Handles persistent workspace path configuration, including default path
 * resolution, user path selection, and automatic subdirectory creation.
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
