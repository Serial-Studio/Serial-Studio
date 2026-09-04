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

#include <QHash>
#include <QTest>

#include "Protocols/Iec104/Asdu.h"

using namespace IO::Drivers::Iec104Proto;

/**
 * @brief Slot identity for the IEC 60870-5-104 point table. A station qualifies an
 *        information-object address by the type identifier that carries it, so the address alone
 *        is NOT an identity: keying the table on it latched a measurand into a single-point input's
 *        slot and published it with that slot's wire type (spec 0075 E16).
 */
class TstIec104Slots : public QObject {
  Q_OBJECT

private slots:
  void oneAddressUnderTwoTypesIsTwoSlots();
  void theSamePairAlwaysResolvesToOneSlot();
  void keysNeverCollideAcrossTheAddressRange();
  void theTypeIdDecidesTheValueClass();
};

/**
 * @brief One address answered under two different type identifiers is two different measurements,
 *        so it has to occupy two slots. Both are legal at once: a station may report the state of
 *        a breaker and a measurand at the same object address.
 */
void TstIec104Slots::oneAddressUnderTwoTypesIsTwoSlots()
{
  const quint32 ioa = 1001;

  QHash<quint64, int> table;
  table.insert(slotKey(ioa, kTypeSinglePoint), 0);
  table.insert(slotKey(ioa, kTypeMeasuredFloat), 1);

  QCOMPARE(table.size(), 2);
  QCOMPARE(table.value(slotKey(ioa, kTypeSinglePoint)), 0);
  QCOMPARE(table.value(slotKey(ioa, kTypeMeasuredFloat)), 1);
  QVERIFY(slotKey(ioa, kTypeSinglePoint) != slotKey(ioa, kTypeMeasuredFloat));
}

/**
 * @brief The same pair resolves to the same slot on every later report; that is what keeps a
 *        generated project's dataset indices pointed at the object they were generated for.
 */
void TstIec104Slots::theSamePairAlwaysResolvesToOneSlot()
{
  QCOMPARE(slotKey(7, kTypeMeasuredScaled), slotKey(7, kTypeMeasuredScaled));
  QCOMPARE(slotKey(0, 0), quint64(0));
  QVERIFY(slotKey(7, kTypeMeasuredScaled) != slotKey(8, kTypeMeasuredScaled));
}

/**
 * @brief The address occupies the low half of the key and the type identifier the high half, so no
 *        address anywhere in the 24-bit range can be confused with another type's.
 */
void TstIec104Slots::keysNeverCollideAcrossTheAddressRange()
{
  QHash<quint64, int> seen;
  const std::uint8_t types[] = {kTypeSinglePoint, kTypeDoublePoint, kTypeMeasuredFloat};
  const quint32 addresses[]  = {0, 1, 255, 65535, kMaxIoa};

  int next = 0;
  for (const auto type : types)
    for (const auto ioa : addresses) {
      const auto key = slotKey(ioa, type);
      QVERIFY2(!seen.contains(key), "two (address, type) pairs produced one key");
      seen.insert(key, next++);
    }

  QCOMPARE(seen.size(), next);
  QCOMPARE(next, 15);
}

/**
 * @brief The type identifier is what decides the value class a slot encodes, which is why the
 *        LIVE type wins over a restored one: the station is the authority on what it is sending.
 */
void TstIec104Slots::theTypeIdDecidesTheValueClass()
{
  QCOMPARE(kindForType(kTypeSinglePoint), PointKind::Single);
  QCOMPARE(kindForType(kTypeDoublePoint), PointKind::Double);
  QCOMPARE(kindForType(kTypeMeasuredNormalized), PointKind::Normalized);
  QCOMPARE(kindForType(kTypeMeasuredScaled), PointKind::Scaled);
  QCOMPARE(kindForType(kTypeMeasuredFloat), PointKind::Float);
  QCOMPARE(kindForType(kTypeIntegratedTotals), PointKind::Counter);
  QCOMPARE(kindForType(kTypeInterrogation), PointKind::Invalid);

  QVERIFY(kindForType(kTypeSinglePoint) != kindForType(kTypeMeasuredFloat));
  QVERIFY(kindForType(kTypeSinglePoint) == kindForType(kTypeSinglePointTime));
}

QTEST_APPLESS_MAIN(TstIec104Slots)

#include "tst_iec104_slots.moc"
