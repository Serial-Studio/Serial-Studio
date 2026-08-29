/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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
#include <QStringList>
#include <QTest>

#include "AI/Tools/ToolCompact.h"

using AI::ToolDetail::compactProjectSnapshotResult;
using AI::ToolDetail::optionSlugForWidget;

/**
 * @brief Pure JSON reductions behind assistant.snapshot and the tile option lookup.
 */
class TstToolCompact : public QObject {
  Q_OBJECT

private slots:
  void optionSlug_data();
  void optionSlug();

  void headerFieldsAreCopied();
  void missingHeaderFieldsDegrade();

  void groupRowIsReduced();
  void datasetRow_data();
  void datasetRow();

  void workspacesPreferLiveList_data();
  void workspacesPreferLiveList();

  void rawSnapshotIsOptIn_data();
  void rawSnapshotIsOptIn();
};

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds one dataset row in the shape project.snapshot emits.
 */
static QJsonObject makeDataset(int datasetId, int uniqueId, const QString& title)
{
  QJsonObject dataset;
  dataset[QStringLiteral("datasetId")] = datasetId;
  dataset[QStringLiteral("uniqueId")]  = uniqueId;
  dataset[QStringLiteral("index")]     = datasetId - 1;
  dataset[QStringLiteral("title")]     = title;
  return dataset;
}

/**
 * @brief Builds a project.snapshot result envelope around the given groups array.
 */
static QJsonObject makeProjectResult(const QJsonArray& groups)
{
  QJsonObject explanations;
  explanations[QStringLiteral("summary")] = QStringLiteral("Two groups, one dataset each.");

  QJsonObject snapshot;
  snapshot[QStringLiteral("title")]         = QStringLiteral("Telemetry");
  snapshot[QStringLiteral("filePath")]      = QStringLiteral("/tmp/telemetry.ssproj");
  snapshot[QStringLiteral("operationMode")] = 2;
  snapshot[QStringLiteral("groupCount")]    = groups.size();
  snapshot[QStringLiteral("datasetCount")]  = 4;
  snapshot[QStringLiteral("_explanations")] = explanations;
  snapshot[QStringLiteral("groups")]        = groups;
  snapshot[QStringLiteral("dataTables")]    = QJsonArray{QStringLiteral("table-a")};
  snapshot[QStringLiteral("workspaces")]    = QJsonArray{QJsonObject{{QStringLiteral("id"), 7}}};
  snapshot[QStringLiteral("verboseNoise")]  = QStringLiteral("dropped by the compactor");

  QJsonObject result;
  result[QStringLiteral("snapshot")]     = snapshot;
  result[QStringLiteral("projectEpoch")] = 42;
  result[QStringLiteral("hint")]         = QStringLiteral("Use assistant.dataset.resolve.");
  return result;
}

/**
 * @brief Builds a single-group project result carrying exactly the given dataset row.
 */
static QJsonObject makeSingleDatasetResult(const QJsonObject& dataset)
{
  QJsonObject group;
  group[QStringLiteral("groupId")]      = 0;
  group[QStringLiteral("title")]        = QStringLiteral("Group");
  group[QStringLiteral("widget")]       = QStringLiteral("none");
  group[QStringLiteral("datasetCount")] = 1;
  group[QStringLiteral("datasets")]     = QJsonArray{dataset};
  return makeProjectResult(QJsonArray{group});
}

/**
 * @brief Returns the first compacted dataset row of a compacted snapshot.
 */
static QJsonObject firstDatasetOf(const QJsonObject& compact)
{
  return compact.value(QStringLiteral("groups"))
    .toArray()
    .first()
    .toObject()
    .value(QStringLiteral("datasets"))
    .toArray()
    .first()
    .toObject();
}

//--------------------------------------------------------------------------------------------------
// Widget option slugs
//--------------------------------------------------------------------------------------------------

