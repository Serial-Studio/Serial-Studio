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

#include <cmath>
#include <limits>
#include <QTest>

#include "InfluxDB/LineProtocol.h"

using namespace InfluxDB;

/**
 * @brief Renders one single-field point and returns the line without its trailing newline.
 */
[[nodiscard]] static QByteArray onePoint(const QString& measurement,
                                         const QString& key,
                                         const QString& value,
                                         qint64 timestampNs)
{
  LineBatch batch;
  batch.beginPoint(measurement);
  (void)batch.addFieldString(key, value);
  (void)batch.endPoint(timestampNs);
  return batch.payload().trimmed();
}

/**
 * @brief InfluxDB 2.x line-protocol writer: per-position escaping, the field-type spellings,
 *        nanosecond timestamps, the batch boundary, and the non-finite skip (spec 0073, AC11).
 */
class TstInfluxLineProtocol : public QObject {
  Q_OBJECT

private slots:
  void measurementEscaping_data();
  void measurementEscaping();
  void tagEscaping_data();
  void tagEscaping();
  void stringFieldEscaping_data();
  void stringFieldEscaping();
  void fieldTypeSpellings();
  void floatRoundTrip_data();
  void floatRoundTrip();
  void nonFiniteFieldsSkipped();
  void pointWithoutFieldsRolledBack();
  void tagAfterFieldRefused();
  void timestampPrecision();
  void batchBoundary();
  void clearKeepsSkipTally();
};

void TstInfluxLineProtocol::measurementEscaping_data()
{
  QTest::addColumn<QString>("measurement");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("plain") << QStringLiteral("temps") << QByteArrayLiteral("temps");
  QTest::newRow("comma") << QStringLiteral("a,b") << QByteArrayLiteral("a\\,b");
  QTest::newRow("space") << QStringLiteral("a b") << QByteArrayLiteral("a\\ b");
  QTest::newRow("equals-kept") << QStringLiteral("a=b") << QByteArrayLiteral("a=b");
  QTest::newRow("quote-kept") << QStringLiteral("a\"b") << QByteArrayLiteral("a\"b");
  QTest::newRow("newline") << QStringLiteral("a\nb") << QByteArrayLiteral("a\\ b");
  QTest::newRow("utf8") << QString::fromUtf8("°C")
                        << QByteArrayLiteral("\xC2\xB0"
                                             "C");
}

void TstInfluxLineProtocol::measurementEscaping()
{
  QFETCH(QString, measurement);
  QFETCH(QByteArray, expected);

  const QByteArray line = onePoint(measurement, QStringLiteral("v"), QStringLiteral("x"), 1);
  QCOMPARE(line, expected + QByteArrayLiteral(" v=\"x\" 1"));
}

void TstInfluxLineProtocol::tagEscaping_data()
{
  QTest::addColumn<QString>("key");
  QTest::addColumn<QString>("value");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("plain") << QStringLiteral("host") << QStringLiteral("a1")
                         << QByteArrayLiteral("host=a1");
  QTest::newRow("comma") << QStringLiteral("h,k") << QStringLiteral("a,1")
                         << QByteArrayLiteral("h\\,k=a\\,1");
  QTest::newRow("equals") << QStringLiteral("h=k") << QStringLiteral("a=1")
                          << QByteArrayLiteral("h\\=k=a\\=1");
  QTest::newRow("space") << QStringLiteral("h k") << QStringLiteral("a 1")
                         << QByteArrayLiteral("h\\ k=a\\ 1");
}

void TstInfluxLineProtocol::tagEscaping()
{
  QFETCH(QString, key);
  QFETCH(QString, value);
  QFETCH(QByteArray, expected);

  LineBatch batch;
  batch.beginPoint(QStringLiteral("m"));
  batch.addTag(key, value);
  QVERIFY(batch.addFieldInteger(QStringLiteral("v"), 1));
  QVERIFY(batch.endPoint(7));

  QCOMPARE(batch.payload().trimmed(),
           QByteArrayLiteral("m,") + expected + QByteArrayLiteral(" v=1i 7"));
}

void TstInfluxLineProtocol::stringFieldEscaping_data()
{
  QTest::addColumn<QString>("value");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("plain") << QStringLiteral("ok") << QByteArrayLiteral("ok");
  QTest::newRow("quote") << QStringLiteral("a\"b") << QByteArrayLiteral("a\\\"b");
  QTest::newRow("backslash") << QStringLiteral("a\\b") << QByteArrayLiteral("a\\\\b");
  QTest::newRow("comma-kept") << QStringLiteral("a,b") << QByteArrayLiteral("a,b");
  QTest::newRow("space-kept") << QStringLiteral("a b") << QByteArrayLiteral("a b");
  QTest::newRow("equals-kept") << QStringLiteral("a=b") << QByteArrayLiteral("a=b");
  QTest::newRow("newline") << QStringLiteral("a\r\nb") << QByteArrayLiteral("a  b");
}

void TstInfluxLineProtocol::stringFieldEscaping()
{
  QFETCH(QString, value);
  QFETCH(QByteArray, expected);

  const QByteArray line = onePoint(QStringLiteral("m"), QStringLiteral("v"), value, 3);
  QCOMPARE(line, QByteArrayLiteral("m v=\"") + expected + QByteArrayLiteral("\" 3"));
}

