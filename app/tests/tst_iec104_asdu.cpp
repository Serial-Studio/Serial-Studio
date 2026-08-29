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

#include <cstring>
#include <QDateTime>
#include <QTest>
#include <QTimeZone>

#include "IO/Drivers/Iec104/Asdu.h"

using namespace IO::Drivers::Iec104Proto;

/**
 * @brief Builds the six-octet ASDU header a station would send.
 */
[[nodiscard]] static QByteArray header(
  std::uint8_t typeId, int count, bool sequence, std::uint8_t cause, quint16 commonAddress)
{
  QByteArray asdu;
  asdu.append(static_cast<char>(typeId));
  asdu.append(static_cast<char>((sequence ? 0x80 : 0x00) | (count & 0x7F)));
  asdu.append(static_cast<char>(cause));
  asdu.append(static_cast<char>(0));
  asdu.append(static_cast<char>(commonAddress & 0xFF));
  asdu.append(static_cast<char>((commonAddress >> 8) & 0xFF));
  return asdu;
}

/**
 * @brief Builds a three-octet information object address.
 */
[[nodiscard]] static QByteArray address(quint32 ioa)
{
  QByteArray bytes;
  bytes.append(static_cast<char>(ioa & 0xFF));
  bytes.append(static_cast<char>((ioa >> 8) & 0xFF));
  bytes.append(static_cast<char>((ioa >> 16) & 0xFF));
  return bytes;
}

/**
 * @brief Builds a little-endian 16-bit measurand.
 */
[[nodiscard]] static QByteArray word16(qint16 value)
{
  const auto raw = static_cast<quint16>(value);
  QByteArray bytes;
  bytes.append(static_cast<char>(raw & 0xFF));
  bytes.append(static_cast<char>((raw >> 8) & 0xFF));
  return bytes;
}

/**
 * @brief Builds a little-endian 32-bit word.
 */
[[nodiscard]] static QByteArray word32(quint32 value)
{
  QByteArray bytes;
  for (int i = 0; i < 4; ++i)
    bytes.append(static_cast<char>((value >> (8 * i)) & 0xFF));

  return bytes;
}

/**
 * @brief Builds the four payload octets of an IEEE 754 short float.
 */
[[nodiscard]] static QByteArray shortFloat(float value)
{
  quint32 bits = 0;
  std::memcpy(&bits, &value, sizeof(bits));
  return word32(bits);
}

/**
 * @brief Builds a CP56Time2a stamp for the given calendar reading.
 */
[[nodiscard]] static QByteArray cp56(const QDateTime& stamp, bool invalid)
{
  const auto date  = stamp.date();
  const auto time  = stamp.time();
  const int millis = time.second() * 1000 + time.msec();

  QByteArray bytes;
  bytes.append(static_cast<char>(millis & 0xFF));
  bytes.append(static_cast<char>((millis >> 8) & 0xFF));
  bytes.append(static_cast<char>(time.minute() | (invalid ? 0x80 : 0x00)));
  bytes.append(static_cast<char>(time.hour()));
  bytes.append(static_cast<char>(date.day() | (date.dayOfWeek() << 5)));
  bytes.append(static_cast<char>(date.month()));
  bytes.append(static_cast<char>(date.year() - 2000));
  return bytes;
}

/**
 * @brief The reference stamp every timestamped decode is measured against.
 */
[[nodiscard]] static QDateTime referenceStamp()
{
  return QDateTime(QDate(2026, 8, 27), QTime(12, 34, 56, 789), QTimeZone::UTC);
}

/**
 * @brief Pins the ASDU application layer of the IEC 60870-5-104 client: the twelve monitor-
 *        direction type identifications, their CP56Time2a twins, the SQ addressing modes, the
 *        quality descriptors and the one command this build encodes. A misread element width
 *        silently shifts every point after it, so each layout is golden-decoded here.
 */
