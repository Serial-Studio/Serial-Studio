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

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QQuickItem>
#include <QSettings>

namespace UI {
/**
 * @class WindowManager
 * @brief Manages layout, geometry, z-ordering, and interactive manipulation of
 *        floating dashboard windows.
 *
 * The WindowManager class provides comprehensive window management capabilities
 * for the dashboard's floating window system. It handles window registration,
 * tracks geometry and z-order, provides automatic layout algorithms, and
 * manages user interactions like dragging, resizing, and snapping.
 *
 * Key Features:
 * - **Window Registration**: Tracks QML window items with unique IDs
 * - **Z-Order Management**: Maintains window stacking order with
 *   bring-to-front functionality
 * - **Layout Algorithms**: Automatic grid layout and cascade layout modes
 * - **Drag & Drop**: Interactive window repositioning with visual feedback
 * - **Window Resizing**: Edge and corner-based window resizing
 * - **Snap Indicators**: Visual guides for window docking/alignment
 * - **Layout Persistence**: Serialization/deserialization of window layouts
 * - **Background Images**: Optional background image for the workspace
 * - **Constraint Enforcement**: Keeps windows within container bounds
 *
 * Layout Modes:
 * - **Auto Layout**: Intelligent grid-based arrangement that optimizes space
 *   usage
 * - **Cascade Layout**: Traditional cascading window arrangement
 * - **Manual Layout**: Free-form positioning with optional snap guides
 *
 * The WindowManager coordinates with the Taskbar for window visibility state
 * management and provides real-time geometry change notifications for layout
 * persistence.
 *
 * @note This class extends QQuickItem to receive mouse events for window
 *       manipulation. It should be used as the root container for all dashboard
 *       windows.
 */
class Taskbar;

class WindowManager : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int zCounter
             READ zCounter
             NOTIFY zCounterChanged)
  Q_PROPERTY(QString backgroundImage
             READ backgroundImage
             WRITE setBackgroundImage
             NOTIFY backgroundImageChanged)
  Q_PROPERTY(bool autoLayoutEnabled
             READ autoLayoutEnabled
             WRITE setAutoLayoutEnabled
             NOTIFY autoLayoutEnabledChanged)
  Q_PROPERTY(QRect snapIndicator
             READ snapIndicator
             NOTIFY snapIndicatorChanged)
  Q_PROPERTY(bool snapIndicatorVisible
             READ snapIndicatorVisible
             NOTIFY snapIndicatorChanged)
  // clang-format on

signals:
  void zCounterChanged();
  void snapIndicatorChanged();
  void backgroundImageChanged();
  void autoLayoutEnabledChanged();
  void rightClicked(int x, int y);
  void zOrderChanged(QQuickItem* item);
  void geometryChanged(QQuickItem* item);

public:
  WindowManager(QQuickItem* parent = nullptr);

  [[nodiscard]] int zCounter() const;
  [[nodiscard]] bool autoLayoutEnabled() const;
  [[nodiscard]] const QString& backgroundImage() const;

  [[nodiscard]] bool snapIndicatorVisible() const;
  [[nodiscard]] const QRect& snapIndicator() const;

  Q_INVOKABLE int zOrder(QQuickItem* item) const;
  Q_INVOKABLE QJsonObject serializeLayout() const;
  Q_INVOKABLE bool restoreLayout(const QJsonObject& layout);

  enum class ResizeEdge {
    None,
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
  };

public slots:
  void clear();
  void loadLayout();
  void autoLayout();
  void cascadeLayout();
  void constrainWindows();
  void triggerLayoutUpdate();
  void clearBackgroundImage();
  void selectBackgroundImage();
  void bringToFront(QQuickItem* item);
  void setTaskbar(QQuickItem* taskbar);
  void unregisterWindow(QQuickItem* item);
  void setBackgroundImage(const QString& path);
  void setAutoLayoutEnabled(const bool enabled);
  void registerWindow(const int id, QQuickItem* item);

private:
  int getIdForWindow(QQuickItem* item) const;
  int determineNewIndexFromMousePos(const QPoint& pos) const;

  QRect extractGeometry(QQuickItem* item) const;
  ResizeEdge detectResizeEdge(QQuickItem* target) const;
  QQuickItem* getWindow(const int x, const int y) const;

protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  int m_zCounter;
  bool m_layoutRestored;
  bool m_autoLayoutEnabled;
  QString m_backgroundImage;

  QVector<int> m_windowOrder;
  QMap<int, QQuickItem*> m_windows;
  QMap<QQuickItem*, int> m_windowZ;

  ResizeEdge m_resizeEdge;

  QRect m_snapIndicator;
  bool m_snapIndicatorVisible;

  QRect m_initialGeometry;
  QPoint m_initialMousePos;

  Taskbar* m_taskbar;
  QQuickItem* m_dragWindow;
  QQuickItem* m_targetWindow;
  QQuickItem* m_resizeWindow;
  QQuickItem* m_focusedWindow;

  QSettings m_settings;
};
}  // namespace UI
