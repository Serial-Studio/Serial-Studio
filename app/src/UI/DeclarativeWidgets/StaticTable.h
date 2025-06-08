/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QTableWidget>
#include <QStyledItemDelegate>

#include "DeclarativeWidget.h"

class StaticTable : public DeclarativeWidget
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(const QFont& font
             READ font
             WRITE setFont
             NOTIFY fontChanged)
  Q_PROPERTY(const QFont& headerFont
             READ headerFont
             WRITE setHeaderFont
             NOTIFY headerFontChanged)
  Q_PROPERTY(const QList<QStringList>& data
             READ data
             WRITE setData
             NOTIFY dataChanged)
  // clang-format on

Q_SIGNALS:
  void fontChanged();
  void dataChanged();
  void headerFontChanged();

public:
  explicit StaticTable(QQuickItem *parent = nullptr);

  [[nodiscard]] const QFont &font() const;
  [[nodiscard]] const QFont &headerFont() const;
  [[nodiscard]] const QList<QStringList> &data() const;

public Q_SLOTS:
  void setFont(const QFont &font);
  void setHeaderFont(const QFont &font);
  void setData(const QList<QStringList> &data);

private Q_SLOTS:
  void loadTheme();

private:
  QFont m_font;
  QFont m_headerFont;
  QTableWidget m_widget;
  QList<QStringList> m_data;
};