class TstIec104Asdu : public QObject {
  Q_OBJECT

private slots:
  void singlePointsDecode();
  void doublePointsDecode();
  void normalizedMeasurandsDecode();
  void scaledMeasurandsDecode();
  void floatMeasurandsDecode();
  void integratedTotalsDecode();
  void timestampedTwinsDecode_data();
  void timestampedTwinsDecode();
  void sequenceAddressingIncrementsTheAddress();
  void qualityBitsPropagatePerPoint_data();
  void qualityBitsPropagatePerPoint();
  void measurandQualityCarriesOverflow();
  void counterQualityCarriesCarryAndAdjustment();
  void headerFieldsDecode();
  void elementWidthsMatchTheTypeTable_data();
  void elementWidthsMatchTheTypeTable();
  void unknownTypesAreRefusedNotGuessed();
  void recognizedControlTypesYieldNoPoints();
  void truncatedBuffersAreRefused_data();
  void truncatedBuffersAreRefused();
  void timestampEdgeCases();
  void interrogationEncodesTheStationRequest();
};

//--------------------------------------------------------------------------------------------------
// Untimed monitor types
//--------------------------------------------------------------------------------------------------

/**
 * @brief M_SP_NA_1 carries one boolean per object in the SIQ's low bit.
 */
void TstIec104Asdu::singlePointsDecode()
{
  auto asdu = header(kTypeSinglePoint, 2, false, kCauseInterrogated, 1);
  asdu.append(address(100)).append(static_cast<char>(0x01));
  asdu.append(address(101)).append(static_cast<char>(0x00));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.size(), 2);
  QCOMPARE(points.at(0).ioa, quint32(100));
  QCOMPARE(points.at(0).kind, PointKind::Single);
  QCOMPARE(points.at(0).value.toBool(), true);
  QCOMPARE(points.at(0).quality, std::uint8_t(QualityGood));
  QCOMPARE(points.at(1).ioa, quint32(101));
  QCOMPARE(points.at(1).value.toBool(), false);
  QVERIFY(!points.at(1).timeValid);
}

/**
 * @brief M_DP_NA_1 carries the two-bit DPI, whose intermediate states are values, not errors.
 */
void TstIec104Asdu::doublePointsDecode()
{
  auto asdu = header(kTypeDoublePoint, 4, false, kCauseSpontaneous, 1);
  for (int i = 0; i < 4; ++i)
    asdu.append(address(200 + i)).append(static_cast<char>(i));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.size(), 4);
  for (int i = 0; i < 4; ++i) {
    QCOMPARE(points.at(i).ioa, quint32(200 + i));
    QCOMPARE(points.at(i).kind, PointKind::Double);
    QCOMPARE(points.at(i).value.toInt(), i);
  }
}

/**
 * @brief M_ME_NA_1 is a fraction of full scale, so 16384 reads as one half.
 */
void TstIec104Asdu::normalizedMeasurandsDecode()
{
  auto asdu = header(kTypeMeasuredNormalized, 2, false, kCauseSpontaneous, 1);
  asdu.append(address(300)).append(word16(16384)).append(static_cast<char>(0x00));
  asdu.append(address(301)).append(word16(-16384)).append(static_cast<char>(0x00));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.size(), 2);
  QCOMPARE(points.at(0).kind, PointKind::Normalized);
  QVERIFY(qFuzzyCompare(points.at(0).value.toDouble() + 1.0, 1.5));
  QVERIFY(qFuzzyCompare(points.at(1).value.toDouble() + 1.0, 0.5));
}

/**
 * @brief M_ME_NB_1 is a signed engineering integer and keeps its sign through the decode.
 */
void TstIec104Asdu::scaledMeasurandsDecode()
{
  auto asdu = header(kTypeMeasuredScaled, 2, false, kCauseSpontaneous, 1);
  asdu.append(address(400)).append(word16(-1234)).append(static_cast<char>(0x00));
  asdu.append(address(401)).append(word16(32767)).append(static_cast<char>(0x00));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.size(), 2);
  QCOMPARE(points.at(0).kind, PointKind::Scaled);
  QCOMPARE(points.at(0).value.toInt(), -1234);
  QCOMPARE(points.at(1).value.toInt(), 32767);
}

/**
 * @brief M_ME_NC_1 carries a little-endian IEEE 754 short float.
 */
