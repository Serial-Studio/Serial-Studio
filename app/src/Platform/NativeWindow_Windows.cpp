/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include*
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

#include <dwmapi.h>

#include <QColor>
#include <QSysInfo>
#include <QWindow>

#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

/**
 * @brief Constructor for NativeWindow class.
 * @param parent The parent QObject.
 *
 * Connects theme change signals to the appropriate slot for handling UI theme
 * updates.
 */
NativeWindow::NativeWindow(QObject *parent)
  : QObject(parent)
{
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &NativeWindow::onThemeChanged);
}

/**
 * @brief Returns the height of the title bar.
 * @param window The window for which the title bar height is queried.
 * @return Height of the title bar in pixels. Always returns 0 in this
 * implementation, since the client does not need to move the window elements
 * (e.g. the caption/titlebar is not transparent, as in macOS).
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  (void)window;
  return 0;
}

/**
 * @brief Adds a window to the management list of NativeWindow.
 * @param window Pointer to the window object to be managed.
 *
 * Registers the window for active change notifications and emits an initial
 * active changed signal.
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  auto *w = qobject_cast<QWindow *>(window);
  if (!m_windows.contains(w))
  {
    m_windows.append(w);
    m_colors.insert(w, color);
    connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);

    Q_EMIT w->activeChanged();
  }
}

/**
 * @brief Handles theme change events.
 *
 * Emits an active changed signal for each managed window to potentially update
 * its appearance based on the new theme.
 */
void NativeWindow::onThemeChanged()
{
  for (auto *window : m_windows)
    Q_EMIT window->activeChanged();
}

/**
 * @brief Handles the active state change of a window.
 *
 * Updates the window's caption color based on its active state using the
 * current theme's color settings.
 */
void NativeWindow::onActiveChanged()
{
  // Get caller window
  auto *window = static_cast<QWindow *>(sender());
  if (!window || !m_windows.contains(window))
    return;

  // Get color from color list
  auto color = m_colors.value(window);

  // Get color name
  if (color.isEmpty())
  {
    const auto &colors = Misc::ThemeManager::instance().colors();
    color = colors.value("toolbar_top").toString();
  }

  // Detect Windows version
  const QString version = QSysInfo::productVersion();
  const bool isWin11 = version.contains("11");
  const bool isWin10 = version.contains("10");

  // Convert hex string to native Windows color
  const QColor c(color);
  const COLORREF colorref = c.red() | (c.green() << 8) | (c.blue() << 16);

  // Obtain native window handle
  HWND hwnd = reinterpret_cast<HWND>(window->winId());

  // On Windows 11, set exact caption color using DWMWA_CAPTION_COLOR
  if (isWin11)
  {
    const DWORD attribute = 35;
    DwmSetWindowAttribute(hwnd, attribute, &colorref, sizeof(colorref));
  }

  // On Windows 10, set light or dark mode using DWMWA_USE_IMMERSIVE_DARK_MODE
  else if (isWin10)
  {
    const DWORD darkModeAttr = 20;
    BOOL useDarkMode = (c.lightness() < 128) ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, darkModeAttr, &useDarkMode,
                          sizeof(useDarkMode));
  }
}
