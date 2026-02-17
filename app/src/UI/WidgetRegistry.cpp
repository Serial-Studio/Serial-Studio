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

#include "UI/WidgetRegistry.h"

/**
 * @brief Constructor function
 */
UI::WidgetRegistry::WidgetRegistry()
  : QObject(nullptr), m_nextId(1), m_batchDepth(0), m_batchHadChanges(false)
{}

/**
 * @brief Retrieves the singleton instance of the WidgetRegistry.
 * @return Reference to the singleton WidgetRegistry instance.
 */
UI::WidgetRegistry& UI::WidgetRegistry::instance()
{
  static WidgetRegistry instance;
  return instance;
}

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
void UI::WidgetRegistry::beginBatchUpdate()
{
  ++m_batchDepth;
}

/**
 * @brief Ends a batch update operation.
 *
 * Emits batchUpdateCompleted() if this is the outermost batch and
 * any widgets were added or removed during the batch.
 */
void UI::WidgetRegistry::endBatchUpdate()
{
  if (m_batchDepth > 0)
    --m_batchDepth;

  if (m_batchDepth == 0 && m_batchHadChanges) {
    m_batchHadChanges = false;
    Q_EMIT batchUpdateCompleted();
  }
}

/**
 * @brief Checks if currently in a batch update.
 */
bool UI::WidgetRegistry::isInBatchUpdate() const
{
  return m_batchDepth > 0;
}

/**
 * @brief Creates a new widget and assigns a unique ID.
 *
 * The ID is guaranteed to be unique and will not be reused during this
 * session, even after the widget is destroyed.
 *
 * @param type The widget type (from SerialStudio::DashboardWidget).
 * @param title Display title for the widget.
 * @param groupId The parent group ID (-1 for standalone widgets like terminal).
 * @param datasetIndex Dataset index for dataset widgets (-1 for group widgets).
 * @param isGroupWidget True if this is a group-level widget.
 * @return The newly assigned widget ID.
 */
UI::WidgetID UI::WidgetRegistry::createWidget(SerialStudio::DashboardWidget type,
                                              const QString& title,
                                              int groupId,
                                              int datasetIndex,
                                              bool isGroupWidget)
{
  WidgetInfo info;
  info.id            = m_nextId++;
  info.type          = type;
  info.title         = title;
  info.groupId       = groupId;
  info.datasetIndex  = datasetIndex;
  info.isGroupWidget = isGroupWidget;

  m_widgetOrder.append(info.id);
  m_widgets.insert(info.id, info);

  if (m_batchDepth > 0)
    m_batchHadChanges = true;

  Q_EMIT widgetCreated(info.id, info);

  return info.id;
}

/**
 * @brief Retrieves widget information by ID.
 * @param id The widget ID.
 * @return Widget info, or invalid info (id=0) if not found.
 */
UI::WidgetInfo UI::WidgetRegistry::widgetInfo(UI::WidgetID id) const
{
  return m_widgets.value(id, WidgetInfo{});
}

/**
 * @brief Checks if a widget ID exists in the registry.
 * @param id The widget ID to check.
 * @return True if the widget exists.
 */
bool UI::WidgetRegistry::contains(UI::WidgetID id) const
{
  return m_widgets.contains(id);
}

/**
 * @brief Gets all widget IDs in creation order.
 * @return Vector of all registered widget IDs.
 */
QVector<UI::WidgetID> UI::WidgetRegistry::allWidgetIds() const
{
  return m_widgetOrder;
}

/**
 * @brief Gets widget IDs filtered by type.
 * @param type The widget type to filter by.
 * @return Vector of matching widget IDs in creation order.
 */
QVector<UI::WidgetID> UI::WidgetRegistry::widgetIdsByType(SerialStudio::DashboardWidget type) const
{
  QVector<WidgetID> result;
  for (const auto& id : m_widgetOrder)
    if (m_widgets.value(id).type == type)
      result.append(id);

  return result;
}

/**
 * @brief Gets widget IDs filtered by group.
 * @param groupId The group ID to filter by.
 * @return Vector of matching widget IDs in creation order.
 */
QVector<UI::WidgetID> UI::WidgetRegistry::widgetIdsByGroup(int groupId) const
{
  QVector<WidgetID> result;
  for (const auto& id : m_widgetOrder)
    if (m_widgets.value(id).groupId == groupId)
      result.append(id);

  return result;
}

/**
 * @brief Gets the Nth widget ID of a specific type.
 * @param type The widget type.
 * @param relativeIndex The index among widgets of this type.
 * @return The widget ID, or kInvalidWidgetId if not found.
 */
UI::WidgetID UI::WidgetRegistry::widgetIdByTypeAndIndex(SerialStudio::DashboardWidget type,
                                                        int relativeIndex) const
{
  auto ids = widgetIdsByType(type);
  if (relativeIndex >= 0 && relativeIndex < ids.size())
    return ids[relativeIndex];

  return UI::kInvalidWidgetId;
}

/**
 * @brief Gets the total number of registered widgets.
 */
int UI::WidgetRegistry::widgetCount() const
{
  return m_widgets.size();
}

/**
 * @brief Clears all widgets from the registry.
 *
 * This emits widgetDestroyed for each widget (in reverse creation order),
 * then registryCleared.
 */
void UI::WidgetRegistry::clear()
{
  auto idsToDestroy = m_widgetOrder;
  std::reverse(idsToDestroy.begin(), idsToDestroy.end());

  for (const auto& id : idsToDestroy) {
    Q_EMIT widgetDestroyed(id);
    m_widgets.remove(id);
  }

  m_widgetOrder.clear();

  Q_EMIT registryCleared();
}

/**
 * @brief Destroys a widget and removes it from the registry.
 *
 * Note: The ID will NOT be reused during this session to prevent
 * dangling references.
 *
 * @param id The widget ID to destroy.
 * @return True if the widget existed and was destroyed.
 */
void UI::WidgetRegistry::destroyWidget(UI::WidgetID id)
{
  if (!m_widgets.contains(id))
    return;

  Q_EMIT widgetDestroyed(id);

  m_widgets.remove(id);
  m_widgetOrder.removeOne(id);

  if (m_batchDepth > 0)
    m_batchHadChanges = true;
}

/**
 * @brief Updates widget metadata.
 * @param id The widget ID to update.
 * @param title New title (empty string keeps current title).
 * @param icon New icon path (empty string keeps current icon).
 * @param userData Optional user data to associate with widget.
 */
void UI::WidgetRegistry::updateWidget(UI::WidgetID id,
                                      const QString& title,
                                      const QString& icon,
                                      const QVariant& userData)
{
  if (!m_widgets.contains(id))
    return;

  WidgetInfo& info = m_widgets[id];

  bool changed = false;

  if (!title.isEmpty() && info.title != title) {
    info.title = title;
    changed    = true;
  }

  if (!icon.isEmpty() && info.icon != icon) {
    info.icon = icon;
    changed   = true;
  }

  if (userData.isValid()) {
    info.userData = userData;
    changed       = true;
  }

  if (changed)
    Q_EMIT widgetUpdated(id, info);

  return;
}
