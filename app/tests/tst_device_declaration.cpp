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

#include <QColor>
#include <QFile>
#include <QJSEngine>
#include <QJsonArray>
#include <QTest>

#include "Core/DataModel/FrameKeys.h"
#include "Core/DataModel/PropertyValidators.h"
#include "DataModel/Project/DeclaredControls.h"
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
  void goldenControlsBuildOutputGroup();
  void controlNamesAreRefused_data();
  void controlNamesAreRefused();
  void sliderTransmitsCommand();
  void controlsBindReplyChannels();
  void textFieldIsANumberField();
  void refusedLines_data();
  void refusedLines();
  void codeAndPathsAreStripped();
  void goldenBinarySource();
  void binarySourceRefusals_data();
  void binarySourceRefusals();
  void sanitizerBoundsValues();
  void indexPastNextPositionIsRefused();
  void sameLineRebuildsOnce();
  void changedLineWithinASecondIsHeld();
  void refusedLineIsReportedOnce();
  void resetForgetsTheDeclaration();
  void projectionMatchesALoadedProject();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, and installs the colour parser
 *        the composition root installs, without which the sanitizer accepts any colour.
 */
void DeviceDeclarationTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
  DataModel::PropertyHooks::setColorValidator(
    [](const QString& color) { return QColor::fromString(color).isValid(); });
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
 * @brief The control group becomes an output group whose every control sends through the command
 *        template, and the data group keeps its labels and alarm bands.
 */
void DeviceDeclarationTest::goldenControlsBuildOutputGroup()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("controls.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.groups.size(), size_t(3));
  QCOMPARE(d.channels, 6);

  const auto& panel = d.groups[1];
  QCOMPARE(panel.groupType, DataModel::GroupType::Output);
  QVERIFY(panel.widget.isEmpty());
  QVERIFY(panel.datasets.empty());
  QCOMPARE(panel.outputWidgets.size(), size_t(4));
  QCOMPARE(panel.outputWidgets[0].type, DataModel::OutputWidgetType::Button);
  QCOMPARE(panel.outputWidgets[1].offLabel, QStringLiteral("Off"));
  QCOMPARE(panel.outputWidgets[2].type, DataModel::OutputWidgetType::Slider);
  QCOMPARE(panel.outputWidgets[2].maxValue, 255.0);
  QCOMPARE(panel.outputWidgets[3].stepSize, 0.5);
  for (const auto& control : panel.outputWidgets)
    QVERIFY(control.transmitFunction.contains("const CONTROL = \"" + control.title + "\";"));

  const auto& datasets = d.groups[0].datasets;
  QCOMPARE(datasets[0].labels.size(), size_t(4));
  QCOMPARE(datasets[0].labelsDefault, QStringLiteral("Unknown"));
  QCOMPARE(datasets[1].alarmBands.size(), size_t(1));
  QCOMPARE(datasets[1].alarmBands[0].max, 200.0);
  QCOMPARE(datasets[2].title, QStringLiteral("Tank 2 Pressure"));
}

/**
 * @brief Controls get three hidden reply channels after the declared ones (token, code, message),
 *        and every control is bound to the unique ids a build and a project load both derive.
 */
void DeviceDeclarationTest::controlsBindReplyChannels()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("controls.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.groups.size(), size_t(3));

  const auto& replies = d.groups[2];
  QCOMPARE(replies.groupId, 2);
  QCOMPARE(replies.datasets.size(), size_t(3));
  for (int i = 0; i < 3; ++i) {
    QVERIFY(replies.datasets[static_cast<size_t>(i)].hideOnDashboard);
    QCOMPARE(replies.datasets[static_cast<size_t>(i)].index, 4 + i);
  }

  for (const auto& control : d.groups[1].outputWidgets) {
    QCOMPARE(control.replyTokenDatasetId, DataModel::dataset_unique_id(0, 2, 0));
    QCOMPARE(control.replyCodeDatasetId, DataModel::dataset_unique_id(0, 2, 1));
    QCOMPARE(control.replyMessageDatasetId, DataModel::dataset_unique_id(0, 2, 2));
    QVERIFY(DataModel::outputExpectsReply(control));
  }

  const auto channels = DataModel::declaredChannels(d);
  QCOMPARE(channels.size(), size_t(6));
  QCOMPARE(channels[5].name, QByteArray("Reply Message"));

  const auto plain = DataModel::parseDeclaration(golden(QStringLiteral("mpu6050.decl")));
  QCOMPARE(plain.groups.size(), size_t(3));
  QCOMPARE(plain.channels, 7);
}

/**
 * @brief A declared text field is a Number field (spec 0109), so its typed value is admitted to
 *        Min to Max before it is sent.
 */
