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

namespace UI
{

WidgetRegistry::WidgetRegistry()
  : QObject(nullptr)
{
}

WidgetRegistry &WidgetRegistry::instance()
{
  static WidgetRegistry instance;
  return instance;
}

WidgetID WidgetRegistry::createWidget(SerialStudio::DashboardWidget type,
                                      const QString &title, int groupId,
                                      int datasetIndex, bool isGroupWidget)
{
  WidgetInfo info;
  info.id = m_nextId++;
  info.type = type;
  info.title = title;
  info.groupId = groupId;
  info.datasetIndex = datasetIndex;
  info.isGroupWidget = isGroupWidget;

  m_widgetOrder.append(info.id);
  m_widgets.insert(info.id, info);

  if (m_batchDepth > 0)
    m_batchHadChanges = true;

  Q_EMIT widgetCreated(info.id, info);

  return info.id;
}

bool WidgetRegistry::destroyWidget(WidgetID id)
{
  if (!m_widgets.contains(id))
    return false;

  Q_EMIT widgetDestroyed(id);

  m_widgets.remove(id);
  m_widgetOrder.removeOne(id);

  if (m_batchDepth > 0)
    m_batchHadChanges = true;

  return true;
}

bool WidgetRegistry::updateWidget(WidgetID id, const QString &title,
                                  const QString &icon, const QVariant &userData)
{
  if (!m_widgets.contains(id))
    return false;

  WidgetInfo &info = m_widgets[id];

  bool changed = false;

  if (!title.isEmpty() && info.title != title)
  {
    info.title = title;
    changed = true;
  }

  if (!icon.isEmpty() && info.icon != icon)
  {
    info.icon = icon;
    changed = true;
  }

  if (userData.isValid())
  {
    info.userData = userData;
    changed = true;
  }

  if (changed)
    Q_EMIT widgetUpdated(id, info);

  return true;
}

WidgetInfo WidgetRegistry::widgetInfo(WidgetID id) const
{
  return m_widgets.value(id, WidgetInfo{});
}

bool WidgetRegistry::contains(WidgetID id) const
{
  return m_widgets.contains(id);
}

QVector<WidgetID> WidgetRegistry::allWidgetIds() const
{
  return m_widgetOrder;
}

QVector<WidgetID>
WidgetRegistry::widgetIdsByType(SerialStudio::DashboardWidget type) const
{
  QVector<WidgetID> result;
  for (const auto &id : m_widgetOrder)
  {
    if (m_widgets.value(id).type == type)
      result.append(id);
  }

  return result;
}

QVector<WidgetID> WidgetRegistry::widgetIdsByGroup(int groupId) const
{
  QVector<WidgetID> result;
  for (const auto &id : m_widgetOrder)
  {
    if (m_widgets.value(id).groupId == groupId)
      result.append(id);
  }

  return result;
}

WidgetID
WidgetRegistry::widgetIdByTypeAndIndex(SerialStudio::DashboardWidget type,
                                       int relativeIndex) const
{
  auto ids = widgetIdsByType(type);
  if (relativeIndex >= 0 && relativeIndex < ids.size())
    return ids[relativeIndex];

  return kInvalidWidgetId;
}

int WidgetRegistry::widgetCount() const
{
  return m_widgets.size();
}

void WidgetRegistry::clear()
{
  auto idsToDestroy = m_widgetOrder;
  std::reverse(idsToDestroy.begin(), idsToDestroy.end());

  for (const auto &id : idsToDestroy)
  {
    Q_EMIT widgetDestroyed(id);
    m_widgets.remove(id);
  }

  m_widgetOrder.clear();

  Q_EMIT registryCleared();
}

void WidgetRegistry::beginBatchUpdate()
{
  ++m_batchDepth;
}

void WidgetRegistry::endBatchUpdate()
{
  if (m_batchDepth > 0)
    --m_batchDepth;

  if (m_batchDepth == 0 && m_batchHadChanges)
  {
    m_batchHadChanges = false;
    Q_EMIT batchUpdateCompleted();
  }
}

bool WidgetRegistry::isInBatchUpdate() const
{
  return m_batchDepth > 0;
}

} // namespace UI
