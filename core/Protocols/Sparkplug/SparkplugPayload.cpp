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

#include "Protocols/Sparkplug/SparkplugPayload.h"

#include <bit>
#include <cmath>
#include <limits>
#include <utility>

#include "Core/SSAssert.h"

namespace Sparkplug = IO::Drivers::SparkplugB;

/**
 * @brief Translation-unit-private helpers for the Sparkplug B reader.
 */
namespace SparkplugDetail {

/**
 * @brief Scratch state for one metric decode. The value oneof arrives as a raw integer whose
 *        meaning depends on the datatype field, and the wire is free to place that field after
 *        the value, so the interpretation is deferred until the metric is fully walked.
 */
struct MetricScratch {
  quint64 raw;
  quint32 field;

  /**
   * @brief Starts a metric decode with no value field seen yet.
   */
  MetricScratch() : raw(0), field(0) {}
};

}  // namespace SparkplugDetail

//--------------------------------------------------------------------------------------------------
// Wire-format primitives
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records a decode failure reason for the caller and returns false, so every guard in the
 *        reader below stays a single `return failDecode(...)` line.
 */
static bool failDecode(QString* error, const QString& reason)
{
  SS_ASSERT_LOG(!reason.isEmpty());

  if (error)
    *error = reason;

  return false;
}

/**
 * @brief Reads a base-128 varint, advancing @p pos only on success. The byte count is capped at
 *        kMaxVarintBytes, so a continuation-bit run in hostile input terminates instead of
 *        walking the buffer.
 */
static bool readVarint(QByteArrayView data, qsizetype& pos, quint64& value) noexcept
{
  SS_ASSERT(pos >= 0, return false);
  SS_ASSERT(pos <= data.size(), return false);

  quint64 result = 0;
  for (int i = 0; i < Sparkplug::kMaxVarintBytes; ++i) {
    if (pos >= data.size())
      return false;

    const auto byte = static_cast<quint8>(data[pos]);
    ++pos;

    result |= static_cast<quint64>(byte & 0x7Fu) << (7 * i);
    if ((byte & 0x80u) == 0) {
      value = result;
      return true;
    }
  }

  return false;
}

/**
 * @brief Reads a little-endian fixed-width field of 4 or 8 bytes, advancing @p pos on success.
 */
static bool readFixed(QByteArrayView data, qsizetype& pos, int width, quint64& value) noexcept
{
  SS_ASSERT(width == 4 || width == 8, return false);
  SS_ASSERT(pos >= 0 && pos <= data.size(), return false);

  if (data.size() - pos < width)
    return false;

  quint64 result = 0;
  for (int i = 0; i < width; ++i)
    result |= static_cast<quint64>(static_cast<quint8>(data[pos + i])) << (8 * i);

  pos   += width;
  value  = result;
  return true;
}

/**
 * @brief Reads a length-delimited block into a sub-view of @p data. The declared length is
 *        checked against the payload cap and against the bytes that actually remain, so a lying
 *        length yields false rather than an out-of-range slice.
 */
static bool readLengthDelimited(QByteArrayView data, qsizetype& pos, QByteArrayView& block)
{
  SS_ASSERT(pos >= 0, return false);
  SS_ASSERT(pos <= data.size(), return false);

  quint64 length = 0;
  if (!readVarint(data, pos, length))
    return false;

  if (length > static_cast<quint64>(Sparkplug::kMaxPayloadBytes))
    return false;

  const auto size = static_cast<qsizetype>(length);
  if (data.size() - pos < size)
    return false;

  block  = data.sliced(pos, size);
  pos   += size;
  return true;
}

/**
 * @brief Splits a protobuf tag into its field number and wire type, rejecting the reserved
 *        number 0 and anything past the 29-bit range. Without the range check a hostile 10-byte
 *        tag would truncate into a modelled field number and be parsed as that field.
 */