void TstInfluxLineProtocol::fieldTypeSpellings()
{
  LineBatch batch;
  batch.beginPoint(QStringLiteral("m"));
  QVERIFY(batch.addFieldInteger(QStringLiteral("i"), -42));
  QVERIFY(batch.addFieldFloat(QStringLiteral("f"), 1.5));
  QVERIFY(batch.addFieldBoolean(QStringLiteral("t"), true));
  QVERIFY(batch.addFieldBoolean(QStringLiteral("f2"), false));
  QVERIFY(batch.addFieldString(QStringLiteral("s"), QStringLiteral("hi")));
  QVERIFY(batch.endPoint(11));

  QCOMPARE(batch.payload(), QByteArrayLiteral("m i=-42i,f=1.5,t=t,f2=f,s=\"hi\" 11\n"));
  QCOMPARE(batch.points(), qsizetype(1));
}

void TstInfluxLineProtocol::floatRoundTrip_data()
{
  QTest::addColumn<double>("value");

  QTest::newRow("zero") << 0.0;
  QTest::newRow("negative") << -273.15;
  QTest::newRow("tenth") << 0.1;
  QTest::newRow("third") << (1.0 / 3.0);
  QTest::newRow("large") << 1.7976931348623157e+300;
  QTest::newRow("small") << 2.2250738585072014e-308;
  QTest::newRow("integral") << 4096.0;
}

void TstInfluxLineProtocol::floatRoundTrip()
{
  QFETCH(double, value);

  const QByteArray text = formatDouble(value);
  QCOMPARE(text.toDouble(), value);
}

void TstInfluxLineProtocol::nonFiniteFieldsSkipped()
{
  LineBatch batch;
  batch.beginPoint(QStringLiteral("m"));
  QVERIFY(!batch.addFieldFloat(QStringLiteral("nan"), std::nan("")));
  QVERIFY(!batch.addFieldFloat(QStringLiteral("inf"), std::numeric_limits<double>::infinity()));
  QVERIFY(!batch.addFieldFloat(QStringLiteral("ninf"), -std::numeric_limits<double>::infinity()));
  QVERIFY(batch.addFieldFloat(QStringLiteral("ok"), 2.0));
  QVERIFY(batch.endPoint(5));

  QCOMPARE(batch.skippedFields(), qsizetype(3));
  QCOMPARE(batch.payload(), QByteArrayLiteral("m ok=2 5\n"));
}

void TstInfluxLineProtocol::pointWithoutFieldsRolledBack()
{
  LineBatch batch;
  batch.beginPoint(QStringLiteral("kept"));
  QVERIFY(batch.addFieldInteger(QStringLiteral("v"), 1));
  QVERIFY(batch.endPoint(1));

  batch.beginPoint(QStringLiteral("dropped"));
  batch.addTag(QStringLiteral("host"), QStringLiteral("a"));
  QVERIFY(!batch.addFieldFloat(QStringLiteral("v"), std::nan("")));
  QVERIFY(!batch.endPoint(2));

  QCOMPARE(batch.points(), qsizetype(1));
  QCOMPARE(batch.payload(), QByteArrayLiteral("kept v=1i 1\n"));
}

void TstInfluxLineProtocol::tagAfterFieldRefused()
{
  LineBatch batch;
  batch.beginPoint(QStringLiteral("m"));
  QVERIFY(batch.addFieldInteger(QStringLiteral("v"), 1));
  batch.addTag(QStringLiteral("host"), QStringLiteral("a"));
  QVERIFY(batch.endPoint(9));

  QCOMPARE(batch.payload(), QByteArrayLiteral("m v=1i 9\n"));
}

void TstInfluxLineProtocol::timestampPrecision()
{
  constexpr qint64 ns = 1700000000123456789LL;

  LineBatch batch;
  batch.beginPoint(QStringLiteral("m"));
  QVERIFY(batch.addFieldInteger(QStringLiteral("v"), 1));
  QVERIFY(batch.endPoint(ns));

  QCOMPARE(batch.payload(), QByteArrayLiteral("m v=1i 1700000000123456789\n"));
}

void TstInfluxLineProtocol::batchBoundary()
{
  LineBatch batch(64);
  QVERIFY(!batch.full());
  QVERIFY(batch.isEmpty());

  int guard = 0;
  while (!batch.full() && guard < 1000) {
    batch.beginPoint(QStringLiteral("measurement"));
    QVERIFY(batch.addFieldInteger(QStringLiteral("value"), guard));
    QVERIFY(batch.endPoint(guard));
    ++guard;
  }

  QVERIFY(batch.full());
  QVERIFY(!batch.isEmpty());
  QVERIFY(batch.payload().size() >= 64);
  QCOMPARE(batch.points(), qsizetype(guard));
  QCOMPARE(batch.payload().count('\n'), qsizetype(guard));

  batch.clear();
  QVERIFY(!batch.full());
  QVERIFY(batch.isEmpty());
  QCOMPARE(batch.points(), qsizetype(0));
  QVERIFY(batch.payload().isEmpty());
}

void TstInfluxLineProtocol::clearKeepsSkipTally()
{
  LineBatch batch;
  batch.beginPoint(QStringLiteral("m"));
  QVERIFY(!batch.addFieldFloat(QStringLiteral("v"), std::nan("")));
  QVERIFY(!batch.endPoint(1));

  batch.clear();
  QCOMPARE(batch.skippedFields(), qsizetype(1));
}

QTEST_APPLESS_MAIN(TstInfluxLineProtocol)

#include "tst_influx_lineprotocol.moc"
