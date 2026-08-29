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

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTest>

#include "Misc/CLI/CliSpecParsers.h"

/**
 * @brief Validation sweep of the pure command-line spec parsers extracted from Misc::CLI
 *        (spec 0070): the contract every one of them shares is that a rejected token leaves the
 *        caller's out-parameter untouched, so a mistyped option never reaches a driver as a
 *        clamped value. Each slot is self-contained; declaration order is never load-bearing.
 */
class TstCliSpecParsers : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void intOptionRejectsAnUnsetOption();
  void intOptionAcceptsAnInRangeValue();
  void intOptionRejectsOutOfRangeValues();
  void intOptionRejectsANonNumericToken();
  void intOptionAcceptsADegenerateRange();
  void intOptionRejectsAnInvertedRange();

  void modbusTcpAddressDefaultsThePort();
  void modbusTcpAddressReadsAnExplicitPort();
  void modbusTcpAddressKeepsTheDefaultOnABadPort();
  void modbusTcpAddressRejectsThreeFields();

  void registerSpecParsesEveryType();
  void registerSpecRejectsAWrongFieldCount();
  void registerSpecRejectsAnUnknownType();
  void registerSpecRejectsAnOutOfRangeCount();

  void serialIndexTablesMapKnownTokens();
  void serialIndexTablesRejectUnknownTokens();

private:
  static bool parse(QCommandLineParser& parser,
                    const QCommandLineOption& opt,
                    const QString& value);
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, so the inverted-range guard is
 *        observable from a debug build.
 */
void TstCliSpecParsers::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief Registers @p opt on @p parser and feeds it one `--<name>=<value>` argument. The joined
 *        form is deliberate: a separate token starting with '-' (a negative rack number) would be
 *        read as another option rather than as this one's value.
 */
bool TstCliSpecParsers::parse(QCommandLineParser& parser,
                              const QCommandLineOption& opt,
                              const QString& value)
{
  if (!parser.addOption(opt))
    return false;

  const QString arg = QStringLiteral("--") + opt.names().first() + QStringLiteral("=") + value;
  return parser.parse(QStringList{QStringLiteral("app"), arg});
}

//---------------------------------------------------------------------------------------------------
// parseIntOption
//---------------------------------------------------------------------------------------------------

void TstCliSpecParsers::intOptionRejectsAnUnsetOption()
{
  QCommandLineParser parser;
  const QCommandLineOption opt(QStringLiteral("rack"), QStringLiteral("rack"), QStringLiteral("n"));
  QVERIFY(parser.addOption(opt));
  QVERIFY(parser.parse(QStringList{QStringLiteral("app")}));

  int out = 7;
  QVERIFY(!Misc::CliSpecParsers::parseIntOption(parser, opt, 0, 31, QStringLiteral("rack"), out));
  QCOMPARE(out, 7);
}

void TstCliSpecParsers::intOptionAcceptsAnInRangeValue()
{
  QCommandLineParser parser;
  const QCommandLineOption opt(QStringLiteral("rack"), QStringLiteral("rack"), QStringLiteral("n"));
  QVERIFY(parse(parser, opt, QStringLiteral("5")));

  int out = -1;
  QVERIFY(Misc::CliSpecParsers::parseIntOption(parser, opt, 0, 31, QStringLiteral("rack"), out));
  QCOMPARE(out, 5);
}

void TstCliSpecParsers::intOptionRejectsOutOfRangeValues()
{
  for (const QString& value : {QStringLiteral("-1"), QStringLiteral("32")}) {
    QCommandLineParser parser;
    const QCommandLineOption opt(
      QStringLiteral("rack"), QStringLiteral("rack"), QStringLiteral("n"));
    QVERIFY(parse(parser, opt, value));

    int out = 4;
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Invalid rack")));
    QVERIFY(!Misc::CliSpecParsers::parseIntOption(parser, opt, 0, 31, QStringLiteral("rack"), out));
    QCOMPARE(out, 4);
  }
}

