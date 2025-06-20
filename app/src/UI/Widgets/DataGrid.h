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
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "JSON/Dataset.h"
#include "UI/DeclarativeWidgets/StaticTable.h"

namespace Widgets
{
class DataGrid : public StaticTable
{
  Q_OBJECT
  Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

signals:
  void pausedChanged();

public:
  explicit DataGrid(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] bool paused() const;

public slots:
  void setPaused(const bool paused);

private slots:
  void updateData();

private:
  QStringList getRow(const JSON::Dataset &dataset);

private:
  int m_index;
  bool m_paused;
};
} // namespace Widgets
