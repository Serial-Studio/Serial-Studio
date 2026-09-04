/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTest>

#include "Core/Checksum.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Known-answer inputs
//--------------------------------------------------------------------------------------------------

// The published CRC catalogue check string. Every expected value below is the catalogue's own check
// value for the algorithm as implemented in Checksum.cpp: CRC-8/NRSC-5 (0xF7), CRC-16/IBM-3740
// a.k.a. CCITT-FALSE (0x29B1), CRC-16/MODBUS (0x4B37), CRC-16/XMODEM (0x31C3), Fletcher-16
// (0x1EDE), CRC-32/ISO-HDLC (0xCBF43926) and Adler-32 (0x091E01DE). Seven of the nine agree with
// tests/utils/data_generator.py, which is what tests/integration/test_frame_parsing.py sends on the
// wire; CRC-16-MODBUS and CRC-16-CCITT have no counterpart there, so nothing can disagree.
static const QByteArray kCheckInput("123456789");

static const QByteArray kSingleByte("a");

/**
 * @brief Byte-level contract of IO::checksum() and the algorithm registry it dispatches through.
 */
class TstChecksums : public QObject {
  Q_OBJECT

private slots:
  void registryHoldsEveryShippedAlgorithm();
  void availableNamesMatchTheFunctionMap();

  void checkVectors_data();
  void checkVectors();
  void emptyInput_data();
  void emptyInput();
  void singleByteInput_data();
  void singleByteInput();

  void outputLength_data();
  void outputLength();

  void crc16ModbusPacksLittleEndian();
  void sixteenBitFamilyPacksBigEndian();
  void thirtyTwoBitFamilyPacksBigEndian();

  void emptyAlgorithmProducesNoBytes();
  void unknownAlgorithmIsIndistinguishableFromNone();
  void validateChecksumAcceptsOnlyTheMatchingDigest();
};

//--------------------------------------------------------------------------------------------------
// Registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Ten entries ship: the nine named algorithms plus the empty name that means "no checksum".
 */
void TstChecksums::registryHoldsEveryShippedAlgorithm()
{
  const auto& map = IO::checksumFunctionMap();

  QCOMPARE(map.size(), qsizetype(10));
  QVERIFY(map.contains(QString()));
  QVERIFY(map.contains(QStringLiteral("XOR-8")));
  QVERIFY(map.contains(QStringLiteral("MOD-256")));
  QVERIFY(map.contains(QStringLiteral("CRC-8")));
  QVERIFY(map.contains(QStringLiteral("CRC-16")));
  QVERIFY(map.contains(QStringLiteral("CRC-16-MODBUS")));
  QVERIFY(map.contains(QStringLiteral("CRC-16-CCITT")));
  QVERIFY(map.contains(QStringLiteral("Fletcher-16")));
  QVERIFY(map.contains(QStringLiteral("CRC-32")));
  QVERIFY(map.contains(QStringLiteral("Adler-32")));
}

/**
 * @brief The name list the UI offers and the dispatch map must stay key-for-key identical, or a
 *        project can select an algorithm that resolves to nothing at parse time.
 */
void TstChecksums::availableNamesMatchTheFunctionMap()
{
  const auto& names = IO::availableChecksums();
  const auto& map   = IO::checksumFunctionMap();

  QCOMPARE(names.size(), map.size());
  QCOMPARE(names, map.keys());

  for (const auto& name : names) {
    QVERIFY(map.contains(name));
    QVERIFY(static_cast<bool>(map.value(name)));
  }
}

//--------------------------------------------------------------------------------------------------
// Known-answer vectors
//--------------------------------------------------------------------------------------------------

void TstChecksums::checkVectors_data()
{
  QTest::addColumn<QString>("algorithm");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("XOR-8") << QStringLiteral("XOR-8") << QByteArray::fromHex("31");
  QTest::newRow("MOD-256") << QStringLiteral("MOD-256") << QByteArray::fromHex("dd");
  QTest::newRow("CRC-8") << QStringLiteral("CRC-8") << QByteArray::fromHex("f7");
  QTest::newRow("CRC-16") << QStringLiteral("CRC-16") << QByteArray::fromHex("29b1");
  QTest::newRow("CRC-16-MODBUS") << QStringLiteral("CRC-16-MODBUS") << QByteArray::fromHex("374b");
  QTest::newRow("CRC-16-CCITT") << QStringLiteral("CRC-16-CCITT") << QByteArray::fromHex("31c3");
  QTest::newRow("Fletcher-16") << QStringLiteral("Fletcher-16") << QByteArray::fromHex("1ede");
  QTest::newRow("CRC-32") << QStringLiteral("CRC-32") << QByteArray::fromHex("cbf43926");
  QTest::newRow("Adler-32") << QStringLiteral("Adler-32") << QByteArray::fromHex("091e01de");
}

