/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "Misc/Problems/LinkCheckers.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QString>

#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager.h"
#include "Misc/ProblemCenter.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constants, local aliases & sampler state
//--------------------------------------------------------------------------------------------------

using Finding  = Misc::ProblemCenter::Finding;
using Severity = Misc::ProblemCenter::Severity;

static constexpr int kSustainTicks           = 3;
static constexpr qint64 kMinSampleMs         = 500;
static constexpr double kChecksumWarnRate    = 0.05;
static constexpr quint64 kMinChecksumSamples = 20;

static bool s_haveBaseline   = false;
static qint64 s_lastSampleMs = 0;
static IO::LinkStats s_previousStats{};
static quint64 s_previousParsedFrames = 0;

static int s_noFrameTicks = 0;
static int s_noParseTicks = 0;

static quint64 s_windowBytes         = 0;
static quint64 s_windowExtracted     = 0;
static quint64 s_totalExtracted      = 0;
static quint64 s_totalDroppedFrames  = 0;
static quint64 s_totalChecksumErrors = 0;
static quint64 s_totalOverflowBytes  = 0;

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated text for the shared "Problems" translation context.
 */
[[nodiscard]] static QString trLinkProblem(const char* text)
{
  return QCoreApplication::translate("Problems", text);
}

/**
 * @brief Assembles one finding; the checker id is stamped by the problem center after the run.
 */
[[nodiscard]] static Finding makeLinkFinding(Severity severity,
                                             const char* code,
                                             const QString& title,
                                             const QString& explanation,
                                             const QString& remedy)
{
  Finding finding;
  finding.severity    = severity;
  finding.code        = QString::fromLatin1(code);
  finding.title       = title;
  finding.remedy      = remedy;
  finding.explanation = explanation;
  return finding;
}

/**
 * @brief Maps a live counter onto a decade bucket, so the finding text stays identical while the
 *        condition stays the same and the panel is not reset once per second.
 */
[[nodiscard]] static QString bucketLabel(quint64 value)
{
  if (value >= 1000000)
    return trLinkProblem("more than a million");

  if (value >= 100000)
    return trLinkProblem("more than 100,000");

  if (value >= 10000)
    return trLinkProblem("more than 10,000");

  if (value >= 1000)
    return trLinkProblem("more than 1,000");

  if (value >= 100)
    return trLinkProblem("more than 100");

  if (value >= 10)
    return trLinkProblem("more than 10");

  return trLinkProblem("a few");
}

/**
 * @brief Maps a failure ratio onto a coarse band, for the same text-stability reason as
 *        bucketLabel().
 */
[[nodiscard]] static QString rateBand(double rate)
{
  if (rate >= 0.5)
    return trLinkProblem("More than half");

  if (rate >= 0.2)
    return trLinkProblem("More than a fifth");

  return trLinkProblem("More than one in twenty");
}

//--------------------------------------------------------------------------------------------------
// Sampler
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears the sustained-condition windows and the accumulated totals; called whenever the
 *        link goes away or the reader is recreated, so a fixed link starts from a clean slate.
 */
static void resetWindows()
{
  s_noFrameTicks        = 0;
  s_noParseTicks        = 0;
  s_windowBytes         = 0;
  s_windowExtracted     = 0;
  s_totalExtracted      = 0;
  s_totalDroppedFrames  = 0;
  s_totalChecksumErrors = 0;
  s_totalOverflowBytes  = 0;
}

/**
 * @brief Drops the sampler baseline as well, so the next sample re-seeds instead of computing a
 *        delta against counters that belong to a different reader.
 */
static void resetSampler()
{
  resetWindows();
  s_haveBaseline         = false;
  s_previousStats        = IO::LinkStats{};
  s_previousParsedFrames = 0;
}

/**
 * @brief Reports whether link diagnostics apply right now: a replay bypasses FrameReader entirely
 *        and a closed link produces no counters, so both suppress the checks instead of reporting
 *        a silent stream.
 */
[[nodiscard]] static bool linkSuppressed()
{
  if (SerialStudio::isAnyPlayerOpen())
    return true;

  static auto& manager = IO::ConnectionManager::instance();

  return !manager.isConnected();
}

/**
 * @brief Detects a recreated FrameReader: reconnecting or reconfiguring builds a fresh reader whose
 *        counters restart at zero, so any decrease means "reset", never a negative rate.
 */
[[nodiscard]] static bool countersWereReset(const IO::LinkStats& stats, quint64 parsed)
{
  return stats.bytesIn < s_previousStats.bytesIn
      || stats.framesExtracted < s_previousStats.framesExtracted
      || stats.checksumErrors < s_previousStats.checksumErrors
      || stats.droppedFrames < s_previousStats.droppedFrames
      || stats.overflowBytes < s_previousStats.overflowBytes || parsed < s_previousParsedFrames;
}

