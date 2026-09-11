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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Core/SerialStudio.h"

#include <QCoreApplication>
#include <QHash>

#include "Core/IconRegistry.h"

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
    case DashboardWebView:
    case DashboardBarPanel:
#ifdef BUILD_COMMERCIAL
    case DashboardImageView:
    case DashboardOutputPanel:
    case DashboardNotificationLog:
    case DashboardPainter:
#endif
      return true;
    default:
      return false;
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
  }
}

/**
 * @brief Maps a dashboard widget to its logical icon name in the "widgets" registry category.
 */
static QString dashboardWidgetIconName(const SerialStudio::DashboardWidget w)
{
  switch (w) {
    case SerialStudio::DashboardDataGrid:
      return QStringLiteral("datagrid");
    case SerialStudio::DashboardMultiPlot:
      return QStringLiteral("multiplot");
    case SerialStudio::DashboardAccelerometer:
      return QStringLiteral("accelerometer");
    case SerialStudio::DashboardGyroscope:
      return QStringLiteral("gyroscope");
    case SerialStudio::DashboardGPS:
      return QStringLiteral("gps");
    case SerialStudio::DashboardFFT:
      return QStringLiteral("fft");
    case SerialStudio::DashboardLED:
      return QStringLiteral("led");
    case SerialStudio::DashboardPlot:
      return QStringLiteral("plot");
    case SerialStudio::DashboardBar:
      return QStringLiteral("bar");
    case SerialStudio::DashboardGauge:
      return QStringLiteral("gauge");
    case SerialStudio::DashboardCompass:
      return QStringLiteral("compass");
    case SerialStudio::DashboardMeter:
      return QStringLiteral("meter");
    case SerialStudio::DashboardTerminal:
      return QStringLiteral("terminal");
    case SerialStudio::DashboardClock:
      return QStringLiteral("clock");
    case SerialStudio::DashboardStopwatch:
      return QStringLiteral("stopwatch");
    case SerialStudio::DashboardPlot3D:
      return QStringLiteral("plot3d");
    case SerialStudio::DashboardWebView:
      return QStringLiteral("webview");
    case SerialStudio::DashboardBarPanel:
      return QStringLiteral("barpanel");
#ifdef BUILD_COMMERCIAL
    case SerialStudio::DashboardImageView:
      return QStringLiteral("image");
    case SerialStudio::DashboardOutputPanel:
      return QStringLiteral("output-panel");
    case SerialStudio::DashboardNotificationLog:
      return QStringLiteral("notification-log");
    case SerialStudio::DashboardWaterfall:
      return QStringLiteral("waterfall");
    case SerialStudio::DashboardPainter:
      return QStringLiteral("painter");
#endif
    case SerialStudio::DashboardNoWidget:
      return QStringLiteral("group");
    default:
      return QStringLiteral("group");
  }
}

/**
 * @brief Retrieves the icon path for a specified dashboard widget via the icon
 *        registry (16 px tier for the small variant, 32 px for the large one).
 */
QString SerialStudio::dashboardWidgetIcon(const DashboardWidget w, const bool large)
{
  static auto& registry = Misc::IconRegistry::instance();
  return registry.icon(QStringLiteral("widgets"), dashboardWidgetIconName(w), large ? 32 : 16);
}

/**
 * @brief Returns the icon-registry id ("widgets/<name>") for a dashboard widget, letting
 *        consumers resolve the artwork tier for their own display size.
 */
QString SerialStudio::dashboardWidgetIconId(const DashboardWidget w)
{
  return QStringLiteral("widgets/") + dashboardWidgetIconName(w);
}

/**
 * @brief The persisted widget-type token of an extension package ("ext:<id>", spec 0038): the
 *        string a project stores where a built-in widget stores its kind.
 */
QString SerialStudio::persistedExtensionTypeToken(const QString& packageId)
{
  return QStringLiteral("ext:") + packageId;
}

/**
 * @brief Returns whether a group-level widget key should appear on workspaces.
 */
bool SerialStudio::groupWidgetEligibleForWorkspace(const DashboardWidget w)
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
bool SerialStudio::datasetWidgetEligibleForWorkspace(const DashboardWidget w)
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
 * @brief Translates one dashboard widget title in the "SerialStudio" context the .ts files carry.
 */
static QString widgetTitle(const char* source)
{
  return QCoreApplication::translate("SerialStudio", source);
}

/**
 * @brief Retrieves the display title for a specified dashboard widget.
 */