void TstToolCompact::optionSlug_data()
{
  QTest::addColumn<QString>("widgetType");
  QTest::addColumn<QString>("slug");

  QTest::newRow("plot") << QStringLiteral("plot") << QStringLiteral("plot");
  QTest::newRow("fft") << QStringLiteral("fft") << QStringLiteral("fft");
  QTest::newRow("bar") << QStringLiteral("bar") << QStringLiteral("bar");
  QTest::newRow("gauge") << QStringLiteral("gauge") << QStringLiteral("gauge");
  QTest::newRow("compass") << QStringLiteral("compass") << QStringLiteral("compass");
  QTest::newRow("led") << QStringLiteral("led") << QStringLiteral("led");
  QTest::newRow("waterfall") << QStringLiteral("waterfall") << QStringLiteral("waterfall");
  QTest::newRow("group widget needs no option") << QStringLiteral("multiplot") << QString();
  QTest::newRow("3d plot needs no option") << QStringLiteral("plot3d") << QString();
  QTest::newRow("datagrid needs no option") << QStringLiteral("datagrid") << QString();
  QTest::newRow("unknown slug") << QStringLiteral("not-a-widget") << QString();
  QTest::newRow("case sensitive") << QStringLiteral("Plot") << QString();
  QTest::newRow("empty") << QString() << QString();
}

/**
 * @brief Only the seven per-dataset widgets need a dataset option enabled before a tile is
 *        pinned; everything else must map to an empty slug so addTile skips the setOptions step.
 */
void TstToolCompact::optionSlug()
{
  QFETCH(QString, widgetType);
  QFETCH(QString, slug);

  QCOMPARE(optionSlugForWidget(widgetType), slug);
}

//--------------------------------------------------------------------------------------------------
// Snapshot header
//--------------------------------------------------------------------------------------------------

/**
 * @brief The compact header lifts projectEpoch and hint from the envelope and the summary out
 *        of the nested _explanations block, and drops every field it does not name.
 */
void TstToolCompact::headerFieldsAreCopied()
{
  const auto compact = compactProjectSnapshotResult(makeProjectResult({}), {}, false);

  QCOMPARE(compact.value(QStringLiteral("title")).toString(), QStringLiteral("Telemetry"));
  QCOMPARE(compact.value(QStringLiteral("filePath")).toString(),
           QStringLiteral("/tmp/telemetry.ssproj"));
  QCOMPARE(compact.value(QStringLiteral("operationMode")).toInt(), 2);
  QCOMPARE(compact.value(QStringLiteral("datasetCount")).toInt(), 4);
  QCOMPARE(compact.value(QStringLiteral("projectEpoch")).toInt(), 42);
  QCOMPARE(compact.value(QStringLiteral("summary")).toString(),
           QStringLiteral("Two groups, one dataset each."));
  QCOMPARE(compact.value(QStringLiteral("hint")).toString(),
           QStringLiteral("Use assistant.dataset.resolve."));
  QCOMPARE(compact.value(QStringLiteral("dataTables")).toArray().size(), 1);
  QVERIFY(!compact.contains(QStringLiteral("verboseNoise")));
  QVERIFY(!compact.contains(QStringLiteral("_explanations")));
}

/**
 * @brief An empty or partial project result must still produce the full compact envelope so
 *        the assistant never has to branch on missing keys.
 */
void TstToolCompact::missingHeaderFieldsDegrade()
{
  const auto compact = compactProjectSnapshotResult({}, {}, false);

  QVERIFY(compact.value(QStringLiteral("title")).toString().isEmpty());
  QCOMPARE(compact.value(QStringLiteral("operationMode")).toInt(), 0);
  QCOMPARE(compact.value(QStringLiteral("groupCount")).toInt(), 0);
  QCOMPARE(compact.value(QStringLiteral("groups")).toArray().size(), 0);
  QCOMPARE(compact.value(QStringLiteral("workspaces")).toArray().size(), 0);
  QVERIFY(compact.contains(QStringLiteral("hint")));
}

