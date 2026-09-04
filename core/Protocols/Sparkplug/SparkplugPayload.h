/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QtGlobal>
#include <QVector>

namespace IO {
namespace Drivers {

/**
 * @brief Sparkplug B v1.0 payload vocabulary (Eclipse Tahu sparkplug_b.proto), read and written
 *        by a hand-rolled protobuf codec so no protobuf runtime is linked and the ctest tier can
 *        exercise it against Qt Core alone. Payloads arrive from a broker, so the reader treats
 *        every byte as hostile: reads are bounds-checked, varints are capped, and the fixed caps
 *        below fail a decode rather than allocating on demand. The writer emits the same field
 *        numbers the reader declares, so everything it produces decodes back.
 */
namespace SparkplugB {

// Payload field numbers; 1 and 3 are varints, 2, 4 and 5 length-delimited (4 uuid, 5 body)
inline constexpr quint32 kPayloadTimestamp = 1;
inline constexpr quint32 kPayloadMetrics   = 2;
inline constexpr quint32 kPayloadSeq       = 3;
inline constexpr quint32 kPayloadUuid      = 4;
inline constexpr quint32 kPayloadBody      = 5;

// Metric header field numbers; 5, 6, 8, 9 and 16-20 carry metadata this codec skips by wire type
inline constexpr quint32 kMetricName      = 1;
inline constexpr quint32 kMetricAlias     = 2;
inline constexpr quint32 kMetricTimestamp = 3;
inline constexpr quint32 kMetricDatatype  = 4;
inline constexpr quint32 kMetricIsNull    = 7;

// Metric value oneof; 10, 11 and 14 are varints, 12 is fixed32, 13 fixed64, 15 length-delimited
inline constexpr quint32 kMetricIntValue     = 10;
inline constexpr quint32 kMetricLongValue    = 11;
inline constexpr quint32 kMetricFloatValue   = 12;
inline constexpr quint32 kMetricDoubleValue  = 13;
inline constexpr quint32 kMetricBooleanValue = 14;
inline constexpr quint32 kMetricStringValue  = 15;

// Protobuf wire types; the Sparkplug B schema uses no groups, so 3 and 4 are always malformed
inline constexpr quint32 kWireVarint          = 0;
inline constexpr quint32 kWireFixed64         = 1;
inline constexpr quint32 kWireLengthDelimited = 2;
inline constexpr quint32 kWireFixed32         = 5;

// Largest field number a valid protobuf tag can carry; a wider one is a malformed tag
inline constexpr quint64 kMaxFieldNumber = 0x1FFFFFFFull;

// Fixed caps applied to broker traffic; exceeding any one of them fails the decode with a reason
inline constexpr qsizetype kMaxPayloadBytes = 256 * 1024;
inline constexpr qsizetype kMaxStringBytes  = 64 * 1024;
inline constexpr int kMaxMetrics            = 2048;
inline constexpr int kMaxVarintBytes        = 10;

// Longest identity (metric name, topic element); retained per slot, capped at what the wire emits
inline constexpr qsizetype kMaxIdentityBytes = 256;

// NCMD metric that asks an edge node to re-publish its birth certificate
inline constexpr const char* kRebirthMetricName = "Node Control/Rebirth";

/**
 * @brief Sparkplug B datatype codes; the numeric value is the wire value carried in Metric field
 *        4. Codes outside the scalar range this codec renders (13 DateTime, the array, template
 *        and dataset codes) leave the metric marked unsupported so the caller can count it.
 */
enum class DataType : quint32 {
  Unknown  = 0,
  Int8     = 1,
  Int16    = 2,
  Int32    = 3,
  Int64    = 4,
  UInt8    = 5,
  UInt16   = 6,
  UInt32   = 7,
  UInt64   = 8,
  Float    = 9,
  Double   = 10,
  Boolean  = 11,
  String   = 12,
  DateTime = 13,
  Text     = 14,
  Uuid     = 15,
};

/**
 * @brief Channel a decoded metric feeds: a numeric slot, a boolean slot, a string slot, or none
 *        when the datatype is unsupported.
 */
enum class ValueKind : quint8 {
  None    = 0,
  Numeric = 1,
  Boolean = 2,
  String  = 3,
};

/**
 * @brief Channel for a datatype code. Codes 1-12 are the numeric, boolean and string scalars;
 *        14 Text and 15 UUID are strings on the wire and are rendered as such. Anything else,
 *        including 13 DateTime, has no channel here.
 */
[[nodiscard]] inline constexpr ValueKind kindForDataType(quint32 datatype) noexcept
{
  switch (static_cast<DataType>(datatype)) {
    case DataType::Int8:
    case DataType::Int16:
    case DataType::Int32:
    case DataType::Int64:
    case DataType::UInt8:
    case DataType::UInt16:
    case DataType::UInt32:
    case DataType::UInt64:
    case DataType::Float:
    case DataType::Double:
      return ValueKind::Numeric;
    case DataType::Boolean:
      return ValueKind::Boolean;
    case DataType::String:
    case DataType::Text:
    case DataType::Uuid:
      return ValueKind::String;
    case DataType::Unknown:
    case DataType::DateTime:
      break;
    default:
      break;
  }

  return ValueKind::None;
}

/**
 * @brief Whether the codec can render a value for this datatype code.
 */
[[nodiscard]] inline constexpr bool isSupportedDataType(quint32 datatype) noexcept
{
  return kindForDataType(datatype) != ValueKind::None;
}

/**
 * @brief One decoded metric. A birth certificate carries the name, later data messages carry the
 *        alias alone, so both identifiers have their own presence flag. `numericValue` and
 *        `boolValue` are meaningful only for the matching `kind`, and only when `isNull` is
 *        false and `supported` is true.
 */
struct Metric {
  QString name;
  QString stringValue;
  quint64 alias;
  quint64 timestampMs;
  double numericValue;
  quint32 datatype;
  ValueKind kind;
  bool boolValue;
  bool isNull;
  bool supported;
  bool hasName;
  bool hasAlias;

  Metric()
    : alias(0)
    , timestampMs(0)
    , numericValue(0.0)
    , datatype(0)
    , kind(ValueKind::None)
    , boolValue(false)
    , isNull(false)
    , supported(false)
    , hasName(false)
    , hasAlias(false)
  {}
};

/**
 * @brief One decoded Sparkplug B payload. The uuid and body fields are skipped rather than
 *        stored: nothing in the session layer reads them.
 */
struct Payload {
  QVector<Metric> metrics;
  quint64 timestampMs;
  quint64 seq;
  bool hasTimestamp;
  bool hasSeq;

  Payload() : timestampMs(0), seq(0), hasTimestamp(false), hasSeq(false) {}
};

[[nodiscard]] QByteArray encodePayload(const Payload& payload);
[[nodiscard]] QByteArray encodeRebirthRequest(quint64 timestampMs);
[[nodiscard]] bool numericFitsDataType(quint32 datatype, double value) noexcept;
[[nodiscard]] bool decodePayload(QByteArrayView data, Payload& out, QString* error = nullptr);

}  // namespace SparkplugB
}  // namespace Drivers
}  // namespace IO
