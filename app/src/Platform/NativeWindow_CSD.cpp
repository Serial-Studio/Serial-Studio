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

#include <QColor>
#include <QSysInfo>

#if defined(Q_OS_WIN)
#  include <dwmapi.h>
#endif

#include "CSD.h"
#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

//------------------------------------------------------------------------------
// Static storage for window decorators
//------------------------------------------------------------------------------

static QHash<QWindow *, CSD::Window *> s_decorators;

//------------------------------------------------------------------------------
// Helper function to detect Windows 11
//------------------------------------------------------------------------------

static bool isWindows11()
{
#if defined(Q_OS_WIN)
  return QSysInfo::productVersion().contains("11");
#else
  return false;
#endif
}

//------------------------------------------------------------------------------
// NativeWindow Implementation
//------------------------------------------------------------------------------

/**
 * @brief Constructor for NativeWindow class.
 * @param parent The parent QObject.
 *
 * Connects theme change signals to the appropriate slot for handling UI theme
 * updates across all managed windows.
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
 *         implementation, since the client does not need to move the window
 *         elements (e.g. the caption/titlebar is not transparent, as in macOS).
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  (void)window;
  return 0;
}

/**
 * @brief Removes a window to the management list of NativeWindow.
 * @param window Pointer to the window object to be managed.
 */
void NativeWindow::removeWindow(QObject *window)
{
  (void)window;
}

/**
 * @brief Configures native window customization.
 * @param window The window to customize.
 * @param color Optional color for the title bar (hex string).
 *
 * On Windows 11: Uses native DWM caption color customization.
 * On Windows 10 and other platforms: Creates a CSD::Window which provides:
 * - Frameless window with custom title bar
 * - Window dragging and resizing
 * - Minimize, maximize, and close buttons
 * - Theme-aware coloring
 *
 * If a decorator already exists for the window, this method does nothing.
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  auto *w = qobject_cast<QWindow *>(window);
  if (!w || m_windows.contains(w))
    return;

  // Add to managed windows list
  m_windows.append(w);
  m_colors.insert(w, color);

  // Windows 11: Use native caption coloring instead of CSD
  if (isWindows11())
  {
    connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);
    Q_EMIT w->activeChanged();
  }

  // Windows 10 / Linux: Create CSD decorator for this window
  else
  {
    auto *decorator = new CSD::Window(w, color, this);
    s_decorators.insert(w, decorator);
    connect(w, &QObject::destroyed, this, [this, w]() {
      s_decorators.remove(w);
      auto *win = qobject_cast<QWindow *>(w);
      auto index = m_windows.indexOf(win);
      if (index != -1 && index >= 0)
      {
        m_windows.removeAt(index);
        if (m_colors.contains(win))
          m_colors.remove(win);
      }
    });

    connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);
  }
}

/**
 * @brief Handles theme change events.
 *
 * On Windows 11: Triggers caption color updates for all windows.
 * On other platforms: Propagates theme updates to all CSD decorators,
 * causing them to update their title bar colors based on the new theme.
 */
void NativeWindow::onThemeChanged()
{
  // Trigger active changed to update caption colors
  if (isWindows11())
  {
    for (auto *window : std::as_const(m_windows))
      Q_EMIT window->activeChanged();
  }

  // Update CSD decorators
  else
  {
    for (auto *window : std::as_const(m_windows))
    {
      auto *decorator = s_decorators.value(window, nullptr);
      if (decorator)
        decorator->updateTheme();
    }
  }
}

/**
 * @brief Handles the active state change of a window.
 *
 * On Windows 11: Updates the native caption color using DWM API based on
 * the window's active state and current theme.
 *
 * On other platforms: The CSD window class handles active state changes
 * internally via its connection to QWindow::activeChanged.
 */
void NativeWindow::onActiveChanged()
{
#if defined(Q_OS_WIN)
  if (!isWindows11())
    return;

  // Get caller window
  auto *window = static_cast<QWindow *>(sender());
  if (!window || !m_windows.contains(window))
    return;

  // Get color name based on active state
  const auto &colors = Misc::ThemeManager::instance().colors();
  QString colorName;

  // Use custom color if provided, otherwise use theme colors
  if (m_colors.contains(window) && !m_colors[window].isEmpty())
  {
    colorName = m_colors[window];
  }
  else
  {
    if (window->isActive())
      colorName = colors.value("toolbar_top").toString();
    else
      colorName = colors.value("toolbar_top").toString();
  }

  // Convert hex string to native Windows COLORREF
  const QColor color(colorName);
  const COLORREF colorref
      = color.red() | (color.green() << 8) | (color.blue() << 16);

  // Change color of the caption using DWM API
  const DWORD attribute = 35;
  DwmSetWindowAttribute((HWND)window->winId(), attribute, &colorref,
                        sizeof(colorref));
#else
  Q_UNUSED(this);
#endif
}
