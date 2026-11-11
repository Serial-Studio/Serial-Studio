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

#include <algorithm>
#include <initializer_list>
#include <QTest>
#include <vector>

#include "DataModel/FrameBuilder/SessionAnnouncer.h"
#include "UI/Dashboard/RebuildRetryLedger.h"

// The re-sync decisions (spec 0119). On 2026-11-09 a licence update re-synced a live two-source
// session: the builder dropped its per-source layouts without announcing them again, the combined
// frame went out as source 0's layout, and one global retry flag kept being re-armed by the healthy
// source, so the dashboard rebuilt on every tick. Each case below pins one half of that fix.

namespace {

constexpr int kSourceA         = 0;
constexpr int kSourceB         = 1;
constexpr int kUnknownSource   = 42;
constexpr quint64 kGeneration  = 7;
constexpr quint64 kGenerationB = 8;

enum class Call {
  Forget,
  Source,
  ProjectFrame,
};

struct Recorded {
  Call call;
  int sourceId;
};

class RecordingHost final : public DataModel::SessionAnnouncerHost {
public:
  void announceSourceTemplate(const DataModel::Source& src) override
  {
    log.push_back({Call::Source, src.sourceId});
  }

  void announceProjectFrame() override { log.push_back({Call::ProjectFrame, -1}); }

  void forgetPublishedStructures() override { log.push_back({Call::Forget, -1}); }

  [[nodiscard]] int announcements(int sourceId) const
  {
    return static_cast<int>(std::count_if(log.begin(), log.end(), [sourceId](const Recorded& r) {
      return r.call == Call::Source && r.sourceId == sourceId;
    }));
  }

  std::vector<Recorded> log;
};

/**
 * @brief Builds a titled project with the given sources and one group of @p widget per grouped id.
 */
DataModel::Frame makeProject(std::initializer_list<int> sources,
                             std::initializer_list<int> groupedSources,
                             const QString& widget = QStringLiteral("datagrid"))
{
  DataModel::Frame project;
  project.title = QStringLiteral("Project");
  for (const int id : sources) {
    DataModel::Source source;
    source.sourceId = id;
    project.sources.push_back(source);
  }

  for (const int id : groupedSources) {
    DataModel::Group group;
    group.sourceId = id;
    group.widget   = widget;
    project.groups.push_back(group);
  }

  return project;
}

}  // namespace

class ResyncDecisionsTest : public QObject {
  Q_OBJECT

private slots:
  void connectAnnouncesEverySourceWithGroups();
  void connectSkipsGrouplessSource();
  void connectAnnouncesProjectFrameForImageOnlySource();
  void connectAnnouncesNothingForOtherSingleSource();
  void connectAnnouncesNothingForMixedSingleSource();
  void zeroSourcesBehaveAsSingleSource();
  void resyncForgetsBeforeAnnouncing();
  void resyncAnnouncesEveryStreamSource();
  void resyncAnnouncesStreamSourceOfSingleSourceProject();
  void resyncRepublishesProjectFrameForImageOnlySource();
  void resyncIsInertOutsideALiveProject_data();
  void resyncIsInertOutsideALiveProject();
  void announceSourceIgnoresUnknownId();
  void combinedFrameNeverStandsInForTwoSources();
  void combinedFrameStandsInForOneSourceWithoutOwnFrame();
  void combinedFrameNeedsGroupsAndTitle();
  void streamSourceDoesNotPublishOwnFrame();
  void otherSourceNeverRearmsARetry();
  void quarantinedPairDrops();
  void newGenerationRebuildsOnce();
  void generationReturningToAnEarlierValueRebuilds();
  void clearForgetsRetryAndQuarantine();
};

/**
 * @brief A multi-source connect announces one layout per source, and no whole frame.
 */
void ResyncDecisionsTest::connectAnnouncesEverySourceWithGroups()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA, kSourceB}, {kSourceA, kSourceB});
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceConnect();

  QCOMPARE(host.log.size(), std::size_t(2));
  QCOMPARE(host.announcements(kSourceA), 1);
  QCOMPARE(host.announcements(kSourceB), 1);
}

