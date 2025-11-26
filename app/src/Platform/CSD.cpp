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
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"

#include <QCursor>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>
#include <QScreen>
#include <QSvgRenderer>

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

static constexpr int kIconSize = 18;
static constexpr int kButtonSize = 28;
static constexpr int kIconMargin = 10;
static constexpr int kResizeMargin = 8;
static constexpr int kButtonMargin = 4;
static constexpr int kButtonSpacing = 4;
static constexpr int kDefaultTitleBarHeight = 32;
static constexpr int kMaximizedTitleBarHeight = 28;

//------------------------------------------------------------------------------
// CSD_Titlebar Implementation
//------------------------------------------------------------------------------

/**
 * @brief Constructs a CSD_Titlebar.
 * @param parent The parent QQuickItem.
 *
 * Initializes member variables to default values and configures the item
 * to accept left mouse button events and hover events for interactive
 * window controls.
 */
CSD_Titlebar::CSD_Titlebar(QQuickItem *parent)
  : QQuickPaintedItem(parent)
  , m_title()
  , m_dragging(false)
  , m_windowActive(true)
  , m_hoveredButton(Button::None)
  , m_pressedButton(Button::None)
  , m_dragStartPos()
  , m_backgroundColor("#2d2d2d")
{
  setAcceptedMouseButtons(Qt::LeftButton);
  setAcceptHoverEvents(true);
  m_icon = QGuiApplication::windowIcon().pixmap(kIconSize, kIconSize);
}

/**
 * @brief Paints the title bar including background, icon, title text, and
 *        window control buttons.
 * @param painter The QPainter used for rendering.
 *
 * Renders the title bar with:
 * - A solid background color
 * - Application icon on the left
 * - Centered title text
 * - Minimize, maximize, and close buttons with hover/press states
 */
void CSD_Titlebar::paint(QPainter *painter)
{
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  // Draw background
  painter->fillRect(boundingRect(), m_backgroundColor);

  // Draw application icon
  const qreal iconY = (height() - kIconSize) / 2.0;
  if (!m_icon.isNull())
    painter->drawPixmap(QPointF(kIconMargin, iconY), m_icon);

  // Draw centered title text
  const QColor fgColor = foregroundColor();
  painter->setPen(m_windowActive ? fgColor : fgColor.darker(130));
  painter->setFont(Misc::CommonFonts::instance().boldUiFont());
  painter->drawText(boundingRect(), Qt::AlignCenter, m_title);

  // Draw window control buttons with SVG icons
  drawButton(painter, Button::Minimize,
             QStringLiteral(":/rcc/icons/miniwindow/minimize.svg"));
  drawButton(painter, Button::Maximize,
             (window() && (window()->windowStates() & Qt::WindowMaximized))
                 ? QStringLiteral(":/rcc/icons/miniwindow/restore.svg")
                 : QStringLiteral(":/rcc/icons/miniwindow/maximize.svg"));
  drawButton(painter, Button::Close,
             QStringLiteral(":/rcc/icons/miniwindow/close.svg"));
}

/**
 * @brief Returns the window title text.
 * @return The current title string.
 */
QString CSD_Titlebar::title() const
{
  return m_title;
}

/**
 * @brief Returns whether the parent window is currently active.
 * @return True if the window is active, false otherwise.
 */
bool CSD_Titlebar::windowActive() const
{
  return m_windowActive;
}

/**
 * @brief Returns the title bar background color.
 * @return The current background color.
 */
QColor CSD_Titlebar::backgroundColor() const
{
  return m_backgroundColor;
}

/**
 * @brief Calculates the appropriate foreground color based on background.
 * @return The color to use for text and icons.
 *
 * If the background color matches the theme's toolbar_top color, returns
 * the titlebar_text color from the theme. Otherwise, calculates whether
 * to use white or black based on the background's relative luminance
 * using the W3C contrast algorithm.
 */
