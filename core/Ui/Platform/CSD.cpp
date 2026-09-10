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

#include "CSD.h"

#include <QChildEvent>
#include <QCursor>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSvgRenderer>
#include <QTimer>
#include <QtMath>

#if defined(Q_OS_WIN)
#  include <windows.h>
#endif

#include "Core/IconRegistry.h"
#include "Core/Services.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"

namespace CSD {

//--------------------------------------------------------------------------------------------------
// Platform-specific constants
//--------------------------------------------------------------------------------------------------

// TitleBarHeight / TitleBarHeightMaximized live in CSD.h for pre-show fallback use.
#if defined(Q_OS_WIN)
constexpr int IconSize      = 16;
constexpr int IconMargin    = 8;
constexpr int ButtonSize    = 28;
constexpr int ButtonWidth   = 46;
constexpr int ResizeMargin  = 8;
constexpr int ButtonMargin  = 8;
constexpr int ButtonSpacing = 18;
#else
constexpr int IconSize      = 16;
constexpr int IconMargin    = 10;
constexpr int ButtonSize    = 28;
constexpr int ButtonWidth   = 32;
constexpr int ResizeMargin  = 8;
constexpr int ButtonMargin  = 12;
constexpr int ButtonSpacing = 0;
#endif

/**
 * @brief Returns true if @a window has identical minimum and maximum sizes.
 */
static bool isFixedSizeWindow(const QWindow* window)
{
  if (!window)
    return false;

  const auto fixedSize = window->property("fixedSize");
  if (fixedSize.isValid())
    return fixedSize.toBool();

  const auto minSize = window->minimumSize();
  const auto maxSize = window->maximumSize();
  if (!minSize.isValid() || !maxSize.isValid())
    return false;

  return minSize == maxSize;
}

//--------------------------------------------------------------------------------------------------
// Native system menu (Windows)
//--------------------------------------------------------------------------------------------------

#if defined(Q_OS_WIN)
/**
 * @brief Shows the native Win32 system menu for @a window at the cursor and posts the picked
 *        command, restoring the SSD titlebar right-click behavior that frameless windows lose.
 */
static void showNativeSystemMenu(QWindow* window)
{
  if (!window)
    return;

  auto* hwnd = reinterpret_cast<HWND>(window->winId());
  if (!hwnd)
    return;

  HMENU menu = GetSystemMenu(hwnd, FALSE);
  if (!menu)
    return;

  const auto flags       = window->flags();
  const bool custom      = flags & Qt::CustomizeWindowHint;
  const bool maximized   = IsZoomed(hwnd);
  const bool resizable   = !isFixedSizeWindow(window);
  const bool canMinimize = !custom || (flags & Qt::WindowMinimizeButtonHint);

  const UINT on  = MF_BYCOMMAND | MF_ENABLED;
  const UINT off = MF_BYCOMMAND | MF_GRAYED;
  (void)EnableMenuItem(menu, SC_RESTORE, maximized ? on : off);
  (void)EnableMenuItem(menu, SC_MOVE, maximized ? off : on);
  (void)EnableMenuItem(menu, SC_SIZE, (resizable && !maximized) ? on : off);
  (void)EnableMenuItem(menu, SC_MINIMIZE, canMinimize ? on : off);
  (void)EnableMenuItem(menu, SC_MAXIMIZE, (resizable && !maximized) ? on : off);
  (void)EnableMenuItem(menu, SC_CLOSE, on);
  (void)SetMenuDefaultItem(menu, SC_CLOSE, FALSE);

  POINT cursor{};
  if (!GetCursorPos(&cursor))
    return;

  const UINT trackFlags = TPM_RETURNCMD | TPM_RIGHTBUTTON;
  const auto command    = TrackPopupMenu(menu, trackFlags, cursor.x, cursor.y, 0, hwnd, nullptr);
  if (command != 0)
    (void)PostMessage(hwnd, WM_SYSCOMMAND, static_cast<WPARAM>(command), 0);
}
#endif

//--------------------------------------------------------------------------------------------------
// Titlebar
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Titlebar.
 */
Titlebar::Titlebar(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_title()
  , m_dragging(false)
  , m_windowActive(true)
  , m_hoveredButton(Button::None)
  , m_pressedButton(Button::None)
  , m_backgroundColor(QStringLiteral("#2d2d2d"))
  , m_theme(&Misc::ThemeManager::instance())
  , m_commonFonts(&Misc::CommonFonts::instance())
{
  setOpaquePainting(false);
  setAcceptHoverEvents(true);
  setFillColor(Qt::transparent);
  setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

  const auto& icon = QGuiApplication::windowIcon();
  if (!icon.isNull()) {
    const qreal dpr = qGuiApp->devicePixelRatio();
    m_icon          = icon.pixmap(QSize(CSD::IconSize, CSD::IconSize));
    m_icon.setDevicePixelRatio(dpr);
  }
}

/**
 * @brief Paints the title bar.
 */
void Titlebar::paint(QPainter* painter)
{
  if (!window())
    return;

  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  QRectF rect = boundingRect();
  painter->fillRect(rect, m_backgroundColor);

  if (!m_icon.isNull()) {
    const qreal y = (height() - CSD::IconSize) * 0.5;
    painter->drawPixmap(QPointF(CSD::IconMargin, y), m_icon);
  }

  if (m_windowActive)
    painter->setPen(foregroundColor());
  else
    painter->setPen(foregroundColor().darker(130));

#if defined(Q_OS_WIN)
  rect.setX(CSD::IconSize + CSD::IconMargin * 2);
  painter->setFont(m_commonFonts->uiFont());
  painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, title());
#else
  painter->setFont(m_commonFonts->boldUiFont());
  painter->drawText(rect, Qt::AlignCenter, title());
#endif

  auto& registry = Core::services().iconRegistry;

  // clang-format off
  const QString closeSvg =
    registry.iconPath(QStringLiteral("window"), QStringLiteral("close"), 32);
  const QString minimizeSvg =
    registry.iconPath(QStringLiteral("window"), QStringLiteral("minimize"), 32);
  const QString maximizeSvg =
    isMaximized() ? registry.iconPath(QStringLiteral("window"), QStringLiteral("restore"), 32)
                  : registry.iconPath(QStringLiteral("window"), QStringLiteral("maximize"), 32);
  // clang-format on

  if (shouldShowButton(Button::Close))
    drawButton(painter, Button::Close, closeSvg);

  if (shouldShowButton(Button::Minimize))
    drawButton(painter, Button::Minimize, minimizeSvg);

  if (shouldShowButton(Button::Maximize))
    drawButton(painter, Button::Maximize, maximizeSvg);
}

/**
 * @brief Returns the window title text.
 */
QString Titlebar::title() const
{
#if defined(Q_OS_WIN)
  return m_title + " - Serial Studio";
#else
  // code-verify off
  return m_title + " — Serial Studio";
  // code-verify on
#endif
}

/**
 * @brief Returns whether the parent window is currently active.
 */
bool Titlebar::windowActive() const
{
  return m_windowActive;
}

/**
 * @brief Returns the title bar background color.
 */
QColor Titlebar::backgroundColor() const
{
  return m_backgroundColor;
}

/**
 * @brief Returns the foreground color with optimal contrast to the background.
 */
QColor Titlebar::foregroundColor() const
{
  if (m_fgCacheKey == m_backgroundColor && m_fgCache.isValid())
    return m_fgCache;

  m_fgCacheKey = m_backgroundColor;

  const QColor toolbarTop = m_theme->getColor(QStringLiteral("toolbar_top"));

  if (m_backgroundColor == toolbarTop) {
    m_fgCache = m_theme->getColor(QStringLiteral("titlebar_text"));
    return m_fgCache;
  }

  // clang-format off
  constexpr qreal kInv1292 = 1.0 / 12.92;
  constexpr qreal kInv1055 = 1.0 / 1.055;
  auto linearize = [](qreal c) { return c <= 0.03928 ? c * kInv1292 : qPow((c + 0.055) * kInv1055, 2.4); };
  const qreal luminance = 0.2126 * linearize(m_backgroundColor.redF())
                          + 0.7152 * linearize(m_backgroundColor.greenF())
                          + 0.0722 * linearize(m_backgroundColor.blueF());
  // clang-format on

  m_fgCache = luminance > 0.179 ? QColor(Qt::black) : QColor(Qt::white);
  return m_fgCache;
}

/**
 * @brief Sets the title bar text.
 */
void Titlebar::setTitle(const QString& title)
{
  if (m_title != title) {
    m_title = title;
    Q_EMIT titleChanged();
    update();
  }
}

/**
 * @brief Sets the window active state.
 */
void Titlebar::setWindowActive(bool active)
{
  if (m_windowActive != active) {
    m_windowActive = active;
    Q_EMIT windowActiveChanged();
    update();
  }
}

/**
 * @brief Sets the title bar background color.
 */
void Titlebar::setBackgroundColor(const QColor& color)
{
  m_fgCacheKey = QColor();

  if (m_backgroundColor != color) {
    m_backgroundColor = color;
    Q_EMIT backgroundColorChanged();
    update();
  }
}

/**
 * @brief Returns the bounding rectangle for a window control button.
 */
QRectF Titlebar::buttonRect(Button button) const
{
  const QRectF bgRect  = buttonBackgroundRect(button);
  const qreal iconSize = CSD::ButtonSize * 0.5;

  return {
    bgRect.center().x() - iconSize / 2, bgRect.center().y() - iconSize / 2, iconSize, iconSize};
}

/**
 * @brief Returns the button (if any) at the given position.
 */
Titlebar::Button Titlebar::buttonAt(const QPointF& pos) const
{
  if (shouldShowButton(Button::Close) && buttonBackgroundRect(Button::Close).contains(pos))
    return Button::Close;

  else if (shouldShowButton(Button::Maximize)
           && buttonBackgroundRect(Button::Maximize).contains(pos))
    return Button::Maximize;

  else if (shouldShowButton(Button::Minimize)
           && buttonBackgroundRect(Button::Minimize).contains(pos))
    return Button::Minimize;

  return Button::None;
}

/**
 * @brief Returns true if a button should be shown based on window flags.
 */
bool Titlebar::shouldShowButton(Button button) const
{
  if (!window())
    return true;

  const auto flags          = window()->flags();
  const bool useCustomHints = flags & Qt::CustomizeWindowHint;

  switch (button) {
    case Button::Minimize:
      if (!useCustomHints)
        return true;

      return flags & Qt::WindowMinimizeButtonHint;

    case Button::Maximize: {
      if (isFixedSizeWindow(window()))
        return false;

      if (!useCustomHints)
        return true;

      return flags & Qt::WindowMaximizeButtonHint;
    }

    case Button::Close:
      if (!useCustomHints)
        return true;

      return flags & Qt::WindowCloseButtonHint;

    default:
      return true;
  }
}

/**
 * @brief Returns the background rectangle for drawing button hover/press states.
 */
QRectF Titlebar::buttonBackgroundRect(Button button) const
{
  const qreal h = height();

  int buttonIndex         = 0;
  const bool showClose    = shouldShowButton(Button::Close);
  const bool showMaximize = shouldShowButton(Button::Maximize);
  const bool showMinimize = shouldShowButton(Button::Minimize);

  switch (button) {
    case Button::Close:
      if (!showClose)
        return {};

      buttonIndex = 0;
      break;

    case Button::Maximize:
      if (!showMaximize)
        return {};

      buttonIndex = showClose ? 1 : 0;
      break;

    case Button::Minimize:
      if (!showMinimize)
        return {};

      buttonIndex = (showClose ? 1 : 0) + (showMaximize ? 1 : 0);
      break;

    default:
      return {};
  }

  return {width() - ButtonWidth * (buttonIndex + 1), 0, ButtonWidth, h};
}

/**
 * @brief Picks the icon tint for a control button based on hover/press/active state.
 */
QColor Titlebar::buttonIconColor(Button button, bool hovered, bool pressed) const
{
#if defined(Q_OS_WIN)
  if (button == Button::Close && (hovered || pressed))
    return Qt::white;

  if (!m_windowActive) {
    QColor c = foregroundColor();
    c.setAlpha(128);
    return c;
  }

  return foregroundColor();
#else
  Q_UNUSED(button)

  if (pressed)
    return m_theme->getColor(QStringLiteral("highlight")).darker(120);

  if (hovered)
    return m_theme->getColor(QStringLiteral("highlight"));

  if (!m_windowActive) {
    QColor c = foregroundColor();
    c.setAlpha(128);
    return c;
  }

  return foregroundColor();
#endif
}

/**
 * @brief Paints the hover/pressed background swatch behind a control button on Windows.
 */
void Titlebar::drawButtonHoverBackground(QPainter* painter,
                                         Button button,
                                         bool hovered,
                                         bool pressed)
{
#if defined(Q_OS_WIN)
  if (!hovered && !pressed)
    return;

  const auto bg          = m_backgroundColor;
  const auto fg          = foregroundColor();
  const bool isDarkTheme = fg.lightness() > bg.lightness();

  QColor bgColor;
  if (button == Button::Close)
    bgColor = pressed ? QColor(0xB4, 0x27, 0x1A) : QColor(0xC4, 0x2B, 0x1C);
  else if (isDarkTheme)
    bgColor = QColor(255, 255, 255, pressed ? 11 : 20);
  else
    bgColor = QColor(0, 0, 0, pressed ? 6 : 10);

  painter->fillRect(buttonBackgroundRect(button), bgColor);
#else
  Q_UNUSED(painter)
  Q_UNUSED(button)
  Q_UNUSED(hovered)
  Q_UNUSED(pressed)
#endif
}

/**
 * @brief Renders an SVG at native DPI and source-in composites it with the requested tint.
 */
QPixmap Titlebar::renderColorizedSvg(const QString& svgPath,
                                     const QSize& pixelSize,
                                     const QRectF& logicalRect,
                                     qreal dpr,
                                     const QColor& iconColor) const
{
  QSvgRenderer renderer(svgPath);
  if (!renderer.isValid())
    return QPixmap();

  QPixmap pixmap(pixelSize);
  pixmap.setDevicePixelRatio(dpr);
  pixmap.fill(Qt::transparent);

  QPainter svgPainter(&pixmap);
  svgPainter.setRenderHint(QPainter::Antialiasing);
  svgPainter.setRenderHint(QPainter::SmoothPixmapTransform);
  renderer.render(&svgPainter, logicalRect);
  svgPainter.end();

  QPixmap colorized(pixelSize);
  colorized.setDevicePixelRatio(dpr);
  colorized.fill(Qt::transparent);

  QPainter colorPainter(&colorized);
  colorPainter.drawPixmap(0, 0, pixmap);
  colorPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  colorPainter.fillRect(logicalRect, iconColor);
  colorPainter.end();

  return colorized;
}

/**
 * @brief Draws a window control button with an SVG icon.
 */
void Titlebar::drawButton(QPainter* painter, Button button, const QString& svgPath)
{
  const QRectF iconRect = buttonRect(button);
  const bool hovered    = (m_hoveredButton == button);
  const bool pressed    = (m_pressedButton == button);

  drawButtonHoverBackground(painter, button, hovered, pressed);
  const QColor iconColor = buttonIconColor(button, hovered, pressed);

  const qreal dpr = qApp->devicePixelRatio();
  const QSize pixelSize(qRound(iconRect.width() * dpr), qRound(iconRect.height() * dpr));
  const QRectF logicalRect(0, 0, iconRect.width(), iconRect.height());

  if (pixelSize.isEmpty())
    return;

  const QString cacheKey = QStringLiteral("%1|%2|%3x%4|%5")
                             .arg(svgPath)
                             .arg(QString::number(iconColor.rgba(), 16))
                             .arg(pixelSize.width())
                             .arg(pixelSize.height())
                             .arg(QString::number(dpr, 'f', 3));
  auto cacheIt = m_iconCache.constFind(cacheKey);
  if (cacheIt != m_iconCache.cend()) {
    painter->drawPixmap(iconRect.topLeft(), cacheIt.value());
    return;
  }

  const QPixmap colorized = renderColorizedSvg(svgPath, pixelSize, logicalRect, dpr, iconColor);
  if (colorized.isNull())
    return;

  m_iconCache.insert(cacheKey, colorized);
  painter->drawPixmap(iconRect.topLeft(), colorized);
}

/**
 * @brief Handles loss of mouse grab.
 */
void Titlebar::mouseUngrabEvent()
{
  m_hoveredButton = Button::None;
  m_pressedButton = Button::None;
  m_dragging      = false;
  update();
}

/**
 * @brief Handles mouse press events.
 */
void Titlebar::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton) {
    event->accept();
    return;
  }

  m_pressedButton = buttonAt(event->position());

  if (m_pressedButton != Button::None) {
    update();
    event->accept();
    return;
  }

  m_dragging = true;
  event->accept();
}

