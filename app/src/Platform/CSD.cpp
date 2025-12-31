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

namespace CSD
{

//------------------------------------------------------------------------------
// Platform-specific constants
//------------------------------------------------------------------------------

#ifndef QWINDOWSIZE_MAX
constexpr int QWINDOWSIZE_MAX = (1 << 24) - 1;
#endif

#if defined(Q_OS_WIN)
constexpr int IconSize = 16;
constexpr int IconMargin = 8;
constexpr int ButtonSize = 28;
constexpr int ButtonWidth = 46;
constexpr int ResizeMargin = 8;
constexpr int ButtonMargin = 8;
constexpr int CornerRadius = 0;
constexpr int ShadowRadius = 24;
constexpr int ButtonSpacing = 18;
constexpr int TitleBarHeight = 32;
constexpr int TitleBarHeightMaximized = 28;
constexpr qreal ShadowOpacity = 0.10;
#else
constexpr int IconSize = 16;
constexpr int IconMargin = 10;
constexpr int ButtonSize = 28;
constexpr int ButtonWidth = 32;
constexpr int ResizeMargin = 8;
constexpr int ButtonMargin = 12;
constexpr int CornerRadius = 0;
constexpr int ShadowRadius = 24;
constexpr int ButtonSpacing = 0;
constexpr int TitleBarHeight = 32;
constexpr int TitleBarHeightMaximized = 28;
constexpr qreal ShadowOpacity = 0.10;
#endif

//------------------------------------------------------------------------------
// Titlebar
//------------------------------------------------------------------------------

/**
 * @brief Constructs a Titlebar.
 * @param parent The parent QQuickItem.
 *
 * Initializes the title bar with default values, enables mouse and hover
 * event handling, and loads the application icon for display.
 */
Titlebar::Titlebar(QQuickItem *parent)
  : QQuickPaintedItem(parent)
  , m_title()
  , m_dragging(false)
  , m_windowActive(true)
  , m_hoveredButton(Button::None)
  , m_pressedButton(Button::None)
  , m_backgroundColor(QStringLiteral("#2d2d2d"))
{
  // Set item flags & background color
  setOpaquePainting(false);
  setAcceptHoverEvents(true);
  setFillColor(Qt::transparent);
  setAcceptedMouseButtons(Qt::LeftButton);

  // Load application icon if possible (at appropriate DPI scale)
  const auto &icon = QGuiApplication::windowIcon();
  if (!icon.isNull())
  {
    const qreal dpr = qGuiApp->devicePixelRatio();
    m_icon = icon.pixmap(QSize(CSD::IconSize, CSD::IconSize));
    m_icon.setDevicePixelRatio(dpr);
  }
}

/**
 * @brief Paints the title bar.
 * @param painter The QPainter used for rendering.
 *
 * Renders the complete title bar including:
 * - Background with rounded top corners (when not maximized)
 * - Application icon on the left
 * - Centered window title text
 * - Minimize, maximize/restore, and close buttons
 */
void Titlebar::paint(QPainter *painter)
{
  if (!window())
    return;

  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  QRectF rect = boundingRect();
  painter->fillRect(rect, m_backgroundColor);

  // Draw application icon
  if (!m_icon.isNull())
  {
    const qreal y = (height() - CSD::IconSize) / 2.0;
    painter->drawPixmap(QPointF(CSD::IconMargin, y), m_icon);
  }

  // Set text color for the title
  if (m_windowActive)
    painter->setPen(foregroundColor());
  else
    painter->setPen(foregroundColor().darker(130));

  // Draw window title
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

  // Draw window buttons (only if they should be shown)
  if (shouldShowButton(Button::Close))
    drawButton(painter, Button::Close, closeSvg);
  if (shouldShowButton(Button::Minimize))
    drawButton(painter, Button::Minimize, minimizeSvg);
  if (shouldShowButton(Button::Maximize))
    drawButton(painter, Button::Maximize, maximizeSvg);
}

/**
 * @brief Returns the window title text.
 * @return The current title string.
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
 * @return True if the window is active, false otherwise.
 */
bool Titlebar::windowActive() const
{
  return m_windowActive;
}

/**
 * @brief Returns the title bar background color.
 * @return The current background color.
 */
QColor Titlebar::backgroundColor() const
{
  return m_backgroundColor;
}

/**
 * @brief Calculates the foreground color based on background luminance.
 * @return Appropriate text color for readability.
 *
 * If the background matches the theme's toolbar_top color, returns the
 * theme's titlebar_text color. Otherwise, calculates optimal contrast
 * using the W3C relative luminance formula and WCAG AA threshold.
 */
QColor Titlebar::foregroundColor() const
{
  const auto &theme = Misc::ThemeManager::instance();
  const QColor toolbarTop = theme.getColor(QStringLiteral("toolbar_top"));

  // Use theme color if background matches toolbar
  if (m_backgroundColor == toolbarTop)
    return theme.getColor(QStringLiteral("titlebar_text"));

  // Calculate relative luminance (W3C formula)
  // clang-format off
  auto linearize = [](qreal c) { return c <= 0.03928 ? c / 12.92 : qPow((c + 0.055) / 1.055, 2.4); };
  const qreal luminance = 0.2126 * linearize(m_backgroundColor.redF())
                          + 0.7152 * linearize(m_backgroundColor.greenF())
                          + 0.0722 * linearize(m_backgroundColor.blueF());
  // clang-format on

  // WCAG AA contrast threshold
  return luminance > 0.179 ? QColor(Qt::black) : QColor(Qt::white);
}

/**
 * @brief Sets the title bar text.
 * @param title The new title string.
 *
 * Emits titleChanged() and triggers a repaint if the title changes.
 */
void Titlebar::setTitle(const QString &title)
{
  if (m_title != title)
  {
    m_title = title;
    Q_EMIT titleChanged();
    update();
  }
}

/**
 * @brief Sets the window active state.
 * @param active The new active state.
 *
 * When inactive, the title bar renders with dimmed colors.
 * Emits windowActiveChanged() and triggers a repaint if the state changes.
 */
void Titlebar::setWindowActive(bool active)
{
  if (m_windowActive != active)
  {
    m_windowActive = active;
    Q_EMIT windowActiveChanged();
    update();
  }
}

/**
 * @brief Sets the title bar background color.
 * @param color The new background color.
 *
 * Emits backgroundColorChanged() and triggers a repaint if the color changes.
 */
void Titlebar::setBackgroundColor(const QColor &color)
{
  if (m_backgroundColor != color)
  {
    m_backgroundColor = color;
    Q_EMIT backgroundColorChanged();
    update();
  }
}

/**
 * @brief Calculates the bounding rectangle for a window control button.
 * @param button The button type to get the rectangle for.
 * @return The button's bounding rectangle in item coordinates.
 *
 * Buttons are positioned from right to left: close, maximize, minimize.
 */
QRectF Titlebar::buttonRect(Button button) const
{
  const QRectF bgRect = buttonBackgroundRect(button);
  const qreal iconSize = CSD::ButtonSize * 0.5;

  return {bgRect.center().x() - iconSize / 2,
          bgRect.center().y() - iconSize / 2, iconSize, iconSize};
}

/**
 * @brief Determines which button (if any) is at the given position.
 * @param pos The position to test in item coordinates.
 * @return The button at the position, or Button::None if no button.
 */
Titlebar::Button Titlebar::buttonAt(const QPointF &pos) const
{
  if (shouldShowButton(Button::Close)
      && buttonBackgroundRect(Button::Close).contains(pos))
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
 * @brief Determines if a button should be shown based on window flags.
 * @param button The button type to check.
 * @return True if the button should be displayed, false otherwise.
 *
 * Checks the window flags to determine visibility:
 * - If CustomizeWindowHint is not set, all buttons are shown (default behavior)
 * - If CustomizeWindowHint is set, only buttons with explicit hints are shown
 * - Maximize button is hidden if window has fixed size (minimumSize ==
 * maximumSize)
 */
bool Titlebar::shouldShowButton(Button button) const
{
  if (!window())
    return true;

  const auto flags = window()->flags();

  // If CustomizeWindowHint is not set, show default buttons
  const bool useCustomHints = flags & Qt::CustomizeWindowHint;

  switch (button)
  {
    case Button::Minimize:
      if (!useCustomHints)
        return true;
      return flags & Qt::WindowMinimizeButtonHint;

    case Button::Maximize: {
      // Check for fixed size windows
      const auto minSize = window()->minimumSize();
      const auto maxSize = window()->maximumSize();
      if (minSize.isValid() && maxSize.isValid() && minSize == maxSize)
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
 * @brief Calculates the background rectangle for drawing button hover/press
 *        states.
 *
 * @param button The button type to get the rectangle for.
 * @return The button's background rectangle.
 *
 * Windows 11 style: buttons span full titlebar height with ~46px width.
 * Close button extends to window edge. Buttons are positioned dynamically
 * based on visibility (window flags and resize constraints).
 */
QRectF Titlebar::buttonBackgroundRect(Button button) const
{
  const qreal h = height();

  int buttonIndex = 0;
  const bool showClose = shouldShowButton(Button::Close);
  const bool showMaximize = shouldShowButton(Button::Maximize);
  const bool showMinimize = shouldShowButton(Button::Minimize);

  switch (button)
  {
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
 * @param painter The painter to use.
 * @param button The button type to draw.
 * @param svgPath Path to the SVG icon resource.
 *
 * Renders the SVG icon centered within the button bounds, colorized
 * based on the current state (normal, hovered, pressed, or inactive).
 */
void Titlebar::drawButton(QPainter *painter, Button button,
                          const QString &svgPath)
{
  const QRectF iconRect = buttonRect(button);
  const bool hovered = (m_hoveredButton == button);
  const bool pressed = (m_pressedButton == button);

  QColor iconColor;

#if defined(Q_OS_WIN)
  const auto bg = m_backgroundColor;
  const auto fg = foregroundColor();
  const bool isDarkTheme = fg.lightness() > bg.lightness();

  if (hovered || pressed)
  {
    QColor bgColor;

    if (button == Button::Close)
    {
      if (pressed)
        bgColor = QColor(0xB4, 0x27, 0x1A);
      else
        bgColor = QColor(0xC4, 0x2B, 0x1C);
    }
    else
    {
      if (isDarkTheme)
        bgColor = QColor(255, 255, 255, pressed ? 11 : 20);
      else
        bgColor = QColor(0, 0, 0, pressed ? 6 : 10);
    }

    painter->fillRect(buttonBackgroundRect(button), bgColor);
  }

  if (button == Button::Close && (hovered || pressed))
    iconColor = Qt::white;

  else if (!m_windowActive)
  {
    iconColor = foregroundColor();
    iconColor.setAlpha(128);
  }

  else
    iconColor = foregroundColor();
#else
  const auto &theme = Misc::ThemeManager::instance();

  if (pressed)
    iconColor = theme.getColor(QStringLiteral("highlight")).darker(120);
  else if (hovered)
    iconColor = theme.getColor(QStringLiteral("highlight"));

  else if (!m_windowActive)
  {
    iconColor = foregroundColor();
    iconColor.setAlpha(128);
  }

  else
    iconColor = foregroundColor();
#endif

  QSvgRenderer renderer(svgPath);
  if (!renderer.isValid())
    return;

  const qreal dpr = qApp->devicePixelRatio();
  const QSize pixelSize(qRound(iconRect.width() * dpr),
                        qRound(iconRect.height() * dpr));
  const QRectF logicalRect(0, 0, iconRect.width(), iconRect.height());

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

  painter->drawPixmap(iconRect.topLeft(), colorized);
}

/**
 * @brief Handles loss of mouse grab.
 *
 * Resets all interaction state (hover, press, drag) to prevent buttons
 * from remaining stuck in highlighted state when the grab is lost
 * unexpectedly.
 */
void Titlebar::mouseUngrabEvent()
{
  m_hoveredButton = Button::None;
  m_pressedButton = Button::None;
  m_dragging = false;
  update();
}

/**
 * @brief Handles mouse press events.
 * @param event The mouse event.
 *
 * If a button is pressed, records the pressed state for visual feedback.
 * Otherwise, marks the start of a potential window drag operation.
 */
void Titlebar::mousePressEvent(QMouseEvent *event)
{
  m_pressedButton = buttonAt(event->position());

  if (m_pressedButton != Button::None)
  {
    update();
    event->accept();
    return;
  }

  m_dragging = true;
  event->accept();
}

/**
 * @brief Handles mouse release events.
 * @param event The mouse event.
 *
 * If a button was pressed and released on the same button, emits the
 * corresponding signal (minimizeClicked, maximizeClicked, or closeClicked).
 * Updates hover state based on current mouse position.
 */
void Titlebar::mouseReleaseEvent(QMouseEvent *event)
{
  const Button releasedOn = buttonAt(event->position());

  if (m_pressedButton != Button::None && m_pressedButton == releasedOn)
  {
    switch (m_pressedButton)
    {
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
  m_dragging = false;
  m_hoveredButton = releasedOn;
  update();
  event->accept();
}

/**
 * @brief Handles mouse move events for window dragging.
 * @param event The mouse event.
 *
 * On first movement after press, initiates native window drag via
 * startSystemMove(). If the window is maximized, restores it first
 * and repositions so the cursor maintains its relative position.
 */
void Titlebar::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_dragging || !window())
  {
    event->accept();
    return;
  }

  auto *win = window();
  m_dragging = false;

  if (isMaximized())
  {
    const qreal relativeX = event->position().x() / width();
    const QPointF globalPos = event->globalPosition();

    win->showNormal();
    win->setPosition(qRound(globalPos.x() - win->width() * relativeX),
                     qRound(globalPos.y() - height() / 2));
  }

  win->startSystemMove();
  event->accept();
}

/**
 * @brief Handles double-click events to toggle window maximized state.
 * @param event The mouse event.
 *
 * Double-clicking on the title bar (not on a button) toggles the window
 * between maximized and normal states, unless the window has a fixed size
 * (minimumSize == maximumSize) or lacks the maximize button hint.
 */
void Titlebar::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (buttonAt(event->position()) == Button::None && window())
  {
    m_dragging = false;
    m_pressedButton = Button::None;
    m_hoveredButton = Button::None;

    if (shouldShowButton(Button::Maximize))
    {
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
 * @param event The hover event.
 *
 * Updates the currently hovered button and triggers a repaint if
 * the hover state changes.
 */
void Titlebar::hoverMoveEvent(QHoverEvent *event)
{
  const Button newHovered = buttonAt(event->position());
  if (newHovered != m_hoveredButton)
  {
    m_hoveredButton = newHovered;
    update();
  }
}

/**
 * @brief Handles hover leave events to clear button hover states.
 * @param event The hover event.
 *
 * Clears the hovered button state when the mouse leaves the title bar.
 */
void Titlebar::hoverLeaveEvent(QHoverEvent *event)
{
  Q_UNUSED(event)
  if (m_hoveredButton != Button::None)
  {
    m_hoveredButton = Button::None;
    update();
  }
}

//------------------------------------------------------------------------------
// Frame
//------------------------------------------------------------------------------

/**
 * @brief Constructs a Frame.
 * @param parent The parent QQuickItem.
 *
 * Initializes the frame with default shadow settings, enables transparency,
 * and generates the initial shadow tile images.
 */
Frame::Frame(QQuickItem *parent)
  : QQuickPaintedItem(parent)
  , m_shadowRadius(CSD::ShadowRadius)
  , m_shadowEnabled(true)
{
  setFillColor(Qt::transparent);
  setOpaquePainting(false);
  regenerateShadow();
}

/**
 * @brief Paints the window frame including shadow and background.
 * @param painter The QPainter used for rendering.
 *
 * Renders the complete window frame including:
 * - Corner shadow tiles at each corner
 * - Edge shadow strips between corners
 * - Background fill matching the theme
 *
 * When shadows are disabled (e.g., window maximized), only the background
 * is drawn.
 */
void Frame::paint(QPainter *painter)
{
  const int r = m_shadowEnabled ? m_shadowRadius : 0;
  const QRectF content(r, r, width() - 2 * r, height() - 2 * r);

  // Draw shadow
  if (m_shadowEnabled && !m_shadowCorner.isNull() && r > 0)
  {
    painter->drawImage(0, 0, m_shadowCorner);
    painter->drawImage(QPointF(width() - r, 0),
                       m_shadowCorner.flipped(Qt::Horizontal));
    painter->drawImage(QPointF(0, height() - r),
                       m_shadowCorner.flipped(Qt::Vertical));
    painter->drawImage(QPointF(width() - r, height() - r),
                       m_shadowCorner.flipped(Qt::Horizontal | Qt::Vertical));

    if (!m_shadowEdge.isNull())
    {
      const qreal hEdgeLen = width() - 2 * r;
      const qreal vEdgeLen = height() - 2 * r;

      if (hEdgeLen > 0)
      {
        painter->drawImage(QRectF(r, 0, hEdgeLen, r), m_shadowEdge,
                           QRectF(0, 0, 1, r));
        painter->drawImage(QRectF(r, height() - r, hEdgeLen, r),
                           m_shadowEdge.flipped(Qt::Vertical),
                           QRectF(0, 0, 1, r));
      }

      if (vEdgeLen > 0)
      {
        QImage hEdge(r, 1, QImage::Format_ARGB32_Premultiplied);
        hEdge.fill(Qt::transparent);
        for (int i = 0; i < r; ++i)
        {
          const qreal dist = static_cast<qreal>(i) / r;
          qreal alpha = 1.0 - dist;
          alpha = alpha * alpha * (3.0 - 2.0 * alpha);
          alpha *= CSD::ShadowOpacity;
          hEdge.setPixelColor(i, 0, QColor(0, 0, 0, qRound(alpha * 255)));
        }

        painter->drawImage(QRectF(0, r, r, vEdgeLen),
                           hEdge.flipped(Qt::Horizontal), QRectF(0, 0, r, 1));
        painter->drawImage(QRectF(width() - r, r, r, vEdgeLen), hEdge,
                           QRectF(0, 0, r, 1));
      }
    }
  }

  // Draw content background
  if (content.width() > 0 && content.height() > 0)
  {
    const auto &theme = Misc::ThemeManager::instance();
    const QColor bgColor = theme.getColor(QStringLiteral("toolbar_top"));

    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    painter->fillRect(content, bgColor);
  }
}

/**
 * @brief Returns the shadow blur radius in pixels.
 * @return The current shadow radius.
 */
int Frame::shadowRadius() const
{
  return m_shadowRadius;
}

/**
 * @brief Returns whether the shadow is enabled.
 * @return True if shadow rendering is enabled, false otherwise.
 */
bool Frame::shadowEnabled() const
{
  return m_shadowEnabled;
}

/**
 * @brief Sets the shadow blur radius.
 * @param radius The new radius in pixels.
 *
 * Regenerates shadow tiles and triggers a repaint if the radius changes.
 * Emits shadowRadiusChanged().
 */
void Frame::setShadowRadius(int radius)
{
  if (m_shadowRadius != radius)
  {
    m_shadowRadius = radius;
    regenerateShadow();
    Q_EMIT shadowRadiusChanged();
    update();
  }
}

/**
 * @brief Enables or disables shadow rendering.
 * @param enabled True to enable shadows, false to disable.
 *
 * When disabled, the frame renders without shadows or rounded corners
 * (typically used when the window is maximized).
 * Emits shadowEnabledChanged() and triggers a repaint if the state changes.
 */
void Frame::setShadowEnabled(bool enabled)
{
  if (m_shadowEnabled != enabled)
  {
    m_shadowEnabled = enabled;
    Q_EMIT shadowEnabledChanged();
    update();
  }
}

/**
 * @brief Regenerates the shadow tile images.
 *
 * Creates pre-rendered corner and edge tiles for efficient shadow painting.
 * The corner tile accounts for the rounded content corners, while the edge
 * tile is a 1-pixel wide gradient that gets stretched during rendering.
 * Uses a smoothstep falloff for natural shadow appearance.
 */
void Frame::regenerateShadow()
{
  if (m_shadowRadius <= 0)
  {
    m_shadowCorner = QImage();
    m_shadowEdge = QImage();
    return;
  }

  m_shadowCorner = generateShadowCorner(m_shadowRadius);
  m_shadowEdge = QImage(1, m_shadowRadius, QImage::Format_ARGB32_Premultiplied);
  m_shadowEdge.fill(Qt::transparent);

  for (int y = 0; y < m_shadowRadius; ++y)
  {
    const qreal dist
        = static_cast<qreal>(m_shadowRadius - y - 1) / m_shadowRadius;
    qreal alpha = 1.0 - dist;
    alpha = alpha * alpha * (3.0 - 2.0 * alpha);
    alpha *= CSD::ShadowOpacity;
    m_shadowEdge.setPixelColor(0, y, QColor(0, 0, 0, qRound(alpha * 255)));
  }
}

/**
 * @brief Generates a shadow corner tile image.
 * @param size The shadow blur radius in pixels.
 * @return The generated corner tile image.
 *
 * Creates a square shadow tile with a diagonal gradient.
 * Uses smoothstep falloff for natural shadow appearance.
 */
QImage Frame::generateShadowCorner(int size)
{
  QImage tile(size, size, QImage::Format_ARGB32_Premultiplied);
  tile.fill(Qt::transparent);

  for (int y = 0; y < size; ++y)
  {
    for (int x = 0; x < size; ++x)
    {
      const qreal dx = static_cast<qreal>(size - x - 1);
      const qreal dy = static_cast<qreal>(size - y - 1);
      const qreal dist = qSqrt(dx * dx + dy * dy);

      qreal alpha = 1.0 - qMin(dist / static_cast<qreal>(size), 1.0);
      alpha = alpha * alpha * (3.0 - 2.0 * alpha);
      alpha *= CSD::ShadowOpacity;
      tile.setPixelColor(x, y, QColor(0, 0, 0, qRound(alpha * 255)));
    }
  }

  return tile;
}

//------------------------------------------------------------------------------
// Border
//------------------------------------------------------------------------------

/**
 * @brief Constructs a Border.
 * @param parent The parent QQuickItem.
 *
 * Initializes the border overlay with transparency and configures
 * the render target for proper alpha blending.
 */
Border::Border(QQuickItem *parent)
  : QQuickPaintedItem(parent)
{
  setFillColor(Qt::transparent);
  setOpaquePainting(false);
  setRenderTarget(QQuickPaintedItem::FramebufferObject);
  setAntialiasing(true);
}

/**
 * @brief Paints the window border.
 * @param painter The QPainter used for rendering.
 *
 * Draws a 1px border around the entire window area using a semi-transparent
 * gray color.
 */
void Border::paint(QPainter *painter)
{
  const QColor borderColor = QColor(102, 102, 102, 115);

  painter->setPen(QPen(borderColor, 1));
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(QRectF(0.5, 0.5, width() - 1, height() - 1));
}

//------------------------------------------------------------------------------
// Window
//------------------------------------------------------------------------------

/**
 * @brief Constructs a Window decorator.
 * @param window The QWindow to decorate with client-side decorations.
 * @param color Optional custom title bar color (hex string). Empty for theme
 * default.
 * @param parent Parent QObject.
 *
 * Initializes complete CSD functionality including:
 * - Frameless window configuration with transparency
 * - Shadow frame, title bar, and content container setup
 * - Event filter installation for resize handling
 * - Minimum size tracking and adjustment
 * - Theme change monitoring
 */
Window::Window(QWindow *window, const QString &color, QObject *parent)
  : QObject(parent)
  , m_resizing(false)
  , m_frame(nullptr)
  , m_border(nullptr)
  , m_color(color)
  , m_titleBar(nullptr)
  , m_resizeEdge(ResizeEdge::None)
  , m_minSize(0, 0)
  , m_maxSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX)
  , m_window(window)
  , m_contentContainer(nullptr)
{
  // Stop if window pointer is invalid
  if (!m_window)
    return;

  // Configure window for CSD
  m_window->setFlags(m_window->flags() | Qt::FramelessWindowHint);
  m_window->installEventFilter(this);

  // Setup components in order: frame -> container -> titlebar -> border
  setupFrame();
  setupContentContainer();
  setupTitleBar();
  setupBorder();

  // Handle window state changes
  connect(m_window, &QWindow::windowStateChanged, this, [this]() {
    const auto state = m_window->windowStates();
    const bool fillScreen
        = state & (Qt::WindowMaximized | Qt::WindowFullScreen);

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

  // Track minimum size changes from QML/user
  connect(m_window, &QWindow::minimumWidthChanged, this,
          &Window::onMinimumSizeChanged);
  connect(m_window, &QWindow::minimumHeightChanged, this,
          &Window::onMinimumSizeChanged);

  // Track maximum size changes from QML/user
  connect(m_window, &QWindow::maximumWidthChanged, this,
          &Window::onMinimumSizeChanged);
  connect(m_window, &QWindow::maximumHeightChanged, this,
          &Window::onMinimumSizeChanged);

  // Theme updates
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Window::updateTheme);
  updateTheme();

  // Store initial content maximum size before applying CSD adjustments
  const auto initialMaxSize = m_window->maximumSize();
  if (initialMaxSize.isValid() && initialMaxSize.width() > 0
      && initialMaxSize.height() > 0 && initialMaxSize.width() < QWINDOWSIZE_MAX
      && initialMaxSize.height() < QWINDOWSIZE_MAX)
  {
    m_maxSize = initialMaxSize;
  }

  // Apply initial minimum size
  updateMinimumSize();
}

/**
 * @brief Destructor.
 *
 * Removes event filters from the window and its content item to prevent
 * dangling callbacks after destruction.
 */
Window::~Window()
{
  if (m_window)
  {
    m_window->removeEventFilter(this);
    if (auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data()))
      quickWindow->contentItem()->removeEventFilter(this);
  }
}

/**
 * @brief Returns the shadow frame component.
 * @return Pointer to the Frame used for shadow and border rendering.
 */
Frame *Window::frame() const
{
  return m_frame;
}

/**
 * @brief Returns the decorated window.
 * @return Pointer to the QWindow being decorated, or nullptr if invalid.
 */
QWindow *Window::window() const
{
  return m_window;
}

/**
 * @brief Returns the title bar component.
 * @return Pointer to the Titlebar used for window controls.
 */
Titlebar *Window::titleBar() const
{
  return m_titleBar;
}

/**
 * @brief Returns the current shadow margin.
 * @return Shadow radius in pixels, or 0 if window is maximized/fullscreen.
 *
 * The shadow margin determines the offset between the window edge
 * and the visible content area.
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
 * @brief Returns the current title bar height.
 * @return Height in pixels, adjusted for maximized/fullscreen state.
 *
 * Returns a smaller height when maximized, and 0 when fullscreen,
 * following common desktop environment conventions.
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
 * @param color The color as a hex string (e.g., "#2d2d2d").
 *
 * Setting an empty string causes the title bar to use the
 * theme's toolbar_top color.
 */
void Window::setColor(const QString &color)
{
  m_color = color;
  updateTheme();
}

/**
 * @brief Creates and configures the shadow frame.
 *
 * Sets up window transparency, creates the Frame component,
 * and connects geometry update signals. The frame is placed
 * at z-index -1 to render behind all content.
 */
void Window::setupFrame()
{
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return;

  QSurfaceFormat format = quickWindow->format();
  if (format.alphaBufferSize() < 8)
  {
    format.setAlphaBufferSize(8);
    quickWindow->setFormat(format);
  }

  quickWindow->setColor(Qt::transparent);
  m_frame = new Frame(quickWindow->contentItem());
  m_frame->setZ(-1);

  connect(quickWindow, &QQuickWindow::widthChanged, this,
          &Window::updateFrameGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this,
          &Window::updateFrameGeometry);

  updateFrameGeometry();
}

/**
 * @brief Creates and configures the border overlay.
 *
 * Creates the Border component on top of all other content.
 * The border is hidden when the window is maximized or fullscreen.
 */
void Window::setupBorder()
{
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return;

  m_border = new Border(quickWindow->contentItem());
  m_border->setZ(1000000);

  connect(quickWindow, &QQuickWindow::widthChanged, this,
          &Window::updateBorderGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this,
          &Window::updateBorderGeometry);

  updateBorderGeometry();
}

/**
 * @brief Creates and configures the title bar.
 *
 * Creates the Titlebar component, connects window control signals
 * (minimize, maximize, close), and sets up tracking for window
 * title and active state changes.
 */
void Window::setupTitleBar()
{
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return;

  m_titleBar = new Titlebar(quickWindow->contentItem());
  m_titleBar->setZ(999999);

  // Window controls
  connect(m_titleBar, &Titlebar::closeClicked, this,
          [this]() { m_window->close(); });
  connect(m_titleBar, &Titlebar::minimizeClicked, this,
          [this]() { m_window->showMinimized(); });
  connect(m_titleBar, &Titlebar::maximizeClicked, this, [this]() {
    if (m_window->windowStates() & Qt::WindowMaximized)
      m_window->showNormal();
    else
      m_window->showMaximized();
  });

  // Track state
  connect(m_window, &QWindow::activeChanged, this,
          [this]() { m_titleBar->setWindowActive(m_window->isActive()); });

  m_titleBar->setTitle(m_window->title());
  connect(m_window, &QWindow::windowTitleChanged, m_titleBar,
          &Titlebar::setTitle);
  connect(quickWindow, &QQuickWindow::widthChanged, this,
          &Window::updateTitleBarGeometry);
  updateTitleBarGeometry();
}

/**
 * @brief Updates the window minimum and maximum sizes to account for CSD
 *        decorations.
 *
 * Adds shadow margins and title bar height to the content minimum size
 * to ensure the content area never becomes smaller than intended.
 * Also enforces a minimum width to fit title bar controls.
 *
 * If the window has a maximum size set (indicating a fixed-size window),
 * the maximum size is also adjusted by adding the same CSD margins,
 * preserving the fixed-size constraint.
 */
void Window::updateMinimumSize()
{
  if (!m_window)
    return;

  m_minSize = preferredSize();
  const int margin = shadowMargin();
  const int tbHeight = titleBarHeight();
  const int minWidth = qMax(0, m_minSize.width()) + 2 * margin;
  const int minHeight = qMax(0, m_minSize.height()) + 2 * margin + tbHeight;
  const int minTitleBarWidth = 3 * (CSD::ButtonSize + CSD::ButtonSpacing)
                               + 2 * CSD::ButtonMargin + CSD::IconSize
                               + 2 * CSD::IconMargin;

  m_window->setMinimumSize(QSize(qMax(minWidth, minTitleBarWidth + 2 * margin),
                                 qMax(minHeight, tbHeight + 2 * margin)));

  if (m_maxSize.width() > 0 && m_maxSize.height() > 0
      && m_maxSize.width() < QWINDOWSIZE_MAX
      && m_maxSize.height() < QWINDOWSIZE_MAX)
  {
    const int maxWidth = m_maxSize.width() + 2 * margin;
    const int maxHeight = m_maxSize.height() + 2 * margin + tbHeight;
    m_window->setMaximumSize(QSize(maxWidth, maxHeight));
  }
}

/**
 * @brief Updates the frame position and size to fill the window.
 *
 * The frame always covers the entire window area, including the
 * shadow region around the content.
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
 *
 * Positions the border to cover the visible content area
 * (excluding the shadow margin).
 */
void Window::updateBorderGeometry()
{
  if (!m_border || !m_window)
    return;

  const int margin = shadowMargin();
  m_border->setPosition(QPointF(margin, margin));
  m_border->setSize(
      QSizeF(m_window->width() - 2 * margin, m_window->height() - 2 * margin));
}

/**
 * @brief Re-calculates window size constraints (considering titlebar & shadows)
 *        when the QML user interface changes its size requirements.
 *
 * Tracks both minimum and maximum size changes from the QML layer and
 * updates the stored content sizes (m_minSize, m_maxSize) by removing
 * CSD margins, then calls updateMinimumSize() to reapply the constraints
 * with correct CSD margins.
 */
void Window::onMinimumSizeChanged()
{
  const auto size = preferredSize();
  const int expW = m_minSize.width() + 2 * shadowMargin();
  const int expH = m_minSize.height() + 2 * shadowMargin() + titleBarHeight();

  bool changed = false;
  if (size.width() != expW)
  {
    changed = true;
    m_minSize.setWidth(size.width() - 2 * shadowMargin());
  }

  if (size.height() != expH)
  {
    changed = true;
    m_minSize.setHeight(size.height() - 2 * shadowMargin() - titleBarHeight());
  }

  const auto maxSize = m_window->maximumSize();
  if (maxSize.isValid() && maxSize.width() > 0 && maxSize.height() > 0
      && maxSize.width() < QWINDOWSIZE_MAX
      && maxSize.height() < QWINDOWSIZE_MAX)
  {
    const int expMaxW = m_maxSize.width() + 2 * shadowMargin();
    const int expMaxH
        = m_maxSize.height() + 2 * shadowMargin() + titleBarHeight();

    if (maxSize.width() != expMaxW)
    {
      changed = true;
      m_maxSize.setWidth(maxSize.width() - 2 * shadowMargin());
    }

    if (maxSize.height() != expMaxH)
    {
      changed = true;
      m_maxSize.setHeight(maxSize.height() - 2 * shadowMargin()
                          - titleBarHeight());
    }
  }

  if (changed)
    updateMinimumSize();
}

/**
 * @brief Creates content container for QML content.
 *
 * Creates a simple Item container that holds all QML content,
 * positioned below the title bar with appropriate margins.
 */
void Window::setupContentContainer()
{
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return;

  QQuickItem *root = quickWindow->contentItem();
  QQmlEngine *engine = qmlEngine(quickWindow);
  if (!engine)
    return;

  static const char *qmlCode = R"(
    import QtQuick

    Item {
      id: contentContainer
    }
  )";

  QQmlComponent component(engine);
  component.setData(qmlCode, QUrl());

  if (component.isError())
  {
    for (const auto &error : component.errors())
      qWarning() << "CSD QML error:" << error.toString();
    return;
  }

  m_contentContainer = qobject_cast<QQuickItem *>(component.create());
  if (!m_contentContainer)
    return;

  m_contentContainer->setParentItem(root);
  m_contentContainer->setZ(0);

  connect(quickWindow, &QQuickWindow::widthChanged, this,
          &Window::updateContentContainerGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this,
          &Window::updateContentContainerGeometry);

  updateContentContainerGeometry();

  const auto children = root->childItems();
  for (QQuickItem *child : children)
    reparentChildToContainer(child);
}

/**
 * @brief Updates the title bar position and size.
 *
 * Positions the title bar at the top of the visible content area.
 */
void Window::updateTitleBarGeometry()
{
  if (!m_titleBar || !m_window)
    return;

  const int margin = shadowMargin();
  const qreal w = m_window->width() - 2 * margin;

  m_titleBar->setPosition(QPointF(margin, margin));
  m_titleBar->setSize(QSizeF(w, titleBarHeight()));
}

/**
 * @brief Updates the content container position and size.
 *
 * Positions the container below the title bar with appropriate margins.
 * Also updates the size of all children to match the container.
 */
void Window::updateContentContainerGeometry()
{
  if (!m_contentContainer || !m_window)
    return;

  const int margin = shadowMargin();
  const int tbHeight = titleBarHeight();
  const qreal w = m_window->width() - 2 * margin;
  const qreal h = m_window->height() - 2 * margin - tbHeight;

  m_contentContainer->setPosition(QPointF(margin, margin + tbHeight));
  m_contentContainer->setSize(QSizeF(w, h));

  const auto children = m_contentContainer->childItems();
  for (QQuickItem *child : children)
  {
    child->setPosition(QPointF(0, 0));
    child->setSize(QSizeF(w, h));
  }
}

/**
 * @brief Retrieves the preferred size of the window
 *
 * Determines the preferred dimensions for the window by first using the
 * minimum size as a baseline. For QQuickWindow instances, this can be
 * overridden by custom QML properties @c preferredWidth and @c preferredHeight.
 *
 * @note If the underlying window is null, returns a zero size (0x0).
 *
 * @return QSize The preferred window dimensions, determined by:
 *         - Custom QML properties if available and valid (for QQuickWindow)
 *         - Minimum window size otherwise
 *
 * @see QWindow::minimumSize()
 * @see QQuickWindow
 */
QSize Window::preferredSize() const
{
  if (!m_window)
    return QSize(0, 0);

  auto preferredSize = m_window->minimumSize();
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return preferredSize;

  auto pWidth = quickWindow->property("preferredWidth");
  auto pHeight = quickWindow->property("preferredHeight");
  if (!pWidth.isNull() && pWidth.isValid())
    preferredSize.setWidth(pWidth.toInt());
  if (!pHeight.isNull() && pHeight.isValid())
    preferredSize.setHeight(pHeight.toInt());

  return preferredSize;
}

/**
 * @brief Updates theme colors for the title bar.
 *
 * Applies either the custom color or the theme's toolbar_top color
 * to the title bar background.
 */
void Window::updateTheme()
{
  const auto &theme = Misc::ThemeManager::instance();

  if (m_titleBar)
  {
    const QString color
        = m_color.isEmpty()
              ? theme.getColor(QStringLiteral("toolbar_top")).name()
              : m_color;
    m_titleBar->setBackgroundColor(QColor(color));
  }
}

/**
 * @brief Reparents a child item to the content container.
 * @param child The item to reparent.
 *
 * Skips CSD components (frame, border, title bar, content container)
 * and items already in the container. This ensures all user QML content
 * respects the CSD margins automatically.
 */
void Window::reparentChildToContainer(QQuickItem *child)
{
  if (!child || !m_contentContainer)
    return;

  if (child == m_frame || child == m_border || child == m_titleBar
      || child == m_contentContainer)
    return;

  if (child->parentItem() == m_contentContainer)
    return;

  child->setParentItem(m_contentContainer);
  child->setPosition(QPointF(0, 0));
  child->setSize(m_contentContainer->size());
}

/**
 * @brief Determines the resize edge at the given position.
 * @param pos The position in window coordinates.
 * @return The ResizeEdge flags indicating which edges are under the cursor.
 *
 * Returns ResizeEdge::None if the window is maximized, has fixed size
 * (minimumSize == maximumSize), or the position is not within the
 * resize margin of any edge.
 */
Window::ResizeEdge Window::edgeAt(const QPointF &pos) const
{
  if (!m_window || (m_window->windowStates() & Qt::WindowMaximized))
    return ResizeEdge::None;

  const auto minSize = m_window->minimumSize();
  const auto maxSize = m_window->maximumSize();
  if (minSize.isValid() && maxSize.isValid() && minSize == maxSize)
    return ResizeEdge::None;

  const int margin = shadowMargin();
  const int w = m_window->width();
  const int h = m_window->height();
  const int area = margin + CSD::ResizeMargin;

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
 * @brief Returns the appropriate cursor shape for a resize edge.
 * @param edge The resize edge.
 * @return The Qt::CursorShape to display for resize feedback.
 */
Qt::CursorShape Window::cursorForEdge(ResizeEdge edge) const
{
  switch (edge)
  {
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
 * @param edge The internal resize edge value.
 * @return The equivalent Qt::Edges flags for use with startSystemResize().
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
 * @param watched The object being watched.
 * @param event The event to process.
 * @return True if the event was handled, false to continue propagation.
 *
 * Handles two types of events:
 * - ChildAdded on content item: Reparents new QML items to the container
 * - Mouse events on window: Updates resize cursors and initiates system resize
 *
 * Uses native startSystemResize() for proper compositor integration.
 */
bool Window::eventFilter(QObject *watched, QEvent *event)
{
  if (auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data()))
  {
    if (watched == quickWindow->contentItem()
        && event->type() == QEvent::ChildAdded)
    {
      auto *childEvent = static_cast<QChildEvent *>(event);
      if (auto *child = qobject_cast<QQuickItem *>(childEvent->child()))
      {
        QTimer::singleShot(
            0, this, [this, child]() { reparentChildToContainer(child); });
      }

      return false;
    }
  }

  if (watched != m_window)
    return false;

  switch (event->type())
  {
    case QEvent::MouseMove: {
      auto *me = static_cast<QMouseEvent *>(event);
      if (!m_resizing)
        m_window->setCursor(cursorForEdge(edgeAt(me->position())));
      break;
    }

    case QEvent::MouseButtonPress: {
      auto *me = static_cast<QMouseEvent *>(event);
      if (me->button() == Qt::LeftButton)
      {
        m_resizeEdge = edgeAt(me->position());
        if (m_resizeEdge != ResizeEdge::None)
        {
          m_resizing = true;
          m_window->startSystemResize(qtEdgesFromResizeEdge(m_resizeEdge));
          m_resizing = false;
          return true;
        }
      }
      break;
    }

    case QEvent::MouseButtonRelease:
      m_resizing = false;
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
} // namespace CSD
