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

#include <QTest>

#include "IO/Drivers/USB/UsbHex.h"

using namespace IO::Drivers::UsbHex;

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Boundary sweep of the USB Advanced Control composer's text helpers. Everything typed
 *        into the setup packet and the data payload passes through these three functions before
 *        a control transfer is submitted to real hardware, so a parser that accepts junk writes
 *        junk to a device.
 */
class TstUsbHexParsing : public QObject {
  Q_OBJECT

private slots:
  void isHexChar_data();
  void isHexChar();

  void parseHexUInt_data();
  void parseHexUInt();
  void parseHexUInt_overMaxKeepsValue();

  void parseHexBytes_data();
  void parseHexBytes();
  void parseHexBytes_alwaysWritesOk();

  void controlStatusText_data();
  void controlStatusText();
};

//--------------------------------------------------------------------------------------------------
// isHexChar
//--------------------------------------------------------------------------------------------------

void TstUsbHexParsing::isHexChar_data()
{
  QTest::addColumn<QChar>("character");
  QTest::addColumn<bool>("expected");

  QTest::newRow("digit zero") << QChar(u'0') << true;
  QTest::newRow("digit nine") << QChar(u'9') << true;
  QTest::newRow("lowercase a") << QChar(u'a') << true;
  QTest::newRow("lowercase f") << QChar(u'f') << true;
  QTest::newRow("uppercase A") << QChar(u'A') << true;
  QTest::newRow("uppercase F") << QChar(u'F') << true;
  QTest::newRow("letter g") << QChar(u'g') << false;
  QTest::newRow("letter G") << QChar(u'G') << false;
  QTest::newRow("letter x") << QChar(u'x') << false;
  QTest::newRow("space") << QChar(u' ') << false;
  QTest::newRow("colon separator") << QChar(u':') << false;
  QTest::newRow("null character") << QChar(u'\0') << false;
}

/**
 * @brief The accepted alphabet is exactly 0-9, a-f and A-F; every separator a user may type is
 *        rejected here and skipped (whitespace) or refused (anything else) by parseHexBytes.
 */
void TstUsbHexParsing::isHexChar()
{
  QFETCH(QChar, character);
  QFETCH(bool, expected);

  QCOMPARE(IO::Drivers::UsbHex::isHexChar(character), expected);
}

//--------------------------------------------------------------------------------------------------
// parseHexUInt
//--------------------------------------------------------------------------------------------------

void TstUsbHexParsing::parseHexUInt_data()
{
  QTest::addColumn<QString>("text");
  QTest::addColumn<uint>("max");
  QTest::addColumn<bool>("expectedOk");
  QTest::addColumn<uint>("expectedValue");

  QTest::newRow("byte at zero") << QStringLiteral("00") << 0xFFu << true << 0x00u;
  QTest::newRow("byte at max") << QStringLiteral("FF") << 0xFFu << true << 0xFFu;
  QTest::newRow("lowercase digits") << QStringLiteral("ab") << 0xFFu << true << 0xABu;
  QTest::newRow("0x prefix") << QStringLiteral("0xFF") << 0xFFu << true << 0xFFu;
  QTest::newRow("0X prefix uppercase") << QStringLiteral("0X1f") << 0xFFu << true << 0x1Fu;
  QTest::newRow("surrounding whitespace") << QStringLiteral("  1A  ") << 0xFFu << true << 0x1Au;
  QTest::newRow("word at max") << QStringLiteral("FFFF") << 0xFFFFu << true << 0xFFFFu;
  QTest::newRow("word needs no padding") << QStringLiteral("1") << 0xFFFFu << true << 0x1u;

  QTest::newRow("one over max") << QStringLiteral("100") << 0xFFu << false << 0x100u;
  QTest::newRow("word one over max") << QStringLiteral("10000") << 0xFFFFu << false << 0x10000u;
  QTest::newRow("empty") << QString() << 0xFFu << false << 0u;
  QTest::newRow("prefix only") << QStringLiteral("0x") << 0xFFu << false << 0u;
  QTest::newRow("not hex") << QStringLiteral("zz") << 0xFFu << false << 0u;
  QTest::newRow("trailing garbage") << QStringLiteral("1G") << 0xFFu << false << 0u;
  QTest::newRow("negative") << QStringLiteral("-1") << 0xFFu << false << 0u;
  QTest::newRow("inner space") << QStringLiteral("1 A") << 0xFFu << false << 0u;
  QTest::newRow("wider than 32 bits") << QStringLiteral("FFFFFFFFF") << 0xFFFFu << false << 0u;
}

/**
 * @brief Each setup field is bounded by the width the USB setup packet gives it (0xFF for
 *        bmRequestType and bRequest, 0xFFFF for wValue and wIndex), and the 0x prefix is optional
 *        because the composer's placeholder text shows both spellings.
 */
void TstUsbHexParsing::parseHexUInt()
{
  QFETCH(QString, text);
  QFETCH(uint, max);
  QFETCH(bool, expectedOk);
  QFETCH(uint, expectedValue);

  bool ok           = false;
  const uint parsed = IO::Drivers::UsbHex::parseHexUInt(text, max, ok);

  QCOMPARE(ok, expectedOk);
  QCOMPARE(parsed, expectedValue);
}

