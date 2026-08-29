/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <QTest>

#include "IO/Drivers/S7Address.h"

using namespace IO::Drivers::S7Address;

/**
 * @brief Parses @p text, failing the test when the parser refuses an address it should accept.
 */
[[nodiscard]] static Address accepted(const QString& text)
{
  QString error;
  const auto address = parse(text, error);
  if (!isValid(address))
    qWarning("\"%s\" was rejected: %s", qPrintable(text), qPrintable(error));

  return address;
}

/**
 * @brief Pins the absolute S7 addressing grammar the driver, the project generator and the tag
 *        dialog all read. A misparsed offset reads the wrong memory on a live controller, so
 *        every accepted spelling and every rejection reason is fixed here.
 */
class TstS7Address : public QObject {
  Q_OBJECT

private slots:
  void dataBlockForms_data();
  void dataBlockForms();
  void processImageForms_data();
  void processImageForms();
  void bitAddressesCarryTheirIndex();
  void typeSuffixOverridesTheWidth();
  void stringAddressesCarryTheirLength();
  void malformedInputIsRejected_data();
  void malformedInputIsRejected();
  void everyPrefixOfAValidAddressIsSafe();
  void normalizeRoundTrips_data();
  void normalizeRoundTrips();
  void typeCodesRoundTrip();
};

//--------------------------------------------------------------------------------------------------
// Accepted spellings
//--------------------------------------------------------------------------------------------------

void TstS7Address::dataBlockForms_data()
{
  QTest::addColumn<QString>("text");
  QTest::addColumn<int>("db");
  QTest::addColumn<int>("offset");
  QTest::addColumn<int>("size");

  QTest::newRow("DB5.DBD20") << QStringLiteral("DB5.DBD20") << 5 << 20 << 4;
  QTest::newRow("DB1.DBW0") << QStringLiteral("DB1.DBW0") << 1 << 0 << 2;
  QTest::newRow("DB12.DBB7") << QStringLiteral("DB12.DBB7") << 12 << 7 << 1;
  QTest::newRow("lowercase") << QStringLiteral("db3.dbd8") << 3 << 8 << 4;
  QTest::newRow("padded") << QStringLiteral("  DB9.DBW100  ") << 9 << 100 << 2;
}

/**
 * @brief Data-block addresses resolve their block number, byte offset and read width.
 */
void TstS7Address::dataBlockForms()
{
  QFETCH(QString, text);
  QFETCH(int, db);
  QFETCH(int, offset);
  QFETCH(int, size);

  const auto address = accepted(text);
  QVERIFY(isValid(address));
  QCOMPARE(address.area, Area::DataBlk);
  QCOMPARE(address.dbNumber, db);
  QCOMPARE(address.byteOffset, offset);
  QCOMPARE(address.size, size);
  QCOMPARE(address.bitOffset, -1);
}

void TstS7Address::processImageForms_data()
{
  QTest::addColumn<QString>("text");
  QTest::addColumn<int>("area");
  QTest::addColumn<int>("offset");
  QTest::addColumn<int>("size");

  QTest::newRow("MW10") << QStringLiteral("MW10") << static_cast<int>(Area::Memory) << 10 << 2;
  QTest::newRow("MB5") << QStringLiteral("MB5") << static_cast<int>(Area::Memory) << 5 << 1;
  QTest::newRow("MD8") << QStringLiteral("MD8") << static_cast<int>(Area::Memory) << 8 << 4;
  QTest::newRow("IB0") << QStringLiteral("IB0") << static_cast<int>(Area::Input) << 0 << 1;
  QTest::newRow("QW4") << QStringLiteral("QW4") << static_cast<int>(Area::Output) << 4 << 2;
  QTest::newRow("german-E") << QStringLiteral("EW2") << static_cast<int>(Area::Input) << 2 << 2;
  QTest::newRow("german-A") << QStringLiteral("AB1") << static_cast<int>(Area::Output) << 1 << 1;
}

/**
 * @brief Process-image and flag addresses resolve their area, offset and width, and the German
 *        area letters (E, A) name the same areas as I and Q.
 */
