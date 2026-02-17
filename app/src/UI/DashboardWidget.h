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

#include <QQuickItem>

#include "SerialStudio.h"

namespace UI {
/**
 * @class DashboardWidget
 * @brief QML-exposed widget container that dynamically instantiates and
 *        manages dashboard visualization widgets.
 *
 * The DashboardWidget class acts as a bridge between the QML dashboard UI and
 * the C++ widget implementations. It dynamically creates the appropriate widget
 * type based on configuration, provides a consistent property interface to QML,
 * and manages widget lifecycle.
 *
 * Key Features:
 * - **Dynamic Widget Creation**: Instantiates C++ widgets based on widget type
 *   enum
 * - **Dual Rendering Support**: Handles both QQuickItem and QQuickPaintedItem
 *   widgets
 * - **QML Integration**: Exposes widget model and metadata as Qt properties
 * - **Color Theming**: Provides theme-aware color assignment per widget
 * - **Indexed Access**: Supports both absolute and relative widget indexing
 * - **Lazy Loading**: Creates widgets on-demand when index is set
 *
 * Widget Types Supported:
 * - Plot widgets (Plot, MultiPlot, FFTPlot, Plot3D)
 * - Instrument widgets (Bar, Gauge, Compass, Gyroscope, Accelerometer)
 * - Data display widgets (DataGrid, Terminal, LEDPanel)
 * - Geographic widgets (GPS)
 *
 * Usage Pattern:
 * 1. Create DashboardWidget instance in QML
 * 2. Set widgetIndex property (triggers widget creation)
 * 3. Bind to widgetModel property to access the C++ widget
 * 4. Use widgetQmlPath to load the corresponding QML visualization
 *
 * @note The widget type is determined by the UI::Dashboard based on the current
 *       project configuration. This class should not be instantiated directly
 *       outside of the dashboard system.
 */
class DashboardWidget : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int widgetIndex
             READ widgetIndex
             WRITE setWidgetIndex
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(int relativeIndex
             READ relativeIndex
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QColor widgetColor
             READ widgetColor
             NOTIFY widgetColorChanged)
  Q_PROPERTY(QString widgetTitle
             READ widgetTitle
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QString widgetQmlPath
             READ widgetQmlPath
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QQuickItem* widgetModel
             READ widgetModel
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(SerialStudio::DashboardWidget widgetType
             READ widgetType
             NOTIFY widgetIndexChanged)
  // clang-format on

signals:
  void widgetIndexChanged();
  void widgetColorChanged();

public:
  DashboardWidget(QQuickItem* parent = 0);
  ~DashboardWidget();

  [[nodiscard]] int widgetIndex() const;
  [[nodiscard]] int relativeIndex() const;
  [[nodiscard]] QColor widgetColor() const;
  [[nodiscard]] QString widgetTitle() const;
  [[nodiscard]] SerialStudio::DashboardWidget widgetType() const;

  [[nodiscard]] QString widgetQmlPath() const;
  [[nodiscard]] QQuickItem* widgetModel() const;

public slots:
  void setWidgetIndex(const int index);

private:
  int m_index;
  int m_relativeIndex;
  SerialStudio::DashboardWidget m_widgetType;

  QString m_qmlPath;
  QQuickItem* m_dbWidget;
};
}  // namespace UI