void DeviceDeclarationTest::textFieldIsANumberField()
{
  const QByteArray line =
    "#ss1{\"title\":\"T\",\"groups\":[{\"title\":\"G\",\"datasets\":[{\"title\":\"d\"}]},{\"title\":"
    "\"C\",\"groupType\":1,\"outputWidgets\":[{\"title\":\"Rate\",\"outputType\":3,\"outputMin\":"
    "0,\"outputMax\":50,\"outputUnits\":\"Hz\",\"inputKind\":0}]}]}";

  const auto d = DataModel::parseDeclaration(line);
  QVERIFY2(d.valid, qPrintable(d.reason));
  QCOMPARE(d.groups.size(), size_t(3));

  const auto& field = d.groups[1].outputWidgets.front();
  QCOMPARE(field.type, DataModel::OutputWidgetType::TextField);
  QCOMPARE(field.inputKind, DataModel::OutputInputKind::Number);
  QCOMPARE(field.maxValue, 50.0);
  QVERIFY(DataModel::outputIsNumeric(field));
  QCOMPARE(field.units, QStringLiteral("Hz"));
}

/**
 * @brief One row per control name that must refuse the whole declaration (spec 0111 R10).
 */
void DeviceDeclarationTest::controlNamesAreRefused_data()
{
  QTest::addColumn<QByteArray>("title");

  QTest::newRow("quote") << QByteArray("a\\\"b");
  QTest::newRow("backslash") << QByteArray("a\\\\b");
  QTest::newRow("newline") << QByteArray("a\\nb");
  QTest::newRow("script tag") << QByteArray("</script>");
  QTest::newRow("33 bytes") << QByteArray(33, 'a');
  QTest::newRow("leading space") << QByteArray(" a");
  QTest::newRow("double space") << QByteArray("a  b");
  QTest::newRow("non-ascii") << QByteArray("Gr\xC3\xBC\xC3\x9F");
  QTest::newRow("empty") << QByteArray();
}

/**
 * @brief A control name outside [A-Za-z0-9_ .-]{1,32} refuses the line, so device text never
 *        becomes part of a transmit function.
 */
void DeviceDeclarationTest::controlNamesAreRefused()
{
  QFETCH(QByteArray, title);
  const QByteArray line =
    "#ss1{\"title\":\"T\",\"groups\":[{\"title\":\"G\",\"datasets\":[{\"title\""
    ":\"d\"}]},{\"title\":\"C\",\"groupType\":1,\"outputWidgets\":[{\"title\":\""
    + title + "\",\"outputType\":1}]}]}";

  const auto d = DataModel::parseDeclaration(line);
  QVERIFY(!d.valid);
  QVERIFY(d.reason.contains(QStringLiteral("valid name")));
  QVERIFY(DataModel::DeclaredControls::transmitFunction(QString::fromUtf8(title)).isEmpty());
}

/**
 * @brief A declared slider at 128 sends the library's command line, and the token suffix rides
 *        along when the app passes one (spec 0110).
 */