/**
 * @brief A source with no group has no layout to announce.
 */
void ResyncDecisionsTest::connectSkipsGrouplessSource()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA, kSourceB}, {kSourceA});
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceConnect();

  QCOMPARE(host.announcements(kSourceA), 1);
  QCOMPARE(host.announcements(kSourceB), 0);
}

/**
 * @brief An image-only single source announces the whole frame: it may never send a value block.
 */
void ResyncDecisionsTest::connectAnnouncesProjectFrameForImageOnlySource()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA}, {kSourceA}, QStringLiteral("image"));
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceConnect();

  QCOMPARE(host.log.size(), std::size_t(1));
  QCOMPARE(host.log.front().call, Call::ProjectFrame);
}

/**
 * @brief Any other single source announces nothing ahead of its first block.
 */
void ResyncDecisionsTest::connectAnnouncesNothingForOtherSingleSource()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA}, {kSourceA});
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceConnect();

  QVERIFY(host.log.empty());
}

/**
 * @brief One image group among others is not image-only, so the first block carries the layout.
 */
void ResyncDecisionsTest::connectAnnouncesNothingForMixedSingleSource()
{
  RecordingHost host;
  auto project = makeProject({kSourceA}, {kSourceA}, QStringLiteral("image"));
  DataModel::Group plain;
  plain.sourceId = kSourceA;
  plain.widget   = QStringLiteral("datagrid");
  project.groups.push_back(plain);

  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceConnect();

  QVERIFY(host.log.empty());
}

/**
 * @brief A project with no source list is treated as single-source, as the code before spec 0119.
 */
void ResyncDecisionsTest::zeroSourcesBehaveAsSingleSource()
{
  RecordingHost host;
  const auto project = makeProject({}, {kSourceA});
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceConnect();

  QVERIFY(host.log.empty());
  QVERIFY(announcer.combinedFrameStandsIn(false));
}

/**
 * @brief A re-sync forgets first, or layouts already marked published would never go out again.
 */
void ResyncDecisionsTest::resyncForgetsBeforeAnnouncing()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA, kSourceB}, {kSourceA, kSourceB});
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceResync(true, SerialStudio::ProjectFile);

  QCOMPARE(host.log.size(), std::size_t(3));
  QCOMPARE(host.log.front().call, Call::Forget);
  QCOMPARE(host.announcements(kSourceA), 1);
  QCOMPARE(host.announcements(kSourceB), 1);
}

/**
 * @brief A re-sync reaches every stream source; the count is deliberately not pinned (spec 0119
 *        records a duplicate template rebuild as a non-goal). The connect pass already covers a
 *        multi-source project, so the single-source case below is what pins the stream pass.
 */
void ResyncDecisionsTest::resyncAnnouncesEveryStreamSource()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA, kSourceB}, {kSourceA, kSourceB});
  const QSet<int> stream{kSourceB};
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceResync(true, SerialStudio::ProjectFile);

  QVERIFY(host.announcements(kSourceA) >= 1);
  QVERIFY(host.announcements(kSourceB) >= 1);
}

/**
 * @brief A single-source project announces nothing at connect, so only the stream pass reaches it.
 */
void ResyncDecisionsTest::resyncAnnouncesStreamSourceOfSingleSourceProject()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA}, {kSourceA});
  const QSet<int> stream{kSourceA};
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceResync(true, SerialStudio::ProjectFile);

  QVERIFY(host.announcements(kSourceA) >= 1);
}

/**
 * @brief An image-only project gets its whole frame again, after the forget.
 */
void ResyncDecisionsTest::resyncRepublishesProjectFrameForImageOnlySource()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA}, {kSourceA}, QStringLiteral("image"));
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceResync(true, SerialStudio::ProjectFile);

  QCOMPARE(host.log.size(), std::size_t(2));
  QCOMPARE(host.log.front().call, Call::Forget);
  QCOMPARE(host.log.back().call, Call::ProjectFrame);
}

