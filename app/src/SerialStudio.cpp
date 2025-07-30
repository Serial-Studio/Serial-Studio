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

#include "SerialStudio.h"
#include "Misc/ThemeManager.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/Trial.h"
#  include "Licensing/LemonSqueezy.h"
#endif

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
#ifdef BUILD_COMMERCIAL
  return Licensing::LemonSqueezy::instance().isActivated()
         || Licensing::Trial::instance().trialEnabled();
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
bool SerialStudio::commercialCfg(const QVector<JSON::Group> &g)
{
  for (const auto &group : std::as_const(g))
  {
    if (group.widget == QStringLiteral("plot3d"))
    {
      return true;
      break;
    }

    for (const auto &dataset : std::as_const((group.datasets)))
    {
      if (dataset.xAxisId > 0)
      {
        return true;
        break;
      }
    }
  }

  return false;
}

bool SerialStudio::commercialCfg(const std::vector<JSON::Group> &g)
{
  for (const auto &group : std::as_const(g))
  {
    if (group.widget == QStringLiteral("plot3d"))
    {
      return true;
      break;
    }

    for (const auto &dataset : std::as_const((group.datasets)))
    {
      if (dataset.xAxisId > 0)
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
    case DashboardTerminal:
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
QString SerialStudio::dashboardWidgetIcon(const DashboardWidget w,
                                          const bool large)
{
  const QString iconPath = large ? "qrc:/rcc/icons/dashboard-large/"
                                 : "qrc:/rcc/icons/dashboard-small/";

  switch (w)
  {
    case DashboardDataGrid:
      return iconPath + "datagrid.svg";
      break;
    case DashboardMultiPlot:
      return iconPath + "multiplot.svg";
      break;
    case DashboardAccelerometer:
      return iconPath + "accelerometer.svg";
      break;
    case DashboardGyroscope:
      return iconPath + "gyroscope.svg";
      break;
    case DashboardGPS:
      return iconPath + "gps.svg";
      break;
    case DashboardFFT:
      return iconPath + "fft.svg";
      break;
    case DashboardLED:
      return iconPath + "led.svg";
      break;
    case DashboardPlot:
      return iconPath + "plot.svg";
      break;
    case DashboardBar:
      return iconPath + "bar.svg";
      break;
    case DashboardGauge:
      return iconPath + "gauge.svg";
      break;
    case DashboardCompass:
      return iconPath + "compass.svg";
      break;
    case DashboardTerminal:
      return iconPath + "terminal.svg";
      break;
    case DashboardPlot3D:
      return iconPath + "plot3d.svg";
      break;
    case DashboardNoWidget:
      return iconPath + "group.svg";
      break;
    default:
      return iconPath + "group.svg";
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
    case DashboardTerminal:
      return tr("Terminal");
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
  const auto &widget = group.widget;

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

  else if (widget == "terminal")
    return DashboardTerminal;

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
  const auto &widget = dataset.widget;

  if (widget == "compass")
    list.append(DashboardCompass);

  else if (widget == "bar")
    list.append(DashboardBar);

  else if (widget == "gauge")
    list.append(DashboardGauge);

  if (dataset.plt)
    list.append(DashboardPlot);

  if (dataset.fft)
    list.append(DashboardFFT);

  if (dataset.led)
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

  else if (id == "gps" || id == "map")
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
QColor SerialStudio::getDatasetColor(const int index)
{
  static const auto *theme = &Misc::ThemeManager::instance();
  const auto idx = index - 1;
  const auto colors = theme->widgetColors();
  const auto color
      = colors.count() > idx ? colors.at(idx) : colors.at(idx % colors.count());

  return color;
}

/**
 * @brief Converts a hex-encoded string into a UTF-8 decoded string with visible
 *        escape sequences.
 *
 * @param hex Hexadecimal string (e.g. "48 65 6C 6C 6F 0A").
 * @return Decoded QString with control characters escaped (e.g. "Hello\\n").
 */
QString SerialStudio::hexToString(const QString &hex)
{
  QString raw = QString::fromUtf8(
      QByteArray::fromHex(QString(hex).remove(' ').toUtf8()));
  return escapeControlCharacters(raw);
}

/**
 * @brief Converts a string containing escape sequences into a space-separated
 *        hexadecimal string.
 *
 * @param str Input QString with escape sequences (e.g. "Hello\\n").
 * @return Hex representation of the UTF-8 encoded string (e.g. "48 65 6C 6C 6F
 * 0A").
 */
QString SerialStudio::stringToHex(const QString &str)
{
  QString resolved = resolveEscapeSequences(str);
  return QString::fromLatin1(resolved.toUtf8().toHex(' '));
}

/**
 * Converts the given @a data in HEX format into real binary data.
 */
QByteArray SerialStudio::hexToBytes(const QString &data)
{
  // Remove spaces from the input data
  QString withoutSpaces = data;
  withoutSpaces.replace(QStringLiteral(" "), "");

  // Check if the length of the string is even
  if (withoutSpaces.length() % 2 != 0)
  {
    qWarning() << data << "is not a valid hexadecimal array";
    return QByteArray();
  }

  // Iterate over the string in steps of 2
  bool ok;
  QByteArray array;
  for (int i = 0; i < withoutSpaces.length(); i += 2)
  {
    // Get two characters (a hex pair)
    auto chr1 = withoutSpaces.at(i);
    auto chr2 = withoutSpaces.at(i + 1);

    // Convert the hex pair into a byte
    QString byteStr = QStringLiteral("%1%2").arg(chr1, chr2);
    int byte = byteStr.toInt(&ok, 16);

    // If the conversion fails, return an empty array
    if (!ok)
    {
      qWarning() << data << "is not a valid hexadecimal array";
      return QByteArray();
    }

    // Append the byte to the result array
    array.append(static_cast<char>(byte));
  }

  return array;
}

/**
 * @brief Resolves C-style escape sequences in a string into their corresponding
 *        control characters.
 *
 * @param str Input QString (e.g. "Hello\\n").
 * @return QString with escape sequences converted to real control characters
 * (e.g. "Hello\n").
 */
QString SerialStudio::resolveEscapeSequences(const QString &str)
{
  QString escapedStr = str;
  escapedStr.replace('\n', QLatin1String(""));
  escapedStr.replace('\r', QLatin1String(""));
  escapedStr.replace(QStringLiteral("\\a"), QStringLiteral("\a"));
  escapedStr.replace(QStringLiteral("\\b"), QStringLiteral("\b"));
  escapedStr.replace(QStringLiteral("\\f"), QStringLiteral("\f"));
  escapedStr.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
  escapedStr.replace(QStringLiteral("\\r"), QStringLiteral("\r"));
  escapedStr.replace(QStringLiteral("\\t"), QStringLiteral("\t"));
  escapedStr.replace(QStringLiteral("\\v"), QStringLiteral("\v"));
  escapedStr.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
  return escapedStr;
}

/**
 * @brief Escapes control characters in a string using C-style escape sequences.
 *
 * @param str Input QString with raw control characters (e.g. "Hello\n").
 * @return QString with escape sequences (e.g. "Hello\\n").
 */
QString SerialStudio::escapeControlCharacters(const QString &str)
{
  QString result = str;
  result.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
  result.replace(QStringLiteral("\a"), QStringLiteral("\\a"));
  result.replace(QStringLiteral("\b"), QStringLiteral("\\b"));
  result.replace(QStringLiteral("\f"), QStringLiteral("\\f"));
  result.replace(QStringLiteral("\n"), QStringLiteral("\\n"));
  result.replace(QStringLiteral("\r"), QStringLiteral("\\r"));
  result.replace(QStringLiteral("\t"), QStringLiteral("\\t"));
  result.replace(QStringLiteral("\v"), QStringLiteral("\\v"));
  return result;
}