//--------------------------------------------------------------------------------------------------
// Group and dataset rows
//--------------------------------------------------------------------------------------------------

/**
 * @brief A group row keeps identity, widget, dataset count and the compatibility slugs the
 *        planner reads; nested datasets are replaced by their compacted form.
 */
void TstToolCompact::groupRowIsReduced()
{
  QJsonObject group;
  group[QStringLiteral("groupId")]                   = 3;
  group[QStringLiteral("title")]                     = QStringLiteral("IMU");
  group[QStringLiteral("widget")]                    = QStringLiteral("accelerometer");
  group[QStringLiteral("datasetCount")]              = 2;
  group[QStringLiteral("compatibleWidgetTypeSlugs")] = QJsonArray{QStringLiteral("plot")};
  group[QStringLiteral("painterCode")]               = QStringLiteral("dropped");
  group[QStringLiteral("datasets")] =
    QJsonArray{makeDataset(1, 11, QStringLiteral("X")), makeDataset(2, 12, QStringLiteral("Y"))};

  const auto result  = makeProjectResult(QJsonArray{group});
  const auto compact = compactProjectSnapshotResult(result, {}, false);
  const auto row     = compact.value(QStringLiteral("groups")).toArray().first().toObject();

  QCOMPARE(row.value(QStringLiteral("groupId")).toInt(), 3);
  QCOMPARE(row.value(QStringLiteral("title")).toString(), QStringLiteral("IMU"));
  QCOMPARE(row.value(QStringLiteral("widget")).toString(), QStringLiteral("accelerometer"));
  QCOMPARE(row.value(QStringLiteral("datasetCount")).toInt(), 2);
  QCOMPARE(row.value(QStringLiteral("compatibleWidgetTypeSlugs")).toArray().size(), 1);
  QCOMPARE(row.value(QStringLiteral("datasets")).toArray().size(), 2);
  QVERIFY(!row.contains(QStringLiteral("painterCode")));
}

void TstToolCompact::datasetRow_data()
{
  QTest::addColumn<QJsonObject>("dataset");
  QTest::addColumn<QStringList>("present");
  QTest::addColumn<QStringList>("absent");

  const QStringList identity{QStringLiteral("datasetId"),
                             QStringLiteral("uniqueId"),
                             QStringLiteral("index"),
                             QStringLiteral("title")};

  const auto base = makeDataset(1, 91, QStringLiteral("Voltage"));
  QStringList optional{QStringLiteral("units"),
                       QStringLiteral("enabledOptionsSlugs"),
                       QStringLiteral("hasTransform"),
                       QStringLiteral("isVirtual")};
  QTest::newRow("identity only") << base << identity << optional;

  auto withUnits                         = base;
  withUnits[QStringLiteral("units")]     = QStringLiteral("V");
  withUnits[QStringLiteral("alarmHigh")] = 12;
  QStringList withUnitsPresent           = identity;
  withUnitsPresent.append(QStringLiteral("units"));
  QTest::newRow("units kept, unrelated field dropped")
    << withUnits << withUnitsPresent << QStringList{QStringLiteral("alarmHigh")};

  auto withOptions                                   = base;
  withOptions[QStringLiteral("enabledOptionsSlugs")] = QJsonArray{QStringLiteral("plot")};
  QStringList withOptionsPresent                     = identity;
  withOptionsPresent.append(QStringLiteral("enabledOptionsSlugs"));
  QTest::newRow("enabled options kept")
    << withOptions << withOptionsPresent << QStringList{QStringLiteral("hasTransform")};

  auto flagsOff                            = base;
  flagsOff[QStringLiteral("hasTransform")] = false;
  flagsOff[QStringLiteral("isVirtual")]    = false;
  QTest::newRow("false flags are omitted")
    << flagsOff << identity
    << QStringList{QStringLiteral("hasTransform"), QStringLiteral("isVirtual")};

  auto flagsOn                            = base;
  flagsOn[QStringLiteral("hasTransform")] = true;
  flagsOn[QStringLiteral("isVirtual")]    = true;
  QStringList flagsOnPresent              = identity;
  flagsOnPresent.append(QStringLiteral("hasTransform"));
  flagsOnPresent.append(QStringLiteral("isVirtual"));
  QTest::newRow("true flags are kept")
    << flagsOn << flagsOnPresent << QStringList{QStringLiteral("units")};
}