/**
 * @brief The states in which a re-sync must do nothing.
 */
void ResyncDecisionsTest::resyncIsInertOutsideALiveProject_data()
{
  QTest::addColumn<bool>("connected");
  QTest::addColumn<int>("mode");
  QTest::addColumn<bool>("hasGroups");
  QTest::addColumn<bool>("hasTitle");

  QTest::newRow("disconnected") << false << int(SerialStudio::ProjectFile) << true << true;
  QTest::newRow("quick plot") << true << int(SerialStudio::QuickPlot) << true << true;
  QTest::newRow("console only") << true << int(SerialStudio::ConsoleOnly) << true << true;
  QTest::newRow("no groups") << true << int(SerialStudio::ProjectFile) << false << true;
  QTest::newRow("no title") << true << int(SerialStudio::ProjectFile) << true << false;
}

/**
 * @brief An inert re-sync also forgets nothing: clearing the marks with no announcement to follow
 *        would republish layouts the next block did not need.
 */
void ResyncDecisionsTest::resyncIsInertOutsideALiveProject()
{
  QFETCH(bool, connected);
  QFETCH(int, mode);
  QFETCH(bool, hasGroups);
  QFETCH(bool, hasTitle);

  RecordingHost host;
  auto project = makeProject({kSourceA, kSourceB}, {kSourceA, kSourceB});
  if (!hasGroups)
    project.groups.clear();

  if (!hasTitle)
    project.title.clear();

  const QSet<int> stream{kSourceB};
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceResync(connected, static_cast<SerialStudio::OperationMode>(mode));

  QVERIFY(host.log.empty());
}

/**
 * @brief A stream worker naming a source the project no longer has announces nothing.
 */
void ResyncDecisionsTest::announceSourceIgnoresUnknownId()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA}, {kSourceA});
  const QSet<int> stream;
  DataModel::SessionAnnouncer announcer(host, project, stream);

  announcer.announceSource(kUnknownSource);

  QVERIFY(host.log.empty());
}

/**
 * @brief In a two-source project the combined frame would claim every layout as source 0's.
 */
void ResyncDecisionsTest::combinedFrameNeverStandsInForTwoSources()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA, kSourceB}, {kSourceA, kSourceB});
  const QSet<int> stream;
  const DataModel::SessionAnnouncer announcer(host, project, stream);

  QVERIFY(!announcer.combinedFrameStandsIn(false));
  QVERIFY(!announcer.combinedFrameStandsIn(true));
}

/**
 * @brief One source: the combined frame stands in only when no own frame took part.
 */
void ResyncDecisionsTest::combinedFrameStandsInForOneSourceWithoutOwnFrame()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA}, {kSourceA});
  const QSet<int> stream;
  const DataModel::SessionAnnouncer announcer(host, project, stream);

  QVERIFY(announcer.combinedFrameStandsIn(false));
  QVERIFY(!announcer.combinedFrameStandsIn(true));
}

/**
 * @brief An empty project has no frame worth standing in.
 */
void ResyncDecisionsTest::combinedFrameNeedsGroupsAndTitle()
{
  RecordingHost host;
  const QSet<int> stream;

  auto noGroups = makeProject({kSourceA}, {});
  const DataModel::SessionAnnouncer withoutGroups(host, noGroups, stream);
  QVERIFY(!withoutGroups.combinedFrameStandsIn(false));

  auto noTitle = makeProject({kSourceA}, {kSourceA});
  noTitle.title.clear();
  const DataModel::SessionAnnouncer withoutTitle(host, noTitle, stream);
  QVERIFY(!withoutTitle.combinedFrameStandsIn(false));
}

/**
 * @brief A stream source's values come from the stream ingest, so it never publishes its own frame.
 */
