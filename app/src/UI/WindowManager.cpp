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

#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>
#include <QtMath>
#include <QUrl>

#include "UI/Taskbar.h"
#include "UI/UISessionRegistry.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the WindowManager singleton.
 */
UI::WindowManager::WindowManager(QQuickItem* parent)
  : QQuickItem(parent)
  , m_zCounter(1)
  , m_layoutRestored(false)
  , m_autoLayoutEnabled(true)
  , m_resizeEdge(ResizeEdge::None)
  , m_snapIndicatorVisible(false)
  , m_taskbar(nullptr)
  , m_dragWindow(nullptr)
  , m_targetWindow(nullptr)
  , m_resizeWindow(nullptr)
  , m_focusedWindow(nullptr)
{
  // Configure item flags for mouse event handling
  setEnabled(true);
  setAcceptHoverEvents(false);
  setFlag(ItemHasContents, false);
  setFiltersChildMouseEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);

  // Restore persisted wallpaper
  m_backgroundImage = m_settings.value("WindowManager_Wallpaper").toString();

  // Re-layout when canvas dimensions change
  connect(this, &UI::WindowManager::widthChanged, this, &UI::WindowManager::triggerLayoutUpdate);
  connect(this, &UI::WindowManager::heightChanged, this, &UI::WindowManager::triggerLayoutUpdate);

  // Register with the global session registry
  UISessionRegistry::instance().registerWindowManager(this);
}