/**
 * @brief Handles mouse release events; a right-click outside the control buttons requests the
 *        native system menu.
 */
void Titlebar::mouseReleaseEvent(QMouseEvent* event)
{
  const Button releasedOn = buttonAt(event->position());

  if (event->button() == Qt::RightButton) {
    if (releasedOn == Button::None)
      Q_EMIT systemMenuRequested();

    event->accept();
    return;
  }

  if (m_pressedButton != Button::None && m_pressedButton == releasedOn) {
    switch (m_pressedButton) {
      case Button::Minimize:
        Q_EMIT minimizeClicked();
        break;
      case Button::Maximize:
        Q_EMIT maximizeClicked();
        break;
      case Button::Close:
        Q_EMIT closeClicked();
        break;
      default:
        break;
    }
  }

  m_pressedButton = Button::None;
  m_dragging      = false;
  m_hoveredButton = releasedOn;
  update();
  event->accept();
}

/**
 * @brief Handles mouse move events for window dragging.
 */
void Titlebar::mouseMoveEvent(QMouseEvent* event)
{
  if (!m_dragging || !window()) {
    event->accept();
    return;
  }

  auto* win  = window();
  m_dragging = false;

  if (isMaximized()) {
    const qreal relativeX   = event->position().x() / width();
    const QPointF globalPos = event->globalPosition();

    win->showNormal();
    win->setPosition(qRound(globalPos.x() - win->width() * relativeX),
                     qRound(globalPos.y() - height() / 2));
  }

  win->startSystemMove();
  event->accept();
}

