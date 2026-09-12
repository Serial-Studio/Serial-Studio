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
#include <QTest>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "DataModel/Project/ProjectMerge.h"

// "Add to current project" (spec 0083): an importer's standalone project is appended to an open
// document. Every id it carries must land on free ground -- source ids, group and dataset
// uniqueIds, workspace ids -- and a table whose name the document already uses is renamed in the
// generated parser and transforms too, or the imported code would read the wrong table.

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief An importer-shaped project: one source, one group with one table-fed dataset, the table
 *        the parser writes, and an Overview workspace pointing at the group's widget.
 */
static QJsonObject importedProject()
{
  DataModel::Source source;
  source.sourceId        = 0;
  source.title           = QStringLiteral("Modbus");
  source.frameParserCode = QStringLiteral("tableSet(\"Holding Registers\", \"x\", 1)");

  DataModel::Dataset dataset;
  dataset.title         = QStringLiteral("x");
  dataset.virtual_      = true;
  dataset.transformCode = QStringLiteral("return tableGet(\"Holding Registers\", \"x\")");

  DataModel::Group group;
  group.uniqueId = 10000;
  group.title    = QStringLiteral("Holding Registers");
  group.widget   = QStringLiteral("datagrid");
  group.datasets.push_back(dataset);

  DataModel::TableDef table;
  table.name = QStringLiteral("Holding Registers");

  DataModel::WidgetRef ref;
  ref.widgetType    = static_cast<int>(SerialStudio::DashboardDataGrid);
  ref.groupUniqueId = 10000;
  ref.relativeIndex = 0;

  DataModel::Workspace overview;
  overview.workspaceId = WorkspaceIds::UserStart;
  overview.title       = QStringLiteral("Overview");
  overview.widgetRefs.push_back(ref);

  QJsonObject project;
  project.insert(Keys::Sources, QJsonArray{DataModel::serialize(source)});
  project.insert(Keys::Groups, QJsonArray{DataModel::serialize(group)});
  project.insert(Keys::Tables, QJsonArray{DataModel::serialize(table)});
  project.insert(Keys::Workspaces, QJsonArray{DataModel::serialize(overview)});
  return project;
}

/**
 * @brief A document already holding one source, two groups, the same table name and an Overview.
 */
static DataModel::ProjectMerge::Base occupiedBase()
{
  DataModel::ProjectMerge::Base base;
  base.groupCount      = 2;
  base.nextSourceId    = 1;
  base.nextUniqueId    = 50;
  base.nextWorkspaceId = WorkspaceIds::UserStart + 2;
  base.tableNames.insert(QStringLiteral("Holding Registers"));
  base.workspaceTitles.insert(QStringLiteral("Overview"));
  return base;
}

//--------------------------------------------------------------------------------------------------
// Test suite
//--------------------------------------------------------------------------------------------------

class ProjectMergeTest : public QObject {
  Q_OBJECT

private slots:
  void sourcesAndEntitiesLandOnFreeIds();
  void collidingTableIsRenamedInCodeToo();
  void workspacesTakeFreeIdsAndKeepIdentityOnly();
  void freeNameLeavesAnUnusedNameAlone();
};

/**
 * @brief The source moves past the document's, group and datasets draw fresh uniqueIds in order,
 *        and positional ids continue the document's group list.
 */
void ProjectMergeTest::sourcesAndEntitiesLandOnFreeIds()
{
  const auto merged =
    DataModel::ProjectMerge::remap(importedProject(), occupiedBase(), QStringLiteral("LoadBank"));

  QCOMPARE(merged.sources.size(), size_t(1));
  QCOMPARE(merged.sources.front().sourceId, 1);

  QCOMPARE(merged.groups.size(), size_t(1));
  const auto& group = merged.groups.front();
  QCOMPARE(group.uniqueId, 50);
  QCOMPARE(group.groupId, 2);
  QCOMPARE(group.sourceId, 1);
  QCOMPARE(group.datasets.front().uniqueId, 51);
  QCOMPARE(group.datasets.front().groupId, 2);
  QCOMPARE(group.datasets.front().sourceId, 1);
  QCOMPARE(merged.nextUniqueId, 52);
}

/**
 * @brief A table the document already owns is renamed, and every quoted reference in the imported
 *        parser and transform code follows it.
 */
void ProjectMergeTest::collidingTableIsRenamedInCodeToo()
{
  const auto merged =
    DataModel::ProjectMerge::remap(importedProject(), occupiedBase(), QStringLiteral("LoadBank"));

  QCOMPARE(merged.tables.size(), size_t(1));
  QCOMPARE(merged.tables.front().name, QStringLiteral("Holding Registers (LoadBank)"));
  QCOMPARE(merged.renamedTables.value(QStringLiteral("Holding Registers")),
           QStringLiteral("Holding Registers (LoadBank)"));
  QVERIFY(merged.sources.front().frameParserCode.contains(
    QStringLiteral("tableSet(\"Holding Registers (LoadBank)\"")));
  QVERIFY(merged.groups.front().datasets.front().transformCode.contains(
    QStringLiteral("tableGet(\"Holding Registers (LoadBank)\"")));
}

/**
 * @brief Workspaces continue the document's id range, a colliding title is suffixed, and a ref
 *        keeps only its (remapped) identity: the ordinal is rebound once the groups are in place.
 */
void ProjectMergeTest::workspacesTakeFreeIdsAndKeepIdentityOnly()
{
  const auto merged =
    DataModel::ProjectMerge::remap(importedProject(), occupiedBase(), QStringLiteral("LoadBank"));

  QCOMPARE(merged.workspaces.size(), size_t(1));
  const auto& workspace = merged.workspaces.front();
  QCOMPARE(workspace.workspaceId, WorkspaceIds::UserStart + 2);
  QCOMPARE(workspace.title, QStringLiteral("Overview (LoadBank)"));
  QCOMPARE(workspace.widgetRefs.front().groupUniqueId, 50);
  QCOMPARE(workspace.widgetRefs.front().relativeIndex, -1);
}

/**
 * @brief freeName returns the wanted name untouched when nothing holds it, and numbers the suffix
 *        once the suffixed form is taken as well.
 */
void ProjectMergeTest::freeNameLeavesAnUnusedNameAlone()
{
  QSet<QString> taken{QStringLiteral("A"), QStringLiteral("A (L)")};
  QCOMPARE(DataModel::ProjectMerge::freeName(QStringLiteral("B"), taken, QStringLiteral("L")),
           QStringLiteral("B"));
  QCOMPARE(DataModel::ProjectMerge::freeName(QStringLiteral("A"), taken, QStringLiteral("L")),
           QStringLiteral("A (L 2)"));
}

QTEST_APPLESS_MAIN(ProjectMergeTest)

#include "tst_project_merge.moc"