/**
 * @brief A value that parses but exceeds the field width still comes back: only @c ok says the
 *        field is unusable, so a caller that ignores it would submit a truncated setup packet.
 */
void TstUsbHexParsing::parseHexUInt_overMaxKeepsValue()
{
  bool ok           = true;
  const uint parsed = IO::Drivers::UsbHex::parseHexUInt(QStringLiteral("1FF"), 0xFF, ok);

  QVERIFY(!ok);
  QCOMPARE(parsed, 0x1FFu);
}

//--------------------------------------------------------------------------------------------------
// parseHexBytes
//--------------------------------------------------------------------------------------------------

void TstUsbHexParsing::parseHexBytes_data()
{
  QTest::addColumn<QString>("text");
  QTest::addColumn<bool>("expectedOk");
  QTest::addColumn<QByteArray>("expectedBytes");

  QTest::newRow("empty is an empty payload") << QString() << true << QByteArray();
  QTest::newRow("whitespace only") << QStringLiteral("   ") << true << QByteArray();
  QTest::newRow("single byte") << QStringLiteral("00") << true << QByteArray::fromHex("00");
  QTest::newRow("four bytes") << QStringLiteral("DEADBEEF") << true
                              << QByteArray::fromHex("deadbeef");
  QTest::newRow("lowercase") << QStringLiteral("deadbeef") << true
                             << QByteArray::fromHex("deadbeef");
  QTest::newRow("space separated")
    << QStringLiteral("de ad be ef") << true << QByteArray::fromHex("deadbeef");
  QTest::newRow("tabs and newlines")
    << QStringLiteral("de\tad\nbe") << true << QByteArray::fromHex("deadbe");
  QTest::newRow("leading and trailing spaces")
    << QStringLiteral("  12 34  ") << true << QByteArray::fromHex("1234");

  QTest::newRow("odd digit count") << QStringLiteral("ABC") << false << QByteArray();
  QTest::newRow("single digit") << QStringLiteral("A") << false << QByteArray();
  QTest::newRow("odd count across spaces") << QStringLiteral("AB C") << false << QByteArray();
  QTest::newRow("non hex letter") << QStringLiteral("GG") << false << QByteArray();
  QTest::newRow("0x prefix is not a payload") << QStringLiteral("0x12") << false << QByteArray();
  QTest::newRow("comma separator") << QStringLiteral("de,ad") << false << QByteArray();
  QTest::newRow("colon separator") << QStringLiteral("de:ad") << false << QByteArray();
}

/**
 * @brief The payload field is whitespace-tolerant so a byte dump can be pasted in, but every
 *        other separator is refused rather than silently dropped: a comma read as nothing would
 *        change what gets written to the device.
 */
void TstUsbHexParsing::parseHexBytes()
{
  QFETCH(QString, text);
  QFETCH(bool, expectedOk);
  QFETCH(QByteArray, expectedBytes);

  bool ok                = true;
  const QByteArray bytes = IO::Drivers::UsbHex::parseHexBytes(text, ok);

  QCOMPARE(ok, expectedOk);
  QCOMPARE(bytes, expectedBytes);
}

/**
 * @brief The driver chains the setup-field parses through one shared @c ok, so parseHexBytes has
 *        to write the flag on every path rather than only clearing it on failure.
 */
void TstUsbHexParsing::parseHexBytes_alwaysWritesOk()
{
  bool ok = false;
  QCOMPARE(IO::Drivers::UsbHex::parseHexBytes(QStringLiteral("00"), ok), QByteArray::fromHex("00"));
  QVERIFY(ok);

  ok = true;
  QCOMPARE(IO::Drivers::UsbHex::parseHexBytes(QStringLiteral("0"), ok), QByteArray());
  QVERIFY(!ok);
}

//--------------------------------------------------------------------------------------------------
// controlStatusText
//--------------------------------------------------------------------------------------------------

void TstUsbHexParsing::controlStatusText_data()
{
  QTest::addColumn<int>("status");
  QTest::addColumn<QString>("expected");

  QTest::newRow("timed out") << int(kTransferTimedOut) << QStringLiteral("timed out");
  QTest::newRow("cancelled") << int(kTransferCancelled) << QStringLiteral("cancelled");
  QTest::newRow("stalled") << int(kTransferStall)
                           << QStringLiteral("stalled (request not supported)");
  QTest::newRow("no device") << int(kTransferNoDevice) << QStringLiteral("device disconnected");
  QTest::newRow("overflow") << int(kTransferOverflow) << QStringLiteral("buffer overflow");
  QTest::newRow("generic error") << int(kTransferError) << QStringLiteral("transfer error");
  QTest::newRow("completed is never reported")
    << int(kTransferCompleted) << QStringLiteral("transfer error");
  QTest::newRow("unknown status") << 99 << QStringLiteral("transfer error");
  QTest::newRow("negative status") << -1 << QStringLiteral("transfer error");
}

/**
 * @brief Only the statuses a user can act on get their own wording; everything else, including
 *        the two success-path codes that never reach this function, collapses to the generic
 *        failure text.
 */
void TstUsbHexParsing::controlStatusText()
{
  QFETCH(int, status);
  QFETCH(QString, expected);

  QCOMPARE(IO::Drivers::UsbHex::controlStatusText(status), expected);
}

QTEST_APPLESS_MAIN(TstUsbHexParsing)

#include "tst_usb_hex_parsing.moc"
