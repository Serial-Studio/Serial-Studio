/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <bit>
#include <QByteArray>
#include <QJsonObject>
#include <QStringList>
#include <QTest>

#include "DataModel/Scripting/NativeTemplates/BinaryJustFloat.h"

// The JustFloat Built-In template (spec 0127 R18, AC10): 1, 3 and 64 floats with and without the
// end marker, a NaN payload, a partial float and the marker option. The frames were packed with
// Python's struct module ("<f").

//--------------------------------------------------------------------------------------------------
// Shared inputs
//--------------------------------------------------------------------------------------------------

static const QByteArray kMarker = QByteArray::fromHex("0000807f");
static const QByteArray kThree  = QByteArray::fromHex("0000c03f000010c09a99993f");
static const QByteArray kNan    = QByteArray::fromHex("0000c07f");

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The first row a JustFloat parser emits for @p frame; empty when it emits none.
 */
[[nodiscard]] static QStringList decode(const QByteArray& frame, const bool stripMarker = true)
{
  QString error;
  const auto parser = DataModel::justFloatTemplate().makeParser(
    QJsonObject{
      {QStringLiteral("strip_marker"), stripMarker}
  },
    error);
  if (!parser)
    return {};

  const auto rows = parser->parseBinary(frame);
  return rows.isEmpty() ? QStringList() : rows.constFirst();
}

/**
 * @brief A frame of @p count floats holding 0, 0.5, 1, 1.5 and so on.
 */
[[nodiscard]] static QByteArray ramp(const int count)
{
  QByteArray frame;
  for (int i = 0; i < count; ++i) {
    const float value  = static_cast<float>(i) * 0.5F;
    const auto bits    = std::bit_cast<quint32>(value);
    const char bytes[] = {static_cast<char>(bits & 0xFFU),
                          static_cast<char>((bits >> 8) & 0xFFU),
                          static_cast<char>((bits >> 16) & 0xFFU),
                          static_cast<char>((bits >> 24) & 0xFFU)};
    frame.append(bytes, 4);
  }

  return frame;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class JustFloatTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void channelCount_data();
  void channelCount();
  void threeFloatsWithAndWithoutMarker();
  void sixtyFourFloats();
  void countFollowsTheFrame();
  void nanPayloadKeepsItsChannel();
  void partialFloatYieldsNothing();
  void markerOptionKeepsATrailingInfinity();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void JustFloatTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief One row per frame shape the count rule must classify.
 */
void JustFloatTest::channelCount_data()
{
  QTest::addColumn<QByteArray>("frame");
  QTest::addColumn<bool>("strip");
  QTest::addColumn<qsizetype>("expected");

  QTest::newRow("one") << ramp(1) << true << qsizetype(1);
  QTest::newRow("three") << kThree << true << qsizetype(3);
  QTest::newRow("three + marker") << kThree + kMarker << true << qsizetype(3);
  QTest::newRow("marker kept") << kThree + kMarker << false << qsizetype(4);
  QTest::newRow("empty") << QByteArray() << true << qsizetype(-1);
  QTest::newRow("marker only") << kMarker << true << qsizetype(-1);
  QTest::newRow("partial") << kThree.chopped(1) << true << qsizetype(-1);
  QTest::newRow("cap") << ramp(512) << true << qsizetype(512);
  QTest::newRow("over cap") << ramp(513) << true << qsizetype(-1);
}

/**
 * @brief The count is the whole floats a frame holds, or -1 for a frame that cannot be decoded.
 */
void JustFloatTest::channelCount()
{
  QFETCH(QByteArray, frame);
  QFETCH(bool, strip);
  QFETCH(qsizetype, expected);

  QCOMPARE(DataModel::justFloatChannelCount(frame, strip), expected);
}

/**
 * @brief The same three values come out whether or not the reader left the marker attached, and
 *        1.2f reads as 1.2.
 */
void JustFloatTest::threeFloatsWithAndWithoutMarker()
{
  const QStringList expected = {
    QStringLiteral("1.5"), QStringLiteral("-2.25"), QStringLiteral("1.2")};
  QCOMPARE(decode(kThree), expected);
  QCOMPARE(decode(kThree + kMarker), expected);
}

/**
 * @brief Sixty-four channels decode in wire order.
 */
void JustFloatTest::sixtyFourFloats()
{
  const auto row = decode(ramp(64) + kMarker);
  QCOMPARE(row.size(), 64);
  QCOMPARE(row.at(0), QStringLiteral("0"));
  QCOMPARE(row.at(1), QStringLiteral("0.5"));
  QCOMPARE(row.at(63), QStringLiteral("31.5"));
}

/**
 * @brief One parser instance follows a device that changes its channel count mid-stream.
 */
void JustFloatTest::countFollowsTheFrame()
{
  QString error;
  const auto parser = DataModel::justFloatTemplate().makeParser(QJsonObject(), error);
  QVERIFY2(parser, qPrintable(error));

  QCOMPARE(parser->parseBinary(ramp(3)).constFirst().size(), 3);
  QCOMPARE(parser->parseBinary(ramp(5)).constFirst().size(), 5);
  QCOMPARE(parser->parseBinary(ramp(1)).constFirst().size(), 1);
}

/**
 * @brief A NaN value keeps its position in the row instead of shifting the channels after it.
 */
void JustFloatTest::nanPayloadKeepsItsChannel()
{
  const auto row = decode(kNan + kThree + kMarker);
  QCOMPARE(row.size(), 4);
  QCOMPARE(row.at(1), QStringLiteral("1.5"));
  QCOMPARE(row.at(3), QStringLiteral("1.2"));
}

/**
 * @brief A frame holding a partial float yields no row and builds no parser error.
 */
void JustFloatTest::partialFloatYieldsNothing()
{
  QVERIFY(decode(kThree.chopped(1)).isEmpty());
  QVERIFY(decode(kThree + kMarker.chopped(2)).isEmpty());
  QVERIFY(decode(QByteArray()).isEmpty());
}

/**
 * @brief With the marker option off a trailing positive infinity is a value, not a marker.
 */
void JustFloatTest::markerOptionKeepsATrailingInfinity()
{
  QCOMPARE(decode(kThree + kMarker, false).size(), 4);
  QCOMPARE(decode(kThree + kMarker, true).size(), 3);
  QVERIFY(DataModel::endsWithJustFloatMarker(kThree + kMarker));
  QVERIFY(!DataModel::endsWithJustFloatMarker(kThree));
}

QTEST_APPLESS_MAIN(JustFloatTest)

#include "tst_justfloat.moc"
