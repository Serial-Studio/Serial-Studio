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

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QTest>

#include "DataModel/Scripting/LineFormat.h"

// Declaration inertness (spec 0111 AC8, R11): every golden declaration is a text line to spec
// 0106's grammar. It never establishes a format and yields no number under any separator, decimal
// comma or the JSON scanner, so an app with 0106 but not 0111 shows it in the console and leaves
// the dashboard alone. The counter-cases break the escape rule and must yield a number.

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every candidate format of spec 0106: five separators with and without a decimal comma,
 *        plus the JSON leaf scanner.
 */
[[nodiscard]] static QList<DataModel::LineFormat> everyFormat()
{
  QList<DataModel::LineFormat> formats;
  for (const char separator : {',', ';', '\t', ' ', '\0'}) {
    for (const bool decimalComma : {false, true}) {
      DataModel::LineFormat format;
      format.separator    = separator;
      format.decimalComma = decimalComma;
      formats.append(format);
    }
  }

  DataModel::LineFormat json;
  json.kind      = DataModel::LineFormatKind::Json;
  json.separator = '\0';
  formats.append(json);
  return formats;
}

/**
 * @brief The golden declaration lines, one per fixture file.
 */
[[nodiscard]] static QList<QByteArray> goldenLines()
{
  QList<QByteArray> lines;
  const QDir dir(QStringLiteral(SS_DECLARATION_GOLDEN_DIR));
  for (const auto& name : dir.entryList({QStringLiteral("*.decl")}, QDir::Files, QDir::Name)) {
    QFile file(dir.filePath(name));
    if (file.open(QIODevice::ReadOnly))
      lines.append(file.readLine().trimmed());
  }

  return lines;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class DeclarationInertTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void goldenLinesAreText_data();
  void goldenLinesAreText();
  void brokenEscapesYieldNumbers_data();
  void brokenEscapesYieldNumbers();

private:
  [[nodiscard]] qsizetype mostNumbers(const QByteArray& line);

  DataModel::LineFields m_fields;
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void DeclarationInertTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief The most numeric fields @p line yields under any one candidate format.
 */
qsizetype DeclarationInertTest::mostNumbers(const QByteArray& line)
{
  qsizetype most = 0;
  for (const auto& format : everyFormat()) {
    (void)DataModel::tokenizeLine(QByteArrayView(line), format, m_fields);
    most = std::max(most, DataModel::numericLineFieldCount(m_fields));
  }

  return most;
}

/**
 * @brief One row per golden fixture; the fixture set must not be empty.
 */
void DeclarationInertTest::goldenLinesAreText_data()
{
  QTest::addColumn<QByteArray>("line");
  const auto lines = goldenLines();
  QVERIFY(lines.size() >= 4);
  for (const auto& line : lines)
    QTest::newRow(line.left(32).constData()) << line;
}

/**
 * @brief Detection refuses the line and no format finds a number in it.
 */
void DeclarationInertTest::goldenLinesAreText()
{
  QFETCH(QByteArray, line);
  QVERIFY(line.startsWith("#ss1{"));
  QVERIFY(line.size() < 1024);

  DataModel::LineFormat detected;
  QVERIFY(!DataModel::detectLineFormat(QByteArrayView(line), detected, m_fields));
  QCOMPARE(mostNumbers(line), qsizetype(0));
}

/**
 * @brief Lines that leave a separator or a name delimiter raw before a digit.
 */
void DeclarationInertTest::brokenEscapesYieldNumbers_data()
{
  QTest::addColumn<QByteArray>("line");
  QTest::newRow("space before digit") << QByteArray("#ss1{\"title\":\"a 1 b\"}");
  QTest::newRow("comma before digit") << QByteArray("#ss1{\"title\":\"a,2,b\"}");
  QTest::newRow("semicolon before sign") << QByteArray("#ss1{\"title\":\"a;-3;b\"}");
  QTest::newRow("name equals digit") << QByteArray("#ss1{\"title\":\"a x=5 b\"}");
  QTest::newRow("name colon digit") << QByteArray("#ss1{\"title\":\"a x:5 b\"}");
  QTest::newRow("bare array number") << QByteArray("#ss1{\"t\":[\"a\",2,3]}");
}

/**
 * @brief The test can fail: each counter-case yields a number under some format.
 */
void DeclarationInertTest::brokenEscapesYieldNumbers()
{
  QFETCH(QByteArray, line);
  QVERIFY(mostNumbers(line) > 0);
}

QTEST_APPLESS_MAIN(DeclarationInertTest)

#include "tst_declaration_inert.moc"
