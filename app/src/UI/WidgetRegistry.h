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
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

#include "SerialStudio.h"

namespace UI
{
using WidgetID = quint64;
constexpr WidgetID kInvalidWidgetId = 0;

struct WidgetInfo
{
  WidgetID id = kInvalidWidgetId;
  SerialStudio::DashboardWidget type = SerialStudio::DashboardNoWidget;

  QString title;
  QString icon;

  int groupId = -1;
  int datasetIndex = -1;

  bool isGroupWidget = false;

  QVariant userData;

  [[nodiscard]] bool isValid() const { return id != kInvalidWidgetId; }
};

/**
 * @class WidgetRegistry
 * @brief Central registry for all dashboard widgets.
 *
 * The WidgetRegistry maintains the authoritative list of all widgets in the
 * dashboard. It provides:
 *
 * - **Stable IDs**: Widget IDs are monotonically increasing and never reused
 *   during a session, ensuring QML components can maintain stable references
 *   even across rebuilds.
 *
 * - **Lifecycle events**: Signals for widget creation, destruction, and
 *   updates allow subscribers (Taskbar, WindowManager) to react incrementally
 *   rather than rebuilding their entire state.
 *
 * - **Decoupled state**: Widget metadata is stored separately from any
 *   presentation model, enabling independent updates.
 *
 * This design is inspired by window management systems like X11/Wayland
 * where window IDs are assigned by the display server and remain stable
 * throughout the window's lifetime.
 *
 * ## Usage Pattern
 *
 * ```cpp
 * // Dashboard creates widgets during frame processing
 * auto id = WidgetRegistry::instance().createWidget(
 *     SerialStudio::DashboardPlot, "Temperature", groupId, datasetIdx, false);
 *
 * // Taskbar listens for creation events
 * connect(&WidgetRegistry::instance(), &WidgetRegistry::widgetCreated,
 *         this, &Taskbar::onWidgetCreated);
 *
 * // WindowManager listens for destruction events
 * connect(&WidgetRegistry::instance(), &WidgetRegistry::widgetDestroyed,
 *         this, &WindowManager::onWidgetDestroyed);
 * ```
 */
class WidgetRegistry : public QObject
{
  Q_OBJECT

signals:
  void widgetCreated(UI::WidgetID id, const UI::WidgetInfo &info);
  void widgetDestroyed(UI::WidgetID id);
  void widgetUpdated(UI::WidgetID id, const UI::WidgetInfo &info);
  void registryCleared();
  void batchUpdateCompleted();

private:
  WidgetRegistry();
  ~WidgetRegistry() = default;

  WidgetRegistry(const WidgetRegistry &) = delete;
  WidgetRegistry &operator=(const WidgetRegistry &) = delete;

public:
  static WidgetRegistry &instance();

  [[nodiscard]] bool contains(WidgetID id) const;
  [[nodiscard]] int widgetCount() const;
  [[nodiscard]] bool isInBatchUpdate() const;

  [[nodiscard]] WidgetInfo widgetInfo(WidgetID id) const;
  [[nodiscard]] QVector<WidgetID> allWidgetIds() const;
  [[nodiscard]] QVector<WidgetID>
  widgetIdsByType(SerialStudio::DashboardWidget type) const;
  [[nodiscard]] QVector<WidgetID> widgetIdsByGroup(int groupId) const;
  [[nodiscard]] WidgetID
  widgetIdByTypeAndIndex(SerialStudio::DashboardWidget type,
                         int relativeIndex) const;

  WidgetID createWidget(SerialStudio::DashboardWidget type,
                        const QString &title, int groupId = -1,
                        int datasetIndex = -1, bool isGroupWidget = true);

  bool destroyWidget(WidgetID id);
  bool updateWidget(WidgetID id, const QString &title = QString(),
                    const QString &icon = QString(),
                    const QVariant &userData = QVariant());

  void clear();
  void beginBatchUpdate();
  void endBatchUpdate();

private:
  WidgetID m_nextId;
  int m_batchDepth;
  bool m_batchHadChanges;

  QVector<WidgetID> m_widgetOrder;
  QHash<WidgetID, WidgetInfo> m_widgets;
};

} // namespace UI
