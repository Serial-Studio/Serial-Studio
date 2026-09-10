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

#include <map>
#include <memory>
#include <QTest>
#include <vector>

#include "Core/DataModel/DataBlock.h"
#include "Core/DataModel/ExportSchema.h"
#include "Core/DataModel/Frame.h"
#include "MQTT/CsvExpansion.h"

// Every test function here is self-contained: the transform under test carries no state, so Qt
// Test's declaration-order execution is never load-bearing.

/**
 * @brief The MQTT publisher's dashboard-payload transform: RFC 4180 escaping, the column label the
 *        CSV header and the Sparkplug registry share, header/row assembly, and the block-to-frame
 *        materialisation that feeds both dashboard modes.
 */
class TstMqttCsvExpansion : public QObject {
  Q_OBJECT

private slots:
  void escapeCsvField_data();
  void escapeCsvField();

  void columnLabel_data();
  void columnLabel();

  void headerFromSchema();
  void headerIsEmptyWithoutColumns();
  void headerFollowsShapeChange();

  void rowFollowsSchemaOrder();
  void rowKeepsMissingValuesAligned();

  void expandsEverySample();
  void expandsHonoursSampleCap();
  void expandsSkipsUnknownSource();
  void expandsCarriesTextColumns();
};

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a one-group frame whose datasets carry the given ids and titles.
 */
static DataModel::Frame makeFrame(const QString& groupTitle, const QList<QPair<int, QString>>& sets)
{
  DataModel::Frame frame;
  frame.title = QStringLiteral("Telemetry");

  DataModel::Group group;
  group.groupId   = 0;
  group.groupType = DataModel::GroupType::Input;
  group.title     = groupTitle;

  for (const auto& entry : sets) {
    DataModel::Dataset dataset;
    dataset.uniqueId  = entry.first;
    dataset.title     = entry.second;
    dataset.isNumeric = true;
    group.datasets.push_back(dataset);
  }

  frame.groups.push_back(group);
  return frame;
}

/**
 * @brief Builds a numeric block: one column per unique id, one row per sample vector entry.
 */
static DataModel::DataBlockPtr makeBlock(int sourceId,
                                         const QList<int>& uniqueIds,
                                         const QList<QList<double>>& columnValues)
{
  auto block      = std::make_shared<DataModel::DataBlock>();
  block->sourceId = sourceId;
  block->samples  = columnValues.isEmpty() ? 0 : columnValues.first().size();
  block->dt       = std::chrono::nanoseconds(1000);
  block->t0       = DataModel::DataBlock::SteadyClock::now();

  for (int i = 0; i < uniqueIds.size(); ++i) {
    DataModel::BlockColumn column;
    column.uniqueId = uniqueIds.at(i);
    column.hasText  = false;
    for (const double value : columnValues.at(i))
      column.values.push_back(value);

    block->columns.push_back(std::move(column));
  }

  return block;
}

//--------------------------------------------------------------------------------------------------
// escapeCsvField
//--------------------------------------------------------------------------------------------------

void TstMqttCsvExpansion::escapeCsvField_data()
{
  QTest::addColumn<QString>("field");
  QTest::addColumn<QString>("expected");

  QTest::newRow("empty") << QString() << QString();
  QTest::newRow("plain") << QStringLiteral("42.5") << QStringLiteral("42.5");
  QTest::newRow("spaces are not delimiters")
    << QStringLiteral("hello world") << QStringLiteral("hello world");
  QTest::newRow("comma") << QStringLiteral("a,b") << QStringLiteral("\"a,b\"");
  QTest::newRow("quote") << QStringLiteral("say \"hi\"") << QStringLiteral("\"say \"\"hi\"\"\"");
  QTest::newRow("newline") << QStringLiteral("a\nb") << QStringLiteral("\"a\nb\"");
  QTest::newRow("carriage return") << QStringLiteral("a\rb") << QStringLiteral("\"a\rb\"");
  QTest::newRow("tab") << QStringLiteral("a\tb") << QStringLiteral("\"a\tb\"");
  QTest::newRow("quote and comma") << QStringLiteral("a\",b") << QStringLiteral("\"a\"\",b\"");
  QTest::newRow("unicode passes through")
    << QStringLiteral("温度 25°C") << QStringLiteral("温度 25°C");
  QTest::newRow("unicode with comma") << QStringLiteral("温度,°C") << QStringLiteral("\"温度,°C\"");
  QTest::newRow("only a quote") << QStringLiteral("\"") << QStringLiteral("\"\"\"\"");
}

