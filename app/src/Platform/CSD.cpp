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
  setAcceptedMouseButtons(Qt::LeftButton);
  setAcceptHoverEvents(true);
  setFillColor(Qt::transparent);
  setOpaquePainting(false);

  // Load application icon from rcc:/logo/icon.svg
  const auto &icon = QGuiApplication::windowIcon();
  if (!icon.isNull())
    m_icon = icon.pixmap(CSD::IconSize, CSD::IconSize);
}

void Titlebar::paint(QPainter *painter)
{
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  const bool maximized
      = window() && (window()->windowStates() & Qt::WindowMaximized);
  const qreal radius = maximized ? 0 : CSD::CornerRadius;
  const QRectF rect = boundingRect();

  // Draw background
  if (radius > 0)
  {
    // Rounded top corners only
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    // Start at bottom-left, go clockwise
    path.moveTo(rect.left(), rect.bottom());
    path.lineTo(rect.left(), rect.top() + radius);
    path.arcTo(QRectF(rect.left(), rect.top(), radius * 2, radius * 2), 180,
               -90);
    path.lineTo(rect.right() - radius, rect.top());
    path.arcTo(
        QRectF(rect.right() - radius * 2, rect.top(), radius * 2, radius * 2),
        90, -90);
    path.lineTo(rect.right(), rect.bottom());
    path.closeSubpath();

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_backgroundColor);
    painter->drawPath(path);
  }
  else
  {
    painter->fillRect(rect, m_backgroundColor);
  }

  // Application icon
  if (!m_icon.isNull())
  {
    const qreal y = (height() - CSD::IconSize) / 2.0;
    painter->drawPixmap(QPointF(CSD::IconMargin, y), m_icon);
  }

  // Title text (centered)
  const QColor fg
      = m_windowActive ? foregroundColor() : foregroundColor().darker(130);
  painter->setPen(fg);
  painter->setFont(Misc::CommonFonts::instance().boldUiFont());
  painter->drawText(rect, Qt::AlignCenter, m_title);

  // Window control buttons
  const QString minimizeSvg = QStringLiteral(":/rcc/icons/csd/minimize.svg");
  const QString maximizeSvg
      = maximized ? QStringLiteral(":/rcc/icons/csd/restore.svg")
                  : QStringLiteral(":/rcc/icons/csd/maximize.svg");
  const QString closeSvg = QStringLiteral(":/rcc/icons/csd/close.svg");

  drawButton(painter, Button::Minimize, minimizeSvg);
  drawButton(painter, Button::Maximize, maximizeSvg);
  drawButton(painter, Button::Close, closeSvg);
}

QString Titlebar::title() const
{
  return m_title;
}

bool Titlebar::windowActive() const
{
  return m_windowActive;
}

QColor Titlebar::backgroundColor() const
{
  return m_backgroundColor;
}

/**
 * @brief Calculates the foreground color based on background luminance.
 * @return Appropriate text color for readability.
 */
QColor Titlebar::foregroundColor() const
{
  const auto &theme = Misc::ThemeManager::instance();
  const QColor toolbarTop = theme.getColor(QStringLiteral("toolbar_top"));

  // Use theme color if background matches toolbar
  if (m_backgroundColor == toolbarTop)
    return theme.getColor(QStringLiteral("titlebar_text"));

  // Calculate relative luminance (W3C formula)
  auto linearize = [](qreal c) {
    return c <= 0.03928 ? c / 12.92 : qPow((c + 0.055) / 1.055, 2.4);
  };

  const qreal luminance = 0.2126 * linearize(m_backgroundColor.redF())
                          + 0.7152 * linearize(m_backgroundColor.greenF())
                          + 0.0722 * linearize(m_backgroundColor.blueF());

  // WCAG AA contrast threshold
  return luminance > 0.179 ? QColor(Qt::black) : QColor(Qt::white);
}

void Titlebar::setTitle(const QString &title)
{
  if (m_title != title)
  {
    m_title = title;
    Q_EMIT titleChanged();
    update();
  }
}