void TstIec104Asdu::floatMeasurandsDecode()
{
  auto asdu = header(kTypeMeasuredFloat, 2, false, kCauseSpontaneous, 1);
  asdu.append(address(500)).append(shortFloat(3.5F)).append(static_cast<char>(0x00));
  asdu.append(address(501)).append(shortFloat(-0.125F)).append(static_cast<char>(0x00));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.size(), 2);
  QCOMPARE(points.at(0).kind, PointKind::Float);
  QVERIFY(qFuzzyCompare(points.at(0).value.toDouble(), 3.5));
  QVERIFY(qFuzzyCompare(points.at(1).value.toDouble(), -0.125));
}

/**
 * @brief M_IT_NA_1 carries a signed 32-bit counter followed by its sequence octet.
 */
void TstIec104Asdu::integratedTotalsDecode()
{
  auto asdu = header(kTypeIntegratedTotals, 1, false, kCauseInterrogated, 1);
  asdu.append(address(600)).append(word32(static_cast<quint32>(-70000)));
  asdu.append(static_cast<char>(0x05));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.size(), 1);
  QCOMPARE(points.at(0).kind, PointKind::Counter);
  QCOMPARE(points.at(0).value.toInt(), -70000);
  QCOMPARE(points.at(0).quality, std::uint8_t(QualityGood));
}

//--------------------------------------------------------------------------------------------------
// Timestamped twins
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every timestamped type, paired with the untimed body its element starts with.
 */
void TstIec104Asdu::timestampedTwinsDecode_data()
{
  QTest::addColumn<int>("typeId");
  QTest::addColumn<QByteArray>("body");
  QTest::addColumn<int>("kind");

  const QByteArray qds(1, static_cast<char>(0x00));

  QTest::newRow("M_SP_TB_1") << int(kTypeSinglePointTime) << QByteArray(1, static_cast<char>(0x01))
                             << int(PointKind::Single);
  QTest::newRow("M_DP_TB_1") << int(kTypeDoublePointTime) << QByteArray(1, static_cast<char>(0x02))
                             << int(PointKind::Double);
  QTest::newRow("M_ME_TD_1") << int(kTypeMeasuredNormalTime) << (word16(16384) + qds)
                             << int(PointKind::Normalized);
  QTest::newRow("M_ME_TE_1") << int(kTypeMeasuredScaledTime) << (word16(-1234) + qds)
                             << int(PointKind::Scaled);
  QTest::newRow("M_ME_TF_1") << int(kTypeMeasuredFloatTime) << (shortFloat(3.5F) + qds)
                             << int(PointKind::Float);
  QTest::newRow("M_IT_TB_1") << int(kTypeIntegratedTotalsTime)
                             << (word32(quint32(1234)) + QByteArray(1, static_cast<char>(0x01)))
                             << int(PointKind::Counter);
}

/**
 * @brief Each timestamped twin decodes to the same value class as its untimed sibling and carries
 *        the station's own CP56Time2a stamp; a wrong element width would shift the stamp instead.
 */
void TstIec104Asdu::timestampedTwinsDecode()
{
  QFETCH(int, typeId);
  QFETCH(QByteArray, body);
  QFETCH(int, kind);

  const auto stamp = referenceStamp();
  auto asdu        = header(static_cast<std::uint8_t>(typeId), 1, false, kCauseSpontaneous, 7);
  asdu.append(address(700)).append(body).append(cp56(stamp, false));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(decoded.commonAddress, quint16(7));
  QCOMPARE(points.size(), 1);
  QCOMPARE(int(points.at(0).kind), kind);
  QCOMPARE(points.at(0).ioa, quint32(700));
  QVERIFY(points.at(0).timeValid);
  QCOMPARE(points.at(0).timeMsecs, stamp.toMSecsSinceEpoch());
  QVERIFY(typeCarriesTime(static_cast<std::uint8_t>(typeId)));
}

//--------------------------------------------------------------------------------------------------
// Addressing and quality
//--------------------------------------------------------------------------------------------------

/**
 * @brief With SQ = 1 the addresses are implicit: the base is sent once and every element after it
 *        takes the next address, which is what makes a bulk interrogation reply compact.
 */