void TstCliSpecParsers::intOptionRejectsANonNumericToken()
{
  QCommandLineParser parser;
  const QCommandLineOption opt(QStringLiteral("rack"), QStringLiteral("rack"), QStringLiteral("n"));
  QVERIFY(parse(parser, opt, QStringLiteral("two")));

  int out = 3;
  QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Invalid rack")));
  QVERIFY(!Misc::CliSpecParsers::parseIntOption(parser, opt, 0, 31, QStringLiteral("rack"), out));
  QCOMPARE(out, 3);
}

void TstCliSpecParsers::intOptionAcceptsADegenerateRange()
{
  QCommandLineParser parser;
  const QCommandLineOption opt(QStringLiteral("slot"), QStringLiteral("slot"), QStringLiteral("n"));
  QVERIFY(parse(parser, opt, QStringLiteral("1")));

  int out = -1;
  QVERIFY(Misc::CliSpecParsers::parseIntOption(parser, opt, 1, 1, QStringLiteral("slot"), out));
  QCOMPARE(out, 1);
}

void TstCliSpecParsers::intOptionRejectsAnInvertedRange()
{
  QCommandLineParser parser;
  const QCommandLineOption opt(QStringLiteral("slot"), QStringLiteral("slot"), QStringLiteral("n"));
  QVERIFY(parse(parser, opt, QStringLiteral("5")));

  int out = 9;
  QVERIFY(!Misc::CliSpecParsers::parseIntOption(parser, opt, 31, 0, QStringLiteral("slot"), out));
  QCOMPARE(out, 9);
}

//---------------------------------------------------------------------------------------------------
// parseModbusTcpAddress
//---------------------------------------------------------------------------------------------------

void TstCliSpecParsers::modbusTcpAddressDefaultsThePort()
{
  QString host;
  quint16 port = 0;
  QVERIFY(Misc::CliSpecParsers::parseModbusTcpAddress(QStringLiteral("192.168.1.10"), host, port));
  QCOMPARE(host, QStringLiteral("192.168.1.10"));
  QCOMPARE(port, static_cast<quint16>(502));
}

void TstCliSpecParsers::modbusTcpAddressReadsAnExplicitPort()
{
  QString host;
  quint16 port = 0;
  QVERIFY(
    Misc::CliSpecParsers::parseModbusTcpAddress(QStringLiteral("plc.local:1502"), host, port));
  QCOMPARE(host, QStringLiteral("plc.local"));
  QCOMPARE(port, static_cast<quint16>(1502));
}

void TstCliSpecParsers::modbusTcpAddressKeepsTheDefaultOnABadPort()
{
  QString host;
  quint16 port = 0;
  QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Invalid ModBus TCP port")));
  QVERIFY(Misc::CliSpecParsers::parseModbusTcpAddress(QStringLiteral("plc.local:0"), host, port));
  QCOMPARE(host, QStringLiteral("plc.local"));
  QCOMPARE(port, static_cast<quint16>(502));
}

void TstCliSpecParsers::modbusTcpAddressRejectsThreeFields()
{
  QString host = QStringLiteral("untouched");
  quint16 port = 7;
  QVERIFY(!Misc::CliSpecParsers::parseModbusTcpAddress(QStringLiteral("a:b:c"), host, port));
  QCOMPARE(host, QStringLiteral("untouched"));
  QCOMPARE(port, static_cast<quint16>(7));
}

//---------------------------------------------------------------------------------------------------
// parseModbusRegisterSpec
//---------------------------------------------------------------------------------------------------

void TstCliSpecParsers::registerSpecParsesEveryType()
{
  const QStringList names = {QStringLiteral("holding"),
                             QStringLiteral("input"),
                             QStringLiteral("coils"),
                             QStringLiteral("DISCRETE")};

  for (int i = 0; i < names.size(); ++i) {
    quint8 type   = 255;
    quint16 start = 0;
    quint16 count = 0;
    const QString spec =
      names.at(i) + QStringLiteral(":") + QString::number(100 + i) + QStringLiteral(":8");
    QVERIFY(Misc::CliSpecParsers::parseModbusRegisterSpec(spec, type, start, count));
    QCOMPARE(static_cast<int>(type), i);
    QCOMPARE(static_cast<int>(start), 100 + i);
    QCOMPARE(static_cast<int>(count), 8);
  }
}