/**
 * @brief Toggles the window maximized state on double-click.
 */
void Titlebar::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton) {
    event->accept();
    return;
  }

  if (buttonAt(event->position()) == Button::None && window()) {
    m_dragging      = false;
    m_pressedButton = Button::None;
    m_hoveredButton = Button::None;

    if (shouldShowButton(Button::Maximize)) {
      if (isMaximized())
        window()->showNormal();
      else
        window()->showMaximized();

      update();
    }
  }

  event->accept();
}

/**
 * @brief Handles hover move events to update button hover states.
 */
void Titlebar::hoverMoveEvent(QHoverEvent* event)
{
  const Button newHovered = buttonAt(event->position());
  if (newHovered != m_hoveredButton) {
    m_hoveredButton = newHovered;
    update();
  }
}

/**
 * @brief Handles hover leave events to clear button hover states.
 */
void Titlebar::hoverLeaveEvent(QHoverEvent* event)
{
  Q_UNUSED(event)
  if (m_hoveredButton != Button::None) {
    m_hoveredButton = Button::None;
    update();
  }
}

//--------------------------------------------------------------------------------------------------
// Window
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Window decorator.
 */
Window::Window(QWindow* window, const QString& color, QObject* parent)
  : QObject(parent)
  , m_resizing(false)
  , m_border(nullptr)
  , m_color(color)
  , m_titleBar(nullptr)
  , m_resizeEdge(ResizeEdge::None)
  , m_minSize(0, 0)
  , m_window(window)
  , m_contentContainer(nullptr)
  , m_lastCursor(Qt::ArrowCursor)
  , m_theme(&Misc::ThemeManager::instance())
{
  if (!m_window)
    return;

  m_window->setFlags(m_window->flags() | Qt::FramelessWindowHint);
  m_window->installEventFilter(this);

  setupContentContainer();
  setupTitleBar();
  setupBorder();

  connect(m_window, &QWindow::windowStateChanged, this, [this]() {
    if (!m_window)
      return;

    const auto state      = m_window->windowStates();
    const bool fillScreen = state & (Qt::WindowMaximized | Qt::WindowFullScreen);

    if (m_border)
      m_border->setVisible(!fillScreen);

    updateBorderGeometry();
    updateContentContainerGeometry();
    updateTitleBarGeometry();
    updateMinimumSize();

    if (m_titleBar)
      m_titleBar->update();
  });

  connect(m_window, &QWindow::minimumWidthChanged, this, &Window::onMinimumSizeChanged);
  connect(m_window, &QWindow::minimumHeightChanged, this, &Window::onMinimumSizeChanged);

  connect(m_theme, &Misc::ThemeManager::themeChanged, this, &Window::updateTheme);
  updateTheme();
  updateMinimumSize();
}