void Titlebar::setWindowActive(bool active)
{
  if (m_windowActive != active)
  {
    m_windowActive = active;
    Q_EMIT windowActiveChanged();
    update();
  }
}

void Titlebar::setBackgroundColor(const QColor &color)
{
  if (m_backgroundColor != color)
  {
    m_backgroundColor = color;
    Q_EMIT backgroundColorChanged();
    update();
  }
}

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

Titlebar::Button Titlebar::buttonAt(const QPointF &pos) const
{
  if (buttonRect(Button::Close).contains(pos))
    return Button::Close;
  if (buttonRect(Button::Maximize).contains(pos))
    return Button::Maximize;
  if (buttonRect(Button::Minimize).contains(pos))
    return Button::Minimize;
  return Button::None;
}

/**
 * @brief Draws a window control button with SVG icon.
 * @param painter The painter to use.
 * @param button The button type.
 * @param svgPath Path to the SVG icon resource.
 */
void Titlebar::drawButton(QPainter *painter, Button button,
                          const QString &svgPath)
{
  const QRectF rect = buttonRect(button);
  const bool hovered = (m_hoveredButton == button);
  const bool pressed = (m_pressedButton == button);

  // Determine icon color
  QColor iconColor = foregroundColor();
  if (!m_windowActive)
    iconColor = iconColor.darker(130);
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

  // Render and colorize
  const qreal iconSize = CSD::ButtonSize * 0.5;
  const QRectF iconRect(rect.center().x() - iconSize / 2,
                        rect.center().y() - iconSize / 2, iconSize, iconSize);

  const qreal dpr = painter->device()->devicePixelRatio();
  const QSize pixelSize(qRound(iconSize * dpr), qRound(iconSize * dpr));

  QPixmap pixmap(pixelSize);
  pixmap.setDevicePixelRatio(dpr);
  pixmap.fill(Qt::transparent);

  QPainter svgPainter(&pixmap);
  renderer.render(&svgPainter, QRectF(0, 0, iconSize, iconSize));
  svgPainter.end();

  // Apply color
  QPixmap colorized(pixelSize);
  colorized.setDevicePixelRatio(dpr);
  colorized.fill(Qt::transparent);

  QPainter colorPainter(&colorized);
  colorPainter.drawPixmap(0, 0, pixmap);
  colorPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  colorPainter.fillRect(colorized.rect(), iconColor);
  colorPainter.end();

  painter->drawPixmap(iconRect.topLeft(), colorized);
}

void Titlebar::mouseUngrabEvent()
{
  m_hoveredButton = Button::None;
  m_pressedButton = Button::None;
  m_dragging = false;
  update();
}

void Titlebar::mousePressEvent(QMouseEvent *event)
{
  m_pressedButton = buttonAt(event->position());

  if (m_pressedButton != Button::None)
  {
    update();
    event->accept();
    return;
  }

  // Mark as potential drag, actual move starts in mouseMoveEvent
  m_dragging = true;
  event->accept();
}

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

void Titlebar::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_dragging || !window())
  {
    event->accept();
    return;
  }

  auto *win = window();
  m_dragging = false;

  // Handle dragging from maximized state
  if (win->windowStates() & Qt::WindowMaximized)
  {
    const qreal relativeX = event->position().x() / width();
    const QPointF globalPos = event->globalPosition();

    win->showNormal();
    win->setPosition(qRound(globalPos.x() - win->width() * relativeX),
                     qRound(globalPos.y() - height() / 2));
  }

  // Hand off to native system move
  win->startSystemMove();
  event->accept();
}

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

void Titlebar::hoverMoveEvent(QHoverEvent *event)
{
  const Button newHovered = buttonAt(event->position());
  if (newHovered != m_hoveredButton)
  {
    m_hoveredButton = newHovered;
    update();
  }
}

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