/**
 * @brief A field is quoted exactly when it carries a delimiter, and an embedded quote is doubled
 *        inside the quoted form; everything else, unicode included, is published verbatim.
 */
void TstMqttCsvExpansion::escapeCsvField()
{
  QFETCH(QString, field);
  QFETCH(QString, expected);

  QCOMPARE(MQTT::escapeCsvField(field), expected);
}

//--------------------------------------------------------------------------------------------------
// csvColumnLabel
//--------------------------------------------------------------------------------------------------

void TstMqttCsvExpansion::columnLabel_data()
{
  QTest::addColumn<QString>("sourceTitle");
  QTest::addColumn<QString>("groupTitle");
  QTest::addColumn<QString>("title");
  QTest::addColumn<QString>("expected");

  QTest::newRow("single source") << QString() << QStringLiteral("IMU") << QStringLiteral("Roll")
                                 << QStringLiteral("IMU/Roll");
  QTest::newRow("multi source") << QStringLiteral("Bench") << QStringLiteral("IMU")
                                << QStringLiteral("Roll") << QStringLiteral("Bench/IMU/Roll");
  QTest::newRow("whitespace is simplified")
    << QString() << QStringLiteral("  IMU ") << QStringLiteral("Roll  Rate")
    << QStringLiteral("IMU /Roll Rate");
  QTest::newRow("empty group") << QString() << QString() << QStringLiteral("Roll")
                               << QStringLiteral("/Roll");
}

/**
 * @brief The label is "group/dataset", prefixed with the source title only when the project reads
 *        from more than one source -- the identity the Sparkplug registry publishes under too.
 */
void TstMqttCsvExpansion::columnLabel()
{
  QFETCH(QString, sourceTitle);
  QFETCH(QString, groupTitle);
  QFETCH(QString, title);
  QFETCH(QString, expected);

  DataModel::ExportColumn column;
  column.uniqueId    = 1;
  column.sourceTitle = sourceTitle;
  column.groupTitle  = groupTitle;
  column.title       = title;

  QCOMPARE(MQTT::csvColumnLabel(column), expected);
}

//--------------------------------------------------------------------------------------------------
// Header assembly
//--------------------------------------------------------------------------------------------------

/**
 * @brief The header names every column in schema order and escapes a label that carries a
 *        delimiter, so a group title with a comma cannot shift the columns under it.
 */
void TstMqttCsvExpansion::headerFromSchema()
{
  const auto frame  = makeFrame(QStringLiteral("Power, DC"),
                                {
                                 {1, QStringLiteral("Voltage")},
                                 {2, QStringLiteral("Current")}
  });
  const auto schema = DataModel::buildExportSchema(frame);
  QCOMPARE(schema.columns.size(), std::size_t(2));

  const auto header = MQTT::buildCsvHeader(schema);
  QCOMPARE(header, QByteArray("\"Power, DC/Voltage\",\"Power, DC/Current\"\n"));
}

/**
 * @brief A schema with no columns publishes nothing: a lone newline would be indistinguishable
 *        from an empty frame on the retained header topic.
 */
void TstMqttCsvExpansion::headerIsEmptyWithoutColumns()
{
  const DataModel::ExportSchema schema;
  QVERIFY(MQTT::buildCsvHeader(schema).isEmpty());
}

