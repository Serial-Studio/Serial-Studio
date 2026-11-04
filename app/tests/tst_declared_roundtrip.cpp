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
#include <QJsonObject>
#include <QTest>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder/QuickPlotFormatState.h"
#include "DataModel/Project/DeviceDeclaration.h"
#include "DataModel/Project/QuickPlotProject.h"
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
  for (const char* name : {"mpu6050", "tinygps", "hexadc", "controls"})
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

QTEST_APPLESS_MAIN(DeclaredRoundTripTest)

#include "tst_declared_roundtrip.moc"
