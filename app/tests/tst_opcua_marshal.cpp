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
#include <QTimeZone>
#include <QVariantList>

#include "IO/Drivers/OpcUaMarshal.h"
#include "IO/Drivers/OpcUaTypes.h"

using namespace IO::Drivers;

/**
 * @brief Wraps a scalar in a borrowed UA_Variant; the caller owns the storage.
 */
[[nodiscard]] static UA_Variant scalarVariant(void* data, quint16 typeIndex)
{
  UA_Variant variant;
  UA_Variant_init(&variant);
  variant.type        = &UA_TYPES[typeIndex];
  variant.storageType = UA_VARIANT_DATA_NODELETE;
  variant.data        = data;
  variant.arrayLength = 0;
  return variant;
}

/**
 * @brief Wraps an array in a borrowed UA_Variant; the caller owns the storage.
 */
[[nodiscard]] static UA_Variant arrayVariant(void* data, size_t count, quint16 typeIndex)
{
  UA_Variant variant  = scalarVariant(data, typeIndex);
  variant.arrayLength = count;
  return variant;
}

class TstOpcUaMarshal : public QObject {
  Q_OBJECT

private slots:
  void numericScalarsRoundTrip();
  void stringAndTextValues();
  void engineeringUnitWrappers();
  void arraysBecomeVariantLists();
  void nodeIdsRoundTrip();
  void statusSeverityClassification();
  void sourceTimestampIsPreserved();
  void absentTimestampIsInvalid();
};

/**
 * @brief Every builtin numeric type reaches Qt with its value and signedness intact.
 */
void TstOpcUaMarshal::numericScalarsRoundTrip()
{
  UA_Boolean flag = true;
  auto value      = scalarVariant(&flag, UA_TYPES_BOOLEAN);
  QCOMPARE(OpcUaMarshal::toVariant(value).toBool(), true);

  UA_Int32 i32 = -1234567;
  value        = scalarVariant(&i32, UA_TYPES_INT32);
  QCOMPARE(OpcUaMarshal::toVariant(value).toInt(), -1234567);

  UA_UInt32 u32 = 4000000000u;
  value         = scalarVariant(&u32, UA_TYPES_UINT32);
  QCOMPARE(OpcUaMarshal::toVariant(value).toUInt(), 4000000000u);

  UA_Int64 i64 = Q_INT64_C(-9007199254740993);
  value        = scalarVariant(&i64, UA_TYPES_INT64);
  QCOMPARE(OpcUaMarshal::toVariant(value).toLongLong(), Q_INT64_C(-9007199254740993));

  UA_Double dbl = 3.5;
  value         = scalarVariant(&dbl, UA_TYPES_DOUBLE);
  QCOMPARE(OpcUaMarshal::toVariant(value).toDouble(), 3.5);

  UA_Float flt = 0.5f;
  value        = scalarVariant(&flt, UA_TYPES_FLOAT);
  QCOMPARE(OpcUaMarshal::toVariant(value).toFloat(), 0.5f);

  UA_SByte sbyte = -128;
  value          = scalarVariant(&sbyte, UA_TYPES_SBYTE);
  QCOMPARE(OpcUaMarshal::toVariant(value).toInt(), -128);

  UA_Int16 i16 = -32768;
  value        = scalarVariant(&i16, UA_TYPES_INT16);
  QCOMPARE(OpcUaMarshal::toVariant(value).toInt(), -32768);
}

/**
 * @brief Strings are length-delimited rather than null-terminated, and LocalizedText carries its
 *        payload one level down; both must land as plain text for the string widgets.
 */
void TstOpcUaMarshal::stringAndTextValues()
{
  UA_String text = UA_STRING_STATIC("FILLING");
  auto value     = scalarVariant(&text, UA_TYPES_STRING);
  QCOMPARE(OpcUaMarshal::toVariant(value).toString(), QStringLiteral("FILLING"));

  UA_LocalizedText localized;
  localized.locale = UA_STRING_STATIC("en");
  localized.text   = UA_STRING_STATIC("Filler Level");
  value            = scalarVariant(&localized, UA_TYPES_LOCALIZEDTEXT);
  QCOMPARE(OpcUaMarshal::toVariant(value).toString(), QStringLiteral("Filler Level"));

  UA_String empty;
  empty.length = 0;
  empty.data   = nullptr;
  value        = scalarVariant(&empty, UA_TYPES_STRING);
  QVERIFY(OpcUaMarshal::toVariant(value).toString().isEmpty());
}

/**
 * @brief EUInformation and Range are both plain structures, so only the type pointer separates
 *        them; the tag browser reads units and display bounds through this path. Range must keep
 *        BOTH bounds: they become a generated dataset's plot minimum and maximum, and a low-only
 *        conversion would give every plot a zero-width range with nothing to see.
 */
void TstOpcUaMarshal::engineeringUnitWrappers()
{
  UA_EUInformation unit;
  UA_EUInformation_init(&unit);
  unit.displayName.text = UA_STRING_STATIC("%");
  auto value            = scalarVariant(&unit, UA_TYPES_EUINFORMATION);
  QCOMPARE(OpcUaMarshal::toVariant(value).toString(), QStringLiteral("%"));

  UA_Range range;
  range.low  = -50.0;
  range.high = 250.0;
  value      = scalarVariant(&range, UA_TYPES_RANGE);

  const auto bounds = OpcUaMarshal::toVariant(value).toList();
  QCOMPARE(bounds.size(), 2);
  QCOMPARE(bounds.at(0).toDouble(), -50.0);
  QCOMPARE(bounds.at(1).toDouble(), 250.0);
}

