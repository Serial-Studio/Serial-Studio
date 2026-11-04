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

#include <QFile>
#include <QJsonArray>
#include <QTest>

#include "Core/DataModel/FrameKeys.h"
#include "DataModel/Project/DeviceDeclaration.h"

// Device declarations (spec 0111 AC2): refused and stripped lines, the once-per-content rebuild,
// the rebuild hold, index reuse, the wire sanitizer and the golden fixtures.

using Outcome   = DataModel::DeclarationOutcome;
using TimePoint = DataModel::DeclarationState::TimePoint;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The first line of a golden declaration fixture.
 */
[[nodiscard]] static QByteArray golden(const QString& name)
{
  QFile file(QStringLiteral(SS_DECLARATION_GOLDEN_DIR "/") + name);
  if (!file.open(QIODevice::ReadOnly))
    return {};

  return file.readLine().trimmed();
}

/**
 * @brief A one-group declaration line wrapping @p datasets (a JSON array body).
 */
[[nodiscard]] static QByteArray oneGroup(const QByteArray& datasets)
{
  return "#ss1{\"title\":\"Probe\",\"groups\":[{\"title\":\"G\",\"datasets\":[" + datasets + "]}]}";
}

/**
 * @brief A steady-clock instant @p ms after an arbitrary origin.
 */
