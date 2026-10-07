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

#include <QTest>

#include "DataModel/RepublishGate.h"

// The two synthetic-refresh lanes (spec 0064). The property that matters is asymmetry: a
// dashboard-only refresh consumes the change-driven transform clock, so if it also discharged the
// export lane's obligation every recording sink would sit one publish behind the screen for as
// long as a stream source keeps the masked lane firing. That is how a 635-dataset project recorded
// four channels while its dashboard updated normally.

namespace {
constexpr int kSource = 0;
}  // namespace

class RepublishLanesTest : public QObject {
  Q_OBJECT

private slots:
  void bothLanesOweAFirstPublish();
  void dashboardLaneSuppressesUnchangedSource();
  void exportLaneSuppressesUnchangedSourceOnceSinksAreCurrent();
  void maskedRefreshDoesNotDischargeTheExportLane();
  void repeatedMaskedRefreshesNeverStarveTheExportLane();
  void templatePublishSuppressesTheDashboardLaneOnly();
  void clearRestoresTheFirstPublishObligation();
  void lanesCarryTheirIdentity();
};

/**
 * @brief A source neither lane has published yet is owed a publish by both.
 */
void RepublishLanesTest::bothLanesOweAFirstPublish()
{
  DataModel::RepublishGate gate;
  QVERIFY(gate.dashboardLane().needed(kSource, false));
  QVERIFY(gate.exportLane().needed(kSource, false));
}

/**
 * @brief An unchanged source is not redrawn: the dashboard lane exists to avoid touching a plot
 *        clock for data that did not move.
 */
void RepublishLanesTest::dashboardLaneSuppressesUnchangedSource()
{
  DataModel::RepublishGate gate;
  gate.dashboardLane().notePublished(kSource);

  QVERIFY(!gate.dashboardLane().needed(kSource, false));
  QVERIFY(gate.dashboardLane().needed(kSource, true));
}

/**
 * @brief Once an export publish has carried the current values, an unchanged export pass is
 *        suppressed too -- the sinks are not owed a duplicate row.
 */
void RepublishLanesTest::exportLaneSuppressesUnchangedSourceOnceSinksAreCurrent()
{
  DataModel::RepublishGate gate;
  gate.noteChanged(kSource);
  gate.exportLane().notePublished(kSource);

  QVERIFY(!gate.sinkDirty(kSource));
  QVERIFY(!gate.exportLane().needed(kSource, false));
}

/**
 * @brief The regression lock. A masked refresh sees the change and publishes to the dashboard;
 *        the export lane must still be owed a publish even though its own pass now observes
 *        changed == false, because no sink has seen those values.
 */
void RepublishLanesTest::maskedRefreshDoesNotDischargeTheExportLane()
{
  DataModel::RepublishGate gate;

  gate.noteChanged(kSource);
  gate.dashboardLane().notePublished(kSource);

  QVERIFY(gate.sinkDirty(kSource));
  QVERIFY(gate.exportLane().needed(kSource, false));
}

/**
 * @brief A stream source drives the masked lane continuously; no number of masked refreshes may
 *        leave the export lane un-owed, which is the shape of the original failure.
 */
void RepublishLanesTest::repeatedMaskedRefreshesNeverStarveTheExportLane()
{
  DataModel::RepublishGate gate;

  for (int i = 0; i < 1000; ++i) {
    gate.noteChanged(kSource);
    gate.dashboardLane().notePublished(kSource);
    QVERIFY(gate.exportLane().needed(kSource, false));
  }

  gate.exportLane().notePublished(kSource);
  QVERIFY(!gate.exportLane().needed(kSource, false));
}

/**
 * @brief A freshly published template suppresses the next dashboard refresh, but must not hide a
 *        source whose values the sinks have never seen.
 */
void RepublishLanesTest::templatePublishSuppressesTheDashboardLaneOnly()
{
  DataModel::RepublishGate gate;
  gate.notePublishedTemplate(kSource);

  QVERIFY(!gate.dashboardLane().needed(kSource, false));
  QVERIFY(!gate.exportLane().needed(kSource, false));

  gate.noteChanged(kSource);
  QVERIFY(gate.exportLane().needed(kSource, false));
}

/**
 * @brief A new session owes both lanes a first publish again.
 */
void RepublishLanesTest::clearRestoresTheFirstPublishObligation()
{
  DataModel::RepublishGate gate;
  gate.noteChanged(kSource);
  gate.exportLane().notePublished(kSource);
  gate.clear();

  QVERIFY(gate.dashboardLane().needed(kSource, false));
  QVERIFY(gate.exportLane().needed(kSource, false));
}

/**
 * @brief Each handle answers for the lane it was issued as: only the export handle feeds the
 *        sinks, and only its publish brings them current.
 */
void RepublishLanesTest::lanesCarryTheirIdentity()
{
  DataModel::RepublishGate gate;
  const auto dashboard = gate.dashboardLane();
  auto exports         = gate.exportLane();

  QVERIFY(!dashboard.feedsExports());
  QVERIFY(exports.feedsExports());

  gate.noteChanged(kSource);
  gate.dashboardLane().notePublished(kSource);
  QVERIFY(gate.sinkDirty(kSource));

  exports.notePublished(kSource);
  QVERIFY(!gate.sinkDirty(kSource));
}

QTEST_APPLESS_MAIN(RepublishLanesTest)

#include "tst_republish_lanes.moc"
