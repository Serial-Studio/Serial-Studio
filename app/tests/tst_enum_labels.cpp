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

#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTest>

#include "API/EnumLabels.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing. This is a GPL (non-BUILD_COMMERCIAL) unit
// binary, so only GPL-reachable enumerators are exercised; the commercial gaps in BusType and
// DashboardWidget are covered as "falls back to unknown" cases instead.

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Matches the slug format every EnumLabels function promises: lowercase ASCII words
 *        joined by single hyphens, never empty.
 */
static bool isWellFormedSlug(const QString& slug)
{
  static const QRegularExpression kPattern(QStringLiteral("^[a-z0-9]+(-[a-z0-9]+)*$"));
  return kPattern.match(slug).hasMatch();
}

/**
 * @brief Byte-level contract of the API::EnumLabels slug/label registries.
 */
class TstEnumLabels : public QObject {
  Q_OBJECT

private slots:
  void busTypeGplSweep_data();
  void busTypeGplSweep();
  void busTypeSlugsAreWellFormedAndUnique();
  void busTypeUnknownGapsFallBackToDefault_data();
  void busTypeUnknownGapsFallBackToDefault();

  void frameDetectionFullSweep_data();
  void frameDetectionFullSweep();
  void frameDetectionOutOfRange_data();
  void frameDetectionOutOfRange();

  void decoderMethodFullSweep_data();
  void decoderMethodFullSweep();
  void decoderMethodOutOfRange_data();
  void decoderMethodOutOfRange();

  void operationModeFullSweep_data();
  void operationModeFullSweep();
  void operationModeOutOfRange_data();
  void operationModeOutOfRange();

  void scriptLanguageFullSweep_data();
  void scriptLanguageFullSweep();
  void scriptLanguageOutOfRange_data();
  void scriptLanguageOutOfRange();

  void groupWidgetFullSweep_data();
  void groupWidgetFullSweep();
  void groupWidgetSlugsAreWellFormedAndUnique();
  void groupWidgetOutOfRange_data();
  void groupWidgetOutOfRange();

  void datasetWidgetFullSweep_data();
  void datasetWidgetFullSweep();
  void datasetWidgetSlugsAreWellFormedAndUnique();
  void datasetWidgetOutOfRange_data();
  void datasetWidgetOutOfRange();

  void dashboardWidgetGplSweepRoundTrips_data();
  void dashboardWidgetGplSweepRoundTrips();
  void dashboardWidgetFromSlugAcceptsAliasesCaseAndWhitespace_data();
  void dashboardWidgetFromSlugAcceptsAliasesCaseAndWhitespace();
  void dashboardWidgetFromSlugBogusOrEmptyMissesToMinusOne();
  void dashboardWidgetCommercialGapsFallBackToUnknownInGplBuild();

  void datasetOptionSingleBitSweep_data();
  void datasetOptionSingleBitSweep();
  void datasetOptionSingleBitRoundTrip();
  void datasetOptionFromSlugMissReturnsZero();
  void datasetOptionsBitsToSlugsCombinedMask();
  void datasetOptionsSlugsToBitsIsTheInverse();
  void datasetOptionsLabelHumanReadable_data();
  void datasetOptionsLabelHumanReadable();
};

//--------------------------------------------------------------------------------------------------
// BusType
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::busTypeGplSweep_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");
  QTest::addColumn<QString>("label");

  QTest::newRow("UART") << static_cast<int>(SerialStudio::BusType::UART) << QStringLiteral("uart")
                        << QStringLiteral("UART (serial port)");
  QTest::newRow("Network") << static_cast<int>(SerialStudio::BusType::Network)
                           << QStringLiteral("network") << QStringLiteral("Network (TCP/UDP)");
  QTest::newRow("BluetoothLE") << static_cast<int>(SerialStudio::BusType::BluetoothLE)
                               << QStringLiteral("bluetooth-le") << QStringLiteral("Bluetooth LE");
}

/**
 * @brief Every BusType enumerator that survives a GPL build resolves to its exact slug and label.
 */
void TstEnumLabels::busTypeGplSweep()
{
  QFETCH(int, value);
  QFETCH(QString, slug);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::busTypeSlug(value), slug);
  QCOMPARE(API::EnumLabels::busTypeLabel(value), label);
}