[[nodiscard]] static TimePoint at(const int ms)
{
  return TimePoint() + std::chrono::milliseconds(ms);
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class DeviceDeclarationTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void goldenMpu6050();
  void goldenTinyGps();
  void goldenHexAdcSharesChannels();
  void goldenControlsDropsOutputGroup();
  void refusedLines_data();
  void refusedLines();
  void codeAndPathsAreStripped();
  void sanitizerBoundsValues();
  void indexPastNextPositionIsRefused();
  void sameLineRebuildsOnce();
  void changedLineWithinASecondIsHeld();
  void refusedLineIsReportedOnce();
  void resetForgetsTheDeclaration();
  void projectionMatchesALoadedProject();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void DeviceDeclarationTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief The MPU6050 line yields the example project's three groups, roles, units and ranges.
 */
void DeviceDeclarationTest::goldenMpu6050()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("mpu6050.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.title, QStringLiteral("MPU6050"));
  QCOMPARE(d.channels, 7);
  QCOMPARE(d.groups.size(), size_t(3));
  QCOMPARE(d.groups[0].widget, QStringLiteral("accelerometer"));
  QCOMPARE(d.groups[1].widget, QStringLiteral("gyro"));
  QVERIFY(d.groups[2].widget.isEmpty());

  const auto& accel = d.groups[0].datasets;
  QCOMPARE(accel.size(), size_t(3));
  QCOMPARE(accel[0].title, QStringLiteral("Accelerometer X"));
  QCOMPARE(accel[0].units, QString::fromUtf8("m/s\xC2\xB2"));
  QCOMPARE(accel[2].widget, QStringLiteral("z"));
  QVERIFY(accel[1].plt);
  QCOMPARE(d.groups[1].datasets[2].index, 6);

  const auto& temp = d.groups[2].datasets.front();
  QCOMPARE(temp.index, 7);
  QCOMPARE(temp.widget, QStringLiteral("bar"));
  QCOMPARE(temp.wgtMax, 100.0);
  QCOMPARE(temp.pltMax, 100.0);
  QVERIFY(!temp.plt);
  QVERIFY(d.notes.isEmpty());
}

/**
 * @brief The GPS line keeps its map widget and lat/lon/alt roles.
 */
void DeviceDeclarationTest::goldenTinyGps()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("tinygps.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.channels, 3);
  QCOMPARE(d.groups.size(), size_t(1));
  QCOMPARE(d.groups[0].widget, QStringLiteral("gps"));
  QCOMPARE(d.groups[0].datasets[1].widget, QStringLiteral("lon"));
  QCOMPARE(d.groups[0].datasets[2].units, QStringLiteral("m"));
}

/**
 * @brief The data grid reuses the multi-plot's six frame positions through index, and the escaped
 *        space of "ADC 0" decodes back to a space.
 */
void DeviceDeclarationTest::goldenHexAdcSharesChannels()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("hexadc.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.channels, 6);
  QCOMPARE(d.groups.size(), size_t(2));
  for (size_t i = 0; i < 6; ++i) {
    QCOMPARE(d.groups[0].datasets[i].index, static_cast<int>(i) + 1);
    QCOMPARE(d.groups[1].datasets[i].index, static_cast<int>(i) + 1);
  }

  QCOMPARE(d.groups[0].datasets[0].title, QStringLiteral("ADC 0"));
  QVERIFY(d.groups[0].datasets[0].fft);
  QCOMPARE(d.groups[0].datasets[0].fftSamplingRate, 200);
  QCOMPARE(d.groups[1].widget, QStringLiteral("datagrid"));
}

/**
 * @brief Version 1 drops the output group with a note and keeps labels and alarm bands.
 */
void DeviceDeclarationTest::goldenControlsDropsOutputGroup()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("controls.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.groups.size(), size_t(1));
  QCOMPARE(d.channels, 3);
  QVERIFY(d.notes.join(' ').contains(QStringLiteral("Controls")));

  const auto& datasets = d.groups[0].datasets;
  QCOMPARE(datasets[0].labels.size(), size_t(4));
  QCOMPARE(datasets[0].labelsDefault, QStringLiteral("Unknown"));
  QCOMPARE(datasets[1].alarmBands.size(), size_t(1));
  QCOMPARE(datasets[1].alarmBands[0].max, 200.0);
  QCOMPARE(datasets[2].title, QStringLiteral("Tank 2 Pressure"));
}

/**
 * @brief One row per line that must be refused outright.
 */
void DeviceDeclarationTest::refusedLines_data()
{
  QTest::addColumn<QByteArray>("line");

  QByteArray manyGroups = "#ss1{\"title\":\"T\",\"groups\":[";
  for (int i = 0; i < 65; ++i)
    manyGroups += QByteArray(i ? "," : "") + "{\"title\":\"g\",\"datasets\":[{\"title\":\"d\"}]}";

  QByteArray huge = oneGroup("{\"title\":\"d\"}");
  huge.insert(huge.size() - 1, ",\"pad\":\"" + QByteArray(20000, 'x') + "\"");

  QByteArray manyBands = "{\"title\":\"d\",\"alarmBands\":[";
  for (int i = 0; i < 17; ++i)
    manyBands += QByteArray(i ? "," : "") + "{\"min\":1,\"max\":2}";

  QTest::newRow("malformed") << QByteArray("#ss1{\"title\":\"T\",\"groups\":[");
  QTest::newRow("unknown version") << QByteArray("#ss2{\"title\":\"T\",\"groups\":[]}");
  QTest::newRow("no version") << QByteArray("#ss{\"title\":\"T\"}");
  QTest::newRow("request echo") << QByteArray("#ss?");
  QTest::newRow("oversized") << huge;
  QTest::newRow("too many groups") << manyGroups + "]}";
  QTest::newRow("too many bands") << oneGroup(manyBands + "]}");
  QTest::newRow("long title") << oneGroup("{\"title\":\"" + QByteArray(65, 'a') + "\"}");
  QTest::newRow("no title") << QByteArray("#ss1{\"groups\":[{\"title\":\"G\",\"datasets\":[{}]}]}");
  QTest::newRow("no dataset") << oneGroup("");
  QTest::newRow("group without title") << QByteArray("#ss1{\"title\":\"T\",\"groups\":[{"
                                                     "\"datasets\":[{\"title\":\"d\"}]}]}");
}

/**
 * @brief A refused line is invalid, carries a reason and no structure, and builds nothing.
 */
void DeviceDeclarationTest::refusedLines()
{
  QFETCH(QByteArray, line);
  const auto d = DataModel::parseDeclaration(line);
  QVERIFY(!d.valid);
  QVERIFY(!d.reason.isEmpty());
  QVERIFY(d.groups.empty());

  DataModel::DeclarationState state;
  QCOMPARE(state.offer(line, at(0)), Outcome::Rejected);
  QVERIFY(!state.active());
}

/**
 * @brief Code, file paths, cross-dataset ids and code-bearing widgets never reach the structure.
 */
void DeviceDeclarationTest::codeAndPathsAreStripped()
{
  const QByteArray line =
    "#ss1{\"title\":\"T\",\"controlScriptCode\":\"x\",\"sources\":[{\"frameParserCode\":\"x\"}],"
    "\"groups\":[{\"title\":\"G\",\"widget\":\"painter\",\"painterCode\":\"p\",\"datasets\":[{"
    "\"title\":\"d\",\"transformCode\":\"return 1\",\"virtual\":true,\"xAxis\":7,"
    "\"transformParams\":{\"k\":1},\"alarmBands\":[{\"min\":1,\"max\":2,\"sound\":\"/etc/a.wav\"}]"
    "}]}]}";

  const auto d = DataModel::parseDeclaration(line);
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.groups.size(), size_t(1));

  const auto& group = d.groups.front();
  QCOMPARE(group.widget, QStringLiteral("datagrid"));
  QVERIFY(group.painterCode.isEmpty());

  const auto& dataset = group.datasets.front();
  QVERIFY(dataset.transformCode.isEmpty());
  QVERIFY(dataset.transformParams.isEmpty());
  QVERIFY(!dataset.virtual_);
  QCOMPARE(dataset.xAxisId, DataModel::kXAxisTime);
  QCOMPARE(dataset.alarmBands.size(), size_t(1));
  QVERIFY(dataset.alarmBands.front().sound.isEmpty());

  const auto notes = d.notes.join(' ');
  QVERIFY(notes.contains(QStringLiteral("transformCode")));
  QVERIFY(notes.contains(QStringLiteral("sources")));
  QVERIFY(notes.contains(QStringLiteral("painter")));
}

