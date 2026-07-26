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

#pragma once

#include <QCoreApplication>
#include <QList>
#include <QString>

#include "Misc/ProblemCenter.h"

namespace Misc::Diagnostics {

/**
 * @brief The buses a diagnostics run can cover. The enumerator order is the run order, so two
 *        runs on the same machine produce the same sequence of results.
 */
enum class Bus : int {
  Serial    = 0,
  Bluetooth = 1,
  Network   = 2,
  Broker    = 3,
  Audio     = 4,
};

/**
 * @brief Number of declared buses; sizes the per-bus result cache.
 */
inline constexpr int kBusCount = 5;

/**
 * @brief Bitset of Bus values, so one run can be scoped to any subset of the buses.
 */
using BusMask = quint32;

/**
 * @brief Empty scope: a run that covers no bus at all.
 */
inline constexpr BusMask kBusNone = 0;

/**
 * @brief Scope covering every declared bus, regardless of what the build supports.
 */
inline constexpr BusMask kBusAll = (1u << kBusCount) - 1u;

/**
 * @brief Verdict of one check. Pass produces no finding; Info states a pass worth stating.
 */
enum class Verdict : int {
  Pass    = 0,
  Info    = 1,
  Warning = 2,
  Failure = 3,
};

/**
 * @brief One check outcome: what is wrong, with the concrete value involved, and what to do
 *        about it. The remedy embeds any shell command verbatim, never through a tr() literal.
 */
struct Result {
  Bus bus         = Bus::Serial;
  Verdict verdict = Verdict::Pass;
  QString code;
  QString title;
  QString remedy;
  QString explanation;
};

/**
 * @brief Returns the bit that represents @p bus inside a BusMask.
 */
[[nodiscard]] inline constexpr BusMask busBit(Bus bus)
{
  return static_cast<BusMask>(1u) << static_cast<int>(bus);
}

/**
 * @brief Reports whether @p scope covers @p bus.
 */
[[nodiscard]] inline constexpr bool scopeCovers(BusMask scope, Bus bus)
{
  return (scope & busBit(bus)) != 0;
}

/**
 * @brief Returns the translated text for the shared "Diagnostics" translation context.
 */
[[nodiscard]] inline QString trDiag(const char* text)
{
  return QCoreApplication::translate("Diagnostics", text);
}

/**
 * @brief Returns the stable API slug of @p bus.
 */
[[nodiscard]] inline QString busSlug(Bus bus)
{
  switch (bus) {
    case Bus::Serial:
      return QStringLiteral("serial");
    case Bus::Bluetooth:
      return QStringLiteral("bluetooth");
    case Bus::Network:
      return QStringLiteral("network");
    case Bus::Broker:
      return QStringLiteral("broker");
    case Bus::Audio:
      return QStringLiteral("audio");
  }

  return QString();
}

/**
 * @brief Parses a bus slug into @p bus, returning false when the slug names no bus.
 */
[[nodiscard]] inline bool busFromSlug(const QString& slug, Bus& bus)
{
  for (int i = 0; i < kBusCount; ++i) {
    const auto candidate = static_cast<Bus>(i);
    if (busSlug(candidate).compare(slug, Qt::CaseInsensitive) != 0)
      continue;

    bus = candidate;
    return true;
  }

  return false;
}

/**
 * @brief Returns the problem-center checker id that publishes @p bus results.
 */
[[nodiscard]] inline QString checkerId(Bus bus)
{
  return QStringLiteral("diagnostics.") + busSlug(bus);
}

/**
 * @brief Returns the lowercase token the API speaks for @p verdict.
 */
[[nodiscard]] inline QString verdictName(Verdict verdict)
{
  switch (verdict) {
    case Verdict::Pass:
      return QStringLiteral("pass");
    case Verdict::Info:
      return QStringLiteral("info");
    case Verdict::Warning:
      return QStringLiteral("warning");
    case Verdict::Failure:
      return QStringLiteral("failure");
  }

  return QString();
}

/**
 * @brief Assembles one check result; the code is a stable latin-1 identifier, never translated.
 */
[[nodiscard]] inline Result makeResult(Bus bus,
                                       Verdict verdict,
                                       const char* code,
                                       const QString& title,
                                       const QString& explanation,
                                       const QString& remedy)
{
  Result result;
  result.bus         = bus;
  result.verdict     = verdict;
  result.code        = QString::fromLatin1(code);
  result.title       = title;
  result.remedy      = remedy;
  result.explanation = explanation;
  return result;
}

/**
 * @brief Maps a verdict onto the problem-center severity that renders it.
 */
[[nodiscard]] inline ProblemCenter::Severity severityOf(Verdict verdict)
{
  if (verdict == Verdict::Failure)
    return ProblemCenter::Error;

  if (verdict == Verdict::Warning)
    return ProblemCenter::Warning;

  return ProblemCenter::Info;
}

/**
 * @brief Lowers one result into the problem-center finding the panel renders; the checker id is
 *        stamped by the collector after the run.
 */
[[nodiscard]] inline ProblemCenter::Finding toFinding(const Result& result)
{
  ProblemCenter::Finding finding;
  finding.severity    = severityOf(result.verdict);
  finding.code        = result.code;
  finding.title       = result.title;
  finding.remedy      = result.remedy;
  finding.explanation = result.explanation;
  return finding;
}

}  // namespace Misc::Diagnostics
