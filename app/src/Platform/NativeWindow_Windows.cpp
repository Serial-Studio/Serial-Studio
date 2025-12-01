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

#include "CSD.h"
#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

//------------------------------------------------------------------------------
// Static storage for window decorators (used on Windows 10)
//------------------------------------------------------------------------------

static QHash<QWindow *, CSD::Window *> s_decorators;

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------

/**
 * @brief Indicates whenever application is running on Windows 11
 * @return @c true if the operating system is Windows 11 or later
 */
static bool isWindows11OrLater()
{
  const QString version = QSysInfo::productVersion();
  return version.contains("11");
}

/**
 * @brief Updates the DWM caption color for a window (Windows 11 only).
 * @param window The window to update.
 *
 * Uses DWMWA_CAPTION_COLOR to set the exact title bar color matching
 * the application theme.
 */
static void updateWindowCaptionColor(QWindow *window)
{
  if (!window)
    return;

  // Get color from color list or theme
  QString color = m_colors.value(window);
  if (color.isEmpty())
  {
    const auto &colors = Misc::ThemeManager::instance().colors();
    color = colors.value("toolbar_top").toString();
  }

  // Convert hex string to native Windows color
  const QColor c(color);
  const COLORREF colorref = c.red() | (c.green() << 8) | (c.blue() << 16);

  // Obtain native window handle
  HWND hwnd = reinterpret_cast<HWND>(window->winId());

  // Set caption color using DWMWA_CAPTION_COLOR (attribute 35)
  constexpr DWORD DWMWA_CAPTION_COLOR = 35;
  DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &colorref, sizeof(colorref));
}

//------------------------------------------------------------------------------
// NativeWindow Implementation
//------------------------------------------------------------------------------

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
 * @return Height of the title bar in pixels.
 *
 * On Windows 11, returns 0 since native DWM caption is used.
 * On Windows 10, returns the CSD title bar height plus shadow margin.
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  if (isWindows11OrLater())
    return 0;

  auto *w = qobject_cast<QWindow *>(window);
  auto *decorator = s_decorators.value(w, nullptr);
  if (decorator)
    return decorator->shadowMargin() + decorator->titleBarHeight();

  return 0;
}

/**
 * @brief Removes a window from the management list of NativeWindow.
 * @param window Pointer to the window object to be removed.
 *
 * On Windows 10, also cleans up the associated CSD decorator.
 */
void NativeWindow::removeWindow(QObject *window)
{
  if (!isWindows11OrLater())
    return;

  auto *win = qobject_cast<QWindow *>(window);
  if (!win)
    return;

  auto index = m_windows.indexOf(win);
  if (index != -1 && index >= 0)
  {
    m_windows.removeAt(index);
    m_colors.remove(win);
  }
}

/**
 * @brief Adds a window to the management list of NativeWindow.
 * @param window Pointer to the window object to be managed.
 * @param color Optional color for the title bar (hex string).
 *
 * On Windows 11, uses native DWM caption color customization.
 * On Windows 10, creates a CSD decorator for custom title bar rendering.
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  auto *w = qobject_cast<QWindow *>(window);
  if (!w || m_windows.contains(w))
    return;

  // Add to managed windows list
  m_windows.append(w);
  m_colors.insert(w, color);

  // Connect active state changes
  connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);

  // Windows 11: Use native DWM caption color
  if (isWindows11OrLater())
    updateWindowCaptionColor(w);

  // Windows 10: Use CSD for custom title bar
  else
  {
    auto *decorator = new CSD::Window(w, color, this);
    s_decorators.insert(w, decorator);
    connect(w, &QObject::destroyed, this, [this, w]() {
      s_decorators.remove(w);
      m_windows.removeOne(w);
      m_colors.remove(w);
    });
  }
}

/**
 * @brief Handles theme change events.
 *
 * On Windows 11, updates DWM caption colors for all managed windows.
 * On Windows 10, propagates theme updates to CSD decorators.
 */
void NativeWindow::onThemeChanged()
{
  if (isWindows11OrLater())
  {
    for (auto *window : m_windows)
      updateWindowCaptionColor(window);
  }

  else
  {
    for (auto *window : m_windows)
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
 * On Windows 11, updates the DWM caption color.
 * On Windows 10, the CSD decorator handles active state internally.
 */
void NativeWindow::onActiveChanged()
{
  if (!isWindows11OrLater())
    return;

  auto *window = qobject_cast<QWindow *>(sender());
  if (window && m_windows.contains(window))
    updateWindowCaptionColor(window);
}
