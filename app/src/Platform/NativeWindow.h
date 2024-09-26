/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _PLATFORM_NATIVE_WINDOW_H
#define _PLATFORM_NATIVE_WINDOW_H

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

#endif