/**
 * @brief An array must stay a list: the driver fans elements across wire indices, so collapsing
 *        one to a scalar would silently latch only its first element.
 */
void TstOpcUaMarshal::arraysBecomeVariantLists()
{
  UA_Double zones[6] = {60.0, 61.5, 63.0, 64.5, 66.0, 67.5};
  const auto value   = arrayVariant(zones, 6, UA_TYPES_DOUBLE);

  const auto converted = OpcUaMarshal::toVariant(value);
  QCOMPARE(converted.typeId(), QMetaType::QVariantList);

  const auto list = converted.toList();
  QCOMPARE(list.size(), 6);
  QCOMPARE(list.at(0).toDouble(), 60.0);
  QCOMPARE(list.at(5).toDouble(), 67.5);
}

/**
 * @brief Node ids are the driver's tag identity and travel through project files as text, so both
 *        the string and numeric forms must survive a round trip unchanged.
 */
void TstOpcUaMarshal::nodeIdsRoundTrip()
{
  const QString stringId = QStringLiteral("ns=2;s=Plant.Line1.Filler.Level_pct");

  UA_NodeId parsed;
  QVERIFY(OpcUaMarshal::nodeIdFromString(stringId, parsed));
  QCOMPARE(parsed.namespaceIndex, static_cast<UA_UInt16>(2));
  QCOMPARE(OpcUaMarshal::nodeIdToString(parsed), stringId);
  UA_NodeId_clear(&parsed);

  const QString numericId = QStringLiteral("ns=2;i=5000");
  QVERIFY(OpcUaMarshal::nodeIdFromString(numericId, parsed));
  QCOMPARE(OpcUaMarshal::nodeIdToString(parsed), numericId);
  UA_NodeId_clear(&parsed);

  UA_NodeId rejected;
  QVERIFY(!OpcUaMarshal::nodeIdFromString(QStringLiteral("not a node id"), rejected));
  UA_NodeId_clear(&rejected);
}

/**
 * @brief Quality is the two severity bits, never "!= Good". Uncertain is real data a PLC flagged:
 *        treating it as Bad flat-lines the dashboard, which spec 0066 R11 forbids.
 */
void TstOpcUaMarshal::statusSeverityClassification()
{
  using namespace OpcUaTypes;

  QVERIFY(isGood(UA_STATUSCODE_GOOD));
  QVERIFY(!isBad(UA_STATUSCODE_GOOD));
  QVERIFY(!isUncertain(UA_STATUSCODE_GOOD));

  QVERIFY(isUncertain(UA_STATUSCODE_UNCERTAIN));
  QVERIFY(!isBad(UA_STATUSCODE_UNCERTAIN));
  QVERIFY(!isGood(UA_STATUSCODE_UNCERTAIN));

  QVERIFY(isBad(UA_STATUSCODE_BADINTERNALERROR));
  QVERIFY(isBad(UA_STATUSCODE_BADNODEIDUNKNOWN));
  QVERIFY(!isGood(UA_STATUSCODE_BADNODEIDUNKNOWN));

  QVERIFY(!OpcUaMarshal::statusText(UA_STATUSCODE_BADNODEIDUNKNOWN).isEmpty());
}

/**
 * @brief The server's clock must survive conversion exactly. Source owns time (spec 0066 R10):
 *        a stamp lost here degrades to the local monotonic fallback with no visible error.
 */
void TstOpcUaMarshal::sourceTimestampIsPreserved()
{
  const QDateTime expected =
    QDateTime::fromMSecsSinceEpoch(Q_INT64_C(1755000000000), QTimeZone::UTC);

  UA_DataValue value;
  UA_DataValue_init(&value);
  value.hasSourceTimestamp = true;
  value.sourceTimestamp    = (Q_INT64_C(1755000000000) * UA_DATETIME_MSEC) + UA_DATETIME_UNIX_EPOCH;

  const auto stamped = OpcUaMarshal::sourceTimeOf(value);
  QVERIFY(stamped.isValid());
  QCOMPARE(stamped.toUTC(), expected);
}

/**
 * @brief A server that sends no source timestamp must yield an INVALID QDateTime, not the 1601
 *        epoch: a plausible-looking stamp would be recorded as real and never questioned.
 */
void TstOpcUaMarshal::absentTimestampIsInvalid()
{
  UA_DataValue value;
  UA_DataValue_init(&value);
  value.hasSourceTimestamp = false;
  value.sourceTimestamp    = 0;
  QVERIFY(!OpcUaMarshal::sourceTimeOf(value).isValid());

  value.hasSourceTimestamp = true;
  value.sourceTimestamp    = 0;
  QVERIFY(!OpcUaMarshal::sourceTimeOf(value).isValid());

  QVERIFY(!OpcUaMarshal::toDateTime(0).isValid());
}

QTEST_APPLESS_MAIN(TstOpcUaMarshal)

#include "tst_opcua_marshal.moc"
