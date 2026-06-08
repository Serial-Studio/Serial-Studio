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

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the widget registry with default counters and batch state.
 */
UI::WidgetRegistry::WidgetRegistry()
  : QObject(nullptr), m_nextId(1), m_batchDepth(0), m_batchHadChanges(false)
{}

/**
 * @brief Retrieves the singleton instance of the WidgetRegistry.
 */
UI::WidgetRegistry& UI::WidgetRegistry::instance()
{
  static WidgetRegistry instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Batch update control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Begins a batch update operation.
 */
void UI::WidgetRegistry::beginBatchUpdate()
{
  ++m_batchDepth;
}

/**
 * @brief Ends a batch update operation.
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

//--------------------------------------------------------------------------------------------------
// Widget management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new widget and assigns a unique ID.
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
 */
UI::WidgetInfo UI::WidgetRegistry::widgetInfo(UI::WidgetID id) const
{
  return m_widgets.value(id, WidgetInfo{});
}

/**
 * @brief Checks if a widget ID exists in the registry.
 */
bool UI::WidgetRegistry::contains(UI::WidgetID id) const
{
  return m_widgets.contains(id);
}

/**
 * @brief Gets all widget IDs in creation order.
 */
QVector<UI::WidgetID> UI::WidgetRegistry::allWidgetIds() const
{
  return m_widgetOrder;
}

/**
 * @brief Gets widget IDs filtered by type.
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
 */
void UI::WidgetRegistry::updateWidget(UI::WidgetID id,
                                      const QString& title,
                                      const QString& icon,
                                      const QVariant& userData)
{
  if (!m_widgets.contains(id))
    return;

  WidgetInfo& info = m_widgets[id];
  bool changed     = false;

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