void TstIec104Asdu::sequenceAddressingIncrementsTheAddress()
{
  auto asdu = header(kTypeMeasuredScaled, 3, true, kCauseInterrogated, 1);
  asdu.append(address(1000));
  for (int i = 0; i < 3; ++i)
    asdu.append(word16(static_cast<qint16>(10 * i))).append(static_cast<char>(0x00));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QVERIFY(decoded.sequence);
  QCOMPARE(decoded.objectCount, std::uint8_t(3));
  QCOMPARE(points.size(), 3);
  for (int i = 0; i < 3; ++i) {
    QCOMPARE(points.at(i).ioa, quint32(1000 + i));
    QCOMPARE(points.at(i).value.toInt(), 10 * i);
  }
}

/**
 * @brief The four descriptor bits every monitor type shares, one row each.
 */
void TstIec104Asdu::qualityBitsPropagatePerPoint_data()
{
  QTest::addColumn<int>("descriptor");
  QTest::addColumn<int>("quality");

  QTest::newRow("good") << 0x01 << int(QualityGood);
  QTest::newRow("blocked") << 0x11 << int(QualityBlocked);
  QTest::newRow("substituted") << 0x21 << int(QualitySubstituted);
  QTest::newRow("not topical") << 0x41 << int(QualityNotTopical);
  QTest::newRow("invalid") << 0x81 << int(QualityInvalid);
  QTest::newRow("blocked and invalid") << 0x91 << int(QualityBlocked | QualityInvalid);
}

/**
 * @brief A single point keeps its quality bits per point, which is what lets the dashboard show a
 *        stale value as stale rather than as a fresh reading of the same number (R29).
 */
void TstIec104Asdu::qualityBitsPropagatePerPoint()
{
  QFETCH(int, descriptor);
  QFETCH(int, quality);

  auto asdu = header(kTypeSinglePoint, 1, false, kCauseSpontaneous, 1);
  asdu.append(address(1)).append(static_cast<char>(descriptor));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.size(), 1);
  QCOMPARE(points.at(0).value.toBool(), true);
  QCOMPARE(int(points.at(0).quality), quality);
}

/**
 * @brief The QDS descriptor adds an overflow bit the point descriptors do not have.
 */
void TstIec104Asdu::measurandQualityCarriesOverflow()
{
  auto asdu = header(kTypeMeasuredScaled, 1, false, kCauseSpontaneous, 1);
  asdu.append(address(2)).append(word16(1)).append(static_cast<char>(0x41));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(int(points.at(0).quality), int(QualityOverflow | QualityNotTopical));
}

/**
 * @brief A counter that carried reads as overflowed and one the station adjusted as substituted.
 */
void TstIec104Asdu::counterQualityCarriesCarryAndAdjustment()
{
  auto asdu = header(kTypeIntegratedTotals, 1, false, kCauseInterrogated, 1);
  asdu.append(address(3)).append(word32(quint32(9))).append(static_cast<char>(0xE1));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(points.at(0).value.toInt(), 9);
  QCOMPARE(int(points.at(0).quality), int(QualityOverflow | QualitySubstituted | QualityInvalid));
}

/**
 * @brief The header splits the cause octet into the cause, the negative flag and the test flag.
 */
void TstIec104Asdu::headerFieldsDecode()
{
  auto asdu = header(kTypeSinglePoint, 1, false, static_cast<std::uint8_t>(0xC3), 513);
  asdu.append(address(1)).append(static_cast<char>(0x00));

  Header decoded;
  QCOMPARE(decodeHeader(asdu, decoded), DecodeResult::Ok);
  QCOMPARE(decoded.typeId, std::uint8_t(kTypeSinglePoint));
  QCOMPARE(decoded.cause, std::uint8_t(kCauseSpontaneous));
  QCOMPARE(decoded.commonAddress, quint16(513));
  QVERIFY(decoded.negative);
  QVERIFY(decoded.test);
  QVERIFY(!decoded.sequence);
}

//--------------------------------------------------------------------------------------------------
// Type table and refusals
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every element width the driver relies on to walk an object list.
 */
