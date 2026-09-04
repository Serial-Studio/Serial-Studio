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

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QPointF>
#include <QQuickItem>
#include <QRect>
#include <QSettings>
#include <QSize>
#include <QVariant>
#include <QVector>

#include "UI/LayoutPatterns.h"
#include "UI/WindowManager/SnapOverlay.h"
#include "UI/WindowManager/WindowGeometry.h"
#include "UI/WindowManager/WindowLayoutStore.h"

namespace UI {
class Taskbar;
class Dashboard;
class UISessionRegistry;

/**
 * @brief Manages layout, geometry, z-ordering, and interactive manipulation of floating dashboard
 *        windows. The gesture visuals live in SnapOverlay and the manual-layout reference in
 *        WindowLayoutStore; this class owns the window registry, the interaction state machine and
 *        the QML surface, and forwards the overlay properties QML binds to.
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
  Q_PROPERTY(QRect fractionPreviewRect
             READ fractionPreviewRect
             NOTIFY fractionPreviewChanged)
  Q_PROPERTY(QString fractionPreviewLabel
             READ fractionPreviewLabel
             NOTIFY fractionPreviewChanged)
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
  Q_PROPERTY(QVariantMap mergedEdges
             READ mergedEdges
             NOTIFY mergedEdgesChanged)
  Q_PROPERTY(int layoutRatio
             READ layoutRatio
             NOTIFY layoutChoiceChanged)
  Q_PROPERTY(QString layoutPattern
             READ layoutPattern
             NOTIFY layoutChoiceChanged)
  // clang-format on

signals:
  void zCounterChanged();
  void frozenChanged();
  void gridSizeChanged();
  void gridEnabledChanged();
  void mergedEdgesChanged();
  void layoutChoiceChanged();
  void sizeMatchRectChanged();
  void manualGestureChanged();
  void fractionPreviewChanged();
  void snapIndicatorChanged();
  void alignmentGuidesChanged();
  void backgroundImageChanged();
  void autoLayoutEnabledChanged();
  void spacingIndicatorsChanged();
  void rightClicked(int x, int y);
  void zOrderChanged(QQuickItem* item);
  void geometryChanged(QQuickItem* item);

public:
  using ResizeEdge = WindowGeometry::ResizeEdge;

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
  [[nodiscard]] int layoutRatio() const;
  [[nodiscard]] const QString& layoutPattern() const;
  [[nodiscard]] const QVariantMap& mergedEdges() const;
  [[nodiscard]] bool sizeMatchVisible() const;
  [[nodiscard]] bool manualGestureActive() const;
  [[nodiscard]] const QRect& sizeMatchRect() const;
  [[nodiscard]] const QRect& fractionPreviewRect() const;
  [[nodiscard]] const QString& fractionPreviewLabel() const;
  [[nodiscard]] const QRect& manualGestureGeometry() const;
  [[nodiscard]] const QVariantList& alignmentGuides() const;
  [[nodiscard]] const QVariantList& spacingIndicators() const;

  [[nodiscard]] Q_INVOKABLE int zOrder(QQuickItem* item) const;
  [[nodiscard]] Q_INVOKABLE QVariantList layoutRatioStops() const;
  [[nodiscard]] Q_INVOKABLE bool patternHasPrimary(int pattern) const;
  [[nodiscard]] Q_INVOKABLE QVariantList
  patternPreview(int pattern, int count, int width, int height, int ratio) const;
  [[nodiscard]] Q_INVOKABLE QJsonObject serializeLayout() const;
  [[nodiscard]] Q_INVOKABLE bool restoreLayout(const QJsonObject& layout);
  [[nodiscard]] QMap<int, QString> savedWindowStates(const QJsonObject& layout) const;
  [[nodiscard]] int firstTileWindowId() const;
  [[nodiscard]] const QVector<int>& windowOrder() const;

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
  void setLayoutContext(const QString& key);
  void setAutoLayoutEnabled(const bool enabled);
  void selectLayoutPattern(const QString& pattern, const int ratio);
  void setFrozen(const bool frozen);
  void setGridEnabled(const bool enabled);
  void setGridSize(const int size);
  void registerWindow(const int id, QQuickItem* item);
  void reconcileWindowOrder(const QVector<int>& taskbarOrder);
  void preloadPendingGeometries(const QJsonObject& layout);

private:
  void refreshLayoutChoice();
  void computeMergedEdges();
  void applyResizeCursor(ResizeEdge edge);
  void commitManualGeometry(QQuickItem* window);
  void handleDragMove(QMouseEvent* event, const QPoint& delta);
  void handleResizeMove(QMouseEvent* event, const QPoint& delta);

  [[nodiscard]] QSize canvasSize() const;
  [[nodiscard]] bool tryReorderDraggedWindow();
  [[nodiscard]] bool startManualPress(const QPointF& pos, Qt::MouseButton button);

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

  SnapOverlay m_snapOverlay;
  WindowLayoutStore m_layoutStore;

  int m_zCounter;
  bool m_layoutRestored;
  bool m_autoLayoutEnabled;
  bool m_frozen;
  bool m_userReordered;
  bool m_suppressGeometrySignal;
  int m_lastCanvasWidth;
  int m_lastCanvasHeight;
  QString m_backgroundImage;

  QVector<int> m_windowOrder;
  QMap<int, QQuickItem*> m_windows;
  QMap<QQuickItem*, int> m_windowZ;

  ResizeEdge m_resizeEdge;

  QRect m_snapIndicator;
  bool m_snapIndicatorVisible;

  bool m_gridEnabled;
  int m_gridSize;
  int m_layoutRatio;
  QString m_layoutPattern;
  QString m_layoutContextKey;
  QVariantMap m_mergedEdges;

  QRect m_initialGeometry;
  QPoint m_initialMousePos;

  Taskbar* m_taskbar;
  QMetaObject::Connection m_workspaceConnection;
  QQuickItem* m_dragWindow;
  QQuickItem* m_targetWindow;
  QQuickItem* m_resizeWindow;
  QQuickItem* m_focusedWindow;

  QSettings m_settings;
};
}  // namespace UI
