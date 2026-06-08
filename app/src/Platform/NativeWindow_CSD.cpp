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
#include <QOperatingSystemVersion>

#if defined(Q_OS_WIN)
#  include <dwmapi.h>
#endif

#include "CSD.h"
#include "Misc/ThemeManager.h"
#include "NativeWindow.h"

//--------------------------------------------------------------------------------------------------
// Static storage for window decorators
//--------------------------------------------------------------------------------------------------

static QHash<QWindow*, CSD::Window*> s_decorators;

//--------------------------------------------------------------------------------------------------
// Helper function to detect Windows 11
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the running OS reports Windows 11 or later.
 */
static bool isWindows11()
{
#if defined(Q_OS_WIN)
  static const auto current = QOperatingSystemVersion::current();
  return current >= QOperatingSystemVersion::Windows11;
#else
  return false;
#endif
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs NativeWindow.
 */
NativeWindow::NativeWindow(QObject* parent) : QObject(parent)
{
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &NativeWindow::onThemeChanged);
}

/**
 * @brief No-op on non-macOS platforms.
 */
void NativeWindow::installMacOSQuitInterceptor() {}

//--------------------------------------------------------------------------------------------------
// Window management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the height of the title bar in pixels.
 */
int NativeWindow::titlebarHeight(QObject* window)
{
  (void)window;
  return 0;
}

/**
 * @brief Returns the horizontal/bottom shadow margin reserved by CSD around the content area.
 */
int NativeWindow::frameMargin(QObject* window)
{
  auto* w = qobject_cast<QWindow*>(window);
  if (!w)
    return 0;

  if (isWindows11())
    return 0;

  if (auto* decorator = s_decorators.value(w, nullptr))
    return decorator->shadowMargin();

  return CSD::ShadowRadius;
}

/**
 * @brief Returns the total top inset (shadow + CSD titlebar) reserved above the content area.
 */
int NativeWindow::frameTopInset(QObject* window)
{
  auto* w = qobject_cast<QWindow*>(window);
  if (!w)
    return 0;

  if (isWindows11())
    return 0;

  if (auto* decorator = s_decorators.value(w, nullptr))
    return decorator->shadowMargin() + decorator->titleBarHeight();

  return CSD::ShadowRadius + CSD::TitleBarHeight;
}

/**
 * @brief Removes a window from the management list.
 */
void NativeWindow::removeWindow(QObject* window)
{
  auto* w = qobject_cast<QWindow*>(window);
  if (!w)
    return;

  auto index = m_windows.indexOf(w);
  if (index == -1)
    return;

  m_windows.removeAt(index);
  m_colors.remove(w);

  disconnect(w, nullptr, this, nullptr);

  auto* decorator = s_decorators.value(w, nullptr);
  if (decorator) {
    s_decorators.remove(w);
    delete decorator;
  }
}

/**
 * @brief Configures native window customization.
 */
void NativeWindow::addWindow(QObject* window, const QString& color)
{
  auto* w = qobject_cast<QWindow*>(window);
  if (!w)
    return;

  if (m_windows.contains(w)) {
    m_colors.insert(w, color);

    if (isWindows11())
      Q_EMIT w->activeChanged();

    else {
      auto* decorator = s_decorators.value(w, nullptr);
      if (decorator)
        decorator->setColor(color);
    }

    return;
  }

  m_windows.append(w);
  m_colors.insert(w, color);

  if (isWindows11()) {
    connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);
    connect(w, &QObject::destroyed, this, [this, w]() {
      auto index = m_windows.indexOf(w);
      if (index != -1 && index >= 0) {
        m_windows.removeAt(index);
        m_colors.remove(w);
      }
    });
    Q_EMIT w->activeChanged();
  }

  else {
    auto* decorator = new CSD::Window(w, color, this);
    s_decorators.insert(w, decorator);
    connect(w, &QObject::destroyed, this, [this, w]() {
      auto* dec = s_decorators.value(w, nullptr);
      s_decorators.remove(w);
      delete dec;

      auto index = m_windows.indexOf(w);
      if (index != -1 && index >= 0) {
        m_windows.removeAt(index);
        m_colors.remove(w);
      }
    });

    connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);
  }
}

//--------------------------------------------------------------------------------------------------
// Theme management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles theme change events.
 */
void NativeWindow::onThemeChanged()
{
  if (isWindows11()) {
    for (auto* window : std::as_const(m_windows))
      Q_EMIT window->activeChanged();
  }

  else {
    for (auto* window : std::as_const(m_windows)) {
      auto* decorator = s_decorators.value(window, nullptr);
      if (decorator)
        decorator->updateTheme();
    }
  }
}

/**
 * @brief Handles window state changes.
 */
void NativeWindow::onWindowStateChanged(Qt::WindowState state)
{
  (void)state;
}

/**
 * @brief Handles the active state change of a window.
 */
void NativeWindow::onActiveChanged()
{
#if defined(Q_OS_WIN)
  if (!isWindows11())
    return;

  auto* window = qobject_cast<QWindow*>(sender());
  if (!window || !m_windows.contains(window))
    return;

  const auto& colors = Misc::ThemeManager::instance().colors();
  QString colorName;

  if (m_colors.contains(window) && !m_colors[window].isEmpty())
    colorName = m_colors[window];

  else if (window->isActive())
    colorName = colors.value("toolbar_top").toString();
  else
    colorName = colors.value("toolbar_bottom").toString();

  const QColor color(colorName);
  const COLORREF colorref = color.red() | (color.green() << 8) | (color.blue() << 16);

  const DWORD attribute = 35;
  DwmSetWindowAttribute((HWND)window->winId(), attribute, &colorref, sizeof(colorref));
#else
  Q_UNUSED(this);
#endif
}