void Frame::paint(QPainter *painter)
{
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  const int r = m_shadowEnabled ? m_shadowRadius : 0;
  const int cr = CSD::CornerRadius;
  const int cornerTileSize = r + cr;
  const QRectF content(r, r, width() - 2 * r, height() - 2 * r);

  // Draw shadow
  if (m_shadowEnabled && !m_shadowCorner.isNull() && r > 0)
  {
    // Four corner tiles (each tile is shadowRadius + cornerRadius in size)
    // Top-left
    painter->drawImage(0, 0, m_shadowCorner);

    // Top-right
    painter->drawImage(QPointF(width() - cornerTileSize, 0),
                       m_shadowCorner.flipped(Qt::Horizontal));

    // Bottom-left
    painter->drawImage(QPointF(0, height() - cornerTileSize),
                       m_shadowCorner.flipped(Qt::Vertical));

    // Bottom-right
    painter->drawImage(
        QPointF(width() - cornerTileSize, height() - cornerTileSize),
        m_shadowCorner.flipped(Qt::Horizontal | Qt::Vertical));

    // Four edge strips (between corner tiles)
    if (!m_shadowEdge.isNull())
    {
      const qreal hEdgeLen = width() - 2 * cornerTileSize;
      const qreal vEdgeLen = height() - 2 * cornerTileSize;

      // Top and bottom edges
      if (hEdgeLen > 0)
      {
        // Top edge
        painter->drawImage(QRectF(cornerTileSize, 0, hEdgeLen, r), m_shadowEdge,
                           QRectF(0, 0, 1, r));

        // Bottom edge
        painter->drawImage(QRectF(cornerTileSize, height() - r, hEdgeLen, r),
                           m_shadowEdge.flipped(Qt::Vertical),
                           QRectF(0, 0, 1, r));
      }

      // Left and right edges
      if (vEdgeLen > 0)
      {
        // Create horizontal gradient for side edges
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

        // Left edge (gradient goes right-to-left, so flip)
        painter->drawImage(QRectF(0, cornerTileSize, r, vEdgeLen),
                           hEdge.flipped(Qt::Horizontal), QRectF(0, 0, r, 1));

        // Right edge (gradient goes left-to-right)
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

int Frame::shadowRadius() const
{
  return m_shadowRadius;
}

bool Frame::shadowEnabled() const
{
  return m_shadowEnabled;
}

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
 * @brief Regenerates shadow tile images.
 */
void Frame::regenerateShadow()
{
  if (m_shadowRadius <= 0)
  {
    m_shadowCorner = QImage();
    m_shadowEdge = QImage();
    return;
  }

  m_shadowCorner = generateShadowCorner(m_shadowRadius, m_shadowRadius);

  // Edge tile (1px wide)
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

QImage Frame::generateShadowCorner(int shadowSize, int radius)
{
  Q_UNUSED(radius)

  const int cr = CSD::CornerRadius;
  const int tileSize = shadowSize + cr;

  QImage tile(tileSize, tileSize, QImage::Format_ARGB32_Premultiplied);
  tile.fill(Qt::transparent);

  // The content's rounded corner arc is centered at (tileSize, tileSize)
  // which is the bottom-right corner of this tile (for top-left window corner)
  const qreal arcCenterX = static_cast<qreal>(tileSize);
  const qreal arcCenterY = static_cast<qreal>(tileSize);

  for (int y = 0; y < tileSize; ++y)
  {
    for (int x = 0; x < tileSize; ++x)
    {
      // Distance from this pixel to the arc center
      const qreal dx = arcCenterX - x - 0.5;
      const qreal dy = arcCenterY - y - 0.5;
      const qreal distFromArcCenter = qSqrt(dx * dx + dy * dy);

      // Distance to the content edge (the arc)
      // Positive = outside content (in shadow region)
      // Negative or zero = inside content (no shadow)
      const qreal distToContent = distFromArcCenter - cr;

      if (distToContent > 0)
      {
        // Normalize distance: 0 at content edge, 1 at outer shadow edge
        const qreal normalizedDist
            = qMin(distToContent / static_cast<qreal>(shadowSize), 1.0);

        // Smoothstep falloff for natural shadow appearance
        qreal alpha = 1.0 - normalizedDist;
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
 * @param window The QWindow to decorate.
 * @param color Optional custom title bar color.
 * @param parent Parent QObject.
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
    // Only update if it's a user change, not our own adjustment
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

Window::~Window()
{
  if (m_window)
  {
    m_window->removeEventFilter(this);
    if (auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data()))
      quickWindow->contentItem()->removeEventFilter(this);
  }
}

QWindow *Window::window() const
{
  return m_window;
}

Frame *Window::frame() const
{
  return m_frame;
}

Titlebar *Window::titleBar() const
{
  return m_titleBar;
}

int Window::shadowMargin() const
{
  if (!m_window || (m_window->windowStates() & Qt::WindowMaximized))
    return 0;

  return CSD::ShadowRadius;
}

int Window::titleBarHeight() const
{
  if (!m_window)
    return CSD::TitleBarHeight;

  return (m_window->windowStates() & Qt::WindowMaximized)
             ? CSD::TitleBarHeightMaximized
             : CSD::TitleBarHeight;
}

void Window::setColor(const QString &color)
{
  m_color = color;
  updateTheme();
}

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

void Window::setupTitleBar()
{
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return;

  m_titleBar = new Titlebar(quickWindow->contentItem());
  m_titleBar->setZ(999999);

  // Window controls
  connect(m_titleBar, &Titlebar::minimizeClicked, this,
          [this]() { m_window->showMinimized(); });
  connect(m_titleBar, &Titlebar::maximizeClicked, this, [this]() {
    if (m_window->windowStates() & Qt::WindowMaximized)
      m_window->showNormal();
    else
      m_window->showMaximized();
  });
  connect(m_titleBar, &Titlebar::closeClicked, this,
          [this]() { m_window->close(); });

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
 */
void Window::updateMinimumSize()
{
  if (!m_window)
    return;

  const int margin = shadowMargin();
  const int tbHeight = titleBarHeight();

  // Calculate minimum window size from content minimum size
  const int minWidth = qMax(0, m_contentMinimumSize.width()) + 2 * margin;
  const int minHeight
      = qMax(0, m_contentMinimumSize.height()) + 2 * margin + tbHeight;

  // Ensure minimum size can at least fit the title bar buttons
  const int minTitleBarWidth = 3 * (CSD::ButtonSize + CSD::ButtonSpacing)
                               + 2 * CSD::ButtonMargin + CSD::IconSize
                               + 2 * CSD::IconMargin;

  m_window->setMinimumSize(QSize(qMax(minWidth, minTitleBarWidth + 2 * margin),
                                 qMax(minHeight, tbHeight + 2 * margin)));
}

void Window::updateFrameGeometry()
{
  if (!m_frame || !m_window)
    return;

  m_frame->setPosition(QPointF(0, 0));
  m_frame->setSize(QSizeF(m_window->width(), m_window->height()));
}

/**
 * @brief Creates content container and reparents QML content.
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

  // Reparent existing children
  const auto children = root->childItems();
  for (QQuickItem *child : children)
    reparentChildToContainer(child);

  // Watch for new children
  root->installEventFilter(this);

  connect(quickWindow, &QQuickWindow::widthChanged, this,
          &Window::updateContentContainerGeometry);
  connect(quickWindow, &QQuickWindow::heightChanged, this,
          &Window::updateContentContainerGeometry);

  updateContentContainerGeometry();
}

void Window::updateTitleBarGeometry()
{
  if (!m_titleBar || !m_window)
    return;

  const int margin = shadowMargin();
  m_titleBar->setPosition(QPointF(margin, margin));
  m_titleBar->setSize(QSizeF(m_window->width() - 2 * margin, titleBarHeight()));
}

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
 */
void Window::reparentChildToContainer(QQuickItem *child)
{
  if (!child || !m_contentContainer)
    return;

  // Skip our own items
  if (child == m_frame || child == m_titleBar || child == m_contentContainer)
    return;

  // Skip if already in container
  if (child->parentItem() == m_contentContainer)
    return;

  child->setParentItem(m_contentContainer);
}

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
 */
bool Window::eventFilter(QObject *watched, QEvent *event)
{
  // Handle new children added to content item
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

  // Handle window events
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
