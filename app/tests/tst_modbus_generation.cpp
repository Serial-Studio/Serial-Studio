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
#include <QTest>

#include "DataModel/Frame.h"
#include "IO/Drivers/Modbus/ModbusProjectGenerator.h"
#include "SerialStudio.h"

// Every test function builds its own groups: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

using IO::Drivers::ModbusProjectGenerator;
using IO::Drivers::ModbusRegisterGroup;

/**
 * @brief Known-answer tests for the Modbus register-group to project translation.
 */
class TstModbusGeneration : public QObject {
  Q_OBJECT

private slots:
  void datasetTotals();
  void projectSkeleton();
  void connectionSettingsAreStored();
  void registerGroupsBecomeDataGrids();
  void coilGroupsBecomeLedDatasets();
  void datasetIndicesRunAcrossGroups();
  void parserDecodesRegistersAsBigEndianPairs();
  void parserDecodesCoilsAsBits();
  void parserCyclesThroughEveryGroup();
  void emptyConfigurationStillParses();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the groups array of a generated project.
 */
[[nodiscard]] static QJsonArray projectGroups(const QJsonObject& project)
{
  return project.value(QStringLiteral("groups")).toArray();
}

/**
 * @brief Returns the single source object of a generated project.
 */
[[nodiscard]] static QJsonObject projectSource(const QJsonObject& project)
{
  return project.value(Keys::Sources).toArray().at(0).toObject();
}

//--------------------------------------------------------------------------------------------------
// Project structure
//--------------------------------------------------------------------------------------------------

/**
 * @brief One dataset per register or bit, summed over every group.
 */
void TstModbusGeneration::datasetTotals()
{
  const QVector<ModbusRegisterGroup> groups = {
    ModbusRegisterGroup(0, 0, 4),
    ModbusRegisterGroup(2, 100, 9),
  };

  QCOMPARE(ModbusProjectGenerator(groups).totalDatasets(), 13);
  QCOMPARE(ModbusProjectGenerator({}).totalDatasets(), 0);
}

/**
 * @brief The generated source is a delimiter-less binary Modbus source parsed by Lua.
 */
void TstModbusGeneration::projectSkeleton()
{
  const QVector<ModbusRegisterGroup> groups = {ModbusRegisterGroup(0, 0, 1)};
  const auto project                        = ModbusProjectGenerator(groups).buildProject({});
  const auto source                         = projectSource(project);

  QCOMPARE(project.value(Keys::Sources).toArray().count(), 1);
  QCOMPARE(source.value(Keys::SourceId).toInt(), 0);
  QCOMPARE(source.value(Keys::BusType).toInt(), static_cast<int>(SerialStudio::BusType::ModBus));
  QCOMPARE(source.value(Keys::Decoder).toInt(), static_cast<int>(SerialStudio::Binary));
  QCOMPARE(source.value(Keys::FrameDetection).toInt(),
           static_cast<int>(SerialStudio::NoDelimiters));
  QCOMPARE(source.value(Keys::FrameParserLanguage).toInt(), static_cast<int>(SerialStudio::Lua));
  QVERIFY(source.value(Keys::FrameStart).toString().isEmpty());
  QVERIFY(source.value(Keys::FrameEnd).toString().isEmpty());
  QVERIFY(!project.value(Keys::Title).toString().isEmpty());
}

/**
 * @brief The driver's connection settings travel into the project verbatim, so the generated
 *        project reconnects to the device the groups were polled from.
 */
void TstModbusGeneration::connectionSettingsAreStored()
{
  QJsonObject connection;
  connection.insert(QStringLiteral("host"), QStringLiteral("192.168.0.10"));
  connection.insert(QStringLiteral("port"), 502);

  const QVector<ModbusRegisterGroup> groups = {ModbusRegisterGroup(1, 7, 2)};
  const auto source = projectSource(ModbusProjectGenerator(groups).buildProject(connection));

  QCOMPARE(source.value(Keys::SourceConn).toObject(), connection);
}

/**
 * @brief A holding/input-register group becomes one data grid of plottable 16-bit datasets.
 */
void TstModbusGeneration::registerGroupsBecomeDataGrids()
{
  const QVector<ModbusRegisterGroup> groups = {ModbusRegisterGroup(0, 40, 3)};
  const auto array = projectGroups(ModbusProjectGenerator(groups).buildProject({}));

  QCOMPARE(array.count(), 1);

  const auto group = array.at(0).toObject();
  QCOMPARE(group.value(Keys::Widget).toString(), QStringLiteral("datagrid"));
  QVERIFY(group.value(Keys::Title).toString().contains(QStringLiteral("40")));

  const auto datasets = group.value(Keys::Datasets).toArray();
  QCOMPARE(datasets.count(), 3);

  const auto dataset = datasets.at(0).toObject();
  QCOMPARE(dataset.value(Keys::Graph).toBool(), true);
  QCOMPARE(dataset.value(Keys::WgtMax).toVariant().toInt(), 65535);
  QCOMPARE(dataset.value(Keys::Log).toBool(), true);
}

/**
 * @brief Coils and discrete inputs become LED datasets bounded to a single bit.
 */
void TstModbusGeneration::coilGroupsBecomeLedDatasets()
{
  const QVector<ModbusRegisterGroup> groups = {ModbusRegisterGroup(2, 0, 2)};
  const auto array    = projectGroups(ModbusProjectGenerator(groups).buildProject({}));
  const auto datasets = array.at(0).toObject().value(Keys::Datasets).toArray();

  QCOMPARE(datasets.count(), 2);

  const auto dataset = datasets.at(0).toObject();
  QCOMPARE(dataset.value(Keys::LED).toBool(), true);
  QCOMPARE(dataset.value(Keys::WgtMax).toVariant().toInt(), 1);
  QVERIFY(!dataset.value(Keys::Graph).toBool());
}

/**
 * @brief Dataset indices are one-based and keep counting across groups: the parser writes into a
 *        single flat value table shared by every group.
 */
void TstModbusGeneration::datasetIndicesRunAcrossGroups()
{
  const QVector<ModbusRegisterGroup> groups = {
    ModbusRegisterGroup(0, 0, 2),
    ModbusRegisterGroup(1, 10, 2),
  };

  const auto array = projectGroups(ModbusProjectGenerator(groups).buildProject({}));
  QCOMPARE(array.count(), 2);

  QList<int> indices;
  for (const auto& groupValue : array) {
    const auto datasets = groupValue.toObject().value(Keys::Datasets).toArray();
    for (const auto& datasetValue : datasets)
      indices.append(datasetValue.toObject().value(Keys::Index).toInt());
  }

  QCOMPARE(indices, QList<int>({1, 2, 3, 4}));
}

//--------------------------------------------------------------------------------------------------
// Generated parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register payloads are decoded big-endian, two payload bytes per dataset.
 */
void TstModbusGeneration::parserDecodesRegistersAsBigEndianPairs()
{
  const QVector<ModbusRegisterGroup> groups = {ModbusRegisterGroup(0, 0, 2)};
  const auto code                           = ModbusProjectGenerator(groups).buildFrameParser();

  QVERIFY(code.contains(QStringLiteral("values[1] = (data[1] << 8) | data[2]")));
  QVERIFY(code.contains(QStringLiteral("values[2] = (data[3] << 8) | data[4]")));
  QVERIFY(code.contains(QStringLiteral("for i = 1, 2 do values[i] = 0 end")));
}

/**
 * @brief Coil payloads are decoded as packed bits, eight per payload byte.
 */
void TstModbusGeneration::parserDecodesCoilsAsBits()
{
  const QVector<ModbusRegisterGroup> groups = {ModbusRegisterGroup(3, 0, 9)};
  const auto code                           = ModbusProjectGenerator(groups).buildFrameParser();

  QVERIFY(code.contains(QStringLiteral("values[1] = (data[1] >> 0) & 1")));
  QVERIFY(code.contains(QStringLiteral("values[8] = (data[1] >> 7) & 1")));
  QVERIFY(code.contains(QStringLiteral("values[9] = (data[2] >> 0) & 1")));
}

/**
 * @brief The parser owns the poll cycle: one branch per group, and the counter wraps at the group
 *        count, because every reply carries the same header and no group id.
 */
void TstModbusGeneration::parserCyclesThroughEveryGroup()
{
  const QVector<ModbusRegisterGroup> groups = {
    ModbusRegisterGroup(0, 0, 1),
    ModbusRegisterGroup(1, 5, 1),
    ModbusRegisterGroup(2, 9, 1),
  };

  const auto code = ModbusProjectGenerator(groups).buildFrameParser();

  QVERIFY(code.contains(QStringLiteral("if currentGroup == 0 then")));
  QVERIFY(code.contains(QStringLiteral("elseif currentGroup == 1 then")));
  QVERIFY(code.contains(QStringLiteral("elseif currentGroup == 2 then")));
  QVERIFY(code.contains(QStringLiteral("currentGroup = (currentGroup + 1) % 3")));
  QCOMPARE(code.count(QStringLiteral("elseif currentGroup ==")), 2);
}

/**
 * @brief A generator with no groups still emits a complete parse() function; the driver refuses
 *        the empty configuration before it gets here, and the parser must not be half-written if
 *        something ever calls it anyway.
 */
void TstModbusGeneration::emptyConfigurationStillParses()
{
  const auto code = ModbusProjectGenerator({}).buildFrameParser();

  QVERIFY(code.contains(QStringLiteral("function parse(frame)")));
  QVERIFY(code.contains(QStringLiteral("return values")));
  QVERIFY(!code.contains(QStringLiteral("currentGroup == 0")));
  QVERIFY(projectGroups(ModbusProjectGenerator({}).buildProject({})).isEmpty());
}

QTEST_APPLESS_MAIN(TstModbusGeneration)

#include "tst_modbus_generation.moc"
