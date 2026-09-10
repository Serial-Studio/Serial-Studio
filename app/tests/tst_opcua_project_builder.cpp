/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QTest>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "IO/Drivers/OpcUa/OpcUaProjectBuilder.h"

// Every test function builds its own tag list: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

using IO::Drivers::OpcUaProjectBuilder;
using IO::Drivers::OpcUaTag;

/**
 * @brief Known-answer tests for the OPC UA tag-list to project translation.
 */
class TstOpcUaProjectBuilder : public QObject {
  Q_OBJECT

private slots:
  void wireSchemaIsOneEntryPerElement();
  void wireSchemaStopsAtTheSlotCeiling();
  void booleanTagsBecomeLedDatasets();
  void numericTagsBecomePlotsWithRanges();
  void arrayElementsCarryIndexedTitles();
  void projectSkeleton();
  void connectionSettingsAreStored();
  void tagPathsBecomeDataGrids();
  void datasetIndicesRunAcrossGroups();
  void emptyTagListStillBuilds();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds one tag; the array length, unit and range default to a scalar with no range.
 */
[[nodiscard]] static OpcUaTag makeTag(const QString& nodeId,
                                      const QString& name,
                                      const IO::Drivers::OpcUaWire::Type type,
                                      const QString& path = QString(),
                                      const int arrayLen  = 1)
{
  OpcUaTag tag;
  tag.nodeId   = nodeId;
  tag.name     = name;
  tag.path     = path;
  tag.type     = type;
  tag.arrayLen = arrayLen;
  return tag;
}

/**
 * @brief Returns the groups array of a generated project.
 */
[[nodiscard]] static QJsonArray projectGroups(const QJsonObject& project)
{
  return project.value(Keys::Groups).toArray();
}

/**
 * @brief Returns the single source object of a generated project.
 */
[[nodiscard]] static QJsonObject projectSource(const QJsonObject& project)
{
  return project.value(Keys::Sources).toArray().at(0).toObject();
}

/**
 * @brief Returns the datasets of one group of a generated project.
 */
[[nodiscard]] static QJsonArray groupDatasets(const QJsonObject& project, const int group)
{
  return projectGroups(project).at(group).toObject().value(Keys::Datasets).toArray();
}

//--------------------------------------------------------------------------------------------------
// Wire schema
//--------------------------------------------------------------------------------------------------

/**
 * @brief One {i, t, id} entry per wire index, array tags expanding into consecutive indices.
 */
void TstOpcUaProjectBuilder::wireSchemaIsOneEntryPerElement()
{
  const QList<OpcUaTag> tags = {
    makeTag("ns=2;i=1", "Speed", IO::Drivers::OpcUaWire::Type::F64),
    makeTag("ns=2;i=2", "Axes", IO::Drivers::OpcUaWire::Type::I16, QString(), 3),
  };

  const auto schema = OpcUaProjectBuilder(tags).wireSchema();
  QCOMPARE(schema.count(), 4);
  QCOMPARE(schema.at(0).toObject().value(QStringLiteral("i")).toInt(), 0);
  QCOMPARE(schema.at(0).toObject().value(QStringLiteral("t")).toString(), QStringLiteral("f64"));
  QCOMPARE(schema.at(0).toObject().value(QStringLiteral("id")).toString(),
           QStringLiteral("ns=2;i=1"));
  QCOMPARE(schema.at(3).toObject().value(QStringLiteral("i")).toInt(), 3);
  QCOMPARE(schema.at(3).toObject().value(QStringLiteral("t")).toString(), QStringLiteral("i16"));
  QCOMPARE(schema.at(3).toObject().value(QStringLiteral("id")).toString(),
           QStringLiteral("ns=2;i=2"));
}

/**
 * @brief A tag list wider than the slot ceiling is truncated rather than overrunning the decoder.
 */
void TstOpcUaProjectBuilder::wireSchemaStopsAtTheSlotCeiling()
{
  const QList<OpcUaTag> tags = {
    makeTag("ns=2;i=1",
            "Wide",
            IO::Drivers::OpcUaWire::Type::U8,
            QString(),
            IO::Drivers::OpcUaWire::kMaxTags + 16),
  };

  QCOMPARE(OpcUaProjectBuilder(tags).wireSchema().count(), IO::Drivers::OpcUaWire::kMaxTags);
}

//--------------------------------------------------------------------------------------------------
// Dataset translation
//--------------------------------------------------------------------------------------------------

/**
 * @brief A boolean tag becomes an LED dataset, never a plot.
 */
void TstOpcUaProjectBuilder::booleanTagsBecomeLedDatasets()
{
  const auto tag = makeTag("ns=2;i=7", "Running", IO::Drivers::OpcUaWire::Type::Bool);
  const auto set = OpcUaProjectBuilder::datasetFor(tag, 0, 1);

  QCOMPARE(set.index, 1);
  QVERIFY(set.led);
  QVERIFY(!set.plt);
  QCOMPARE(set.ledHigh, 1.0);
  QCOMPARE(set.wgtMax, 1.0);
  QCOMPARE(set.title, QStringLiteral("Running"));
}

/**
 * @brief A numeric tag becomes a plot, and a declared range reaches both the widget and the plot.
 */
void TstOpcUaProjectBuilder::numericTagsBecomePlotsWithRanges()
{
  auto tag       = makeTag("ns=2;i=8", "Torque", IO::Drivers::OpcUaWire::Type::F32);
  tag.unit       = QStringLiteral("Nm");
  tag.min        = -5;
  tag.max        = 25;
  const auto set = OpcUaProjectBuilder::datasetFor(tag, 0, 4);

  QVERIFY(set.plt);
  QVERIFY(!set.led);
  QCOMPARE(set.units, QStringLiteral("Nm"));
  QCOMPARE(set.wgtMin, -5.0);
  QCOMPARE(set.wgtMax, 25.0);
  QCOMPARE(set.pltMin, -5.0);
  QCOMPARE(set.pltMax, 25.0);

  const auto plain = OpcUaProjectBuilder::datasetFor(
    makeTag("ns=2;i=9", "Text", IO::Drivers::OpcUaWire::Type::Str), 0, 5);
  QVERIFY(!plain.plt);
  QVERIFY(!plain.led);
}

/**
 * @brief Only an array tag gets an element suffix; a scalar keeps its bare name.
 */
void TstOpcUaProjectBuilder::arrayElementsCarryIndexedTitles()
{
  const auto array = makeTag("ns=2;i=3", "Axis", IO::Drivers::OpcUaWire::Type::I32, QString(), 3);
  QCOMPARE(OpcUaProjectBuilder::datasetFor(array, 2, 3).title, QStringLiteral("Axis[2]"));

  const auto scalar = makeTag("ns=2;i=4", "Axis", IO::Drivers::OpcUaWire::Type::I32);
  QCOMPARE(OpcUaProjectBuilder::datasetFor(scalar, 0, 1).title, QStringLiteral("Axis"));
}

//--------------------------------------------------------------------------------------------------
// Project structure
//--------------------------------------------------------------------------------------------------

/**
 * @brief The generated source is a delimiter-less binary OPC UA source parsed by the native
 *        `opcua` template, whose parameters carry the wire schema.
 */
void TstOpcUaProjectBuilder::projectSkeleton()
{
  const QList<OpcUaTag> tags = {makeTag("ns=2;i=1", "Speed", IO::Drivers::OpcUaWire::Type::F64)};
  const auto project         = OpcUaProjectBuilder(tags).buildProject({});
  const auto source          = projectSource(project);

  QCOMPARE(project.value(Keys::Sources).toArray().count(), 1);
  QCOMPARE(source.value(Keys::SourceId).toInt(), 0);
  QCOMPARE(source.value(Keys::BusType).toInt(), static_cast<int>(SerialStudio::BusType::OpcUa));
  QCOMPARE(source.value(Keys::Decoder).toInt(), static_cast<int>(SerialStudio::Binary));
  QCOMPARE(source.value(Keys::FrameDetection).toInt(),
           static_cast<int>(SerialStudio::NoDelimiters));
  QCOMPARE(source.value(Keys::FrameParserLanguage).toInt(), static_cast<int>(SerialStudio::Native));
  QCOMPARE(source.value(Keys::FrameParserTemplate).toString(), QStringLiteral("opcua"));
  QVERIFY(source.value(Keys::FrameStart).toString().isEmpty());
  QVERIFY(source.value(Keys::FrameEnd).toString().isEmpty());
  QVERIFY(source.value(Keys::FrameParserCode).toString().isEmpty());

  const auto params = source.value(Keys::FrameParserParams).toObject();
  QCOMPARE(params.value(QStringLiteral("schema")).toArray().count(), 1);
}

/**
 * @brief The connection settings handed in are stored verbatim on the source.
 */
void TstOpcUaProjectBuilder::connectionSettingsAreStored()
{
  const QJsonObject connection{
    {QStringLiteral("endpointUrl"), QStringLiteral("opc.tcp://plc.local:4840")},
    {   QStringLiteral("authMode"),                                          1},
  };

  const QList<OpcUaTag> tags = {makeTag("ns=2;i=1", "Speed", IO::Drivers::OpcUaWire::Type::F64)};
  const auto stored          = projectSource(OpcUaProjectBuilder(tags).buildProject(connection))
                        .value(Keys::SourceConn)
                        .toObject();

  QCOMPARE(stored.value(QStringLiteral("endpointUrl")).toString(),
           QStringLiteral("opc.tcp://plc.local:4840"));
  QCOMPARE(stored.value(QStringLiteral("authMode")).toInt(), 1);
}

/**
 * @brief One data-grid group per parent folder, titled with the last path segment, in the order
 *        the tags were selected.
 */
void TstOpcUaProjectBuilder::tagPathsBecomeDataGrids()
{
  const QList<OpcUaTag> tags = {
    makeTag("ns=2;i=1", "Speed", IO::Drivers::OpcUaWire::Type::F64, "Plant/Motor"),
    makeTag("ns=2;i=2", "Torque", IO::Drivers::OpcUaWire::Type::F64, "Plant/Motor"),
    makeTag("ns=2;i=3", "Level", IO::Drivers::OpcUaWire::Type::F64, "Plant/Tank"),
  };

  const auto groups = projectGroups(OpcUaProjectBuilder(tags).buildProject({}));
  QCOMPARE(groups.count(), 2);
  QCOMPARE(groups.at(0).toObject().value(Keys::Title).toString(), QStringLiteral("Motor"));
  QCOMPARE(groups.at(0).toObject().value(Keys::Widget).toString(), QStringLiteral("datagrid"));
  QCOMPARE(groups.at(1).toObject().value(Keys::Title).toString(), QStringLiteral("Tank"));
}

/**
 * @brief Dataset indices are the wire indices: they run across every group, one-based.
 */
void TstOpcUaProjectBuilder::datasetIndicesRunAcrossGroups()
{
  const QList<OpcUaTag> tags = {
    makeTag("ns=2;i=1", "Speed", IO::Drivers::OpcUaWire::Type::F64, "Plant/Motor"),
    makeTag("ns=2;i=2", "Axes", IO::Drivers::OpcUaWire::Type::I16, "Plant/Motor", 2),
    makeTag("ns=2;i=3", "Level", IO::Drivers::OpcUaWire::Type::F64, "Plant/Tank"),
  };

  const auto project = OpcUaProjectBuilder(tags).buildProject({});
  const auto motor   = groupDatasets(project, 0);
  const auto tank    = groupDatasets(project, 1);

  QCOMPARE(motor.count(), 3);
  QCOMPARE(tank.count(), 1);
  QCOMPARE(motor.at(0).toObject().value(Keys::Index).toInt(), 1);
  QCOMPARE(motor.at(2).toObject().value(Keys::Index).toInt(), 3);
  QCOMPARE(tank.at(0).toObject().value(Keys::Index).toInt(), 4);
}

/**
 * @brief An empty tag list still produces a loadable project with the source and no groups.
 */
void TstOpcUaProjectBuilder::emptyTagListStillBuilds()
{
  const auto project = OpcUaProjectBuilder({}).buildProject({});

  QVERIFY(projectGroups(project).isEmpty());
  QCOMPARE(project.value(Keys::Sources).toArray().count(), 1);
  QVERIFY(OpcUaProjectBuilder({}).wireSchema().isEmpty());
}

QTEST_MAIN(TstOpcUaProjectBuilder)

#include "tst_opcua_project_builder.moc"