QColor CSD_Titlebar::foregroundColor() const
{
  const auto &theme = Misc::ThemeManager::instance();
  const QColor toolbarTop = theme.getColor("toolbar_top");

  // If background matches toolbar_top, use theme's titlebar_text
  if (m_backgroundColor == toolbarTop)
    return theme.getColor("titlebar_text");

  // Otherwise calculate optimal contrast color using relative luminance
  // Formula: L = 0.2126 * R + 0.7152 * G + 0.0722 * B (sRGB)
  const qreal r = m_backgroundColor.redF();
  const qreal g = m_backgroundColor.greenF();
  const qreal b = m_backgroundColor.blueF();

  // Apply gamma correction for sRGB
  auto linearize = [](qreal c) {
    return c <= 0.03928 ? c / 12.92 : qPow((c + 0.055) / 1.055, 2.4);
  };
  const qreal luminance
      = 0.2126 * linearize(r) + 0.7152 * linearize(g) + 0.0722 * linearize(b);

  // Use white text on dark backgrounds, black on light
  // Threshold of 0.179 provides ~4.5:1 contrast ratio (WCAG AA)
  return luminance > 0.179 ? QColor("#000000") : QColor("#ffffff");
}

/**
 * @brief Sets the window active state and triggers a repaint.
 * @param active The new active state.
 *
 * When the window becomes inactive, the title bar is rendered with
 * dimmed colors to provide visual feedback.
 */
void CSD_Titlebar::setWindowActive(bool active)
{
  if (m_windowActive != active)
  {
    m_windowActive = active;
    Q_EMIT windowActiveChanged();
    update();
  }
}

/**
 * @brief Sets the title bar text.
 * @param title The new title string.
 */
void CSD_Titlebar::setTitle(const QString &title)
{
  if (m_title != title)
  {
    m_title = title;
    Q_EMIT titleChanged();
    update();
  }
}

/**
 * @brief Sets the title bar background color.
 * @param color The new background color.
 */
void CSD_Titlebar::setBackgroundColor(const QColor &color)
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
 * @param button The button to get the rectangle for.
 * @return The button's bounding rectangle in item coordinates.
 */
QRectF CSD_Titlebar::buttonRect(Button button) const
{
  const qreal y = (height() - kButtonSize) / 2.0;
  const qreal closeX = width() - kButtonMargin - kButtonSize;

  switch (button)
  {
    case Button::Close:
      return QRectF(closeX, y, kButtonSize, kButtonSize);
    case Button::Maximize:
      return QRectF(closeX - kButtonSize - kButtonSpacing, y, kButtonSize,
                    kButtonSize);
    case Button::Minimize:
      return QRectF(closeX - 2 * (kButtonSize + kButtonSpacing), y, kButtonSize,
                    kButtonSize);
    default:
      return QRectF();
  }
}

/**
 * @brief Determines which button (if any) is at the given position.
 * @param pos The position to test in item coordinates.
 * @return The button at the position, or Button::None if no button.
 */
CSD_Titlebar::Button CSD_Titlebar::buttonAt(const QPointF &pos) const
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
 * @brief Helper function to draw a window control button with an SVG icon.
 * @param painter The painter to use.
 * @param button The button type to draw.
 * @param svgPath The resource path to the SVG icon.
 *
 * Renders the button with a rounded rectangle background on hover/press,
 * and draws the SVG icon centered within the button bounds. The icon
 * color is determined by the current foreground color.
 */