QString SerialStudio::dashboardWidgetTitle(const DashboardWidget w)
{
  switch (w) {
    case DashboardDataGrid:
      return widgetTitle("Data Grids");
    case DashboardMultiPlot:
      return widgetTitle("Multi-Plots");
    case DashboardAccelerometer:
      return widgetTitle("Accelerometers");
    case DashboardGyroscope:
      return widgetTitle("Gyroscopes");
    case DashboardGPS:
      return widgetTitle("GPS");
    case DashboardFFT:
      return widgetTitle("FFT Plots");
    case DashboardLED:
      return widgetTitle("LED Panels");
    case DashboardPlot:
      return widgetTitle("Data Plots");
    case DashboardBar:
      return widgetTitle("Bars");
    case DashboardGauge:
      return widgetTitle("Gauges");
    case DashboardTerminal:
      return widgetTitle("Terminal");
    case DashboardClock:
      return widgetTitle("Clock");
    case DashboardStopwatch:
      return widgetTitle("Stopwatch");
    case DashboardCompass:
      return widgetTitle("Compasses");
    case DashboardMeter:
      return widgetTitle("Meters");
    case DashboardPlot3D:
      return widgetTitle("3D Plots");
    case DashboardWebView:
      return widgetTitle("Web Views");
    case DashboardBarPanel:
      return widgetTitle("Bar Panels");
#ifdef BUILD_COMMERCIAL
    case DashboardImageView:
      return widgetTitle("Image Views");
    case DashboardOutputPanel:
      return widgetTitle("Output Panels");
    case DashboardNotificationLog:
      return widgetTitle("Notifications");
    case DashboardWaterfall:
      return widgetTitle("Waterfalls");
    case DashboardPainter:
      return widgetTitle("Canvas Widgets");
#endif
    case DashboardExtension:
      return widgetTitle("Extension Widgets");
    default:
      return QString();
  }
}

/**
 * @brief Returns true when the widget is a dashboard tool (terminal, notification log,
 *        clock, stopwatch); tools live in external windows, never on the canvas.
 */
bool SerialStudio::isDashboardTool(const DashboardWidget w)
{
#ifdef BUILD_COMMERCIAL
  if (w == DashboardNotificationLog)
    return true;
#endif

  return w == DashboardTerminal || w == DashboardClock || w == DashboardStopwatch;
}

/**
 * @brief Returns true when the widget paints its own title on the instrument face; these
 *        widgets default to the "painted" freeze-title mode instead of "bar".
 */
bool SerialStudio::dashboardWidgetPaintsTitle(const DashboardWidget w)
{
  return w == DashboardBar || w == DashboardGauge || w == DashboardMeter;
}

//--------------------------------------------------------------------------------------------------
// Search matching
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lowercases @p text and folds the separators users type inconsistently (dash, underscore,
 *        dot, slash) into single spaces, so "X-Axis" and "x axis" normalize identically.
 */
static QString normalizedSearchText(const QString& text)
{
  const QString composed = text.normalized(QString::NormalizationForm_C);
  QString out;
  out.reserve(composed.size());
  for (const QChar& c : composed)
    if (c == QChar('-') || c == QChar('_') || c == QChar('.') || c == QChar('/'))
      out.append(QChar(' '));
    else
      out.append(c.toLower());

  return out.simplified();
}

/**
 * @brief Separator- and case-insensitive search predicate shared by every search box: each query
 *        token must appear in the normalized text, either as a substring or with the spaces
 *        squashed out (so "x axis", "X-Axis" and "xaxis" all match "X-Axis Selection"). An empty
 *        query matches everything.
 */
bool SerialStudio::searchMatches(const QString& query, const QString& text)
{
  const QString needle = normalizedSearchText(query);
  if (needle.isEmpty())
    return true;

  const QString hay = normalizedSearchText(text);
  QString squashed  = hay;
  squashed.remove(QChar(' '));

  const auto tokens = needle.split(QChar(' '), Qt::SkipEmptyParts);
  for (const auto& token : tokens)
    if (!hay.contains(token) && !squashed.contains(token))
      return false;

  return true;
}

//--------------------------------------------------------------------------------------------------
// Widget id strings
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves the ID string associated with a specified group widget.
 */
