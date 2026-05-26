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

#include "SerialStudio.h"

#include <QHash>
#include <QStringConverter>
#include <QtCore5Compat/QTextCodec>

#include "CSV/Player.h"
#include "MDF4/Player.h"
#include "Misc/ThemeManager.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Licensing/Trial.h"
#  include "Sessions/Player.h"
#endif

//--------------------------------------------------------------------------------------------------
// Commercial feature detection, appreciate your respect for this project
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if Serial Studio is activated with a commercial license.
 */
bool SerialStudio::activated()
{
#ifdef BUILD_COMMERCIAL
  return Licensing::CommercialToken::current().isValid() && SS_LICENSE_GUARD();
#else
  return false;
#endif
}

/**
 * @brief Checks whether Pro-only dashboard widgets (Plot3D, ImageView, OutputPanel) should be
 * materialised for this build.
 */
bool SerialStudio::proWidgetsEnabled()
{
#ifdef BUILD_COMMERCIAL
  const auto& token = Licensing::CommercialToken::current();
  return token.isValid() && SS_LICENSE_GUARD()
      && token.featureTier() >= Licensing::FeatureTier::Trial && !token.variantName().isEmpty();
#else
  return false;
#endif
}

/**
 * @brief Returns true if a transform script references the notify() API family.
 */
static bool transformUsesNotifications(const QString& code)
{
  // Substring screen for the notify*( identifier family
  return code.contains(QStringLiteral("notify(")) || code.contains(QStringLiteral("notifyInfo("))
      || code.contains(QStringLiteral("notifyWarning("))
      || code.contains(QStringLiteral("notifyCritical("))
      || code.contains(QStringLiteral("notifyClear("));
}

/**
 * @brief Checks if a project configuration (QVector form) requires commercial features.
 */
bool SerialStudio::commercialCfg(const QVector<DataModel::Group>& g)
{
  for (const auto& group : std::as_const(g)) {
    if (group.groupType == DataModel::GroupType::Output)
      return true;

    if (group.widget == QStringLiteral("plot3d"))
      return true;

    if (group.widget == QStringLiteral("image"))
      return true;

    if (group.widget == QStringLiteral("painter"))
      return true;

    for (const auto& dataset : std::as_const((group.datasets))) {
      if (dataset.xAxisId >= 0)
        return true;

      if (dataset.waterfall)
        return true;

      if (!dataset.transformCode.isEmpty() && transformUsesNotifications(dataset.transformCode))
        return true;
    }
  }

  return false;
}

/**
 * @brief Checks if a project configuration requires commercial features.
 */