/**
 * @brief Out-of-domain and out-of-range numbers fall back to the registry defaults.
 */
void DeviceDeclarationTest::sanitizerBoundsValues()
{
  const auto d = DataModel::parseDeclaration(
    oneGroup("{\"title\":\"d\",\"fftSamples\":300,\"fftWindow\":99,\"decimalPoints\":99,"
             "\"displayFormat\":\"%n\",\"formatWidth\":8,\"formatFractionBits\":12,"
             "\"fftBallisticsRelease\":1,\"color\":\"not a colour\"}"));
  QVERIFY2(d.valid, qPrintable(d.reason));

  const auto& dataset = d.groups.front().datasets.front();
  QCOMPARE(dataset.fftSamples, 256);
  QCOMPARE(dataset.fftWindow, 5);
  QCOMPARE(dataset.decimalPoints, 15);
  QCOMPARE(dataset.displayFormat, QStringLiteral("0d"));
  QCOMPARE(dataset.formatWidth, 8);
  QCOMPARE(dataset.formatFractionBits, 0);
  QCOMPARE(dataset.fftBallisticsRelease, 50);
  QVERIFY(dataset.color.isEmpty());
}

/**
 * @brief An index may reuse an earlier position or name the next one, never skip ahead.
 */
void DeviceDeclarationTest::indexPastNextPositionIsRefused()
{
  const auto ok = DataModel::parseDeclaration(
    oneGroup("{\"title\":\"a\",\"index\":1},{\"title\":\"b\"},{\"title\":\"c\",\"index\":1}"));
  QVERIFY2(ok.valid, qPrintable(ok.reason));
  QCOMPARE(ok.channels, 2);
  QCOMPARE(ok.groups.front().datasets[2].index, 1);

  const auto gap = DataModel::parseDeclaration(oneGroup("{\"title\":\"a\",\"index\":3}"));
  QVERIFY(!gap.valid);
  QVERIFY(gap.reason.contains(QStringLiteral("index 3")));
}

