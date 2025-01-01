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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
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