/**
 * @brief Destructor.
 */
Window::~Window()
{
  QQuickWindow* quickWindow = nullptr;
  QQuickItem* root          = nullptr;

  if (m_window) {
    m_window->removeEventFilter(this);
    quickWindow = qobject_cast<QQuickWindow*>(m_window.data());
    if (quickWindow && quickWindow->contentItem()) {
      root = quickWindow->contentItem();
      root->removeEventFilter(this);
    }
  }

  if (root && m_contentContainer) {
    const auto children = m_contentContainer->childItems();
    for (QQuickItem* child : children) {
      if (!child)
        continue;

      child->setParentItem(root);
      child->setPosition(QPointF(0, 0));
      child->setSize(root->size());
    }
  }

  if (root) {
    if (m_contentContainer)
      m_contentContainer->deleteLater();

    if (m_titleBar)
      m_titleBar->deleteLater();

    if (m_border)
      m_border->deleteLater();
  }
}

/**
 * @brief Returns the decorated window.
 */
QWindow* Window::window() const
{
  return m_window;
}

/**
 * @brief Returns the title bar component.
 */
Titlebar* Window::titleBar() const
{
  return m_titleBar;
}

/**
 * @brief Returns the current title bar height in pixels.
 */
int Window::titleBarHeight() const
{
  if (!m_window)
    return CSD::TitleBarHeight;

  const auto state = m_window->windowStates();
  if (state & Qt::WindowFullScreen)
    return 0;

  if (state & Qt::WindowMaximized)
    return CSD::TitleBarHeightMaximized;

  return CSD::TitleBarHeight;
}

