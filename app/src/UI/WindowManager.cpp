/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WindowManager.h"
#include "UI/Taskbar.h"

#include <QUrl>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>

/**
 * @brief Constructs the WindowManager singleton.
 */
UI::WindowManager::WindowManager(QQuickItem *parent)
  : QQuickItem(parent)
  , m_zCounter(1)
  , m_autoLayoutEnabled(true)
  , m_taskbar(nullptr)
  , m_dragWindow(nullptr)
  , m_resizeWindow(nullptr)
  , m_focusedWindow(nullptr)
{
  setEnabled(true);
  setAcceptHoverEvents(false);
  setFlag(ItemHasContents, false);
  setFiltersChildMouseEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);

  connect(this, &UI::WindowManager::widthChanged, this,
          &UI::WindowManager::triggerLayoutUpdate);
  connect(this, &UI::WindowManager::heightChanged, this,
          &UI::WindowManager::triggerLayoutUpdate);
}

/**
 * @brief Gets the current z-order counter.
 * @return Internal z-order counter value.
 */
int UI::WindowManager::zCounter() const
{
  return m_zCounter;
}

/**
 * @brief Returns whether automatic layout is enabled.
 *
 * If true, the window manager will automatically arrange windows
 * using the auto-layout system when changes occur.
 *
 * @return True if auto-layout is enabled, false otherwise.
 */
bool UI::WindowManager::autoLayoutEnabled() const
{
  return m_autoLayoutEnabled;
}

/**
 * @brief Returns the currently set background image path.
 * @return Background image URL as a QString.
 */
const QString &UI::WindowManager::backgroundImage() const
{
  return m_backgroundImage;
}

/**
 * @brief Retrieves the z-order for a given window item.
 * @param item Pointer to the QQuickItem representing the window.
 * @return Z-order value, or -1 if not registered.
 */
int UI::WindowManager::zOrder(QQuickItem *item) const
{
  if (m_windowZ.contains(item))
    return m_windowZ.value(item);

  return -1;
}

/**
 * @brief Retrieves the stored geometry of a given window.
 * @param item Pointer to the QQuickItem.
 * @return QRectF with the position and size of the window, empty if not found.
 */
QRect UI::WindowManager::geometry(QQuickItem *item) const
{
  if (m_windowGeometry.contains(item))
    return m_windowGeometry.value(item);

  return QRect(0, 0, 0, 0);
}

/**
 * @brief Clears all tracked windows, z-order, and geometry.
 *        Resets the z-order counter.
 */
void UI::WindowManager::clear()
{
  m_zCounter = 1;
  m_windowZ.clear();
  m_windows.clear();
  m_windowGeometry.clear();

  Q_EMIT zCounterChanged();
}

/**
 * @brief Loads the appropriate layout based on current settings.
 *
 * If auto-layout is enabled, calls autoLayout() to arrange windows using a
 * split-based tiling strategy. Otherwise, falls back to cascadeLayout(),
 * which stacks windows diagonally like classic desktop environments.
 *
 * This should be called after windows are added, removed, or their visibility
 * changes, to re-apply the user's preferred layout mode.
 */
void UI::WindowManager::loadLayout()
{
  if (autoLayoutEnabled())
    autoLayout();
  else
    cascadeLayout();
}

/**
 * @brief Automatically tiles visible windows into the available screen space.
 *
 * Arranges all visible QQuickItem windows using a recursive, split-based
 * layout. The layout alternates between vertical and horizontal splits based
 * on the area shape.
 *
 * Each window is resized and positioned to fit cleanly with spacing and
 * margins. Emits geometryChanged() for each updated window.
 *
 * Call this after adding, removing, or changing window visibility or size.
 */
