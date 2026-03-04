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

namespace UI {
class Taskbar;
class WindowManager;

/**
 * @brief Singleton registry tracking active Taskbar and WindowManager instances.
 *
 * Because Taskbar and WindowManager are QML-instantiated QQuickItems (not
 * singletons), this registry provides a stable access point for the API layer.
 * The primary instance is always the first one registered.
 */
class UISessionRegistry : public QObject {
  Q_OBJECT

public:
  static UISessionRegistry& instance();

  void registerTaskbar(Taskbar* t);
  void unregisterTaskbar(Taskbar* t);

  void registerWindowManager(WindowManager* wm);
  void unregisterWindowManager(WindowManager* wm);

  [[nodiscard]] Taskbar* primaryTaskbar() const;
  [[nodiscard]] WindowManager* primaryWindowManager() const;

signals:
  void taskbarAvailable();
  void taskbarUnavailable();
  void windowManagerAvailable();
  void windowManagerUnavailable();

private:
  explicit UISessionRegistry(QObject* parent = nullptr);

  Taskbar* m_primaryTaskbar;
  WindowManager* m_primaryWindowManager;
};

}  // namespace UI