/**
 * @brief GPL-reachable BusType slugs are well-formed and distinct from one another.
 */
void TstEnumLabels::busTypeSlugsAreWellFormedAndUnique()
{
  QSet<QString> slugs;
  for (auto value : {SerialStudio::BusType::UART,
                     SerialStudio::BusType::Network,
                     SerialStudio::BusType::BluetoothLE}) {
    const auto slug = API::EnumLabels::busTypeSlug(static_cast<int>(value));
    QVERIFY(isWellFormedSlug(slug));
    slugs.insert(slug);
  }

  QCOMPARE(slugs.size(), qsizetype(3));
}

void TstEnumLabels::busTypeUnknownGapsFallBackToDefault_data()
{
  QTest::addColumn<int>("value");

  QTest::newRow("commercial-Audio-ordinal") << 3;
  QTest::newRow("commercial-ModBus-ordinal") << 4;
  QTest::newRow("commercial-Mqtt-ordinal") << 9;
  QTest::newRow("commercial-OpcUa-ordinal") << 10;
  QTest::newRow("negative") << -1;
  QTest::newRow("far-out-of-range") << 100;
}

/**
 * @brief In a GPL build the commercial BusType ordinals are not compiled in, so casting their
 *        integer positions falls through the switch exactly like any other unrecognized value.
 */
void TstEnumLabels::busTypeUnknownGapsFallBackToDefault()
{
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::busTypeSlug(value), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::busTypeLabel(value), QStringLiteral("Unknown"));
}

//--------------------------------------------------------------------------------------------------
// FrameDetection
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::frameDetectionFullSweep_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");
  QTest::addColumn<QString>("label");

  QTest::newRow("EndDelimiterOnly")
    << static_cast<int>(SerialStudio::EndDelimiterOnly) << QStringLiteral("end-delimiter")
    << QStringLiteral("End delimiter only (split on frameEnd)");
  QTest::newRow("StartAndEndDelimiter")
    << static_cast<int>(SerialStudio::StartAndEndDelimiter) << QStringLiteral("start-end-delimiter")
    << QStringLiteral("Start and end delimiters (split between frameStart and frameEnd)");
  QTest::newRow("NoDelimiters") << static_cast<int>(SerialStudio::NoDelimiters)
                                << QStringLiteral("no-delimiters")
                                << QStringLiteral("No delimiters (raw byte stream)");
  QTest::newRow("StartDelimiterOnly")
    << static_cast<int>(SerialStudio::StartDelimiterOnly) << QStringLiteral("start-delimiter")
    << QStringLiteral("Start delimiter only");
}

/**
 * @brief Every FrameDetection enumerator resolves to its exact slug and label.
 */
void TstEnumLabels::frameDetectionFullSweep()
{
  QFETCH(int, value);
  QFETCH(QString, slug);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::frameDetectionSlug(value), slug);
  QCOMPARE(API::EnumLabels::frameDetectionLabel(value), label);
}

void TstEnumLabels::frameDetectionOutOfRange_data()
{
  QTest::addColumn<int>("value");

  QTest::newRow("negative") << -1;
  QTest::newRow("just-past-last") << 4;
  QTest::newRow("far-out-of-range") << 100;
}

/**
 * @brief An integer with no matching FrameDetection enumerator falls back to unknown, not a crash.
 */
void TstEnumLabels::frameDetectionOutOfRange()
{
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::frameDetectionSlug(value), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::frameDetectionLabel(value), QStringLiteral("Unknown"));
}

//--------------------------------------------------------------------------------------------------
// DecoderMethod
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::decoderMethodFullSweep_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");
  QTest::addColumn<QString>("label");

  QTest::newRow("PlainText") << static_cast<int>(SerialStudio::PlainText)
                             << QStringLiteral("plain-text")
                             << QStringLiteral("Plain text (UTF-8)");
  QTest::newRow("Hexadecimal") << static_cast<int>(SerialStudio::Hexadecimal)
                               << QStringLiteral("hex")
                               << QStringLiteral("Hexadecimal-encoded ASCII");
  QTest::newRow("Base64") << static_cast<int>(SerialStudio::Base64) << QStringLiteral("base64")
                          << QStringLiteral("Base64-encoded ASCII");
  QTest::newRow("Binary") << static_cast<int>(SerialStudio::Binary) << QStringLiteral("binary")
                          << QStringLiteral("Raw binary bytes");
}

