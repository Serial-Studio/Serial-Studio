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
 * @brief Initializes the native window customization for macOS.
 *
 * This function sets the native macOS window to have a transparent titlebar
 * and to integrate the titlebar with the window's content.
 *
 * @param window A pointer to the QWindow object that represents the window
 *               being customized.
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  (void)color;

  QWindow *win = qobject_cast<QWindow *>(window);
  NSView *view = reinterpret_cast<NSView *>(win->winId());
  NSWindow *w = [view window];

  [w setStyleMask:([w styleMask] | NSWindowStyleMaskFullSizeContentView)];
  [w setTitlebarAppearsTransparent:YES];
  [w setTitleVisibility:NSWindowTitleHidden];

  NSButton *zoomButton = [w standardWindowButton:NSWindowZoomButton];
  [zoomButton setEnabled:YES];
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
