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

#include "Misc/HotpathBenchmark.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <QByteArray>
#include <QList>

#include "IO/FrameReader.h"
#include "SerialStudio.h"

namespace Misc {

//--------------------------------------------------------------------------------------------------
// Benchmark synthesis
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds one socket-sized chunk of newline-delimited QuickPlot frames.
 */
QByteArray HotpathBenchmark::buildChunk(int frames)
{
  Q_ASSERT(frames > 0);

  static const QByteArray kFrameLine = QByteArrayLiteral("12.34,56.78,90.12,34.56\n");

  QByteArray chunk;
  chunk.reserve(frames * kFrameLine.size());
  for (int i = 0; i < frames; ++i)
    chunk.append(kFrameLine);

  Q_ASSERT(!chunk.isEmpty());
  return chunk;
}

//--------------------------------------------------------------------------------------------------
// Benchmark execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drives the real frame-extraction hotpath and measures sustained frames/sec.
 */
HotpathBenchmark::Result HotpathBenchmark::run(quint64 targetFrames, double minFps)
{
  Q_ASSERT(targetFrames > 0);
  Q_ASSERT(minFps > 0.0);

  // One chunk stays under the per-call frame cap (32768) and the 1 MiB buffer, so each call drains.
  constexpr int kFramesPerChunk = 1000;
  const QByteArray chunk        = buildChunk(kFramesPerChunk);

  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFinishSequences({QByteArrayLiteral("\n")});

  auto& queue = reader.queue();
  IO::CapturedDataPtr drained;

  using Clock          = std::chrono::steady_clock;
  const quint64 chunks = (targetFrames + kFramesPerChunk - 1) / kFramesPerChunk;
  quint64 processed    = 0;
  const auto start     = Clock::now();

  for (quint64 c = 0; c < chunks; ++c) {
    reader.processData(IO::makeCapturedData(chunk));
    // code-verify off
    while (queue.try_dequeue(drained))
      ++processed;
    // code-verify on
  }

  const auto end       = Clock::now();
  const double seconds = std::chrono::duration<double>(end - start).count();
  const double fps     = seconds > 0.0 ? static_cast<double>(processed) / seconds : 0.0;

  Result result;
  result.passed          = fps >= minFps;
  result.minFps          = minFps;
  result.framesPerSecond = fps;
  result.elapsedSeconds  = seconds;
  result.framesProcessed = processed;
  return result;
}

/**
 * @brief Runs the benchmark, prints a human + machine-readable summary, returns an exit code.
 */
int HotpathBenchmark::runAndReport(quint64 targetFrames, double minFps)
{
  const Result result = run(targetFrames, minFps);

  std::fprintf(stdout,
               "hotpath: %llu frames in %.2fs\n",
               static_cast<unsigned long long>(result.framesProcessed),
               result.elapsedSeconds);
  std::fprintf(stdout,
               "hotpath: %.0f frames/s  (target %.0f)  %s\n",
               result.framesPerSecond,
               result.minFps,
               result.passed ? "PASS" : "FAIL");
  std::fprintf(stdout,
               "HOTPATH_FPS=%.0f HOTPATH_TARGET=%.0f HOTPATH_PASS=%d\n",
               result.framesPerSecond,
               result.minFps,
               result.passed ? 1 : 0);
  std::fflush(stdout);

  return result.passed ? EXIT_SUCCESS : EXIT_FAILURE;
}

}  // namespace Misc
