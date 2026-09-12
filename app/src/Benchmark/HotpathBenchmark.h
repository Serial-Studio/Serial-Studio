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
#include <QJsonObject>
#include <QString>
#include <QtGlobal>

namespace Benchmark {

/**
 * @brief End-to-end throughput benchmark for the frame parse pipeline.
 */
class HotpathBenchmark {
public:
  /**
   * @brief Outcome of one benchmark run.
   */
  struct Result {
    bool passed;
    bool allocTainted;
    int language;
    double minFps;
    double framesPerSecond;
    double elapsedSeconds;
    double allocationsPerFrame;
    quint64 framesParsed;
    quint64 framesSkipped;
  };

  static constexpr int kDefaultChannels = 8;

  /**
   * @brief Per-stage attribution (ns/frame) for the native numeric run, measured by composition
   *        (extraction-only and tokenize-only loops) so the hotpath carries no instrumentation.
   */
  struct StageBreakdown {
    bool valid;
    double extractNs;
    double tokenizeNs;
    double datasetsPublishNs;
    double totalNs;
  };

  [[nodiscard]] static Result run(quint64 targetFrames,
                                  double minFps,
                                  double minSeconds,
                                  int language,
                                  bool withExporters,
                                  bool withStrings     = true,
                                  bool withDashboard   = false,
                                  bool dashboardIngest = true,
                                  int channels         = kDefaultChannels);
  [[nodiscard]] static Result runDataPipeline(quint64 targetFrames,
                                              double minFps,
                                              double minSeconds,
                                              int channels = kDefaultChannels);
  [[nodiscard]] static int runAndReport(quint64 targetFrames,
                                        double minFps,
                                        double minSeconds,
                                        const QString& outputFile = QString(),
                                        int channels              = kDefaultChannels);

  [[nodiscard]] static QString buildProvenance();

  [[nodiscard]] static bool active() noexcept;
  static void setActive(bool active) noexcept;

private:
  [[nodiscard]] static StageBreakdown measureNativeStages(const Result& data,
                                                          const Result& native,
                                                          int channels);
  static void enableConsumers();
  static void disableConsumers();
  static void setupProject(int language, int channels, bool withStrings, bool dashboard);
  static void activateDashboardWidgets();
  [[nodiscard]] static QByteArray buildChunk(int frames, int channels, int stringColumns = 0);
  [[nodiscard]] static QJsonObject buildProjectJson(int language, int channels, bool withStrings);
  [[nodiscard]] static QJsonObject buildDashboardProjectJson(int language, bool withStrings);
};

}  // namespace Benchmark
