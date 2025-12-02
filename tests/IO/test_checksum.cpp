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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include <QtTest>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "IO/Checksum.h"

class TestChecksum : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void testAvailableChecksums();
  void testEmptyInput();
  void testXor8();
  void testMod256();
  void testCrc8();
  void testCrc16();
  void testCrc16Modbus();
  void testCrc16Ccitt();
  void testFletcher16();
  void testCrc32();
  void testAdler32();
  void testUnknownAlgorithm();
  void cleanupTestCase();

private:
  QJsonObject m_vectors;

  QByteArray hexToBytes(const QString &hex)
  {
    return QByteArray::fromHex(hex.toUtf8());
  }

  void testAlgorithm(const QString &name)
  {
    if (!m_vectors.contains(name))
    {
      QSKIP(QString("No test vectors for %1").arg(name).toUtf8());
      return;
    }

    const QJsonArray vectors = m_vectors[name].toArray();
    for (const auto &vecValue : vectors)
    {
      const QJsonObject vec = vecValue.toObject();
      const QString inputHex = vec["input"].toString();
      const QString expectedHex = vec["expected"].toString();
      const QString ascii = vec["ascii"].toString();

      const QByteArray input = hexToBytes(inputHex);
      const QByteArray expected = hexToBytes(expectedHex);
      const QByteArray result = IO::checksum(name, input);

      QVERIFY2(result == expected,
               QString("Algorithm: %1, Input: '%2' (hex: %3), Expected: %4, "
                       "Got: %5")
                   .arg(name, ascii, inputHex, expectedHex,
                        QString(result.toHex().toUpper()))
                   .toUtf8());
    }
  }
};

void TestChecksum::initTestCase()
{
  QFile file("fixtures/checksum_vectors.json");
  QVERIFY2(file.open(QIODevice::ReadOnly),
           "Failed to open checksum_vectors.json");

  const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  QVERIFY2(!doc.isNull(), "Failed to parse checksum_vectors.json");

  const QJsonObject root = doc.object();
  QVERIFY2(root.contains("vectors"),
           "checksum_vectors.json missing 'vectors' key");

  m_vectors = root["vectors"].toObject();
  QVERIFY2(!m_vectors.isEmpty(), "No test vectors loaded");
}

void TestChecksum::testAvailableChecksums()
{
  const QStringList checksums = IO::availableChecksums();

  QVERIFY(!checksums.isEmpty());
  QVERIFY(checksums.contains("XOR-8"));
  QVERIFY(checksums.contains("MOD-256"));
  QVERIFY(checksums.contains("CRC-8"));
  QVERIFY(checksums.contains("CRC-16"));
  QVERIFY(checksums.contains("CRC-16-MODBUS"));
  QVERIFY(checksums.contains("CRC-16-CCITT"));
  QVERIFY(checksums.contains("Fletcher-16"));
  QVERIFY(checksums.contains("CRC-32"));
  QVERIFY(checksums.contains("Adler-32"));

  QCOMPARE(checksums.size(), 10);
}

void TestChecksum::testEmptyInput()
{
  const QByteArray empty;

  QByteArray result = IO::checksum("XOR-8", empty);
  QCOMPARE(result.size(), 1);
  QCOMPARE(static_cast<uint8_t>(result[0]), static_cast<uint8_t>(0x00));

  result = IO::checksum("CRC-16", empty);
  QCOMPARE(result.size(), 2);

  result = IO::checksum("CRC-32", empty);
  QCOMPARE(result.size(), 4);

  result = IO::checksum("Adler-32", empty);
  QCOMPARE(result.size(), 4);
}

void TestChecksum::testXor8()
{
  testAlgorithm("XOR-8");
}

void TestChecksum::testMod256()
{
  testAlgorithm("MOD-256");
}

void TestChecksum::testCrc8()
{
  testAlgorithm("CRC-8");
}

void TestChecksum::testCrc16()
{
  testAlgorithm("CRC-16");
}

void TestChecksum::testCrc16Modbus()
{
  testAlgorithm("CRC-16-MODBUS");
}

void TestChecksum::testCrc16Ccitt()
{
  testAlgorithm("CRC-16-CCITT");
}

void TestChecksum::testFletcher16()
{
  testAlgorithm("Fletcher-16");
}

void TestChecksum::testCrc32()
{
  testAlgorithm("CRC-32");
}

void TestChecksum::testAdler32()
{
  testAlgorithm("Adler-32");
}

void TestChecksum::testUnknownAlgorithm()
{
  const QByteArray data = "Hello World";
  const QByteArray result = IO::checksum("UNKNOWN_ALGORITHM", data);

  QVERIFY(result.isEmpty());
}

void TestChecksum::cleanupTestCase()
{
  m_vectors = QJsonObject();
}

QTEST_MAIN(TestChecksum)
#include "test_checksum.moc"