/**
 * @brief Every DecoderMethod enumerator resolves to its exact slug and label.
 */
void TstEnumLabels::decoderMethodFullSweep()
{
  QFETCH(int, value);
  QFETCH(QString, slug);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::decoderMethodSlug(value), slug);
  QCOMPARE(API::EnumLabels::decoderMethodLabel(value), label);
}

void TstEnumLabels::decoderMethodOutOfRange_data()
{
  QTest::addColumn<int>("value");

  QTest::newRow("negative") << -1;
  QTest::newRow("just-past-last") << 4;
  QTest::newRow("far-out-of-range") << 100;
}

/**
 * @brief An integer with no matching DecoderMethod enumerator falls back to unknown, not a crash.
 */
void TstEnumLabels::decoderMethodOutOfRange()
{
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::decoderMethodSlug(value), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::decoderMethodLabel(value), QStringLiteral("Unknown"));
}

//--------------------------------------------------------------------------------------------------
// OperationMode
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::operationModeFullSweep_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");
  QTest::addColumn<QString>("label");

  QTest::newRow("ProjectFile") << static_cast<int>(SerialStudio::ProjectFile)
                               << QStringLiteral("project-file")
                               << QStringLiteral("Project File (full dashboard with parser)");
  QTest::newRow("ConsoleOnly") << static_cast<int>(SerialStudio::ConsoleOnly)
                               << QStringLiteral("console-only")
                               << QStringLiteral("Console Only (raw terminal, no dashboard)");
  QTest::newRow("QuickPlot") << static_cast<int>(SerialStudio::QuickPlot)
                             << QStringLiteral("quick-plot")
                             << QStringLiteral("Quick Plot (auto CSV plotting)");
}

/**
 * @brief Every OperationMode enumerator resolves to its exact slug and label.
 */
void TstEnumLabels::operationModeFullSweep()
{
  QFETCH(int, value);
  QFETCH(QString, slug);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::operationModeSlug(value), slug);
  QCOMPARE(API::EnumLabels::operationModeLabel(value), label);
}

void TstEnumLabels::operationModeOutOfRange_data()
{
  QTest::addColumn<int>("value");

  QTest::newRow("negative") << -1;
  QTest::newRow("just-past-last") << 4;
  QTest::newRow("far-out-of-range") << 100;
}

/**
 * @brief An integer with no matching OperationMode enumerator falls back to unknown, not a crash.
 */
void TstEnumLabels::operationModeOutOfRange()
{
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::operationModeSlug(value), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::operationModeLabel(value), QStringLiteral("Unknown"));
}

//--------------------------------------------------------------------------------------------------
// ScriptLanguage
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::scriptLanguageFullSweep_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");
  QTest::addColumn<QString>("label");

  QTest::newRow("inherit") << -1 << QStringLiteral("inherit")
                           << QStringLiteral("Inherit from source");
  QTest::newRow("JavaScript") << static_cast<int>(SerialStudio::JavaScript)
                              << QStringLiteral("javascript")
                              << QStringLiteral("JavaScript (QJSEngine)");
  QTest::newRow("Lua") << static_cast<int>(SerialStudio::Lua) << QStringLiteral("lua")
                       << QStringLiteral("Lua 5.4");
  QTest::newRow("Native") << static_cast<int>(SerialStudio::Native) << QStringLiteral("native")
                          << QStringLiteral("Built-In (C++ template)");
  QTest::newRow("Expression") << static_cast<int>(SerialStudio::Expression)
                              << QStringLiteral("expression")
                              << QStringLiteral("Expression (compiled formula)");
}

/**
 * @brief Every ScriptLanguage enumerator resolves to its exact slug and label, including the -1
 *        sentinel that means "inherit from source".
 */
void TstEnumLabels::scriptLanguageFullSweep()
{
  QFETCH(int, value);
  QFETCH(QString, slug);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::scriptLanguageSlug(value), slug);
  QCOMPARE(API::EnumLabels::scriptLanguageLabel(value), label);
}

void TstEnumLabels::scriptLanguageOutOfRange_data()
{
  QTest::addColumn<int>("value");

  QTest::newRow("negative-not-inherit") << -2;
  QTest::newRow("just-past-last") << 4;
  QTest::newRow("far-out-of-range") << 100;
}