/**
 * @brief Adding a dataset regenerates a header that is one column wider, which is what the
 *        publisher retains when a project's shape changes mid-session.
 */
void TstMqttCsvExpansion::headerFollowsShapeChange()
{
  const auto before  = makeFrame(QStringLiteral("IMU"),
                                 {
                                  {1, QStringLiteral("Roll")}
  });
  const auto headerA = MQTT::buildCsvHeader(DataModel::buildExportSchema(before));
  QCOMPARE(headerA, QByteArray("IMU/Roll\n"));

  const auto after   = makeFrame(QStringLiteral("IMU"),
                                 {
                                 {1,  QStringLiteral("Roll")},
                                 {2, QStringLiteral("Pitch")}
  });
  const auto headerB = MQTT::buildCsvHeader(DataModel::buildExportSchema(after));
  QCOMPARE(headerB, QByteArray("IMU/Roll,IMU/Pitch\n"));
  QVERIFY(headerA != headerB);
}

//--------------------------------------------------------------------------------------------------
// Row assembly
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fields follow schema order rather than the order values were latched in, and each is
 *        escaped on its own; rows append so a batch is one publish.
 */
void TstMqttCsvExpansion::rowFollowsSchemaOrder()
{
  const auto frame  = makeFrame(QStringLiteral("IMU"),
                                {
                                 {1,  QStringLiteral("Roll")},
                                 {2, QStringLiteral("Pitch")}
  });
  const auto schema = DataModel::buildExportSchema(frame);

  QMap<int, QString> values;
  values.insert(2, QStringLiteral("b,2"));
  values.insert(1, QStringLiteral("a1"));

  QByteArray out;
  MQTT::appendCsvRow(out, schema, values);
  values.insert(1, QStringLiteral("a2"));
  MQTT::appendCsvRow(out, schema, values);

  QCOMPARE(out, QByteArray("a1,\"b,2\"\na2,\"b,2\"\n"));
}

/**
 * @brief A dataset with no latched value writes an empty field instead of shifting the row: the
 *        header the broker retains stays valid for every row published under it.
 */
void TstMqttCsvExpansion::rowKeepsMissingValuesAligned()
{
  const auto frame = makeFrame(
    QStringLiteral("IMU"),
    {
      {1,  QStringLiteral("Roll")},
      {2, QStringLiteral("Pitch")},
      {3,   QStringLiteral("Yaw")}
  });
  const auto schema = DataModel::buildExportSchema(frame);

  QMap<int, QString> values;
  values.insert(1, QStringLiteral("1"));
  values.insert(3, QStringLiteral("3"));

  QByteArray out;
  MQTT::appendCsvRow(out, schema, values);

  QCOMPARE(out, QByteArray("1,,3\n"));
  QCOMPARE(out.count(','), qsizetype(2));
}

//--------------------------------------------------------------------------------------------------
// Block expansion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every sample in the block becomes one frame carrying that sample's values, in arrival
 *        order: MQTT publishes frame-shaped payloads, so a batch is materialised, never summarised.
 */
void TstMqttCsvExpansion::expandsEverySample()
{
  const auto frame = makeFrame(QStringLiteral("IMU"),
                               {
                                 {1,  QStringLiteral("Roll")},
                                 {2, QStringLiteral("Pitch")}
  });

  std::map<int, DataModel::FrameTemplate> templates;
  DataModel::bind_frame_template(templates[0], frame);

  const auto block = makeBlock(0,
                               {
                                 1, 2
  },
                               {{1.0, 2.0, 3.0}, {10.0, 20.0, 30.0}});

  std::vector<DataModel::Frame> out;
  MQTT::expandBlocks({block}, templates, 4096, out);

  QCOMPARE(out.size(), std::size_t(3));
  for (std::size_t i = 0; i < out.size(); ++i) {
    QCOMPARE(out[i].groups.size(), std::size_t(1));
    QCOMPARE(out[i].groups[0].datasets.size(), std::size_t(2));
    QCOMPARE(out[i].groups[0].datasets[0].numericValue, double(i) + 1.0);
    QCOMPARE(out[i].groups[0].datasets[1].numericValue, (double(i) + 1.0) * 10.0);
  }

  QCOMPARE(out[2].groups[0].datasets[0].value, QStringLiteral("3"));
}

