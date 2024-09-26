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