UI::WindowManager::~WindowManager()
{
  UISessionRegistry::instance().unregisterWindowManager(this);
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

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
const QString& UI::WindowManager::backgroundImage() const
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
const QRect& UI::WindowManager::snapIndicator() const
{
  return m_snapIndicator;
}

/**
 * @brief Retrieves the z-order for a given window item.
 * @param item Pointer to the QQuickItem representing the window.
 * @return Z-order value, or -1 if not registered.
 */
int UI::WindowManager::zOrder(QQuickItem* item) const
{
  if (m_windowZ.contains(item))
    return m_windowZ.value(item);

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Layout management
//--------------------------------------------------------------------------------------------------

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
  // Build a JSON object containing window order, mode, and geometries
  QJsonObject layout;
  if (!m_autoLayoutEnabled) {
    QJsonArray geometries;
    for (int id : m_windowOrder) {
      auto* win = m_windows.value(id);
      if (!win)
        continue;

      QJsonObject winGeom;
      winGeom["id"]     = id;
      winGeom["x"]      = win->x();
      winGeom["y"]      = win->y();
      winGeom["width"]  = win->width();
      winGeom["height"] = win->height();
      winGeom["state"]  = win->state();
      geometries.append(winGeom);
    }

    layout["geometries"] = geometries;
  }

  // Save window order and layout mode
  QJsonArray orderArray;
  for (int id : m_windowOrder)
    orderArray.append(id);

  layout["windowOrder"] = orderArray;
  layout["autoLayout"]  = m_autoLayoutEnabled;

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
bool UI::WindowManager::restoreLayout(const QJsonObject& layout)
{
  // Validate input
  if (layout.isEmpty())
    return false;

  bool autoLayout = layout["autoLayout"].toBool(true);

  // Restore window order, appending any unsaved windows at the end
  if (layout.contains("windowOrder")) {
    QJsonArray orderArray = layout["windowOrder"].toArray();
    QVector<int> newOrder;

    for (const auto& val : std::as_const(orderArray)) {
      int id = val.toInt(-1);
      if (id >= 0 && m_windows.contains(id))
        newOrder.append(id);
    }

    for (int id : std::as_const(m_windowOrder))
      if (!newOrder.contains(id))
        newOrder.append(id);

    m_windowOrder = newOrder;
  }

  // Restore individual window positions and sizes in manual mode
  if (!autoLayout && layout.contains("geometries")) {
    QJsonArray geometries = layout["geometries"].toArray();
    for (const auto& val : std::as_const(geometries)) {
      QJsonObject winGeom = val.toObject();
      int id              = winGeom["id"].toInt(-1);

      auto* win = m_windows.value(id);
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
    }

    constrainWindows();
    Q_EMIT geometryChanged(nullptr);
  }

  // Apply layout mode
  if (m_autoLayoutEnabled != autoLayout) {
    m_autoLayoutEnabled = autoLayout;
    Q_EMIT autoLayoutEnabledChanged();
  }

  // Tile windows or mark layout as restored
  if (autoLayout)
    loadLayout();
  else
    m_layoutRestored = true;

  return true;
}

/**
 * @brief Clears all tracked windows, z-order, and geometry.
 *        Resets the z-order counter.
 */
void UI::WindowManager::clear()
{
  // Reset all tracking state
  m_zCounter = 1;
  m_windowZ.clear();
  m_windows.clear();
  m_windowOrder.clear();
  m_dragWindow           = nullptr;
  m_targetWindow         = nullptr;
  m_resizeWindow         = nullptr;
  m_focusedWindow        = nullptr;
  m_layoutRestored       = false;
  m_snapIndicatorVisible = false;

  // Re-enable auto layout if it was disabled
  if (!m_autoLayoutEnabled) {
    m_autoLayoutEnabled = true;
    Q_EMIT autoLayoutEnabledChanged();
  }

  // Notify listeners of reset
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
  if (m_layoutRestored)
    constrainWindows();

  else if (autoLayoutEnabled())
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
  // Define margins and spacing for the tiling grid
  const int margin  = 4;
  const int spacing = -1;

  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  // Collect visible normal-state windows in display order
  QList<QQuickItem*> windows;
  for (int id : std::as_const(m_windowOrder)) {
    auto* win = m_windows.value(id);
    if (win && win->state() == "normal")
      windows.append(win);
  }

  if (windows.isEmpty())
    return;

  // Compute available space and orientation
  const int n            = windows.size();
  const int availW       = canvasW - 2 * margin;
  const int availH       = canvasH - 2 * margin;
  const bool isLandscape = availW >= availH;

  auto placeWindow = [](QQuickItem* win, int x, int y, int w, int h) {
    win->setX(x);
    win->setY(y);
    win->setWidth(w);
    win->setHeight(h);
  };

  // Single window fills the canvas
  if (n == 1)
    placeWindow(windows[0], margin, margin, availW, availH);

  // Two windows side-by-side or stacked
  else if (n == 2) {
    if (isLandscape) {
      int w = (availW - spacing) / 2;
      placeWindow(windows[0], margin, margin, w, availH);
      placeWindow(windows[1], margin + w + spacing, margin, w, availH);
    } else {
      int h = (availH - spacing) / 2;
      placeWindow(windows[0], margin, margin, availW, h);
      placeWindow(windows[1], margin, margin + h + spacing, availW, h);
    }
  }

  // Three windows in master-stack layout
  else if (n == 3) {
    if (isLandscape) {
      int masterW = availW / 2;
      int stackW  = availW - masterW - spacing;
      int stackH  = (availH - spacing) / 2;

      placeWindow(windows[0], margin, margin, masterW, availH);
      placeWindow(windows[1], margin + masterW + spacing, margin, stackW, stackH);
      placeWindow(
        windows[2], margin + masterW + spacing, margin + stackH + spacing, stackW, stackH);
    } else {
      int masterH = availH / 2;
      int stackH  = availH - masterH - spacing;
      int stackW  = (availW - spacing) / 2;

      placeWindow(windows[0], margin, margin, availW, masterH);
      placeWindow(windows[1], margin, margin + masterH + spacing, stackW, stackH);
      placeWindow(
        windows[2], margin + stackW + spacing, margin + masterH + spacing, stackW, stackH);
    }
  }

  // Four windows in 2x2 grid
  else if (n == 4) {
    int w = (availW - spacing) / 2;
    int h = (availH - spacing) / 2;

    placeWindow(windows[0], margin, margin, w, h);
    placeWindow(windows[1], margin + w + spacing, margin, w, h);
    placeWindow(windows[2], margin, margin + h + spacing, w, h);
    placeWindow(windows[3], margin + w + spacing, margin + h + spacing, w, h);
  }

  // Five windows in asymmetric 2+3 layout
  else if (n == 5) {
    if (isLandscape) {
      int topW = (availW - spacing) / 2;
      int botW = (availW - 2 * spacing) / 3;
      int h    = (availH - spacing) / 2;

      placeWindow(windows[0], margin, margin, topW, h);
      placeWindow(windows[1], margin + topW + spacing, margin, topW, h);
      placeWindow(windows[2], margin, margin + h + spacing, botW, h);
      placeWindow(windows[3], margin + botW + spacing, margin + h + spacing, botW, h);
      placeWindow(windows[4], margin + 2 * (botW + spacing), margin + h + spacing, botW, h);
    } else {
      int leftH  = (availH - spacing) / 2;
      int rightH = (availH - 2 * spacing) / 3;
      int w      = (availW - spacing) / 2;

      placeWindow(windows[0], margin, margin, w, leftH);
      placeWindow(windows[1], margin, margin + leftH + spacing, w, leftH);
      placeWindow(windows[2], margin + w + spacing, margin, w, rightH);
      placeWindow(windows[3], margin + w + spacing, margin + rightH + spacing, w, rightH);
      placeWindow(windows[4], margin + w + spacing, margin + 2 * (rightH + spacing), w, rightH);
    }
  }

  // Six windows in 3x2 or 2x3 grid
  else if (n == 6) {
    if (isLandscape) {
      int w = (availW - 2 * spacing) / 3;
      int h = (availH - spacing) / 2;

      for (int i = 0; i < 6; ++i) {
        int col = i % 3;
        int row = i / 3;
        placeWindow(windows[i], margin + col * (w + spacing), margin + row * (h + spacing), w, h);
      }
    } else {
      int w = (availW - spacing) / 2;
      int h = (availH - 2 * spacing) / 3;

      for (int i = 0; i < 6; ++i) {
        int col = i % 2;
        int row = i / 2;
        placeWindow(windows[i], margin + col * (w + spacing), margin + row * (h + spacing), w, h);
      }
    }
  }

  // Seven or more windows in optimal grid
  else {
    // Compute grid dimensions based on aspect ratio
    int cols, rows;
    if (isLandscape) {
      cols = qCeil(qSqrt(static_cast<double>(n) * availW / availH));
      rows = qCeil(static_cast<double>(n) / cols);
    } else {
      rows = qCeil(qSqrt(static_cast<double>(n) * availH / availW));
      cols = qCeil(static_cast<double>(n) / rows);
    }

    // Ensure enough cells for all windows
    while (cols * rows < n)
      if (isLandscape)
        cols++;
      else
        rows++;

    // Distribute available space evenly across cells
    const int spacingForSizing = qMax(spacing, 0);
    const int totalSpacingW    = (cols - 1) * spacingForSizing;
    const int totalSpacingH    = (rows - 1) * spacingForSizing;
    const int totalCellsW      = qMax(1, availW - totalSpacingW);
    const int totalCellsH      = qMax(1, availH - totalSpacingH);

    const int baseCellW = totalCellsW / cols;
    const int baseCellH = totalCellsH / rows;
    const int extraW    = totalCellsW % cols;
    const int extraH    = totalCellsH % rows;

    // Build column and row position tables
    QVector<int> colWidths(cols), colXs(cols);
    QVector<int> rowHeights(rows), rowYs(rows);

    int runningX = margin;
    for (int c = 0; c < cols; ++c) {
      colWidths[c] = baseCellW + (c < extraW ? 1 : 0);
      colXs[c]     = runningX;
      runningX += colWidths[c] + spacing;
    }

    int runningY = margin;
    for (int r = 0; r < rows; ++r) {
      rowHeights[r] = baseCellH + (r < extraH ? 1 : 0);
      rowYs[r]      = runningY;
      runningY += rowHeights[r] + spacing;
    }

    // Stretch partial last row to fill full width
    const int windowsInLastRow   = n - (rows - 1) * cols;
    const bool hasPartialLastRow = windowsInLastRow > 0 && windowsInLastRow < cols;
    QVector<int> lastRowWidths, lastRowXs;

    if (hasPartialLastRow) {
      const int totalSpacingLast = (windowsInLastRow - 1) * spacingForSizing;
      const int totalCellsLast   = qMax(1, availW - totalSpacingLast);
      const int baseLastW        = totalCellsLast / windowsInLastRow;
      const int extraLastW       = totalCellsLast % windowsInLastRow;

      lastRowWidths.resize(windowsInLastRow);
      lastRowXs.resize(windowsInLastRow);

      int runningLastX = margin;
      for (int c = 0; c < windowsInLastRow; ++c) {
        lastRowWidths[c] = baseLastW + (c < extraLastW ? 1 : 0);
        lastRowXs[c]     = runningLastX;
        runningLastX += lastRowWidths[c] + spacing;
      }
    }

    // Place each window in its grid cell
    for (int i = 0; i < n; ++i) {
      int row = i / cols;
      int col = i % cols;

      if (hasPartialLastRow && row == rows - 1)
        placeWindow(windows[i], lastRowXs[col], rowYs[row], lastRowWidths[col], rowHeights[row]);
      else
        placeWindow(windows[i], colXs[col], rowYs[row], colWidths[col], rowHeights[row]);
    }
  }

  // Ensure all windows are visible after tiling
  for (auto* win : std::as_const(m_windows)) {
    if (win && !win->isVisible()) {
      if (win->state() == "normal" || win->state() == "maximized")
        win->setVisible(true);
    }
  }

  Q_EMIT geometryChanged(nullptr);
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
  // Obtain canvas dimensions and validate
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  // Collect visible normal-state windows
  QList<QQuickItem*> visibleWindows;
  for (int id : std::as_const(m_windowOrder)) {
    auto* win = m_windows.value(id);
    if (win && win->state() == "normal")
      visibleWindows.append(win);
  }

  if (visibleWindows.isEmpty())
    return;

  // Cascade layout constants
  const int margin         = 8;
  const int cascadeOffsetX = 26;
  const int cascadeOffsetY = 26;

  const int availableW = canvasW - 2 * margin;
  const int availableH = canvasH - 2 * margin;

  // Position each window with diagonal offset
  for (int i = 0; i < visibleWindows.size(); ++i) {
    QQuickItem* win = visibleWindows[i];
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
    while (baseY + offsetY + winH > canvasH - margin && wrapCount < 10) {
      offsetY -= (availableH - winH);
      offsetX += cascadeOffsetX * 2;
      wrapCount++;
    }

    while (baseX + offsetX + winW > canvasW - margin && wrapCount < 20) {
      offsetX -= (availableW - winW);
      wrapCount++;
    }

    int winX = baseX + offsetX;
    int winY = baseY + offsetY;

    winX = qBound(margin, winX, canvasW - winW - margin);
    winY = qBound(margin, winY, canvasH - winH - margin);

    if (winW > availableW) {
      winW = availableW;
      winX = margin;
    }

    if (winH > availableH) {
      winH = availableH;
      winY = margin;
    }

    win->setX(winX);
    win->setY(winY);
    win->setWidth(winW);
    win->setHeight(winH);
  }

  // Ensure all windows are visible after cascading
  for (auto* win : std::as_const(m_windows)) {
    if (win && !win->isVisible()) {
      if (win->state() == "normal" || win->state() == "maximized")
        win->setVisible(true);
    }
  }

  Q_EMIT geometryChanged(nullptr);
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
  // Open a file dialog to let the user pick a wallpaper image
  auto* dialog = new QFileDialog(nullptr,
                                 tr("Select Background Image"),
                                 QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                 tr("Images (*.png *.jpg *.jpeg *.bmp)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  // Apply selected image and clean up dialog
  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
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
void UI::WindowManager::bringToFront(QQuickItem* item)
{
  if (!item)
    return;

  // Promote window and keep manager Z above all windows
  m_windowZ[item] = ++m_zCounter;
  item->setZ(m_windowZ[item]);
  setZ(m_zCounter + 2);

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
void UI::WindowManager::setTaskbar(QQuickItem* taskbar)
{
  m_taskbar = qobject_cast<UI::Taskbar*>(taskbar);
}

/**
 * @brief Registers a new window item with the manager, assigning initial
 *        z-order and geometry.
 *
 * @param item Pointer to the QQuickItem to register.
 */
void UI::WindowManager::registerWindow(const int id, QQuickItem* item)
{
  if (!item)
    return;

  // Track window and assign initial Z order
  m_windows[id] = item;
  m_windowOrder.append(id);
  m_windowZ[item] = ++m_zCounter;
  item->setZ(m_windowZ[item]);
  setZ(m_zCounter + 1);

  Q_EMIT zCounterChanged();
  Q_EMIT zOrderChanged(item);
}

/**
 * @brief Unregisters a window, removing its z-order and geometry tracking.
 *
 * After unregistering, triggers a layout update if auto-layout is enabled
 * to redistribute the remaining windows.
 *
 * @param item Pointer to the QQuickItem to remove.
 */
void UI::WindowManager::unregisterWindow(QQuickItem* item)
{
  // Remove window from tracking maps and order list
  m_windowZ.remove(item);
  m_windowOrder.removeAll(getIdForWindow(item));
  for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
    if (it.value() == item) {
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
void UI::WindowManager::setBackgroundImage(const QString& path)
{
  if (m_backgroundImage != path) {
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
  // Toggle layout mode and restore any maximized windows before re-tiling
  if (m_autoLayoutEnabled != enabled) {
    m_layoutRestored    = false;
    m_autoLayoutEnabled = enabled;
    Q_EMIT autoLayoutEnabledChanged();

    // Restore any maximized windows before re-tiling
    for (auto* win : std::as_const(m_windows))
      if (win->state() == "maximized")
        QMetaObject::invokeMethod(win, "restoreClicked");

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
  // Validate canvas dimensions
  const int canvasW = static_cast<int>(width());
  const int canvasH = static_cast<int>(height());

  if (canvasW <= 0 || canvasH <= 0)
    return;

  // Clamp each window to fit within canvas bounds
  for (auto* win : std::as_const(m_windows)) {
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

    if (winW > canvasW) {
      winW    = canvasW;
      changed = true;
    }

    if (winH > canvasH) {
      winH    = canvasH;
      changed = true;
    }

    if (winW < minW && canvasW >= minW) {
      winW    = minW;
      changed = true;
    }

    if (winH < minH && canvasH >= minH) {
      winH    = minH;
      changed = true;
    }

    if (winX < 0) {
      winX    = 0;
      changed = true;
    }

    if (winY < 0) {
      winY    = 0;
      changed = true;
    }

    if (winX + winW > canvasW) {
      winX = canvasW - winW;
      if (winX < 0) {
        winX = 0;
        winW = canvasW;
      }
      changed = true;
    }

    if (winY + winH > canvasH) {
      winY = canvasH - winH;
      if (winY < 0) {
        winY = 0;
        winH = canvasH;
      }
      changed = true;
    }

    if (changed) {
      win->setX(winX);
      win->setY(winY);
      win->setWidth(winW);
      win->setHeight(winH);
      Q_EMIT geometryChanged(win);
    }
  }

  // Ensure constrained windows remain visible
  for (auto* win : std::as_const(m_windows)) {
    if (win && !win->isVisible()) {
      if (win->state() == "normal" || win->state() == "maximized")
        win->setVisible(true);
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
  // Auto-layout mode: re-tile all windows
  if (autoLayoutEnabled())
    autoLayout();

  // Manual mode: cascade new windows or constrain existing ones
  else {
    bool hasUninitializedWindows = false;
    for (auto* win : std::as_const(m_windows)) {
      if (win && !win->isVisible() && (win->state() == "normal" || win->state() == "maximized")) {
        hasUninitializedWindows = true;
        break;
      }
    }

    if (hasUninitializedWindows && !m_layoutRestored)
      cascadeLayout();
    else
      constrainWindows();
  }
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
int UI::WindowManager::getIdForWindow(QQuickItem* item) const
{
  for (auto it = m_windows.constBegin(); it != m_windows.constEnd(); ++it)
    if (it.value() == item)
      return it.key();

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
int UI::WindowManager::determineNewIndexFromMousePos(const QPoint& pos) const
{
  // Hit-test to find the window under the cursor and return its order index
  QQuickItem* hoveredWindow = getWindow(pos.x(), pos.y());
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
QRect UI::WindowManager::extractGeometry(QQuickItem* item) const
{
  return QRect(item->x(),
               item->y(),
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
UI::WindowManager::ResizeEdge UI::WindowManager::detectResizeEdge(QQuickItem* target) const
{
  // Determine which edge or corner the cursor is near for resize
  if (target->state() == "normal") {
    // Map mouse position to window-local coordinates
    const int kResizeMargin = 8;
    QPointF localPos        = target->mapFromItem(this, m_initialMousePos);
    const int x             = static_cast<int>(localPos.x());
    const int y             = static_cast<int>(localPos.y());
    const int w             = static_cast<int>(target->width());
    const int h             = static_cast<int>(target->height());

    // Check proximity to each edge
    const bool nearLeft   = x <= kResizeMargin;
    const bool nearRight  = x >= w - kResizeMargin;
    const bool nearTop    = y <= kResizeMargin;
    const bool nearBottom = y >= h - kResizeMargin;

    // Return corner or edge closest to cursor
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
QQuickItem* UI::WindowManager::getWindow(const int x, const int y) const
{
  // Find the topmost visible window containing the given point
  QPointF point(x, y);
  QList<QQuickItem*> windows = m_windows.values();
  std::sort(
    windows.begin(), windows.end(), [](QQuickItem* a, QQuickItem* b) { return a->z() > b->z(); });

  // Return first visible window containing the point
  for (QQuickItem* window : std::as_const(windows)) {
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
void UI::WindowManager::mouseMoveEvent(QMouseEvent* event)
{
  // Compute movement delta from the initial press position
  const QPoint currentPos = event->pos();
  const QPoint delta      = currentPos - m_initialMousePos;
  int dragDistance        = delta.manhattanLength();

  // No focused window, pass through
  if (!m_focusedWindow) {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  // Skip geometry changes for non-normal windows
  if (m_focusedWindow->state() != "normal") {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  // Drag window to new position
  if (m_dragWindow && dragDistance >= 20) {
    int newX          = m_initialGeometry.x() + delta.x();
    int newY          = m_initialGeometry.y() + delta.y();
    int w             = static_cast<int>(m_dragWindow->width());
    int h             = static_cast<int>(m_dragWindow->height());
    const int canvasW = static_cast<int>(width());
    const int canvasH = static_cast<int>(height());

    // Shrink near-fullscreen window back to implicit size
    if ((w >= canvasW - 20 || h >= canvasH - 20) && !autoLayoutEnabled()) {
      w = static_cast<int>(m_dragWindow->implicitWidth());
      h = static_cast<int>(m_dragWindow->implicitHeight());
      m_dragWindow->setWidth(w);
      m_dragWindow->setHeight(h);
    }

    // Clamp and apply position
    newX = qBound(0, newX, canvasW - w);
    newY = qBound(0, newY, canvasH - h);
    m_dragWindow->setX(newX);
    m_dragWindow->setY(newY);

    // Show snap indicator for auto-layout reorder feedback
    if (autoLayoutEnabled()) {
      int targetIndex = determineNewIndexFromMousePos(currentPos);
      if (targetIndex >= 0 && targetIndex < m_windowOrder.size())
        m_targetWindow = m_windows.value(m_windowOrder[targetIndex]);

      // Snap to target window if hovering over a different one
      if (m_targetWindow && m_targetWindow != m_dragWindow) {
        m_dragWindow->setWidth(qMin(w, static_cast<int>(m_targetWindow->width())));
        m_dragWindow->setHeight(qMin(h, static_cast<int>(m_targetWindow->height())));
        m_snapIndicator        = extractGeometry(m_targetWindow);
        m_snapIndicatorVisible = true;
        Q_EMIT snapIndicatorChanged();
        event->accept();
        return;
      }

      // No valid target, hide snap indicator
      if (m_snapIndicatorVisible) {
        m_snapIndicatorVisible = false;
        Q_EMIT snapIndicatorChanged();
      }
    }

    // Manual layout: edge snap indicators
    else {
      const int top    = newY;
      const int left   = newX;
      const int right  = newX + w;
      const int bottom = newY + h;
      bool snapped     = false;

      // Top-left corner
      if (left <= 0 && top <= 0) {
        m_snapIndicator = QRect(0, 0, canvasW / 2, canvasH / 2);
        snapped         = true;
      }

      // Top-right corner
      else if (right >= canvasW && top <= 0) {
        m_snapIndicator = QRect(canvasW / 2, 0, canvasW / 2, canvasH / 2);
        snapped         = true;
      }

      // Bottom-left corner
      else if (left <= 0 && bottom >= canvasH) {
        m_snapIndicator = QRect(0, canvasH / 2, canvasW / 2, canvasH / 2);
        snapped         = true;
      }

      // Bottom-right corner
      else if (right >= canvasW && bottom >= canvasH) {
        m_snapIndicator = QRect(canvasW / 2, canvasH / 2, canvasW / 2, canvasH / 2);
        snapped         = true;
      }

      // Top edge = maximize
      else if (top <= 0) {
        m_snapIndicator = QRect(0, 0, canvasW, canvasH);
        snapped         = true;
      }

      // Left edge = left half
      else if (left <= 0) {
        m_snapIndicator = QRect(0, 0, canvasW / 2, canvasH);
        snapped         = true;
      }

      // Right edge = right half
      else if (right >= canvasW) {
        m_snapIndicator = QRect(canvasW / 2, 0, canvasW / 2, canvasH);
        snapped         = true;
      }

      if (snapped) {
        m_snapIndicatorVisible = true;
        Q_EMIT snapIndicatorChanged();
      }

      else if (m_snapIndicatorVisible) {
        m_snapIndicatorVisible = false;
        Q_EMIT snapIndicatorChanged();
      }
    }

    event->accept();
    return;
  }

  // Resize window along the active edge
  if (m_resizeWindow) {
    QRect geometry = m_initialGeometry;
    const int minW = m_resizeWindow->implicitWidth();
    const int minH = m_resizeWindow->implicitHeight();
    switch (m_resizeEdge) {
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

    // Clamp geometry to canvas bounds
    const QRect unclamped = geometry;
    geometry.setX(qMax(0, geometry.x()));
    geometry.setY(qMax(0, geometry.y()));
    if (geometry.right() > int(width()) - 1)
      geometry.setWidth(int(width()) - geometry.x());

    if (geometry.bottom() > int(height()) - 1)
      geometry.setHeight(int(height()) - geometry.y());

    // Only apply if clamping did not alter the result
    if (geometry == unclamped) {
      m_resizeWindow->setX(geometry.x());
      m_resizeWindow->setY(geometry.y());
      m_resizeWindow->setWidth(geometry.width());
      m_resizeWindow->setHeight(geometry.height());
      event->accept();
    }

    return;
  }

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
void UI::WindowManager::mousePressEvent(QMouseEvent* event)
{
  // Clear previous interaction state before hit-testing
  m_dragWindow      = nullptr;
  m_targetWindow    = nullptr;
  m_resizeWindow    = nullptr;
  m_focusedWindow   = nullptr;
  m_resizeEdge      = ResizeEdge::None;
  m_initialMousePos = event->pos();

  if (m_snapIndicatorVisible) {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

  // Hit-test for topmost window
  m_focusedWindow = getWindow(m_initialMousePos.x(), m_initialMousePos.y());
  if (!m_focusedWindow) {
    if (m_taskbar)
      m_taskbar->setActiveWindow(nullptr);

    if (event->button() == Qt::RightButton)
      Q_EMIT rightClicked(m_initialMousePos.x(), m_initialMousePos.y());

    return;
  }

  if (m_taskbar)
    m_taskbar->setActiveWindow(m_focusedWindow);

  // Determine if click is on the caption bar
  bool captionClick     = false;
  const int captionH    = m_focusedWindow->property("captionHeight").toInt();
  const int externcW    = m_focusedWindow->property("externControlWidth").toInt();
  const int buttonsW    = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto mouseClick = m_focusedWindow->mapFromItem(this, m_initialMousePos);
  if (mouseClick.y() <= captionH) {
    if (mouseClick.x() <= m_focusedWindow->width() - buttonsW && mouseClick.x() > externcW)
      captionClick = true;
    else {
      QQuickItem::mousePressEvent(event);
      return;
    }
  }

  // Start resize or drag for normal-state windows
  if (m_focusedWindow->state() == "normal") {
    m_resizeEdge = detectResizeEdge(m_focusedWindow);
    if (m_resizeEdge != ResizeEdge::None && !autoLayoutEnabled()) {
      grabMouse();
      switch (m_resizeEdge) {
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

      m_resizeWindow    = m_focusedWindow;
      m_initialGeometry = extractGeometry(m_focusedWindow);
      event->accept();
      return;
    }

    if (captionClick) {
      grabMouse();
      setCursor(Qt::ClosedHandCursor);
      m_dragWindow      = m_focusedWindow;
      m_initialGeometry = extractGeometry(m_focusedWindow);
      event->accept();
    }
  }

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
void UI::WindowManager::mouseReleaseEvent(QMouseEvent* event)
{
  // Finalize drag/resize: apply snap, swap order, or commit geometry
  if (autoLayoutEnabled()) {
    if (m_dragWindow && m_targetWindow && m_snapIndicatorVisible) {
      const int draggedId    = getIdForWindow(m_dragWindow);
      const int targetId     = getIdForWindow(m_targetWindow);
      const int newIndex     = m_windowOrder.indexOf(targetId);
      const int currentIndex = m_windowOrder.indexOf(draggedId);
      if (draggedId >= 0 && targetId >= 0 && currentIndex >= 0 && newIndex >= 0
          && newIndex != currentIndex)
        std::swap(m_windowOrder[currentIndex], m_windowOrder[newIndex]);
    }

    loadLayout();
  }

  // Manual layout: snap window to indicator region or emit geometry change
  else {
    if (m_dragWindow && m_snapIndicatorVisible) {
      auto x = m_snapIndicator.x();
      auto y = m_snapIndicator.y();
      auto w = m_snapIndicator.width();
      auto h = m_snapIndicator.height();

      if (x == 0 && y == 0 && w >= width() && h >= height())
        QMetaObject::invokeMethod(m_dragWindow, "maximizeClicked");

      else {
        m_dragWindow->setX(x);
        m_dragWindow->setY(y);
        m_dragWindow->setWidth(w);
        m_dragWindow->setHeight(h);
      }
    }

    if (m_dragWindow)
      Q_EMIT geometryChanged(m_dragWindow);

    else if (m_resizeWindow)
      Q_EMIT geometryChanged(m_resizeWindow);
  }

  // Release mouse and reset cursor
  ungrabMouse();
  unsetCursor();

  // Hide snap indicator
  if (m_snapIndicatorVisible) {
    m_snapIndicatorVisible = false;
    Q_EMIT snapIndicatorChanged();
  }

  // Clear all interaction state
  m_dragWindow      = nullptr;
  m_targetWindow    = nullptr;
  m_resizeWindow    = nullptr;
  m_focusedWindow   = nullptr;
  m_resizeEdge      = ResizeEdge::None;
  m_initialMousePos = event->pos();

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
void UI::WindowManager::mouseDoubleClickEvent(QMouseEvent* event)
{
  // Hit-test for a window under the cursor
  m_focusedWindow = getWindow(event->pos().x(), event->pos().y());
  if (!m_focusedWindow) {
    QQuickItem::mouseDoubleClickEvent(event);
    return;
  }

  // Check if double-click was in the title bar area (not on window buttons)
  const int captionH  = m_focusedWindow->property("captionHeight").toInt();
  const int externcW  = m_focusedWindow->property("externControlWidth").toInt();
  const int buttonsW  = m_focusedWindow->property("windowControlsWidth").toInt();
  const auto localPos = m_focusedWindow->mapFromItem(this, event->pos());
  if (localPos.y() <= captionH && localPos.x() <= m_focusedWindow->width() - buttonsW
      && localPos.x() > externcW) {
    // Obtain current state
    const QString state = m_focusedWindow->property("state").toString();

    // Restore the window
    if (state == "maximized")
      QMetaObject::invokeMethod(m_focusedWindow, "restoreClicked");

    // Maximize the window
    else if (state == "normal")
      QMetaObject::invokeMethod(m_focusedWindow, "maximizeClicked");

    // Block further processing of the event
    event->accept();
    return;
  }

  // Pass the event through
  QQuickItem::mouseDoubleClickEvent(event);
}