/**
 * @brief Sets a custom color for the title bar.
 */
void Window::setColor(const QString& color)
{
  if (m_color == color)
    return;

  m_color = color;
  updateTheme();
}

/**
 * @brief Creates the 1px window border as four scene-graph edges (no painter, no FBO).
 */
void Window::setupBorder()
{
  auto* quickWindow = qobject_cast<QQuickWindow*>(m_window.data());
  if (!quickWindow)
    return;

  QQmlEngine* engine = qmlEngine(quickWindow);
  if (!engine)
    return;

  static const char* qmlCode = R"(
    import QtQuick

    Item {
      id: border
      readonly property color stroke: "#73666666"
      Rectangle { color: border.stroke; height: 1; anchors { left: parent.left; right: parent.right; top: parent.top } }
      Rectangle { color: border.stroke; height: 1; anchors { left: parent.left; right: parent.right; bottom: parent.bottom } }
      Rectangle { color: border.stroke; width: 1; anchors { top: parent.top; bottom: parent.bottom; left: parent.left } }
      Rectangle { color: border.stroke; width: 1; anchors { top: parent.top; bottom: parent.bottom; right: parent.right } }
    }
  )";

  QQmlComponent component(engine);
  component.setData(qmlCode, QUrl());
  if (component.isError()) {
    for (const auto& error : component.errors())
      qWarning() << "CSD border QML error:" << error.toString();

    return;
  }

  m_border = qobject_cast<QQuickItem*>(component.create());
  if (!m_border)
    return;

  m_border->setParentItem(quickWindow->contentItem());
  m_border->setZ(1000000);

  connect(quickWindow, &QQuickWindow::widthChanged, this, &Window::updateBorderGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this, &Window::updateBorderGeometry);

  updateBorderGeometry();
}