/**
 * @brief An integer with no matching ScriptLanguage enumerator (and not the -1 inherit sentinel)
 *        falls back to unknown, not a crash.
 */
void TstEnumLabels::scriptLanguageOutOfRange()
{
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::scriptLanguageSlug(value), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::scriptLanguageLabel(value), QStringLiteral("Unknown"));
}

//--------------------------------------------------------------------------------------------------
// GroupWidget
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::groupWidgetFullSweep_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");
  QTest::addColumn<QString>("label");

  QTest::newRow("DataGrid") << static_cast<int>(SerialStudio::DataGrid)
                            << QStringLiteral("data-grid") << QStringLiteral("Data grid (table)");
  QTest::newRow("Accelerometer") << static_cast<int>(SerialStudio::Accelerometer)
                                 << QStringLiteral("accelerometer")
                                 << QStringLiteral("Accelerometer (3-axis bar)");
  QTest::newRow("Gyroscope") << static_cast<int>(SerialStudio::Gyroscope)
                             << QStringLiteral("gyroscope")
                             << QStringLiteral("Gyroscope (3-axis dial)");
  QTest::newRow("GPS") << static_cast<int>(SerialStudio::GPS) << QStringLiteral("gps")
                       << QStringLiteral("GPS map");
  QTest::newRow("MultiPlot") << static_cast<int>(SerialStudio::MultiPlot)
                             << QStringLiteral("multi-plot")
                             << QStringLiteral("Multi-plot (overlaid line chart)");
  QTest::newRow("NoGroupWidget") << static_cast<int>(SerialStudio::NoGroupWidget)
                                 << QStringLiteral("none") << QStringLiteral("No group widget");
  QTest::newRow("Plot3D") << static_cast<int>(SerialStudio::Plot3D) << QStringLiteral("plot-3d")
                          << QStringLiteral("3D plot");
  QTest::newRow("ImageView") << static_cast<int>(SerialStudio::ImageView)
                             << QStringLiteral("image-view") << QStringLiteral("Image view");
  QTest::newRow("Painter") << static_cast<int>(SerialStudio::Painter) << QStringLiteral("painter")
                           << QStringLiteral("Painter (custom canvas)");
  QTest::newRow("WebView") << static_cast<int>(SerialStudio::WebView) << QStringLiteral("web-view")
                           << QStringLiteral("Web view (embedded browser)");
  QTest::newRow("BarPanel") << static_cast<int>(SerialStudio::BarPanel)
                            << QStringLiteral("bar-panel")
                            << QStringLiteral("Bar panel (multi-channel bars)");
}

/**
 * @brief Every GroupWidget enumerator resolves to its exact slug and label.
 */
void TstEnumLabels::groupWidgetFullSweep()
{
  QFETCH(int, value);
  QFETCH(QString, slug);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::groupWidgetSlug(value), slug);
  QCOMPARE(API::EnumLabels::groupWidgetLabel(value), label);
}

/**
 * @brief GroupWidget slugs are well-formed and distinct from one another across the whole enum.
 */
void TstEnumLabels::groupWidgetSlugsAreWellFormedAndUnique()
{
  QSet<QString> slugs;
  for (auto value : {SerialStudio::DataGrid,
                     SerialStudio::Accelerometer,
                     SerialStudio::Gyroscope,
                     SerialStudio::GPS,
                     SerialStudio::MultiPlot,
                     SerialStudio::NoGroupWidget,
                     SerialStudio::Plot3D,
                     SerialStudio::ImageView,
                     SerialStudio::Painter,
                     SerialStudio::WebView,
                     SerialStudio::BarPanel}) {
    const auto slug = API::EnumLabels::groupWidgetSlug(static_cast<int>(value));
    QVERIFY(isWellFormedSlug(slug));
    slugs.insert(slug);
  }

  QCOMPARE(slugs.size(), qsizetype(11));
}

void TstEnumLabels::groupWidgetOutOfRange_data()
{
  QTest::addColumn<int>("value");

  QTest::newRow("negative") << -1;
  QTest::newRow("just-past-last") << 11;
  QTest::newRow("far-out-of-range") << 100;
}

/**
 * @brief An integer with no matching GroupWidget enumerator falls back to unknown, not a crash.
 */