void UI::WindowManager::autoLayout()
{
  // Set function constants
  const int margin = 8;
  const int spacing = 4;

  // Collect all currently visible and valid QQuickItems (windows)
  QList<QQuickItem *> visibleWindows;
  for (auto *win : std::as_const(m_windows))
  {
    if (win && win->state() != "closed" && win->state() != "minimized")
      visibleWindows.append(win);
  }

  // If there are no visible windows, there's nothing to layout
  if (visibleWindows.isEmpty())
    return;

  // Define the initial available space (dashboard area minus margin)
  QRect rootArea(margin, margin, width() - 2 * margin, height() - 2 * margin);

  // Recursive lambda to tile the given list of windows into the given area
  auto tileRecursive = [&](auto &&self, const QRect &area,
                           const QList<QQuickItem *> &windows) -> void {
    // No available windows, nothing to do
    if (windows.isEmpty())
      return;

    // Single window, fill the entire area
    if (windows.size() == 1)
    {
      QQuickItem *win = windows.first();
      QRect windowArea;
      if (win->state() == "normal")
        windowArea = area;
      else
        windowArea = QRect(x(), y(), width(), height());

      win->setX(windowArea.x());
      win->setY(windowArea.y());
      win->setWidth(windowArea.width());
      win->setHeight(windowArea.height());
      m_windowGeometry[win] = windowArea;
      Q_EMIT geometryChanged(win);
      return;
    }

    // Decide split direction based on current area's aspect ratio
    bool splitVertically = area.width() > area.height();

    // Split the window list into two halves
    int mid = windows.size() / 2;
    QList<QQuickItem *> firstHalf = windows.mid(0, mid);
    QList<QQuickItem *> secondHalf = windows.mid(mid);
    QRect firstArea, secondArea;

    // Vertical split, divide the area into left/right panels
    if (splitVertically)
    {
      qreal splitX = area.x() + area.width() / 2.0 - spacing / 2.0;
      firstArea = QRect(area.x(), area.y(), area.width() / 2.0 - spacing / 2.0,
                        area.height());
      secondArea = QRect(splitX + spacing, area.y(),
                         area.width() / 2.0 - spacing / 2.0, area.height());
    }

    // Horizontal split, divide the area into top/bottom panels
    else
    {
      qreal splitY = area.y() + area.height() / 2.0 - spacing / 2.0;
      firstArea = QRect(area.x(), area.y(), area.width(),
                        area.height() / 2.0 - spacing / 2.0);
      secondArea = QRect(area.x(), splitY + spacing, area.width(),
                         area.height() / 2.0 - spacing / 2.0);
    }

    // Recursively lay out each half of the window list in the sub-area
    self(self, firstArea, firstHalf);
    self(self, secondArea, secondHalf);
  };

  // Kick off the recursive tiling process
  tileRecursive(tileRecursive, rootArea, visibleWindows);
}

/**
 * @brief Cascades visible windows from top-left, offsetting each one.
 *
 * Starts cascading from the top-left corner and offsets each window diagonally.
 * If the cascade hits screen edges, starts a new cascade column.
 */
void UI::WindowManager::cascadeLayout()
{
  // Set function constants
  const int margin = 8;
  const int offsetStep = 32;

  // Collect all currently visible and valid QQuickItems (windows)
  QList<QQuickItem *> visibleWindows;
  for (auto *win : std::as_const(m_windows))
  {
    if (win && win->state() != "closed" && win->state() != "minimized")
      visibleWindows.append(win);
  }

  // If there are no visible windows, there's nothing to layout
  if (visibleWindows.isEmpty())
    return;

  // Set start and end points
  int startX = margin;
  int startY = margin;
  int maxRight = width() - margin;
  int maxBottom = height() - margin;

  // Initialize position trackers
  int x = startX;
  int y = startY;

  // Initialize cascade counter
  int cascadeCount = 0;

  // Cascade all the windows
  for (auto *win : visibleWindows)
  {
    // Validate pointer
    if (!win)
      continue;

    // Obtain minimum recommended window sizes
    qreal minWidth = win->property("implicitWidth").toReal();
    qreal minHeight = win->property("implicitHeight").toReal();

    // Fallback to sane defaults if minimum sizes aren't set
    if (minWidth <= 0)
      minWidth = 200;
    if (minHeight <= 0)
      minHeight = 150;

    // If the next cascade step goes off screen, start a new cascade
    if (x + minWidth > maxRight || y + minHeight > maxBottom)
    {
      cascadeCount++;
      x = startX + cascadeCount * offsetStep;
      y = startY;
    }

    // Update window geometry
    win->setX(x);
    win->setY(y);
    win->setWidth(minWidth);
    win->setHeight(minHeight);
    m_windowGeometry[win] = QRect(x, y, minWidth, minHeight);
    Q_EMIT geometryChanged(win);

    // Update position counters
    x += offsetStep;
    y += offsetStep;
  }
}

/**
 * @brief Opens a file dialog to allow the user to select a background image.
 */
void UI::WindowManager::selectBackgroundImage()
{
  auto path = QFileDialog::getOpenFileName(
      nullptr, tr("Select Background Image"),
      QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
      tr("Images (*.png *.jpg *.jpeg *.bmp)"));

  if (!path.isEmpty())
    setBackgroundImage(QUrl::fromLocalFile(path).toString());
}

