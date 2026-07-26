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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QMargins>
#include <QObject>
#include <QPointF>
#include <QQuickItem>
#include <QRect>
#include <QSettings>
#include <QVariant>
#include <QVector>

namespace detail {
struct StableKey;
}  // namespace detail

namespace UI {
class Taskbar;
class Dashboard;
class UISessionRegistry;

namespace Snap {
struct SnapResult;
}  // namespace Snap

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
  Q_PROPERTY(bool frozen
             READ frozen
             WRITE setFrozen
             NOTIFY frozenChanged)
  Q_PROPERTY(QRect snapIndicator
             READ snapIndicator
             NOTIFY snapIndicatorChanged)
  Q_PROPERTY(bool snapIndicatorVisible
             READ snapIndicatorVisible
             NOTIFY snapIndicatorChanged)
  Q_PROPERTY(QVariantList alignmentGuides
             READ alignmentGuides
             NOTIFY alignmentGuidesChanged)
  Q_PROPERTY(QVariantList spacingIndicators
             READ spacingIndicators
             NOTIFY spacingIndicatorsChanged)
  Q_PROPERTY(QRect sizeMatchRect
             READ sizeMatchRect
             NOTIFY sizeMatchRectChanged)
  Q_PROPERTY(bool sizeMatchVisible
             READ sizeMatchVisible
             NOTIFY sizeMatchRectChanged)
  Q_PROPERTY(bool manualGestureActive
             READ manualGestureActive
             NOTIFY manualGestureChanged)
  Q_PROPERTY(QRect manualGestureGeometry
             READ manualGestureGeometry
             NOTIFY manualGestureChanged)
  Q_PROPERTY(bool gridEnabled
             READ gridEnabled
             WRITE setGridEnabled
             NOTIFY gridEnabledChanged)
  Q_PROPERTY(int gridSize
             READ gridSize
             WRITE setGridSize
             NOTIFY gridSizeChanged)
  // clang-format on

signals:
  void zCounterChanged();
  void frozenChanged();
  void gridSizeChanged();
  void gridEnabledChanged();
  void sizeMatchRectChanged();
  void manualGestureChanged();
  void snapIndicatorChanged();
  void alignmentGuidesChanged();
  void backgroundImageChanged();
  void autoLayoutEnabledChanged();
  void spacingIndicatorsChanged();
  void rightClicked(int x, int y);
  void zOrderChanged(QQuickItem* item);
  void geometryChanged(QQuickItem* item);

public:
  WindowManager(QQuickItem* parent = nullptr);
  ~WindowManager();

  [[nodiscard]] int zCounter() const;
  [[nodiscard]] bool frozen() const;
  [[nodiscard]] bool autoLayoutEnabled() const;
  [[nodiscard]] const QString& backgroundImage() const;

  [[nodiscard]] bool snapIndicatorVisible() const;
  [[nodiscard]] const QRect& snapIndicator() const;

  [[nodiscard]] int gridSize() const;
  [[nodiscard]] bool gridEnabled() const;
  [[nodiscard]] bool sizeMatchVisible() const;
  [[nodiscard]] bool manualGestureActive() const;
  [[nodiscard]] const QRect& sizeMatchRect() const;
  [[nodiscard]] const QRect& manualGestureGeometry() const;
  [[nodiscard]] const QVariantList& alignmentGuides() const;
  [[nodiscard]] const QVariantList& spacingIndicators() const;

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
  void updateHoverCursor(const QPointF& pos);
  void focusWindowUnderCursor(const QPointF& pos);
  void setBackgroundImage(const QString& path);
  void setAutoLayoutEnabled(const bool enabled);
  void setFrozen(const bool frozen);
  void setGridEnabled(const bool enabled);
  void setGridSize(const int size);
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

  void applyResizeCursor(ResizeEdge edge);
  void applySavedGeometries(const QJsonObject& layout,
                            const QHash<detail::StableKey, int>& stableLookup,
                            int marginCanvasW,
                            int marginCanvasH);

  [[nodiscard]] bool startManualPress(const QPointF& pos, Qt::MouseButton button);

  void handleDragMove(QMouseEvent* event, const QPoint& delta);
  void handleResizeMove(QMouseEvent* event, const QPoint& delta);
  void applyManualAnchors(int newWidth, int newHeight);

  void clearSnapGuides();
  void clearManualGesture();
  void cacheSnapSiblings(QQuickItem* target);
  void publishManualGesture(const QRect& geometry);
  void publishSnapGuides(const Snap::SnapResult& result);

  [[nodiscard]] bool tryReorderDraggedWindow();

  void commitManualGeometry(QQuickItem* window);
  void storeManualGeometry(int id, QQuickItem* item, int canvasWidth = -1, int canvasHeight = -1);

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
  UI::Dashboard& m_dashboard;
  UI::UISessionRegistry& m_sessionRegistry;

  int m_zCounter;
  bool m_layoutRestored;
  bool m_autoLayoutEnabled;
  bool m_frozen;
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

  bool m_gridEnabled;
  int m_gridSize;
  bool m_manualGestureActive;
  QRect m_manualGestureGeometry;
  QRect m_sizeMatchRect;
  QVariantList m_alignmentGuides;
  QVariantList m_spacingIndicators;
  QVector<QRect> m_snapSiblings;

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
