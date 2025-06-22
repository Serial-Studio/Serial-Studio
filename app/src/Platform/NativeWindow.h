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

#include <QMap>
#include <QObject>
#include <QWindow>

/**
 * @class NativeWindow
 * @brief Provides native window customization features across different
 *        operating systems.
 *
 * The class offers a set of methods to modify and interact with the window
 * system at a native level. This includes setting the style of the window's
 * caption and calculating the title bar height based on the window's state and
 * operating system specifics.
 *
 * The actual behavior of the class methods are dependent on the operating
 * system on which the application is compiled and run. This allows for
 * platform-specific window customizations such as transparent title bars on
 * macOS or caption colorization on Windows.
 */
class NativeWindow : public QObject
{
  Q_OBJECT

public:
  explicit NativeWindow(QObject *parent = nullptr);

  Q_INVOKABLE int titlebarHeight(QObject *window);

public slots:
  void addWindow(QObject *window, const QString &color = "");
  void removeWindow(QObject *window)
  {
    auto *win = qobject_cast<QWindow *>(window);
    auto index = m_windows.indexOf(win);
    if (index != -1 && index >= 0)
    {
      m_windows.removeAt(index);
      if (m_colors.contains(win))
        m_colors.remove(win);
    }
  }

private slots:
  void onThemeChanged();
  void onActiveChanged();

private:
  QList<QWindow *> m_windows;
  QMap<QWindow *, QString> m_colors;
};