void TstChecksums::checkVectors()
{
  QFETCH(QString, algorithm);
  QFETCH(QByteArray, expected);

  QCOMPARE(IO::checksum(algorithm, kCheckInput), expected);
}

void TstChecksums::emptyInput_data()
{
  QTest::addColumn<QString>("algorithm");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("XOR-8") << QStringLiteral("XOR-8") << QByteArray::fromHex("00");
  QTest::newRow("MOD-256") << QStringLiteral("MOD-256") << QByteArray::fromHex("00");
  QTest::newRow("CRC-8") << QStringLiteral("CRC-8") << QByteArray::fromHex("ff");
  QTest::newRow("CRC-16") << QStringLiteral("CRC-16") << QByteArray::fromHex("ffff");
  QTest::newRow("CRC-16-MODBUS") << QStringLiteral("CRC-16-MODBUS") << QByteArray::fromHex("ffff");
  QTest::newRow("CRC-16-CCITT") << QStringLiteral("CRC-16-CCITT") << QByteArray::fromHex("0000");
  QTest::newRow("Fletcher-16") << QStringLiteral("Fletcher-16") << QByteArray::fromHex("0000");
  QTest::newRow("CRC-32") << QStringLiteral("CRC-32") << QByteArray::fromHex("00000000");
  QTest::newRow("Adler-32") << QStringLiteral("Adler-32") << QByteArray::fromHex("00000001");
}

/**
 * @brief Zero-length input returns each algorithm's seed, not an empty array: FrameReader sizes its
 *        trailing checksum field from exactly this call.
 */
void TstChecksums::emptyInput()
{
  QFETCH(QString, algorithm);
  QFETCH(QByteArray, expected);

  QCOMPARE(IO::checksum(algorithm, QByteArray()), expected);
}

void TstChecksums::singleByteInput_data()
{
  QTest::addColumn<QString>("algorithm");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("XOR-8") << QStringLiteral("XOR-8") << QByteArray::fromHex("61");
  QTest::newRow("MOD-256") << QStringLiteral("MOD-256") << QByteArray::fromHex("61");
  QTest::newRow("CRC-8") << QStringLiteral("CRC-8") << QByteArray::fromHex("26");
  QTest::newRow("CRC-16") << QStringLiteral("CRC-16") << QByteArray::fromHex("9d77");
  QTest::newRow("CRC-16-MODBUS") << QStringLiteral("CRC-16-MODBUS") << QByteArray::fromHex("7ea8");
  QTest::newRow("CRC-16-CCITT") << QStringLiteral("CRC-16-CCITT") << QByteArray::fromHex("7c87");
  QTest::newRow("Fletcher-16") << QStringLiteral("Fletcher-16") << QByteArray::fromHex("6161");
  QTest::newRow("CRC-32") << QStringLiteral("CRC-32") << QByteArray::fromHex("e8b7be43");
  QTest::newRow("Adler-32") << QStringLiteral("Adler-32") << QByteArray::fromHex("00620062");
}

void TstChecksums::singleByteInput()
{
  QFETCH(QString, algorithm);
  QFETCH(QByteArray, expected);

  QCOMPARE(IO::checksum(algorithm, kSingleByte), expected);
}

//--------------------------------------------------------------------------------------------------
// Output width
//--------------------------------------------------------------------------------------------------

void TstChecksums::outputLength_data()
{
  QTest::addColumn<QString>("algorithm");
  QTest::addColumn<int>("length");

  QTest::newRow("none") << QString() << 0;
  QTest::newRow("XOR-8") << QStringLiteral("XOR-8") << 1;
  QTest::newRow("MOD-256") << QStringLiteral("MOD-256") << 1;
  QTest::newRow("CRC-8") << QStringLiteral("CRC-8") << 1;
  QTest::newRow("CRC-16") << QStringLiteral("CRC-16") << 2;
  QTest::newRow("CRC-16-MODBUS") << QStringLiteral("CRC-16-MODBUS") << 2;
  QTest::newRow("CRC-16-CCITT") << QStringLiteral("CRC-16-CCITT") << 2;
  QTest::newRow("Fletcher-16") << QStringLiteral("Fletcher-16") << 2;
  QTest::newRow("CRC-32") << QStringLiteral("CRC-32") << 4;
  QTest::newRow("Adler-32") << QStringLiteral("Adler-32") << 4;
}

/**
 * @brief Output width is a property of the algorithm, never of the payload: FrameReader caches it
 *        once from a zero-length call and then trusts it for every frame.
 */
void TstChecksums::outputLength()
{
  QFETCH(QString, algorithm);
  QFETCH(int, length);

  QCOMPARE(IO::checksum(algorithm, QByteArray()).size(), qsizetype(length));
  QCOMPARE(IO::checksum(algorithm, kSingleByte).size(), qsizetype(length));
  QCOMPARE(IO::checksum(algorithm, kCheckInput).size(), qsizetype(length));
  QCOMPARE(IO::checksum(algorithm, QByteArray(4096, 'z')).size(), qsizetype(length));
}

