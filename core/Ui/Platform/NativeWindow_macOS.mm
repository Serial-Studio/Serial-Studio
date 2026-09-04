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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "NativeWindow.h"
#include "Misc/ThemeManager.h"

#include <QColor>
#include <QHash>
#include <QWindow>
#include <utility>
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
  , m_csdEnabled(m_settings.value("Window/CSDEnabled", true).toBool())
{
  static auto& themeManager = Misc::ThemeManager::instance();
  connect(&themeManager, &Misc::ThemeManager::themeChanged,
          this, &NativeWindow::onThemeChanged);
}

/**
 * @brief macOS uses native decorations, so the custom CSD chrome is never used.
 */
bool NativeWindow::csdAvailable() const
{
  return false;
}

/**
 * @brief Returns whether the custom CSD decorations are enabled.
 */
bool NativeWindow::csdEnabled() const
{
  return m_csdEnabled;
}

/**
 * @brief Persists the CSD decoration preference (inert on macOS).
 */
void NativeWindow::setCsdEnabled(bool enabled)
{
  if (m_csdEnabled == enabled)
    return;

  m_csdEnabled = enabled;
  m_settings.setValue("Window/CSDEnabled", m_csdEnabled);
  Q_EMIT csdEnabledChanged();
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
 * @brief Retrieves the height of the title bar; must never force platform-window creation,
 * since materializing a hidden dialog centers it over its still-unpolished transient parent
 * and Cocoa then warns about an off-screen position.
 */
int NativeWindow::titlebarHeight(QObject *window)
{
  QWindow *win = qobject_cast<QWindow *>(window);
  if (!win)
    return 0;

  if (!win->handle())
    return 32;

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
 * @brief No-op on macOS; the titlebar height is reported via titlebarHeight().
 */
int NativeWindow::frameTopInset(QObject *window)
{
  Q_UNUSED(window)
  return 0;
}

static QHash<QWindow *, id> s_fullScreenObservers;

/**
 * @brief Drops the fullscreen-exit observer registered for a window, if any.
 */
static void removeFullScreenObserver(QWindow *win)
{
  id token = s_fullScreenObservers.take(win);
  if (token)
    [[NSNotificationCenter defaultCenter] removeObserver:token];
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
    removeFullScreenObserver(win);
    m_windows.removeAt(index);
    if (m_colors.contains(win))
      m_colors.remove(win);
  }
}

/**
 * @brief Pins the NSWindow appearance to the caption the window paints: the addWindow() color
 * when given (dialogs fill the titlebar with the window color), else titlebar text vs toolbar.
 * Qt's app-wide scheme follows the content palette, and a light appearance over a dark
 * titlebar renders the stoplight buttons flat.
 */
static void applyMacOSWindowAppearance(NSWindow *w, const QString &requested)
{
  if (!w)
    return;

  const QColor requestedColor(requested);
  if (requestedColor.isValid())
  {
    const bool dark = requestedColor.lightness() < 128;
    [w setAppearance:[NSAppearance appearanceNamed:(dark ? NSAppearanceNameDarkAqua
                                                         : NSAppearanceNameAqua)]];
    return;
  }

  static auto& themeManager = Misc::ThemeManager::instance();
  const auto& colors = themeManager.colors();
  if (!colors.contains(QStringLiteral("toolbar_top"))
      || !colors.contains(QStringLiteral("titlebar_text")))
  {
    [w setAppearance:nil];
    return;
  }

  const auto bg = themeManager.getColor(QStringLiteral("toolbar_top"));
  const auto fg = themeManager.getColor(QStringLiteral("titlebar_text"));
  const bool dark = fg.lightness() > bg.lightness();
  [w setAppearance:[NSAppearance appearanceNamed:(dark ? NSAppearanceNameDarkAqua
                                                       : NSAppearanceNameAqua)]];
}

/**
 * @brief Applies the integrated-titlebar style to a raw NSWindow.
 */
static void applyMacOSWindowStyleToNSWindow(NSWindow *w, const QString &color)
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
  applyMacOSWindowAppearance(w, color);
}

/**
 * @brief Applies native window customization for macOS.
 */
static void applyMacOSWindowStyle(QWindow *win, const QString &color)
{
  if (!win)
    return;

  NSView *view = reinterpret_cast<NSView *>(win->winId());
  if (!view)
    return;

  applyMacOSWindowStyleToNSWindow([view window], color);
}

/**
 * @brief Initializes the native window customization for macOS; for a window that is already
 * tracked only the appearance is refreshed, because re-running setStyleMask resets the
 * NSWindow first responder, dropping keyboard focus from the active text field.
 */
void NativeWindow::addWindow(QObject *window, const QString &color)
{
  QWindow *win = qobject_cast<QWindow *>(window);
  if (!win)
    return;

  if (!win->handle())
    win->create();

  m_colors.insert(win, color);
  if (m_windows.contains(win))
  {
    NSView *view = reinterpret_cast<NSView *>(win->winId());
    applyMacOSWindowAppearance(view ? [view window] : nil, color);
    return;
  }

  applyMacOSWindowStyle(win, color);
  m_windows.append(win);
  connect(win, &QWindow::windowStateChanged, this,
          &NativeWindow::onWindowStateChanged);
  connect(win, &QObject::destroyed, this, [this](QObject *obj) {
    auto *w = static_cast<QWindow *>(obj);
    removeFullScreenObserver(w);
    m_windows.removeAll(w);
    m_colors.remove(w);
  });

  NSView *view = reinterpret_cast<NSView *>(win->winId());
  NSWindow *nsWin = view ? [view window] : nil;
  if (nsWin)
  {
    id token = [[NSNotificationCenter defaultCenter]
        addObserverForName:NSWindowDidExitFullScreenNotification
                    object:nsWin
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification *note) {
                  applyMacOSWindowStyleToNSWindow((NSWindow *)note.object,
                                                  m_colors.value(win));
                }];
    s_fullScreenObservers.insert(win, token);
  }
}

/**
 * @brief Re-pins the appearance of every tracked window; never re-runs the style mask here
 * because that resets the first responder.
 */
void NativeWindow::onThemeChanged()
{
  for (QWindow *win : std::as_const(m_windows))
  {
    if (!win || !win->handle())
      continue;

    NSView *view = reinterpret_cast<NSView *>(win->winId());
    applyMacOSWindowAppearance(view ? [view window] : nil, m_colors.value(win));
  }
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
    applyMacOSWindowStyle(win, m_colors.value(win));
}
