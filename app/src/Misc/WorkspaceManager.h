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
