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

#include "Benchmark/HotpathReport.h"

#include <cmath>
#include <cstdio>
#include <QChar>
#include <QFile>
#include <QIODevice>
#include <QLocale>
#include <QString>

#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "Platform/AppPlatform.h"

namespace Benchmark {

/**
 * @brief Formats a count with thousands separators (fixed English grouping for stable CI logs).
 */
[[nodiscard]] static QString groupedCount(double value)
{
  static const QLocale s_locale(QLocale::English);
  return s_locale.toString(static_cast<qulonglong>(value > 0.0 ? std::llround(value) : 0));
}

/**
 * @brief Fills one report row: ungated runs (informational tiers) show n/a target and result.
 */
static void fillReportRow(const HotpathBenchmark::Result& r, const char* tag, QString* cells)
{
  SS_ASSERT(tag != nullptr, return);
  SS_ASSERT(cells != nullptr, return);

  const bool gated = r.minFps > 1.0;
  cells[0]         = QString::fromLatin1(tag);
  cells[1]         = groupedCount(static_cast<double>(r.framesParsed));
  cells[2]         = groupedCount(static_cast<double>(r.framesSkipped));
  cells[3]         = QString::number(r.elapsedSeconds, 'f', 2);
  cells[4]         = groupedCount(r.framesPerSecond);
  cells[5]         = r.allocationsPerFrame < 0.0 ? QStringLiteral("n/a")
                                                 : QString::number(r.allocationsPerFrame, 'f', 4);
  cells[6]         = gated ? groupedCount(r.minFps) : QStringLiteral("n/a");
  if (gated)
    cells[7] = r.passed ? QStringLiteral("PASS") : QStringLiteral("FAIL");
  else
    cells[7] = QStringLiteral("n/a");
}

/**
 * @brief Machine-readable key stem for a report tag: upper-cased, each punctuation run folded to
 *        one '_', none trailing ("native(numeric)" -> "NATIVE_NUMERIC").
 */
[[nodiscard]] static QString summaryKeyStem(const char* tag)
{
  SS_ASSERT(tag != nullptr, return {});

  const QString upper = QString::fromLatin1(tag).toUpper();
  QString stem;
  stem.reserve(upper.size());
  for (const QChar c : upper)
    if (c.isLetterOrNumber())
      stem.append(c);
    else if (!stem.isEmpty() && !stem.endsWith(QLatin1Char('_')))
      stem.append(QLatin1Char('_'));

  while (stem.endsWith(QLatin1Char('_')))
    stem.chop(1);

  SS_ASSERT_LOG(!stem.isEmpty());
  return stem;
}

/**
 * @brief Allocation figure as the summary line prints it: the count, or n/a for a build that
 *        cannot count.
 */
[[nodiscard]] static QString allocationsText(const HotpathBenchmark::Result& r)
{
  if (r.allocTainted)
    return QStringLiteral("tainted");

  return r.allocationsPerFrame < 0.0 ? QStringLiteral("n/a")
                                     : QString::number(r.allocationsPerFrame, 'f', 4);
}

/**
 * @brief Prints the exporter / dashboard / dashboard-ingest cost ratios off the Lua reference rows
 *        and returns the ingest ratio, which the summary line repeats.
 */
template<typename PrintFn>
[[nodiscard]] static double printRatioLines(const HotpathBenchmark::Result* results,
                                            const PrintFn& printData)
{
  SS_ASSERT(results != nullptr, return 0.0);

  const HotpathBenchmark::Result& lua     = results[kReportLua];
  const HotpathBenchmark::Result& luaMix  = results[kReportLuaMix];
  const HotpathBenchmark::Result& luaX    = results[kReportLuaX];
  const HotpathBenchmark::Result& luaD    = results[kReportLuaD];
  const HotpathBenchmark::Result& luaDoff = results[kReportLuaDoff];

  const double slowdown =
    luaX.framesPerSecond > 0.0 ? luaMix.framesPerSecond / luaX.framesPerSecond : 0.0;
  printData("hotpath: exporters cost %.2fx throughput\n", slowdown);

  const double dashSlowdown =
    luaD.framesPerSecond > 0.0 ? lua.framesPerSecond / luaD.framesPerSecond : 0.0;
  printData("hotpath: dashboard costs %.2fx throughput\n", dashSlowdown);

  const double dashIngest =
    luaD.framesPerSecond > 0.0 ? luaDoff.framesPerSecond / luaD.framesPerSecond : 0.0;
  printData("hotpath: dashboard ingest costs %.2fx throughput (same project, ingest on vs off)\n",
            dashIngest);

  SS_ASSERT_LOG(dashIngest >= 0.0);
  return dashIngest;
}

/**
 * @brief Prints the build line, the run width and which lane the Native rows took (the span
 *        lane accepts at most kMaxSpanFields fields per frame; wider frames parse through the
 *        list path).
 */
template<typename PrintFn>
static void printReportHeader(int channels, const PrintFn& printData)
{
  SS_ASSERT(channels > 0, return);

  constexpr qsizetype kSpanCap = DataModel::FrameBuilder::kMaxSpanFields;
  printData("build: %s\n", HotpathBenchmark::buildProvenance().toUtf8().constData());
  printData("channels: %d\n", channels);
  if (channels <= kSpanCap)
    printData("native lane: span\n");
  else
    printData("native lane: list (> %lld fields)\n", static_cast<long long>(kSpanCap));

  SS_ASSERT_LOG(kSpanCap > 0);
}

// The three verdicts printReport folds into HOTPATH_PASS
struct GateVerdict {
  bool throughputOk;
  bool allocCounted;
  bool allocGateOk;
  bool rowsParsed;
};

/**
 * @brief Evaluates the throughput tiers (gated at the default width only), the Native allocation
 *        gate (spec 0084) and the aborted-run guard, printing one line for each that fails.
 */
template<typename PrintFn>
[[nodiscard]] static GateVerdict evaluateGates(const HotpathBenchmark::Result* results,
                                               int channels,
                                               const PrintFn& printData)
{
  static constexpr GateVerdict kFailed{false, false, false, false};
  SS_ASSERT(results != nullptr, return kFailed);
  SS_ASSERT(channels > 0, return kFailed);

  const HotpathBenchmark::Result& native    = results[kReportNative];
  const HotpathBenchmark::Result& nativeMix = results[kReportNativeMix];
  const bool defaultWidth                   = channels == HotpathBenchmark::kDefaultChannels;

  GateVerdict verdict{true, false, true, true};
  if (defaultWidth) {
    for (int i = 0; i < kReportCount && verdict.throughputOk; ++i)
      verdict.throughputOk = results[i].minFps <= 1.0 || results[i].passed;
  } else {
    printData("hotpath: width %d, throughput tiers not gated (calibrated at %d channels)\n",
              channels,
              HotpathBenchmark::kDefaultChannels);
  }

  verdict.allocCounted = native.allocationsPerFrame >= 0.0 && nativeMix.allocationsPerFrame >= 0.0;
  verdict.allocGateOk  = !verdict.allocCounted
                      || (native.allocationsPerFrame == 0.0 && nativeMix.allocationsPerFrame == 0.0);
  if (!verdict.allocGateOk)
    printData("hotpath: allocation gate FAILED: native(numeric) %s/frame, native(mixed) %s/frame "
              "(the Native lanes must not allocate per frame)\n",
              allocationsText(native).toUtf8().constData(),
              allocationsText(nativeMix).toUtf8().constData());

  if (native.allocTainted || nativeMix.allocTainted)
    printData("hotpath: allocation count tainted (a thread ended inside a Native window); "
              "re-run to gate\n");

  for (int i = 0; i < kReportCount && verdict.rowsParsed; ++i)
    verdict.rowsParsed = results[i].minFps <= 1.0 || results[i].framesParsed > 0;

  if (!verdict.rowsParsed)
    printData("hotpath: a gated row parsed no frames (aborted run); the report cannot pass\n");

  return verdict;
}

/**
 * @brief Prints the spec-0084 summary keys: the run width, the Native allocation gate verdict, and
 *        one allocations-per-frame value per named row.
 */
template<typename PrintFn>
static void printAllocationSummary(const HotpathBenchmark::Result* results,
                                   int channels,
                                   bool allocCounted,
                                   bool allocGateOk,
                                   const PrintFn& printData)
{
  SS_ASSERT(results != nullptr, return);
  SS_ASSERT(channels > 0, return);

  const char* allocGate = allocCounted ? (allocGateOk ? "pass" : "fail") : "n/a";
  printData("HOTPATH_CHANNELS=%d HOTPATH_ALLOC_GATE=%s\n", channels, allocGate);
  for (int i = 0; i < kReportCount; ++i)
    printData("HOTPATH_%s_ALLOC_PER_FRAME=%s\n",
              summaryKeyStem(kReportTags[i]).toUtf8().constData(),
              allocationsText(results[i]).toUtf8().constData());
}

/**
 * @brief Prints the per-run throughput table through the shared stdout + file sink. The named
 *        rows (gated tiers + lua reference rows) print first, then the ungated coverage matrix.
 */
template<typename PrintFn>
static void printRunTable(const HotpathBenchmark::Result* results,
                          const HotpathBenchmark::Result* coverage,
                          const PrintFn& printData)
{
  SS_ASSERT(results != nullptr, return);
  SS_ASSERT(coverage != nullptr, return);

  static const QString s_headers[kReportColumns] = {QStringLiteral("Benchmark"),
                                                    QStringLiteral("Parsed"),
                                                    QStringLiteral("Skipped"),
                                                    QStringLiteral("Time (s)"),
                                                    QStringLiteral("Frames/s"),
                                                    QStringLiteral("Alloc/frame"),
                                                    QStringLiteral("Target"),
                                                    QStringLiteral("Result")};

  constexpr int kRowCount = kReportCount + kCoverageCount;
  QString cells[kRowCount][kReportColumns];
  for (int i = 0; i < kReportCount; ++i)
    fillReportRow(results[i], kReportTags[i], cells[i]);

  for (int i = 0; i < kCoverageCount; ++i)
    fillReportRow(coverage[i], kCoverageMatrix[i].tag, cells[kReportCount + i]);

  qsizetype widths[kReportColumns];
  for (int col = 0; col < kReportColumns; ++col) {
    widths[col] = s_headers[col].size();
    for (int row = 0; row < kRowCount; ++row)
      widths[col] = qMax(widths[col], cells[row][col].size());
  }

  const auto formatRow = [&](const QString* row) {
    QString line = QStringLiteral("|");
    for (int col = 0; col < kReportColumns; ++col) {
      const bool left  = (col == 0 || col == kReportColumns - 1);
      line            += QStringLiteral(" ");
      line += left ? row[col].leftJustified(widths[col]) : row[col].rightJustified(widths[col]);
      line += QStringLiteral(" |");
    }
    return line;
  };

  QString separator = QStringLiteral("+");
  for (int col = 0; col < kReportColumns; ++col)
    separator += QString(widths[col] + 2, QLatin1Char('-')) + QLatin1Char('+');

  printData("%s\n", separator.toUtf8().constData());
  printData("%s\n", formatRow(s_headers).toUtf8().constData());
  printData("%s\n", separator.toUtf8().constData());
  for (int row = 0; row < kRowCount; ++row)
    printData("%s\n", formatRow(cells[row]).toUtf8().constData());

  printData("%s\n", separator.toUtf8().constData());
}

/**
 * @brief Prints the per-run, stage, ratio, and machine-readable readouts; true when every gate
 *        passed.
 */
bool HotpathReport::print(const HotpathBenchmark::Result* results,
                          const HotpathBenchmark::Result* coverage,
                          const HotpathBenchmark::StageBreakdown& stages,
                          const QString& outputFile,
                          int channels)
{
  SS_ASSERT(results != nullptr, return false);
  SS_ASSERT(coverage != nullptr, return false);
  SS_ASSERT(channels > 0, return false);

  QFile file(outputFile);
  const bool fileOpen  = !outputFile.isEmpty() && file.open(QIODevice::WriteOnly | QIODevice::Text);
  const auto printData = [&](const char* fmt, auto... args) {
    const QByteArray line = QString::asprintf(fmt, args...).toUtf8();
    std::fputs(line.constData(), stdout);
    if (fileOpen)
      file.write(line);
  };

  printReportHeader(channels, printData);

  printRunTable(results, coverage, printData);

  const HotpathBenchmark::Result& data      = results[kReportData];
  const HotpathBenchmark::Result& native    = results[kReportNative];
  const HotpathBenchmark::Result& nativeMix = results[kReportNativeMix];
  const HotpathBenchmark::Result& lua       = results[kReportLua];
  const HotpathBenchmark::Result& js        = results[kReportJs];
  const HotpathBenchmark::Result& luaMix    = results[kReportLuaMix];
  const HotpathBenchmark::Result& jsMix     = results[kReportJsMix];
  const HotpathBenchmark::Result& luaX      = results[kReportLuaX];
  const HotpathBenchmark::Result& luaD      = results[kReportLuaD];
  const HotpathBenchmark::Result& luaDoff   = results[kReportLuaDoff];

  if (stages.valid) {
    printData("hotpath-stage[native]: extract %.0f ns, tokenize %.0f ns, datasets+publish %.0f ns "
              "(total %.0f ns/frame)\n",
              stages.extractNs,
              stages.tokenizeNs,
              stages.datasetsPublishNs,
              stages.totalNs);
  }

  const double dashIngest = printRatioLines(results, printData);

  const quint64 peakRssBytes = Platform::AppPlatform::peakResidentBytes();
  printData("hotpath: peak rss %.1f MiB (%s bytes)\n",
            static_cast<double>(peakRssBytes) / (1024.0 * 1024.0),
            groupedCount(static_cast<double>(peakRssBytes)).toUtf8().constData());

  const GateVerdict verdict = evaluateGates(results, channels, printData);
  const bool allPassed      = verdict.throughputOk && verdict.allocGateOk && verdict.rowsParsed;
  printData("HOTPATH_FPS=%.0f HOTPATH_TARGET=%.0f HOTPATH_JS_FPS=%.0f HOTPATH_JS_TARGET=%.0f "
            "HOTPATH_PASS=%d HOTPATH_EXPORTER_FPS=%.0f HOTPATH_DASHBOARD_FPS=%.0f "
            "HOTPATH_DATA_FPS=%.0f HOTPATH_DATA_TARGET=%.0f\n",
            lua.framesPerSecond,
            lua.minFps,
            js.framesPerSecond,
            js.minFps,
            allPassed ? 1 : 0,
            luaX.framesPerSecond,
            luaD.framesPerSecond,
            data.framesPerSecond,
            data.minFps);
  printData("HOTPATH_LUA_MIXED_FPS=%.0f HOTPATH_LUA_MIXED_TARGET=%.0f "
            "HOTPATH_JS_MIXED_FPS=%.0f HOTPATH_JS_MIXED_TARGET=%.0f\n",
            luaMix.framesPerSecond,
            luaMix.minFps,
            jsMix.framesPerSecond,
            jsMix.minFps);
  printData("HOTPATH_DASHBOARD_OFF_FPS=%.0f HOTPATH_DASHBOARD_INGEST_COST=%.2f\n",
            luaDoff.framesPerSecond,
            dashIngest);
  printData("HOTPATH_NATIVE_FPS=%.0f HOTPATH_NATIVE_TARGET=%.0f "
            "HOTPATH_NATIVE_MIXED_FPS=%.0f HOTPATH_NATIVE_MIXED_TARGET=%.0f\n",
            native.framesPerSecond,
            native.minFps,
            nativeMix.framesPerSecond,
            nativeMix.minFps);
  if (stages.valid)
    printData("HOTPATH_STAGE_EXTRACT_NS=%.0f HOTPATH_STAGE_TOKENIZE_NS=%.0f "
              "HOTPATH_STAGE_PUBLISH_NS=%.0f\n",
              stages.extractNs,
              stages.tokenizeNs,
              stages.datasetsPublishNs);

  printData("HOTPATH_PEAK_RSS_BYTES=%llu\n", static_cast<unsigned long long>(peakRssBytes));

  printAllocationSummary(results, channels, verdict.allocCounted, verdict.allocGateOk, printData);

  std::fflush(stdout);
  if (fileOpen) {
    file.flush();
    file.close();
  }

  return allPassed;
}

}  // namespace Benchmark
