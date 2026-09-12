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

#include <QString>
#include <QtGlobal>

#include "Benchmark/HotpathBenchmark.h"
#include "Core/SerialStudio.h"

namespace Benchmark {

/**
 * @brief Named rows: gated parser tiers, floor-gated lua exporter/dashboard rows, ungated
 *        dashboard(off).
 */
enum ReportIndex {
  kReportData,
  kReportNative,
  kReportNativeMix,
  kReportLua,
  kReportJs,
  kReportLuaMix,
  kReportJsMix,
  kReportLuaX,
  kReportLuaD,
  kReportLuaDoff,
  kReportCount
};

static constexpr const char* kReportTags[kReportCount] = {"data-pipeline",
                                                          "native(numeric)",
                                                          "native(mixed)",
                                                          "lua(numeric)",
                                                          "js(numeric)",
                                                          "lua(mixed)",
                                                          "js(mixed)",
                                                          "lua+exporters",
                                                          "lua+dashboard",
                                                          "lua+dashboard(off)"};

/**
 * @brief One ungated coverage row: an engine x {numeric,mixed} x {exporters,dashboard}
 *        combination.
 */
struct CoverageRow {
  int language;
  bool strings;
  bool exporters;
  bool dashboard;
  const char* tag;
};

// The rest of the matrix the named rows do not cover; ungated, run for code-path coverage only.
static constexpr CoverageRow kCoverageMatrix[] = {
  {    SerialStudio::Native, false,  true, false, "native+exporters(numeric)"},
  {       SerialStudio::Lua, false,  true, false,    "lua+exporters(numeric)"},
  {SerialStudio::JavaScript, false,  true, false,     "js+exporters(numeric)"},
  {    SerialStudio::Native,  true,  true, false,   "native+exporters(mixed)"},
  {SerialStudio::JavaScript,  true,  true, false,       "js+exporters(mixed)"},
  {    SerialStudio::Native, false, false,  true, "native+dashboard(numeric)"},
  {SerialStudio::JavaScript, false, false,  true,     "js+dashboard(numeric)"},
  {    SerialStudio::Native,  true, false,  true,   "native+dashboard(mixed)"},
  {       SerialStudio::Lua,  true, false,  true,      "lua+dashboard(mixed)"},
  {SerialStudio::JavaScript,  true, false,  true,       "js+dashboard(mixed)"},
};

static constexpr int kCoverageCount =
  static_cast<int>(sizeof(kCoverageMatrix) / sizeof(CoverageRow));

static constexpr int kReportColumns = 8;

/**
 * @brief Renders the hotpath benchmark report: the per-run table, the stage and ratio readouts,
 *        the spec-0084 gates and the machine-readable summary keys.
 */
class HotpathReport {
public:
  [[nodiscard]] static bool print(const HotpathBenchmark::Result* results,
                                  const HotpathBenchmark::Result* coverage,
                                  const HotpathBenchmark::StageBreakdown& stages,
                                  const QString& outputFile,
                                  int channels);
};

}  // namespace Benchmark