void CSD_Titlebar::drawButton(QPainter *painter, Button button,
                              const QString &svgPath)
{
  // Get button location & state
  const QRectF rect = buttonRect(button);
  const bool hovered = (m_hoveredButton == button);
  const bool pressed = (m_pressedButton == button);

  // Determine icon color
  QColor iconColor = foregroundColor();
  if (!m_windowActive)
    iconColor = iconColor.darker(130);
  else if (hovered || pressed)
  {
    iconColor = Misc::ThemeManager::instance().getColor("highlight");
    if (pressed)
      iconColor = iconColor.darker(80);
  }

  // Load and render SVG with color replacement
  QSvgRenderer renderer(svgPath);
  if (!renderer.isValid())
    return;

  // Calculate icon bounds (centered within button, slightly smaller)
  const qreal iconSize = kButtonSize * 0.5;
  const QRectF iconRect(rect.center().x() - iconSize / 2,
                        rect.center().y() - iconSize / 2, iconSize, iconSize);

  // Render SVG to a pixmap so we can colorize it
  const qreal dpr = painter->device()->devicePixelRatio();
  QPixmap pixmap(QSize(iconSize * dpr, iconSize * dpr));
  pixmap.setDevicePixelRatio(dpr);
  pixmap.fill(Qt::transparent);
  QPainter svgPainter(&pixmap);
  renderer.render(&svgPainter, QRectF(0, 0, iconSize, iconSize));
  svgPainter.end();

  // Apply color by compositing
  QPixmap colorized(pixmap.size());
  colorized.setDevicePixelRatio(dpr);
  colorized.fill(Qt::transparent);
  QPainter colorPainter(&colorized);
  colorPainter.drawPixmap(0, 0, pixmap);
  colorPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  colorPainter.fillRect(colorized.rect(), iconColor);
  colorPainter.end();

  // Draw the colorized icon
  painter->drawPixmap(iconRect.topLeft(), colorized);
}

/**
 * @brief Handles mouse move events for window dragging.
 * @param event The mouse event.
 *
 * When dragging is active, moves the parent window to follow the cursor.
 * If the window is maximized, it will be restored to normal state before
 * dragging begins, with the cursor position maintained relative to the
 * window width.
 */
void CSD_Titlebar::mouseMoveEvent(QMouseEvent *event)
{
  if (m_dragging && window())
  {
    auto *win = window();

    if (win->windowStates() & Qt::WindowMaximized)
    {
      const qreal relativeX = event->position().x() / width();
      win->showNormal();
      const QPointF globalPos = event->globalPosition();
      win->setX(static_cast<int>(globalPos.x() - win->width() * relativeX));
      win->setY(static_cast<int>(globalPos.y() - height() / 2));
      m_dragStartPos = globalPos;
    }

    else
    {
      const QPointF delta = event->globalPosition() - m_dragStartPos;
      win->setX(static_cast<int>(win->x() + delta.x()));
      win->setY(static_cast<int>(win->y() + delta.y()));
      m_dragStartPos = event->globalPosition();
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
void CSD_Titlebar::hoverMoveEvent(QHoverEvent *event)
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
 */
void CSD_Titlebar::hoverLeaveEvent(QHoverEvent *event)
{
  Q_UNUSED(event)
  if (m_hoveredButton != Button::None)
  {
    m_hoveredButton = Button::None;
    update();
  }
}

/**
 * @brief Handles mouse press events for button clicks and drag initiation.
 * @param event The mouse event.
 *
 * If a button is pressed, records the pressed state. Otherwise, initiates
 * window dragging by recording the start position.
 */
void CSD_Titlebar::mousePressEvent(QMouseEvent *event)
{
  m_pressedButton = buttonAt(event->position());

  if (m_pressedButton != Button::None)
  {
    update();
    event->accept();
    return;
  }

  m_dragStartPos = event->globalPosition();
  m_dragging = true;
  event->accept();
}

/**
 * @brief Handles mouse release events to complete button clicks or dragging.
 * @param event The mouse event.
 *
 * If a button was pressed and released on the same button, emits the
 * corresponding signal (minimizeClicked, maximizeClicked, or closeClicked).
 */
void CSD_Titlebar::mouseReleaseEvent(QMouseEvent *event)
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

  m_hoveredButton = Button::None;
  m_pressedButton = Button::None;
  m_dragging = false;
  update();
  event->accept();
}

/**
 * @brief Handles double-click events to toggle window maximized state.
 * @param event The mouse event.
 *
 * Double-clicking on the title bar (not on a button) toggles the window
 * between maximized and normal states.
 */
void CSD_Titlebar::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (buttonAt(event->position()) == Button::None)
    Q_EMIT maximizeClicked();

  event->accept();
}

//------------------------------------------------------------------------------
// UnixWindowDecorator Implementation
//------------------------------------------------------------------------------

/**
 * @brief Constructs a CSD_Window for the given window.
 * @param window The QWindow to decorate.
 * @param color Optional initial color for the title bar.
 * @param parent The parent QObject.
 *
 * Makes the window frameless, installs an event filter for resize handling,
 * creates the title bar item, and connects to theme change signals.
 */
CSD_Window::CSD_Window(QWindow *window, const QString &color, QObject *parent)
  : QObject(parent)
  , m_resizing(false)
  , m_color(color)
  , m_resizeEdge(ResizeEdge::None)
  , m_titleBar(nullptr)
  , m_window(window)
{
  if (!m_window)
    return;

  // Make window frameless for CSD
  m_window->setFlags(m_window->flags() | Qt::FramelessWindowHint);

  // Install event filter for resize edge detection
  m_window->installEventFilter(this);

  // Create and setup title bar
  setupTitleBar();

  // Update geometry when window state changes
  connect(m_window, &QWindow::windowStateChanged, this, [this]() {
    updateTitleBarGeometry();
    if (m_titleBar)
      m_titleBar->update();
  });

  // Connect to theme manager for automatic theme updates
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &CSD_Window::updateTheme);

  // Apply initial theme
  updateTheme();
}