/**
 * @brief The cap is a hard stop, not a hint: a dense source can present far more samples in one
 *        batch than a live feed should publish, and the excess is dropped rather than queued.
 */
void TstMqttCsvExpansion::expandsHonoursSampleCap()
{
  const auto frame = makeFrame(QStringLiteral("IMU"),
                               {
                                 {1, QStringLiteral("Roll")}
  });

  std::map<int, DataModel::FrameTemplate> templates;
  DataModel::bind_frame_template(templates[0], frame);

  const auto block = makeBlock(0,
                               {
                                 1
  },
                               {{1.0, 2.0, 3.0, 4.0, 5.0}});

  std::vector<DataModel::Frame> out;
  MQTT::expandBlocks({block}, templates, 2, out);

  QCOMPARE(out.size(), std::size_t(2));
  QCOMPARE(out[1].groups[0].datasets[0].numericValue, 2.0);
}

/**
 * @brief A block whose source has published no structure yet is skipped: the template is the
 *        published shape, and a payload may not invent fields the subscriber never saw declared.
 */
void TstMqttCsvExpansion::expandsSkipsUnknownSource()
{
  const auto frame = makeFrame(QStringLiteral("IMU"),
                               {
                                 {1, QStringLiteral("Roll")}
  });

  std::map<int, DataModel::FrameTemplate> templates;
  DataModel::bind_frame_template(templates[0], frame);

  const auto known   = makeBlock(0, {1}, {{7.0}});
  const auto unknown = makeBlock(9, {1}, {{8.0}});

  std::vector<DataModel::Frame> out;
  MQTT::expandBlocks({unknown, known}, templates, 4096, out);

  QCOMPARE(out.size(), std::size_t(1));
  QCOMPARE(out[0].groups[0].datasets[0].numericValue, 7.0);
}

/**
 * @brief A text column publishes its string sample and its parsed-as-number verdict, so a dataset
 *        that failed to parse is not published as a silent zero.
 */
void TstMqttCsvExpansion::expandsCarriesTextColumns()
{
  const auto frame = makeFrame(QStringLiteral("Status"),
                               {
                                 {1, QStringLiteral("State")}
  });

  std::map<int, DataModel::FrameTemplate> templates;
  DataModel::bind_frame_template(templates[0], frame);

  auto block      = std::make_shared<DataModel::DataBlock>();
  block->sourceId = 0;
  block->samples  = 2;
  block->dt       = std::chrono::nanoseconds(1000);
  block->t0       = DataModel::DataBlock::SteadyClock::now();

  DataModel::BlockColumn column;
  column.uniqueId = 1;
  column.hasText  = true;
  column.values   = {0.0, 1.0};
  column.text     = {QStringLiteral("offline"), QStringLiteral("1")};
  column.numeric  = {0, 1};
  block->columns.push_back(std::move(column));

  std::vector<DataModel::Frame> out;
  MQTT::expandBlocks({block}, templates, 4096, out);

  QCOMPARE(out.size(), std::size_t(2));
  QCOMPARE(out[0].groups[0].datasets[0].value, QStringLiteral("offline"));
  QCOMPARE(out[0].groups[0].datasets[0].isNumeric, false);
  QCOMPARE(out[1].groups[0].datasets[0].value, QStringLiteral("1"));
  QCOMPARE(out[1].groups[0].datasets[0].isNumeric, true);
}

QTEST_APPLESS_MAIN(TstMqttCsvExpansion)

#include "tst_mqtt_csv_expansion.moc"
