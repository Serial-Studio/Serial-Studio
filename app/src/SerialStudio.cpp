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

#include "SerialStudio.h"
#include "Misc/ThemeManager.h"

#ifdef USE_QT_COMMERCIAL
#  include "Licensing/LemonSqueezy.h"
#endif

#include <QJsonArray>

//------------------------------------------------------------------------------
// Commercial feature detection, appreciate your respect for this project
//------------------------------------------------------------------------------

/**
 * @brief Checks if Serial Studio is activated with a commercial license.
 *
 * This function determines whether the application is running with a valid
 * commercial license. If the application is built with the commercial flag,
 * it queries the licensing system; otherwise, it always returns false.
 *
 * @return true if the app is activated with a valid license, false otherwise.
 */
bool SerialStudio::activated()
{
#ifdef USE_QT_COMMERCIAL
  return Licensing::LemonSqueezy::instance().isActivated();
#else
  return false;
#endif
}

/**
 * @brief Checks if a project configuration requires commercial features.
 *
 * This function inspects the provided list of JSON groups and determines
 * if any of them use features that are exclusive to the commercial license.
 *
 * @param groups A vector of JSON::Group objects to analyze.
 * @return true if any commercial-only features are detected; false otherwise.
 */
bool SerialStudio::commercialCfg(const QVector<JSON::Group> &groups)
{
  for (const auto &group : std::as_const(groups))
  {
    if (group.widget() == QStringLiteral("plot3d"))
    {
      return true;
      break;
    }

    for (const auto &dataset : std::as_const((group.datasets())))
    {
      if (dataset.xAxisId() > 0)
      {
        return true;
        break;
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------------
// Dashboard widget logic
//------------------------------------------------------------------------------

/**
 * @brief Checks if a dashboard widget is a group widget type.
 * @param widget The `DashboardWidget` type to check.
 * @return True if the widget is a group widget; false otherwise.
 */
bool SerialStudio::isGroupWidget(const DashboardWidget widget)
{
  switch (widget)
  {
    case DashboardDataGrid:
    case DashboardMultiPlot:
    case DashboardAccelerometer:
    case DashboardGyroscope:
    case DashboardGPS:
    case DashboardLED:
    case DashboardPlot3D:
      return true;
      break;
    default:
      return false;
      break;
  }
}

/**
 * @brief Checks if a dashboard widget is a dataset widget type.
 * @param widget The `DashboardWidget` type to check.
 * @return True if the widget is a dataset widget; false otherwise.
 */
bool SerialStudio::isDatasetWidget(const DashboardWidget widget)
{
  switch (widget)
  {
    case DashboardFFT:
    case DashboardPlot:
    case DashboardBar:
    case DashboardGauge:
    case DashboardCompass:
      return true;
      break;
    default:
      return false;
      break;
  }
}

/**
 * @brief Retrieves the icon path for a specified dashboard widget.
 * @param w The `DashboardWidget` for which to retrieve the icon.
 * @return A QString representing the path to the widget's icon.
 */
QString SerialStudio::dashboardWidgetIcon(const DashboardWidget w)
{
  switch (w)
  {
    case DashboardDataGrid:
      return "qrc:/rcc/icons/dashboard/datagrid.svg";
      break;
    case DashboardMultiPlot:
      return "qrc:/rcc/icons/dashboard/multiplot.svg";
      break;
    case DashboardAccelerometer:
      return "qrc:/rcc/icons/dashboard/accelerometer.svg";
      break;
    case DashboardGyroscope:
      return "qrc:/rcc/icons/dashboard/gyroscope.svg";
      break;
    case DashboardGPS:
      return "qrc:/rcc/icons/dashboard/gps.svg";
      break;
    case DashboardFFT:
      return "qrc:/rcc/icons/dashboard/fft.svg";
      break;
    case DashboardLED:
      return "qrc:/rcc/icons/dashboard/led.svg";
      break;
    case DashboardPlot:
      return "qrc:/rcc/icons/dashboard/plot.svg";
      break;
    case DashboardBar:
      return "qrc:/rcc/icons/dashboard/bar.svg";
      break;
    case DashboardGauge:
      return "qrc:/rcc/icons/dashboard/gauge.svg";
      break;
    case DashboardCompass:
      return "qrc:/rcc/icons/dashboard/compass.svg";
      break;
    case DashboardPlot3D:
      return "qrc:/rcc/icons/dashboard/plot3d.svg";
      break;
    case DashboardNoWidget:
      return "";
      break;
    default:
      return "";
      break;
  }
}

/**
 * @brief Retrieves the display title for a specified dashboard widget.
 * @param w The `DashboardWidget` for which to retrieve the title.
 * @return A QString containing the widget's display title.
 */
QString SerialStudio::dashboardWidgetTitle(const DashboardWidget w)
{
  switch (w)
  {
    case DashboardDataGrid:
      return tr("Data Grids");
      break;
    case DashboardMultiPlot:
      return tr("Multiple Data Plots");
      break;
    case DashboardAccelerometer:
      return tr("Accelerometers");
      break;
    case DashboardGyroscope:
      return tr("Gyroscopes");
      break;
    case DashboardGPS:
      return tr("GPS");
      break;
    case DashboardFFT:
      return tr("FFT Plots");
      break;
    case DashboardLED:
      return tr("LED Panels");
      break;
    case DashboardPlot:
      return tr("Data Plots");
      break;
    case DashboardBar:
      return tr("Bars");
      break;
    case DashboardGauge:
      return tr("Gauges");
      break;
    case DashboardCompass:
      return tr("Compasses");
      break;
    case DashboardPlot3D:
      return tr("3D Plots");
      break;
    case DashboardNoWidget:
      return "";
      break;
    default:
      return "";
      break;
  }
}

/**
 * @brief Determines the dashboard widget type from a given JSON group.
 * @param group The JSON group to interpret.
 *
 * @return The corresponding `DashboardWidget` type, or `DashboardNoWidget` if
 *         not recognized.
 */
SerialStudio::DashboardWidget
SerialStudio::getDashboardWidget(const JSON::Group &group)
{
  const auto &widget = group.widget();

  if (widget == "accelerometer")
    return DashboardAccelerometer;

  else if (widget == "datagrid")
    return DashboardDataGrid;

  else if (widget == "gyro" || widget == "gyroscope")
    return DashboardGyroscope;

  else if (widget == "gps" || widget == "map")
    return DashboardGPS;

  else if (widget == "multiplot")
    return DashboardMultiPlot;

  else if (widget == "plot3d")
    return DashboardPlot3D;

  return DashboardNoWidget;
}

/**
 * @brief Retrieves a list of dashboard widgets for a specified JSON dataset.
 * @param dataset The JSON dataset to interpret.
 * @return A QList of `DashboardWidget` types matching the widgets generated
 *         by the input @c dataset parameter.
 */
QList<SerialStudio::DashboardWidget>
SerialStudio::getDashboardWidgets(const JSON::Dataset &dataset)
{
  QList<DashboardWidget> list;
  const auto &widget = dataset.widget();

  if (widget == "compass")
    list.append(DashboardCompass);

  else if (widget == "bar")
    list.append(DashboardBar);

  else if (widget == "gauge")
    list.append(DashboardGauge);

  if (dataset.graph())
    list.append(DashboardPlot);

  if (dataset.fft())
    list.append(DashboardFFT);

  if (dataset.led())
    list.append(DashboardLED);

  return list;
}

//------------------------------------------------------------------------------
// Parsing & project model logic
//------------------------------------------------------------------------------

/**
 * @brief Retrieves the ID string associated with a specified group widget.
 * @param widget The `GroupWidget` type to get the ID for.
 * @return A QString representing the widget ID.
 */
QString SerialStudio::groupWidgetId(const GroupWidget widget)
{
  switch (widget)
  {
    case DataGrid:
      return "datagrid";
      break;
    case Accelerometer:
      return "accelerometer";
      break;
    case Gyroscope:
      return "gyro";
      break;
    case GPS:
      return "gps";
      break;
    case MultiPlot:
      return "multiplot";
      break;
    case Plot3D:
      return "plot3d";
      break;
    case NoGroupWidget:
      return "";
      break;
    default:
      return "";
      break;
  }
}

/**
 * @brief Determines the group widget type from a given ID string.
 * @param id The ID string to interpret.
 * @return The corresponding `GroupWidget` type, or `NoGroupWidget` if not
 *         recognized.
 */
SerialStudio::GroupWidget SerialStudio::groupWidgetFromId(const QString &id)
{
  if (id == "datagrid")
    return DataGrid;

  else if (id == "accelerometer")
    return Accelerometer;

  else if (id == "gyro" || id == "gyroscope")
    return Gyroscope;

  else if (id == "gps")
    return GPS;

  else if (id == "multiplot")
    return MultiPlot;

  else if (id == "plot3d")
    return Plot3D;

  return NoGroupWidget;
}

/**
 * @brief Retrieves the ID string associated with a specified dataset widget.
 * @param widget The `DatasetWidget` type to get the ID for.
 * @return A QString representing the widget ID.
 */
QString SerialStudio::datasetWidgetId(const DatasetWidget widget)
{
  switch (widget)
  {
    case Bar:
      return "bar";
      break;
    case Gauge:
      return "gauge";
      break;
    case Compass:
      return "compass";
      break;
    case NoDatasetWidget:
      return "";
      break;
    default:
      return "";
      break;
  }
}

/**
 * @brief Determines the dataset widget type from a given ID string.
 * @param id The ID string to interpret.
 * @return The corresponding `DatasetWidget` type, or `NoDatasetWidget` if not
 *         recognized.
 */
SerialStudio::DatasetWidget SerialStudio::datasetWidgetFromId(const QString &id)
{
  if (id == "bar")
    return Bar;

  else if (id == "gauge")
    return Gauge;

  else if (id == "compass")
    return Compass;

  return NoDatasetWidget;
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

/**
 * @brief Retrieves the appropriate color for a dataset based on its index.
 *
 * This function accesses the theme's color array to obtain a color that
 * corresponds to the given dataset index.
 *
 * @param index The frame index of the dataset.
 * @return A QString representing the color associated with the dataset index.
 */
QString SerialStudio::getDatasetColor(const int index)
{
  static const auto *theme = &Misc::ThemeManager::instance();
  const auto idx = index - 1;
  const auto colors = theme->colors()["widget_colors"].toArray();
  const auto color = colors.count() > idx
                         ? colors.at(idx).toString()
                         : colors.at(idx % colors.count()).toString();
  return color;
}