/**
 * @brief Brings a window to the front by increasing its z-order.
 * @param item Pointer to the QQuickItem to promote in z-order.
 */
void UI::WindowManager::bringToFront(QQuickItem *item)
{
  // Validate pointer
  if (!item)
    return;

  // Set window Z
  m_windowZ[item] = ++m_zCounter;
  item->setZ(m_windowZ[item]);

  // Window manager Z is always one step ahead
  setZ(m_zCounter + 2);

  // Update UI
  Q_EMIT zCounterChanged();
  Q_EMIT zOrderChanged(item);
}

/**
 * @brief Sets the associated Taskbar instance for window management.
 *
 * This allows the WindowManager to interact with the Taskbar, such as
 * updating the active window, propagating window focus changes,
 * or accessing taskbar-specific features.
 *
 * @param taskbar Pointer to the UI::Taskbar instance to associate.
 */
void UI::WindowManager::setTaskbar(QQuickItem *taskbar)
{
  m_taskbar = static_cast<UI::Taskbar *>(taskbar);
}

/**
 * @brief Registers a new window item with the manager, assigning initial
 *        z-order and geometry.
 *
 * @param item Pointer to the QQuickItem to register.
 */
void UI::WindowManager::registerWindow(const int id, QQuickItem *item)
{
  // Validate item
  if (!item)
    return;

  // Register the window with the manager
  m_windows[id] = item;
  m_windowZ[item] = ++m_zCounter;
  m_windowGeometry[item] = extractGeometry(item);

  // Ensure window Z is in sync
  item->setZ(m_windowZ[item]);

  // Window manager Z is always one step ahead
  setZ(m_zCounter + 1);

  // Update UI
  Q_EMIT zCounterChanged();
  Q_EMIT zOrderChanged(item);
  Q_EMIT geometryChanged(item);
}

/**
 * @brief Unregisters a window, removing its z-order and geometry tracking.
 * @param item Pointer to the QQuickItem to remove.
 */
void UI::WindowManager::unregisterWindow(QQuickItem *item)
{
  m_windowZ.remove(item);
  m_windowGeometry.remove(item);
}

/**
 * @brief Sets the background image to be used for the container.
 * @param path URL string to the image file.
 */
void UI::WindowManager::setBackgroundImage(const QString &path)
{
  if (!path.isEmpty())
  {
    m_backgroundImage = path;
    Q_EMIT backgroundImageChanged();
  }
}

/**
 * @brief Enables or disables automatic window layout.
 *
 * Sets whether the window manager should automatically arrange windows
 * using the tiling layout algorithm. If enabled, it immediately triggers
 * a layout update.
 *
 * Emits autoLayoutEnabledChanged() if the state changes.
 *
 * @param enabled True to enable auto-layout, false to disable it.
 */
void UI::WindowManager::setAutoLayoutEnabled(const bool enabled)
{
  if (m_autoLayoutEnabled != enabled)
  {
    m_autoLayoutEnabled = enabled;
    Q_EMIT autoLayoutEnabledChanged();

    for (auto *win : std::as_const(m_windows))
    {
      if (win->state() == "maximized")
        QMetaObject::invokeMethod(win, "restoreClicked", Qt::DirectConnection);
    }

    loadLayout();
  }
}

/**
 * @brief Updates the stored geometry for a window.
 * @param item Pointer to the QQuickItem.
 * @param rect New QRectF representing the window’s position and size.
 */
void UI::WindowManager::updateGeometry(QQuickItem *item, const QRect &rect)
{
  if (!item)
    return;

  m_windowGeometry[item] = rect;
  Q_EMIT geometryChanged(item);
}

/**
 * @brief Reacts to changes in the desktop or available layout area.
 *
 * If auto-layout is enabled, this triggers a re-arrangement of all
 * visible windows to adapt to the new geometry.
 */
void UI::WindowManager::triggerLayoutUpdate()
{
  if (autoLayoutEnabled())
    autoLayout();
}

/**
 * @brief Utility function to extract a window’s actual geometry from its
 *        QQuickItem.
 *
 * @param item Pointer to the QQuickItem.
 * @return QRectF with x, y, width, and height, using implicit sizes as
 *         fallback.
 */
QRect UI::WindowManager::extractGeometry(QQuickItem *item) const
{
  return QRect(item->x(), item->y(),
               item->width() > 0 ? item->width() : item->implicitWidth(),
               item->height() > 0 ? item->height() : item->implicitHeight());
}

