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
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QQuickItem>

namespace UI
{
/**
 * @class WindowManager
 * @brief Manages the layout, geometry, and z-order of QML-based floating
 *        windows.
 *
 * This class tracks registered QQuickItems representing windows, handles their
 * automatic layout, manages stacking order (z-index), and maintains their
 * geometry within a defined container.
 *
 * It also supports background image configuration for the window canvas.
 */
class Taskbar;
class WindowManager : public QQuickItem
{
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
  void zOrderChanged(QQuickItem *item);
  void geometryChanged(QQuickItem *item);

public:
  WindowManager(QQuickItem *parent = nullptr);

  [[nodiscard]] int zCounter() const;
  [[nodiscard]] bool autoLayoutEnabled() const;
  [[nodiscard]] const QString &backgroundImage() const;

  [[nodiscard]] bool snapIndicatorVisible() const;
  [[nodiscard]] const QRect &snapIndicator() const;

  Q_INVOKABLE int zOrder(QQuickItem *item) const;

  enum class ResizeEdge
  {
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
  void triggerLayoutUpdate();
  void clearBackgroundImage();
  void selectBackgroundImage();
  void bringToFront(QQuickItem *item);
  void setTaskbar(QQuickItem *taskbar);
  void unregisterWindow(QQuickItem *item);
  void setBackgroundImage(const QString &path);
  void setAutoLayoutEnabled(const bool enabled);
  void registerWindow(const int id, QQuickItem *item);

private:
  int getIdForWindow(QQuickItem *item) const;
  int determineNewIndexFromMousePos(const QPoint &pos) const;

  QRect extractGeometry(QQuickItem *item) const;
  ResizeEdge detectResizeEdge(QQuickItem *target) const;
  QQuickItem *getWindow(const int x, const int y) const;

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  int m_zCounter;
  bool m_autoLayoutEnabled;
  QString m_backgroundImage;

  QVector<int> m_windowOrder;
  QMap<int, QQuickItem *> m_windows;
  QMap<QQuickItem *, int> m_windowZ;

  ResizeEdge m_resizeEdge;

  QRect m_snapIndicator;
  bool m_snapIndicatorVisible;

  QRect m_initialGeometry;
  QPoint m_initialMousePos;

  Taskbar *m_taskbar;
  QQuickItem *m_dragWindow;
  QQuickItem *m_targetWindow;
  QQuickItem *m_resizeWindow;
  QQuickItem *m_focusedWindow;

  QSettings m_settings;
};
} // namespace UI
