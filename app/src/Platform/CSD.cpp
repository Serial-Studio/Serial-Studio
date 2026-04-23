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
#include <QSurfaceFormat>
#include <QSvgRenderer>
#include <QTimer>
#include <QtMath>

#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"

namespace CSD {

//--------------------------------------------------------------------------------------------------
// Platform-specific constants
//--------------------------------------------------------------------------------------------------

#if defined(Q_OS_WIN)
constexpr int IconSize                = 16;
constexpr int IconMargin              = 8;
constexpr int ButtonSize              = 28;
constexpr int ButtonWidth             = 46;
constexpr int ResizeMargin            = 8;
constexpr int ButtonMargin            = 8;
constexpr int ShadowRadius            = 24;
constexpr int ButtonSpacing           = 18;
constexpr int TitleBarHeight          = 32;
constexpr int TitleBarHeightMaximized = 28;
constexpr qreal ShadowOpacity         = 0.10;
#else
constexpr int IconSize                = 16;
constexpr int IconMargin              = 10;
constexpr int ButtonSize              = 28;
constexpr int ButtonWidth             = 32;
constexpr int ResizeMargin            = 8;
constexpr int ButtonMargin            = 12;
constexpr int ShadowRadius            = 24;
constexpr int ButtonSpacing           = 0;
constexpr int TitleBarHeight          = 32;
constexpr int TitleBarHeightMaximized = 28;
constexpr qreal ShadowOpacity         = 0.10;
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
// Titlebar
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Titlebar.
 * @param parent Parent QQuickItem.
 */
Titlebar::Titlebar(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_title()
  , m_dragging(false)
  , m_windowActive(true)
  , m_hoveredButton(Button::None)
  , m_pressedButton(Button::None)
  , m_backgroundColor(QStringLiteral("#2d2d2d"))
{
  // Enable mouse and hover event handling
  setOpaquePainting(false);
  setAcceptHoverEvents(true);
  setFillColor(Qt::transparent);
  setAcceptedMouseButtons(Qt::LeftButton);

  // Load the application icon at the current DPI scale
  const auto& icon = QGuiApplication::windowIcon();
  if (!icon.isNull()) {
    const qreal dpr = qGuiApp->devicePixelRatio();
    m_icon          = icon.pixmap(QSize(CSD::IconSize, CSD::IconSize));
    m_icon.setDevicePixelRatio(dpr);
  }
}

/**
 * @brief Paints the title bar.
 * @param painter QPainter used for rendering.
 */
void Titlebar::paint(QPainter* painter)
{
  if (!window())
    return;

  // Fill background
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  QRectF rect = boundingRect();
  painter->fillRect(rect, m_backgroundColor);

  // Draw application icon
  if (!m_icon.isNull()) {
    const qreal y = (height() - CSD::IconSize) / 2.0;
    painter->drawPixmap(QPointF(CSD::IconMargin, y), m_icon);
  }

  // Dim title text when window is inactive
  if (m_windowActive)
    painter->setPen(foregroundColor());
  else
    painter->setPen(foregroundColor().darker(130));

  // Draw title text (left-aligned on Windows, centered elsewhere)
#if defined(Q_OS_WIN)
  rect.setX(CSD::IconSize + CSD::IconMargin * 2);
  painter->setFont(Misc::CommonFonts::instance().uiFont());
  painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, title());
#else
  painter->setFont(Misc::CommonFonts::instance().boldUiFont());
  painter->drawText(rect, Qt::AlignCenter, title());
#endif

  // clang-format off
  const QString closeSvg = QStringLiteral(":/rcc/icons/csd/close.svg");
  const QString minimizeSvg = QStringLiteral(":/rcc/icons/csd/minimize.svg");
  const QString maximizeSvg = isMaximized() ? QStringLiteral(":/rcc/icons/csd/restore.svg")
                                            : QStringLiteral(":/rcc/icons/csd/maximize.svg");
  // clang-format on

  // Draw visible window control buttons
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
  return m_title + " — Serial Studio";
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
  // Use theme's titlebar_text if background matches toolbar color
  const auto& theme       = Misc::ThemeManager::instance();
  const QColor toolbarTop = theme.getColor(QStringLiteral("toolbar_top"));

  if (m_backgroundColor == toolbarTop)
    return theme.getColor(QStringLiteral("titlebar_text"));

  // Fall back to luminance-based contrast (W3C formula)
  // clang-format off
  auto linearize = [](qreal c) { return c <= 0.03928 ? c / 12.92 : qPow((c + 0.055) / 1.055, 2.4); };
  const qreal luminance = 0.2126 * linearize(m_backgroundColor.redF())
                          + 0.7152 * linearize(m_backgroundColor.greenF())
                          + 0.0722 * linearize(m_backgroundColor.blueF());
  // clang-format on

  return luminance > 0.179 ? QColor(Qt::black) : QColor(Qt::white);
}

/**
 * @brief Sets the title bar text.
 * @param title New title string.
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
 * @param active New active state.
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
 * @param color New background color.
 */
void Titlebar::setBackgroundColor(const QColor& color)
{
  if (m_backgroundColor != color) {
    m_backgroundColor = color;
    Q_EMIT backgroundColorChanged();
    update();
  }
}

/**
 * @brief Returns the bounding rectangle for a window control button.
 * @param button Button type to query.
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
 * @param pos Position in item coordinates.
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
 * @param button Button type to query.
 */
bool Titlebar::shouldShowButton(Button button) const
{
  if (!window())
    return true;

  // Use custom hints only if CustomizeWindowHint is set
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

      // Check window flags
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
 * @param button Button type to query.
 */
QRectF Titlebar::buttonBackgroundRect(Button button) const
{
  // Position buttons right-to-left, skipping hidden siblings
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
 * @brief Draws a window control button with an SVG icon.
 * @param painter Painter to use.
 * @param button  Button type to draw.
 * @param svgPath Path to the SVG icon resource.
 */
void Titlebar::drawButton(QPainter* painter, Button button, const QString& svgPath)
{
  const QRectF iconRect = buttonRect(button);
  const bool hovered    = (m_hoveredButton == button);
  const bool pressed    = (m_pressedButton == button);

  // Determine icon color based on hover/press/active state
  QColor iconColor;

#if defined(Q_OS_WIN)
  const auto bg          = m_backgroundColor;
  const auto fg          = foregroundColor();
  const bool isDarkTheme = fg.lightness() > bg.lightness();

  if (hovered || pressed) {
    QColor bgColor;

    if (button == Button::Close)
      bgColor = pressed ? QColor(0xB4, 0x27, 0x1A) : QColor(0xC4, 0x2B, 0x1C);
    else if (isDarkTheme)
      bgColor = QColor(255, 255, 255, pressed ? 11 : 20);
    else
      bgColor = QColor(0, 0, 0, pressed ? 6 : 10);

    painter->fillRect(buttonBackgroundRect(button), bgColor);
  }

  if (button == Button::Close && (hovered || pressed))
    iconColor = Qt::white;

  else if (!m_windowActive) {
    iconColor = foregroundColor();
    iconColor.setAlpha(128);
  }

  else
    iconColor = foregroundColor();
#else
  const auto& theme = Misc::ThemeManager::instance();

  if (pressed)
    iconColor = theme.getColor(QStringLiteral("highlight")).darker(120);
  else if (hovered)
    iconColor = theme.getColor(QStringLiteral("highlight"));

  else if (!m_windowActive) {
    iconColor = foregroundColor();
    iconColor.setAlpha(128);
  }

  else
    iconColor = foregroundColor();
#endif

  // Render at native DPI with icon cache
  const qreal dpr = qApp->devicePixelRatio();
  const QSize pixelSize(qRound(iconRect.width() * dpr), qRound(iconRect.height() * dpr));
  const QRectF logicalRect(0, 0, iconRect.width(), iconRect.height());

  if (pixelSize.isEmpty())
    return;

  // Check the icon cache first
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

  // Render SVG and colorize it
  QSvgRenderer renderer(svgPath);
  if (!renderer.isValid())
    return;

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
 * @param event Mouse event.
 */
void Titlebar::mousePressEvent(QMouseEvent* event)
{
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
 * @brief Handles mouse release events.
 * @param event Mouse event.
 */
void Titlebar::mouseReleaseEvent(QMouseEvent* event)
{
  // Emit click signal if press and release target the same button
  const Button releasedOn = buttonAt(event->position());

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
 * @param event Mouse event.
 */
void Titlebar::mouseMoveEvent(QMouseEvent* event)
{
  if (!m_dragging || !window()) {
    event->accept();
    return;
  }

  auto* win  = window();
  m_dragging = false;

  // Restore from maximized so the window can be dragged
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
 * @param event Mouse event.
 */
void Titlebar::mouseDoubleClickEvent(QMouseEvent* event)
{
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
 * @param event Hover event.
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
 * @param event Hover event.
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
// Frame
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Frame.
 * @param parent Parent QQuickItem.
 */
Frame::Frame(QQuickItem* parent)
  : QQuickPaintedItem(parent), m_shadowRadius(CSD::ShadowRadius), m_shadowEnabled(true)
{
  setFillColor(Qt::transparent);
  setOpaquePainting(false);
  regenerateShadow();
}

/**
 * @brief Paints the window frame including shadow and background.
 * @param painter QPainter used for rendering.
 */
void Frame::paint(QPainter* painter)
{
  const int r = m_shadowEnabled ? m_shadowRadius : 0;
  const QRectF content(r, r, width() - 2 * r, height() - 2 * r);

  // Draw shadow corners and edges
  if (m_shadowEnabled && !m_shadowCorner.isNull() && r > 0) {
    painter->drawImage(0, 0, m_shadowCorner);
    painter->drawImage(QPointF(width() - r, 0), m_shadowCornerFlippedH);
    painter->drawImage(QPointF(0, height() - r), m_shadowCornerFlippedV);
    painter->drawImage(QPointF(width() - r, height() - r), m_shadowCornerFlippedHV);

    if (!m_shadowEdge.isNull()) {
      const qreal hEdgeLen = width() - 2 * r;
      const qreal vEdgeLen = height() - 2 * r;

      if (hEdgeLen > 0) {
        painter->drawImage(QRectF(r, 0, hEdgeLen, r), m_shadowEdge, QRectF(0, 0, 1, r));
        painter->drawImage(
          QRectF(r, height() - r, hEdgeLen, r), m_shadowEdgeFlipped, QRectF(0, 0, 1, r));
      }

      if (vEdgeLen > 0) {
        painter->drawImage(
          QRectF(0, r, r, vEdgeLen), m_shadowEdgeVerticalFlipped, QRectF(0, 0, r, 1));
        painter->drawImage(
          QRectF(width() - r, r, r, vEdgeLen), m_shadowEdgeVertical, QRectF(0, 0, r, 1));
      }
    }
  }

  // Fill content area with theme background
  if (content.width() > 0 && content.height() > 0) {
    const auto& theme    = Misc::ThemeManager::instance();
    const QColor bgColor = theme.getColor(QStringLiteral("toolbar_top"));

    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    painter->fillRect(content, bgColor);
  }
}

/**
 * @brief Returns the shadow blur radius in pixels.
 */
int Frame::shadowRadius() const
{
  return m_shadowRadius;
}

/**
 * @brief Returns whether the shadow is enabled.
 */
bool Frame::shadowEnabled() const
{
  return m_shadowEnabled;
}

/**
 * @brief Sets the shadow blur radius.
 * @param radius New radius in pixels.
 */
void Frame::setShadowRadius(int radius)
{
  if (m_shadowRadius != radius) {
    m_shadowRadius = radius;
    regenerateShadow();
    Q_EMIT shadowRadiusChanged();
    update();
  }
}

/**
 * @brief Enables or disables shadow rendering.
 * @param enabled True to enable shadows, false to disable.
 */
void Frame::setShadowEnabled(bool enabled)
{
  if (m_shadowEnabled != enabled) {
    m_shadowEnabled = enabled;
    Q_EMIT shadowEnabledChanged();
    update();
  }
}

/**
 * @brief Regenerates the shadow tile images.
 */
void Frame::regenerateShadow()
{
  // No shadow needed, clear all tiles
  if (m_shadowRadius <= 0) {
    m_shadowCorner              = QImage();
    m_shadowEdge                = QImage();
    m_shadowEdgeFlipped         = QImage();
    m_shadowEdgeVertical        = QImage();
    m_shadowEdgeVerticalFlipped = QImage();
    m_shadowCornerFlippedH      = QImage();
    m_shadowCornerFlippedV      = QImage();
    m_shadowCornerFlippedHV     = QImage();
    return;
  }

  // Generate corner tile and its mirror variants
  m_shadowCorner          = generateShadowCorner(m_shadowRadius);
  m_shadowCornerFlippedH  = m_shadowCorner.flipped(Qt::Horizontal);
  m_shadowCornerFlippedV  = m_shadowCorner.flipped(Qt::Vertical);
  m_shadowCornerFlippedHV = m_shadowCorner.flipped(Qt::Horizontal | Qt::Vertical);

  // Generate horizontal edge tile (1px wide, smoothstep falloff)
  m_shadowEdge = QImage(1, m_shadowRadius, QImage::Format_ARGB32_Premultiplied);
  m_shadowEdge.fill(Qt::transparent);

  for (int y = 0; y < m_shadowRadius; ++y) {
    const qreal dist = static_cast<qreal>(m_shadowRadius - y - 1) / m_shadowRadius;
    qreal alpha      = 1.0 - dist;
    alpha            = alpha * alpha * (3.0 - 2.0 * alpha);
    alpha *= CSD::ShadowOpacity;
    m_shadowEdge.setPixelColor(0, y, QColor(0, 0, 0, qRound(alpha * 255)));
  }

  m_shadowEdgeFlipped = m_shadowEdge.flipped(Qt::Vertical);

  // Generate vertical edge tile (1px tall, smoothstep falloff)
  m_shadowEdgeVertical = QImage(m_shadowRadius, 1, QImage::Format_ARGB32_Premultiplied);
  m_shadowEdgeVertical.fill(Qt::transparent);

  for (int x = 0; x < m_shadowRadius; ++x) {
    const qreal dist = static_cast<qreal>(x) / m_shadowRadius;
    qreal alpha      = 1.0 - dist;
    alpha            = alpha * alpha * (3.0 - 2.0 * alpha);
    alpha *= CSD::ShadowOpacity;
    m_shadowEdgeVertical.setPixelColor(x, 0, QColor(0, 0, 0, qRound(alpha * 255)));
  }

  m_shadowEdgeVerticalFlipped = m_shadowEdgeVertical.flipped(Qt::Horizontal);
}

/**
 * @brief Generates a shadow corner tile image.
 * @param size Shadow blur radius in pixels.
 */
QImage Frame::generateShadowCorner(int size)
{
  QImage tile(size, size, QImage::Format_ARGB32_Premultiplied);
  tile.fill(Qt::transparent);

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      const qreal dx   = static_cast<qreal>(size - x - 1);
      const qreal dy   = static_cast<qreal>(size - y - 1);
      const qreal dist = qSqrt(dx * dx + dy * dy);

      qreal alpha = 1.0 - qMin(dist / static_cast<qreal>(size), 1.0);
      alpha       = alpha * alpha * (3.0 - 2.0 * alpha);
      alpha *= CSD::ShadowOpacity;
      tile.setPixelColor(x, y, QColor(0, 0, 0, qRound(alpha * 255)));
    }
  }

  return tile;
}

//--------------------------------------------------------------------------------------------------
// Border
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Border.
 * @param parent Parent QQuickItem.
 */
Border::Border(QQuickItem* parent) : QQuickPaintedItem(parent)
{
  setFillColor(Qt::transparent);
  setOpaquePainting(false);
  setRenderTarget(QQuickPaintedItem::FramebufferObject);
  setAntialiasing(true);
}

/**
 * @brief Paints the window border.
 * @param painter QPainter used for rendering.
 */
void Border::paint(QPainter* painter)
{
  const QColor borderColor = QColor(102, 102, 102, 115);

  painter->setPen(QPen(borderColor, 1));
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(QRectF(0.5, 0.5, width() - 1, height() - 1));
}

//--------------------------------------------------------------------------------------------------
// Window
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Window decorator.
 * @param window QWindow to decorate.
 * @param color  Optional custom title bar color (hex string).
 * @param parent Parent QObject.
 */
Window::Window(QWindow* window, const QString& color, QObject* parent)
  : QObject(parent)
  , m_resizing(false)
  , m_frame(nullptr)
  , m_border(nullptr)
  , m_color(color)
  , m_titleBar(nullptr)
  , m_resizeEdge(ResizeEdge::None)
  , m_minSize(0, 0)
  , m_window(window)
  , m_contentContainer(nullptr)
{
  if (!m_window)
    return;

  // Configure frameless window and install resize event filter
  m_window->setFlags(m_window->flags() | Qt::FramelessWindowHint);
  m_window->installEventFilter(this);

  // Create CSD components in z-order
  setupFrame();
  setupContentContainer();
  setupTitleBar();
  setupBorder();

  // Update shadow and geometry on maximize/fullscreen transitions
  connect(m_window, &QWindow::windowStateChanged, this, [this]() {
    if (!m_window)
      return;

    const auto state      = m_window->windowStates();
    const bool fillScreen = state & (Qt::WindowMaximized | Qt::WindowFullScreen);

    if (m_frame)
      m_frame->setShadowEnabled(!fillScreen);

    if (m_border)
      m_border->setVisible(!fillScreen);

    updateFrameGeometry();
    updateBorderGeometry();
    updateContentContainerGeometry();
    updateTitleBarGeometry();
    updateMinimumSize();

    if (m_titleBar)
      m_titleBar->update();
  });

  // Track minimum size changes and theme updates
  connect(m_window, &QWindow::minimumWidthChanged, this, &Window::onMinimumSizeChanged);
  connect(m_window, &QWindow::minimumHeightChanged, this, &Window::onMinimumSizeChanged);

  connect(
    &Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged, this, &Window::updateTheme);
  updateTheme();
  updateMinimumSize();
}

/**
 * @brief Destructor.
 */
Window::~Window()
{
  // Remove event filters
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

  // Reparent content items back to the root before destruction
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

  // Schedule CSD component deletion
  if (root) {
    if (m_contentContainer)
      m_contentContainer->deleteLater();
    if (m_titleBar)
      m_titleBar->deleteLater();
    if (m_border)
      m_border->deleteLater();
    if (m_frame)
      m_frame->deleteLater();
  }
}

/**
 * @brief Returns the shadow frame component.
 */
Frame* Window::frame() const
{
  return m_frame;
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
 * @brief Returns the current shadow margin in pixels.
 */
int Window::shadowMargin() const
{
  if (!m_window)
    return 0;

  const auto state = m_window->windowStates();
  if (state & (Qt::WindowMaximized | Qt::WindowFullScreen))
    return 0;

  return CSD::ShadowRadius;
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
 * @param color Color as a hex string (empty to use theme default).
 */
void Window::setColor(const QString& color)
{
  if (m_color == color)
    return;

  m_color = color;
  updateTheme();
}

/**
 * @brief Creates and configures the shadow frame.
 */
void Window::setupFrame()
{
  auto* quickWindow = qobject_cast<QQuickWindow*>(m_window.data());
  if (!quickWindow)
    return;

  QSurfaceFormat format = quickWindow->format();
  if (format.alphaBufferSize() < 8) {
    format.setAlphaBufferSize(8);
    quickWindow->setFormat(format);
  }

  quickWindow->setColor(Qt::transparent);
  m_frame = new Frame(quickWindow->contentItem());
  m_frame->setZ(-1);

  connect(quickWindow, &QQuickWindow::widthChanged, this, &Window::updateFrameGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this, &Window::updateFrameGeometry);

  updateFrameGeometry();
}

/**
 * @brief Creates and configures the border overlay.
 */
void Window::setupBorder()
{
  auto* quickWindow = qobject_cast<QQuickWindow*>(m_window.data());
  if (!quickWindow)
    return;

  m_border = new Border(quickWindow->contentItem());
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

  // Connect window control buttons
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

  // Sync active/inactive state for dimmed rendering
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

  // Add shadow margins and title bar height to content minimum
  m_minSize                  = preferredSize();
  const int margin           = shadowMargin();
  const int tbHeight         = titleBarHeight();
  const int minWidth         = qMax(0, m_minSize.width()) + 2 * margin;
  const int minHeight        = qMax(0, m_minSize.height()) + 2 * margin + tbHeight;
  const int minTitleBarWidth = 3 * (CSD::ButtonSize + CSD::ButtonSpacing) + 2 * CSD::ButtonMargin
                             + CSD::IconSize + 2 * CSD::IconMargin;

  m_window->setMinimumSize(
    QSize(qMax(minWidth, minTitleBarWidth + 2 * margin), qMax(minHeight, tbHeight + 2 * margin)));
}

/**
 * @brief Updates the frame position and size to fill the window.
 */
void Window::updateFrameGeometry()
{
  if (!m_frame || !m_window)
    return;

  m_frame->setPosition(QPointF(0, 0));
  m_frame->setSize(QSizeF(m_window->width(), m_window->height()));
}

/**
 * @brief Updates the border position and size.
 */
void Window::updateBorderGeometry()
{
  if (!m_border || !m_window)
    return;

  const int margin = shadowMargin();
  m_border->setPosition(QPointF(margin, margin));
  m_border->setSize(QSizeF(m_window->width() - 2 * margin, m_window->height() - 2 * margin));
}

/**
 * @brief Recalculates minimum window size when QML minimums change.
 */
void Window::onMinimumSizeChanged()
{
  // Detect if the content minimum size has actually changed
  const auto size = preferredSize();
  const int expW  = m_minSize.width() + 2 * shadowMargin();
  const int expH  = m_minSize.height() + 2 * shadowMargin() + titleBarHeight();

  bool changed = false;
  if (size.width() != expW) {
    changed = true;
    m_minSize.setWidth(size.width() - 2 * shadowMargin());
  }

  if (size.height() != expH) {
    changed = true;
    m_minSize.setHeight(size.height() - 2 * shadowMargin() - titleBarHeight());
  }

  if (changed)
    updateMinimumSize();
}

/**
 * @brief Creates the content container for QML content.
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

    Item {
      id: contentContainer
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

  const int margin = shadowMargin();
  const qreal w    = m_window->width() - 2 * margin;

  m_titleBar->setPosition(QPointF(margin, margin));
  m_titleBar->setSize(QSizeF(w, titleBarHeight()));
}

/**
 * @brief Updates the content container position and size.
 */
void Window::updateContentContainerGeometry()
{
  if (!m_contentContainer || !m_window)
    return;

  // Position content below the title bar with shadow margins
  const int margin   = shadowMargin();
  const int tbHeight = titleBarHeight();
  const qreal w      = m_window->width() - 2 * margin;
  const qreal h      = m_window->height() - 2 * margin - tbHeight;

  m_contentContainer->setPosition(QPointF(margin, margin + tbHeight));
  m_contentContainer->setSize(QSizeF(w, h));

  // Resize all children to match the container
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

  // Override with QML preferredWidth/preferredHeight if available
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
  const auto& theme = Misc::ThemeManager::instance();

  if (m_titleBar) {
    const QString color =
      m_color.isEmpty() ? theme.getColor(QStringLiteral("toolbar_top")).name() : m_color;
    m_titleBar->setBackgroundColor(QColor(color));
  }
}

/**
 * @brief Reparents a child item to the content container.
 * @param child Item to reparent.
 */
void Window::reparentChildToContainer(QQuickItem* child)
{
  if (!child || !m_contentContainer)
    return;

  if (child == m_frame || child == m_border || child == m_titleBar || child == m_contentContainer)
    return;

  if (child->parentItem() == m_contentContainer)
    return;

  child->setParentItem(m_contentContainer);
  child->setPosition(QPointF(0, 0));
  child->setSize(m_contentContainer->size());
}

/**
 * @brief Returns the resize edge flags at the given position.
 * @param pos Position in window coordinates.
 */
Window::ResizeEdge Window::edgeAt(const QPointF& pos) const
{
  if (!m_window || (m_window->windowStates() & Qt::WindowMaximized))
    return ResizeEdge::None;

  if (isFixedSizeWindow(m_window))
    return ResizeEdge::None;

  // Check proximity to each window edge
  const int margin = shadowMargin();
  const int w      = m_window->width();
  const int h      = m_window->height();
  const int area   = margin + CSD::ResizeMargin;

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
 * @param edge Resize edge.
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
 * @param edge Internal resize edge value.
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
 * @brief Event filter for window resize and content reparenting.
 * @param watched Object being watched.
 * @param event   Event to process.
 */
bool Window::eventFilter(QObject* watched, QEvent* event)
{
  // Reparent newly added QML items to the content container
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
        m_window->setCursor(cursorForEdge(edgeAt(me->position())));
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
        m_window->setCursor(Qt::ArrowCursor);
      break;

    default:
      break;
  }

  return false;
}
}  // namespace CSD
