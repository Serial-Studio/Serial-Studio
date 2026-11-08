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

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QTest>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/LedPanelGroup.h"
#include "Core/SerialStudio.h"

// Spec 0114: the one definition of an LED Panel split off a group. The loader's migration and
// every boolean-tag project generator call it, so a drift here changes both at once.

using namespace DataModel;

/**
 * @brief Builds a four-dataset data grid with one output widget and distinct frame indices.
 */
static Group sourceGroup()
{
  Group g;
  g.uniqueId       = 17;
  g.parentFolderId = 3;
  g.sourceId       = 2;
  g.title          = QStringLiteral("BCM_LIGHTS");
  g.widget         = QStringLiteral("datagrid");
  for (int i = 0; i < 4; ++i) {
    Dataset d;
    d.index           = 10 + i;
    d.uniqueId        = 100 + i;
    d.title           = QStringLiteral("Light %1").arg(i);
    d.ledHigh         = 1;
    d.wgtMax          = 1;
    d.hideOnDashboard = (i == 3);
    g.datasets.push_back(d);
  }

  AlarmBand band;
  band.min      = 0.5;
  band.max      = 1.5;
  band.severity = AlarmSeverity::Critical;
  g.datasets[1].alarmBands.push_back(band);
  g.outputWidgets.emplace_back();
  return g;
}

class TstLedPanelGroup : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void copiesNamedDatasetsInOrder();
  void keepsIndexBandsAndThreshold();
  void leavesIdentityUnassigned();
  void stripsOutputWidgets();
  void ignoresOutOfRangePositions();
  void emptyPositionsGiveNoDatasets();
  void legacyPositionsSkipHiddenAndUnflagged();
  void legacyPositionsSurviveARemap();
  void legacyInsertKeepsOrderAndAllocatesIds();
  void legacyInsertRetargetsWorkspaceRefs();
};

/**
 * @brief Raw project groups as a pre-0114 file wrote them: dataset 0 and 2 flagged, 3 flagged but
 *        hidden, 1 plain; a second group with nothing flagged.
 */
static QJsonArray rawGroups()
{
  QJsonArray datasets;
  for (int i = 0; i < 4; ++i) {
    QJsonObject d;
    d.insert(QStringLiteral("title"), QStringLiteral("Light %1").arg(i));
    d.insert(QStringLiteral("index"), 10 + i);
    d.insert(QStringLiteral("led"), i != 1);
    if (i == 3)
      d.insert(QStringLiteral("hideOnDashboard"), true);
    datasets.append(d);
  }

  QJsonObject first;
  first.insert(QStringLiteral("title"), QStringLiteral("BCM_LIGHTS"));
  first.insert(QStringLiteral("datasets"), datasets);

  QJsonObject second;
  second.insert(QStringLiteral("title"), QStringLiteral("Plain"));
  second.insert(QStringLiteral("datasets"), QJsonArray{datasets.at(1)});
  return QJsonArray{first, second};
}

/**
 * @brief The loaded twin of rawGroups(): positional ids as the reader stores them.
 */
static std::vector<Group> loadedGroups()
{
  auto first    = sourceGroup();
  first.groupId = 0;
  for (size_t i = 0; i < first.datasets.size(); ++i)
    first.datasets[i].datasetId = static_cast<int>(i);

  Group second;
  second.groupId  = 1;
  second.uniqueId = 18;
  second.title    = QStringLiteral("Plain");
  second.widget   = QStringLiteral("datagrid");
  second.datasets.push_back(first.datasets[1]);
  second.datasets[0].datasetId = 0;
  return {first, second};
}

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, so the out-of-range skip is
 *        observable from a debug build.
 */
void TstLedPanelGroup::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

void TstLedPanelGroup::copiesNamedDatasetsInOrder()
{
  const auto panel = deriveLedPanelGroup(sourceGroup(), {2, 0});
  QCOMPARE(panel.datasets.size(), size_t(2));
  QCOMPARE(panel.datasets[0].title, QStringLiteral("Light 2"));
  QCOMPARE(panel.datasets[1].title, QStringLiteral("Light 0"));
  QCOMPARE(panel.datasets[0].datasetId, 0);
  QCOMPARE(panel.datasets[1].datasetId, 1);
  QCOMPARE(panel.widget, QStringLiteral("ledpanel"));
  QCOMPARE(panel.title, QStringLiteral("BCM_LIGHTS LEDs"));
  QCOMPARE(panel.sourceId, 2);
  QCOMPARE(panel.parentFolderId, 3);
  QCOMPARE(panel.groupType, GroupType::Input);
}

