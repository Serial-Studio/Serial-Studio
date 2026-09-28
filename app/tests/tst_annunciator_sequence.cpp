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

#include <QtTest>

#include "UI/Alarms/AnnunciatorSequence.h"

using UI::Alarms::AnnunciatorSequence;
using UI::Alarms::PointKey;
using UI::Alarms::PointKind;
using UI::Alarms::PointState;
using UI::Alarms::Priority;
using UI::Alarms::Sequence;

/**
 * @brief ISA-18.1 transition tables for sequences A, M and R, the operator actions, the audible
 *        arbitration and the reflash rule (spec 0087 R2 to R7).
 */
class TstAnnunciatorSequence : public QObject {
  Q_OBJECT

private slots:
  void raiseCreatesAlertAndSounds();
  void sequenceADropsOnClearAfterAcknowledge();
  void sequenceA4DropsOnClearBeforeAcknowledge();
  void sequenceM4ParksUnacknowledgedOnClear();
  void plainSequencesLockInUntilAcknowledged();
  void sequenceMParksInReturnToNormalUntilReset();
  void sequenceRRingsBackOnlyWhenNothingAlerts();
  void silenceQuietsWithoutAcknowledging();
  void reraiseAfterSilenceReflashes();
  void priorityRiseReflashesAndReAlertsAcknowledged();
  void arbitrationPicksHighestUnsilencedAlert();
  void latestAlertIsMostRecentRaise();
  void clearKindDropsOnlyThatKind();
  void switchingToADropsReturnToNormal();
  void pointCapIsEnforced();

private:
  [[nodiscard]] static PointKey band(int id) { return PointKey{PointKind::Band, id}; }

  [[nodiscard]] static PointKey note(int id) { return PointKey{PointKind::Notification, id}; }

  [[nodiscard]] static bool raise(AnnunciatorSequence& s, const PointKey& k, Priority p, qint64 t)
  {
    return s.raise(k, p, QStringLiteral("t"), QStringLiteral("c"), QString(), QString(), t);
  }
};

void TstAnnunciatorSequence::raiseCreatesAlertAndSounds()
{
  AnnunciatorSequence s(Sequence::A);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QCOMPARE(s.unacknowledgedCount(), 1);
  QCOMPARE(s.soundingPriority(), Priority::Warning);
  QCOMPARE(s.highestActivePriority(), Priority::Warning);
  QVERIFY(s.anyAlert());
  QVERIFY(!raise(s, band(1), Priority::Warning, 11));
}

void TstAnnunciatorSequence::sequenceADropsOnClearAfterAcknowledge()
{
  AnnunciatorSequence s(Sequence::A);
  QVERIFY(raise(s, band(1), Priority::Caution, 10));
  QCOMPARE(s.acknowledge(), 1);
  QCOMPARE(s.find(band(1))->state, PointState::Acknowledged);
  QVERIFY(!s.clear(band(1), 20));
  QVERIFY(s.find(band(1)) == nullptr);
  QCOMPARE(s.highestActivePriority(), Priority::None);
}

void TstAnnunciatorSequence::sequenceA4DropsOnClearBeforeAcknowledge()
{
  AnnunciatorSequence s(Sequence::A4);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QVERIFY(!s.clear(band(1), 20));
  QVERIFY(s.find(band(1)) == nullptr);
  QCOMPARE(s.soundingPriority(), Priority::None);
  QCOMPARE(s.unacknowledgedCount(), 0);
  QCOMPARE(s.acknowledge(), 0);
}

void TstAnnunciatorSequence::sequenceM4ParksUnacknowledgedOnClear()
{
  AnnunciatorSequence s(Sequence::M4);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QVERIFY(!s.clear(band(1), 20));
  QCOMPARE(s.find(band(1))->state, PointState::ReturnToNormal);
  QCOMPARE(s.soundingPriority(), Priority::None);
  QCOMPARE(s.unacknowledgedCount(), 0);
  QVERIFY(raise(s, band(1), Priority::Warning, 30));
  QCOMPARE(s.find(band(1))->state, PointState::Alert);
}

void TstAnnunciatorSequence::plainSequencesLockInUntilAcknowledged()
{
  for (const Sequence sequence : {Sequence::A, Sequence::M, Sequence::R}) {
    AnnunciatorSequence s(sequence);
    QVERIFY(raise(s, band(1), Priority::Warning, 10));
    QVERIFY(!s.clear(band(1), 20));
    QCOMPARE(s.find(band(1))->state, PointState::Alert);
    QCOMPARE(s.soundingPriority(), Priority::Warning);
    QCOMPARE(s.acknowledge(), 1);
    if (sequence == Sequence::A)
      QVERIFY(s.find(band(1)) == nullptr);
    else
      QCOMPARE(s.find(band(1))->state, PointState::ReturnToNormal);
  }
}

void TstAnnunciatorSequence::sequenceMParksInReturnToNormalUntilReset()
{
  AnnunciatorSequence s(Sequence::M);
  QVERIFY(raise(s, note(7), Priority::Warning, 10));
  QCOMPARE(s.acknowledge(), 1);
  QVERIFY(!s.clear(note(7), 20));
  QCOMPARE(s.find(note(7))->state, PointState::ReturnToNormal);
  QVERIFY(!s.ringbackPending());
  QCOMPARE(s.soundingPriority(), Priority::None);
  QCOMPARE(s.reset(), 1);
  QVERIFY(s.find(note(7)) == nullptr);
}