/**
 * @brief Folds one sample's deltas into the accumulated totals and advances the sustained-condition
 *        tick counters.
 */
static void accumulate(const IO::LinkStats& stats, quint64 parsed)
{
  const quint64 bytes     = stats.bytesIn - s_previousStats.bytesIn;
  const quint64 extracted = stats.framesExtracted - s_previousStats.framesExtracted;
  const quint64 parsedNow = parsed - s_previousParsedFrames;

  s_totalExtracted      += extracted;
  s_totalDroppedFrames  += stats.droppedFrames - s_previousStats.droppedFrames;
  s_totalChecksumErrors += stats.checksumErrors - s_previousStats.checksumErrors;
  s_totalOverflowBytes  += stats.overflowBytes - s_previousStats.overflowBytes;

  if (bytes > 0 && extracted == 0) {
    ++s_noFrameTicks;
    s_windowBytes += bytes;
  } else {
    s_noFrameTicks = 0;
    s_windowBytes  = 0;
  }

  if (extracted > 0 && parsedNow == 0) {
    ++s_noParseTicks;
    s_windowExtracted += extracted;
  } else {
    s_noParseTicks    = 0;
    s_windowExtracted = 0;
  }
}

/**
 * @brief Takes at most one sample per half second, so an on-demand re-run reports the state the
 *        1 Hz tick built instead of collapsing the sustained-condition windows.
 */
static void advanceSampler()
{
  const qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (s_haveBaseline && (now - s_lastSampleMs) < kMinSampleMs)
    return;

  static auto& manager = IO::ConnectionManager::instance();
  static auto& builder = DataModel::FrameBuilder::instance();

  const auto stats  = manager.linkStats();
  const auto parsed = builder.parsedFrameCount();
  if (!s_haveBaseline || countersWereReset(stats, parsed))
    resetWindows();
  else
    accumulate(stats, parsed);

  s_lastSampleMs         = now;
  s_haveBaseline         = true;
  s_previousStats        = stats;
  s_previousParsedFrames = parsed;
}

//--------------------------------------------------------------------------------------------------
// Individual reports
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports a link that receives bytes without ever completing a frame, the signature of a
 *        delimiter that does not match what the device sends.
 */
static void reportNoFrames(QList<Finding>& out)
{
  if (s_noFrameTicks < kSustainTicks)
    return;

  out.append(
    makeLinkFinding(Misc::ProblemCenter::Error,
                    "bytes-without-frames",
                    trLinkProblem("Data is arriving but no frames are extracted"),
                    trLinkProblem("The link has received %1 bytes over the last few seconds "
                                  "without completing a single frame.")
                      .arg(bucketLabel(s_windowBytes)),
                    trLinkProblem("Check the frame detection mode and the start/end delimiters "
                                  "of the source; they must match what the device sends.")));
}

/**
 * @brief Reports frames that are extracted but yield no dataset values, the signature of a parser
 *        that returns an empty result.
 */
static void reportNoParsedFrames(QList<Finding>& out)
{
  if (s_noParseTicks < kSustainTicks)
    return;

  out.append(
    makeLinkFinding(Misc::ProblemCenter::Error,
                    "frames-without-values",
                    trLinkProblem("Frames are arriving but none are parsed"),
                    trLinkProblem("%1 frames were extracted from the link over the last few "
                                  "seconds, and none of them produced dataset values.")
                      .arg(bucketLabel(s_windowExtracted)),
                    trLinkProblem("Open the frame parser and confirm it returns one value per "
                                  "dataset for the frames the device sends.")));
}

/**
 * @brief Reports a checksum failure rate above the threshold, computed over the totals accumulated
 *        since the link opened so the reported band does not flicker.
 */
static void reportChecksumFailures(QList<Finding>& out)
{
  const quint64 total = s_totalChecksumErrors + s_totalExtracted;
  if (total < kMinChecksumSamples || s_totalChecksumErrors == 0)
    return;

  const double rate = static_cast<double>(s_totalChecksumErrors) / static_cast<double>(total);
  if (rate < kChecksumWarnRate)
    return;

  out.append(
    makeLinkFinding(Misc::ProblemCenter::Warning,
                    "checksum-failures",
                    trLinkProblem("Frames are failing the checksum"),
                    trLinkProblem("%1 of the frames received on this link failed the configured "
                                  "checksum and were discarded.")
                      .arg(rateBand(rate)),
                    trLinkProblem("Confirm the checksum algorithm of the source matches the "
                                  "device, and check the link for noise or a baud-rate "
                                  "mismatch.")));
}

/**
 * @brief Reports frames discarded because the frame queue could not be drained fast enough.
 */