void TstEnumLabels::groupWidgetOutOfRange()
{
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::groupWidgetSlug(value), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::groupWidgetLabel(value), QStringLiteral("Unknown"));
}

//--------------------------------------------------------------------------------------------------
// DatasetWidget
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::datasetWidgetFullSweep_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");
  QTest::addColumn<QString>("label");

  QTest::newRow("Bar") << static_cast<int>(SerialStudio::Bar) << QStringLiteral("bar")
                       << QStringLiteral("Bar (level meter)");
  QTest::newRow("Gauge") << static_cast<int>(SerialStudio::Gauge) << QStringLiteral("gauge")
                         << QStringLiteral("Gauge (analog dial)");
  QTest::newRow("Compass") << static_cast<int>(SerialStudio::Compass) << QStringLiteral("compass")
                           << QStringLiteral("Compass");
  QTest::newRow("Meter") << static_cast<int>(SerialStudio::Meter) << QStringLiteral("meter")
                         << QStringLiteral("Meter (analog half-arc)");
  QTest::newRow("NoDatasetWidget") << static_cast<int>(SerialStudio::NoDatasetWidget)
                                   << QStringLiteral("none") << QStringLiteral("No dataset widget");
}

/**
 * @brief Every DatasetWidget enumerator resolves to its exact slug and label.
 */
void TstEnumLabels::datasetWidgetFullSweep()
{
  QFETCH(int, value);
  QFETCH(QString, slug);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::datasetWidgetSlug(value), slug);
  QCOMPARE(API::EnumLabels::datasetWidgetLabel(value), label);
}

/**
 * @brief DatasetWidget slugs are well-formed and distinct from one another across the whole enum.
 */
void TstEnumLabels::datasetWidgetSlugsAreWellFormedAndUnique()
{
  QSet<QString> slugs;
  for (auto value : {SerialStudio::Bar,
                     SerialStudio::Gauge,
                     SerialStudio::Compass,
                     SerialStudio::Meter,
                     SerialStudio::NoDatasetWidget}) {
    const auto slug = API::EnumLabels::datasetWidgetSlug(static_cast<int>(value));
    QVERIFY(isWellFormedSlug(slug));
    slugs.insert(slug);
  }

  QCOMPARE(slugs.size(), qsizetype(5));
}

void TstEnumLabels::datasetWidgetOutOfRange_data()
{
  QTest::addColumn<int>("value");

  QTest::newRow("negative") << -1;
  QTest::newRow("just-past-last") << 5;
  QTest::newRow("far-out-of-range") << 100;
}

/**
 * @brief An integer with no matching DatasetWidget enumerator falls back to unknown, not a crash.
 */
void TstEnumLabels::datasetWidgetOutOfRange()
{
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::datasetWidgetSlug(value), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::datasetWidgetLabel(value), QStringLiteral("Unknown"));
}