void TstAnnunciatorSequence::sequenceRRingsBackOnlyWhenNothingAlerts()
{
  AnnunciatorSequence s(Sequence::R);
  QVERIFY(raise(s, band(1), Priority::Caution, 10));
  QVERIFY(raise(s, band(2), Priority::Warning, 11));
  QCOMPARE(s.acknowledge(), 2);
  QVERIFY(s.clear(band(1), 20));
  QVERIFY(s.ringbackPending());
  QVERIFY(raise(s, band(3), Priority::Caution, 30));
  QCOMPARE(s.soundingPriority(), Priority::Caution);
  QCOMPARE(s.silence(), 2);
  QVERIFY(!s.ringbackPending());
  QCOMPARE(s.reset(), 1);
  QVERIFY(s.find(band(1)) == nullptr);
  QVERIFY(s.find(band(2)) != nullptr);
}

void TstAnnunciatorSequence::silenceQuietsWithoutAcknowledging()
{
  AnnunciatorSequence s(Sequence::A);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QCOMPARE(s.silence(), 1);
  QCOMPARE(s.soundingPriority(), Priority::None);
  QCOMPARE(s.unacknowledgedCount(), 1);
  QVERIFY(s.anyAlert());
  QCOMPARE(s.silence(), 0);
}

void TstAnnunciatorSequence::reraiseAfterSilenceReflashes()
{
  AnnunciatorSequence s(Sequence::M);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QCOMPARE(s.acknowledge(), 1);
  QVERIFY(!s.clear(band(1), 20));
  QVERIFY(raise(s, band(1), Priority::Warning, 30));
  QCOMPARE(s.find(band(1))->state, PointState::Alert);
  QCOMPARE(s.silence(), 1);
  QVERIFY(!raise(s, band(1), Priority::Warning, 40));
  QCOMPARE(s.soundingPriority(), Priority::None);
  QVERIFY(raise(s, band(2), Priority::Caution, 50));
  QCOMPARE(s.soundingPriority(), Priority::Caution);
}

void TstAnnunciatorSequence::priorityRiseReflashesAndReAlertsAcknowledged()
{
  AnnunciatorSequence s(Sequence::A);
  QVERIFY(raise(s, band(1), Priority::Caution, 10));
  QCOMPARE(s.silence(), 1);
  QVERIFY(raise(s, band(1), Priority::Warning, 20));
  QVERIFY(!s.find(band(1))->silenced);
  QCOMPARE(s.soundingPriority(), Priority::Warning);
  QCOMPARE(s.acknowledge(), 1);
  QVERIFY(!raise(s, band(1), Priority::Warning, 30));
  QCOMPARE(s.find(band(1))->state, PointState::Acknowledged);
}

void TstAnnunciatorSequence::arbitrationPicksHighestUnsilencedAlert()
{
  AnnunciatorSequence s(Sequence::A);
  QVERIFY(raise(s, band(1), Priority::Caution, 10));
  QVERIFY(raise(s, note(2), Priority::Warning, 11));
  QCOMPARE(s.soundingPriority(), Priority::Warning);
  QCOMPARE(s.silence(), 2);
  QVERIFY(raise(s, band(3), Priority::Caution, 12));
  QCOMPARE(s.soundingPriority(), Priority::Caution);
  QCOMPARE(s.highestActivePriority(), Priority::Warning);
}

void TstAnnunciatorSequence::latestAlertIsMostRecentRaise()
{
  AnnunciatorSequence s(Sequence::A);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QVERIFY(raise(s, band(2), Priority::Warning, 20));
  QCOMPARE(s.latestAlert(Priority::Warning)->key.id, 2);
  QVERIFY(s.latestAlert(Priority::Caution) == nullptr);
}

void TstAnnunciatorSequence::clearKindDropsOnlyThatKind()
{
  AnnunciatorSequence s(Sequence::A);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QVERIFY(raise(s, note(1), Priority::Caution, 11));
  s.clearKind(PointKind::Band);
  QVERIFY(s.find(band(1)) == nullptr);
  QVERIFY(s.find(note(1)) != nullptr);
  s.clearAll();
  QCOMPARE(s.points().size(), std::size_t{0});
}

void TstAnnunciatorSequence::switchingToADropsReturnToNormal()
{
  AnnunciatorSequence s(Sequence::M);
  QVERIFY(raise(s, band(1), Priority::Warning, 10));
  QCOMPARE(s.acknowledge(), 1);
  QVERIFY(!s.clear(band(1), 20));
  QCOMPARE(s.find(band(1))->state, PointState::ReturnToNormal);
  s.setSequence(Sequence::A);
  QVERIFY(s.find(band(1)) == nullptr);
}

void TstAnnunciatorSequence::pointCapIsEnforced()
{
  AnnunciatorSequence s(Sequence::A);
  for (int i = 0; i < AnnunciatorSequence::kMaxPoints; ++i)
    QVERIFY(raise(s, band(i), Priority::Caution, i));

  QVERIFY(!raise(s, band(AnnunciatorSequence::kMaxPoints), Priority::Caution, 999));
  QCOMPARE(s.unacknowledgedCount(), AnnunciatorSequence::kMaxPoints);
}

QTEST_APPLESS_MAIN(TstAnnunciatorSequence)
#include "tst_annunciator_sequence.moc"