bool SerialStudio::commercialCfg(const std::vector<DataModel::Group>& g)
{
  for (const auto& group : std::as_const(g)) {
    if (group.groupType == DataModel::GroupType::Output)
      return true;

    if (group.widget == QStringLiteral("plot3d"))
      return true;

    if (group.widget == QStringLiteral("image"))
      return true;

    if (group.widget == QStringLiteral("painter"))
      return true;

    for (const auto& dataset : std::as_const((group.datasets))) {
      if (dataset.xAxisId >= 0)
        return true;

      if (dataset.waterfall)
        return true;

      if (!dataset.transformCode.isEmpty() && transformUsesNotifications(dataset.transformCode))
        return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Dashboard widget logic
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if a dashboard widget is a group widget type.
 */
bool SerialStudio::isGroupWidget(const DashboardWidget widget)
{
  switch (widget) {
    case DashboardDataGrid:
    case DashboardMultiPlot:
    case DashboardAccelerometer:
    case DashboardGyroscope:
    case DashboardGPS:
    case DashboardLED:
    case DashboardPlot3D:
    case DashboardTerminal:
    case DashboardClock:
    case DashboardStopwatch:
#ifdef BUILD_COMMERCIAL
    case DashboardImageView:
    case DashboardOutputPanel:
    case DashboardNotificationLog:
    case DashboardPainter:
#endif
      return true;
    default:
      return false;
      break;
  }
}

/**
 * @brief Checks if a dashboard widget is a dataset widget type.
 */
bool SerialStudio::isDatasetWidget(const DashboardWidget widget)
{
  switch (widget) {
    case DashboardFFT:
    case DashboardPlot:
    case DashboardBar:
    case DashboardGauge:
    case DashboardCompass:
    case DashboardMeter:
#ifdef BUILD_COMMERCIAL
    case DashboardWaterfall:
#endif
      return true;
    default:
      return false;
      break;
  }
}

/**
 * @brief Retrieves the icon path for a specified dashboard widget.
 */
QString SerialStudio::dashboardWidgetIcon(const DashboardWidget w, const bool large)
{
  const QString iconPath = large ? "qrc:/icons/dashboard-large/" : "qrc:/icons/dashboard-small/";

  switch (w) {
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
    case DashboardMeter:
      return iconPath + "meter.svg";
      break;
    case DashboardTerminal:
      return iconPath + "terminal.svg";
      break;
    case DashboardClock:
      return iconPath + "clock.svg";
      break;
    case DashboardStopwatch:
      return iconPath + "stopwatch.svg";
      break;
    case DashboardPlot3D:
      return iconPath + "plot3d.svg";
      break;
#ifdef BUILD_COMMERCIAL
    case DashboardImageView:
      return iconPath + "image.svg";
      break;
    case DashboardOutputPanel:
      return iconPath + "output-panel.svg";
      break;
    case DashboardNotificationLog:
      return iconPath + "notification-log.svg";
      break;
    case DashboardWaterfall:
      return iconPath + "waterfall.svg";
      break;
    case DashboardPainter:
      return iconPath + "painter.svg";
      break;
#endif
    case DashboardNoWidget:
      return iconPath + "group.svg";
      break;
    default:
      return iconPath + "group.svg";
      break;
  }
}

/**
 * @brief Returns whether a group contributes any widgets to Dashboard's walker.
 */
bool SerialStudio::groupEligibleForWorkspace(const DataModel::Group& g)
{
  // Output panels filter downstream via groupWidgetEligibleForWorkspace
  Q_UNUSED(g);
  return true;
}

/**
 * @brief Returns whether a group-level widget key should appear on workspaces.
 */
bool SerialStudio::groupWidgetEligibleForWorkspace(SerialStudio::DashboardWidget w)
{
  if (w == DashboardNoWidget || w == DashboardTerminal || w == DashboardClock
      || w == DashboardStopwatch)
    return false;

#ifdef BUILD_COMMERCIAL
  if (w == DashboardNotificationLog)
    return false;
#endif

  return true;
}

/**
 * @brief Returns whether a dataset-level widget key should appear on workspaces.
 */
bool SerialStudio::datasetWidgetEligibleForWorkspace(SerialStudio::DashboardWidget w)
{
  if (w == DashboardNoWidget || w == DashboardLED || w == DashboardTerminal || w == DashboardClock
      || w == DashboardStopwatch)
    return false;

#ifdef BUILD_COMMERCIAL
  if (w == DashboardNotificationLog)
    return false;
#endif

  return true;
}

/**
 * @brief Retrieves the display title for a specified dashboard widget.
 */
QString SerialStudio::dashboardWidgetTitle(const DashboardWidget w)
{
  switch (w) {
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
    case DashboardClock:
      return tr("Clock");
      break;
    case DashboardStopwatch:
      return tr("Stopwatch");
      break;
    case DashboardCompass:
      return tr("Compasses");
      break;
    case DashboardMeter:
      return tr("Meters");
      break;
    case DashboardPlot3D:
      return tr("3D Plots");
      break;
#ifdef BUILD_COMMERCIAL
    case DashboardImageView:
      return tr("Image Views");
      break;
    case DashboardOutputPanel:
      return tr("Output Panels");
      break;
    case DashboardNotificationLog:
      return tr("Notifications");
      break;
    case DashboardWaterfall:
      return tr("Waterfalls");
      break;
    case DashboardPainter:
      return tr("Painter Widgets");
      break;
#endif
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
 */
SerialStudio::DashboardWidget SerialStudio::getDashboardWidget(const DataModel::Group& group)
{
#ifdef BUILD_COMMERCIAL
  if (group.groupType == DataModel::GroupType::Output)
    return DashboardOutputPanel;
#else
  if (group.groupType == DataModel::GroupType::Output)
    return DashboardNoWidget;
#endif

  const auto& widget = group.widget;

  if (widget == "accelerometer")
    return DashboardAccelerometer;

  if (widget == "datagrid")
    return DashboardDataGrid;

  if (widget == "gyro" || widget == "gyroscope")
    return DashboardGyroscope;

  if (widget == "gps" || widget == "map")
    return DashboardGPS;

  if (widget == "multiplot")
    return DashboardMultiPlot;

  if (widget == "plot3d")
    return DashboardPlot3D;

  if (widget == "terminal")
    return DashboardTerminal;

  if (widget == "clock")
    return DashboardClock;

  if (widget == "stopwatch")
    return DashboardStopwatch;

#ifdef BUILD_COMMERCIAL
  if (widget == "image")
    return DashboardImageView;

  if (widget == "notification-log")
    return DashboardNotificationLog;

  if (widget == "painter")
    return DashboardPainter;
#else
  // GPL fallback: render painter groups as a data grid
  if (widget == "painter")
    return DashboardDataGrid;
#endif

  return DashboardNoWidget;
}

/**
 * @brief Retrieves a list of dashboard widgets for a specified JSON dataset.
 */
QList<SerialStudio::DashboardWidget> SerialStudio::getDashboardWidgets(
  const DataModel::Dataset& dataset)
{
  QList<DashboardWidget> list;

  static const QHash<QString, DashboardWidget> kDatasetWidgetMap = {
    {QStringLiteral("compass"), DashboardCompass},
    {    QStringLiteral("bar"),     DashboardBar},
    {  QStringLiteral("gauge"),   DashboardGauge},
    {  QStringLiteral("meter"),   DashboardMeter},
  };
  const auto it = kDatasetWidgetMap.constFind(dataset.widget);
  if (it != kDatasetWidgetMap.constEnd())
    list.append(it.value());

  if (dataset.plt)
    list.append(DashboardPlot);

  if (dataset.fft)
    list.append(DashboardFFT);

  if (dataset.led)
    list.append(DashboardLED);

#ifdef BUILD_COMMERCIAL
  if (dataset.waterfall)
    list.append(DashboardWaterfall);
#endif

  return list;
}

//--------------------------------------------------------------------------------------------------
// Parsing & project model logic
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves the ID string associated with a specified group widget.
 */
QString SerialStudio::groupWidgetId(const GroupWidget widget)
{
  switch (widget) {
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
#ifdef BUILD_COMMERCIAL
    case ImageView:
      return "image";
      break;
    case Painter:
      return "painter";
      break;
#endif
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
 */
SerialStudio::GroupWidget SerialStudio::groupWidgetFromId(const QString& id)
{
  if (id == "datagrid")
    return DataGrid;

  if (id == "accelerometer")
    return Accelerometer;

  if (id == "gyro" || id == "gyroscope")
    return Gyroscope;

  if (id == "gps" || id == "map")
    return GPS;

  if (id == "multiplot")
    return MultiPlot;

  if (id == "plot3d")
    return Plot3D;

#ifdef BUILD_COMMERCIAL
  if (id == "image")
    return ImageView;

  if (id == "painter")
    return Painter;
#endif

  return NoGroupWidget;
}

/**
 * @brief Retrieves the ID string associated with a specified dataset widget.
 */
QString SerialStudio::datasetWidgetId(const DatasetWidget widget)
{
  switch (widget) {
    case Bar:
      return "bar";
      break;
    case Gauge:
      return "gauge";
      break;
    case Compass:
      return "compass";
      break;
    case Meter:
      return "meter";
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
 */
SerialStudio::DatasetWidget SerialStudio::datasetWidgetFromId(const QString& id)
{
  static const QHash<QString, DatasetWidget> kIdMap = {
    {    QStringLiteral("bar"),     Bar},
    {  QStringLiteral("gauge"),   Gauge},
    {QStringLiteral("compass"), Compass},
    {  QStringLiteral("meter"),   Meter},
  };
  return kIdMap.value(id, NoDatasetWidget);
}

//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if any playback players (CSV or MDF4) are currently open.
 */
bool SerialStudio::isAnyPlayerOpen()
{
  static auto& csvPlayer  = CSV::Player::instance();
  static auto& mdf4Player = MDF4::Player::instance();

#ifdef BUILD_COMMERCIAL
  static auto& sqlPlayer = Sessions::Player::instance();
  return csvPlayer.isOpen() || mdf4Player.isOpen() || sqlPlayer.isOpen();
#else
  return csvPlayer.isOpen() || mdf4Player.isOpen();
#endif
}

/**
 * @brief Returns true when a player that stores post-transform values is open.
 */
bool SerialStudio::isFinalValuePlayerOpen()
{
  static auto& csvPlayer  = CSV::Player::instance();
  static auto& mdf4Player = MDF4::Player::instance();
  return csvPlayer.isOpen() || mdf4Player.isOpen();
}

/**
 * @brief Retrieves the appropriate color for a dataset based on its index.
 */
QColor SerialStudio::getDatasetColor(const int index)
{
  static const auto* theme = &Misc::ThemeManager::instance();
  const auto idx           = index - 1;
  const auto colors        = theme->widgetColors();

  if (colors.isEmpty())
    return QColor(Qt::gray);

  if (idx < 0)
    return QColor(Qt::gray);

  const auto count = colors.count();
  if (idx < count)
    return colors.at(idx);

  else {
    const auto cycle    = idx / count;
    const auto position = idx % count;
    const auto offset   = (cycle * 7) % count;
    const auto colorIdx = (position + offset) % count;
    return colors.at(colorIdx);
  }
}

/**
 * @brief Returns the top gradient color for the given device source index.
 */
QColor SerialStudio::getDeviceTopColor(const int sourceId)
{
  if (sourceId <= 0)
    return QColor(Qt::transparent);

  static const auto* theme = &Misc::ThemeManager::instance();
  const auto& colors       = theme->deviceColors();

  if (colors.isEmpty())
    return QColor(Qt::transparent);

  return colors.at((sourceId - 1) % colors.count()).first;
}

/**
 * @brief Returns the bottom gradient color for the given device source index.
 */
QColor SerialStudio::getDeviceBottomColor(const int sourceId)
{
  if (sourceId <= 0)
    return QColor(Qt::transparent);

  static const auto* theme = &Misc::ThemeManager::instance();
  const auto& colors       = theme->deviceColors();

  if (colors.isEmpty())
    return QColor(Qt::transparent);

  return colors.at((sourceId - 1) % colors.count()).second;
}

/**
 * @brief Returns a saturated accent color for a given device index.
 */
QColor SerialStudio::getDeviceColor(const int sourceId)
{
  // Validate source index
  if (sourceId <= 0)
    return QColor(Qt::transparent);

  // Obtain colors from current theme
  static const auto* theme = &Misc::ThemeManager::instance();
  const auto& colors       = theme->deviceColors();
  if (colors.isEmpty())
    return QColor(Qt::transparent);

  // Use the top gradient color's hue, then boost saturation/lightness for text
  const auto& base = colors.at((sourceId - 1) % colors.count()).first;
  const auto bg    = theme->getColor(QStringLiteral("base"));
  const bool dark  = bg.isValid() && bg.lightnessF() < 0.5;

  // Saturate and pick a lightness suitable for text against the background
  float h, s, l, a;
  base.getHslF(&h, &s, &l, &a);
  s = qBound(0.45f, s * 2.5f, 0.85f);
  l = dark ? 0.65f : 0.38f;
  return QColor::fromHslF(h, s, l, 1.0f);
}

//--------------------------------------------------------------------------------------------------
// String processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts a hex-encoded string into a UTF-8 decoded string with visible escape sequences.
 */
QString SerialStudio::hexToString(const QString& hex)
{
  QString raw = QString::fromUtf8(QByteArray::fromHex(QString(hex).remove(' ').toUtf8()));
  return escapeControlCharacters(raw);
}

/**
 * @brief Converts a string containing escape sequences into a space-separated hexadecimal string.
 */
QString SerialStudio::stringToHex(const QString& str)
{
  QString resolved = resolveEscapeSequences(str);
  return QString::fromLatin1(resolved.toUtf8().toHex(' '));
}

/**
 * @brief Converts a hexadecimal string into a raw QByteArray.
 */
QByteArray SerialStudio::hexToBytes(const QString& data)
{
  QString withoutSpaces = data;
  withoutSpaces.replace(QStringLiteral(" "), "");
  if (withoutSpaces.length() % 2 != 0) {
    qWarning() << data << "is not a valid hexadecimal array";
    return QByteArray();
  }

  bool ok;
  QByteArray array;
  for (int i = 0; i < withoutSpaces.length(); i += 2) {
    auto chr1       = withoutSpaces.at(i);
    auto chr2       = withoutSpaces.at(i + 1);
    QString byteStr = QStringLiteral("%1%2").arg(chr1, chr2);

    int byte = byteStr.toInt(&ok, 16);
    if (!ok) {
      qWarning() << data << "is not a valid hexadecimal array";
      return QByteArray();
    }

    array.append(static_cast<char>(byte));
  }

  return array;
}

/**
 * @brief Resolves C-style escape sequences in a string into their corresponding control characters.
 */
QString SerialStudio::resolveEscapeSequences(const QString& str)
{
  QString escapedStr = str;
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
 */
QString SerialStudio::escapeControlCharacters(const QString& str)
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

/**
 * @brief Rewrites legacy qrc:/rcc/... icon paths into the canonical qrc:/... form.
 */
QString SerialStudio::normalizeIconPath(const QString& path)
{
  if (path.isEmpty())
    return path;

  // Rewrite legacy qrc:/rcc/... paths to the current resource root.
  if (path.startsWith(QStringLiteral("qrc:/rcc/")))
    return QStringLiteral("qrc:/") + path.mid(9);

  if (path.startsWith(QStringLiteral("qrc:///rcc/")))
    return QStringLiteral("qrc:///") + path.mid(11);

  if (path.startsWith(QStringLiteral(":/rcc/")))
    return QStringLiteral(":/") + path.mid(6);

  return path;
}

//--------------------------------------------------------------------------------------------------
// Text encoding helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the QStringConverter encoding for natively-supported entries.
 */
static std::optional<QStringConverter::Encoding> nativeEncoding(SerialStudio::TextEncoding enc)
{
  switch (enc) {
    case SerialStudio::EncUtf8:
      return QStringConverter::Utf8;
    case SerialStudio::EncUtf16LE:
      return QStringConverter::Utf16LE;
    case SerialStudio::EncUtf16BE:
      return QStringConverter::Utf16BE;
    case SerialStudio::EncLatin1:
      return QStringConverter::Latin1;
    case SerialStudio::EncSystem:
      return QStringConverter::System;
    default:
      return std::nullopt;
  }
}

/**
 * @brief Returns the `QTextCodec` for multi-byte East-Asian encodings.
 */
static QTextCodec* legacyCodec(SerialStudio::TextEncoding enc)
{
  // Map the enum to a canonical codec name
  const char* name = nullptr;
  switch (enc) {
    case SerialStudio::EncGbk:
      name = "GBK";
      break;
    case SerialStudio::EncGb18030:
      name = "GB18030";
      break;
    case SerialStudio::EncBig5:
      name = "Big5";
      break;
    case SerialStudio::EncShiftJis:
      name = "Shift_JIS";
      break;
    case SerialStudio::EncEucJp:
      name = "EUC-JP";
      break;
    case SerialStudio::EncEucKr:
      name = "EUC-KR";
      break;
    default:
      break;
  }

  // Resolve the codec with a UTF-8 fallback so the caller never sees nullptr
  QTextCodec* codec = name ? QTextCodec::codecForName(name) : nullptr;
  if (!codec)
    codec = QTextCodec::codecForName("UTF-8");

  Q_ASSERT(codec != nullptr);
  return codec;
}

/**
 * @brief Returns the display labels for all supported text encodings.
 */
QStringList SerialStudio::textEncodings()
{
  static const QStringList list{
    tr("UTF-8"),
    tr("UTF-16 LE"),
    tr("UTF-16 BE"),
    tr("Latin-1"),
    tr("System"),
    tr("GBK"),
    tr("GB18030"),
    tr("Big5"),
    tr("Shift-JIS"),
    tr("EUC-JP"),
    tr("EUC-KR"),
  };
  return list;
}

/**
 * @brief Returns the canonical string name for a text encoding.
 */
QString SerialStudio::textEncodingName(SerialStudio::TextEncoding enc)
{
  // Map to a canonical short name (not translated, for persistence)
  switch (enc) {
    case EncUtf8:
      return QStringLiteral("UTF-8");
    case EncUtf16LE:
      return QStringLiteral("UTF-16LE");
    case EncUtf16BE:
      return QStringLiteral("UTF-16BE");
    case EncLatin1:
      return QStringLiteral("ISO-8859-1");
    case EncSystem:
      return QStringLiteral("System");
    case EncGbk:
      return QStringLiteral("GBK");
    case EncGb18030:
      return QStringLiteral("GB18030");
    case EncBig5:
      return QStringLiteral("Big5");
    case EncShiftJis:
      return QStringLiteral("Shift_JIS");
    case EncEucJp:
      return QStringLiteral("EUC-JP");
    case EncEucKr:
      return QStringLiteral("EUC-KR");
  }
  return QStringLiteral("UTF-8");
}

/**
 * @brief Resolves a persisted encoding name back to the enum.
 */
SerialStudio::TextEncoding SerialStudio::textEncodingFromName(const QString& name)
{
  // Guard-return for empty input and normalize the name
  if (name.isEmpty())
    return EncUtf8;

  const QString n = name.trimmed().toUpper();

  // Match against the canonical names and a few popular aliases
  if (n == QLatin1String("UTF-8") || n == QLatin1String("UTF8"))
    return EncUtf8;

  if (n == QLatin1String("UTF-16LE") || n == QLatin1String("UTF16LE"))
    return EncUtf16LE;

  if (n == QLatin1String("UTF-16BE") || n == QLatin1String("UTF16BE"))
    return EncUtf16BE;

  if (n == QLatin1String("ISO-8859-1") || n == QLatin1String("LATIN1")
      || n == QLatin1String("LATIN-1"))
    return EncLatin1;
  if (n == QLatin1String("SYSTEM") || n == QLatin1String("LOCALE"))
    return EncSystem;

  if (n == QLatin1String("GBK") || n == QLatin1String("CP936"))
    return EncGbk;

  if (n == QLatin1String("GB18030"))
    return EncGb18030;

  if (n == QLatin1String("BIG5") || n == QLatin1String("BIG-5"))
    return EncBig5;

  if (n == QLatin1String("SHIFT_JIS") || n == QLatin1String("SHIFT-JIS")
      || n == QLatin1String("SJIS") || n == QLatin1String("CP932"))
    return EncShiftJis;
  if (n == QLatin1String("EUC-JP") || n == QLatin1String("EUCJP"))
    return EncEucJp;

  if (n == QLatin1String("EUC-KR") || n == QLatin1String("EUCKR"))
    return EncEucKr;

  return EncUtf8;
}

/**
 * @brief Encodes a QString to raw bytes using the given text encoding.
 */
QByteArray SerialStudio::encodeText(const QString& text, SerialStudio::TextEncoding enc)
{
  // Fast path: empty input produces empty output
  if (text.isEmpty())
    return {};

  // Use QStringEncoder for natively-supported encodings
  if (const auto native = nativeEncoding(enc); native.has_value()) {
    QStringEncoder encoder(*native);
    return QByteArray(encoder.encode(text));
  }

  // Fall back to QTextCodec for East-Asian multi-byte encodings
  auto* codec = legacyCodec(enc);
  Q_ASSERT(codec != nullptr);
  return codec->fromUnicode(text);
}

/**
 * @brief Decodes raw bytes to a QString using the given text encoding.
 */
QString SerialStudio::decodeText(QByteArrayView bytes, SerialStudio::TextEncoding enc)
{
  // Fast path: empty input produces empty output
  if (bytes.isEmpty())
    return {};

  // Use QStringDecoder for natively-supported encodings
  if (const auto native = nativeEncoding(enc); native.has_value()) {
    QStringDecoder decoder(*native);
    return decoder.decode(bytes);
  }

  // Fall back to QTextCodec for East-Asian multi-byte encodings
  auto* codec = legacyCodec(enc);
  Q_ASSERT(codec != nullptr);
  return codec->toUnicode(bytes.constData(), static_cast<int>(bytes.size()));
}
