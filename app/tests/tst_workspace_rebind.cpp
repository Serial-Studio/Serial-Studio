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

#include <QHash>
#include <QTest>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "DataModel/Project/WorkspaceKeys.h"

// Workspace tile identity (spec 0083): a ref's relativeIndex is a per-type dashboard ordinal
// re-derived from its (widgetType, groupUniqueId, datasetUniqueId) identity, so inserting or
// reordering groups never detaches a tile, and a legacy ref carrying only the ordinal is
// back-filled with the dataset identity it pointed at.

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a datagrid group holding one plotted dataset, both with explicit uniqueIds.
 */
static DataModel::Group makeGroup(int groupUniqueId, int datasetUniqueId, const QString& title)
{
  DataModel::Group group;
  group.uniqueId = groupUniqueId;
  group.title    = title;
  group.widget   = QStringLiteral("datagrid");

  DataModel::Dataset dataset;
  dataset.uniqueId = datasetUniqueId;
  dataset.title    = title + QStringLiteral(" ds");
  dataset.index    = 1;
  dataset.plt      = true;
  group.datasets.push_back(dataset);
  return group;
}

/**
 * @brief Builds one workspace holding a single ref.
 */
static std::vector<DataModel::Workspace> makeWorkspace(const DataModel::WidgetRef& ref)
{
  DataModel::Workspace ws;
  ws.workspaceId = WorkspaceIds::UserStart;
  ws.title       = QStringLiteral("W");
  ws.widgetRefs.push_back(ref);
  return {ws};
}

//--------------------------------------------------------------------------------------------------
// Test suite
//--------------------------------------------------------------------------------------------------

class WorkspaceRebindTest : public QObject {
  Q_OBJECT

private slots:
  void datasetRefFollowsItsDatasetAcrossAnInsert();
  void groupRefFollowsItsGroupAcrossAnInsert();
  void legacyRefIsBackFilledFromItsOrdinal();
  void danglingRefIsLeftUntouchedAndCounted();
  void savedOrdinalStillResolvesAnUnchangedProject();
};

/**
 * @brief A plot tile bound to the second group keeps its dataset when a group is inserted first.
 */
void WorkspaceRebindTest::datasetRefFollowsItsDatasetAcrossAnInsert()
{
  std::vector<DataModel::Group> groups{makeGroup(10, 100, "A"), makeGroup(20, 200, "B")};

  DataModel::WidgetRef ref;
  ref.widgetType      = static_cast<int>(SerialStudio::DashboardPlot);
  ref.groupUniqueId   = 20;
  ref.datasetUniqueId = 200;
  ref.relativeIndex   = 1;
  auto workspaces     = makeWorkspace(ref);

  groups.insert(groups.begin(), makeGroup(30, 300, "C"));
  const auto lookup = DataModel::WorkspaceKeys::buildResolvedWidgetLookup(groups, true);
  QCOMPARE(DataModel::WorkspaceKeys::rebindWidgetRefs(workspaces, lookup), 0);

  const auto& bound = workspaces.front().widgetRefs.front();
  QCOMPARE(bound.relativeIndex, 2);
  QCOMPARE(bound.datasetUniqueId, 200);
  QCOMPARE(bound.groupUniqueId, 20);
}

/**
 * @brief A group-scope tile (the datagrid itself) moves its ordinal with the group.
 */
void WorkspaceRebindTest::groupRefFollowsItsGroupAcrossAnInsert()
{
  std::vector<DataModel::Group> groups{makeGroup(20, 200, "B")};

  DataModel::WidgetRef ref;
  ref.widgetType    = static_cast<int>(SerialStudio::DashboardDataGrid);
  ref.groupUniqueId = 20;
  ref.relativeIndex = 0;
  auto workspaces   = makeWorkspace(ref);

  groups.insert(groups.begin(), makeGroup(10, 100, "A"));
  const auto lookup = DataModel::WorkspaceKeys::buildResolvedWidgetLookup(groups, true);
  QCOMPARE(DataModel::WorkspaceKeys::rebindWidgetRefs(workspaces, lookup), 0);

  const auto& bound = workspaces.front().widgetRefs.front();
  QCOMPARE(bound.relativeIndex, 1);
  QCOMPARE(bound.datasetUniqueId, -1);
}

/**
 * @brief A ref written before datasetUniqueId existed resolves by ordinal once and gains it.
 */
void WorkspaceRebindTest::legacyRefIsBackFilledFromItsOrdinal()
{
  const std::vector<DataModel::Group> groups{makeGroup(10, 100, "A"), makeGroup(20, 200, "B")};

  DataModel::WidgetRef ref;
  ref.widgetType    = static_cast<int>(SerialStudio::DashboardPlot);
  ref.groupUniqueId = 20;
  ref.relativeIndex = 1;
  auto workspaces   = makeWorkspace(ref);

  const auto lookup = DataModel::WorkspaceKeys::buildResolvedWidgetLookup(groups, true);
  QCOMPARE(DataModel::WorkspaceKeys::rebindWidgetRefs(workspaces, lookup), 0);
  QCOMPARE(workspaces.front().widgetRefs.front().datasetUniqueId, 200);
  QCOMPARE(workspaces.front().widgetRefs.front().relativeIndex, 1);
}

/**
 * @brief A ref whose group is gone is counted and left exactly as it was.
 */
void WorkspaceRebindTest::danglingRefIsLeftUntouchedAndCounted()
{
  const std::vector<DataModel::Group> groups{makeGroup(10, 100, "A")};

  DataModel::WidgetRef ref;
  ref.widgetType      = static_cast<int>(SerialStudio::DashboardPlot);
  ref.groupUniqueId   = 99;
  ref.datasetUniqueId = 990;
  ref.relativeIndex   = 7;
  auto workspaces     = makeWorkspace(ref);

  const auto lookup = DataModel::WorkspaceKeys::buildResolvedWidgetLookup(groups, true);
  QCOMPARE(DataModel::WorkspaceKeys::rebindWidgetRefs(workspaces, lookup), 1);
  QCOMPARE(workspaces.front().widgetRefs.front().relativeIndex, 7);
  QCOMPARE(workspaces.front().widgetRefs.front().datasetUniqueId, 990);
}

/**
 * @brief The ordinal an older Serial Studio wrote is what an unchanged project rebinds to.
 */
void WorkspaceRebindTest::savedOrdinalStillResolvesAnUnchangedProject()
{
  const std::vector<DataModel::Group> groups{makeGroup(10, 100, "A"), makeGroup(20, 200, "B")};

  DataModel::WidgetRef ref;
  ref.widgetType      = static_cast<int>(SerialStudio::DashboardPlot);
  ref.groupUniqueId   = 10;
  ref.datasetUniqueId = 100;
  ref.relativeIndex   = 0;
  auto workspaces     = makeWorkspace(ref);

  const auto lookup = DataModel::WorkspaceKeys::buildResolvedWidgetLookup(groups, true);
  QCOMPARE(DataModel::WorkspaceKeys::rebindWidgetRefs(workspaces, lookup), 0);
  QCOMPARE(workspaces.front().widgetRefs.front().relativeIndex, 0);
}

QTEST_APPLESS_MAIN(WorkspaceRebindTest)

#include "tst_workspace_rebind.moc"