/**
 * @brief Determines which edge or corner of a window is being hovered for
 *        resizing.
 *
 * This function calculates the relative position of the mouse within the given
 * window and returns the appropriate ResizeEdge based on proximity to the edges
 * or corners.
 *
 * @param target Pointer to the window under the cursor.
 *
 * @return The ResizeEdge enum value indicating which edge is active, or
 *         ResizeEdge::None.
 */
UI::WindowManager::ResizeEdge
UI::WindowManager::detectResizeEdge(QQuickItem *target) const
{
  const int kResizeMargin = 8;

  QPointF localPos = target->mapFromItem(this, m_initialMousePos);
  const qreal x = localPos.x();
  const qreal y = localPos.y();
  const qreal w = target->width();
  const qreal h = target->height();

  const bool nearLeft = x <= kResizeMargin;
  const bool nearRight = x >= w - kResizeMargin;
  const bool nearTop = y <= kResizeMargin;
  const bool nearBottom = y >= h - kResizeMargin;

  if (nearLeft && nearTop)
    return ResizeEdge::TopLeft;
  else if (nearRight && nearTop)
    return ResizeEdge::TopRight;
  else if (nearLeft && nearBottom)
    return ResizeEdge::BottomLeft;
  else if (nearRight && nearBottom)
    return ResizeEdge::BottomRight;
  else if (nearLeft)
    return ResizeEdge::Left;
  else if (nearRight)
    return ResizeEdge::Right;
  else if (nearTop)
    return ResizeEdge::Top;
  else if (nearBottom)
    return ResizeEdge::Bottom;

  return ResizeEdge::None;
}

/**
 * @brief Returns the topmost window under a given point.
 *
 * This method performs hit-testing against all visible and registered windows,
 * sorted by descending z-order, and returns the first one that contains the
 * point.
 *
 * @param x X-coordinate relative to the WindowManager.
 * @param y Y-coordinate relative to the WindowManager.
 *
 * @return A pointer to the topmost QQuickItem window at the given position, or
 *         nullptr if none found.
 */
QQuickItem *UI::WindowManager::getWindow(const int x, const int y) const
{
  QPointF point(x, y);
  QList<QQuickItem *> windows = m_windows.values();
  std::sort(windows.begin(), windows.end(),
            [](QQuickItem *a, QQuickItem *b) { return a->z() > b->z(); });

  for (QQuickItem *window : std::as_const(windows))
  {
    if (!window || !window->isVisible())
      continue;

    QRectF bounds(window->x(), window->y(), window->width(), window->height());
    if (bounds.contains(point))
      return window;
  }

  return nullptr;
}

/**
 * @brief Handles mouse movement during drag or resize operations.
 *
 * Applies positional delta calculations to either move or resize the currently
 * interacted window. If no window is actively being dragged or resized, the
 * event is passed to the base class.
 *
 * @param event Pointer to the QMouseEvent containing the current mouse
 *              position.
 */
