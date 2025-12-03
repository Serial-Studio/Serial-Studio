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
/**
 * @brief Unique identifier for dashboard widgets.
 *
 * Widget IDs are monotonically increasing and never reused during a session.
 * This ensures stable references even when widgets are added or removed.
 * Similar to PIDs in operating systems or window IDs in X11/Wayland.
 */
using WidgetID = quint64;

/**
 * @brief Invalid widget ID constant.
 */
constexpr WidgetID kInvalidWidgetId = 0;

/**
 * @brief Represents a single widget instance in the dashboard.
 *
 * This structure decouples widget identity from its position in any model.
 * A widget maintains its ID throughout its lifecycle, regardless of how
 * the dashboard is reorganized.
 */
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
 * @brief Central registry for all dashboard widgets.
 *
 * The WidgetRegistry maintains the authoritative list of all widgets in the
 * dashboard. It provides:
 *
 * - **Stable IDs**: Widget IDs are never reused during a session, ensuring
 *   QML components can maintain stable references even across rebuilds.
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
  /**
   * @brief Emitted when a new widget is created.
   * @param id The unique ID of the new widget.
   * @param info Complete information about the new widget.
   */
  void widgetCreated(UI::WidgetID id, const UI::WidgetInfo &info);

  /**
   * @brief Emitted when a widget is about to be destroyed.
   * @param id The ID of the widget being destroyed.
   */
  void widgetDestroyed(UI::WidgetID id);

  /**
   * @brief Emitted when widget metadata changes (title, icon, etc.).
   * @param id The ID of the modified widget.
   * @param info Updated widget information.
   */
  void widgetUpdated(UI::WidgetID id, const UI::WidgetInfo &info);

  /**
   * @brief Emitted when all widgets are cleared.
   */
  void registryCleared();

  /**
   * @brief Emitted after a batch of widgets has been added/removed.
   *
   * This is useful for subscribers that want to wait until all widgets
   * from a frame update are registered before doing expensive operations
   * like layout recalculation.
   */
  void batchUpdateCompleted();

public:
  static WidgetRegistry &instance();

  /**
   * @brief Creates a new widget and assigns a unique ID.
   *
   * The ID is guaranteed to be unique and will not be reused during this
   * session, even after the widget is destroyed.
   *
   * @param type The widget type (from SerialStudio::DashboardWidget).
   * @param title Display title for the widget.
   * @param groupId The parent group ID (-1 for standalone widgets like
   * terminal).
   * @param datasetIndex Dataset index for dataset widgets (-1 for group
   * widgets).
   * @param isGroupWidget True if this is a group-level widget.
   * @return The newly assigned widget ID.
   */
  WidgetID createWidget(SerialStudio::DashboardWidget type,
                        const QString &title, int groupId = -1,
                        int datasetIndex = -1, bool isGroupWidget = true);

  /**
   * @brief Destroys a widget and removes it from the registry.
   *
   * Note: The ID will NOT be reused during this session to prevent
   * dangling references.
   *
   * @param id The widget ID to destroy.
   * @return True if the widget existed and was destroyed.
   */
  bool destroyWidget(WidgetID id);

  /**
   * @brief Updates widget metadata.
   * @param id The widget ID to update.
   * @param title New title (empty string keeps current title).
   * @param icon New icon path (empty string keeps current icon).
   * @param userData Optional user data to associate with widget.
   * @return True if the widget exists and was updated.
   */
  bool updateWidget(WidgetID id, const QString &title = QString(),
                    const QString &icon = QString(),
                    const QVariant &userData = QVariant());

  /**
   * @brief Retrieves widget information by ID.
   * @param id The widget ID.
   * @return Widget info, or invalid info (id=0) if not found.
   */
  [[nodiscard]] WidgetInfo widgetInfo(WidgetID id) const;

  /**
   * @brief Checks if a widget ID exists in the registry.
   * @param id The widget ID to check.
   * @return True if the widget exists.
   */
  [[nodiscard]] bool contains(WidgetID id) const;

  /**
   * @brief Gets all widget IDs in creation order.
   * @return Vector of all registered widget IDs.
   */
  [[nodiscard]] QVector<WidgetID> allWidgetIds() const;

  /**
   * @brief Gets widget IDs filtered by type.
   * @param type The widget type to filter by.
   * @return Vector of matching widget IDs in creation order.
   */
  [[nodiscard]] QVector<WidgetID>
  widgetIdsByType(SerialStudio::DashboardWidget type) const;

  /**
   * @brief Gets widget IDs filtered by group.
   * @param groupId The group ID to filter by.
   * @return Vector of matching widget IDs in creation order.
   */
  [[nodiscard]] QVector<WidgetID> widgetIdsByGroup(int groupId) const;

  /**
   * @brief Gets the Nth widget ID of a specific type.
   * @param type The widget type.
   * @param relativeIndex The index among widgets of this type.
   * @return The widget ID, or kInvalidWidgetId if not found.
   */
  [[nodiscard]] WidgetID
  widgetIdByTypeAndIndex(SerialStudio::DashboardWidget type,
                         int relativeIndex) const;

  /**
   * @brief Gets the total number of registered widgets.
   */
  [[nodiscard]] int widgetCount() const;

  /**
   * @brief Clears all widgets from the registry.
   *
   * This emits widgetDestroyed for each widget (in reverse creation order),
   * then registryCleared.
   */
  void clear();

  /**
   * @brief Begins a batch update operation.
   *
   * During batch mode, individual widgetCreated/widgetDestroyed signals
   * are still emitted immediately, but batchUpdateCompleted() is deferred
   * until endBatchUpdate() is called.
   *
   * This allows subscribers to defer expensive operations (like layout
   * recalculation) until all changes are complete.
   *
   * Batch operations can be nested; only the outermost endBatchUpdate()
   * will emit batchUpdateCompleted().
   */
  void beginBatchUpdate();

  /**
   * @brief Ends a batch update operation.
   *
   * Emits batchUpdateCompleted() if this is the outermost batch and
   * any widgets were added or removed during the batch.
   */
  void endBatchUpdate();

  /**
   * @brief Checks if currently in a batch update.
   */
  [[nodiscard]] bool isInBatchUpdate() const;

private:
  WidgetRegistry();
  ~WidgetRegistry() = default;

  WidgetRegistry(const WidgetRegistry &) = delete;
  WidgetRegistry &operator=(const WidgetRegistry &) = delete;

  WidgetID m_nextId = 1;
  int m_batchDepth = 0;
  bool m_batchHadChanges = false;

  QVector<WidgetID> m_widgetOrder;
  QHash<WidgetID, WidgetInfo> m_widgets;
};

} // namespace UI