/**
 * @brief Identity fields are unconditional, optional metadata is copied only when the source
 *        carries it, and the two booleans are emitted only when true: a false flag that made it
 *        through would cost tokens on every dataset of every snapshot.
 */
void TstToolCompact::datasetRow()
{
  QFETCH(QJsonObject, dataset);
  QFETCH(QStringList, present);
  QFETCH(QStringList, absent);

  const auto compact = compactProjectSnapshotResult(makeSingleDatasetResult(dataset), {}, false);
  const auto row     = firstDatasetOf(compact);

  for (const auto& key : present)
    QVERIFY2(row.contains(key), qPrintable(key));

  for (const auto& key : absent)
    QVERIFY2(!row.contains(key), qPrintable(key));

  QCOMPARE(row.value(QStringLiteral("datasetId")).toInt(), 1);
  QCOMPARE(row.value(QStringLiteral("uniqueId")).toInt(), 91);
  QCOMPARE(row.value(QStringLiteral("title")).toString(), QStringLiteral("Voltage"));
}

//--------------------------------------------------------------------------------------------------
// Workspaces and raw payload
//--------------------------------------------------------------------------------------------------

void TstToolCompact::workspacesPreferLiveList_data()
{
  QTest::addColumn<QJsonObject>("workspaceResult");
  QTest::addColumn<int>("expectedId");

  QTest::newRow("empty workspace reply falls back to the snapshot") << QJsonObject{} << 7;
  QTest::newRow("empty workspaces array falls back to the snapshot") << QJsonObject{
    {QStringLiteral("workspaces"), QJsonArray{}}
  } << 7;
  QTest::newRow("live workspace list wins") << QJsonObject{
    {QStringLiteral("workspaces"),
     QJsonArray{QJsonObject{
       {QStringLiteral("id"), 9}}}}
  } << 9;
}

/**
 * @brief project.workspace.list is the live source of truth; the snapshot copy is only the
 *        fallback for when that call returned nothing.
 */
void TstToolCompact::workspacesPreferLiveList()
{
  QFETCH(QJsonObject, workspaceResult);
  QFETCH(int, expectedId);

  const auto compact = compactProjectSnapshotResult(makeProjectResult({}), workspaceResult, false);
  const auto rows    = compact.value(QStringLiteral("workspaces")).toArray();

  QCOMPARE(rows.size(), 1);
  QCOMPARE(rows.first().toObject().value(QStringLiteral("id")).toInt(), expectedId);
}

void TstToolCompact::rawSnapshotIsOptIn_data()
{
  QTest::addColumn<bool>("includeRaw");
  QTest::addColumn<bool>("expected");

  QTest::newRow("compact by default") << false << false;
  QTest::newRow("verbose attaches the raw result") << true << true;
}

/**
 * @brief The whole point of the compactor is dropping the raw snapshot, so it comes back only
 *        when the caller explicitly asked for verbose output.
 */
void TstToolCompact::rawSnapshotIsOptIn()
{
  QFETCH(bool, includeRaw);
  QFETCH(bool, expected);

  const auto compact = compactProjectSnapshotResult(makeProjectResult({}), {}, includeRaw);

  QCOMPARE(compact.contains(QStringLiteral("rawProjectSnapshot")), expected);
  QVERIFY(compact.contains(QStringLiteral("title")));
}

QTEST_APPLESS_MAIN(TstToolCompact)

#include "tst_tool_compact.moc"
