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
// Constants
//------------------------------------------------------------------------------

constexpr int IconSize = 18;
constexpr int IconMargin = 10;
constexpr int ButtonSize = 28;
constexpr int ResizeMargin = 8;
constexpr int ButtonMargin = 4;
constexpr int CornerRadius = 10;
constexpr int ShadowRadius = 24;
constexpr int ButtonSpacing = 4;
constexpr int TitleBarHeight = 32;
constexpr int TitleBarHeightMaximized = 28;

constexpr qreal ShadowOpacity = 0.15;

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

  // Load application icon if possible
  const auto &icon = QGuiApplication::windowIcon();
  if (!icon.isNull())
    m_icon = icon.pixmap(CSD::IconSize, CSD::IconSize);
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
  // Stop if window pointer is invalid
  if (!window())
    return;

  // Set render hinds
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  // Get window area & state
  const QRectF rect = boundingRect();
  const bool maximized = window()->windowStates() & Qt::WindowMaximized;
  const qreal radius = maximized ? 0 : CSD::CornerRadius;

  // Draw rounded titlebar if window is not maximized
  if (radius > 0)
  {
    // clang-format off
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.moveTo(rect.left(), rect.bottom());
    path.lineTo(rect.left(), rect.top() + radius);
    path.arcTo(QRectF(rect.left(), rect.top(), radius * 2, radius * 2), 180, -90);
    path.lineTo(rect.right() - radius, rect.top());
    path.arcTo(QRectF(rect.right() - radius * 2, rect.top(), radius * 2, radius * 2),90, -90);
    path.lineTo(rect.right(), rect.bottom());
    path.closeSubpath();
    // clang-format on

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_backgroundColor);
    painter->drawPath(path);
  }

  // Draw a simple rectangle if window is maximized
  else
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
  painter->setFont(Misc::CommonFonts::instance().boldUiFont());
  painter->drawText(rect, Qt::AlignCenter, title());

  // Set path for window button icons
  // clang-format off
  const QString closeSvg = QStringLiteral(":/rcc/icons/csd/close.svg");
  const QString minimizeSvg = QStringLiteral(":/rcc/icons/csd/minimize.svg");
  const QString maximizeSvg = maximized ? QStringLiteral(":/rcc/icons/csd/restore.svg")
                                        : QStringLiteral(":/rcc/icons/csd/maximize.svg");
  // clang-format on

  // Draw window buttons
  drawButton(painter, Button::Close, closeSvg);
  drawButton(painter, Button::Minimize, minimizeSvg);
  drawButton(painter, Button::Maximize, maximizeSvg);
}

/**
 * @brief Returns the window title text.
 * @return The current title string.
 */
QString Titlebar::title() const
{
  return m_title + " — Serial Studio";
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
  const qreal y = (height() - CSD::ButtonSize) / 2.0;
  const qreal closeX = width() - CSD::ButtonMargin - CSD::ButtonSize;

  switch (button)
  {
    case Button::Close:
      return {closeX, y, CSD::ButtonSize, CSD::ButtonSize};
    case Button::Maximize:
      return {closeX - CSD::ButtonSize - CSD::ButtonSpacing, y, CSD::ButtonSize,
              CSD::ButtonSize};
    case Button::Minimize:
      return {closeX - 2 * (CSD::ButtonSize + CSD::ButtonSpacing), y,
              CSD::ButtonSize, CSD::ButtonSize};
    default:
      return {};
  }
}

/**
 * @brief Determines which button (if any) is at the given position.
 * @param pos The position to test in item coordinates.
 * @return The button at the position, or Button::None if no button.
 */
