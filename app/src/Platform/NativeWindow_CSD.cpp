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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QColor>
#include <QOperatingSystemVersion>

#if defined(Q_OS_WIN)
#  include <dwmapi.h>
#  include <windows.h>

#  include <QAbstractNativeEventFilter>
#  include <QGuiApplication>
#  include <QSet>
#endif

#include "CSD.h"
#include "Misc/ThemeManager.h"
#include "NativeWindow.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Static storage for window decorators
//--------------------------------------------------------------------------------------------------

static QHash<QWindow*, CSD::Window*> s_decorators;

#if defined(Q_OS_WIN)
static QSet<QWindow*> s_shadowWindows;
#endif

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
// Windows 10 native shadow (DWM-drawn; only the Win32 frame visuals get removed)
//--------------------------------------------------------------------------------------------------

#if defined(Q_OS_WIN)

/**
 * @brief Answers WM_NCCALCSIZE for tracked CSD windows with the full window rect (inset by the
 *        system frame when maximized), so the restored WS_THICKFRAME never draws a native frame
 *        while DWM keeps rendering the drop shadow.
 */
class CsdNativeShadowFilter : public QAbstractNativeEventFilter {
public:
  /**
   * @brief Filters Windows messages for the tracked windows; consumes only WM_NCCALCSIZE.
   *        Matching resolves each window's live handle, so a destroyed or recreated platform
   *        window can never be confused with a recycled HWND value.
   */
  bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override
  {
    SS_ASSERT(message != nullptr, return false);
    SS_ASSERT(result != nullptr, return false);

    if (eventType != "windows_generic_MSG")
      return false;

    auto* msg = static_cast<MSG*>(message);
    if (msg->message != WM_NCCALCSIZE || msg->wParam != TRUE)
      return false;

    bool tracked = false;
    for (auto it = s_shadowWindows.cbegin(); it != s_shadowWindows.cend() && !tracked; ++it)
      tracked = ((*it)->handle() != nullptr && reinterpret_cast<HWND>((*it)->winId()) == msg->hwnd);

    if (!tracked)
      return false;

    if (IsZoomed(msg->hwnd)) {
      const UINT dpi          = GetDpiForWindow(msg->hwnd);
      const int pad           = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
      const int fx            = GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) + pad;
      const int fy            = GetSystemMetricsForDpi(SM_CYSIZEFRAME, dpi) + pad;
      auto* params            = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
      params->rgrc[0].left   += fx;
      params->rgrc[0].top    += fy;
      params->rgrc[0].right  -= fx;
      params->rgrc[0].bottom -= fy;
    }

    *result = 0;
    return true;
  }
};

/**
 * @brief Restores the resizable Win32 frame styles on a frameless CSD window and registers it
 *        with the NCCALCSIZE filter (installed once), so DWM draws the native drop shadow and
 *        aero-snap keeps working. winId() force-creates the platform window, which is safe
 *        because every caller runs while the window is being shown.
 */
static void enableNativeShadow(QWindow* window)
{
  if (!window)
    return;

  static CsdNativeShadowFilter s_filter;
  static bool s_filterInstalled = false;
  if (!s_filterInstalled) {
    qGuiApp->installNativeEventFilter(&s_filter);
    s_filterInstalled = true;
  }

  auto* hwnd = reinterpret_cast<HWND>(window->winId());
  if (!hwnd)
    return;

  const LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
  if (style == 0)
    return;

  s_shadowWindows.insert(window);

  (void)SetWindowLongPtr(
    hwnd, GWL_STYLE, style | WS_THICKFRAME | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

  const MARGINS margins = {0, 0, 1, 0};
  (void)DwmExtendFrameIntoClientArea(hwnd, &margins);

  (void)SetWindowPos(hwnd,
                     nullptr,
                     0,
                     0,
                     0,
                     0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

#endif

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs NativeWindow.
 */
NativeWindow::NativeWindow(QObject* parent)
  : QObject(parent), m_csdEnabled(m_settings.value("Window/CSDEnabled", true).toBool())
{
  static auto& themeManager = Misc::ThemeManager::instance();
  connect(&themeManager, &Misc::ThemeManager::themeChanged, this, &NativeWindow::onThemeChanged);
}

/**
 * @brief Returns true when the custom CSD chrome is the active decoration scheme.
 */
bool NativeWindow::csdAvailable() const
{
  return !isWindows11();
}

/**
 * @brief Returns whether the custom CSD decorations are enabled.
 */
bool NativeWindow::csdEnabled() const
{
  return m_csdEnabled;
}

/**
 * @brief Persists the CSD decoration preference; applied to windows created afterwards.
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
 * @brief Returns the CSD titlebar height reserved above the content area.
 */
int NativeWindow::frameTopInset(QObject* window)
{
  auto* w = qobject_cast<QWindow*>(window);
  if (!w)
    return 0;

  if (isWindows11() || !m_csdEnabled)
    return 0;

  if (auto* decorator = s_decorators.value(w, nullptr))
    return decorator->titleBarHeight();

  return CSD::TitleBarHeight;
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

#if defined(Q_OS_WIN)
  s_shadowWindows.remove(w);
#endif

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

  else if (m_csdEnabled) {
    auto* decorator = new CSD::Window(w, color, this);
    s_decorators.insert(w, decorator);

#if defined(Q_OS_WIN)
    enableNativeShadow(w);
#endif

    connect(w, &QObject::destroyed, this, [this, w]() {
      auto* dec = s_decorators.value(w, nullptr);
      s_decorators.remove(w);
      delete dec;

#if defined(Q_OS_WIN)
      s_shadowWindows.remove(w);
#endif

      auto index = m_windows.indexOf(w);
      if (index != -1 && index >= 0) {
        m_windows.removeAt(index);
        m_colors.remove(w);
      }
    });

    connect(w, &QWindow::activeChanged, this, &NativeWindow::onActiveChanged);
  }

  else {
    connect(w, &QObject::destroyed, this, [this, w]() {
      auto index = m_windows.indexOf(w);
      if (index != -1 && index >= 0) {
        m_windows.removeAt(index);
        m_colors.remove(w);
      }
    });
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

  static auto& themeManager = Misc::ThemeManager::instance();
  const auto& colors        = themeManager.colors();
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