/**
 * @brief The same declaration twice builds once, a trailing CR included (spec 0111 R4).
 */
void DeviceDeclarationTest::sameLineRebuildsOnce()
{
  const auto line = golden(QStringLiteral("mpu6050.decl"));
  DataModel::DeclarationState state;
  QCOMPARE(state.offer(line, at(0)), Outcome::Rebuild);
  QVERIFY(state.active());
  QCOMPARE(state.offer(line, at(5000)), Outcome::Unchanged);
  QCOMPARE(state.offer(line + "\r", at(9000)), Outcome::Unchanged);
  QCOMPARE(state.current().title, QStringLiteral("MPU6050"));
}

/**
 * @brief A changed declaration rebuilds again, but not within a second of the last rebuild.
 */
void DeviceDeclarationTest::changedLineWithinASecondIsHeld()
{
  const auto first  = golden(QStringLiteral("mpu6050.decl"));
  const auto second = golden(QStringLiteral("tinygps.decl"));
  DataModel::DeclarationState state;
  QCOMPARE(state.offer(first, at(0)), Outcome::Rebuild);
  QCOMPARE(state.offer(second, at(400)), Outcome::Held);
  QCOMPARE(state.offer(second, at(600)), Outcome::Unchanged);
  QCOMPARE(state.current().title, QStringLiteral("MPU6050"));
  QCOMPARE(state.offer(second, at(1200)), Outcome::Rebuild);
  QCOMPARE(state.current().title, QStringLiteral("TinyGPS"));
  QCOMPARE(state.offer(first, at(1300)), Outcome::Held);
}

/**
 * @brief A refused line is reported once; the declaration it failed to replace stays.
 */
void DeviceDeclarationTest::refusedLineIsReportedOnce()
{
  DataModel::DeclarationState state;
  QCOMPARE(state.offer(golden(QStringLiteral("tinygps.decl")), at(0)), Outcome::Rebuild);
  QCOMPARE(state.offer("#ss1{broken", at(2000)), Outcome::Rejected);
  QVERIFY(state.message().contains(QStringLiteral("ignored")));
  QCOMPARE(state.offer("#ss1{broken", at(4000)), Outcome::Unchanged);
  QVERIFY(state.active());
  QCOMPARE(state.current().title, QStringLiteral("TinyGPS"));
}

/**
 * @brief After reset the same line builds again, the way a reconnect's request brings it back.
 */
void DeviceDeclarationTest::resetForgetsTheDeclaration()
{
  const auto line = golden(QStringLiteral("hexadc.decl"));
  DataModel::DeclarationState state;
  QCOMPARE(state.offer(line, at(0)), Outcome::Rebuild);
  state.reset();
  QVERIFY(!state.active());
  QCOMPARE(state.offer(line, at(100)), Outcome::Rebuild);
}

/**
 * @brief The wire projection of a declaration equals that of the same groups after a project
 *        save and load, and moves when a title does.
 */
void DeviceDeclarationTest::projectionMatchesALoadedProject()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("mpu6050.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));

  std::vector<DataModel::Group> loaded;
  for (size_t i = 0; i < d.groups.size(); ++i) {
    DataModel::Group group;
    group.groupId = static_cast<int>(i);
    QVERIFY(DataModel::read(group, DataModel::serialize(d.groups[i])));
    loaded.push_back(group);
  }

  const auto declared = DataModel::wireProjection(d.title, d.groups);
  QCOMPARE(DataModel::wireProjection(d.title, loaded), declared);

  loaded[1].datasets[0].title = QStringLiteral("Roll");
  QVERIFY(DataModel::wireProjection(d.title, loaded) != declared);
  QVERIFY(!declared.value(Keys::Groups).toArray().isEmpty());
}

QTEST_APPLESS_MAIN(DeviceDeclarationTest)

#include "tst_device_declaration.moc"
