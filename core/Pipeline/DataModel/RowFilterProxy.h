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

#include <QSortFilterProxyModel>
#include <QStringList>
#include <QVector>

namespace DataModel {

/**
 * @brief QML-instantiable row filter for the editor tables: matches a free-text query against a
 *        configurable set of named roles, case-insensitive. An empty query passes every row, so
 *        an idle proxy costs nothing; setData/flags forward to the source, so editable
 *        delegates keep working through it.
 */
class RowFilterProxy : public QSortFilterProxyModel {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString query
             READ  query
             WRITE setQuery
             NOTIFY queryChanged)
  Q_PROPERTY(QStringList filterRoles
             READ  filterRoles
             WRITE setFilterRoles
             NOTIFY filterRolesChanged)
  // clang-format on

signals:
  void queryChanged();
  void filterRolesChanged();

public:
  explicit RowFilterProxy(QObject* parent = nullptr);
  RowFilterProxy(RowFilterProxy&&)                 = delete;
  RowFilterProxy(const RowFilterProxy&)            = delete;
  RowFilterProxy& operator=(RowFilterProxy&&)      = delete;
  RowFilterProxy& operator=(const RowFilterProxy&) = delete;

  [[nodiscard]] QString query() const;
  [[nodiscard]] QStringList filterRoles() const;

  void setSourceModel(QAbstractItemModel* model) override;

public slots:
  void setQuery(const QString& query);
  void setFilterRoles(const QStringList& roles);

protected:
  [[nodiscard]] bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

private:
  void rebuildRoleCache(const QAbstractItemModel* model);
  [[nodiscard]] bool rowIsSectionHeader(const QAbstractItemModel* model,
                                        int row,
                                        const QModelIndex& parent) const;
  [[nodiscard]] bool rowMatches(const QAbstractItemModel* model,
                                int row,
                                const QModelIndex& parent,
                                const QString& needle) const;

private:
  QString m_query;
  QStringList m_filterRoles;
  QVector<int> m_roleIds;
  int m_widgetTypeRole;
};

}  // namespace DataModel
