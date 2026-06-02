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

#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QtGlobal>

namespace Misc {

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
    int language;
    double minFps;
    double framesPerSecond;
    double elapsedSeconds;
    quint64 framesParsed;
    quint64 framesSkipped;
  };

  [[nodiscard]] static Result run(
    quint64 targetFrames, double minFps, double minSeconds, int language, bool withExporters);
  [[nodiscard]] static int runAndReport(quint64 targetFrames, double minFps, double minSeconds);

private:
  static void enableConsumers();
  static void disableConsumers();
  static void setupProject(int language, int channels);
  [[nodiscard]] static QByteArray buildChunk(int frames, int channels);
  [[nodiscard]] static QJsonObject buildProjectJson(int language, int channels);
};

}  // namespace Misc
