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

#include "DataModel/RowFilterProxy.h"

#include "Core/SSAssert.h"
#include "DataModel/ProjectEditor.h"
#include "SerialStudio.h"

/**
 * @brief Constructs a pass-through proxy; the default role set covers the project editor form
 *        rows (parameter label, section/placeholder text, tooltip description).
 */
DataModel::RowFilterProxy::RowFilterProxy(QObject* parent)
  : QSortFilterProxyModel(parent)
  , m_query()
  , m_filterRoles({QStringLiteral("parameterName"), QStringLiteral("placeholderValue")})
  , m_roleIds()
  , m_widgetTypeRole(-1)
{}

/**
 * @brief Returns the free-text query; empty means every row is accepted.
 */
QString DataModel::RowFilterProxy::query() const
{
  return m_query;
}

/**
 * @brief Returns the role names the query is matched against.
 */
QStringList DataModel::RowFilterProxy::filterRoles() const
{
  return m_filterRoles;
}

/**
 * @brief Attaches @p model, resolving the role cache first so the repopulation that the base
 *        setSourceModel triggers already filters with the new model's role ids.
 */
void DataModel::RowFilterProxy::setSourceModel(QAbstractItemModel* model)
{
  rebuildRoleCache(model);
  QSortFilterProxyModel::setSourceModel(model);
}

/**
 * @brief Sets the query and re-filters; guard-returns when unchanged.
 */
void DataModel::RowFilterProxy::setQuery(const QString& query)
{
  if (m_query == query)
    return;

  beginFilterChange();
  m_query = query;
  endFilterChange(Direction::Rows);
  Q_EMIT queryChanged();
}

/**
 * @brief Replaces the matched role-name set and re-resolves it against the source model.
 */
void DataModel::RowFilterProxy::setFilterRoles(const QStringList& roles)
{
  if (m_filterRoles == roles)
    return;

  beginFilterChange();
  m_filterRoles = roles;
  rebuildRoleCache(sourceModel());
  endFilterChange(Direction::Rows);
  Q_EMIT filterRolesChanged();
}

/**
 * @brief True when any configured role's text on @p row matches @p needle under the shared
 *        separator-insensitive predicate (SerialStudio::searchMatches).
 */
bool DataModel::RowFilterProxy::rowMatches(const QAbstractItemModel* model,
                                           int row,
                                           const QModelIndex& parent,
                                           const QString& needle) const
{
  const QModelIndex index = model->index(row, 0, parent);
  SS_ASSERT(index.isValid(), return true);

  for (const int role : m_roleIds)
    if (SerialStudio::searchMatches(needle, model->data(index, role).toString()))
      return true;

  return false;
}

/**
 * @brief True when @p row is a form section-header row (only meaningful when the source model
 *        exposes a widgetType role).
 */
bool DataModel::RowFilterProxy::rowIsSectionHeader(const QAbstractItemModel* model,
                                                   int row,
                                                   const QModelIndex& parent) const
{
  if (m_widgetTypeRole < 0)
    return false;

  const QModelIndex index = model->index(row, 0, parent);
  return model->data(index, m_widgetTypeRole).toInt() == ProjectEditor::SectionHeader;
}

/**
 * @brief Accepts a row when the trimmed query is empty or the row matches. Section headers are
 *        kept only while at least one row below them (up to the next header) survives, so
 *        results stay grouped and no orphan header bars appear.
 */
bool DataModel::RowFilterProxy::filterAcceptsRow(int row, const QModelIndex& parent) const
{
  const QString needle = m_query.trimmed();
  if (needle.isEmpty())
    return true;

  const auto* model = sourceModel();
  if (!model)
    return true;

  SS_ASSERT(!m_roleIds.isEmpty(), return true);
  if (!rowIsSectionHeader(model, row, parent))
    return rowMatches(model, row, parent, needle);

  const int count = model->rowCount(parent);
  for (int r = row + 1; r < count; ++r) {
    if (rowIsSectionHeader(model, r, parent))
      return false;

    if (rowMatches(model, r, parent, needle))
      return true;
  }

  return false;
}

/**
 * @brief Maps the configured role names to @p model's role ids; unknown names drop out. The
 *        widgetType role is resolved separately for the section-header grouping rule.
 */
void DataModel::RowFilterProxy::rebuildRoleCache(const QAbstractItemModel* model)
{
  m_roleIds.clear();
  m_widgetTypeRole = -1;
  if (!model)
    return;

  const auto roles = model->roleNames();
  for (auto it = roles.constBegin(); it != roles.constEnd(); ++it) {
    if (m_filterRoles.contains(QString::fromUtf8(it.value())))
      m_roleIds.append(it.key());

    if (it.value() == QByteArrayLiteral("widgetType"))
      m_widgetTypeRole = it.key();
  }
}