/**
 * @brief Destructor. Removes the event filter from the window.
 */
CSD_Window::~CSD_Window()
{
  if (m_window)
    m_window->removeEventFilter(this);
}

/**
 * @brief Returns the decorated window.
 * @return Pointer to the QWindow being decorated.
 */
QWindow *CSD_Window::window() const
{
  return m_window;
}

/**
 * @brief Returns the current title bar height.
 * @return The height in pixels, adjusted for maximized state.
 *
 * Returns a smaller height when the window is maximized, following
 * common desktop environment conventions.
 */
int CSD_Window::titleBarHeight() const
{
  if (!m_window)
    return kDefaultTitleBarHeight;

  if (m_window->windowStates() & Qt::WindowMaximized)
    return kMaximizedTitleBarHeight;

  return kDefaultTitleBarHeight;
}

/**
 * @brief Returns the title bar item.
 * @return Pointer to the CSD_Titlebar used for rendering.
 */
CSD_Titlebar *CSD_Window::titleBar() const
{
  return m_titleBar;
}

/**
 * @brief Updates the title bar color from theme or custom color.
 *
 * If a custom color was set via setColor(), uses that color.
 * Otherwise, retrieves the toolbar_top color from the ThemeManager.
 */
void CSD_Window::updateTheme()
{
  if (!m_titleBar)
    return;

  QString color = m_color;
  if (color.isEmpty())
  {
    const auto &colors = Misc::ThemeManager::instance().colors();
    color = colors.value("toolbar_top").toString();
  }

  m_titleBar->setBackgroundColor(QColor(color));
}

/**
 * @brief Sets a custom color for the title bar.
 * @param color The color as a hex string (e.g., "#2d2d2d").
 *
 * Setting an empty string will cause the title bar to use the
 * theme manager's toolbar_top color.
 */
void CSD_Window::setColor(const QString &color)
{
  m_color = color;
  updateTheme();
}

/**
 * @brief Creates and configures the CSD_Titlebar.
 *
 * Creates the title bar as a child of the window's content item,
 * connects button signals to window actions, and sets up tracking
 * for window active state and title changes.
 */
