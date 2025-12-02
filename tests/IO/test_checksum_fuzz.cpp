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
#include <QRandomGenerator>

#include "IO/Checksum.h"

class TestChecksumFuzz : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void testFuzzRandomData();
  void testFuzzBoundaryConditions();
  void testFuzzDeterminism();
  void testFuzzAllAlgorithms();

private:
  QRandomGenerator m_random;
  QStringList m_algorithms;

  QByteArray generateRandomData(int size)
  {
    QByteArray data(size, 0);
    for (int i = 0; i < size; ++i)
      data[i] = static_cast<char>(m_random.bounded(256));
    return data;
  }
};

void TestChecksumFuzz::initTestCase()
{
  m_random.seed(42);

  m_algorithms = IO::availableChecksums();
  m_algorithms.removeAll("");

  QVERIFY(!m_algorithms.isEmpty());
}

void TestChecksumFuzz::testFuzzRandomData()
{
  const int kIterations = 1000;
  const int kMaxSize = 1024;

  for (int i = 0; i < kIterations; ++i)
  {
    const int size = m_random.bounded(1, kMaxSize);
    const QByteArray data = generateRandomData(size);

    for (const QString &algorithm : m_algorithms)
    {
      const QByteArray result = IO::checksum(algorithm, data);

      QVERIFY2(!result.isEmpty(),
               QString("Algorithm %1 returned empty result for %2 bytes")
                   .arg(algorithm)
                   .arg(size)
                   .toUtf8());

      if (algorithm.contains("8"))
        QCOMPARE(result.size(), 1);
      else if (algorithm.contains("16"))
        QCOMPARE(result.size(), 2);
      else if (algorithm.contains("32"))
        QCOMPARE(result.size(), 4);
    }
  }
}

void TestChecksumFuzz::testFuzzBoundaryConditions()
{
  const QVector<int> sizes = {0, 1, 2, 255, 256, 257, 1023, 1024,
                              1025, 4095, 4096, 65535};

  for (int size : sizes)
  {
    const QByteArray data = generateRandomData(size);

    for (const QString &algorithm : m_algorithms)
    {
      const QByteArray result = IO::checksum(algorithm, data);

      QVERIFY2(!result.isEmpty() || size == 0,
               QString("Algorithm %1 failed for size %2")
                   .arg(algorithm)
                   .arg(size)
                   .toUtf8());
    }
  }
}

void TestChecksumFuzz::testFuzzDeterminism()
{
  const int kIterations = 100;

  for (int i = 0; i < kIterations; ++i)
  {
    const int size = m_random.bounded(1, 1024);
    const QByteArray data = generateRandomData(size);

    for (const QString &algorithm : m_algorithms)
    {
      const QByteArray result1 = IO::checksum(algorithm, data);
      const QByteArray result2 = IO::checksum(algorithm, data);

      QVERIFY2(result1 == result2,
               QString("Algorithm %1 is non-deterministic for %2 bytes")
                   .arg(algorithm)
                   .arg(size)
                   .toUtf8());
    }
  }
}

void TestChecksumFuzz::testFuzzAllAlgorithms()
{
  const QByteArray testData = "The quick brown fox jumps over the lazy dog";

  for (const QString &algorithm : m_algorithms)
  {
    QBENCHMARK
    {
      IO::checksum(algorithm, testData);
    }
  }
}

QTEST_MAIN(TestChecksumFuzz)
#include "test_checksum_fuzz.moc"
