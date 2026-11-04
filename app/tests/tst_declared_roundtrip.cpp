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

#include <array>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QTest>
#include <vector>

#include "Core/Checksum.h"
#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder/QuickPlotFormatState.h"
#include "DataModel/Project/DeviceDeclaration.h"
#include "DataModel/Project/QuickPlotProject.h"
#include "DataModel/Scripting/NativeTemplates/BinaryPackedStruct.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/NativeTemplates/TextLineValues.h"

// Declared Quick Plot to project round trip (spec 0111 AC1 save half, R9): for every golden
// declaration and its frames, the project Save as Project writes parses the stream, the device's
// boot declaration included, to the values the live declared lane showed.

using Outcome = DataModel::QuickPlotLineOutcome;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

struct LiveValue {
  double number = 0.0;
  bool numeric  = false;
  bool present  = false;
};

/**
 * @brief Every non-empty line of a golden fixture file.
 */
[[nodiscard]] static QList<QByteArray> fixtureLines(const QString& name)
{
  QList<QByteArray> lines;
  QFile file(QStringLiteral(SS_DECLARATION_GOLDEN_DIR "/") + name);
  if (!file.open(QIODevice::ReadOnly))
    return lines;

  for (const auto& line : file.readAll().split('\n'))
    if (!line.trimmed().isEmpty())
      lines.append(line.trimmed());

  return lines;
}

/**
 * @brief What each declared channel showed live for the line last observed; a channel no field
 *        mapped to is not present (no sample, spec 0111 R5).
 */
[[nodiscard]] static QList<LiveValue> liveValues(DataModel::QuickPlotFormatState& st,
                                                 const DataModel::LineFields& fields)
{
  QList<LiveValue> values(static_cast<qsizetype>(st.channels().size()));
  for (qsizetype f = 0; f < fields.count; ++f) {
    const auto& field = fields.items[static_cast<size_t>(f)];
    const int channel = st.indexOf(field, f);
    if (channel >= 0 && channel < values.size())
      values[channel] = {field.number, field.numeric, true};
  }

  return values;
}

/**
 * @brief The line_values parser the generated project's first source configures.
 */
