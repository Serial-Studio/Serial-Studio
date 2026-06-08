/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
#import <objc/runtime.h>

//--------------------------------------------------------------------------------------------------
// macOS quit interceptor via method swizzling
//--------------------------------------------------------------------------------------------------

static NativeWindow *s_nativeWindowInstance = nullptr;

/**
 * @brief Swizzled applicationShouldTerminate that defers to Qt and cancels native termination.
 */
static NSApplicationTerminateReply swizzled_applicationShouldTerminate(id self, SEL _cmd,
                                                                       NSApplication *sender)
{
  (void)self;
  (void)_cmd;
  (void)sender;

  if (s_nativeWindowInstance)
    QMetaObject::invokeMethod(s_nativeWindowInstance, "quitRequested", Qt::QueuedConnection);

  return NSTerminateCancel;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructor for NativeWindow class.
 */
NativeWindow::NativeWindow(QObject *parent)
  : QObject(parent)
{
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &NativeWindow::onThemeChanged);
}

/**
 * @brief Installs a macOS quit interceptor that prevents NSApp terminate from killing the process,
 *        emitting quitRequested() instead.
 */
void NativeWindow::installMacOSQuitInterceptor()
{
  s_nativeWindowInstance = this;

  id delegate = [NSApp delegate];
  if (!delegate)
    return;

  Class delegateClass = [delegate class];
  SEL selector = @selector(applicationShouldTerminate:);

  class_replaceMethod(delegateClass, selector,
                      (IMP)swizzled_applicationShouldTerminate,
                      "i@:@");
}

/**
 * @brief Retrieves the height of the title bar.
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  QWindow *win = qobject_cast<QWindow *>(window);
  if (!win)
    return 0;

  NSView *view = reinterpret_cast<NSView *>(win->winId());
  if (!view)
    return 32;

  NSWindow *w = [view window];
  if (!w)
    return 32;

  if (([w styleMask] & NSWindowStyleMaskFullScreen))
    return 0;

  else
    return 32;
}

/**
 * @brief No-op on macOS; native frames have no extra horizontal/bottom inset.
 */
int NativeWindow::frameMargin(QObject *window)
{
  Q_UNUSED(window)
  return 0;
}

/**
 * @brief No-op on macOS; the titlebar height is reported via titlebarHeight().
 */
int NativeWindow::frameTopInset(QObject *window)
{
  Q_UNUSED(window)
  return 0;
}

/**
 * @brief Removes a window to the management list of NativeWindow.
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
 * @brief Applies the integrated-titlebar style to a raw NSWindow.
 */
static void applyMacOSWindowStyleToNSWindow(NSWindow *w)
{
  if (!w)
    return;

  [w setStyleMask:([w styleMask] | NSWindowStyleMaskFullSizeContentView)];
  [w setTitlebarAppearsTransparent:YES];
  [w setTitleVisibility:NSWindowTitleHidden];

  NSWindowCollectionBehavior behaviour = [w collectionBehavior];
  behaviour &= ~NSWindowCollectionBehaviorFullScreenPrimary;
  behaviour |=  NSWindowCollectionBehaviorFullScreenAuxiliary;
  [w setCollectionBehavior:behaviour];

  NSButton *zoomButton = [w standardWindowButton:NSWindowZoomButton];
  [zoomButton setEnabled:YES];
}

/**
 * @brief Applies native window customization for macOS.
 */
static void applyMacOSWindowStyle(QWindow *win)
{
  if (!win)
    return;

  NSView *view = reinterpret_cast<NSView *>(win->winId());
  if (!view)
    return;

  applyMacOSWindowStyleToNSWindow([view window]);
}

/**
 * @brief Initializes the native window customization for macOS.
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

    NSView *view = reinterpret_cast<NSView *>(win->winId());
    NSWindow *nsWin = view ? [view window] : nil;
    if (nsWin)
    {
      [[NSNotificationCenter defaultCenter]
          addObserverForName:NSWindowDidExitFullScreenNotification
                      object:nsWin
                       queue:[NSOperationQueue mainQueue]
                  usingBlock:^(NSNotification *note) {
                    applyMacOSWindowStyleToNSWindow((NSWindow *)note.object);
                  }];
    }
  }
}

/**
 * @brief Handles theme change events.
 */
void NativeWindow::onThemeChanged()
{
}

/**
 * @brief Handles the active state change of a window.
 */
void NativeWindow::onActiveChanged()
{
}

/**
 * @brief Handles window state changes to re-apply customization.
 */
void NativeWindow::onWindowStateChanged(Qt::WindowState state)
{
  auto *win = qobject_cast<QWindow *>(sender());
  if (!win || !m_windows.contains(win))
    return;

  if (state != Qt::WindowFullScreen)
    applyMacOSWindowStyle(win);
}