void UI::WindowManager::mouseMoveEvent(QMouseEvent *event)
{
  // Obtain current mouse position and calculate XY differential
  const QPoint currentPos = event->pos();
  const QPoint delta = currentPos - m_initialMousePos;

  // No window has been clicked before, abort
  if (!m_focusedWindow)
  {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  // Only execute geometry changes when it makes sense
  if (m_focusedWindow->state() != "normal" || autoLayoutEnabled())
  {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  // Drag the window & change it's position
  if (m_dragWindow)
  {
    qreal newX = m_initialGeometry.x() + delta.x();
    qreal newY = m_initialGeometry.y() + delta.y();

    const qreal w = m_dragWindow->width();
    const qreal h = m_dragWindow->height();

    newX = qBound(0.0, newX, width() - w);
    newY = qBound(0.0, newY, height() - h);

    m_dragWindow->setX(newX);
    m_dragWindow->setY(newY);

    event->accept();
    return;
  }

  // Resize window...not as easy as one might initially think
  if (m_resizeWindow)
  {
    QRect geometry = m_initialGeometry;
    const int minW = m_resizeWindow->implicitWidth();
    const int minH = m_resizeWindow->implicitHeight();
    switch (m_resizeEdge)
    {
      case ResizeEdge::Right:
        geometry.setWidth(qMax(minW, m_initialGeometry.width() + delta.x()));
        break;
      case ResizeEdge::Bottom:
        geometry.setHeight(qMax(minH, m_initialGeometry.height() + delta.y()));
        break;
      case ResizeEdge::Left: {
        const int w = qMax(minW, m_initialGeometry.width() - delta.x());
        geometry.setX(m_initialGeometry.right() - w);
        geometry.setWidth(w);
        break;
      }
      case ResizeEdge::Top: {
        const int h = qMax(minH, m_initialGeometry.height() - delta.y());
        geometry.setY(m_initialGeometry.bottom() - h);
        geometry.setHeight(h);
        break;
      }
      case ResizeEdge::TopLeft: {
        const int w = qMax(minW, m_initialGeometry.width() - delta.x());
        const int h = qMax(minH, m_initialGeometry.height() - delta.y());
        geometry.setX(m_initialGeometry.right() - w);
        geometry.setWidth(w);
        geometry.setY(m_initialGeometry.bottom() - h);
        geometry.setHeight(h);
        break;
      }
      case ResizeEdge::TopRight: {
        const int w = qMax(minW, m_initialGeometry.width() + delta.x());
        const int h = qMax(minH, m_initialGeometry.height() - delta.y());
        geometry.setY(m_initialGeometry.bottom() - h);
        geometry.setHeight(h);
        geometry.setWidth(w);
        break;
      }
      case ResizeEdge::BottomLeft: {
        const int w = qMax(minW, m_initialGeometry.width() - delta.x());
        const int h = qMax(minH, m_initialGeometry.height() + delta.y());
        geometry.setX(m_initialGeometry.right() - w);
        geometry.setWidth(w);
        geometry.setHeight(h);
        break;
      }
      case ResizeEdge::BottomRight:
        geometry.setWidth(qMax(minW, m_initialGeometry.width() + delta.x()));
        geometry.setHeight(qMax(minH, m_initialGeometry.height() + delta.y()));
        break;
      case ResizeEdge::None:
        break;
    }

    // Store unclamped geometry for comparison
    const QRect unclamped = geometry;

    // Clamp position so it doesn't leave canvas on the left/top
    geometry.setX(qMax(0, geometry.x()));
    geometry.setY(qMax(0, geometry.y()));

    // Clamp width if overflowing to the right
    if (geometry.right() > int(width()) - 1)
      geometry.setWidth(int(width()) - geometry.x());

    // Clamp height if overflowing to the bottom
    if (geometry.bottom() > int(height()) - 1)
      geometry.setHeight(int(height()) - geometry.y());

    // Apply geometry
    if (geometry == unclamped)
    {
      m_resizeWindow->setX(geometry.x());
      m_resizeWindow->setY(geometry.y());
      m_resizeWindow->setWidth(geometry.width());
      m_resizeWindow->setHeight(geometry.height());
      m_windowGeometry.insert(m_resizeWindow, geometry);
      event->accept();
    }

    return;
  }

  // Pass the event through
  QQuickItem::mouseMoveEvent(event);
}

/**
 * @brief Handles mouse press interactions for initiating window drag or resize.
 *
 * This function performs a hit-test to determine the window under the cursor.
 * Based on the position relative to the window edges or title bar, it either
 * starts a drag operation or a resize, and captures the mouse input.
 *
 * If the window is not in a normal state or auto-layout is enabled, the event
 * is passed through.
 *
 * @param event Pointer to the QMouseEvent containing the press information.
 */
void UI::WindowManager::mousePressEvent(QMouseEvent *event)
{
  // Reset window tracking parameters
  m_dragWindow = nullptr;
  m_resizeWindow = nullptr;
  m_focusedWindow = nullptr;
  m_resizeEdge = ResizeEdge::None;
  m_initialMousePos = event->pos();

  // Find the topmost window under the mouse
  m_focusedWindow = getWindow(m_initialMousePos.x(), m_initialMousePos.y());
  if (!m_focusedWindow)
  {
    m_taskbar->setActiveWindow(nullptr);
    if (event->button() == Qt::RightButton)
      Q_EMIT rightClicked(m_initialMousePos.x(), m_initialMousePos.y());

    return;
  }

  // Update focus
  if (m_taskbar)
    m_taskbar->setActiveWindow(m_focusedWindow);

  // Check if we're clicking the title bar (top area)
  bool captionClick = false;
  const int captionH = m_focusedWindow->property("captionHeight").toInt();
  const int buttonsW = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto mouseClick = m_focusedWindow->mapFromItem(this, m_initialMousePos);
  if (mouseClick.y() <= captionH)
  {
    // User clicked on caption
    if (mouseClick.x() <= m_focusedWindow->width() - buttonsW)
      captionClick = true;

    // User clicked on caption buttons, let QML process events
    else
    {
      QQuickItem::mousePressEvent(event);
      return;
    }
  }

  // Only allow resizing & moving window when it makes sense
  if (m_focusedWindow->state() == "normal" && !autoLayoutEnabled())
  {
    // Detect active resize edge & start resizing
    m_resizeEdge = detectResizeEdge(m_focusedWindow);
    if (m_resizeEdge != ResizeEdge::None)
    {
      grabMouse();
      switch (m_resizeEdge)
      {
        case ResizeEdge::Left:
        case ResizeEdge::Right:
          setCursor(Qt::SizeHorCursor);
          break;
        case ResizeEdge::Top:
        case ResizeEdge::Bottom:
          setCursor(Qt::SizeVerCursor);
          break;
        case ResizeEdge::TopRight:
        case ResizeEdge::BottomLeft:
          setCursor(Qt::SizeBDiagCursor);
          break;
        case ResizeEdge::TopLeft:
        case ResizeEdge::BottomRight:
          setCursor(Qt::SizeFDiagCursor);
          break;
        default:
          unsetCursor();
          break;
      }

      m_resizeWindow = m_focusedWindow;
      m_initialGeometry = extractGeometry(m_focusedWindow);
      event->accept();
      return;
    }

    // Check if we're clicking the title bar (top area)
    if (captionClick)
    {
      grabMouse();
      setCursor(Qt::ClosedHandCursor);
      m_dragWindow = m_focusedWindow;
      m_initialGeometry = extractGeometry(m_focusedWindow);
      event->accept();
    }
  }

  // No caption click, let window process mouse event
  if (!captionClick)
    QQuickItem::mousePressEvent(event);
}

/**
 * @brief Handles mouse release to finalize window drag or resize operations.
 *
 * Resets internal tracking state, releases mouse grab, and restores the cursor.
 * If no window interaction is ongoing, the event is passed to the base class.
 *
 * @param event Pointer to the QMouseEvent representing the release action.
 */
void UI::WindowManager::mouseReleaseEvent(QMouseEvent *event)
{
  // Reset mouse cursor
  ungrabMouse();
  unsetCursor();

  // Reset window tracking parameters
  m_dragWindow = nullptr;
  m_resizeWindow = nullptr;
  m_focusedWindow = nullptr;
  m_resizeEdge = ResizeEdge::None;
  m_initialMousePos = event->pos();

  // Pass the event through
  QQuickItem::mouseReleaseEvent(event);
}

/**
 * @brief Handles double-click events on window title bars to toggle
 *        maximize/restore.
 *
 * This method detects a double-click in the caption (title bar) area of a
 * window, excluding the region reserved for window control buttons. If a valid
 * window is detected under the cursor, and the double-click occurs within the
 * allowed area, it toggles the window state between "maximized" and "normal" by
 * invoking the appropriate QML signal (`maximizeClicked` or `restoreClicked`)
 * on the target window.
 *
 * If the double-click occurs outside any interactive window area, the event is
 * passed to the base class for default processing.
 *
 * @param event Pointer to the QMouseEvent containing double-click information.
 */
void UI::WindowManager::mouseDoubleClickEvent(QMouseEvent *event)
{
  // Check if there is a window there
  m_focusedWindow = getWindow(event->pos().x(), event->pos().y());
  if (!m_focusedWindow)
  {
    QQuickItem::mouseDoubleClickEvent(event);
    return;
  }

  // Check if double-click was in the title bar area (not on window buttons)
  const int captionH = m_focusedWindow->property("captionHeight").toInt();
  const int buttonsW = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto localPos = m_focusedWindow->mapFromItem(this, event->pos());
  if (localPos.y() <= captionH
      && localPos.x() <= m_focusedWindow->width() - buttonsW)
  {
    // Obtain current state
    const QString state = m_focusedWindow->property("state").toString();

    // Restore the window
    if (state == "maximized")
    {
      QMetaObject::invokeMethod(m_focusedWindow, "restoreClicked",
                                Qt::DirectConnection);
    }

    // Maximize the window
    else if (state == "normal")
    {
      QMetaObject::invokeMethod(m_focusedWindow, "maximizeClicked",
                                Qt::DirectConnection);
    }

    // Block further processing of the event
    event->accept();
    return;
  }

  // Pass the event through
  QQuickItem::mouseDoubleClickEvent(event);
}
