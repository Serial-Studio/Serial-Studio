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

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QWindow>

/**
 * @class CSD_Titlebar
 * @brief A QQuickPaintedItem that renders the window title bar and handles
 *        mouse interactions for window management.
 */
class CSD_Titlebar : public QQuickPaintedItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QColor backgroundColor
             READ backgroundColor
             WRITE setBackgroundColor
             NOTIFY backgroundColorChanged)
  Q_PROPERTY(QString title
             READ title
             WRITE setTitle
             NOTIFY titleChanged)
  Q_PROPERTY(bool windowActive
             READ windowActive
             NOTIFY windowActiveChanged)
  // clang-format on

signals:
  void titleChanged();
  void closeClicked();
  void minimizeClicked();
  void maximizeClicked();
  void windowActiveChanged();
  void backgroundColorChanged();

public:
  explicit CSD_Titlebar(QQuickItem *parent = nullptr);

  void paint(QPainter *painter) override;

  [[nodiscard]] QString title() const;
  [[nodiscard]] bool windowActive() const;
  [[nodiscard]] QColor backgroundColor() const;
  [[nodiscard]] QColor foregroundColor() const;

public slots:
  void setWindowActive(bool active);
  void setTitle(const QString &title);
  void setBackgroundColor(const QColor &color);

private:
  enum class Button
  {
    None,
    Minimize,
    Maximize,
    Close
  };

  [[nodiscard]] QRectF buttonRect(Button button) const;
  [[nodiscard]] Button buttonAt(const QPointF &pos) const;

  void drawButton(QPainter *painter, Button button, const QString &svgPath);

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void hoverMoveEvent(QHoverEvent *event) override;
  void hoverLeaveEvent(QHoverEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  QPixmap m_icon;
  QString m_title;
  bool m_dragging;
  bool m_windowActive;
  Button m_hoveredButton;
  Button m_pressedButton;
  QPointF m_dragStartPos;
  QColor m_backgroundColor;
};

/**
 * @class CSD_Window
 * @brief Manages client-side decorations for a QWindow on Unix systems.
 *
 * This class wraps a QWindow and provides:
 * - Custom title bar rendering via TitleBarItem
 * - Window dragging and resizing
 * - Minimize/maximize/close button handling
 * - Theme-aware coloring
 */
class CSD_Window : public QObject
{
  Q_OBJECT

public:
  explicit CSD_Window(QWindow *window, const QString &color = "",
                      QObject *parent = nullptr);
  ~CSD_Window() override;

  [[nodiscard]] QWindow *window() const;
  [[nodiscard]] int titleBarHeight() const;
  [[nodiscard]] CSD_Titlebar *titleBar() const;

public slots:
  void updateTheme();
  void setColor(const QString &color);

private slots:
  void setupTitleBar();
  void updateTitleBarGeometry();

private:
  enum class ResizeEdge
  {
    None = 0,
    Left = 1,
    Right = 2,
    Top = 4,
    Bottom = 8,
    TopLeft = Top | Left,
    TopRight = Top | Right,
    BottomLeft = Bottom | Left,
    BottomRight = Bottom | Right
  };

  ResizeEdge edgeAt(const QPointF &pos) const;
  Qt::CursorShape cursorForEdge(ResizeEdge edge) const;
  Qt::Edges qtEdgesFromResizeEdge(ResizeEdge edge) const;

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  bool m_resizing;
  QString m_color;
  ResizeEdge m_resizeEdge;
  CSD_Titlebar *m_titleBar;
  QPointer<QWindow> m_window;
};
