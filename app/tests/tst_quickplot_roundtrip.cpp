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
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder/QuickPlotFormatState.h"
#include "DataModel/Project/QuickPlotProject.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/NativeTemplates/TextDelimited.h"
#include "DataModel/Scripting/NativeTemplates/TextLineValues.h"

// Quick Plot to project round trip (spec 0106 AC5): the project Save as Project writes must parse
// a line to the same values the live Quick Plot lane showed.

using Outcome = DataModel::QuickPlotLineOutcome;

//--------------------------------------------------------------------------------------------------
// Corpus
//--------------------------------------------------------------------------------------------------

/**
 * @brief The single-line streams every round trip is checked against.
 */
[[nodiscard]] static QList<QByteArray> corpusLines()
{
  return {
    "23.5,60,1013",
    "23.5;60;1013",
    "23,5;60;1013,2",
    "23.5\t60\t1013",
    "23.5 60 1013",
    "23,5 60,2 1013",
    "temp:23.5,hum:60",
    "temp: 23.5, hum: 60",
    "temp=23.5;hum=60",
    "temp:23.5 hum:60",
    "temp:23.5\xC2\xB0"
    "C;pressure:1013hPa",
    "3.3V,25\xC2\xB0"
    "C,45%",
    "{\"t\":23.5,\"h\":60}",
    "{\"imu\":{\"ax\":0.1,\"g\":[1,2]},\"t\":5}",
    "/*23.5,60,1013*/",
    "-12.5,1.5e3,+7",
    "temp:23.5,60",
    "temp:23,5;hum:60",
    "Temp Sensor: 23.5, Humidity: 60",
    "temp:23.5,status:OK",
  };
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

struct LiveValue {
  double number = 0.0;
  bool numeric  = false;
};

/**
 * @brief Builds the data grid the live lane would show for the established channels.
 */
[[nodiscard]] static DataModel::Group dataGridFor(const DataModel::QuickPlotFormatState& st)
{
  DataModel::Group group;
  group.groupId  = 0;
  group.title    = QStringLiteral("Data Grid");
  group.widget   = QStringLiteral("datagrid");
  const auto& ch = st.channels();
  for (size_t i = 0; i < ch.size(); ++i) {
    const int channel = static_cast<int>(i);
    DataModel::Dataset dataset;
    dataset.groupId   = 0;
    dataset.index     = channel + 1;
    dataset.datasetId = channel;
    dataset.title     = st.channelTitle(channel);
    if (dataset.title.isEmpty())
      dataset.title = QStringLiteral("Channel %1").arg(channel + 1);

    dataset.units = QString::fromUtf8(ch[i].unit);
    group.datasets.push_back(dataset);
  }

  return group;
}

/**
 * @brief The value each channel showed live; non-numeric when no field maps to the channel.
 */
[[nodiscard]] static QList<LiveValue> liveValues(DataModel::QuickPlotFormatState& st,
                                                 const DataModel::LineFields& fields)
{
  QList<LiveValue> values(static_cast<qsizetype>(st.channels().size()));
  for (qsizetype f = 0; f < fields.count; ++f) {
    const auto& field = fields.items[static_cast<size_t>(f)];
    const int channel = st.indexOf(field, f);
    if (channel < 0 || channel >= values.size())
      continue;

    values[channel] = {field.number, field.numeric};
  }

  return values;
}

/**
 * @brief The descriptor for a project source's template id; null for an id the suite does not link.
 */
[[nodiscard]] static const DataModel::INativeTemplate* templateFor(const QString& id)
{
  if (id == QLatin1String("line_values"))
    return &DataModel::lineValuesTemplate();

  if (id == QLatin1String("delimited"))
    return &DataModel::delimitedTextTemplate();

  return nullptr;
}

/**
 * @brief The first source of a generated project.
 */
[[nodiscard]] static QJsonObject firstSource(const QJsonObject& project)
{
  return project.value(Keys::Sources).toArray().first().toObject();
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class QuickPlotRoundTripTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void roundTrip_data();
  void roundTrip();
  void titlesCarryUnits();

private:
  DataModel::LineFields m_fields;
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void QuickPlotRoundTripTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief One row per corpus line.
 */
void QuickPlotRoundTripTest::roundTrip_data()
{
  QTest::addColumn<QByteArray>("line");
  const auto lines = corpusLines();
  QCOMPARE(lines.size(), 20);
  for (const auto& line : lines)
    QTest::newRow(line.constData()) << line;
}

/**
 * @brief The live values of every channel equal what the saved project's parser yields.
 */
void QuickPlotRoundTripTest::roundTrip()
{
  QFETCH(QByteArray, line);
  DataModel::QuickPlotFormatState st;
  QCOMPARE(st.observe(QByteArrayView(line), m_fields), Outcome::Rebuild);

  const auto live = liveValues(st, m_fields);
  const auto project =
    DataModel::QuickPlotProject::build({dataGridFor(st)}, 0, QJsonObject(), st.format());
  const auto source = firstSource(project);
  const auto id     = source.value(Keys::FrameParserTemplate).toString();
  const auto params = source.value(Keys::FrameParserParams).toObject();
  const auto* tmpl  = templateFor(id);
  QVERIFY2(tmpl, qPrintable(id));

  QString error;
  auto parser = tmpl->makeParser(params, error);
  QVERIFY2(parser, qPrintable(error));

  const auto rows = parser->parseText(QString::fromUtf8(line));
  QCOMPARE(rows.size(), 1);
  QVERIFY(!live.isEmpty());
  for (qsizetype i = 0; i < live.size(); ++i) {
    bool ok            = false;
    const double value = SerialStudio::toDouble(rows[0].value(i), &ok);
    QCOMPARE(ok, live[i].numeric);
    if (live[i].numeric)
      QVERIFY2(qFuzzyCompare(value, live[i].number), qPrintable(QString::number(i)));
  }
}

/**
 * @brief The generated project's datasets keep the units the live channels showed.
 */
void QuickPlotRoundTripTest::titlesCarryUnits()
{
  const struct {
    QByteArray line;
    QStringList units;
  } cases[] = {
    {"temp:23.5\xC2\xB0"
"C;pressure:1013hPa", {QString::fromUtf8("\xC2\xB0"
 "C"),
 QStringLiteral("hPa")}},
    {             "3.3V,25\xC2\xB0"
             "C,45%",   {QStringLiteral("V"),
   QString::fromUtf8("\xC2\xB0"
   "C"),
   QStringLiteral("%")}},
  };

  for (const auto& entry : cases) {
    DataModel::QuickPlotFormatState st;
    QCOMPARE(st.observe(QByteArrayView(entry.line), m_fields), Outcome::Rebuild);

    const auto project =
      DataModel::QuickPlotProject::build({dataGridFor(st)}, 0, QJsonObject(), st.format());
    const auto group    = project.value(Keys::Groups).toArray().first().toObject();
    const auto datasets = group.value(Keys::Datasets).toArray();
    QCOMPARE(datasets.size(), entry.units.size());
    for (qsizetype i = 0; i < datasets.size(); ++i)
      QCOMPARE(datasets.at(i).toObject().value(Keys::Units).toString(), entry.units.at(i));
  }
}

QTEST_APPLESS_MAIN(QuickPlotRoundTripTest)

#include "tst_quickplot_roundtrip.moc"