void TstIec104Asdu::elementWidthsMatchTheTypeTable_data()
{
  QTest::addColumn<int>("typeId");
  QTest::addColumn<int>("width");

  QTest::newRow("M_SP_NA_1") << int(kTypeSinglePoint) << 1;
  QTest::newRow("M_DP_NA_1") << int(kTypeDoublePoint) << 1;
  QTest::newRow("M_ME_NA_1") << int(kTypeMeasuredNormalized) << 3;
  QTest::newRow("M_ME_NB_1") << int(kTypeMeasuredScaled) << 3;
  QTest::newRow("M_ME_NC_1") << int(kTypeMeasuredFloat) << 5;
  QTest::newRow("M_IT_NA_1") << int(kTypeIntegratedTotals) << 5;
  QTest::newRow("M_SP_TB_1") << int(kTypeSinglePointTime) << 8;
  QTest::newRow("M_DP_TB_1") << int(kTypeDoublePointTime) << 8;
  QTest::newRow("M_ME_TD_1") << int(kTypeMeasuredNormalTime) << 10;
  QTest::newRow("M_ME_TE_1") << int(kTypeMeasuredScaledTime) << 10;
  QTest::newRow("M_ME_TF_1") << int(kTypeMeasuredFloatTime) << 12;
  QTest::newRow("M_IT_TB_1") << int(kTypeIntegratedTotalsTime) << 12;
  QTest::newRow("M_EI_NA_1") << int(kTypeEndOfInitialization) << 1;
  QTest::newRow("C_IC_NA_1") << int(kTypeInterrogation) << 1;
  QTest::newRow("C_SC_NA_1") << 45 << -1;
  QTest::newRow("undefined") << 200 << -1;
}

/**
 * @brief The element table is the walk's only stride, so each entry is pinned individually.
 */
void TstIec104Asdu::elementWidthsMatchTheTypeTable()
{
  QFETCH(int, typeId);
  QFETCH(int, width);

  QCOMPARE(elementBytes(static_cast<std::uint8_t>(typeId)), width);
  QCOMPARE(isRecognizedType(static_cast<std::uint8_t>(typeId)), width > 0);
}

/**
 * @brief A type this build does not decode is reported and counted, never walked on a guess.
 */
void TstIec104Asdu::unknownTypesAreRefusedNotGuessed()
{
  auto asdu = header(static_cast<std::uint8_t>(45), 1, false, kCauseActivation, 1);
  asdu.append(address(9)).append(static_cast<char>(0x01));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Unsupported);
  QCOMPARE(decoded.typeId, std::uint8_t(45));
  QVERIFY(points.isEmpty());
  QVERIFY(!isMonitorType(static_cast<std::uint8_t>(45)));
}

/**
 * @brief The end-of-initialization report and the interrogation confirmation are understood but
 *        carry no measurand; counting them as skipped would make a healthy session look lossy.
 */
void TstIec104Asdu::recognizedControlTypesYieldNoPoints()
{
  Header decoded;
  QList<Point> points;

  auto endOfInit = header(kTypeEndOfInitialization, 1, false, kCauseInitialized, 1);
  endOfInit.append(address(0)).append(static_cast<char>(0x00));
  QCOMPARE(decode(endOfInit, decoded, points), DecodeResult::Ok);
  QVERIFY(points.isEmpty());

  auto confirm = header(kTypeInterrogation, 1, false, kCauseActConfirm, 1);
  confirm.append(address(0)).append(static_cast<char>(kQoiStation));
  QCOMPARE(decode(confirm, decoded, points), DecodeResult::Ok);
  QVERIFY(points.isEmpty());
  QCOMPARE(decoded.cause, std::uint8_t(kCauseActConfirm));
}

/**
 * @brief Buffers that end before their own header promises them to.
 */