void TstLedPanelGroup::keepsIndexBandsAndThreshold()
{
  const auto panel = deriveLedPanelGroup(sourceGroup(), {1});
  QCOMPARE(panel.datasets[0].index, 11);
  QCOMPARE(panel.datasets[0].ledHigh, 1.0);
  QCOMPARE(panel.datasets[0].wgtMax, 1.0);
  QCOMPARE(panel.datasets[0].alarmBands.size(), size_t(1));
  QCOMPARE(panel.datasets[0].alarmBands[0].severity, AlarmSeverity::Critical);
}

void TstLedPanelGroup::leavesIdentityUnassigned()
{
  const auto panel = deriveLedPanelGroup(sourceGroup(), {0, 1});
  QCOMPARE(panel.uniqueId, -1);
  QCOMPARE(panel.datasets[0].uniqueId, -1);
  QCOMPARE(panel.datasets[1].uniqueId, -1);
}

void TstLedPanelGroup::stripsOutputWidgets()
{
  const auto panel = deriveLedPanelGroup(sourceGroup(), {0});
  QVERIFY(panel.outputWidgets.empty());
}

void TstLedPanelGroup::ignoresOutOfRangePositions()
{
  const auto panel = deriveLedPanelGroup(sourceGroup(), {-1, 0, 4});
  QCOMPARE(panel.datasets.size(), size_t(1));
  QCOMPARE(panel.datasets[0].title, QStringLiteral("Light 0"));
}

void TstLedPanelGroup::emptyPositionsGiveNoDatasets()
{
  const auto panel = deriveLedPanelGroup(sourceGroup(), {});
  QVERIFY(panel.datasets.empty());
  QCOMPARE(panel.widget, QStringLiteral("ledpanel"));
}

void TstLedPanelGroup::legacyPositionsSkipHiddenAndUnflagged()
{
  const auto groups = loadedGroups();
  QCOMPARE(legacyLedPositions(groups[0], rawGroups()), (std::vector<int>{0, 2}));
  QVERIFY(legacyLedPositions(groups[1], rawGroups()).empty());
}

void TstLedPanelGroup::legacyPositionsSurviveARemap()
{
  auto groups       = loadedGroups();
  groups[0].groupId = 7;
  for (auto& dataset : groups[0].datasets)
    dataset.datasetId = 42;

  QCOMPARE(legacyLedPositions(groups[0], rawGroups()), (std::vector<int>{0, 2}));
  groups[0].datasets[2].index = 99;
  QCOMPARE(legacyLedPositions(groups[0], rawGroups()), (std::vector<int>{0}));
}

void TstLedPanelGroup::legacyInsertKeepsOrderAndAllocatesIds()
{
  auto groups = loadedGroups();
  std::vector<Workspace> workspaces;
  int next       = 1000;
  const auto uid = [&next]() {
    return next++;
  };
  const bool done = insertLegacyLedPanels(groups, workspaces, rawGroups(), uid);
  QVERIFY(done);
  QCOMPARE(groups.size(), size_t(3));
  QCOMPARE(groups[0].title, QStringLiteral("BCM_LIGHTS"));
  QCOMPARE(groups[1].widget, QStringLiteral("ledpanel"));
  QCOMPARE(groups[1].uniqueId, 1000);
  QCOMPARE(groups[1].datasets.size(), size_t(2));
  QCOMPARE(groups[1].datasets[0].uniqueId, 1001);
  QCOMPARE(groups[1].datasets[1].uniqueId, 1002);
  QCOMPARE(groups[1].datasets[1].datasetId, 1);
  QCOMPARE(groups[2].title, QStringLiteral("Plain"));
  QVERIFY(!insertLegacyLedPanels(groups, workspaces, QJsonArray(), uid));
}

void TstLedPanelGroup::legacyInsertRetargetsWorkspaceRefs()
{
  auto groups = loadedGroups();
  Workspace ws;
  WidgetRef led;
  led.widgetType    = static_cast<int>(SerialStudio::DashboardLED);
  led.groupUniqueId = 17;
  WidgetRef grid;
  grid.widgetType    = static_cast<int>(SerialStudio::DashboardDataGrid);
  grid.groupUniqueId = 17;
  ws.widgetRefs      = {led, grid};
  std::vector<Workspace> workspaces{ws};

  int next = 500;
  QVERIFY(insertLegacyLedPanels(groups, workspaces, rawGroups(), [&next]() { return next++; }));
  QCOMPARE(workspaces[0].widgetRefs[0].groupUniqueId, 500);
  QCOMPARE(workspaces[0].widgetRefs[1].groupUniqueId, 17);
}

QTEST_APPLESS_MAIN(TstLedPanelGroup)

#include "tst_led_panel_group.moc"
