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

#include "CSD.h"
#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

//------------------------------------------------------------------------------
// Static storage for window decorators
//------------------------------------------------------------------------------

static QHash<QWindow *, CSD::Window *> s_decorators;

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
 * @brief Configures native window customization for *NIX environments.
 * @param window The window to customize.
 * @param color Optional color for the title bar (hex string).
 *
 * Creates a UnixWindowDecorator for the window, which provides:
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

  // Create decorator for this window
  auto *decorator = new CSD::Window(w, color, this);
  s_decorators.insert(w, decorator);

  // Clean up decorator when window is destroyed
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

  // Connect active state changes
  connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);
}

/**
 * @brief Handles theme change events.
 *
 * Propagates theme updates to all managed window decorators, causing
 * them to update their title bar colors based on the new theme.
 */
void NativeWindow::onThemeChanged()
{
  for (auto *window : m_windows)
  {
    auto *decorator = s_decorators.value(window, nullptr);
    if (decorator)
      decorator->updateTheme();
  }
}

/**
 * @brief Handles the active state change of a window.
 *
 * The CSD window class handles active state changes internally via
 * its connection to QWindow::activeChanged, so this method serves as
 * an extension point for additional behavior if needed.
 */
void NativeWindow::onActiveChanged()
{
  // Decorator handles active state internally via direct connection
  // to QWindow::activeChanged. This slot is available for any
  // additional behavior that may be needed in the future.
}
