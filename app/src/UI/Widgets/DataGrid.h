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
 * @class Widgets::DataGrid
 * @brief Tabular data display widget for structured dataset visualization.
 *
 * The DataGrid class provides a table-based visualization of dataset
 * information, displaying data in rows and columns with optional pause
 * functionality. It extends the StaticTable widget to provide dynamic data
 * updates from Serial Studio data frames.
 *
 * Key Features:
 * - **Tabular Layout**: Presents data in an organized table format
 * - **Pause/Resume**: Can freeze the display while data continues to arrive
 * - **Auto-Update**: Automatically refreshes when new data frames are received
 * - **Structured Display**: Ideal for datasets with multiple fields
 *
 * Typical Use Cases:
 * - Multi-field sensor data display
 * - Structured telemetry visualization
 * - Configuration parameter monitoring
 * - Multi-variable system state display
 *
 * @note This widget inherits table rendering capabilities from StaticTable
 *       and adds data frame integration.
 */
class DataGrid : public StaticTable {
  Q_OBJECT
  Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

signals:
  void pausedChanged();

public:
  explicit DataGrid(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] bool paused() const;

public slots:
  void setPaused(const bool paused);

private slots:
  void updateData();
  void onFontsChanged();

private:
  QStringList getRow(const DataModel::Dataset& dataset);

private:
  int m_index;
  bool m_paused;
};
}  // namespace Widgets