//--------------------------------------------------------------------------------------------------
// Byte order
//--------------------------------------------------------------------------------------------------

/**
 * @brief CRC-16-MODBUS is the one algorithm packed low byte first, matching the Modbus RTU wire
 *        order. Its check value is 0x4B37, so the frame carries 0x37 then 0x4B.
 */
void TstChecksums::crc16ModbusPacksLittleEndian()
{
  const auto packed = IO::checksum(QStringLiteral("CRC-16-MODBUS"), kCheckInput);

  QCOMPARE(packed.size(), qsizetype(2));
  QCOMPARE(static_cast<quint8>(packed.at(0)), quint8(0x37));
  QCOMPARE(static_cast<quint8>(packed.at(1)), quint8(0x4B));
}

/**
 * @brief Every other 16-bit algorithm packs high byte first.
 */
void TstChecksums::sixteenBitFamilyPacksBigEndian()
{
  const auto crc16 = IO::checksum(QStringLiteral("CRC-16"), kCheckInput);
  QCOMPARE(static_cast<quint8>(crc16.at(0)), quint8(0x29));
  QCOMPARE(static_cast<quint8>(crc16.at(1)), quint8(0xB1));

  const auto ccitt = IO::checksum(QStringLiteral("CRC-16-CCITT"), kCheckInput);
  QCOMPARE(static_cast<quint8>(ccitt.at(0)), quint8(0x31));
  QCOMPARE(static_cast<quint8>(ccitt.at(1)), quint8(0xC3));

  const auto fletcher = IO::checksum(QStringLiteral("Fletcher-16"), kCheckInput);
  QCOMPARE(static_cast<quint8>(fletcher.at(0)), quint8(0x1E));
  QCOMPARE(static_cast<quint8>(fletcher.at(1)), quint8(0xDE));
}

/**
 * @brief Both 32-bit algorithms pack most significant byte first.
 */
void TstChecksums::thirtyTwoBitFamilyPacksBigEndian()
{
  const auto crc32 = IO::checksum(QStringLiteral("CRC-32"), kCheckInput);
  QCOMPARE(crc32.size(), qsizetype(4));
  QCOMPARE(static_cast<quint8>(crc32.at(0)), quint8(0xCB));
  QCOMPARE(static_cast<quint8>(crc32.at(3)), quint8(0x26));

  const auto adler = IO::checksum(QStringLiteral("Adler-32"), kCheckInput);
  QCOMPARE(adler.size(), qsizetype(4));
  QCOMPARE(static_cast<quint8>(adler.at(0)), quint8(0x09));
  QCOMPARE(static_cast<quint8>(adler.at(3)), quint8(0xDE));
}

//--------------------------------------------------------------------------------------------------
// Name resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief The empty name is a registered algorithm that appends nothing -- the "no checksum" case.
 */
void TstChecksums::emptyAlgorithmProducesNoBytes()
{
  QVERIFY(IO::checksum(QString(), kCheckInput).isEmpty());
  QVERIFY(IO::checksum(QStringLiteral(""), kCheckInput).isEmpty());
}

/**
 * @brief A name that is not in the registry yields the same empty result as the "no checksum"
 *        entry: a typo in a project file degrades to unvalidated frames rather than to a failure.
 */
void TstChecksums::unknownAlgorithmIsIndistinguishableFromNone()
{
  const auto unknown = IO::checksum(QStringLiteral("CRC-42-NOPE"), kCheckInput);

  QVERIFY(unknown.isEmpty());
  QCOMPARE(unknown, IO::checksum(QString(), kCheckInput));
  QVERIFY(!IO::checksumFunctionMap().contains(QStringLiteral("CRC-42-NOPE")));
}

/**
 * @brief validateChecksum() is the header-inline comparison used by the export and script paths.
 */
void TstChecksums::validateChecksumAcceptsOnlyTheMatchingDigest()
{
  const auto length = static_cast<int>(kCheckInput.size());
  const auto digest = IO::checksum(QStringLiteral("CRC-32"), kCheckInput);

  QVERIFY(IO::validateChecksum(QStringLiteral("CRC-32"), kCheckInput.constData(), length, digest));
  QVERIFY(!IO::validateChecksum(
    QStringLiteral("CRC-32"), kCheckInput.constData(), length, QByteArray::fromHex("00000000")));
  QVERIFY(!IO::validateChecksum(QStringLiteral("CRC-16"), kCheckInput.constData(), length, digest));
  QVERIFY(!IO::validateChecksum(QStringLiteral("CRC-32"), kCheckInput.constData(), 0, digest));
  QVERIFY(!IO::validateChecksum<char>(QStringLiteral("CRC-32"), nullptr, length, digest));

  QCOMPARE(IO::computeChecksum(QStringLiteral("CRC-32"), kCheckInput), digest);
}

QTEST_APPLESS_MAIN(TstChecksums)

#include "tst_checksums.moc"