/**
 * @brief Creates and configures the title bar.
 */
void Window::setupTitleBar()
{
  auto* quickWindow = qobject_cast<QQuickWindow*>(m_window.data());
  if (!quickWindow)
    return;

  m_titleBar = new Titlebar(quickWindow->contentItem());
  m_titleBar->setZ(999999);

  connect(m_titleBar, &Titlebar::closeClicked, this, [this]() {
    if (m_window)
      m_window->close();
  });
  connect(m_titleBar, &Titlebar::minimizeClicked, this, [this]() {
    if (m_window)
      m_window->showMinimized();
  });
  connect(m_titleBar, &Titlebar::maximizeClicked, this, [this]() {
    if (!m_window)
      return;

    if (m_window->windowStates() & Qt::WindowMaximized)
      m_window->showNormal();
    else
      m_window->showMaximized();
  });

#if defined(Q_OS_WIN)
  connect(m_titleBar, &Titlebar::systemMenuRequested, this, [this]() {
    if (m_window)
      showNativeSystemMenu(m_window.data());
  });
#endif

  connect(m_window, &QWindow::activeChanged, this, [this]() {
    if (m_window && m_titleBar)
      m_titleBar->setWindowActive(m_window->isActive());
  });

  m_titleBar->setTitle(m_window->title());
  connect(m_window, &QWindow::windowTitleChanged, m_titleBar, &Titlebar::setTitle);
  connect(quickWindow, &QQuickWindow::widthChanged, this, &Window::updateTitleBarGeometry);
  updateTitleBarGeometry();
}

/**
 * @brief Updates the window minimum size to account for CSD decorations.
 */
void Window::updateMinimumSize()
{
  if (!m_window)
    return;

  m_minSize                  = preferredSize();
  const int tbHeight         = titleBarHeight();
  const int minWidth         = qMax(0, m_minSize.width());
  const int minHeight        = qMax(0, m_minSize.height()) + tbHeight;
  const int minTitleBarWidth = 3 * (CSD::ButtonSize + CSD::ButtonSpacing) + 2 * CSD::ButtonMargin
                             + CSD::IconSize + 2 * CSD::IconMargin;

  m_window->setMinimumSize(QSize(qMax(minWidth, minTitleBarWidth), qMax(minHeight, tbHeight)));
}

/**
 * @brief Updates the border position and size.
 */
void Window::updateBorderGeometry()
{
  if (!m_border || !m_window)
    return;

  m_border->setPosition(QPointF(0, 0));
  m_border->setSize(QSizeF(m_window->width(), m_window->height()));
}

/**
 * @brief Recalculates minimum window size when QML minimums change.
 */
