/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
#include <QRect>
#include <QSize>
#include <QString>
#include <QVector>

class QQuickItem;

namespace UI {
class Dashboard;

/**
 * @brief Owns the manual-layout reference and its on-disk form: the per-window rectangles a manual
 *        layout is re-derived from, the canvas size they were measured on, and the geometries still
 *        waiting for their window to register. Saved entries are keyed by the stable (widgetType,
 *        relativeIndex) identity, so a layout survives a project reload that renumbers windows.
 */
class WindowLayoutStore {
public:
  explicit WindowLayoutStore(UI::Dashboard& dashboard);
  WindowLayoutStore(WindowLayoutStore&&)                 = delete;
  WindowLayoutStore(const WindowLayoutStore&)            = delete;
  WindowLayoutStore& operator=(WindowLayoutStore&&)      = delete;
  WindowLayoutStore& operator=(const WindowLayoutStore&) = delete;

  [[nodiscard]] QSize referenceCanvas() const;

  [[nodiscard]] QJsonObject serialize(const QVector<int>& order,
                                      const QMap<int, QQuickItem*>& windows,
                                      const QSize& canvas,
                                      bool autoLayout,
                                      bool userReordered) const;
  [[nodiscard]] QMap<int, QString> savedWindowStates(const QJsonObject& layout,
                                                     const QMap<int, QQuickItem*>& windows) const;
  [[nodiscard]] QVector<int> resolveSavedOrder(const QJsonObject& layout,
                                               const QMap<int, QQuickItem*>& windows,
                                               const QVector<int>& currentOrder) const;

  void clear();
  void clearGeometries();
  void clearManualReference();
  void applyPendingGeometry(int id, QQuickItem* item);
  void storeManualGeometry(int id, QQuickItem* item, const QSize& canvas);
  void storeManualLayout(const QMap<int, QQuickItem*>& windows, const QSize& canvas);
  void preload(const QJsonObject& layout,
               const QMap<int, QQuickItem*>& windows,
               const QSize& canvas,
               int spacing);
  void applySavedGeometries(const QJsonObject& layout,
                            const QMap<int, QQuickItem*>& windows,
                            const QSize& savedCanvas,
                            const QSize& canvas,
                            int spacing);
  void applyManualLayout(const QMap<int, QQuickItem*>& windows,
                         const QSize& canvas,
                         const QSize& lastCanvas,
                         int spacing);

private:
  UI::Dashboard& m_dashboard;

  int m_manualCanvasWidth;
  int m_manualCanvasHeight;

  QMap<int, QRect> m_manualGeometries;
  QMap<int, QRect> m_pendingGeometries;
};

}  // namespace UI
