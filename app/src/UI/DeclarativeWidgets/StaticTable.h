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

#include <QTableWidget>
#include <QStyledItemDelegate>

#include "DeclarativeWidget.h"

/**
 * @class StaticTable
 * @brief QML-embeddable table widget for displaying structured data in rows and
 *        columns.
 *
 * The StaticTable class extends DeclarativeWidget to provide a QTableWidget
 * that can be rendered within QML. It displays two-dimensional data (rows and
 * columns) with customizable fonts for both content and headers, and
 * automatically adapts to theme changes.
 *
 * Key Features:
 * - **Table Display**: Shows data in a traditional row/column grid format
 * - **Font Customization**: Separate font configuration for headers and content
 * - **Theme Awareness**: Automatically updates colors when theme changes
 * - **Dynamic Data**: Data can be updated at runtime via setData()
 * - **QML Integration**: Renders QTableWidget content within QML via
 *   DeclarativeWidget
 *
 * Data Format:
 * Data is provided as a list of string lists, where each inner list represents
 * a row in the table. The first row is typically used as headers.
 *
 * Typical Use Cases:
 * - Displaying tabular sensor data
 * - Configuration parameter tables
 * - Multi-field telemetry displays
 * - Structured log/event tables
 *
 * @note This class uses DeclarativeWidget's image-based rendering to bridge
 *       QWidget (QTableWidget) into the QML scene graph.
 */
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
  // clang-format on

Q_SIGNALS:
  void fontChanged();
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