//--------------------------------------------------------------------------------------------------
// DashboardWidget slug (bidirectional)
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::dashboardWidgetGplSweepRoundTrips_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("slug");

  QTest::newRow("Terminal") << static_cast<int>(SerialStudio::DashboardTerminal)
                            << QStringLiteral("terminal");
  QTest::newRow("DataGrid") << static_cast<int>(SerialStudio::DashboardDataGrid)
                            << QStringLiteral("datagrid");
  QTest::newRow("MultiPlot") << static_cast<int>(SerialStudio::DashboardMultiPlot)
                             << QStringLiteral("multiplot");
  QTest::newRow("Accelerometer") << static_cast<int>(SerialStudio::DashboardAccelerometer)
                                 << QStringLiteral("accelerometer");
  QTest::newRow("Gyroscope") << static_cast<int>(SerialStudio::DashboardGyroscope)
                             << QStringLiteral("gyroscope");
  QTest::newRow("GPS") << static_cast<int>(SerialStudio::DashboardGPS) << QStringLiteral("gps");
  QTest::newRow("Plot3D") << static_cast<int>(SerialStudio::DashboardPlot3D)
                          << QStringLiteral("plot3d");
  QTest::newRow("FFT") << static_cast<int>(SerialStudio::DashboardFFT) << QStringLiteral("fft");
  QTest::newRow("LED") << static_cast<int>(SerialStudio::DashboardLED) << QStringLiteral("led");
  QTest::newRow("Plot") << static_cast<int>(SerialStudio::DashboardPlot) << QStringLiteral("plot");
  QTest::newRow("Bar") << static_cast<int>(SerialStudio::DashboardBar) << QStringLiteral("bar");
  QTest::newRow("Gauge") << static_cast<int>(SerialStudio::DashboardGauge)
                         << QStringLiteral("gauge");
  QTest::newRow("Compass") << static_cast<int>(SerialStudio::DashboardCompass)
                           << QStringLiteral("compass");
  QTest::newRow("Meter") << static_cast<int>(SerialStudio::DashboardMeter)
                         << QStringLiteral("meter");
  QTest::newRow("Clock") << static_cast<int>(SerialStudio::DashboardClock)
                         << QStringLiteral("clock");
  QTest::newRow("Stopwatch") << static_cast<int>(SerialStudio::DashboardStopwatch)
                             << QStringLiteral("stopwatch");
  QTest::newRow("WebView") << static_cast<int>(SerialStudio::DashboardWebView)
                           << QStringLiteral("webview");
  QTest::newRow("NoWidget") << static_cast<int>(SerialStudio::DashboardNoWidget)
                            << QStringLiteral("none");
  QTest::newRow("BarPanel") << static_cast<int>(SerialStudio::DashboardBarPanel)
                            << QStringLiteral("barpanel");
  QTest::newRow("Extension") << static_cast<int>(SerialStudio::DashboardExtension)
                             << QStringLiteral("extension");
}

/**
 * @brief Every GPL-reachable DashboardWidget enumerator produces its exact slug, and feeding that
 *        slug back through dashboardWidgetFromSlug() recovers the original ordinal.
 */
void TstEnumLabels::dashboardWidgetGplSweepRoundTrips()
{
  QFETCH(int, value);
  QFETCH(QString, slug);

  const auto produced = API::EnumLabels::dashboardWidgetSlug(value);
  QCOMPARE(produced, slug);
  QCOMPARE(API::EnumLabels::dashboardWidgetFromSlug(produced), value);
}

void TstEnumLabels::dashboardWidgetFromSlugAcceptsAliasesCaseAndWhitespace_data()
{
  QTest::addColumn<QString>("slug");
  QTest::addColumn<int>("value");

  QTest::newRow("data-grid-alias")
    << QStringLiteral("data-grid") << static_cast<int>(SerialStudio::DashboardDataGrid);
  QTest::newRow("multi-plot-alias")
    << QStringLiteral("multi-plot") << static_cast<int>(SerialStudio::DashboardMultiPlot);
  QTest::newRow("plot-3d-alias") << QStringLiteral("plot-3d")
                                 << static_cast<int>(SerialStudio::DashboardPlot3D);
  QTest::newRow("web-view-alias") << QStringLiteral("web-view")
                                  << static_cast<int>(SerialStudio::DashboardWebView);
  QTest::newRow("bar-panel-alias")
    << QStringLiteral("bar-panel") << static_cast<int>(SerialStudio::DashboardBarPanel);
  QTest::newRow("mixed-case") << QStringLiteral("DataGrid")
                              << static_cast<int>(SerialStudio::DashboardDataGrid);
  QTest::newRow("padded-whitespace")
    << QStringLiteral("  terminal  ") << static_cast<int>(SerialStudio::DashboardTerminal);
}

/**
 * @brief dashboardWidgetFromSlug() accepts hyphenated aliases, is case-insensitive, and trims
 *        surrounding whitespace before matching.
 */
void TstEnumLabels::dashboardWidgetFromSlugAcceptsAliasesCaseAndWhitespace()
{
  QFETCH(QString, slug);
  QFETCH(int, value);

  QCOMPARE(API::EnumLabels::dashboardWidgetFromSlug(slug), value);
}

/**
 * @brief A slug with no matching DashboardWidget entry (including the empty string) misses to -1.
 */
void TstEnumLabels::dashboardWidgetFromSlugBogusOrEmptyMissesToMinusOne()
{
  QCOMPARE(API::EnumLabels::dashboardWidgetFromSlug(QStringLiteral("totally-bogus")), -1);
  QCOMPARE(API::EnumLabels::dashboardWidgetFromSlug(QString()), -1);
}

