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

#include <cmath>
#include <QTest>

#include "DataModel/Scripting/JsonLeafScanner.h"
#include "DataModel/Scripting/LineFormat.h"

// Quick Plot's line grammar (spec 0106): separator and decimal-comma detection, name:value pairs,
// units, the comment wrapper, the number grammar and the order-preserving JSON leaf scanner.

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief A span copied into an owning array so QCOMPARE prints it.
 */
[[nodiscard]] static QByteArray bytes(QByteArrayView view)
{
  return QByteArray(view.data(), view.size());
}

/**
 * @brief Adds one detection row; a NaN in @p numbers marks a field that must not be numeric.
 */
static void addDetectRow(const QByteArray& line,
                         int separator,
                         bool decimal_comma,
                         bool named,
                         bool wrapper,
                         const QStringList& names,
                         const QList<double>& numbers,
                         const QStringList& units = QStringList())
{
  QTest::newRow(line.constData()) << line << separator << decimal_comma << named << wrapper << names
                                  << numbers << units;
}

/**
 * @brief The JSON path of every leaf, in scan order.
 */
[[nodiscard]] static QStringList leafPaths(const DataModel::LineFields& fields)
{
  QStringList paths;
  for (qsizetype i = 0; i < fields.count; ++i)
    paths << QString::fromLatin1(bytes(fields.items[static_cast<size_t>(i)].name));

  return paths;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class LineFormatTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void detectSingleLines_data();
  void detectSingleLines();
  void detectFieldDetails();
  void detectRejectsText_data();
  void detectRejectsText();
  void numberGrammar_data();
  void numberGrammar();
  void fieldGrammar();
  void jsonScanner();
  void jsonScannerRejectsTrailingBackslashAndCommaKeys();

private:
  DataModel::LineFields m_fields;
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void LineFormatTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief Rows of line, separator, decimal comma, named, wrapper, names, numbers and units.
 */
void LineFormatTest::detectSingleLines_data()
{
  QTest::addColumn<QByteArray>("line");
  QTest::addColumn<int>("separator");
  QTest::addColumn<bool>("decimalComma");
  QTest::addColumn<bool>("named");
  QTest::addColumn<bool>("wrapper");
  QTest::addColumn<QStringList>("names");
  QTest::addColumn<QList<double>>("numbers");
  QTest::addColumn<QStringList>("units");

  const double nan = qQNaN();
  const QStringList th{"temp", "hum"};
  const QByteArray deg("\xC2\xB0");
  addDetectRow("23.5,60,1013", ',', false, false, false, {}, {23.5, 60, 1013});
  addDetectRow("23.5, 60, 1013", ',', false, false, false, {}, {23.5, 60, 1013});
  addDetectRow("23.5;60;1013", ';', true, false, false, {}, {23.5, 60, 1013});
  addDetectRow("23,5;60;1013,2", ';', true, false, false, {}, {23.5, 60, 1013.2});
  addDetectRow("23.5\t60\t1013", '\t', false, false, false, {}, {23.5, 60, 1013});
  addDetectRow("23.5 60 1013", ' ', true, false, false, {}, {23.5, 60, 1013});
  addDetectRow("23,5 60,2 1013", ' ', true, false, false, {}, {23.5, 60.2, 1013});
  addDetectRow("temp:23.5,hum:60", ',', false, true, false, th, {23.5, 60});
  addDetectRow("temp: 23.5, hum: 60", ',', false, true, false, th, {23.5, 60});
  addDetectRow("temp=23.5;hum=60", ';', true, true, false, th, {23.5, 60});
  addDetectRow("temp:23.5 hum:60", ' ', true, true, false, th, {23.5, 60});
  addDetectRow("temp: 23.5 hum: 60", ' ', true, true, false, th, {23.5, 60});
  addDetectRow("temp:23.5" + deg + "C;pressure:1013hPa",
               ';',
               true,
               true,
               false,
               {"temp", "pressure"},
               {23.5, 1013},
               {QString::fromUtf8(deg + "C"), "hPa"});
  addDetectRow("3.3V,25" + deg + "C,45%",
               ',',
               false,
               false,
               false,
               {},
               {3.3, 25, 45},
               {"V", QString::fromUtf8(deg + "C"), "%"});
  addDetectRow("{\"t\":23.5,\"h\":60}", 0, false, true, false, {"t", "h"}, {23.5, 60});
  addDetectRow(
    "{\"t\":23.5,\"ok\":true,\"n\":\"x\",\"h\":60}", 0, false, true, false, {"t", "h"}, {23.5, 60});
  addDetectRow("{\"imu\":{\"ax\":0.1,\"g\":[1,2]},\"t\":5}",
               0,
               false,
               true,
               false,
               {"imu.ax", "imu.g[0]", "imu.g[1]", "t"},
               {0.1, 1, 2, 5});
  addDetectRow("{\"acc\":[1,2]}", 0, false, true, false, {"acc[0]", "acc[1]"}, {1, 2});
  addDetectRow("/*23.5,60,1013*/", ',', false, false, true, {}, {23.5, 60, 1013});
  addDetectRow("-12.5,1.5e3,+7", ',', false, false, false, {}, {-12.5, 1500, 7});
  addDetectRow("23.5,60,1013,", ',', false, false, false, {}, {23.5, 60, 1013, nan});
  addDetectRow("12:30:45,23.5,60", ',', false, false, false, {"", "", ""}, {nan, 23.5, 60});
  addDetectRow("Temp:23.5\tHum:60", '\t', false, true, false, {"Temp", "Hum"}, {23.5, 60});
  addDetectRow("temp:23.5,60", ',', false, true, false, {"temp", ""}, {23.5, 60});
  addDetectRow("temp:23,5;hum:60", ';', true, true, false, th, {23.5, 60});
  addDetectRow("Temp Sensor: 23.5, Humidity: 60",
               ',',
               false,
               true,
               false,
               {"Temp Sensor", "Humidity"},
               {23.5, 60});
  addDetectRow("temp:23.5,status:OK", ',', false, true, false, {"temp", "status"}, {23.5, nan});
  addDetectRow("uptime=1234", 0, true, true, false, {"uptime"}, {1234});
}

/**
 * @brief Each line detects as the listed format and tokenizes into the listed fields.
 */
void LineFormatTest::detectSingleLines()
{
  QFETCH(QByteArray, line);
  QFETCH(int, separator);
  QFETCH(bool, decimalComma);
  QFETCH(bool, named);
  QFETCH(bool, wrapper);
  QFETCH(QStringList, names);
  QFETCH(QList<double>, numbers);
  QFETCH(QStringList, units);

  DataModel::LineFormat format;
  QVERIFY(DataModel::detectLineFormat(line, format, m_fields));
  const bool json = line.startsWith('{');
  QCOMPARE(format.kind == DataModel::LineFormatKind::Json, json);
  QCOMPARE(static_cast<int>(format.separator), separator);
  if (!json) {
    QCOMPARE(format.decimalComma, decimalComma);
    QCOMPARE(format.wrapper, wrapper);
  }

  QCOMPARE(format.named, named);
  QCOMPARE(m_fields.count, static_cast<qsizetype>(numbers.size()));
  for (qsizetype i = 0; i < m_fields.count; ++i) {
    const auto& f = m_fields.items[static_cast<size_t>(i)];
    if (std::isnan(numbers[i])) {
      QVERIFY(!f.numeric);
    } else {
      QVERIFY(f.numeric);
      QCOMPARE(f.number, numbers[i]);
    }

    if (!names.isEmpty())
      QCOMPARE(QString::fromLatin1(bytes(f.name)), names[i]);

    if (!units.isEmpty())
      QCOMPARE(QString::fromUtf8(bytes(f.unit)), units[i]);
  }
}

/**
 * @brief Tokens and values of the fields that are not plain numbers.
 */
void LineFormatTest::detectFieldDetails()
{
  DataModel::LineFormat format;
  QVERIFY(DataModel::detectLineFormat("23.5,60,1013,", format, m_fields));
  QCOMPARE(m_fields.count, qsizetype(4));
  QVERIFY(m_fields.items[3].value.isEmpty());

  QVERIFY(DataModel::detectLineFormat("12:30:45,23.5,60", format, m_fields));
  QCOMPARE(bytes(m_fields.items[0].token), QByteArray("12:30:45"));
  QVERIFY(m_fields.items[0].name.isEmpty());

  QVERIFY(DataModel::detectLineFormat("temp:23.5,status:OK", format, m_fields));
  QCOMPARE(bytes(m_fields.items[1].value), QByteArray("OK"));
}

/**
 * @brief Rows of lines that are text, not telemetry.
 */
void LineFormatTest::detectRejectsText_data()
{
  QTest::addColumn<QByteArray>("line");

  const QList<QByteArray> lines{"0x1F,0x20",
                                "hello",
                                "WiFi connected",
                                "Connected in 1234 ms",
                                "IP: 192.168.1.42",
                                "",
                                "{}",
                                "[1,2]"};
  for (const auto& line : lines)
    QTest::newRow(line.isEmpty() ? "empty" : line.constData()) << line;
}

/**
 * @brief Text never establishes a format.
 */
void LineFormatTest::detectRejectsText()
{
  QFETCH(QByteArray, line);

  DataModel::LineFormat format;
  QVERIFY(!DataModel::detectLineFormat(line, format, m_fields));
}

/**
 * @brief Rows of text, decimal comma, verdict, number and unit.
 */
void LineFormatTest::numberGrammar_data()
{
  QTest::addColumn<QByteArray>("text");
  QTest::addColumn<bool>("decimalComma");
  QTest::addColumn<bool>("ok");
  QTest::addColumn<double>("number");
  QTest::addColumn<QByteArray>("unit");

  QTest::newRow("plus") << QByteArray("+7") << false << true << 7.0 << QByteArray();
  QTest::newRow("exponent") << QByteArray("1.5e3") << false << true << 1500.0 << QByteArray();
  QTest::newRow("negative") << QByteArray("-12.5") << false << true << -12.5 << QByteArray();
  QTest::newRow("comma") << QByteArray("23,5") << true << true << 23.5 << QByteArray();
  QTest::newRow("comma-off") << QByteArray("23,5") << false << false << 0.0 << QByteArray();
  QTest::newRow("hex") << QByteArray("0x1F") << false << false << 0.0 << QByteArray();
  QTest::newRow("clock") << QByteArray("12:30:45") << false << false << 0.0 << QByteArray();
  QTest::newRow("hpa") << QByteArray("1013hPa") << false << true << 1013.0 << QByteArray("hPa");
  QTest::newRow("percent") << QByteArray("45%") << false << true << 45.0 << QByteArray("%");
  QTest::newRow("spaced") << QByteArray("23.5 C") << false << true << 23.5 << QByteArray("C");
  QTest::newRow("volt") << QByteArray("3.3V") << false << true << 3.3 << QByteArray("V");
  QTest::newRow("long-unit") << QByteArray("5abcdefghi") << false << false << 0.0 << QByteArray();
  QTest::newRow("inf") << QByteArray("inf") << false << false << 0.0 << QByteArray();
  QTest::newRow("dot") << QByteArray(".") << false << false << 0.0 << QByteArray();
  QTest::newRow("empty") << QByteArray() << false << false << 0.0 << QByteArray();
  QTest::newRow("thousands") << QByteArray("1,234.5") << true << false << 0.0 << QByteArray();
}

/**
 * @brief The number grammar accepts and rejects the listed texts.
 */
void LineFormatTest::numberGrammar()
{
  QFETCH(QByteArray, text);
  QFETCH(bool, decimalComma);
  QFETCH(bool, ok);
  QFETCH(double, number);
  QFETCH(QByteArray, unit);

  double parsed = 0.0;
  QByteArrayView parsed_unit;
  QCOMPARE(DataModel::parseLineNumber(text, decimalComma, parsed, parsed_unit), ok);
  if (!ok)
    return;

  QCOMPARE(parsed, number);
  QCOMPARE(bytes(parsed_unit), unit);
}

/**
 * @brief Name:value splitting only takes a well-formed name prefix.
 */
void LineFormatTest::fieldGrammar()
{
  DataModel::LineField f;
  QVERIFY(DataModel::parseLineField("temp:23.5", false, f));
  QCOMPARE(bytes(f.name), QByteArray("temp"));
  QCOMPARE(bytes(f.token), QByteArray("temp:23.5"));
  QVERIFY(f.numeric);

  f = DataModel::LineField();
  QVERIFY(!DataModel::parseLineField("http://x", false, f));
  QCOMPARE(bytes(f.name), QByteArray("http"));
  QCOMPARE(bytes(f.value), QByteArray("//x"));

  f = DataModel::LineField();
  QVERIFY(!DataModel::parseLineField("a=b=c", false, f));
  QCOMPARE(bytes(f.name), QByteArray("a"));
  QCOMPARE(bytes(f.value), QByteArray("b=c"));

  f = DataModel::LineField();
  QVERIFY(DataModel::parseLineField(" 42 ", false, f));
  QVERIFY(f.name.isEmpty());
  QCOMPARE(f.number, 42.0);
  QCOMPARE(bytes(f.token), QByteArray("42"));

  f = DataModel::LineField();
  QVERIFY(!DataModel::parseLineField("12:30:45", false, f));
  QVERIFY(f.name.isEmpty());
  QVERIFY(!f.numeric);
}

/**
 * @brief Malformed input fails with no fields; depth is capped; order is the text order.
 */
void LineFormatTest::jsonScanner()
{
  const QList<QByteArray> bad{
    "{\"a\":",
    "{\"a\":[1,}",
    "{\"a\":1} x",
    "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"f\":{\"g\":{\"h\":{\"i\":1}}}}}"
    "}}}}}"};
  for (const auto& text : bad) {
    QVERIFY2(!DataModel::scanJsonLeaves(text, m_fields), text.constData());
    QCOMPARE(m_fields.count, qsizetype(0));
  }

  QVERIFY(DataModel::scanJsonLeaves("{\"s\":\"x\\\"y\",\"n\":1}", m_fields));
  QCOMPARE(m_fields.count, qsizetype(1));
  QCOMPARE(bytes(m_fields.items[0].name), QByteArray("n"));
  QCOMPARE(m_fields.items[0].number, 1.0);

  QVERIFY(DataModel::scanJsonLeaves("{\"a\":1}  ", m_fields));
  QCOMPARE(m_fields.count, qsizetype(1));

  QVERIFY(DataModel::scanJsonLeaves(
    "{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":{\"f\":{\"g\":{\"h\":1}}}}}}}}", m_fields));
  QCOMPARE(leafPaths(m_fields), QStringList{"a.b.c.d.e.f.g.h"});
}

/**
 * @brief An open string ending in a backslash and a comma in a key are refused; the scratch
 *        stays reusable afterwards.
 */
void LineFormatTest::jsonScannerRejectsTrailingBackslashAndCommaKeys()
{
  const QList<QByteArray> bad{"{\"a\\", "{\"a\\\"", "{\"a,b\":1,\"c\":2}"};
  for (const auto& text : bad) {
    QVERIFY2(!DataModel::scanJsonLeaves(text, m_fields), text.constData());
    QCOMPARE(m_fields.count, qsizetype(0));
  }

  QVERIFY(DataModel::scanJsonLeaves("{\"a\":1}", m_fields));
  QCOMPARE(m_fields.count, qsizetype(1));
}

QTEST_APPLESS_MAIN(LineFormatTest)

#include "tst_line_format.moc"