void Window::onMinimumSizeChanged()
{
  const auto size = preferredSize();
  const int expW  = m_minSize.width();
  const int expH  = m_minSize.height() + titleBarHeight();

  bool changed = false;
  if (size.width() != expW) {
    changed = true;
    m_minSize.setWidth(size.width());
  }

  if (size.height() != expH) {
    changed = true;
    m_minSize.setHeight(size.height() - titleBarHeight());
  }

  if (changed)
    updateMinimumSize();
}

/**
 * @brief Creates the opaque content-background container holding the reparented QML content.
 */
void Window::setupContentContainer()
{
  auto* quickWindow = qobject_cast<QQuickWindow*>(m_window.data());
  if (!quickWindow)
    return;

  QQuickItem* root   = quickWindow->contentItem();
  QQmlEngine* engine = qmlEngine(quickWindow);
  if (!engine)
    return;

  static const char* qmlCode = R"(
    import QtQuick

    Rectangle {
      id: contentContainer
      color: "transparent"
    }
  )";

  QQmlComponent component(engine);
  component.setData(qmlCode, QUrl());

  if (component.isError()) {
    for (const auto& error : component.errors())
      qWarning() << "CSD QML error:" << error.toString();

    return;
  }

  m_contentContainer = qobject_cast<QQuickItem*>(component.create());
  if (!m_contentContainer)
    return;

  m_contentContainer->setParentItem(root);
  m_contentContainer->setZ(0);
  root->installEventFilter(this);

  connect(quickWindow, &QQuickWindow::widthChanged, this, &Window::updateContentContainerGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this, &Window::updateContentContainerGeometry);

  updateContentContainerGeometry();

  const auto children = root->childItems();
  for (QQuickItem* child : children)
    reparentChildToContainer(child);
}

/**
 * @brief Updates the title bar position and size.
 */
void Window::updateTitleBarGeometry()
{
  if (!m_titleBar || !m_window)
    return;

  m_titleBar->setPosition(QPointF(0, 0));
  m_titleBar->setSize(QSizeF(m_window->width(), titleBarHeight()));
}

/**
 * @brief Updates the content container position and size.
 */
void Window::updateContentContainerGeometry()
{
  if (!m_contentContainer || !m_window)
    return;

  const int tbHeight = titleBarHeight();
  const qreal w      = m_window->width();
  const qreal h      = m_window->height() - tbHeight;

  m_contentContainer->setPosition(QPointF(0, tbHeight));
  m_contentContainer->setSize(QSizeF(w, h));

  const auto children = m_contentContainer->childItems();
  for (QQuickItem* child : children) {
    child->setPosition(QPointF(0, 0));
    child->setSize(QSizeF(w, h));
  }
}

/**
 * @brief Returns the preferred size of the window.
 */
QSize Window::preferredSize() const
{
  if (!m_window)
    return QSize(0, 0);

  auto preferredSize = m_window->minimumSize();
  auto* quickWindow  = qobject_cast<QQuickWindow*>(m_window.data());
  if (!quickWindow)
    return preferredSize;

  auto pWidth  = quickWindow->property("preferredWidth");
  auto pHeight = quickWindow->property("preferredHeight");
  if (!pWidth.isNull() && pWidth.isValid())
    preferredSize.setWidth(pWidth.toInt());

  if (!pHeight.isNull() && pHeight.isValid())
    preferredSize.setHeight(pHeight.toInt());

  return preferredSize;
}

/**
 * @brief Updates theme colors for the title bar.
 */
void Window::updateTheme()
{
  if (m_titleBar) {
    const QString color =
      m_color.isEmpty() ? m_theme->getColor(QStringLiteral("toolbar_top")).name() : m_color;
    m_titleBar->setBackgroundColor(QColor(color));
  }

  if (m_contentContainer)
    m_contentContainer->setProperty("color", m_theme->getColor(QStringLiteral("toolbar_top")));
}

/**
 * @brief Reparents a child item to the content container.
 */
void Window::reparentChildToContainer(QQuickItem* child)
{
  if (!child || !m_contentContainer)
    return;

  if (child == m_border || child == m_titleBar || child == m_contentContainer)
    return;

  if (child->parentItem() == m_contentContainer)
    return;

  child->setParentItem(m_contentContainer);
  child->setPosition(QPointF(0, 0));
  child->setSize(m_contentContainer->size());
}

/**
 * @brief Returns the resize edge flags at the given position.
 */