/**
 * @brief Commercial DashboardWidget ordinals do not exist in a GPL build: their integer positions
 *        fall through to unknown, and their slugs are unrecognized by dashboardWidgetFromSlug().
 */
void TstEnumLabels::dashboardWidgetCommercialGapsFallBackToUnknownInGplBuild()
{
  QCOMPARE(API::EnumLabels::dashboardWidgetSlug(18), QStringLiteral("unknown"));
  QCOMPARE(API::EnumLabels::dashboardWidgetFromSlug(QStringLiteral("imageview")), -1);
  QCOMPARE(API::EnumLabels::dashboardWidgetFromSlug(QStringLiteral("waterfall")), -1);
  QCOMPARE(API::EnumLabels::dashboardWidgetFromSlug(QStringLiteral("painter")), -1);
}

//--------------------------------------------------------------------------------------------------
// DatasetOption slug (bidirectional)
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::datasetOptionSingleBitSweep_data()
{
  QTest::addColumn<int>("bit");
  QTest::addColumn<QString>("slug");

  QTest::newRow("Plot") << static_cast<int>(SerialStudio::DatasetPlot) << QStringLiteral("plot");
  QTest::newRow("FFT") << static_cast<int>(SerialStudio::DatasetFFT) << QStringLiteral("fft");
  QTest::newRow("Bar") << static_cast<int>(SerialStudio::DatasetBar) << QStringLiteral("bar");
  QTest::newRow("Gauge") << static_cast<int>(SerialStudio::DatasetGauge) << QStringLiteral("gauge");
  QTest::newRow("Compass") << static_cast<int>(SerialStudio::DatasetCompass)
                           << QStringLiteral("compass");
  QTest::newRow("LED") << static_cast<int>(SerialStudio::DatasetLED) << QStringLiteral("led");
  QTest::newRow("Waterfall") << static_cast<int>(SerialStudio::DatasetWaterfall)
                             << QStringLiteral("waterfall");
  QTest::newRow("Meter") << static_cast<int>(SerialStudio::DatasetMeter) << QStringLiteral("meter");
}

/**
 * @brief Every single-bit DatasetOption value resolves to its exact slug, and that slug resolves
 *        back through datasetOptionFromSlug() to the same bit.
 */
void TstEnumLabels::datasetOptionSingleBitSweep()
{
  QFETCH(int, bit);
  QFETCH(QString, slug);

  QCOMPARE(API::EnumLabels::datasetOptionSlug(bit), slug);
  QCOMPARE(API::EnumLabels::datasetOptionFromSlug(slug), bit);
}

/**
 * @brief Round-tripping every single DatasetOption bit through bitsToSlugs()/slugsToBits() and
 *        slug()/fromSlug() individually recovers the original bit in both directions.
 */
void TstEnumLabels::datasetOptionSingleBitRoundTrip()
{
  for (auto bit : {SerialStudio::DatasetPlot,
                   SerialStudio::DatasetFFT,
                   SerialStudio::DatasetBar,
                   SerialStudio::DatasetGauge,
                   SerialStudio::DatasetCompass,
                   SerialStudio::DatasetLED,
                   SerialStudio::DatasetWaterfall,
                   SerialStudio::DatasetMeter}) {
    const auto value = static_cast<int>(bit);
    const auto slugs = API::EnumLabels::datasetOptionsBitsToSlugs(value);

    QCOMPARE(slugs.size(), 1);
    QCOMPARE(API::EnumLabels::datasetOptionsSlugsToBits(slugs), value);
  }
}

/**
 * @brief DatasetGeneric (no bit set) and an unrecognized slug both miss: the former has no switch
 *        case in datasetOptionSlug(), the latter has no match in datasetOptionFromSlug().
 */
void TstEnumLabels::datasetOptionFromSlugMissReturnsZero()
{
  const auto genericSlug =
    API::EnumLabels::datasetOptionSlug(static_cast<int>(SerialStudio::DatasetGeneric));
  QVERIFY(genericSlug.isEmpty());
  QCOMPARE(API::EnumLabels::datasetOptionFromSlug(QStringLiteral("not-a-real-option")), 0);
  QCOMPARE(API::EnumLabels::datasetOptionFromSlug(QString()), 0);
}

