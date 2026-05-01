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

#include "DataModel/Frame.h"
#include "UI/DeclarativeWidgets/StaticTable.h"

namespace Widgets {
/**
 * @brief Tabular data display widget for structured dataset visualization.
 */
class DataGrid : public StaticTable {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool paused
             READ  paused
             WRITE setPaused
             NOTIFY pausedChanged)
  // clang-format on

signals:
  void pausedChanged();

public:
  explicit DataGrid(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] bool paused() const noexcept;

public slots:
  void setPaused(const bool paused);

private slots:
  void updateData();
  void onFontsChanged();

private:
  QStringList getRow(const DataModel::Dataset& dataset);
  QString formatValue(const DataModel::Dataset& dataset);

private:
  int m_index;
  bool m_paused;
};
}  // namespace Widgets