void CSD_Window::setupTitleBar()
{
  auto *quickWindow = qobject_cast<QQuickWindow *>(m_window.data());
  if (!quickWindow)
    return;

  // Create title bar as child of content item
  m_titleBar = new CSD_Titlebar(quickWindow->contentItem());
  m_titleBar->setZ(999999);

  // Connect window control buttons
  connect(m_titleBar, &CSD_Titlebar::minimizeClicked, this,
          [this]() { m_window->showMinimized(); });
  connect(m_titleBar, &CSD_Titlebar::maximizeClicked, this, [this]() {
    if (m_window->windowStates() & Qt::WindowMaximized)
      m_window->showNormal();
    else
      m_window->showMaximized();
  });
  connect(m_titleBar, &CSD_Titlebar::closeClicked, this,
          [this]() { m_window->close(); });

  // Track window active state
  connect(m_window, &QWindow::activeChanged, this,
          [this]() { m_titleBar->setWindowActive(m_window->isActive()); });

  // Track window title changes
  m_titleBar->setTitle(m_window->title());
  connect(m_window, &QWindow::windowTitleChanged, m_titleBar,
          &CSD_Titlebar::setTitle);

  // Update geometry when window resizes
  connect(quickWindow, &QQuickWindow::widthChanged, this,
          &CSD_Window::updateTitleBarGeometry);

  // Set initial geometry
  updateTitleBarGeometry();
}

/**
 * @brief Updates the title bar position and size to match the window.
 *
 * Positions the title bar at the top of the window and stretches it
 * to fill the window width.
 */
void CSD_Window::updateTitleBarGeometry()
{
  if (!m_titleBar || !m_window)
    return;

  m_titleBar->setX(0);
  m_titleBar->setY(0);
  m_titleBar->setWidth(m_window->width());
  m_titleBar->setHeight(titleBarHeight());
}

/**
 * @brief Determines the resize edge at the given position.
 * @param pos The position in window coordinates.
 * @return The ResizeEdge flags indicating which edges are under the cursor.
 *
 * Returns ResizeEdge::None if the window is maximized or the position
 * is not within the resize margin of any edge.
 */
CSD_Window::ResizeEdge CSD_Window::edgeAt(const QPointF &pos) const
{
  if (!m_window || (m_window->windowStates() & Qt::WindowMaximized))
    return ResizeEdge::None;

  const int w = m_window->width();
  const int h = m_window->height();

  int edge = static_cast<int>(ResizeEdge::None);

  if (pos.x() < kResizeMargin)
    edge |= static_cast<int>(ResizeEdge::Left);
  else if (pos.x() >= w - kResizeMargin)
    edge |= static_cast<int>(ResizeEdge::Right);

  if (pos.y() < kResizeMargin)
    edge |= static_cast<int>(ResizeEdge::Top);
  else if (pos.y() >= h - kResizeMargin)
    edge |= static_cast<int>(ResizeEdge::Bottom);

  return static_cast<ResizeEdge>(edge);
}

/**
 * @brief Returns the appropriate cursor shape for a resize edge.
 * @param edge The resize edge.
 * @return The Qt::CursorShape to display.
 */
Qt::CursorShape CSD_Window::cursorForEdge(ResizeEdge edge) const
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
Qt::Edges CSD_Window::qtEdgesFromResizeEdge(ResizeEdge edge) const
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
 * @brief Event filter for handling window resize interactions.
 * @param watched The object being watched (the window).
 * @param event The event to process.
 * @return True if the event was handled, false otherwise.
 *
 * Handles:
 * - MouseMove: Updates cursor shape based on edge proximity
 * - MouseButtonPress: Initiates system resize when clicking on edges
 * - MouseButtonRelease: Resets resize state
 * - Leave: Resets cursor when mouse leaves window
 */
bool CSD_Window::eventFilter(QObject *watched, QEvent *event)
{
  if (watched != m_window)
    return false;

  switch (event->type())
  {
    case QEvent::MouseMove: {
      auto *mouseEvent = static_cast<QMouseEvent *>(event);
      const ResizeEdge edge = edgeAt(mouseEvent->position());

      if (!m_resizing)
        m_window->setCursor(cursorForEdge(edge));

      break;
    }

    case QEvent::MouseButtonPress: {
      auto *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton)
      {
        m_resizeEdge = edgeAt(mouseEvent->position());
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

    case QEvent::MouseButtonRelease: {
      m_resizing = false;
      m_resizeEdge = ResizeEdge::None;
      break;
    }

    case QEvent::Leave: {
      if (!m_resizing)
        m_window->setCursor(Qt::ArrowCursor);

      break;
    }

    default:
      break;
  }

  return false;
}
