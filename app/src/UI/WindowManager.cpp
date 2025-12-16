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

#include "WindowManager.h"
#include "UI/Taskbar.h"

#include <QtMath>

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
  , m_resizeEdge(ResizeEdge::None)
  , m_snapIndicatorVisible(false)
  , m_taskbar(nullptr)
  , m_dragWindow(nullptr)
  , m_targetWindow(nullptr)
  , m_resizeWindow(nullptr)
  , m_focusedWindow(nullptr)
{
  setEnabled(true);
  setAcceptHoverEvents(false);
  setFlag(ItemHasContents, false);
  setFiltersChildMouseEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);

  m_backgroundImage = m_settings.value("WindowManager_Wallpaper").toString();

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
 * @brief Indicates whether the snap indicator is currently visible.
 *
 * The snap indicator is shown while a window is being dragged and overlaps
 * another window in auto-layout mode, giving visual feedback about the
 * potential reorder target.
 *
 * @return True if the snap indicator is visible, false otherwise.
 */
bool UI::WindowManager::snapIndicatorVisible() const
{
  return m_snapIndicatorVisible;
}

/**
 * @brief Returns the geometry of the current snap indicator.
 *
 * This represents the visual bounds of the target window under the dragged
 * window, used to show where the window will snap if released.
 *
 * @return A const reference to the QRect representing the snap indicator's
 * geometry.
 */
