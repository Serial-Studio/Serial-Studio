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

#include <QHash>
#include <QImage>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QWindow>

namespace CSD {
/**
 * @class Titlebar
 * @brief Custom title bar widget with window controls for CSD windows.
 *
 * Provides a painted title bar with:
 * - Application icon
 * - Centered window title
 * - Minimize, maximize, and close buttons
 * - Window dragging support
 * - Double-click to maximize/restore
 */
class Titlebar : public QQuickPaintedItem {
  Q_OBJECT

signals:
  void titleChanged();
  void closeClicked();
  void minimizeClicked();
  void maximizeClicked();
  void windowActiveChanged();
  void backgroundColorChanged();

public:
  explicit Titlebar(QQuickItem* parent = nullptr);

  void paint(QPainter* painter) override;

  [[nodiscard]] QString title() const;
  [[nodiscard]] bool windowActive() const;
  [[nodiscard]] QColor backgroundColor() const;

public slots:
  void setTitle(const QString& title);
  void setWindowActive(bool active);
  void setBackgroundColor(const QColor& color);

protected:
  void mouseUngrabEvent() override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void hoverMoveEvent(QHoverEvent* event) override;
  void hoverLeaveEvent(QHoverEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  enum class Button {
    None,
    Minimize,
    Maximize,
    Close
  };

  [[nodiscard]] QColor foregroundColor() const;
  [[nodiscard]] QRectF buttonRect(Button button) const;
  [[nodiscard]] Button buttonAt(const QPointF& pos) const;
  [[nodiscard]] bool shouldShowButton(Button button) const;
  [[nodiscard]] QRectF buttonBackgroundRect(Button button) const;

  [[nodiscard]] inline bool isMaximized() const
  {
    if (!window())
      return false;

    bool m = window()->windowStates() & Qt::WindowMaximized;
    bool f = window()->windowStates() & Qt::WindowFullScreen;
    return m || f;
  }

  void drawButton(QPainter* painter, Button button, const QString& svgPath);

private:
  QString m_title;
  QPixmap m_icon;
  bool m_dragging;
  bool m_windowActive;
  Button m_hoveredButton;
  Button m_pressedButton;
  QColor m_backgroundColor;
  QHash<QString, QPixmap> m_iconCache;
};

/**
 * @class Border
 * @brief Draws a 1px border around the entire CSD window.
 *
 * Renders a thin border on top of all content, including the titlebar.
 * Hidden when window is maximized or fullscreen.
 */
class Border : public QQuickPaintedItem {
  Q_OBJECT

public:
  explicit Border(QQuickItem* parent = nullptr);
  void paint(QPainter* painter) override;
};

/**
 * @class Frame
 * @brief Draws window shadow for CSD windows.
 *
 * Renders:
 * - Soft drop shadow around the content area
 *
 * Shadow is automatically disabled when window is maximized or fullscreen.
 */
class Frame : public QQuickPaintedItem {
  Q_OBJECT

signals:
  void borderColorChanged();
  void shadowRadiusChanged();
  void shadowEnabledChanged();

public:
  explicit Frame(QQuickItem* parent = nullptr);

  void paint(QPainter* painter) override;

  [[nodiscard]] int shadowRadius() const;
  [[nodiscard]] bool shadowEnabled() const;

public slots:
  void setShadowRadius(int radius);
  void setShadowEnabled(bool enabled);

private:
  void regenerateShadow();
  QImage generateShadowCorner(int size);

private:
  int m_shadowRadius;
  bool m_shadowEnabled;

  QImage m_shadowEdge;
  QImage m_shadowEdgeFlipped;
  QImage m_shadowEdgeVertical;
  QImage m_shadowEdgeVerticalFlipped;
  QImage m_shadowCorner;
  QImage m_shadowCornerFlippedH;
  QImage m_shadowCornerFlippedV;
  QImage m_shadowCornerFlippedHV;
};

/**
 * @class Window
 * @brief Manages client-side decorations for a QQuickWindow.
 *
 * Provides complete CSD functionality:
 * - Frameless window with custom title bar
 * - Window shadow and border
 * - Content container with automatic margins
 * - Window dragging and edge resizing
 * - Theme integration
 *
 * Usage:
 * @code
 * auto *decorator = new Window(quickWindow);
 * @endcode
 *
 * QML content is automatically reparented into a container that
 * respects shadow margins and title bar height.
 */
class Window : public QObject {
  Q_OBJECT

public:
  explicit Window(QWindow* window, const QString& color = QString(), QObject* parent = nullptr);
  ~Window() override;

  [[nodiscard]] Frame* frame() const;
  [[nodiscard]] QWindow* window() const;
  [[nodiscard]] Titlebar* titleBar() const;

  [[nodiscard]] int shadowMargin() const;
  [[nodiscard]] int titleBarHeight() const;

public slots:
  void updateTheme();
  void setColor(const QString& color);

private slots:
  void setupFrame();
  void setupBorder();
  void setupTitleBar();
  void updateMinimumSize();
  void updateFrameGeometry();
  void updateBorderGeometry();
  void onMinimumSizeChanged();
  void setupContentContainer();
  void updateTitleBarGeometry();
  void updateContentContainerGeometry();

private:
  QSize preferredSize() const;

private:
  enum class ResizeEdge {
    None        = 0,
    Left        = 1,
    Right       = 2,
    Top         = 4,
    Bottom      = 8,
    TopLeft     = Top | Left,
    TopRight    = Top | Right,
    BottomLeft  = Bottom | Left,
    BottomRight = Bottom | Right
  };

  [[nodiscard]] ResizeEdge edgeAt(const QPointF& pos) const;
  [[nodiscard]] Qt::CursorShape cursorForEdge(ResizeEdge edge) const;
  [[nodiscard]] Qt::Edges qtEdgesFromResizeEdge(ResizeEdge edge) const;

  void reparentChildToContainer(QQuickItem* child);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  bool m_resizing;
  Frame* m_frame;
  Border* m_border;
  QString m_color;
  Titlebar* m_titleBar;
  ResizeEdge m_resizeEdge;
  QSize m_minSize;
  QPointer<QWindow> m_window;
  QQuickItem* m_contentContainer;
};
}  // namespace CSD
