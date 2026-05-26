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
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QMargins>
#include <QObject>
#include <QQuickItem>
#include <QRect>
#include <QSettings>
#include <QVector>

namespace detail {
struct StableKey;
}  // namespace detail

namespace UI {
class Taskbar;

/**
 * @brief Manages layout, geometry, z-ordering, and interactive manipulation of
 *        floating dashboard windows.
 */
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
  ~WindowManager();

  [[nodiscard]] int zCounter() const;
  [[nodiscard]] bool autoLayoutEnabled() const;
  [[nodiscard]] const QString& backgroundImage() const;

  [[nodiscard]] bool snapIndicatorVisible() const;
  [[nodiscard]] const QRect& snapIndicator() const;

  [[nodiscard]] Q_INVOKABLE int zOrder(QQuickItem* item) const;
  [[nodiscard]] Q_INVOKABLE QJsonObject serializeLayout() const;
  [[nodiscard]] Q_INVOKABLE bool restoreLayout(const QJsonObject& layout);
  [[nodiscard]] int firstTileWindowId() const;
  [[nodiscard]] const QVector<int>& windowOrder() const;

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
  void reconcileWindowOrder(const QVector<int>& taskbarOrder);
  void preloadPendingGeometries(const QJsonObject& layout);

private:
  [[nodiscard]] int getIdForWindow(QQuickItem* item) const;
  [[nodiscard]] QQuickItem* findOverlapTarget(const QRect& dragRect) const;
  [[nodiscard]] QVector<int> resolveSavedOrder(
    const QJsonObject& layout, const QHash<detail::StableKey, int>& stableLookup) const;

  [[nodiscard]] QRect extractGeometry(QQuickItem* item) const;
  [[nodiscard]] QVector<QQuickItem*> sortedByVisualStacking() const;
  [[nodiscard]] QQuickItem* topmostWindowAt(const QPointF& pos) const;
  [[nodiscard]] QQuickItem* manualResizeTargetAt(const QPointF& pos) const;
  [[nodiscard]] ResizeEdge detectResizeEdge(QQuickItem* target, const QPointF& pos) const;
  void applySavedGeometries(const QJsonObject& layout,
                            const QHash<detail::StableKey, int>& stableLookup,
                            int marginCanvasW,
                            int marginCanvasH);

  [[nodiscard]] bool startManualPress(const QPointF& pos, Qt::MouseButton button);
  void handleDragMove(QMouseEvent* event, const QPoint& delta);
  void handleResizeMove(QMouseEvent* event, const QPoint& delta);
  void applyManualAnchors(int newWidth, int newHeight);
  void applyManualSnap();
  [[nodiscard]] bool tryReorderDraggedWindow();
  void commitManualGeometry(QQuickItem* window);
  void storeManualGeometry(int id, QQuickItem* item, int canvasWidth = -1, int canvasHeight = -1);
  void updateManualSnapIndicator(int newX, int newY, int w, int h, int canvasW, int canvasH);
  [[nodiscard]] QRect computeResizedGeometry(const QPoint& delta) const;

protected:
  void hoverLeaveEvent(QHoverEvent* event) override;
  void hoverMoveEvent(QHoverEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  [[nodiscard]] bool childMouseEventFilter(QQuickItem* item, QEvent* event) override;

private:
  int m_zCounter;
  bool m_layoutRestored;
  bool m_autoLayoutEnabled;
  bool m_userReordered;
  bool m_suppressGeometrySignal;
  int m_manualCanvasWidth;
  int m_manualCanvasHeight;
  int m_lastCanvasWidth;
  int m_lastCanvasHeight;
  QString m_backgroundImage;

  QVector<int> m_windowOrder;
  QMap<int, QQuickItem*> m_windows;
  QMap<QQuickItem*, int> m_windowZ;
  QMap<int, QRect> m_manualGeometries;
  QMap<int, QMargins> m_manualMargins;
  QMap<int, QRect> m_pendingGeometries;

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