const QRect &UI::WindowManager::snapIndicator() const
{
  return m_snapIndicator;
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
 * @brief Serializes the current window layout to a JSON object.
 *
 * The serialized layout includes:
 * - Layout mode (auto or manual)
 * - Window order (list of window IDs in display order)
 * - Window geometries (position and size for each window in manual mode)
 *
 * This can be saved to a project file and later restored with restoreLayout().
 *
 * @return QJsonObject containing the complete layout state.
 */
QJsonObject UI::WindowManager::serializeLayout() const
{
  QJsonObject layout;
  if (!m_autoLayoutEnabled)
  {
    QJsonArray geometries;
    for (int id : m_windowOrder)
    {
      auto *win = m_windows.value(id);
      if (!win)
        continue;

      QJsonObject winGeom;
      winGeom["id"] = id;
      winGeom["x"] = win->x();
      winGeom["y"] = win->y();
      winGeom["width"] = win->width();
      winGeom["height"] = win->height();
      winGeom["state"] = win->state();
      geometries.append(winGeom);
    }

    layout["geometries"] = geometries;
  }

  QJsonArray orderArray;
  for (int id : m_windowOrder)
    orderArray.append(id);

  layout["windowOrder"] = orderArray;
  layout["autoLayout"] = m_autoLayoutEnabled;

  return layout;
}

/**
 * @brief Restores a previously serialized window layout.
 *
 * This function applies the layout configuration from a JSON object:
 * - Sets the layout mode (auto or manual)
 * - Reorders windows according to the saved order
 * - Restores window geometries in manual mode
 *
 * Windows that exist in the saved layout but not in the current session
 * are skipped. Windows that exist in the current session but not in the
 * saved layout retain their current position.
 *
 * @param layout The JSON object containing the layout state.
 * @return True if the layout was successfully restored, false otherwise.
 */
bool UI::WindowManager::restoreLayout(const QJsonObject &layout)
{
  if (layout.isEmpty())
    return false;

  bool autoLayout = layout["autoLayout"].toBool(true);

  if (layout.contains("windowOrder"))
  {
    QJsonArray orderArray = layout["windowOrder"].toArray();
    QVector<int> newOrder;

    for (const auto &val : std::as_const(orderArray))
    {
      int id = val.toInt(-1);
      if (id >= 0 && m_windows.contains(id))
        newOrder.append(id);
    }

    for (int id : std::as_const(m_windowOrder))
    {
      if (!newOrder.contains(id))
        newOrder.append(id);
    }

    m_windowOrder = newOrder;
  }

  if (!autoLayout && layout.contains("geometries"))
  {
    QJsonArray geometries = layout["geometries"].toArray();
    for (const auto &val : std::as_const(geometries))
    {
      QJsonObject winGeom = val.toObject();
      int id = winGeom["id"].toInt(-1);

      auto *win = m_windows.value(id);
      if (!win)
        continue;

      double x = winGeom["x"].toDouble(0);
      double y = winGeom["y"].toDouble(0);
      double w = winGeom["width"].toDouble(200);
      double h = winGeom["height"].toDouble(150);

      win->setX(x);
      win->setY(y);
      win->setWidth(w);
      win->setHeight(h);

      Q_EMIT geometryChanged(win);
    }

    constrainWindows();
  }

  setAutoLayoutEnabled(autoLayout);

  return true;
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
  m_windowOrder.clear();
  m_dragWindow = nullptr;
  m_targetWindow = nullptr;
  m_resizeWindow = nullptr;
  m_focusedWindow = nullptr;
  m_snapIndicatorVisible = false;

  Q_EMIT zCounterChanged();
  Q_EMIT snapIndicatorChanged();
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
 * @brief Automatically tiles visible windows using a smart grid-based layout.
 *
 * This layout algorithm creates visually balanced arrangements:
 *
 * - 1 window:  Fills the entire canvas (maximized feel)
 * - 2 windows: Side-by-side (landscape) or stacked (portrait)
 * - 3 windows: Master-stack layout (one large + two stacked)
 * - 4 windows: 2x2 grid
 * - 5 windows: 2 on top row, 3 on bottom (or vice versa based on aspect)
 * - 6 windows: 2x3 or 3x2 grid based on canvas aspect ratio
 * - 7+ windows: Optimal grid with balanced distribution
 *
 * The algorithm prioritizes:
 * - Balanced window sizes (avoids tiny windows)
 * - Aspect ratio awareness (uses canvas shape to decide layout)
 * - Consistent spacing and margins
 * - Master-stack pattern for 3 windows (common productivity layout)
 */
void UI::WindowManager::autoLayout()
{
  const int margin = 4;
  const int spacing = 4;

  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  QList<QQuickItem *> windows;
  for (int id : std::as_const(m_windowOrder))
  {
    auto *win = m_windows.value(id);
    if (win && win->state() == "normal")
      windows.append(win);
  }

  if (windows.isEmpty())
    return;

  const int n = windows.size();
  const int availW = canvasW - 2 * margin;
  const int availH = canvasH - 2 * margin;
  const bool isLandscape = availW >= availH;

  auto placeWindow = [&](QQuickItem *win, int x, int y, int w, int h) {
    win->setX(x);
    win->setY(y);
    win->setWidth(w);
    win->setHeight(h);
    Q_EMIT geometryChanged(win);
  };

  if (n == 1)
    placeWindow(windows[0], margin, margin, availW, availH);

  else if (n == 2)
  {
    if (isLandscape)
    {
      int w = (availW - spacing) / 2;
      placeWindow(windows[0], margin, margin, w, availH);
      placeWindow(windows[1], margin + w + spacing, margin, w, availH);
    }
    else
    {
      int h = (availH - spacing) / 2;
      placeWindow(windows[0], margin, margin, availW, h);
      placeWindow(windows[1], margin, margin + h + spacing, availW, h);
    }
  }

  else if (n == 3)
  {
    if (isLandscape)
    {
      int masterW = availW / 2;
      int stackW = availW - masterW - spacing;
      int stackH = (availH - spacing) / 2;

      placeWindow(windows[0], margin, margin, masterW, availH);
      placeWindow(windows[1], margin + masterW + spacing, margin, stackW,
                  stackH);
      placeWindow(windows[2], margin + masterW + spacing,
                  margin + stackH + spacing, stackW, stackH);
    }
    else
    {
      int masterH = availH / 2;
      int stackH = availH - masterH - spacing;
      int stackW = (availW - spacing) / 2;

      placeWindow(windows[0], margin, margin, availW, masterH);
      placeWindow(windows[1], margin, margin + masterH + spacing, stackW,
                  stackH);
      placeWindow(windows[2], margin + stackW + spacing,
                  margin + masterH + spacing, stackW, stackH);
    }
  }

  else if (n == 4)
  {
    int w = (availW - spacing) / 2;
    int h = (availH - spacing) / 2;

    placeWindow(windows[0], margin, margin, w, h);
    placeWindow(windows[1], margin + w + spacing, margin, w, h);
    placeWindow(windows[2], margin, margin + h + spacing, w, h);
    placeWindow(windows[3], margin + w + spacing, margin + h + spacing, w, h);
  }

  else if (n == 5)
  {
    if (isLandscape)
    {
      int topW = (availW - spacing) / 2;
      int botW = (availW - 2 * spacing) / 3;
      int h = (availH - spacing) / 2;

      placeWindow(windows[0], margin, margin, topW, h);
      placeWindow(windows[1], margin + topW + spacing, margin, topW, h);
      placeWindow(windows[2], margin, margin + h + spacing, botW, h);
      placeWindow(windows[3], margin + botW + spacing, margin + h + spacing,
                  botW, h);
      placeWindow(windows[4], margin + 2 * (botW + spacing),
                  margin + h + spacing, botW, h);
    }
    else
    {
      int leftH = (availH - spacing) / 2;
      int rightH = (availH - 2 * spacing) / 3;
      int w = (availW - spacing) / 2;

      placeWindow(windows[0], margin, margin, w, leftH);
      placeWindow(windows[1], margin, margin + leftH + spacing, w, leftH);
      placeWindow(windows[2], margin + w + spacing, margin, w, rightH);
      placeWindow(windows[3], margin + w + spacing, margin + rightH + spacing,
                  w, rightH);
      placeWindow(windows[4], margin + w + spacing,
                  margin + 2 * (rightH + spacing), w, rightH);
    }
  }

  else if (n == 6)
  {
    if (isLandscape)
    {
      int w = (availW - 2 * spacing) / 3;
      int h = (availH - spacing) / 2;

      for (int i = 0; i < 6; ++i)
      {
        int col = i % 3;
        int row = i / 3;
        placeWindow(windows[i], margin + col * (w + spacing),
                    margin + row * (h + spacing), w, h);
      }
    }
    else
    {
      int w = (availW - spacing) / 2;
      int h = (availH - 2 * spacing) / 3;

      for (int i = 0; i < 6; ++i)
      {
        int col = i % 2;
        int row = i / 2;
        placeWindow(windows[i], margin + col * (w + spacing),
                    margin + row * (h + spacing), w, h);
      }
    }
  }

  else
  {
    int cols, rows;
    if (isLandscape)
    {
      cols = qCeil(qSqrt(static_cast<double>(n) * availW / availH));
      rows = qCeil(static_cast<double>(n) / cols);
    }
    else
    {
      rows = qCeil(qSqrt(static_cast<double>(n) * availH / availW));
      cols = qCeil(static_cast<double>(n) / rows);
    }

    while (cols * rows < n)
    {
      if (isLandscape)
        cols++;
      else
        rows++;
    }

    int cellW = (availW - (cols - 1) * spacing) / cols;
    int cellH = (availH - (rows - 1) * spacing) / rows;

    int lastRowStart = (rows - 1) * cols;
    int windowsInLastRow = n - lastRowStart;
    int lastRowCellW = cellW;
    int lastRowOffsetX = 0;

    if (windowsInLastRow < cols && windowsInLastRow > 0)
    {
      int lastRowTotalW = availW;
      lastRowCellW
          = (lastRowTotalW - (windowsInLastRow - 1) * spacing) / windowsInLastRow;
      lastRowOffsetX = 0;
    }

    for (int i = 0; i < n; ++i)
    {
      int row = i / cols;
      int col = i % cols;

      int x, y, w, h;

      if (row == rows - 1 && windowsInLastRow < cols)
      {
        int lastRowCol = i - lastRowStart;
        x = margin + lastRowOffsetX + lastRowCol * (lastRowCellW + spacing);
        w = lastRowCellW;
      }
      else
      {
        x = margin + col * (cellW + spacing);
        w = cellW;
      }

      y = margin + row * (cellH + spacing);
      h = cellH;

      placeWindow(windows[i], x, y, w, h);
    }
  }

  for (auto *win : std::as_const(m_windows))
  {
    if (win && !win->isVisible())
    {
      if (win->state() == "normal" || win->state() == "maximized")
        win->setVisible(true);
    }
  }
}

/**
 * @brief Arranges windows using a macOS-inspired smart cascade layout.
 *
 * This layout algorithm positions windows in a visually appealing way:
 *
 * 1. Windows are sized to approximately 60% of canvas dimensions, respecting
 *    minimum sizes from implicitWidth/implicitHeight.
 *
 * 2. The first window is centered in the canvas.
 *
 * 3. Subsequent windows are offset diagonally (down-right) with a fixed step.
 *
 * 4. When a window would exceed the bottom-right bounds, the cascade wraps
 *    back to the top with a horizontal offset, similar to macOS behavior.
 *
 * 5. If horizontal space is exhausted, cascading restarts from the origin
 *    with a smaller offset to layer windows visibly.
 *
 * This approach ensures windows remain accessible and visually organized,
 * even with many windows open.
 */
void UI::WindowManager::cascadeLayout()
{
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  QList<QQuickItem *> visibleWindows;
  for (int id : std::as_const(m_windowOrder))
  {
    auto *win = m_windows.value(id);
    if (win && win->state() == "normal")
      visibleWindows.append(win);
  }

  if (visibleWindows.isEmpty())
    return;

  const int margin = 8;
  const int cascadeOffsetX = 26;
  const int cascadeOffsetY = 26;

  const int availableW = canvasW - 2 * margin;
  const int availableH = canvasH - 2 * margin;

  for (int i = 0; i < visibleWindows.size(); ++i)
  {
    QQuickItem *win = visibleWindows[i];
    if (!win)
      continue;

    int minW = qMax(static_cast<int>(win->implicitWidth()), 200);
    int minH = qMax(static_cast<int>(win->implicitHeight()), 150);

    int winW = qMax(minW, availableW * 55 / 100);
    int winH = qMax(minH, availableH * 60 / 100);

    winW = qMin(winW, availableW);
    winH = qMin(winH, availableH);

    int baseX = margin + (availableW - winW) / 2;
    int baseY = margin + (availableH - winH) / 2;

    int offsetX = i * cascadeOffsetX;
    int offsetY = i * cascadeOffsetY;

    int wrapCount = 0;
    while (baseY + offsetY + winH > canvasH - margin && wrapCount < 10)
    {
      offsetY -= (availableH - winH);
      offsetX += cascadeOffsetX * 2;
      wrapCount++;
    }

    while (baseX + offsetX + winW > canvasW - margin && wrapCount < 20)
    {
      offsetX -= (availableW - winW);
      wrapCount++;
    }

    int winX = baseX + offsetX;
    int winY = baseY + offsetY;

    winX = qBound(margin, winX, canvasW - winW - margin);
    winY = qBound(margin, winY, canvasH - winH - margin);

    if (winW > availableW)
    {
      winW = availableW;
      winX = margin;
    }

    if (winH > availableH)
    {
      winH = availableH;
      winY = margin;
    }

    win->setX(winX);
    win->setY(winY);
    win->setWidth(winW);
    win->setHeight(winH);
    Q_EMIT geometryChanged(win);
  }

  for (auto *win : std::as_const(m_windows))
  {
    if (win && !win->isVisible())
    {
      if (win->state() == "normal" || win->state() == "maximized")
        win->setVisible(true);
    }
  }
}

/**
 * @brief Removes the background image and clears the settings.
 */
void UI::WindowManager::clearBackgroundImage()
{
  setBackgroundImage("");
}

/**
 * @brief Opens a file dialog to allow the user to select a background image.
 */
void UI::WindowManager::selectBackgroundImage()
{
  auto *dialog = new QFileDialog(
      nullptr, tr("Select Background Image"),
      QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
      tr("Images (*.png *.jpg *.jpeg *.bmp)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            if (!path.isEmpty())
              setBackgroundImage(QUrl::fromLocalFile(path).toString());

            dialog->deleteLater();
          });

  dialog->open();
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
  m_windowOrder.append(id);
  m_windowZ[item] = ++m_zCounter;

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
 *
 * After unregistering, triggers a layout update if auto-layout is enabled
 * to redistribute the remaining windows.
 *
 * @param item Pointer to the QQuickItem to remove.
 */
void UI::WindowManager::unregisterWindow(QQuickItem *item)
{
  m_windowZ.remove(item);
  m_windowOrder.removeAll(getIdForWindow(item));
  for (auto it = m_windows.begin(); it != m_windows.end(); ++it)
  {
    if (it.value() == item)
    {
      m_windows.remove(it.key());
      break;
    }
  }

  // Trigger layout update to redistribute remaining windows
  triggerLayoutUpdate();
}

/**
 * @brief Sets the background image to be used for the container.
 * @param path URL string to the image file.
 */
void UI::WindowManager::setBackgroundImage(const QString &path)
{
  if (m_backgroundImage != path)
  {
    m_backgroundImage = path;
    m_settings.setValue("WindowManager_Wallpaper", path);
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
 * @brief Constrains all windows to fit within the current canvas bounds.
 *
 * This function ensures that no window is positioned outside the visible
 * canvas area and resizes windows if they exceed the available space.
 * It maintains a minimum visible portion of each window to prevent
 * windows from becoming completely inaccessible.
 *
 * The function handles:
 * - Windows positioned beyond the right/bottom edges
 * - Windows larger than the canvas (resized to fit)
 * - Windows with negative positions (clamped to 0)
 */
void UI::WindowManager::constrainWindows()
{
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  for (auto *win : std::as_const(m_windows))
  {
    if (!win)
      continue;

    if (win->state() != "normal")
      continue;

    int winX = static_cast<int>(win->x());
    int winY = static_cast<int>(win->y());
    int winW = static_cast<int>(win->width());
    int winH = static_cast<int>(win->height());

    const int minW = qMax(static_cast<int>(win->implicitWidth()), 100);
    const int minH = qMax(static_cast<int>(win->implicitHeight()), 80);

    bool changed = false;

    if (winW > canvasW)
    {
      winW = canvasW;
      changed = true;
    }

    if (winH > canvasH)
    {
      winH = canvasH;
      changed = true;
    }

    if (winW < minW && canvasW >= minW)
    {
      winW = minW;
      changed = true;
    }

    if (winH < minH && canvasH >= minH)
    {
      winH = minH;
      changed = true;
    }

    if (winX < 0)
    {
      winX = 0;
      changed = true;
    }

    if (winY < 0)
    {
      winY = 0;
      changed = true;
    }

    if (winX + winW > canvasW)
    {
      winX = canvasW - winW;
      if (winX < 0)
      {
        winX = 0;
        winW = canvasW;
      }
      changed = true;
    }

    if (winY + winH > canvasH)
    {
      winY = canvasH - winH;
      if (winY < 0)
      {
        winY = 0;
        winH = canvasH;
      }
      changed = true;
    }

    if (changed)
    {
      win->setX(winX);
      win->setY(winY);
      win->setWidth(winW);
      win->setHeight(winH);
      Q_EMIT geometryChanged(win);
    }
  }
}

/**
 * @brief Reacts to changes in the desktop or available layout area.
 *
 * If auto-layout is enabled, this triggers a re-arrangement of all
 * visible windows to adapt to the new geometry. If auto-layout is
 * disabled (manual layout), it constrains windows to ensure they
 * remain visible and properly sized within the canvas.
 */
void UI::WindowManager::triggerLayoutUpdate()
{
  if (autoLayoutEnabled())
    autoLayout();
  else
    constrainWindows();
}

/**
 * @brief Retrieves the ID associated with a registered window item.
 *
 * Performs a reverse lookup in the window map to find the integer ID
 * assigned to the given QQuickItem.
 *
 * @param item Pointer to the window item.
 * @return The window ID if found, or -1 if not registered.
 */
int UI::WindowManager::getIdForWindow(QQuickItem *item) const
{
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it)
  {
    if (it.value() == item)
      return it.key();
  }

  return -1;
}

/**
 * @brief Calculates the target index for reordering a dragged window.
 *
 * Uses the current mouse position to determine which window is hovered,
 * then returns the appropriate insertion index.
 *
 * If no window is hovered, returns m_windowOrder.size() to indicate the end.
 *
 * @param pos Mouse position relative to the WindowManager.
 * @return Target index for reordering.
 */
int UI::WindowManager::determineNewIndexFromMousePos(const QPoint &pos) const
{
  QQuickItem *hoveredWindow = getWindow(pos.x(), pos.y());
  if (!hoveredWindow)
    return m_windowOrder.size();

  if (hoveredWindow->state() != "normal")
    return m_windowOrder.size();

  int hoveredId = getIdForWindow(hoveredWindow);
  if (hoveredId < 0)
    return m_windowOrder.size();

  return m_windowOrder.indexOf(hoveredId);
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
  if (target->state() == "normal")
  {
    const int kResizeMargin = 8;
    QPointF localPos = target->mapFromItem(this, m_initialMousePos);
    const int x = static_cast<int>(localPos.x());
    const int y = static_cast<int>(localPos.y());
    const int w = static_cast<int>(target->width());
    const int h = static_cast<int>(target->height());

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
  }

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
    if (!window || !window->isVisible() || window == m_dragWindow)
      continue;

    const auto state = window->state();
    if (state != "normal" && state != "maximized")
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
  int dragDistance = delta.manhattanLength();

  // No window has been clicked before, abort
  if (!m_focusedWindow)
  {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  // Only execute geometry changes when it makes sense
  if (m_focusedWindow->state() != "normal")
  {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  // Drag the window & change it's position
  if (m_dragWindow && dragDistance >= 20)
  {
    // Obtain new X/Y position
    int newX = m_initialGeometry.x() + delta.x();
    int newY = m_initialGeometry.y() + delta.y();

    // Obtain window size
    int w = static_cast<int>(m_dragWindow->width());
    int h = static_cast<int>(m_dragWindow->height());

    // Obtain canvas dimensions
    const int canvasW = static_cast<int>(width());
    const int canvasH = static_cast<int>(height());

    // Restore window size if needed
    if ((w >= canvasW - 20 || h >= canvasH - 20) && !autoLayoutEnabled())
    {
      w = static_cast<int>(m_dragWindow->implicitWidth());
      h = static_cast<int>(m_dragWindow->implicitHeight());
      m_dragWindow->setWidth(w);
      m_dragWindow->setHeight(h);
    }

    // Bound X/Y position
    newX = qBound(0, newX, canvasW - w);
    newY = qBound(0, newY, canvasH - h);

    // Apply X/Y position
    m_dragWindow->setX(newX);
    m_dragWindow->setY(newY);

    // On auto layout, display a snap indicator over the nearest window
    // near the mouse position
    if (autoLayoutEnabled())
    {
      // Detect the window under the cursor (excluding the window being dragged)
      int targetIndex = determineNewIndexFromMousePos(currentPos);

      // Show indicator if target window is valid and we dragged the mouse
      if (targetIndex >= 0 && targetIndex < m_windowOrder.size())
      {
        // Get current window order for target window, and obtain a pointer
        int targetId = m_windowOrder[targetIndex];
        m_targetWindow = m_windows.value(targetId);

        // Resize the drag window if required, and display snap indicator
        if (m_targetWindow && m_targetWindow != m_dragWindow)
        {
          // Resize drag window if needed
          int nW = qMin(w, static_cast<int>(m_targetWindow->width()));
          int nH = qMin(h, static_cast<int>(m_targetWindow->height()));
          if (w != nW || h != nH)
          {
            m_dragWindow->setWidth(nW);
            m_dragWindow->setHeight(nH);
          }

          // Set snap indicator geometry to target window geometry
          m_snapIndicator = extractGeometry(m_targetWindow);
          m_snapIndicatorVisible = true;
          Q_EMIT snapIndicatorChanged();

          // Accept the event and abort
          event->accept();
          return;
        }
      }

      // Something failed, hide the snap indicator and cancel the operation
      if (m_snapIndicatorVisible)
      {
        m_snapIndicatorVisible = false;
        Q_EMIT snapIndicatorChanged();
      }
    }

    // No auto layout, show snap indicator on desktop edges
    else
    {
      // Set window rect
      const int top = newY;
      const int left = newX;
      const int right = newX + w;
      const int bottom = newY + h;

      // Initialize snapped flag
      bool snapped = false;

      // Top-left corner
      if (left <= 0 && top <= 0)
      {
        m_snapIndicator = QRect(0, 0, canvasW / 2, canvasH / 2);
        snapped = true;
      }

      // Top-right corner
      else if (right >= canvasW && top <= 0)
      {
        m_snapIndicator = QRect(canvasW / 2, 0, canvasW / 2, canvasH / 2);
        snapped = true;
      }

      // Bottom-left corner
      else if (left <= 0 && bottom >= canvasH)
      {
        m_snapIndicator = QRect(0, canvasH / 2, canvasW / 2, canvasH / 2);
        snapped = true;
      }

      // Bottom-right corner
      else if (right >= canvasW && bottom >= canvasH)
      {
        m_snapIndicator
            = QRect(canvasW / 2, canvasH / 2, canvasW / 2, canvasH / 2);
        snapped = true;
      }

      // Top edge = maximize
      else if (top <= 0)
      {
        m_snapIndicator = QRect(0, 0, canvasW, canvasH);
        snapped = true;
      }

      // Left edge = left half
      else if (left <= 0)
      {
        m_snapIndicator = QRect(0, 0, canvasW / 2, canvasH);
        snapped = true;
      }

      // Right edge = right half
      else if (right >= canvasW)
      {
        m_snapIndicator = QRect(canvasW / 2, 0, canvasW / 2, canvasH);
        snapped = true;
      }

      // Display the snap indicator
      if (snapped)
      {
        m_snapIndicatorVisible = true;
        Q_EMIT snapIndicatorChanged();
      }

      // Hide the snap indicator
      else if (m_snapIndicatorVisible)
      {
        m_snapIndicatorVisible = false;
        Q_EMIT snapIndicatorChanged();
      }
    }

    // Accept the event and abort, we dont drag and resize at the same time
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
  m_targetWindow = nullptr;
  m_resizeWindow = nullptr;
  m_focusedWindow = nullptr;
  m_resizeEdge = ResizeEdge::None;
  m_initialMousePos = event->pos();

  // Hide snapping rectangle
  if (m_snapIndicatorVisible)
  {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

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
  const int externcW = m_focusedWindow->property("externControlWidth").toInt();
  const int buttonsW = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto mouseClick = m_focusedWindow->mapFromItem(this, m_initialMousePos);
  if (mouseClick.y() <= captionH)
  {
    // User clicked on caption
    if (mouseClick.x() <= m_focusedWindow->width() - buttonsW
        && mouseClick.x() > externcW)
      captionClick = true;

    // User clicked on caption buttons, let QML process events
    else
    {
      QQuickItem::mousePressEvent(event);
      return;
    }
  }

  // Only allow resizing & moving window when it makes sense
  if (m_focusedWindow->state() == "normal")
  {
    // Detect active resize edge & start resizing
    m_resizeEdge = detectResizeEdge(m_focusedWindow);
    if (m_resizeEdge != ResizeEdge::None && !autoLayoutEnabled())
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
  // Finalize reordering after drag in auto-layout mode
  if (autoLayoutEnabled())
  {
    if (m_dragWindow && m_targetWindow && m_snapIndicatorVisible)
    {
      const int draggedId = getIdForWindow(m_dragWindow);
      const int targetId = getIdForWindow(m_targetWindow);
      if (draggedId >= 0 && targetId >= 0)
      {
        const int newIndex = m_windowOrder.indexOf(targetId);
        const int currentIndex = m_windowOrder.indexOf(draggedId);
        if (newIndex >= 0 && newIndex != currentIndex)
          std::swap(m_windowOrder[currentIndex], m_windowOrder[newIndex]);
      }
    }

    loadLayout();
  }

  // No auto-layout, but user snapped the window
  else if (m_dragWindow && m_snapIndicatorVisible)
  {
    // Get snap area geometry
    auto x = m_snapIndicator.x();
    auto y = m_snapIndicator.y();
    auto w = m_snapIndicator.width();
    auto h = m_snapIndicator.height();

    // Maximize the window
    if (x == 0 && y == 0 && w >= width() && h >= height())
    {
      QMetaObject::invokeMethod(m_dragWindow, "maximizeClicked",
                                Qt::DirectConnection);
    }

    // Update window geometry
    else
    {
      m_dragWindow->setX(x);
      m_dragWindow->setY(y);
      m_dragWindow->setWidth(w);
      m_dragWindow->setHeight(h);
    }
  }

  // Reset mouse cursor
  ungrabMouse();
  unsetCursor();

  // Reset snap indicator
  if (m_snapIndicatorVisible)
  {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

  // Reset window tracking parameters
  m_dragWindow = nullptr;
  m_targetWindow = nullptr;
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
  const int externcW = m_focusedWindow->property("externControlWidth").toInt();
  const int buttonsW = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto localPos = m_focusedWindow->mapFromItem(this, event->pos());
  if (localPos.y() <= captionH
      && localPos.x() <= m_focusedWindow->width() - buttonsW
      && localPos.x() > externcW)
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
