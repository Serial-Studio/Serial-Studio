/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QAbstractListModel>
#include <QQuickItem>
#include <QVariant>
#include <QVector>

#include "DataModel/Frame.h"

namespace Widgets {

/**
 * @brief Single DataGrid row: dataset title, formatted value, and one
 *        {windowId, icon, title} map per dashboard widget showing the dataset.
 */
struct DataGridRow {
  QString title;
  QString value;
  QVariantList widgets;
};

/**
 * @brief List model backing the DataGrid's rows with per-row dataChanged() updates.
 */
class DataGridRowsModel : public QAbstractListModel {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  enum Role {
    TitleRole = Qt::UserRole + 1,
    ValueRole,
    WidgetsRole,
  };

  explicit DataGridRowsModel(QObject* parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  void reset(const QVector<DataGridRow>& rows);
  bool updateRow(int row, const QString& title, const QString& value);

private:
  QVector<DataGridRow> m_rows;
};

/**
 * @brief Tabular data display widget; the QQuickItem only owns the row model and paused state.
 */
class DataGrid : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool paused
             READ  paused
             WRITE setPaused
             NOTIFY pausedChanged)
  Q_PROPERTY(DataGridRowsModel* rowsModel
             READ  rowsModel
             CONSTANT)
  Q_PROPERTY(QString titleHeader
             READ  titleHeader
             NOTIFY headersChanged)
  Q_PROPERTY(QString valueHeader
             READ  valueHeader
             NOTIFY headersChanged)
  // clang-format on

signals:
  void pausedChanged();
  void headersChanged();

public:
  explicit DataGrid(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] bool paused() const noexcept;
  [[nodiscard]] DataGridRowsModel* rowsModel() const noexcept;
  [[nodiscard]] const QString& titleHeader() const noexcept;
  [[nodiscard]] const QString& valueHeader() const noexcept;

public slots:
  void setPaused(const bool paused);

private slots:
  void updateData();

private:
  [[nodiscard]] QString formatValue(const DataModel::Dataset& dataset) const;
  [[nodiscard]] QVariantList datasetWidgets(const DataModel::Dataset& dataset) const;
  void rebuildRows();

private:
  int m_index;
  bool m_paused;
  QString m_titleHeader;
  QString m_valueHeader;
  DataGridRowsModel* m_rowsModel;
  int m_lastRowCount;
};
}  // namespace Widgets