void TstS7Address::processImageForms()
{
  QFETCH(QString, text);
  QFETCH(int, area);
  QFETCH(int, offset);
  QFETCH(int, size);

  const auto address = accepted(text);
  QVERIFY(isValid(address));
  QCOMPARE(static_cast<int>(address.area), area);
  QCOMPARE(address.dbNumber, 0);
  QCOMPARE(address.byteOffset, offset);
  QCOMPARE(address.size, size);
}

/**
 * @brief A bit address keeps its bit index, reads one byte, and types as BOOL whether it is
 *        spelled with an explicit X or as the bare dotted form.
 */
void TstS7Address::bitAddressesCarryTheirIndex()
{
  const auto block = accepted(QStringLiteral("DB1.DBX0.3"));
  QCOMPARE(block.area, Area::DataBlk);
  QCOMPARE(block.type, Type::Bool);
  QCOMPARE(block.dbNumber, 1);
  QCOMPARE(block.byteOffset, 0);
  QCOMPARE(block.bitOffset, 3);
  QCOMPARE(block.size, 1);

  const auto bare = accepted(QStringLiteral("Q0.1"));
  QCOMPARE(bare.area, Area::Output);
  QCOMPARE(bare.type, Type::Bool);
  QCOMPARE(bare.bitOffset, 1);

  const auto explicitX = accepted(QStringLiteral("MX12.7"));
  QCOMPARE(explicitX.type, Type::Bool);
  QCOMPARE(explicitX.byteOffset, 12);
  QCOMPARE(explicitX.bitOffset, 7);
}

/**
 * @brief The type suffix decides how the bytes are rendered, and is refused when its width does
 *        not match the address it is attached to.
 */
void TstS7Address::typeSuffixOverridesTheWidth()
{
  const auto real = accepted(QStringLiteral("DB5.DBD20:REAL"));
  QCOMPARE(real.type, Type::Real);
  QCOMPARE(real.size, 4);

  const auto integer = accepted(QStringLiteral("MW10:INT"));
  QCOMPARE(integer.type, Type::Int);
  QCOMPARE(integer.size, 2);

  const auto doubleInt = accepted(QStringLiteral("DB2.DBD4:DINT"));
  QCOMPARE(doubleInt.type, Type::DInt);
  QCOMPARE(doubleInt.size, 4);

  QString error;
  QVERIFY(!isValid(parse(QStringLiteral("MW10:REAL"), error)));
  QVERIFY(!error.isEmpty());
  QVERIFY(!isValid(parse(QStringLiteral("DB1.DBX0.1:BYTE"), error)));
}

/**
 * @brief A STRING address needs a declared length, reads the S7 two-byte header plus the payload,
 *        and rejects a length outside the protocol's range.
 */
void TstS7Address::stringAddressesCarryTheirLength()
{
  const auto text = accepted(QStringLiteral("DB4.DBB10:STRING[32]"));
  QCOMPARE(text.type, Type::Str);
  QCOMPARE(text.byteOffset, 10);
  QCOMPARE(text.size, 34);

  QString error;
  QVERIFY(!isValid(parse(QStringLiteral("DB4.DBB10:STRING"), error)));
  QVERIFY(!isValid(parse(QStringLiteral("DB4.DBB10:STRING[0]"), error)));
  QVERIFY(!isValid(parse(QStringLiteral("DB4.DBB10:STRING[255]"), error)));
  QVERIFY(!isValid(parse(QStringLiteral("DB4.DBW10:STRING[8]"), error)));
}

//--------------------------------------------------------------------------------------------------
// Rejections
//--------------------------------------------------------------------------------------------------