void TstCliSpecParsers::registerSpecRejectsAWrongFieldCount()
{
  quint8 type   = 9;
  quint16 start = 1;
  quint16 count = 2;
  QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Invalid register format")));
  QVERIFY(!Misc::CliSpecParsers::parseModbusRegisterSpec(
    QStringLiteral("holding:0"), type, start, count));
  QCOMPARE(static_cast<int>(type), 9);
  QCOMPARE(static_cast<int>(start), 1);
  QCOMPARE(static_cast<int>(count), 2);
}

void TstCliSpecParsers::registerSpecRejectsAnUnknownType()
{
  quint8 type   = 9;
  quint16 start = 0;
  quint16 count = 0;
  QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Invalid register type")));
  QVERIFY(!Misc::CliSpecParsers::parseModbusRegisterSpec(
    QStringLiteral("floating:0:4"), type, start, count));
  QCOMPARE(static_cast<int>(type), 9);
}

void TstCliSpecParsers::registerSpecRejectsAnOutOfRangeCount()
{
  for (const QString& spec : {QStringLiteral("input:0:0"), QStringLiteral("input:0:126")}) {
    quint8 type   = 9;
    quint16 start = 0;
    quint16 count = 0;
    QTest::ignoreMessage(QtWarningMsg,
                         QRegularExpression(QStringLiteral("Invalid register specification")));
    QVERIFY(!Misc::CliSpecParsers::parseModbusRegisterSpec(spec, type, start, count));
    QCOMPARE(static_cast<int>(type), 9);
  }
}

//---------------------------------------------------------------------------------------------------
// Serial framing index tables
//---------------------------------------------------------------------------------------------------

void TstCliSpecParsers::serialIndexTablesMapKnownTokens()
{
  QCOMPARE(Misc::CliSpecParsers::modbusParityIndex(QStringLiteral("none")), 0);
  QCOMPARE(Misc::CliSpecParsers::modbusParityIndex(QStringLiteral("even")), 1);
  QCOMPARE(Misc::CliSpecParsers::modbusParityIndex(QStringLiteral("odd")), 2);
  QCOMPARE(Misc::CliSpecParsers::modbusParityIndex(QStringLiteral("space")), 3);
  QCOMPARE(Misc::CliSpecParsers::modbusParityIndex(QStringLiteral("mark")), 4);

  QCOMPARE(Misc::CliSpecParsers::modbusDataBitsIndex(QStringLiteral("5")), 0);
  QCOMPARE(Misc::CliSpecParsers::modbusDataBitsIndex(QStringLiteral("8")), 3);

  QCOMPARE(Misc::CliSpecParsers::modbusStopBitsIndex(QStringLiteral("1")), 0);
  QCOMPARE(Misc::CliSpecParsers::modbusStopBitsIndex(QStringLiteral("1.5")), 1);
  QCOMPARE(Misc::CliSpecParsers::modbusStopBitsIndex(QStringLiteral("2")), 2);
}

void TstCliSpecParsers::serialIndexTablesRejectUnknownTokens()
{
  QCOMPARE(Misc::CliSpecParsers::modbusParityIndex(QStringLiteral("NONE")), -1);
  QCOMPARE(Misc::CliSpecParsers::modbusParityIndex(QString()), -1);
  QCOMPARE(Misc::CliSpecParsers::modbusDataBitsIndex(QStringLiteral("9")), -1);
  QCOMPARE(Misc::CliSpecParsers::modbusStopBitsIndex(QStringLiteral("1.0")), -1);
}

QTEST_APPLESS_MAIN(TstCliSpecParsers)

#include "tst_cli_spec_parsers.moc"