void ResyncDecisionsTest::streamSourceDoesNotPublishOwnFrame()
{
  RecordingHost host;
  const auto project = makeProject({kSourceA, kSourceB}, {kSourceA, kSourceB});
  const QSet<int> stream{kSourceB};
  const DataModel::SessionAnnouncer announcer(host, project, stream);

  QVERIFY(announcer.publishesOwnFrame(kSourceA));
  QVERIFY(!announcer.publishesOwnFrame(kSourceB));
}

/**
 * @brief The 2026-11-09 shape: B's whole retry cycle runs between A's two mismatches, and A is
 *        still quarantined. Both use one generation, so a single shared flag fails on B's first.
 */
void ResyncDecisionsTest::otherSourceNeverRearmsARetry()
{
  UI::RebuildRetryLedger ledger;

  QCOMPARE(ledger.onMismatch(kSourceA, kGeneration), UI::RebuildVerdict::Rebuild);
  QCOMPARE(ledger.onMismatch(kSourceB, kGeneration), UI::RebuildVerdict::Rebuild);
  QCOMPARE(ledger.onMismatch(kSourceB, kGeneration), UI::RebuildVerdict::Quarantine);
  QCOMPARE(ledger.onMismatch(kSourceA, kGeneration), UI::RebuildVerdict::Quarantine);
}

/**
 * @brief A quarantined pair stays dropped, and does not warn again.
 */
void ResyncDecisionsTest::quarantinedPairDrops()
{
  UI::RebuildRetryLedger ledger;

  (void)ledger.onMismatch(kSourceA, kGeneration);
  (void)ledger.onMismatch(kSourceA, kGeneration);

  QCOMPARE(ledger.onMismatch(kSourceA, kGeneration), UI::RebuildVerdict::Drop);
  QCOMPARE(ledger.onMismatch(kSourceA, kGeneration), UI::RebuildVerdict::Drop);
}

/**
 * @brief A new structure generation earns exactly one fresh rebuild.
 */
void ResyncDecisionsTest::newGenerationRebuildsOnce()
{
  UI::RebuildRetryLedger ledger;

  (void)ledger.onMismatch(kSourceA, kGeneration);
  (void)ledger.onMismatch(kSourceA, kGeneration);

  QCOMPARE(ledger.onMismatch(kSourceA, kGenerationB), UI::RebuildVerdict::Rebuild);
  QCOMPARE(ledger.onMismatch(kSourceA, kGenerationB), UI::RebuildVerdict::Quarantine);
}

/**
 * @brief Any generation change earns a fresh rebuild, even back to a quarantined value: local
 *        generations only rise, but a mirror session takes them from a peer that can restart.
 */
void ResyncDecisionsTest::generationReturningToAnEarlierValueRebuilds()
{
  UI::RebuildRetryLedger ledger;

  (void)ledger.onMismatch(kSourceA, kGeneration);
  (void)ledger.onMismatch(kSourceA, kGeneration);
  (void)ledger.onMismatch(kSourceA, kGenerationB);

  QCOMPARE(ledger.onMismatch(kSourceA, kGeneration), UI::RebuildVerdict::Rebuild);
}

/**
 * @brief A hard reset forgets both a pending retry and a quarantine.
 */
void ResyncDecisionsTest::clearForgetsRetryAndQuarantine()
{
  UI::RebuildRetryLedger ledger;

  (void)ledger.onMismatch(kSourceA, kGeneration);
  (void)ledger.onMismatch(kSourceA, kGeneration);
  (void)ledger.onMismatch(kSourceB, kGeneration);

  ledger.clear();

  QCOMPARE(ledger.onMismatch(kSourceA, kGeneration), UI::RebuildVerdict::Rebuild);
  QCOMPARE(ledger.onMismatch(kSourceB, kGeneration), UI::RebuildVerdict::Rebuild);
}

QTEST_APPLESS_MAIN(ResyncDecisionsTest)

#include "tst_resync_decisions.moc"