/**
 * @brief datasetOptionsBitsToSlugs() emits every set bit as its slug, in canonical bit order.
 */
void TstEnumLabels::datasetOptionsBitsToSlugsCombinedMask()
{
  const auto mask = static_cast<int>(SerialStudio::DatasetPlot)
                  | static_cast<int>(SerialStudio::DatasetFFT)
                  | static_cast<int>(SerialStudio::DatasetMeter);

  const QStringList expected = {
    QStringLiteral("plot"), QStringLiteral("fft"), QStringLiteral("meter")};

  QCOMPARE(API::EnumLabels::datasetOptionsBitsToSlugs(mask), expected);
  QVERIFY(API::EnumLabels::datasetOptionsBitsToSlugs(0).isEmpty());
}

/**
 * @brief datasetOptionsSlugsToBits() is the exact inverse of datasetOptionsBitsToSlugs(): an empty
 *        list combines to zero, and the full eight-bit mask survives a bits -> slugs -> bits trip.
 */
void TstEnumLabels::datasetOptionsSlugsToBitsIsTheInverse()
{
  const QStringList slugs = {
    QStringLiteral("plot"), QStringLiteral("fft"), QStringLiteral("meter")};
  const auto expected = static_cast<int>(SerialStudio::DatasetPlot)
                      | static_cast<int>(SerialStudio::DatasetFFT)
                      | static_cast<int>(SerialStudio::DatasetMeter);

  QCOMPARE(API::EnumLabels::datasetOptionsSlugsToBits(slugs), expected);
  QCOMPARE(API::EnumLabels::datasetOptionsSlugsToBits(QStringList()), 0);

  const auto fullMask =
    static_cast<int>(SerialStudio::DatasetPlot) | static_cast<int>(SerialStudio::DatasetFFT)
    | static_cast<int>(SerialStudio::DatasetBar) | static_cast<int>(SerialStudio::DatasetGauge)
    | static_cast<int>(SerialStudio::DatasetCompass) | static_cast<int>(SerialStudio::DatasetLED)
    | static_cast<int>(SerialStudio::DatasetWaterfall)
    | static_cast<int>(SerialStudio::DatasetMeter);

  const auto fullMaskSlugs = API::EnumLabels::datasetOptionsBitsToSlugs(fullMask);
  const auto roundTripped  = API::EnumLabels::datasetOptionsSlugsToBits(fullMaskSlugs);
  QCOMPARE(roundTripped, fullMask);
}

//--------------------------------------------------------------------------------------------------
// DatasetOption label (comma-separated)
//--------------------------------------------------------------------------------------------------

void TstEnumLabels::datasetOptionsLabelHumanReadable_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<QString>("label");

  QTest::newRow("generic") << static_cast<int>(SerialStudio::DatasetGeneric)
                           << QStringLiteral("generic");

  const auto combined = static_cast<int>(SerialStudio::DatasetPlot)
                      | static_cast<int>(SerialStudio::DatasetFFT)
                      | static_cast<int>(SerialStudio::DatasetMeter);
  QTest::newRow("plot-fft-meter") << combined << QStringLiteral("plot, FFT, meter");

  const auto fullMask =
    static_cast<int>(SerialStudio::DatasetPlot) | static_cast<int>(SerialStudio::DatasetFFT)
    | static_cast<int>(SerialStudio::DatasetBar) | static_cast<int>(SerialStudio::DatasetGauge)
    | static_cast<int>(SerialStudio::DatasetCompass) | static_cast<int>(SerialStudio::DatasetLED)
    | static_cast<int>(SerialStudio::DatasetWaterfall)
    | static_cast<int>(SerialStudio::DatasetMeter);
  QTest::newRow("all-bits") << fullMask
                            << QStringLiteral(
                                 "plot, FFT, bar, gauge, compass, LED, waterfall, meter");
}

/**
 * @brief datasetOptionsLabel() joins every set flag's human-friendly name with ", ", and falls
 *        back to "generic" when no flag is set.
 */
void TstEnumLabels::datasetOptionsLabelHumanReadable()
{
  QFETCH(int, value);
  QFETCH(QString, label);

  QCOMPARE(API::EnumLabels::datasetOptionsLabel(value), label);
}

QTEST_APPLESS_MAIN(TstEnumLabels)

#include "tst_enum_labels.moc"