Titlebar::Button Titlebar::buttonAt(const QPointF &pos) const
{
  if (buttonRect(Button::Close).contains(pos))
    return Button::Close;

  else if (buttonRect(Button::Maximize).contains(pos))
    return Button::Maximize;

  else if (buttonRect(Button::Minimize).contains(pos))
    return Button::Minimize;

  return Button::None;
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
  // Get button position & state
  const QRectF rect = buttonRect(button);
  const bool hovered = (m_hoveredButton == button);
  const bool pressed = (m_pressedButton == button);

  // Determine button color
  QColor iconColor = foregroundColor();
  if (!m_windowActive)
    iconColor = iconColor.darker(130);

  // Highlight the button if it is hovered or pressed
  else if (hovered || pressed)
  {
    iconColor
        = Misc::ThemeManager::instance().getColor(QStringLiteral("highlight"));
    if (pressed)
      iconColor = iconColor.darker(120);
  }

  // Load SVG
  QSvgRenderer renderer(svgPath);
  if (!renderer.isValid())
    return;

  // Obtain icon size for the button
  const qreal iconSize = CSD::ButtonSize * 0.5;
  const QRectF iconRect(rect.center().x() - iconSize / 2,
                        rect.center().y() - iconSize / 2, iconSize, iconSize);

  // Set pixel size
  const qreal dpr = painter->device()->devicePixelRatio();
  const QSize pixelSize(qRound(iconSize * dpr), qRound(iconSize * dpr));

  // Initialize target pixmap for button icon
  QPixmap pixmap(pixelSize);
  pixmap.setDevicePixelRatio(dpr);
  pixmap.fill(Qt::transparent);

  // Paint the SVG icon into the pixmap
  QPainter svgPainter(&pixmap);
  renderer.render(&svgPainter, QRectF(0, 0, iconSize, iconSize));
  svgPainter.end();

  // Initialize target pixmap for colorized button
  QPixmap colorized(pixelSize);
  colorized.setDevicePixelRatio(dpr);
  colorized.fill(Qt::transparent);

  // Colorize the button
  QPainter colorPainter(&colorized);
  colorPainter.drawPixmap(0, 0, pixmap);
  colorPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  colorPainter.fillRect(colorized.rect(), iconColor);
  colorPainter.end();

  // Draw the final pixmap
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

  if (win->windowStates() & Qt::WindowMaximized)
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
 * between maximized and normal states.
 */
void Titlebar::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (buttonAt(event->position()) == Button::None && window())
  {
    m_dragging = false;
    m_pressedButton = Button::None;
    m_hoveredButton = Button::None;

    if (window()->windowStates() & Qt::WindowMaximized)
      window()->showNormal();
    else
      window()->showMaximized();

    update();
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
 * - Corner shadow tiles at each corner (accounting for rounded content)
 * - Edge shadow strips between corners
 * - Rounded background fill matching the theme
 * - Border stroke around the content area
 *
 * When shadows are disabled (e.g., window maximized), only the background
 * and border are drawn without rounded corners.
 */
void Frame::paint(QPainter *painter)
{
  // Set painter rendering hints
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  // Get window size & corner tile size
  const int r = m_shadowEnabled ? m_shadowRadius : 0;
  const int cr = CSD::CornerRadius;
  const int cornerTileSize = r + cr;
  const QRectF content(r, r, width() - 2 * r, height() - 2 * r);

  // Draw shadow
  if (m_shadowEnabled && !m_shadowCorner.isNull() && r > 0)
  {
    painter->drawImage(0, 0, m_shadowCorner);
    painter->drawImage(QPointF(width() - cornerTileSize, 0),
                       m_shadowCorner.flipped(Qt::Horizontal));
    painter->drawImage(QPointF(0, height() - cornerTileSize),
                       m_shadowCorner.flipped(Qt::Vertical));
    painter->drawImage(
        QPointF(width() - cornerTileSize, height() - cornerTileSize),
        m_shadowCorner.flipped(Qt::Horizontal | Qt::Vertical));

    if (!m_shadowEdge.isNull())
    {
      const qreal hEdgeLen = width() - 2 * cornerTileSize;
      const qreal vEdgeLen = height() - 2 * cornerTileSize;

      if (hEdgeLen > 0)
      {
        painter->drawImage(QRectF(cornerTileSize, 0, hEdgeLen, r), m_shadowEdge,
                           QRectF(0, 0, 1, r));
        painter->drawImage(QRectF(cornerTileSize, height() - r, hEdgeLen, r),
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

        painter->drawImage(QRectF(0, cornerTileSize, r, vEdgeLen),
                           hEdge.flipped(Qt::Horizontal), QRectF(0, 0, r, 1));
        painter->drawImage(QRectF(width() - r, cornerTileSize, r, vEdgeLen),
                           hEdge, QRectF(0, 0, r, 1));
      }
    }
  }

  // Draw content background with rounded corners
  if (content.width() > 0 && content.height() > 0)
  {
    const auto &theme = Misc::ThemeManager::instance();
    const QColor bgColor = theme.getColor(QStringLiteral("toolbar_top"));
    const QColor borderColor = theme.getColor(QStringLiteral("border"));
    const qreal radius = m_shadowEnabled ? cr : 0;

    // Fill background
    painter->setPen(Qt::NoPen);
    painter->setBrush(bgColor);
    if (radius > 0)
      painter->drawRoundedRect(content, radius, radius);
    else
      painter->fillRect(content, bgColor);

    // Draw border
    painter->setPen(QPen(borderColor, 1));
    painter->setBrush(Qt::NoBrush);
    if (radius > 0)
      painter->drawRoundedRect(content.adjusted(0.5, 0.5, -0.5, -0.5), radius,
                               radius);
    else
      painter->drawRect(content.adjusted(0.5, 0.5, -0.5, -0.5));
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
 * @param shadowSize The shadow blur radius in pixels.
 * @param radius Unused parameter (corner radius is taken from
 * CSD::CornerRadius).
 * @return The generated corner tile image.
 *
 * Creates a tile of size (shadowSize + cornerRadius) that renders the shadow
 * around a rounded corner. The shadow follows the arc of the content's
 * rounded corner, creating a smooth curved shadow edge.
 *
 * Uses distance-from-arc calculation with smoothstep falloff to create
 * a natural-looking shadow that properly wraps around the corner.
 */
QImage Frame::generateShadowCorner(int size)
{
  const int cr = CSD::CornerRadius;
  const int tileSize = size + cr;

  QImage tile(tileSize, tileSize, QImage::Format_ARGB32_Premultiplied);
  tile.fill(Qt::transparent);

  const qreal arcCenterX = static_cast<qreal>(tileSize);
  const qreal arcCenterY = static_cast<qreal>(tileSize);

  for (int y = 0; y < tileSize; ++y)
  {
    for (int x = 0; x < tileSize; ++x)
    {
      const qreal dx = arcCenterX - x - 0.5;
      const qreal dy = arcCenterY - y - 0.5;
      const qreal distFromArcCenter = qSqrt(dx * dx + dy * dy);
      const qreal distToContent = distFromArcCenter - cr;

      if (distToContent > 0)
      {
        qreal alpha = 1.0 - qMin(distToContent / static_cast<qreal>(size), 1.0);
        alpha = alpha * alpha * (3.0 - 2.0 * alpha);
        alpha *= CSD::ShadowOpacity;
        tile.setPixelColor(x, y, QColor(0, 0, 0, qRound(alpha * 255)));
      }
    }
  }

  return tile;
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
  , m_color(color)
  , m_titleBar(nullptr)
  , m_resizeEdge(ResizeEdge::None)
  , m_contentMinimumSize(0, 0)
  , m_window(window)
  , m_contentContainer(nullptr)
{
  // Stop if window pointer is invalid
  if (!m_window)
    return;

  // Store original minimum size before we modify it
  m_contentMinimumSize = m_window->minimumSize();

  // Configure window for CSD
  m_window->setFlags(m_window->flags() | Qt::FramelessWindowHint);
  m_window->installEventFilter(this);

  // Setup components in order: frame -> container -> titlebar
  setupFrame();
  setupContentContainer();
  setupTitleBar();

  // Handle window state changes
  connect(m_window, &QWindow::windowStateChanged, this, [this]() {
    const bool maximized = m_window->windowStates() & Qt::WindowMaximized;
    if (m_frame)
      m_frame->setShadowEnabled(!maximized);

    updateFrameGeometry();
    updateContentContainerGeometry();
    updateTitleBarGeometry();
    updateMinimumSize();

    if (m_titleBar)
      m_titleBar->update();
  });

  // Track minimum size changes from QML/user
  connect(m_window, &QWindow::minimumWidthChanged, this, [this]() {
    const int expected = m_contentMinimumSize.width() + 2 * shadowMargin();
    if (m_window->minimumWidth() != expected)
    {
      m_contentMinimumSize.setWidth(m_window->minimumWidth()
                                    - 2 * shadowMargin());
      updateMinimumSize();
    }
  });

  connect(m_window, &QWindow::minimumHeightChanged, this, [this]() {
    const int expected
        = m_contentMinimumSize.height() + 2 * shadowMargin() + titleBarHeight();
    if (m_window->minimumHeight() != expected)
    {
      m_contentMinimumSize.setHeight(m_window->minimumHeight()
                                     - 2 * shadowMargin() - titleBarHeight());
      updateMinimumSize();
    }
  });

  // Theme updates
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Window::updateTheme);
  updateTheme();

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
 * @brief Returns the decorated window.
 * @return Pointer to the QWindow being decorated, or nullptr if invalid.
 */
QWindow *Window::window() const
{
  return m_window;
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
 * @brief Returns the title bar component.
 * @return Pointer to the Titlebar used for window controls.
 */
Titlebar *Window::titleBar() const
{
  return m_titleBar;
}

/**
 * @brief Returns the current shadow margin.
 * @return Shadow radius in pixels, or 0 if window is maximized.
 *
 * The shadow margin determines the offset between the window edge
 * and the visible content area.
 */
int Window::shadowMargin() const
{
  if (!m_window || (m_window->windowStates() & Qt::WindowMaximized))
    return 0;

  return CSD::ShadowRadius;
}

/**
 * @brief Returns the current title bar height.
 * @return Height in pixels, adjusted for maximized state.
 *
 * Returns a smaller height when maximized, following common
 * desktop environment conventions.
 */
int Window::titleBarHeight() const
{
  if (!m_window)
    return CSD::TitleBarHeight;

  return (m_window->windowStates() & Qt::WindowMaximized)
             ? CSD::TitleBarHeightMaximized
             : CSD::TitleBarHeight;
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
 * @brief Creates and configures the title bar.
 *
 * Creates the Titlebar component, connects window control signals
 * (minimize, maximize, close), and sets up tracking for window
 * title and active state changes. The title bar is placed at
 * high z-index to render above all content.
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
 * @brief Updates the window minimum size to account for CSD decorations.
 *
 * Adds shadow margins and title bar height to the content minimum size
 * to ensure the content area never becomes smaller than intended.
 * Also enforces a minimum width to fit title bar controls.
 */
void Window::updateMinimumSize()
{
  if (!m_window)
    return;

  const int margin = shadowMargin();
  const int tbHeight = titleBarHeight();
  const int minWidth = qMax(0, m_contentMinimumSize.width()) + 2 * margin;
  const int minHeight
      = qMax(0, m_contentMinimumSize.height()) + 2 * margin + tbHeight;
  const int minTitleBarWidth = 3 * (CSD::ButtonSize + CSD::ButtonSpacing)
                               + 2 * CSD::ButtonMargin + CSD::IconSize
                               + 2 * CSD::IconMargin;

  m_window->setMinimumSize(QSize(qMax(minWidth, minTitleBarWidth + 2 * margin),
                                 qMax(minHeight, tbHeight + 2 * margin)));
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
 * @brief Creates content container and reparents QML content.
 *
 * Creates an intermediate container item that holds all QML content,
 * automatically applying shadow margins and title bar offset.
 * Installs an event filter to reparent any newly added children.
 */
void Window::setupContentContainer()
{
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return;

  QQuickItem *root = quickWindow->contentItem();

  m_contentContainer = new QQuickItem(root);
  m_contentContainer->setZ(0);
  m_contentContainer->setClip(true);

  const auto children = root->childItems();
  for (QQuickItem *child : children)
    reparentChildToContainer(child);

  root->installEventFilter(this);
  connect(quickWindow, &QQuickWindow::widthChanged, this,
          &Window::updateContentContainerGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this,
          &Window::updateContentContainerGeometry);

  updateContentContainerGeometry();
}

/**
 * @brief Updates the title bar position and size.
 *
 * Positions the title bar at the top of the content area,
 * accounting for shadow margins.
 */
void Window::updateTitleBarGeometry()
{
  if (!m_titleBar || !m_window)
    return;

  const int margin = shadowMargin();
  m_titleBar->setPosition(QPointF(margin, margin));
  m_titleBar->setSize(QSizeF(m_window->width() - 2 * margin, titleBarHeight()));
}

/**
 * @brief Updates the content container position and size.
 *
 * Positions the container below the title bar, inset by shadow
 * margins on all sides.
 */
void Window::updateContentContainerGeometry()
{
  if (!m_contentContainer || !m_window)
    return;

  const int margin = shadowMargin();
  const int tbHeight = titleBarHeight();

  m_contentContainer->setPosition(QPointF(margin, margin + tbHeight));
  m_contentContainer->setSize(
      QSizeF(m_window->width() - 2 * margin,
             m_window->height() - 2 * margin - tbHeight));
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
 * Skips CSD components (frame, title bar, container) and items
 * already in the container. This ensures all user QML content
 * respects the CSD margins automatically.
 */
void Window::reparentChildToContainer(QQuickItem *child)
{
  if (!child || !m_contentContainer)
    return;
  else if (child == m_frame || child == m_titleBar
           || child == m_contentContainer)
    return;
  else if (child->parentItem() == m_contentContainer)
    return;

  child->setParentItem(m_contentContainer);
}

/**
 * @brief Determines the resize edge at the given position.
 * @param pos The position in window coordinates.
 * @return The ResizeEdge flags indicating which edges are under the cursor.
 *
 * Returns ResizeEdge::None if the window is maximized or the position
 * is not within the resize margin of any edge.
 */
Window::ResizeEdge Window::edgeAt(const QPointF &pos) const
{
  if (!m_window || (m_window->windowStates() & Qt::WindowMaximized))
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
