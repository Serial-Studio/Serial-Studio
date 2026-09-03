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

#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <QTest>
#include <QUuid>

#include "AI/SentinelProbe.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing. Each slot uses its own compliance key, so
// the QSettings-backed compliance memory of one case cannot decide another.

/**
 * @brief Pins the context-health probe: what counts as a verbatim sentinel, what the display
 *        strip removes (and refuses to remove), and the compliance state machine that keeps an
 *        uncooperative model from raising a permanent false alarm (spec 0075, M10).
 */
class TstSentinelProbe : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void verbatimSentinelIsHealthy();
  void missingSentinelOnKnownCompliantModelDegrades();
  void unknownModelIsMutedInsteadOfDegraded();
  void mutatedSentinelNamesTheDriftedToken();
  void stripRemovesTrailingSentinel();
  void stripKeepsQuotedMarkerFollowedByProse();
  void stripHoldsBackPartialMarker();
  void restoreLatchSurvivesSnapshotRoundTrip();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a compliance key unique to this run, so no case inherits a persisted verdict.
 */
static QString freshKey()
{
  return QStringLiteral("tst/") + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * @brief Returns a reply whose final line is the verbatim sentinel.
 */
static QString replyWithSentinel(const QString& body)
{
  return body + QStringLiteral("\n") + AI::SentinelProbe::sentinelLine();
}

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Points QSettings at this suite's own organization so the developer's compliance memory
 *        is neither read nor written by the run.
 */
void TstSentinelProbe::initTestCase()
{
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_sentinel_probe"));
  QSettings().clear();
}

//--------------------------------------------------------------------------------------------------
// Classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief A verbatim sentinel is healthy and leaves the conversation undegraded.
 */
void TstSentinelProbe::verbatimSentinelIsHealthy()
{
  AI::SentinelProbe probe;
  probe.reset(freshKey());

  QCOMPARE(probe.evaluateReply(replyWithSentinel(QStringLiteral("done"))),
           AI::SentinelProbe::Outcome::Healthy);
  QVERIFY(!probe.degraded());
}

/**
 * @brief A model that complied once and then stops is the degradation signal.
 */
void TstSentinelProbe::missingSentinelOnKnownCompliantModelDegrades()
{
  AI::SentinelProbe probe;
  probe.reset(freshKey());

  QCOMPARE(probe.evaluateReply(replyWithSentinel(QStringLiteral("first"))),
           AI::SentinelProbe::Outcome::Healthy);
  QCOMPARE(probe.evaluateReply(QStringLiteral("second, no sentinel")),
           AI::SentinelProbe::Outcome::Missing);
  QVERIFY(probe.degraded());
  QCOMPARE(probe.lastFailure(), AI::SentinelProbe::Outcome::Missing);
}

/**
 * @brief A model that never emits the sentinel is muted, not reported as degraded: the banner
 *        would otherwise fire permanently on every weak local model.
 */
void TstSentinelProbe::unknownModelIsMutedInsteadOfDegraded()
{
  AI::SentinelProbe probe;
  probe.reset(freshKey());

  for (int i = 0; i < 4; ++i)
    QCOMPARE(probe.evaluateReply(QStringLiteral("plain answer")),
             AI::SentinelProbe::Outcome::Muted);

  QVERIFY(!probe.degraded());
}

/**
 * @brief A sentinel whose body lost a token reports that token, so the banner can name it.
 */
void TstSentinelProbe::mutatedSentinelNamesTheDriftedToken()
{
  AI::SentinelProbe probe;
  probe.reset(freshKey());

  QCOMPARE(probe.evaluateReply(replyWithSentinel(QStringLiteral("first"))),
           AI::SentinelProbe::Outcome::Healthy);

  auto mutated = AI::SentinelProbe::sentinelLine();
  mutated.replace(QStringLiteral("ids=fresh"), QStringLiteral("ids=reused"));
  QCOMPARE(probe.evaluateReply(QStringLiteral("body\n") + mutated),
           AI::SentinelProbe::Outcome::Mutated);
  QCOMPARE(probe.driftedSegment(), QStringLiteral("ids=fresh"));
}

//--------------------------------------------------------------------------------------------------
// Display stripping
//--------------------------------------------------------------------------------------------------

/**
 * @brief The sentinel never reaches the chat view, and the visible text keeps its own content.
 */
void TstSentinelProbe::stripRemovesTrailingSentinel()
{
  const auto text = replyWithSentinel(QStringLiteral("visible answer"));
  QCOMPARE(AI::SentinelProbe::stripForDisplay(text), QStringLiteral("visible answer"));
}

/**
 * @brief A marker the model quotes mid-reply is content, not a sentinel, and survives display.
 */
void TstSentinelProbe::stripKeepsQuotedMarkerFollowedByProse()
{
  const auto text = AI::SentinelProbe::sentinelLine() + QStringLiteral(" is the check line.");
  QCOMPARE(AI::SentinelProbe::stripForDisplay(text), text);
}

/**
 * @brief A sentinel still streaming in is held back rather than shown half-written.
 */
void TstSentinelProbe::stripHoldsBackPartialMarker()
{
  const auto text = QStringLiteral("answer [[SS-CH");
  QCOMPARE(AI::SentinelProbe::stripForDisplay(text), QStringLiteral("answer "));
}

//--------------------------------------------------------------------------------------------------
// Latch persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief A degraded chat reopened from its snapshot still shows the banner.
 */
void TstSentinelProbe::restoreLatchSurvivesSnapshotRoundTrip()
{
  AI::SentinelProbe probe;
  probe.reset(freshKey());
  probe.restoreLatch(true, AI::SentinelProbe::Outcome::Mutated, QStringLiteral("save=auto"));

  QVERIFY(probe.degraded());
  QCOMPARE(probe.lastFailure(), AI::SentinelProbe::Outcome::Mutated);
  QCOMPARE(probe.driftedSegment(), QStringLiteral("save=auto"));
}

QTEST_GUILESS_MAIN(TstSentinelProbe)

#include "tst_sentinel_probe.moc"