void DeviceDeclarationTest::sliderTransmitsCommand()
{
  const auto code = DataModel::DeclaredControls::transmitFunction(QStringLiteral("Speed"));
  QVERIFY(!code.isEmpty());
  QVERIFY(!code.contains(QStringLiteral("\"control\"")));

  QJSEngine engine;
  auto transmit =
    engine.evaluate(QStringLiteral("(function() { %1\nreturn transmit; })()").arg(code));
  QVERIFY(transmit.isCallable());
  QCOMPARE(transmit.call({QJSValue(128)}).toString(), QStringLiteral("#ss!Speed=128\n"));
  QCOMPARE(transmit.call({QJSValue(128), QJSValue(7)}).toString(),
           QStringLiteral("#ss!Speed=128@7\n"));
  QVERIFY(DataModel::DeclaredControls::validName(QStringLiteral("Pump 2.speed_x-1")));
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
 *        The note names the first eight refused keys in parse order, which ends at connection.
 */
void DeviceDeclarationTest::codeAndPathsAreStripped()
{
  const QByteArray line =
    "#ss1{\"title\":\"T\",\"controlScriptCode\":\"x\",\"sources\":[{\"frameParserCode\":\"x\","
    "\"connection\":{\"port\":\"/dev/x\"},\"frameStart\":\"$\",\"frameDetection\":3,"
    "\"decoderMethod\":3,\"frameParserTemplate\":\"packed_struct\",\"frameParserParams\":{"
    "\"fields\":\"f32\",\"script\":\"x\"}}],"
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

  QVERIFY(d.binary);
  QVERIFY(d.source.frameParserCode.isEmpty());
  QVERIFY(d.source.connectionSettings.isEmpty());
  QVERIFY(!d.source.frameParserParams.contains(QStringLiteral("script")));

  const auto notes = d.notes.join(' ');
  QVERIFY(notes.contains(QStringLiteral("controlScriptCode")));
  QVERIFY(notes.contains(QStringLiteral("transformCode")));
  QVERIFY(notes.contains(QStringLiteral("connection")));
  QVERIFY(notes.contains(QStringLiteral("painter")));
}

/**
 * @brief The binary MPU6050 line carries its framing and a seven-field packed_struct layout.
 */
void DeviceDeclarationTest::goldenBinarySource()
{
  const auto d = DataModel::parseDeclaration(golden(QStringLiteral("mpu6050_bin.decl")));
  QVERIFY2(d.valid, qPrintable(d.reason));
  QVERIFY(d.binary);
  QCOMPARE(d.channels, 7);
  QCOMPARE(d.groups.size(), size_t(3));
  QCOMPARE(d.source.frameStart, QStringLiteral("AA55"));
  QVERIFY(d.source.hexadecimalDelimiters);
  QCOMPARE(d.source.frameDetection, static_cast<int>(SerialStudio::StartDelimiterOnly));
  QCOMPARE(d.source.checksumAlgorithm, QStringLiteral("CRC-16"));
  QCOMPARE(d.source.decoderMethod, static_cast<int>(SerialStudio::Binary));
  QCOMPARE(d.source.frameParserTemplate, QStringLiteral("packed_struct"));
  QCOMPARE(d.source.frameParserParams.value(QStringLiteral("fields")).toString(),
           QStringLiteral("f32,f32,f32,f32,f32,f32,f32"));
  QCOMPARE(d.source.frameParserParams.value(QStringLiteral("endianness")).toString(),
           QStringLiteral("little"));
}

/**
 * @brief One row per binary source that must refuse the whole declaration.
 */
void DeviceDeclarationTest::binarySourceRefusals_data()
{
  QTest::addColumn<QByteArray>("source");

  const QByteArray ok =
    "\"frameStart\":\"AA55\",\"hexadecimalDelimiters\":true,\"frameDetection\":3,"
    "\"decoderMethod\":3,\"frameParserTemplate\":\"packed_struct\"";
  const QByteArray two = ",\"frameParserParams\":{\"fields\":\"f32,u8\"}";

  QTest::newRow("text decoder") << QByteArray(ok).replace("\"decoderMethod\":3",
                                                          "\"decoderMethod\":0")
                                     + two;
  QTest::newRow("other template") << QByteArray(ok).replace("packed_struct", "raw_bytes") + two;
  QTest::newRow("field count") << ok + ",\"frameParserParams\":{\"fields\":\"f32\"}";
  QTest::newRow("unknown type") << ok + ",\"frameParserParams\":{\"fields\":\"f32,f16\"}";
  QTest::newRow("unknown checksum") << ok + two + ",\"checksumAlgorithm\":\"MD5\"";
  QTest::newRow("long delimiter") << QByteArray(ok).replace("AA55", "AABBCCDDEEFF001122") + two;
  QTest::newRow("end only") << QByteArray(ok).replace("\"frameDetection\":3",
                                                      "\"frameDetection\":0")
                                 + two;
  QTest::newRow("no end") << QByteArray(ok).replace("\"frameDetection\":3", "\"frameDetection\":1")
                               + two;
}

/**
 * @brief A binary source the app cannot frame or decode refuses the line, so a device never gets
 *        a dashboard whose values would be read with the wrong layout.
 */
void DeviceDeclarationTest::binarySourceRefusals()
{
  QFETCH(QByteArray, source);
  const QByteArray line = "#ss1{\"title\":\"T\",\"sources\":[{" + source
                        + "}],\"groups\":[{\"title\":\"G\",\"datasets\":[{\"title\":\"a\"},{"
                          "\"title\":\"b\"}]}]}";

  const auto d = DataModel::parseDeclaration(line);
  QVERIFY(!d.valid);
  QVERIFY(!d.binary);
  QVERIFY(!d.reason.isEmpty());
}

/**
 * @brief Out-of-domain and out-of-range numbers fall back to the registry defaults.
 */
void DeviceDeclarationTest::sanitizerBoundsValues()
{
  const auto d = DataModel::parseDeclaration(
    oneGroup("{\"title\":\"d\",\"fftSamples\":300,\"fftWindow\":99,\"decimalPoints\":99,"
             "\"displayFormat\":\"%n\",\"format\":1,\"formatWidth\":8,\"formatFractionBits\":12,"
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

QTEST_GUILESS_MAIN(DeviceDeclarationTest)

#include "tst_device_declaration.moc"
