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

#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

#include <QWindow>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

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
 * @brief Retrieves the height of the title bar.
 *
 * This function returns the height of the title bar. When the window is in
 * full screen mode, the title bar height is considered to be 0. Otherwise,
 * it returns the standard title bar height. The titlebar height can be used
 * by the QML interface to provide functionality on macOS.
 *
 * @param window A pointer to the QWindow object for which to retrieve the
 *               title bar height.
 *
 * @return The height of the title bar. Returns 0 if the window is in full
 *         screen mode, else 32.
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  QWindow *win = qobject_cast<QWindow *>(window);
  NSView *view = reinterpret_cast<NSView *>(win->winId());
  NSWindow *w = [view window];

  if (([w styleMask] & NSWindowStyleMaskFullScreen))
    return 0;

  else
    return 32;
}

/**
 * @brief Removes a window to the management list of NativeWindow.
 * @param window Pointer to the window object to be managed.
 */
void NativeWindow::removeWindow(QObject *window)
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

/**
 * @brief Applies native window customization for macOS.
 *
 * This is a helper function that applies the actual native window styling.
 * It's called both when initially adding a window and when re-applying
 * customization after full screen mode exit.
 *
 * @param win The QWindow to customize.
 */
static void applyMacOSWindowStyle(QWindow *win)
{
  NSView *view = reinterpret_cast<NSView *>(win->winId());
  NSWindow *w = [view window];

  if (w)
  {
    [w setStyleMask:([w styleMask] | NSWindowStyleMaskFullSizeContentView)];
    [w setTitlebarAppearsTransparent:YES];
    [w setTitleVisibility:NSWindowTitleHidden];

    NSButton *zoomButton = [w standardWindowButton:NSWindowZoomButton];
    [zoomButton setEnabled:YES];

    [NSApp activateIgnoringOtherApps:YES];
    [w makeKeyAndOrderFront:nil];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC),
                   dispatch_get_main_queue(), ^{
                     [w makeKeyAndOrderFront:nil];
                   });
  }
}

/**
 * @brief Initializes the native window customization for macOS.
 *
 * This function sets the native macOS window to have a transparent titlebar
 * and to integrate the titlebar with the window's content. It also activates
 * the window and brings it to the front. The window is added to the managed
 * list and monitored for state changes to re-apply customization after
 * full screen mode exit.
 *
 * @param window A pointer to the QWindow object that represents the window
 *               being customized.
 * @param color Optional color parameter (unused on macOS).
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  (void)color;

  QWindow *win = qobject_cast<QWindow *>(window);
  if (!win)
    return;

  if (!win->handle())
    win->create();

  applyMacOSWindowStyle(win);
  if (!m_windows.contains(win))
  {
    m_windows.append(win);
    connect(win, &QWindow::windowStateChanged, this,
            &NativeWindow::onWindowStateChanged);
  }
}

/**
 * @brief Handles theme change events.
 */
void NativeWindow::onThemeChanged()
{
  // Nothing to do...
}

/**
 * @brief Handles the active state change of a window.
 */
void NativeWindow::onActiveChanged()
{
  // Nothing to do...
}

/**
 * @brief Handles window state changes to re-apply customization.
 *
 * When a window exits full screen mode on macOS, the system resets the
 * window style flags. This slot detects when a window exits full screen
 * and re-applies the native window customization.
 *
 * @param state The new window state.
 */
void NativeWindow::onWindowStateChanged(Qt::WindowState state)
{
  auto *win = qobject_cast<QWindow *>(sender());
  if (!win || !m_windows.contains(win))
    return;

  if (state != Qt::WindowFullScreen)
    applyMacOSWindowStyle(win);
}
