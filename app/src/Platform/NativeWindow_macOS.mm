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
 * @brief Installs a macOS quit interceptor that prevents NSApp terminate from
 *        killing the process, emitting quitRequested() instead.
 *
 * Uses Objective-C runtime method swizzling to replace
 * applicationShouldTerminate: on Qt's Cocoa app delegate.
 */
void NativeWindow::installMacOSQuitInterceptor()
{
  s_nativeWindowInstance = this;

  // Get Qt's Cocoa app delegate class
  id delegate = [NSApp delegate];
  if (!delegate)
    return;

  Class delegateClass = [delegate class];
  SEL selector = @selector(applicationShouldTerminate:);

  // Replace the method implementation
  class_replaceMethod(delegateClass, selector,
                      (IMP)swizzled_applicationShouldTerminate,
                      "i@:@");
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
 * @brief Applies the integrated-titlebar style to a raw NSWindow.
 *
 * Also remaps the green traffic-light button to perform a regular zoom
 * (maximize within the current Space, keep the title bar) instead of the
 * macOS Spaces-style full-screen toggle. Spaces full-screen plays badly with
 * Serial Studio's own UI flow -- the user clicked Connect and would suddenly
 * find the window flipping out of the chrome they were just using. Holding
 * Option while clicking the button still triggers the legacy behavior, and
 * `View -> Enter Full Screen` (or programmatic `showFullScreen()`) still works
 * because AppKit honours those independently of the auxiliary flag.
 */
static void applyMacOSWindowStyleToNSWindow(NSWindow *w)
{
  if (!w)
    return;

  [w setStyleMask:([w styleMask] | NSWindowStyleMaskFullSizeContentView)];
  [w setTitlebarAppearsTransparent:YES];
  [w setTitleVisibility:NSWindowTitleHidden];

  // Mark the window as full-screen "auxiliary" so the green button does plain zoom
  NSWindowCollectionBehavior behaviour = [w collectionBehavior];
  behaviour &= ~NSWindowCollectionBehaviorFullScreenPrimary;
  behaviour |=  NSWindowCollectionBehaviorFullScreenAuxiliary;
  [w setCollectionBehavior:behaviour];

  NSButton *zoomButton = [w standardWindowButton:NSWindowZoomButton];
  [zoomButton setEnabled:YES];
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
  if (!win)
    return;

  NSView *view = reinterpret_cast<NSView *>(win->winId());
  if (!view)
    return;

  applyMacOSWindowStyleToNSWindow([view window]);
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

    // Listen for AppKit's post-fullscreen-exit notification to reapply the style
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