void TstS7Address::malformedInputIsRejected_data()
{
  QTest::addColumn<QString>("text");

  QTest::newRow("empty") << QString();
  QTest::newRow("blank") << QStringLiteral("   ");
  QTest::newRow("no-area") << QStringLiteral("W10");
  QTest::newRow("unknown-area") << QStringLiteral("ZW10");
  QTest::newRow("no-width") << QStringLiteral("M10");
  QTest::newRow("db-zero") << QStringLiteral("DB0.DBW0");
  QTest::newRow("db-no-offset") << QStringLiteral("DB5.DBD");
  QTest::newRow("db-missing-width") << QStringLiteral("DB5.DB20");
  QTest::newRow("bit-out-of-range") << QStringLiteral("M0.8");
  QTest::newRow("bit-on-word") << QStringLiteral("MW0.1");
  QTest::newRow("offset-out-of-range") << QStringLiteral("MW70000");
  QTest::newRow("db-out-of-range") << QStringLiteral("DB70000.DBW0");
  QTest::newRow("unknown-type") << QStringLiteral("MW10:FLOAT64");
  QTest::newRow("empty-type") << QStringLiteral("MW10:");
  QTest::newRow("trailing-garbage") << QStringLiteral("MW10abc");
  QTest::newRow("negative") << QStringLiteral("MW-4");
  QTest::newRow("separator-only") << QStringLiteral(".");
}

/**
 * @brief Every malformed spelling comes back invalid with a reason, and never half-parsed.
 */
void TstS7Address::malformedInputIsRejected()
{
  QFETCH(QString, text);

  QString error;
  const auto address = parse(text, error);
  QVERIFY(!isValid(address));
  QVERIFY(!error.isEmpty());
  QCOMPARE(address.area, Area::Invalid);
  QCOMPARE(address.type, Type::Invalid);
  QCOMPARE(address.size, 0);
}

/**
 * @brief Every prefix of a valid address is either a valid address of its own or a clean
 *        rejection: a truncated tag list entry must not reach the controller as a partial read.
 */
void TstS7Address::everyPrefixOfAValidAddressIsSafe()
{
  const QString full = QStringLiteral("DB12.DBD20:REAL");
  for (int length = 0; length <= full.size(); ++length) {
    QString error;
    const auto address = parse(full.left(length), error);
    QVERIFY2(isValid(address) || !error.isEmpty(), qPrintable(full.left(length)));
    if (!isValid(address))
      QCOMPARE(address.size, 0);
  }
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

void TstS7Address::normalizeRoundTrips_data()
{
  QTest::addColumn<QString>("text");
  QTest::addColumn<QString>("canonical");

  QTest::newRow("db-dword") << QStringLiteral("db5.dbd20") << QStringLiteral("DB5.DBD20");
  QTest::newRow("db-bit") << QStringLiteral("DB1.DBX0.3") << QStringLiteral("DB1.DBX0.3");
  QTest::newRow("memory-word") << QStringLiteral("mw10") << QStringLiteral("MW10");
  QTest::newRow("input-byte") << QStringLiteral("EB0") << QStringLiteral("IB0");
  QTest::newRow("output-bit") << QStringLiteral("A0.1") << QStringLiteral("QX0.1");
}

/**
 * @brief The canonical spelling of a parsed address parses back to the same address.
 */
void TstS7Address::normalizeRoundTrips()
{
  QFETCH(QString, text);
  QFETCH(QString, canonical);

  const auto first = accepted(text);
  QCOMPARE(normalize(first), canonical);

  const auto second = accepted(canonical);
  QCOMPARE(second.area, first.area);
  QCOMPARE(second.dbNumber, first.dbNumber);
  QCOMPARE(second.byteOffset, first.byteOffset);
  QCOMPARE(second.bitOffset, first.bitOffset);
  QCOMPARE(second.size, first.size);
}

/**
 * @brief Every declared type has a stable name that maps back onto its enumerator.
 */
void TstS7Address::typeCodesRoundTrip()
{
  const Type types[] = {
    Type::Bool, Type::Byte, Type::Word, Type::DWord, Type::Int, Type::DInt, Type::Real, Type::Str};

  for (const auto type : types) {
    const auto code = codeFromType(type);
    QVERIFY(!code.isEmpty());
    QCOMPARE(typeFromCode(code), type);
    QCOMPARE(typeFromCode(code.toLower()), type);
  }

  QCOMPARE(codeFromType(Type::Invalid), QString());
  QCOMPARE(typeFromCode(QStringLiteral("nonsense")), Type::Invalid);
}

QTEST_APPLESS_MAIN(TstS7Address)

#include "tst_s7_address.moc"