[[nodiscard]] static std::unique_ptr<DataModel::INativeParser> savedParser(
  const QJsonObject& project, QString& error)
{
  const auto source = project.value(Keys::Sources).toArray().first().toObject();
  if (source.value(Keys::FrameParserTemplate).toString() != QLatin1String("line_values")) {
    error = QStringLiteral("a declared session must save line_values");
    return nullptr;
  }

  const auto params = source.value(Keys::FrameParserParams).toObject();
  return DataModel::lineValuesTemplate().makeParser(params, error);
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class DeclaredRoundTripTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void roundTrip_data();
  void roundTrip();
  void projectKeepsTheDeclaredStructure_data();
  void projectKeepsTheDeclaredStructure();
  void outputGroupSurvivesSave();
  void binaryRoundTrip();
  void commaTitleKeepsItsSlot();

private:
  DataModel::LineFields m_fields;
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void DeclaredRoundTripTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief One row per golden declaration that ships a frames file.
 */
void DeclaredRoundTripTest::roundTrip_data()
{
  QTest::addColumn<QString>("name");
  for (const char* name : {"mpu6050", "tinygps", "hexadc", "controls", "formats", "textfield"})
    QTest::newRow(name) << QString::fromLatin1(name);
}

/**
 * @brief The saved parser skips the boot declaration and yields, line by line, the number every
 *        channel the line carried showed live.
 */
void DeclaredRoundTripTest::roundTrip()
{
  QFETCH(QString, name);
  const auto declaration = fixtureLines(name + QStringLiteral(".decl"));
  const auto frames      = fixtureLines(name + QStringLiteral(".frames"));
  QCOMPARE(declaration.size(), 1);
  QVERIFY(!frames.isEmpty());

  const auto parsed = DataModel::parseDeclaration(declaration.first());
  QVERIFY2(parsed.valid, qPrintable(parsed.reason));

  DataModel::QuickPlotFormatState st;
  st.pinDeclared(DataModel::declaredChannels(parsed));
  QList<QList<LiveValue>> live;
  for (const auto& line : frames) {
    QCOMPARE(st.observe(QByteArrayView(line), m_fields), Outcome::Apply);
    live.append(liveValues(st, m_fields));
  }

  const auto project =
    DataModel::QuickPlotProject::build(parsed.groups, 0, QJsonObject(), st.format(), parsed.title);
  QString error;
  auto parser = savedParser(project, error);
  QVERIFY2(parser, qPrintable(error));

  QVERIFY(parser->parseText(QString::fromUtf8(declaration.first())).isEmpty());
  for (qsizetype l = 0; l < frames.size(); ++l) {
    const auto rows = parser->parseText(QString::fromUtf8(frames.at(l)));
    QCOMPARE(rows.size(), 1);
    for (qsizetype c = 0; c < live[l].size(); ++c) {
      if (!live[l][c].present)
        continue;

      bool ok            = false;
      const double value = SerialStudio::toDouble(rows[0].value(c), &ok);
      QCOMPARE(ok, live[l][c].numeric);
      if (ok)
        QVERIFY2(qFuzzyCompare(value, live[l][c].number), qPrintable(frames.at(l)));
    }
  }
}

/**
 * @brief One row per golden declaration.
 */
void DeclaredRoundTripTest::projectKeepsTheDeclaredStructure_data()
{
  roundTrip_data();
}

/**
 * @brief The saved project reads back with the declared title and the same groups, widgets,
 *        titles, units, ranges and frame indices, so the editor opens what the device declared.
 */
void DeclaredRoundTripTest::projectKeepsTheDeclaredStructure()
{
  QFETCH(QString, name);
  const auto parsed =
    DataModel::parseDeclaration(fixtureLines(name + QStringLiteral(".decl")).first());
  QVERIFY2(parsed.valid, qPrintable(parsed.reason));

  const auto project = DataModel::QuickPlotProject::build(
    parsed.groups, 0, QJsonObject(), DataModel::LineFormat(), parsed.title);
  QCOMPARE(project.value(Keys::Title).toString(), parsed.title);

  std::vector<DataModel::Group> groups;
  const auto array = project.value(Keys::Groups).toArray();
  for (qsizetype i = 0; i < array.size(); ++i) {
    DataModel::Group group;
    group.groupId = static_cast<int>(i);
    QVERIFY(DataModel::read(group, array.at(i).toObject()));
    groups.push_back(group);
  }

  QCOMPARE(DataModel::wireProjection(parsed.title, groups),
           DataModel::wireProjection(parsed.title, parsed.groups));
}

/**
 * @brief The declared output group reads back from the saved project as an output group whose
 *        controls keep their kind, range and the app's command transmit function (spec 0111 R7).
 */
void DeclaredRoundTripTest::outputGroupSurvivesSave()
{
  const auto parsed =
    DataModel::parseDeclaration(fixtureLines(QStringLiteral("controls.decl")).first());
  QVERIFY2(parsed.valid, qPrintable(parsed.reason));
  QCOMPARE(parsed.groups.size(), size_t(3));

  const auto project = DataModel::QuickPlotProject::build(
    parsed.groups, 0, QJsonObject(), DataModel::LineFormat(), parsed.title);
  const auto array = project.value(Keys::Groups).toArray();
  QCOMPARE(array.size(), 3);

  DataModel::Group replies;
  replies.groupId = 2;
  QVERIFY(DataModel::read(replies, array.at(2).toObject()));
  QCOMPARE(replies.datasets.size(), size_t(3));
  QCOMPARE(replies.datasets[0].uniqueId, -1);
  QVERIFY(replies.datasets[2].hideOnDashboard);

  DataModel::Group panel;
  panel.groupId = 1;
  QVERIFY(DataModel::read(panel, array.at(1).toObject()));
  QCOMPARE(panel.groupType, DataModel::GroupType::Output);
  QCOMPARE(panel.outputWidgets.size(), parsed.groups[1].outputWidgets.size());
  for (size_t i = 0; i < panel.outputWidgets.size(); ++i) {
    const auto& saved    = panel.outputWidgets[i];
    const auto& declared = parsed.groups[1].outputWidgets[i];
    QCOMPARE(saved.type, declared.type);
    QCOMPARE(saved.title, declared.title);
    QCOMPARE(saved.maxValue, declared.maxValue);
    QCOMPARE(saved.transmitFunction, declared.transmitFunction);
    QVERIFY(!saved.transmitFunction.isEmpty());
    QCOMPARE(saved.replyTokenDatasetId, DataModel::dataset_unique_id(0, 2, 0));
    QCOMPARE(saved.replyMessageDatasetId, DataModel::dataset_unique_id(0, 2, 2));
  }
}

/**
 * @brief The binary MPU6050 declaration saves a project with its framing and packed_struct
 *        decoder, and that parser reads every golden wire frame to the live lane's numbers, which
 *        are the text variant's values (spec 0111 R9/R14, AC10 unit half).
 */
void DeclaredRoundTripTest::binaryRoundTrip()
{
  const auto parsed =
    DataModel::parseDeclaration(fixtureLines(QStringLiteral("mpu6050_bin.decl")).first());
  QVERIFY2(parsed.valid, qPrintable(parsed.reason));
  QVERIFY(parsed.binary);

  const auto project = DataModel::QuickPlotProject::build(
    parsed.groups, 0, QJsonObject(), DataModel::LineFormat(), parsed.title, parsed.source);
  const auto source = project.value(Keys::Sources).toArray().first().toObject();
  QCOMPARE(source.value(Keys::FrameParserTemplate).toString(), QStringLiteral("packed_struct"));
  QCOMPARE(source.value(Keys::FrameDetection).toInt(),
           static_cast<int>(SerialStudio::StartDelimiterOnly));
  QCOMPARE(source.value(Keys::DecoderMethod).toInt(), static_cast<int>(SerialStudio::Binary));
  QCOMPARE(source.value(Keys::ChecksumAlgorithm).toString(), QStringLiteral("CRC-16"));
  QCOMPARE(source.value(Keys::FrameStart).toString(), QStringLiteral("AA55"));

  QString error;
  DataModel::PackedLayout layout;
  const auto params = source.value(Keys::FrameParserParams).toObject();
  const auto parser = DataModel::packedStructTemplate().makeParser(params, error);
  QVERIFY2(parser, qPrintable(error));
  QVERIFY(DataModel::parsePackedLayout(params.value(QStringLiteral("fields")).toString(),
                                       params.value(QStringLiteral("endianness")).toString(),
                                       layout,
                                       error));

  const auto frames = fixtureLines(QStringLiteral("mpu6050_bin.frames"));
  const auto text   = fixtureLines(QStringLiteral("mpu6050.frames"));
  QCOMPARE(frames.size(), text.size());
  for (qsizetype l = 0; l < frames.size(); ++l) {
    const auto wire    = QByteArray::fromHex(frames.at(l));
    const auto payload = wire.sliced(2, wire.size() - 4);
    QCOMPARE(wire.first(2), QByteArray::fromHex("aa55"));
    QCOMPARE(IO::checksum(QStringLiteral("CRC-16"), payload), wire.last(2));

    std::array<double, DataModel::kMaxPackedFields> live{};
    QCOMPARE(DataModel::decodePackedStruct(payload, layout, live.data(), live.size()), 7);

    const auto rows   = parser->parseBinary(payload);
    const auto fields = text.at(l).split(',');
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.first().size(), 7);
    for (int c = 0; c < 7; ++c) {
      QCOMPARE(SerialStudio::toDouble(rows.first().at(c)), live[static_cast<size_t>(c)]);
      QCOMPARE(live[static_cast<size_t>(c)], SerialStudio::toDouble(fields.at(c)));
    }
  }
}

/**
 * @brief A comma in a declared title never splits the saved field list: in a named stream the
 *        channel after it still reads its own name, as it does live (R9).
 */
void DeclaredRoundTripTest::commaTitleKeepsItsSlot()
{
  const auto parsed = DataModel::parseDeclaration(QByteArrayLiteral(
    R"(#ss1{"title":"Rig","groups":[{"title":"Air","datasets":[{"title":"Temp, Inside"},)"
    R"({"title":"Hum"}]}]})"));
  QVERIFY2(parsed.valid, qPrintable(parsed.reason));

  DataModel::LineFormat format;
  format.named = true;
  const auto project =
    DataModel::QuickPlotProject::build(parsed.groups, 0, QJsonObject(), format, parsed.title);
  QString error;
  auto parser = savedParser(project, error);
  QVERIFY2(parser, qPrintable(error));

  const auto rows = parser->parseText(QStringLiteral("Hum:42"));
  QCOMPARE(rows.size(), 1);
  QCOMPARE(rows.first().size(), 2);
  QCOMPARE(SerialStudio::toDouble(rows.first().at(1)), 42.0);
}

QTEST_APPLESS_MAIN(DeclaredRoundTripTest)

#include "tst_declared_roundtrip.moc"