static void reportDroppedFrames(QList<Finding>& out)
{
  if (s_totalDroppedFrames == 0)
    return;

  out.append(
    makeLinkFinding(Misc::ProblemCenter::Warning,
                    "dropped-frames",
                    trLinkProblem("Frames are being dropped"),
                    trLinkProblem("The frame queue overflowed and %1 frames were discarded "
                                  "before they could be processed.")
                      .arg(bucketLabel(s_totalDroppedFrames)),
                    trLinkProblem("Lower the data rate, simplify the frame parser, or reduce the "
                                  "number of widgets on the dashboard.")));
}

/**
 * @brief Maps an EWMA parse duty onto a coarse band, for the same text-stability reason as
 *        bucketLabel().
 */
[[nodiscard]] static QString dutyBand(double duty)
{
  if (duty >= 0.9)
    return trLinkProblem("almost all");

  if (duty >= 0.5)
    return trLinkProblem("more than half");

  if (duty >= 0.25)
    return trLinkProblem("more than a quarter");

  return trLinkProblem("a significant share");
}

/**
 * @brief Maps a keep-every-Nth decimation factor onto a coarse band so the finding text stays
 *        stable while the governor fine-tunes N.
 */
[[nodiscard]] static QString thinningBand(int decimateN)
{
  if (decimateN >= 10)
    return trLinkProblem("most of its frames are being dropped");

  if (decimateN >= 4)
    return trLinkProblem("roughly one in several frames is processed");

  return trLinkProblem("every second or third frame is processed");
}

/**
 * @brief Returns the project title for @p sourceId, falling back to a numbered label. The label
 *        walk marshals to the builder thread: m_frame.sources is rebuilt there (spec 0051 M3).
 */
[[nodiscard]] static QString sourceLabel(int sourceId)
{
  static auto& builder = DataModel::FrameBuilder::instance();

  QString title;
  builder.invokeOnBuilderThreadBlocking([&] {
    const auto& sources = builder.frame().sources;
    for (const auto& source : sources)
      if (source.sourceId == sourceId && !source.title.isEmpty()) {
        title = source.title;
        return;
      }
  });

  if (!title.isEmpty())
    return title;

  return trLinkProblem("Source %1").arg(sourceId);
}

/**
 * @brief Reports every source the fair-share parse governor is currently thinning (spec 0051):
 *        one finding per offender, banded so the text stays stable while the condition holds.
 */
static void reportParseThinning(QList<Finding>& out)
{
  static auto& builder = DataModel::FrameBuilder::instance();
  if (!builder.parseBudgetThinning())
    return;

  const auto loads = builder.parseLoadSnapshot();
  for (const auto& load : loads) {
    if (load.decimateN <= 1)
      continue;

    auto finding = makeLinkFinding(
      Misc::ProblemCenter::Warning,
      "parse-thinning",
      trLinkProblem("Dashboard data from \"%1\" is being thinned").arg(sourceLabel(load.sourceId)),
      trLinkProblem("Scripts for this source are using %1 of the available parse "
                    "capacity, so %2 until its load falls back under the fair share.")
        .arg(dutyBand(load.duty), thinningBand(load.decimateN)),
      trLinkProblem("Simplify or batch the source's parser and transform scripts, lower "
                    "its data rate, or move heavy math into native pre-processing."));

    finding.entityUniqueId = load.sourceId;
    out.append(finding);
  }
}

/**
 * @brief Reports bytes lost because the receive buffer filled up before it could be drained.
 */
static void reportBufferOverflow(QList<Finding>& out)
{
  if (s_totalOverflowBytes == 0)
    return;

  out.append(
    makeLinkFinding(Misc::ProblemCenter::Warning,
                    "buffer-overflow",
                    trLinkProblem("The receive buffer is overflowing"),
                    trLinkProblem("%1 bytes were discarded because the receive buffer filled up "
                                  "before the data could be read.")
                      .arg(bucketLabel(s_totalOverflowBytes)),
                    trLinkProblem("Lower the data rate, or confirm the frames end with the "
                                  "configured delimiter so the buffer is drained.")));
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Samples the link counters once and reports every condition they show; a suppressed link
 *        clears the windows and returns no findings.
 */
static void checkLinkStatistics(QList<Finding>& out)
{
  if (linkSuppressed()) {
    resetSampler();
    return;
  }

  advanceSampler();
  reportNoFrames(out);
  reportNoParsedFrames(out);
  reportChecksumFailures(out);
  reportDroppedFrames(out);
  reportBufferOverflow(out);
  reportParseThinning(out);
}

/**
 * @brief Registers the link checker, which runs on the shared 1 Hz sample tick and on an explicit
 *        re-run request.
 */
void Misc::LinkCheckers::registerAll()
{
  static auto& center   = Misc::ProblemCenter::instance();
  const quint8 triggers = Misc::ProblemCenter::LinkSample | Misc::ProblemCenter::OnDemand;

  center.registerChecker(QStringLiteral("link.statistics"), triggers, checkLinkStatistics);
}