static bool splitTag(quint64 tag, quint32& field, quint32& wire) noexcept
{
  const quint64 number = tag >> 3;
  if (number == 0 || number > Sparkplug::kMaxFieldNumber)
    return false;

  field = static_cast<quint32>(number);
  wire  = static_cast<quint32>(tag & 0x07u);
  SS_ASSERT_LOG(field > 0);
  SS_ASSERT_LOG(wire <= 7);
  return true;
}

/**
 * @brief Skips an unmodelled field by its wire type. Group wire types (3 and 4) do not appear in
 *        the Sparkplug B schema and are reported as malformed rather than guessed at.
 */
static bool skipField(QByteArrayView data, qsizetype& pos, quint32 wire)
{
  SS_ASSERT(pos >= 0, return false);
  SS_ASSERT(pos <= data.size(), return false);

  quint64 scratch = 0;
  QByteArrayView block;
  switch (wire) {
    case Sparkplug::kWireVarint:
      return readVarint(data, pos, scratch);
    case Sparkplug::kWireFixed64:
      return readFixed(data, pos, 8, scratch);
    case Sparkplug::kWireFixed32:
      return readFixed(data, pos, 4, scratch);
    case Sparkplug::kWireLengthDelimited:
      return readLengthDelimited(data, pos, block);
    default:
      break;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Metric decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders the raw integer of a value field as a double under the metric's datatype. The
 *        signed codes arrive as two's-complement varints, so the narrow ones are truncated to
 *        their declared width before the sign is restored.
 */
static double numericFromRaw(quint32 datatype, quint64 raw) noexcept
{
  SS_ASSERT_LOG(Sparkplug::kindForDataType(datatype) == Sparkplug::ValueKind::Numeric);
  SS_ASSERT_LOG(datatype >= 1);

  switch (static_cast<Sparkplug::DataType>(datatype)) {
    case Sparkplug::DataType::Int8:
      return static_cast<double>(static_cast<qint8>(static_cast<quint8>(raw)));
    case Sparkplug::DataType::Int16:
      return static_cast<double>(static_cast<qint16>(static_cast<quint16>(raw)));
    case Sparkplug::DataType::Int32:
      return static_cast<double>(static_cast<qint32>(static_cast<quint32>(raw)));
    case Sparkplug::DataType::Int64:
      return static_cast<double>(static_cast<qint64>(raw));
    case Sparkplug::DataType::UInt8:
      return static_cast<double>(static_cast<quint8>(raw));
    case Sparkplug::DataType::UInt16:
      return static_cast<double>(static_cast<quint16>(raw));
    case Sparkplug::DataType::UInt32:
      return static_cast<double>(static_cast<quint32>(raw));
    case Sparkplug::DataType::UInt64:
      return static_cast<double>(raw);
    case Sparkplug::DataType::Float:
      return static_cast<double>(std::bit_cast<float>(static_cast<quint32>(raw)));
    case Sparkplug::DataType::Double:
      return std::bit_cast<double>(raw);
    case Sparkplug::DataType::Unknown:
    case Sparkplug::DataType::Boolean:
    case Sparkplug::DataType::String:
    case Sparkplug::DataType::DateTime:
    case Sparkplug::DataType::Text:
    case Sparkplug::DataType::Uuid:
      break;
    default:
      break;
  }

  return 0.0;
}

/**
 * @brief Resolves the value oneof once the whole metric is walked: marks the datatype support,
 *        assigns the channel, and renders the raw integer. A null metric or one that carried no
 *        value field keeps its default-constructed value.
 */
static void finalizeMetric(Sparkplug::Metric& out, const SparkplugDetail::MetricScratch& scratch)
{
  SS_ASSERT_LOG(out.kind == Sparkplug::ValueKind::None);
  SS_ASSERT_LOG(scratch.field == 0 || scratch.field >= Sparkplug::kMetricIntValue);

  out.supported = Sparkplug::isSupportedDataType(out.datatype);
  out.kind      = Sparkplug::kindForDataType(out.datatype);
  if (!out.supported || out.isNull || scratch.field == 0)
    return;

  if (out.kind == Sparkplug::ValueKind::Boolean)
    out.boolValue = (scratch.raw != 0);

  if (out.kind == Sparkplug::ValueKind::Numeric)
    out.numericValue = numericFromRaw(out.datatype, scratch.raw);
}

/**
 * @brief Reads the metric name or the string value; both are length-delimited UTF-8. The name is
 *        capped at the identity limit rather than the value limit, because a name is retained per
 *        slot while a value is overwritten, so an oversized one fails the decode.
 */
static bool readMetricString(QByteArrayView data,
                             qsizetype& pos,
                             quint32 field,
                             quint32 wire,
                             Sparkplug::Metric& out,
                             SparkplugDetail::MetricScratch& scratch)
{
  SS_ASSERT(field == Sparkplug::kMetricName || field == Sparkplug::kMetricStringValue,
            return false);
  SS_ASSERT(pos >= 0 && pos <= data.size(), return false);

  if (wire != Sparkplug::kWireLengthDelimited)
    return false;

  QByteArrayView block;
  if (!readLengthDelimited(data, pos, block))
    return false;

  const bool is_name    = (field == Sparkplug::kMetricName);
  const qsizetype limit = is_name ? Sparkplug::kMaxIdentityBytes : Sparkplug::kMaxStringBytes;
  if (block.size() > limit)
    return false;

  if (is_name) {
    out.name    = QString::fromUtf8(block.data(), block.size());
    out.hasName = true;
    return true;
  }

  out.stringValue = QString::fromUtf8(block.data(), block.size());
  scratch.field   = field;
  return true;
}

/**
 * @brief Reads the fixed-width float or double value field into the scratch, rejecting a wire
 *        type that disagrees with the field's declared width.
 */
static bool readMetricReal(QByteArrayView data,
                           qsizetype& pos,
                           quint32 field,
                           quint32 wire,
                           SparkplugDetail::MetricScratch& scratch)
{
  SS_ASSERT(field == Sparkplug::kMetricFloatValue || field == Sparkplug::kMetricDoubleValue,
            return false);
  SS_ASSERT(pos >= 0 && pos <= data.size(), return false);

  const bool is_float    = (field == Sparkplug::kMetricFloatValue);
  const quint32 expected = is_float ? Sparkplug::kWireFixed32 : Sparkplug::kWireFixed64;
  if (wire != expected)
    return false;

  quint64 raw = 0;
  if (!readFixed(data, pos, is_float ? 4 : 8, raw))
    return false;

  scratch.raw   = raw;
  scratch.field = field;
  return true;
}

/**
 * @brief Reads any varint-encoded metric field: the alias, timestamp, datatype and is_null
 *        headers land in the metric, the int, long and boolean value fields in the scratch.
 */
static bool readMetricVarint(QByteArrayView data,
                             qsizetype& pos,
                             quint32 field,
                             quint32 wire,
                             Sparkplug::Metric& out,
                             SparkplugDetail::MetricScratch& scratch)
{
  SS_ASSERT(field > 0, return false);
  SS_ASSERT(pos >= 0 && pos <= data.size(), return false);

  if (wire != Sparkplug::kWireVarint)
    return false;

  quint64 raw = 0;
  if (!readVarint(data, pos, raw))
    return false;

  switch (field) {
    case Sparkplug::kMetricAlias:
      out.alias    = raw;
      out.hasAlias = true;
      return true;
    case Sparkplug::kMetricTimestamp:
      out.timestampMs = raw;
      return true;
    case Sparkplug::kMetricDatatype:
      out.datatype = static_cast<quint32>(raw);
      return true;
    case Sparkplug::kMetricIsNull:
      out.isNull = (raw != 0);
      return true;
    default:
      break;
  }

  scratch.raw   = raw;
  scratch.field = field;
  return true;
}

/**
 * @brief Routes one metric field to its reader. Field numbers the codec does not model are
 *        skipped by wire type, which is what keeps a future schema revision readable.
 */
static bool readMetricField(QByteArrayView data,
                            qsizetype& pos,
                            quint32 field,
                            quint32 wire,
                            Sparkplug::Metric& out,
                            SparkplugDetail::MetricScratch& scratch)
{
  SS_ASSERT(field > 0, return false);
  SS_ASSERT(pos >= 0 && pos <= data.size(), return false);

  if (field == Sparkplug::kMetricName || field == Sparkplug::kMetricStringValue)
    return readMetricString(data, pos, field, wire, out, scratch);

  if (field == Sparkplug::kMetricFloatValue || field == Sparkplug::kMetricDoubleValue)
    return readMetricReal(data, pos, field, wire, scratch);

  const bool is_varint = field == Sparkplug::kMetricAlias || field == Sparkplug::kMetricTimestamp
                      || field == Sparkplug::kMetricDatatype || field == Sparkplug::kMetricIsNull
                      || field == Sparkplug::kMetricIntValue || field == Sparkplug::kMetricLongValue
                      || field == Sparkplug::kMetricBooleanValue;
  if (is_varint)
    return readMetricVarint(data, pos, field, wire, out, scratch);

  return skipField(data, pos, wire);
}

/**
 * @brief Decodes one Metric sub-message. The loop bound is the block size because every
 *        iteration consumes at least the one byte of its tag, and the closing cursor check is
 *        what rejects a block whose last field ran past its own length.
 */
static bool decodeMetric(QByteArrayView data, Sparkplug::Metric& out, QString* error)
{
  SS_ASSERT(data.size() >= 0, return failDecode(error, QStringLiteral("invalid metric view")));
  SS_ASSERT_LOG(out.kind == Sparkplug::ValueKind::None);

  SparkplugDetail::MetricScratch scratch;
  qsizetype pos = 0;
  for (qsizetype i = 0; i < data.size() && pos < data.size(); ++i) {
    quint64 tag = 0;
    if (!readVarint(data, pos, tag))
      return failDecode(error, QStringLiteral("truncated metric field tag"));

    quint32 field = 0;
    quint32 wire  = 0;
    if (!splitTag(tag, field, wire))
      return failDecode(error, QStringLiteral("malformed metric field tag"));

    if (!readMetricField(data, pos, field, wire, out, scratch))
      return failDecode(error, QStringLiteral("malformed metric field %1").arg(field));
  }

  if (pos != data.size())
    return failDecode(error, QStringLiteral("metric field count exceeded"));

  finalizeMetric(out, scratch);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Payload decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the payload timestamp or sequence number, both varint uint64.
 */
static bool readPayloadVarint(
  QByteArrayView data, qsizetype& pos, quint32 field, quint32 wire, Sparkplug::Payload& out)
{
  SS_ASSERT(field == Sparkplug::kPayloadTimestamp || field == Sparkplug::kPayloadSeq, return false);
  SS_ASSERT(pos >= 0 && pos <= data.size(), return false);

  if (wire != Sparkplug::kWireVarint)
    return false;

  quint64 raw = 0;
  if (!readVarint(data, pos, raw))
    return false;

  if (field == Sparkplug::kPayloadTimestamp) {
    out.timestampMs  = raw;
    out.hasTimestamp = true;
    return true;
  }

  out.seq    = raw;
  out.hasSeq = true;
  return true;
}

/**
 * @brief Reads one repeated metrics entry. The count cap is checked before the metric is
 *        appended, so a payload claiming thousands of metrics never grows the vector.
 */
static bool readPayloadMetric(
  QByteArrayView data, qsizetype& pos, quint32 wire, Sparkplug::Payload& out, QString* error)
{
  SS_ASSERT(pos >= 0 && pos <= data.size(),
            return failDecode(error, QStringLiteral("invalid payload cursor")));
  SS_ASSERT_LOG(out.metrics.size() <= Sparkplug::kMaxMetrics);

  if (wire != Sparkplug::kWireLengthDelimited)
    return failDecode(error, QStringLiteral("metrics field has wire type %1").arg(wire));

  QByteArrayView block;
  if (!readLengthDelimited(data, pos, block))
    return failDecode(error, QStringLiteral("truncated metric"));

  if (out.metrics.size() >= Sparkplug::kMaxMetrics)
    return failDecode(
      error, QStringLiteral("payload carries more than %1 metrics").arg(Sparkplug::kMaxMetrics));

  Sparkplug::Metric metric;
  if (!decodeMetric(block, metric, error))
    return false;

  out.metrics.append(std::move(metric));
  return true;
}

/**
 * @brief Routes one payload field to its reader; the uuid and body fields, and anything a
 *        future schema revision adds, are skipped by wire type.
 */
static bool readPayloadField(QByteArrayView data,
                             qsizetype& pos,
                             quint32 field,
                             quint32 wire,
                             Sparkplug::Payload& out,
                             QString* error)
{
  SS_ASSERT(pos >= 0 && pos <= data.size(),
            return failDecode(error, QStringLiteral("invalid payload cursor")));
  SS_ASSERT(field > 0, return failDecode(error, QStringLiteral("field number 0 is reserved")));

  if (field == Sparkplug::kPayloadMetrics)
    return readPayloadMetric(data, pos, wire, out, error);

  const bool is_scalar = field == Sparkplug::kPayloadTimestamp || field == Sparkplug::kPayloadSeq;
  if (is_scalar && !readPayloadVarint(data, pos, field, wire, out))
    return failDecode(error, QStringLiteral("malformed payload field %1").arg(field));

  if (!is_scalar && !skipField(data, pos, wire))
    return failDecode(error, QStringLiteral("malformed payload field %1").arg(field));

  return true;
}

//--------------------------------------------------------------------------------------------------
// Encoding primitives
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a base-128 varint. The loop is bounded at kMaxVarintBytes, which is the exact
 *        worst case for a 64-bit value, so the trailing invariant can never be reached.
 */
static void appendVarint(QByteArray& out, quint64 value)
{
  SS_ASSERT_LOG(out.size() >= 0);

  quint64 remaining = value;
  for (int i = 0; i < Sparkplug::kMaxVarintBytes; ++i) {
    const auto byte   = static_cast<quint8>(remaining & 0x7Fu);
    remaining       >>= 7;
    if (remaining == 0) {
      out.append(static_cast<char>(byte));
      return;
    }

    out.append(static_cast<char>(byte | 0x80u));
  }

  SS_ASSERT_LOG(remaining == 0);
}

/**
 * @brief Appends a protobuf tag, the field number shifted left by three with the wire type in
 *        the low bits.
 */
static void appendTag(QByteArray& out, quint32 field, quint32 wire)
{
  SS_ASSERT(field > 0, return);
  SS_ASSERT(wire <= Sparkplug::kWireFixed32, return);

  appendVarint(out, (static_cast<quint64>(field) << 3) | wire);
}

/**
 * @brief Appends a varint-encoded field: its tag, then the value.
 */
static void appendVarintField(QByteArray& out, quint32 field, quint64 value)
{
  SS_ASSERT(field > 0, return);
  SS_ASSERT_LOG(out.size() >= 0);

  appendTag(out, field, Sparkplug::kWireVarint);
  appendVarint(out, value);
}

/**
 * @brief Appends a length-delimited field: its tag, the declared byte count, then the block.
 */
static void appendLengthDelimited(QByteArray& out, quint32 field, QByteArrayView block)
{
  SS_ASSERT(field > 0, return);
  SS_ASSERT(block.size() >= 0, return);

  appendTag(out, field, Sparkplug::kWireLengthDelimited);
  appendVarint(out, static_cast<quint64>(block.size()));
  if (!block.isEmpty())
    out.append(block.data(), block.size());
}

/**
 * @brief Appends a little-endian fixed-width field of 4 or 8 bytes.
 */
static void appendFixedField(QByteArray& out, quint32 field, quint64 bits, int width)
{
  SS_ASSERT(field > 0, return);
  SS_ASSERT(width == 4 || width == 8, return);

  appendTag(out, field, width == 4 ? Sparkplug::kWireFixed32 : Sparkplug::kWireFixed64);
  for (int i = 0; i < width; ++i)
    out.append(static_cast<char>((bits >> (8 * i)) & 0xFFu));
}

/**
 * @brief Whether an integral @p value lies inside the declared integer code's range. The 64-bit
 *        codes are bounded with a strict upper comparison because their own maximum is not
 *        representable as a double, so an inclusive test would accept 2^63 as int64.
 */
static bool integerFitsDataType(quint32 datatype, double value) noexcept
{
  SS_ASSERT_LOG(std::isfinite(value));
  SS_ASSERT_LOG(value == std::trunc(value));

  switch (static_cast<Sparkplug::DataType>(datatype)) {
    case Sparkplug::DataType::Int8:
      return value >= -128.0 && value <= 127.0;
    case Sparkplug::DataType::Int16:
      return value >= -32768.0 && value <= 32767.0;
    case Sparkplug::DataType::Int32:
      return value >= -2147483648.0 && value <= 2147483647.0;
    case Sparkplug::DataType::Int64:
      return value >= -9223372036854775808.0 && value < 9223372036854775808.0;
    case Sparkplug::DataType::UInt8:
      return value >= 0.0 && value <= 255.0;
    case Sparkplug::DataType::UInt16:
      return value >= 0.0 && value <= 65535.0;
    case Sparkplug::DataType::UInt32:
      return value >= 0.0 && value <= 4294967295.0;
    case Sparkplug::DataType::UInt64:
      return value >= 0.0 && value < 18446744073709551616.0;
    default:
      break;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Metric encoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Value-oneof field number a datatype writes into, mirroring the reader's routing: the
 *        narrow integers share int_value, the 64-bit ones long_value, and the rest have a field
 *        of their own. An unrenderable code has none.
 */
static quint32 valueFieldFor(quint32 datatype) noexcept
{
  switch (static_cast<Sparkplug::DataType>(datatype)) {
    case Sparkplug::DataType::Int8:
    case Sparkplug::DataType::Int16:
    case Sparkplug::DataType::Int32:
    case Sparkplug::DataType::UInt8:
    case Sparkplug::DataType::UInt16:
    case Sparkplug::DataType::UInt32:
      return Sparkplug::kMetricIntValue;
    case Sparkplug::DataType::Int64:
    case Sparkplug::DataType::UInt64:
      return Sparkplug::kMetricLongValue;
    case Sparkplug::DataType::Float:
      return Sparkplug::kMetricFloatValue;
    case Sparkplug::DataType::Double:
      return Sparkplug::kMetricDoubleValue;
    case Sparkplug::DataType::Boolean:
      return Sparkplug::kMetricBooleanValue;
    case Sparkplug::DataType::String:
    case Sparkplug::DataType::Text:
    case Sparkplug::DataType::Uuid:
      return Sparkplug::kMetricStringValue;
    default:
      break;
  }

  return 0;
}

/**
 * @brief Bit pattern a numeric value travels as under its declared datatype: truncated to the
 *        declared width and widened with that width's signedness, which is the exact inverse of
 *        the reader's rendering and what Tahu itself writes for the narrow integer codes.
 */
static quint64 rawFromNumeric(quint32 datatype, double value) noexcept
{
  SS_ASSERT(Sparkplug::numericFitsDataType(datatype, value), return 0);
  SS_ASSERT_LOG(Sparkplug::kindForDataType(datatype) == Sparkplug::ValueKind::Numeric);

  switch (static_cast<Sparkplug::DataType>(datatype)) {
    case Sparkplug::DataType::Int8:
      return static_cast<quint8>(static_cast<qint8>(value));
    case Sparkplug::DataType::Int16:
      return static_cast<quint16>(static_cast<qint16>(value));
    case Sparkplug::DataType::Int32:
      return static_cast<quint32>(static_cast<qint32>(value));
    case Sparkplug::DataType::Int64:
      return static_cast<quint64>(static_cast<qint64>(value));
    case Sparkplug::DataType::UInt8:
    case Sparkplug::DataType::UInt16:
    case Sparkplug::DataType::UInt32:
    case Sparkplug::DataType::UInt64:
      return static_cast<quint64>(value);
    case Sparkplug::DataType::Float:
      return std::bit_cast<quint32>(static_cast<float>(value));
    case Sparkplug::DataType::Double:
      return std::bit_cast<quint64>(value);
    default:
      break;
  }

  return 0;
}

/**
 * @brief Writes the value oneof a metric's datatype selects. Returns false when the value cannot
 *        be rendered, so the caller drops the whole metric instead of shipping it mistyped.
 */
static bool encodeMetricValue(QByteArray& out, const Sparkplug::Metric& metric)
{
  const auto kind = Sparkplug::kindForDataType(metric.datatype);
  SS_ASSERT(kind != Sparkplug::ValueKind::None, return false);
  SS_ASSERT_LOG(valueFieldFor(metric.datatype) > 0);

  if (kind == Sparkplug::ValueKind::String) {
    const QByteArray utf8 = metric.stringValue.toUtf8();
    if (utf8.size() > Sparkplug::kMaxStringBytes)
      return false;

    appendLengthDelimited(out, Sparkplug::kMetricStringValue, utf8);
    return true;
  }

  if (kind == Sparkplug::ValueKind::Boolean) {
    appendVarintField(out, Sparkplug::kMetricBooleanValue, metric.boolValue ? 1 : 0);
    return true;
  }

  if (!Sparkplug::numericFitsDataType(metric.datatype, metric.numericValue))
    return false;

  const quint32 field = valueFieldFor(metric.datatype);
  const quint64 raw   = rawFromNumeric(metric.datatype, metric.numericValue);
  if (field == Sparkplug::kMetricFloatValue || field == Sparkplug::kMetricDoubleValue) {
    appendFixedField(out, field, raw, field == Sparkplug::kMetricFloatValue ? 4 : 8);
    return true;
  }

  appendVarintField(out, field, raw);
  return true;
}

/**
 * @brief Writes one Metric sub-message into @p out: identity, header fields, then the value.
 *        A metric whose identity or value does not fit the codec's caps returns false and is
 *        dropped by the caller, because a truncated name is a different metric to the host.
 */
static bool encodeMetric(const Sparkplug::Metric& metric, QByteArray& out)
{
  SS_ASSERT(out.isEmpty(), return false);
  SS_ASSERT_LOG(metric.hasName || metric.hasAlias);

  if (metric.hasName) {
    const QByteArray name = metric.name.toUtf8();
    if (name.size() > Sparkplug::kMaxIdentityBytes)
      return false;

    appendLengthDelimited(out, Sparkplug::kMetricName, name);
  }

  if (metric.hasAlias)
    appendVarintField(out, Sparkplug::kMetricAlias, metric.alias);

  if (metric.timestampMs != 0)
    appendVarintField(out, Sparkplug::kMetricTimestamp, metric.timestampMs);

  appendVarintField(out, Sparkplug::kMetricDatatype, metric.datatype);

  if (metric.isNull) {
    appendVarintField(out, Sparkplug::kMetricIsNull, 1);
    return true;
  }

  return Sparkplug::isSupportedDataType(metric.datatype) && encodeMetricValue(out, metric);
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes a Sparkplug B payload, replacing @p out and writing a reason into @p error on
 *        failure. The input is broker traffic, so the size cap is applied before the walk and
 *        the loop is bounded by the byte count: every iteration consumes at least its tag byte.
 */
bool IO::Drivers::SparkplugB::decodePayload(QByteArrayView data, Payload& out, QString* error)
{
  SS_ASSERT(data.size() >= 0, return failDecode(error, QStringLiteral("invalid payload view")));

  out = Payload();
  if (error)
    error->clear();

  if (data.size() > kMaxPayloadBytes)
    return failDecode(error, QStringLiteral("payload exceeds %1 bytes").arg(kMaxPayloadBytes));

  qsizetype pos = 0;
  for (qsizetype i = 0; i < data.size() && pos < data.size(); ++i) {
    quint64 tag = 0;
    if (!readVarint(data, pos, tag))
      return failDecode(error, QStringLiteral("truncated payload field tag"));

    quint32 field = 0;
    quint32 wire  = 0;
    if (!splitTag(tag, field, wire))
      return failDecode(error, QStringLiteral("malformed payload field tag"));

    if (!readPayloadField(data, pos, field, wire, out, error))
      return false;
  }

  if (pos != data.size())
    return failDecode(error, QStringLiteral("payload field count exceeded"));

  SS_ASSERT_LOG(out.metrics.size() <= kMaxMetrics);
  return true;
}

/**
 * @brief Whether @p value survives a round trip through @p datatype: finite everywhere, integral
 *        and in range for the integer codes, inside the float range for Float. A value failing
 *        this is the one an edge node must skip and count (spec 0073 R44): rendering it anyway
 *        would ship a wrong number under a right type, which no host can detect.
 */
bool IO::Drivers::SparkplugB::numericFitsDataType(quint32 datatype, double value) noexcept
{
  if (kindForDataType(datatype) != ValueKind::Numeric || !std::isfinite(value))
    return false;

  switch (static_cast<DataType>(datatype)) {
    case DataType::Double:
      return true;
    case DataType::Float:
      return std::fabs(value) <= static_cast<double>(std::numeric_limits<float>::max());
    default:
      break;
  }

  if (value != std::trunc(value))
    return false;

  return integerFitsDataType(datatype, value);
}

/**
 * @brief Encodes a Sparkplug B payload: the header scalars the message carries, then every metric
 *        that renders, each as one repeated entry. A metric the codec cannot express is dropped
 *        rather than shipped mistyped; callers check @ref numericFitsDataType first so they can
 *        count what they skipped instead of discovering it here.
 */
QByteArray IO::Drivers::SparkplugB::encodePayload(const Payload& payload)
{
  SS_ASSERT(payload.metrics.size() <= kMaxMetrics, return QByteArray());
  SS_ASSERT_LOG(payload.seq < 256 || !payload.hasSeq);

  QByteArray out;
  out.reserve(32 + payload.metrics.size() * 32);

  if (payload.hasTimestamp)
    appendVarintField(out, kPayloadTimestamp, payload.timestampMs);

  if (payload.hasSeq)
    appendVarintField(out, kPayloadSeq, payload.seq);

  QByteArray metric;
  metric.reserve(64);
  for (const auto& entry : payload.metrics) {
    metric.resize(0);
    if (!encodeMetric(entry, metric))
      continue;

    appendLengthDelimited(out, kPayloadMetrics, metric);
  }

  SS_ASSERT_LOG(out.size() <= kMaxPayloadBytes);
  return out;
}

/**
 * @brief Encodes the NCMD payload that asks an edge node to re-publish its birth certificate: a
 *        payload timestamp plus one boolean metric "Node Control/Rebirth" set true. The sequence
 *        number is deliberately absent, as command payloads carry none.
 */
QByteArray IO::Drivers::SparkplugB::encodeRebirthRequest(quint64 timestampMs)
{
  const QByteArray name(kRebirthMetricName);
  SS_ASSERT(!name.isEmpty(), return QByteArray());

  QByteArray metric;
  metric.reserve(name.size() + 16);
  appendTag(metric, kMetricName, kWireLengthDelimited);
  appendVarint(metric, static_cast<quint64>(name.size()));
  metric.append(name);
  appendTag(metric, kMetricDatatype, kWireVarint);
  appendVarint(metric, static_cast<quint64>(DataType::Boolean));
  appendTag(metric, kMetricBooleanValue, kWireVarint);
  appendVarint(metric, 1);

  QByteArray payload;
  payload.reserve(metric.size() + 16);
  appendTag(payload, kPayloadTimestamp, kWireVarint);
  appendVarint(payload, timestampMs);
  appendTag(payload, kPayloadMetrics, kWireLengthDelimited);
  appendVarint(payload, static_cast<quint64>(metric.size()));
  payload.append(metric);

  SS_ASSERT_LOG(payload.size() > metric.size());
  return payload;
}
