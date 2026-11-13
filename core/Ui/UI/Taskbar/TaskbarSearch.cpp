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

#include "UI/Taskbar/TaskbarSearch.h"

#include <optional>
#include <QStandardItem>
#include <QVariantMap>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "UI/Taskbar/TaskbarModel.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Result cap of the search popup; the widget browser is deliberately uncapped.
constexpr int kMaxSearchResults = 30;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the search to the taskbar's full model, which stays owned by the taskbar.
 */
UI::TaskbarSearch::TaskbarSearch(TaskbarModel* model, QObject* parent)
  : QObject(parent), m_model(model)
{
  SS_ASSERT(model != nullptr, return);
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current search filter string.
 */
QString UI::TaskbarSearch::filter() const
{
  return m_filter;
}

/**
 * @brief Clears the search filter and reports the dismissal so the popup can close.
 */
void UI::TaskbarSearch::dismiss()
{
  m_filter.clear();
  Q_EMIT filterChanged();
  Q_EMIT resultsChanged();
  Q_EMIT dismissed();
}

/**
 * @brief Sets the search filter, which re-projects the results.
 */
void UI::TaskbarSearch::setFilter(const QString& filter)
{
  if (m_filter == filter)
    return;

  m_filter = filter;
  Q_EMIT filterChanged();
  Q_EMIT resultsChanged();
}

//--------------------------------------------------------------------------------------------------
// Projections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds one widget entry for the popup; the widget type is passed in so each caller keeps
 *        the QVariant type it always stored, and isGroupWidget is only emitted when given.
 */
[[nodiscard]] static QVariantMap makeWidgetEntry(const QStandardItem* item,
                                                 const QString& groupName,
                                                 const QVariant& widgetType,
                                                 std::optional<bool> isGroupWidget = std::nullopt)
{
  QVariantMap entry;
  entry[QStringLiteral("windowId")]    = item->data(UI::TaskbarModel::WindowIdRole);
  entry[QStringLiteral("widgetName")]  = item->data(UI::TaskbarModel::WidgetNameRole);
  entry[QStringLiteral("widgetIcon")]  = item->data(UI::TaskbarModel::WidgetIconRole);
  entry[QStringLiteral("widgetType")]  = widgetType;
  entry[QStringLiteral("groupName")]   = groupName;
  entry[QStringLiteral("groupId")]     = item->data(UI::TaskbarModel::GroupIdRole);
  entry[QStringLiteral("isWorkspace")] = false;
  if (isGroupWidget)
    entry[QStringLiteral("isGroupWidget")] = *isGroupWidget;

  return entry;
}

/**
 * @brief Returns a flat list of widgets matching the current search filter, capped so the popup
 *        stays a picker rather than a browser. An empty filter matches everything.
 */
QVariantList UI::TaskbarSearch::results() const
{
  QVariantList results;
  SS_ASSERT(m_model != nullptr, return results);

  const auto filter   = m_filter.trimmed();
  const bool noFilter = filter.isEmpty();

  for (int i = 0; i < m_model->rowCount() && results.size() < kMaxSearchResults; ++i) {
    auto* groupItem = m_model->item(i);
    if (!groupItem)
      continue;

    const auto groupName = groupItem->data(TaskbarModel::GroupNameRole).toString();

    const auto groupWidgetName = groupItem->data(TaskbarModel::WidgetNameRole).toString();
    const auto groupType       = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (groupType != SerialStudio::DashboardNoWidget
        && (noFilter || SerialStudio::searchMatches(filter, groupWidgetName)
            || SerialStudio::searchMatches(filter, groupName))) {
      results.append(makeWidgetEntry(groupItem, groupName, groupType, true));
    }

    for (int j = 0; j < groupItem->rowCount() && results.size() < kMaxSearchResults; ++j) {
      auto* child = groupItem->child(j);
      if (!child)
        continue;

      const auto childType = child->data(TaskbarModel::WidgetTypeRole).toInt();
      if (childType == SerialStudio::DashboardNoWidget)
        continue;

      const auto name = child->data(TaskbarModel::WidgetNameRole).toString();
      if (noFilter || SerialStudio::searchMatches(filter, name)
          || SerialStudio::searchMatches(filter, groupName)) {
        results.append(
          makeWidgetEntry(child, groupName, child->data(TaskbarModel::WidgetTypeRole), false));
      }
    }
  }

  return results;
}

/**
 * @brief Returns an unfiltered, unlimited flat list of every widget in the full model.
 */
QVariantList UI::TaskbarSearch::allWidgets() const
{
  QVariantList results;
  SS_ASSERT(m_model != nullptr, return results);

  for (int i = 0; i < m_model->rowCount(); ++i) {
    auto* groupItem = m_model->item(i);
    if (!groupItem)
      continue;

    const auto groupName = groupItem->data(TaskbarModel::GroupNameRole).toString();

    const auto groupType = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (groupType != SerialStudio::DashboardNoWidget)
      results.append(makeWidgetEntry(groupItem, groupName, groupType));

    for (int j = 0; j < groupItem->rowCount(); ++j) {
      auto* child = groupItem->child(j);
      if (!child)
        continue;

      const auto childType = child->data(TaskbarModel::WidgetTypeRole).toInt();
      if (childType == SerialStudio::DashboardNoWidget)
        continue;

      results.append(makeWidgetEntry(child, groupName, child->data(TaskbarModel::WidgetTypeRole)));
    }
  }

  return results;
}
