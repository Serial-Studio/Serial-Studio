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

#include "SerialStudio.h"

#include <QHash>

#include "CSV/Player.h"
#include "MDF4/Player.h"
#include "Misc/IconRegistry.h"
#include "Misc/ThemeManager.h"
#include "UI/WidgetExtensions.h"

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
 * @brief Classifies a group's X-axis mode from its front dataset's encoding. Empty groups and the
 * time sentinel map to Time, the samples sentinel to Samples, any dataset id to Dataset. Callers
 * apply their own guards on the empty case (Dashboard's !empty guard sends it to the samples path,
 * ProjectEditor shows "Time"); that asymmetry is intentional and preserved here.
 */
SerialStudio::XAxisMode SerialStudio::groupXAxisMode(const DataModel::Group& g)
{
  if (g.datasets.empty())
    return XAxisMode::Time;

  const int frontXAxisId = g.datasets.front().xAxisId;
  if (frontXAxisId == DataModel::kXAxisSamples)
    return XAxisMode::Samples;

  if (frontXAxisId == DataModel::kXAxisTime)
    return XAxisMode::Time;

  return XAxisMode::Dataset;
}

/**
 * @brief Resolves a dataset's X-axis policy: Time for the time sentinel, Dataset when a licensed
 * dataset X source resolves against the live map, else Samples (the unlicensed/unresolved degrade).
 */
SerialStudio::XAxisPolicy SerialStudio::resolveXAxisPolicy(
  const DataModel::Dataset& d, const QMap<int, DataModel::Dataset>& datasets)
{
  if (d.xAxisId == DataModel::kXAxisTime)
    return {XAxisMode::Time, -1};

  if (d.xAxisId >= 0 && datasets.contains(d.xAxisId))
    return {XAxisMode::Dataset, d.xAxisId};

  return {XAxisMode::Samples, -1};
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
    case DashboardWebView:
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
 * @brief Maps a dashboard widget to its logical icon name in the "widgets"
 *        registry category.
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
 * @brief Returns whether a group contributes any widgets to Dashboard's walker.
 */
bool SerialStudio::groupEligibleForWorkspace(const DataModel::Group& g)
{
  Q_UNUSED(g);
  return true;
}

/**
 * @brief Counts the group-scope extension widgets a project contributes. Both extension scopes
 *        share one enum value, so the dashboard bucket lists the group-scope widgets first and
 *        offsets the dataset-scope ones by this count; every site that mirrors that numbering for
 *        workspace references needs the same offset.
 */
int SerialStudio::extensionGroupWidgetCount(const std::vector<DataModel::Group>& groups)
{
  int count = 0;
  for (const auto& group : groups)
    if (getDashboardWidget(group) == DashboardExtension)
      ++count;

  return count;
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
    case DashboardWebView:
      return tr("Web Views");
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
    case DashboardExtension:
      return tr("Extension Widgets");
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
 * @brief Returns whether @p id names an installed package attaching to @p scope. Consulted only
 *        after every built-in comparison misses, and it can only ever answer DashboardExtension:
 *        the manifest validator refuses a package id equal to a built-in widget string, so no
 *        catalog data resolves to a Pro widget type on any build.
 */
[[nodiscard]] static bool isWidgetExtension(const QString& id, UI::WidgetExtensions::Scope scope)
{
  if (id.isEmpty())
    return false;

  static auto& catalog = UI::WidgetExtensions::instance();
  return catalog.contains(id) && catalog.descriptor(id).scope == scope;
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

  if (widget == "webview")
    return DashboardWebView;

#ifdef BUILD_COMMERCIAL
  if (widget == "image")
    return DashboardImageView;

  if (widget == "notification-log")
    return DashboardNotificationLog;

  if (widget == "painter")
    return DashboardPainter;
#else
  if (widget == "painter")
    return DashboardDataGrid;
#endif

  if (isWidgetExtension(widget, UI::WidgetExtensions::GroupScope))
    return DashboardExtension;

  return DashboardNoWidget;
}

/**
 * @brief Returns true when the widget is a dashboard tool (terminal, notification log,
 *        clock, stopwatch); tools live in external windows, never on the canvas.
 */
bool SerialStudio::isDashboardTool(const SerialStudio::DashboardWidget w)
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
bool SerialStudio::dashboardWidgetPaintsTitle(const SerialStudio::DashboardWidget w)
{
  return w == DashboardBar || w == DashboardGauge || w == DashboardMeter;
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

  else if (isWidgetExtension(dataset.widget, UI::WidgetExtensions::DatasetScope))
    list.append(DashboardExtension);

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
    case WebView:
      return "webview";
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

  if (id == "webview")
    return WebView;

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
 * @brief Returns true when a player that stores post-transform values is open. The Sessions
 *        player replays the stored final values, so transforms must not run again: they read
 *        live inputs (data tables) that do not exist during playback.
 */
bool SerialStudio::isFinalValuePlayerOpen()
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
 * @brief Resolves a dataset's display color: valid explicit override wins, else the
 *        automatic theme-palette color for the dataset's index.
 */
QColor SerialStudio::getDatasetColor(const DataModel::Dataset& dataset)
{
  if (!dataset.color.isEmpty()) {
    const auto color = QColor::fromString(dataset.color);
    if (color.isValid())
      return color;
  }

  return getDatasetColor(dataset.index);
}

/**
 * @brief Returns the first valid per-dataset color override in @p group; invalid when none.
 */
QColor SerialStudio::getGroupColorOverride(const DataModel::Group& group)
{
  for (const auto& dataset : group.datasets) {
    if (dataset.color.isEmpty())
      continue;

    const auto color = QColor::fromString(dataset.color);
    if (color.isValid())
      return color;
  }

  return {};
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
  if (sourceId <= 0)
    return QColor(Qt::transparent);

  static const auto* theme = &Misc::ThemeManager::instance();
  const auto& colors       = theme->deviceColors();
  if (colors.isEmpty())
    return QColor(Qt::transparent);

  const auto& base = colors.at((sourceId - 1) % colors.count()).first;
  const auto bg    = theme->getColor(QStringLiteral("base"));
  const bool dark  = bg.isValid() && bg.lightnessF() < 0.5;

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
// Text encoding helpers
//--------------------------------------------------------------------------------------------------

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
