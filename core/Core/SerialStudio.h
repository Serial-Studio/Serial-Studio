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

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <QJsonValue>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QVariant>

#include "Core/ThirdParty/fast_float.h"

/**
 * @file SerialStudio.h
 * @brief The shared vocabulary every layer speaks: the enums, the widget-kind predicates and the
 *        Qt-Core-only string helpers, as a Q_NAMESPACE so QML keeps reading `SerialStudio.X`
 *        (spec 0077). Anything that needs a frame type, a licence, a theme or an icon registry
 *        lives above this file: Pipeline's FrameSupport and Ui's SerialStudioHelpers.
 */

namespace SerialStudio {
Q_NAMESPACE

/**
 * @brief Widget extension API version (spec 0038), published independently of the application
 *        version: a package declares the host range it supports and is refused outside it.
 *        Bump the minor for additive manifest keys, the major for anything a v1 package cannot
 *        survive.
 */
inline constexpr int kWidgetApiVersionMajor = 1;
inline constexpr int kWidgetApiVersionMinor = 0;

/**
 * @brief Decoding strategy for a continuous data stream.
 */
enum DecoderMethod {
  PlainText,
  Hexadecimal,
  Base64,
  Binary,
};
Q_ENUM_NS(DecoderMethod)

/**
 * @brief Scripting language used for the frame parser function.
 */
enum ScriptLanguage {
  JavaScript = 0,
  Lua        = 1,
  Native     = 2,
  Expression = 3,
};
Q_ENUM_NS(ScriptLanguage)

/**
 * @brief Strategy for detecting data frames inside a continuous stream.
 */
// clang-format off
enum FrameDetection
{
  EndDelimiterOnly     = 0x00,
  StartAndEndDelimiter = 0x01,
  NoDelimiters         = 0x02,
  StartDelimiterOnly   = 0x03
};
Q_ENUM_NS(FrameDetection)
// clang-format on

/**
 * @brief Character encoding used for QString/byte conversions. The fixed underlying type makes
 *        an out-of-range value (a settings file or project from a build with more codecs) a
 *        representable value the codec lookup can fall back on, not undefined behavior.
 */
enum TextEncoding : int {
  EncUtf8 = 0,
  EncUtf16LE,
  EncUtf16BE,
  EncLatin1,
  EncSystem,
  EncGbk,
  EncGb18030,
  EncBig5,
  EncShiftJis,
  EncEucJp,
  EncEucKr,
};
Q_ENUM_NS(TextEncoding)

/**
 * @brief Dashboard construction strategy.
 */
enum OperationMode {
  ProjectFile,
  ConsoleOnly,
  QuickPlot,
};
Q_ENUM_NS(OperationMode)

/**
 * @brief Interpolation modes for line-based plot widgets.
 */
enum InterpolationMode {
  InterpolationNone,
  InterpolationLinear,
  InterpolationZoh,
  InterpolationStem,
};
Q_ENUM_NS(InterpolationMode)

/**
 * @brief Sweep (oscilloscope) trigger modes for time-axis plots.
 */
enum SweepMode {
  SweepAuto,
  SweepNormal,
  SweepSingle,
};
Q_ENUM_NS(SweepMode)

/**
 * @brief Window function applied to the FFT/Waterfall input buffer. Persisted
 *        as an integer, so this list is append-only -- never reorder or remove.
 */
enum FFTWindow {
  FFTWindowRectangular,
  FFTWindowBartlett,
  FFTWindowHann,
  FFTWindowHamming,
  FFTWindowBlackman,
  FFTWindowBlackmanHarris,
  FFTWindowNuttall,
  FFTWindowBlackmanNuttall,
  FFTWindowFlatTop,
  FFTWindowWelch,
  FFTWindowBartlettHann,
  FFTWindowBohman,
  FFTWindowCosine,
  FFTWindowLanczos,
  FFTWindowParzen,
};
Q_ENUM_NS(FFTWindow)

/**
 * @brief Trigger edge polarity used to start a sweep.
 */
enum TriggerEdge {
  TriggerRising,
  TriggerFalling,
};
Q_ENUM_NS(TriggerEdge)

/**
 * @brief Available data-source bus types.
 */
enum class BusType {
  UART,
  Network,
  BluetoothLE,
#ifdef BUILD_COMMERCIAL
  Audio,
  ModBus,
  CanBus,
  RawUsb,
  HidDevice,
  Process,
  Mqtt,
  OpcUa,
  S7,
  EthernetIp,
  Iec104,
#endif
};
Q_ENUM_NS(BusType)

/**
 * @brief Visualization widget types available for groups.
 */
enum GroupWidget {
  DataGrid,
  Accelerometer,
  Gyroscope,
  GPS,
  MultiPlot,
  NoGroupWidget,
  Plot3D,
  ImageView,
  Painter,
  WebView,
  BarPanel,
};
Q_ENUM_NS(GroupWidget)

/**
 * @brief Visualization widget types available for datasets.
 */
enum DatasetWidget {
  Bar,
  Gauge,
  Compass,
  Meter,
  NoDatasetWidget
};
Q_ENUM_NS(DatasetWidget)

/**
 * @brief Distinguishes input (visualization) groups from output (control) groups.
 */
enum GroupType {
  GroupInput  = 0,
  GroupOutput = 1,
};
Q_ENUM_NS(GroupType)

/**
 * @brief Interactive output widget types for bidirectional communication.
 */
enum OutputWidgetType {
  OutputButton,
  OutputSlider,
  OutputToggle,
  OutputTextField,
  OutputKnob,
};
Q_ENUM_NS(OutputWidgetType)

/**
 * @brief Dashboard widget kinds. Ordinals are persisted state, so the list is append-only in
 *        both build configurations; BarPanel (90) and Extension (100) are pinned so nothing moves
 *        across the commercial block. Every third-party package shares DashboardExtension and
 *        takes its scope, title and icon from the UI::WidgetExtensions descriptor.
 */
enum DashboardWidget {
  DashboardTerminal,
  DashboardDataGrid,
  DashboardMultiPlot,
  DashboardAccelerometer,
  DashboardGyroscope,
  DashboardGPS,
  DashboardPlot3D,
  DashboardFFT,
  DashboardLED,
  DashboardPlot,
  DashboardBar,
  DashboardGauge,
  DashboardCompass,
  DashboardMeter,
  DashboardClock,
  DashboardStopwatch,
  DashboardWebView,
  DashboardNoWidget,
#ifdef BUILD_COMMERCIAL
  DashboardImageView,
  DashboardOutputPanel,
  DashboardNotificationLog,
  DashboardWaterfall,
  DashboardPainter,
#endif
  DashboardBarPanel  = 90,
  DashboardExtension = 100,
};
Q_ENUM_NS(DashboardWidget)

/**
 * @brief Bit-flag options for dataset configurations.
 */
// clang-format off
enum DatasetOption
{
  DatasetGeneric   = 0b00000000,
  DatasetPlot      = 0b00000001,
  DatasetFFT       = 0b00000010,
  DatasetBar       = 0b00000100,
  DatasetGauge     = 0b00001000,
  DatasetCompass   = 0b00010000,
  DatasetLED       = 0b00100000,
  DatasetWaterfall = 0b01000000,
  DatasetMeter     = 0b10000000,
};
Q_ENUM_NS(DatasetOption)
// clang-format on

/**
 * @brief Dashboard widget index to (widget kind, entity id), the map the widget walkers build.
 */
typedef QMap<int, QPair<DashboardWidget, int>> WidgetMap;

/**
 * @brief Render mode for a plot's X-axis.
 */
enum class XAxisMode : quint8 {
  Time    = 0,  ///< Plot against the source time base
  Samples = 1,  ///< Plot against the running sample index
  Dataset = 2,  ///< Plot against another dataset's value
};

/**
 * @brief Resolved X-axis policy: the render mode plus the X source dataset id (Dataset mode).
 */
struct XAxisPolicy {
  XAxisMode mode = XAxisMode::Time;  ///< Resolved render mode
  int xDatasetId = -1;               ///< X source dataset uniqueId (Dataset mode; -1 otherwise)
};

// clang-format off
[[nodiscard]] bool isGroupWidget(DashboardWidget widget);
[[nodiscard]] bool isDashboardTool(DashboardWidget w);
[[nodiscard]] bool isDatasetWidget(DashboardWidget widget);
[[nodiscard]] bool dashboardWidgetPaintsTitle(DashboardWidget w);
[[nodiscard]] QString dashboardWidgetTitle(DashboardWidget w);
[[nodiscard]] QString dashboardWidgetIcon(DashboardWidget w, bool large = false);
[[nodiscard]] QString dashboardWidgetIconId(DashboardWidget w);
[[nodiscard]] bool groupWidgetEligibleForWorkspace(DashboardWidget w);
[[nodiscard]] bool datasetWidgetEligibleForWorkspace(DashboardWidget w);
[[nodiscard]] QString persistedExtensionTypeToken(const QString& packageId);
// clang-format on

[[nodiscard]] QString groupWidgetId(GroupWidget widget);
[[nodiscard]] GroupWidget groupWidgetFromId(const QString& id);
[[nodiscard]] QString datasetWidgetId(DatasetWidget widget);
[[nodiscard]] DatasetWidget datasetWidgetFromId(const QString& id);

[[nodiscard]] bool searchMatches(const QString& query, const QString& text);

[[nodiscard]] QString hexToString(const QString& hex);
[[nodiscard]] QString stringToHex(const QString& str);
[[nodiscard]] QByteArray hexToBytes(const QString& data);
[[nodiscard]] QString resolveEscapeSequences(const QString& str);
[[nodiscard]] QString escapeControlCharacters(const QString& str);

[[nodiscard]] QString normalizeIconPath(const QString& path);

[[nodiscard]] QStringList outputControlSizes();

[[nodiscard]] QStringList textEncodings();
[[nodiscard]] QString textEncodingName(TextEncoding enc);
[[nodiscard]] TextEncoding textEncodingFromName(const QString& name);

[[nodiscard]] double toDouble(QStringView text, bool* ok = nullptr) noexcept;
[[nodiscard]] double toDouble(QByteArrayView text, bool* ok = nullptr) noexcept;
[[nodiscard]] double toDouble(const QString& text, bool* ok = nullptr) noexcept;
[[nodiscard]] double toDouble(const QByteArray& text, bool* ok = nullptr) noexcept;
[[nodiscard]] double toDouble(const char* text, bool* ok = nullptr) noexcept;
[[nodiscard]] double toDouble(const QVariant& value, bool* ok = nullptr) noexcept;
[[nodiscard]] double toDouble(const QJsonValue& value, bool* ok = nullptr) noexcept;
[[nodiscard]] double toDouble(const QJsonValue& value, double defaultValue) noexcept;

/**
 * @brief Locale-independent QString::toDouble() replacement built on fast_float.
 */
[[nodiscard]] Q_ALWAYS_INLINE double toDouble(QStringView text, bool* ok) noexcept
{
  const char16_t* first = text.utf16();
  const char16_t* last  = first + text.size();

  for (; first < last && QChar::isSpace(*first); ++first)
    continue;

  for (; last > first && QChar::isSpace(*(last - 1)); --last)
    continue;

  constexpr auto format =
    fast_float::chars_format::general | fast_float::chars_format::allow_leading_plus;

  double value      = 0.0;
  const auto result = fast_float::from_chars(first, last, value, format);
  const bool good   = (result.ec == std::errc()) && (result.ptr == last);
  if (ok)
    *ok = good;

  return good ? value : 0.0;
}

/**
 * @brief Byte-level toDouble() for QByteArray / raw char data (same grammar as above).
 */
[[nodiscard]] Q_ALWAYS_INLINE double toDouble(QByteArrayView text, bool* ok) noexcept
{
  const char* first = text.data();
  const char* last  = first + text.size();

  for (; first < last && QChar::isSpace(char32_t(uchar(*first))); ++first)
    continue;

  for (; last > first && QChar::isSpace(char32_t(uchar(*(last - 1)))); --last)
    continue;

  constexpr auto format =
    fast_float::chars_format::general | fast_float::chars_format::allow_leading_plus;

  double value      = 0.0;
  const auto result = fast_float::from_chars(first, last, value, format);
  const bool good   = (result.ec == std::errc()) && (result.ptr == last);
  if (ok)
    *ok = good;

  return good ? value : 0.0;
}

/**
 * @brief Exact-match QString overload (avoids QVariant/QJsonValue conversion ambiguity).
 */
[[nodiscard]] Q_ALWAYS_INLINE double toDouble(const QString& text, bool* ok) noexcept
{
  return toDouble(QStringView(text), ok);
}

/**
 * @brief Exact-match QByteArray overload (avoids QVariant conversion ambiguity).
 */
[[nodiscard]] Q_ALWAYS_INLINE double toDouble(const QByteArray& text, bool* ok) noexcept
{
  return toDouble(QByteArrayView(text), ok);
}

/**
 * @brief Null-safe C-string convenience overload.
 */
[[nodiscard]] Q_ALWAYS_INLINE double toDouble(const char* text, bool* ok) noexcept
{
  return toDouble(QByteArrayView(text, qstrlen(text)), ok);
}

/**
 * @brief QVariant::toDouble() replacement: string payloads parse through fast_float.
 */
[[nodiscard]] inline double toDouble(const QVariant& value, bool* ok) noexcept
{
  switch (value.typeId()) {
    case QMetaType::QString:
      return toDouble(value.toString(), ok);
    case QMetaType::QByteArray:
      return toDouble(value.toByteArray(), ok);
    case QMetaType::QJsonValue:
      return toDouble(value.toJsonValue(), ok);
    default:
      return value.toDouble(ok);
  }
}

/**
 * @brief QJsonValue::toDouble() replacement: JSON numbers pass through, strings parse.
 */
[[nodiscard]] inline double toDouble(const QJsonValue& value, bool* ok) noexcept
{
  if (value.isDouble()) {
    if (ok)
      *ok = true;

    return value.toDouble();
  }

  if (value.isString())
    return toDouble(value.toString(), ok);

  if (ok)
    *ok = false;

  return 0.0;
}

/**
 * @brief Default-value convenience mirroring QJsonValue::toDouble(double).
 */
[[nodiscard]] inline double toDouble(const QJsonValue& value, double defaultValue) noexcept
{
  bool ok             = false;
  const double parsed = toDouble(value, &ok);
  return ok ? parsed : defaultValue;
}
}  // namespace SerialStudio