Window::ResizeEdge Window::edgeAt(const QPointF& pos) const
{
  if (!m_window || (m_window->windowStates() & Qt::WindowMaximized))
    return ResizeEdge::None;

  if (isFixedSizeWindow(m_window))
    return ResizeEdge::None;

  const int w    = m_window->width();
  const int h    = m_window->height();
  const int area = CSD::ResizeMargin;

  int edge = static_cast<int>(ResizeEdge::None);

  if (pos.x() < area)
    edge |= static_cast<int>(ResizeEdge::Left);
  else if (pos.x() >= w - area)
    edge |= static_cast<int>(ResizeEdge::Right);

  if (pos.y() < area)
    edge |= static_cast<int>(ResizeEdge::Top);
  else if (pos.y() >= h - area)
    edge |= static_cast<int>(ResizeEdge::Bottom);

  return static_cast<ResizeEdge>(edge);
}

/**
 * @brief Returns the cursor shape for a resize edge.
 */
Qt::CursorShape Window::cursorForEdge(ResizeEdge edge) const
{
  switch (edge) {
    case ResizeEdge::Left:
    case ResizeEdge::Right:
      return Qt::SizeHorCursor;
    case ResizeEdge::Top:
    case ResizeEdge::Bottom:
      return Qt::SizeVerCursor;
    case ResizeEdge::TopLeft:
    case ResizeEdge::BottomRight:
      return Qt::SizeFDiagCursor;
    case ResizeEdge::TopRight:
    case ResizeEdge::BottomLeft:
      return Qt::SizeBDiagCursor;
    default:
      return Qt::ArrowCursor;
  }
}

/**
 * @brief Converts internal ResizeEdge to Qt::Edges flags.
 */
Qt::Edges Window::qtEdgesFromResizeEdge(ResizeEdge edge) const
{
  Qt::Edges edges;
  const int e = static_cast<int>(edge);

  if (e & static_cast<int>(ResizeEdge::Left))
    edges |= Qt::LeftEdge;

  if (e & static_cast<int>(ResizeEdge::Right))
    edges |= Qt::RightEdge;

  if (e & static_cast<int>(ResizeEdge::Top))
    edges |= Qt::TopEdge;

  if (e & static_cast<int>(ResizeEdge::Bottom))
    edges |= Qt::BottomEdge;

  return edges;
}

/**
 * @brief Releases the CSD resize-cursor override so widget/QML cursors regain control.
 */
void Window::releaseResizeCursor()
{
  if (m_lastCursor == Qt::ArrowCursor)
    return;

  m_lastCursor = Qt::ArrowCursor;
  if (m_window)
    m_window->unsetCursor();
}

/**
 * @brief Applies a resize cursor only inside the resize band; elsewhere hands cursor control back.
 */
void Window::updateResizeCursor(const QPointF& pos)
{
  const ResizeEdge edge = edgeAt(pos);
  if (edge == ResizeEdge::None) {
    releaseResizeCursor();
    return;
  }

  const Qt::CursorShape shape = cursorForEdge(edge);
  if (shape == m_lastCursor)
    return;

  m_lastCursor = shape;
  if (m_window)
    m_window->setCursor(shape);
}

/**
 * @brief Event filter for window resize and content reparenting.
 */
bool Window::eventFilter(QObject* watched, QEvent* event)
{
  if (auto* quickWindow = qobject_cast<QQuickWindow*>(m_window.data())) {
    if (watched == quickWindow->contentItem() && event->type() == QEvent::ChildAdded) {
      auto* childEvent = static_cast<QChildEvent*>(event);
      if (auto* child = qobject_cast<QQuickItem*>(childEvent->child())) {
        QPointer<QQuickItem> guardedChild(child);
        QTimer::singleShot(0, this, [this, guardedChild]() {
          if (guardedChild)
            reparentChildToContainer(guardedChild);
        });
      }

      return false;
    }
  }

  if (watched != m_window)
    return false;

  switch (event->type()) {
    case QEvent::MouseMove: {
      auto* me = static_cast<QMouseEvent*>(event);
      if (!m_resizing)
        updateResizeCursor(me->position());

      break;
    }

    case QEvent::MouseButtonPress: {
      auto* me = static_cast<QMouseEvent*>(event);
      if (me->button() == Qt::LeftButton) {
        m_resizeEdge = edgeAt(me->position());
        if (m_resizeEdge != ResizeEdge::None) {
          m_resizing = true;
          m_window->startSystemResize(qtEdgesFromResizeEdge(m_resizeEdge));
          m_resizing = false;
          return true;
        }
      }
      break;
    }

    case QEvent::MouseButtonRelease:
      m_resizing   = false;
      m_resizeEdge = ResizeEdge::None;
      break;

    case QEvent::Leave:
      if (!m_resizing)
        releaseResizeCursor();

      break;

    default:
      break;
  }

  return false;
}
}  // namespace CSD