QString SerialStudio::groupWidgetId(const GroupWidget widget)
{
  switch (widget) {
    case DataGrid:
      return QStringLiteral("datagrid");
    case Accelerometer:
      return QStringLiteral("accelerometer");
    case Gyroscope:
      return QStringLiteral("gyro");
    case GPS:
      return QStringLiteral("gps");
    case MultiPlot:
      return QStringLiteral("multiplot");
    case Plot3D:
      return QStringLiteral("plot3d");
    case WebView:
      return QStringLiteral("webview");
    case BarPanel:
      return QStringLiteral("barpanel");
#ifdef BUILD_COMMERCIAL
    case ImageView:
      return QStringLiteral("image");
    case Painter:
      return QStringLiteral("painter");
#endif
    default:
      return QString();
  }
}

/**
 * @brief Determines the group widget type from a given ID string.
 */
SerialStudio::GroupWidget SerialStudio::groupWidgetFromId(const QString& id)
{
  if (id == QStringLiteral("datagrid"))
    return DataGrid;

  if (id == QStringLiteral("accelerometer"))
    return Accelerometer;

  if (id == QStringLiteral("gyro") || id == QStringLiteral("gyroscope"))
    return Gyroscope;

  if (id == QStringLiteral("gps") || id == QStringLiteral("map"))
    return GPS;

  if (id == QStringLiteral("multiplot"))
    return MultiPlot;

  if (id == QStringLiteral("plot3d"))
    return Plot3D;

  if (id == QStringLiteral("webview"))
    return WebView;

  if (id == QStringLiteral("barpanel"))
    return BarPanel;

#ifdef BUILD_COMMERCIAL
  if (id == QStringLiteral("image"))
    return ImageView;

  if (id == QStringLiteral("painter"))
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
      return QStringLiteral("bar");
    case Gauge:
      return QStringLiteral("gauge");
    case Compass:
      return QStringLiteral("compass");
    case Meter:
      return QStringLiteral("meter");
    default:
      return QString();
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
  withoutSpaces.replace(QStringLiteral(" "), QString());
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
  QString escapedStr;
  escapedStr.reserve(str.size());

  for (int i = 0; i < str.size(); ++i) {
    const QChar current = str.at(i);
    if (current != u'\\' || i + 1 >= str.size()) {
      escapedStr.append(current);
      continue;
    }

    const QChar next = str.at(i + 1);
    ++i;
    switch (next.unicode()) {
      case u'a':
        escapedStr.append(QChar(u'\a'));
        break;
      case u'b':
        escapedStr.append(QChar(u'\b'));
        break;
      case u'f':
        escapedStr.append(QChar(u'\f'));
        break;
      case u'n':
        escapedStr.append(QChar(u'\n'));
        break;
      case u'r':
        escapedStr.append(QChar(u'\r'));
        break;
      case u't':
        escapedStr.append(QChar(u'\t'));
        break;
      case u'v':
        escapedStr.append(QChar(u'\v'));
        break;
      case u'\\':
        escapedStr.append(QChar(u'\\'));
        break;
      default:
        escapedStr.append(current).append(next);
        break;
    }
  }

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

  if (path.startsWith(QStringLiteral("qrc:/rcc/")))
    return QStringLiteral("qrc:/") + path.mid(9);

  if (path.startsWith(QStringLiteral("qrc:///rcc/")))
    return QStringLiteral("qrc:///") + path.mid(11);

  if (path.startsWith(QStringLiteral(":/rcc/")))
    return QStringLiteral(":/") + path.mid(6);

  return path;
}

//--------------------------------------------------------------------------------------------------
// Output control metrics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the display labels of the output-control size classes, ordered as
 *        DataModel::OutputWidgetSize.
 */
QStringList SerialStudio::outputControlSizes()
{
  static const QStringList list{
    widgetTitle("Small"),
    widgetTitle("Normal"),
    widgetTitle("Large"),
    widgetTitle("Extra Large"),
  };
  return list;
}

//--------------------------------------------------------------------------------------------------
// Text encoding names
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the display labels for all supported text encodings.
 */
QStringList SerialStudio::textEncodings()
{
  static const QStringList list{
    widgetTitle("UTF-8"),
    widgetTitle("UTF-16 LE"),
    widgetTitle("UTF-16 BE"),
    widgetTitle("Latin-1"),
    widgetTitle("System"),
    widgetTitle("GBK"),
    widgetTitle("GB18030"),
    widgetTitle("Big5"),
    widgetTitle("Shift-JIS"),
    widgetTitle("EUC-JP"),
    widgetTitle("EUC-KR"),
  };
  return list;
}

/**
 * @brief Returns the canonical string name for a text encoding.
 */
QString SerialStudio::textEncodingName(const TextEncoding enc)
{
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
  if (name.isEmpty())
    return EncUtf8;

  const QString n = name.trimmed().toUpper();

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