void TstIec104Asdu::truncatedBuffersAreRefused_data()
{
  QTest::addColumn<QByteArray>("asdu");

  QTest::newRow("empty") << QByteArray();
  QTest::newRow("short header") << header(kTypeSinglePoint, 1, false, kCauseSpontaneous, 1).left(5);
  QTest::newRow("missing object") << header(kTypeSinglePoint, 1, false, kCauseSpontaneous, 1);
  QTest::newRow("half an address")
    << (header(kTypeSinglePoint, 1, false, kCauseSpontaneous, 1) + QByteArray(2, char(0)));
  QTest::newRow("second object missing")
    << (header(kTypeSinglePoint, 2, false, kCauseSpontaneous, 1) + address(1)
        + QByteArray(1, char(0)));
  QTest::newRow("sequence without base")
    << (header(kTypeMeasuredScaled, 3, true, kCauseSpontaneous, 1) + QByteArray(2, char(0)));
  QTest::newRow("sequence short element")
    << (header(kTypeMeasuredScaled, 2, true, kCauseSpontaneous, 1) + address(1)
        + QByteArray(4, char(0)));
  QTest::newRow("timestamp cut short")
    << (header(kTypeSinglePointTime, 1, false, kCauseSpontaneous, 1) + address(1)
        + QByteArray(5, char(0)));
}

/**
 * @brief A truncated buffer is refused whole rather than decoded up to the cut: a half-read
 *        element would publish a value assembled from the next object's bytes.
 */
void TstIec104Asdu::truncatedBuffersAreRefused()
{
  QFETCH(QByteArray, asdu);

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Truncated);
  QVERIFY(points.isEmpty());
}

/**
 * @brief The stamp decoder pins the field order and refuses impossible calendar readings; the IV
 *        bit is reported without discarding the reading, so the driver can still see what arrived.
 */
void TstIec104Asdu::timestampEdgeCases()
{
  const auto stamp = referenceStamp();
  bool valid       = false;

  const auto good = cp56(stamp, false);
  QCOMPARE(decodeCp56Time2a(good, 0, valid), stamp.toMSecsSinceEpoch());
  QVERIFY(valid);

  const auto flagged = cp56(stamp, true);
  QCOMPARE(decodeCp56Time2a(flagged, 0, valid), stamp.toMSecsSinceEpoch());
  QVERIFY(!valid);

  const auto leapDay   = QDateTime(QDate(2028, 2, 29), QTime(23, 59, 59, 999), QTimeZone::UTC);
  const auto leapBytes = cp56(leapDay, false);
  QCOMPARE(decodeCp56Time2a(leapBytes, 0, valid), leapDay.toMSecsSinceEpoch());
  QVERIFY(valid);

  const auto cutShort = good.left(6);
  QCOMPARE(decodeCp56Time2a(cutShort, 0, valid), qint64(-1));
  QVERIFY(!valid);
  QCOMPARE(decodeCp56Time2a(good, -1, valid), qint64(-1));

  auto badMonth = good;
  badMonth[5]   = static_cast<char>(0);
  QCOMPARE(decodeCp56Time2a(badMonth, 0, valid), qint64(-1));

  auto badDay = good;
  badDay[4]   = static_cast<char>(0);
  QCOMPARE(decodeCp56Time2a(badDay, 0, valid), qint64(-1));

  auto badMillis = good;
  badMillis[0]   = static_cast<char>(0xFF);
  badMillis[1]   = static_cast<char>(0xFF);
  QCOMPARE(decodeCp56Time2a(badMillis, 0, valid), qint64(-1));
}

/**
 * @brief The station interrogation is the one frame the driver writes; its bytes are pinned here
 *        because a wrong qualifier returns a subset of the station's points and looks like a
 *        sparse database rather than a malformed request.
 */
void TstIec104Asdu::interrogationEncodesTheStationRequest()
{
  const auto asdu = encodeInterrogation(258, kQoiStation);
  QCOMPARE(asdu, QByteArray("\x64\x01\x06\x00\x02\x01\x00\x00\x00\x14", 10));

  Header decoded;
  QList<Point> points;
  QCOMPARE(decode(asdu, decoded, points), DecodeResult::Ok);
  QCOMPARE(decoded.typeId, std::uint8_t(kTypeInterrogation));
  QCOMPARE(decoded.cause, std::uint8_t(kCauseActivation));
  QCOMPARE(decoded.commonAddress, quint16(258));
  QCOMPARE(decoded.objectCount, std::uint8_t(1));
  QVERIFY(points.isEmpty());
}

QTEST_APPLESS_MAIN(TstIec104Asdu)

#include "tst_iec104_asdu.moc"
