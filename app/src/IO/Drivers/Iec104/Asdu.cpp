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

#include "IO/Drivers/Iec104/Asdu.h"

#include <cstring>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>

#include "SSAssert.h"

static constexpr double kNormalizedScale = 32768.0;
static constexpr int kMaxMillisecond     = 59999;

/**
 * @brief Reads one octet as an unsigned byte; the caller has already bounds-checked the position.
 */
[[nodiscard]] static inline std::uint8_t iec104Octet(QByteArrayView view, qsizetype pos) noexcept
{
  SS_ASSERT(pos >= 0 && pos < view.size(), return 0);

  return static_cast<std::uint8_t>(view[pos]);
}

/**
 * @brief Reads a little-endian 16-bit word.
 */
[[nodiscard]] static quint16 readLe16(QByteArrayView view, qsizetype pos) noexcept
{
  return static_cast<quint16>(iec104Octet(view, pos) | (iec104Octet(view, pos + 1) << 8));
}

/**
 * @brief Reads a little-endian 32-bit word.
 */
[[nodiscard]] static quint32 readLe32(QByteArrayView view, qsizetype pos) noexcept
{
  return static_cast<quint32>(iec104Octet(view, pos))
       | (static_cast<quint32>(iec104Octet(view, pos + 1)) << 8)
       | (static_cast<quint32>(iec104Octet(view, pos + 2)) << 16)
       | (static_cast<quint32>(iec104Octet(view, pos + 3)) << 24);
}

/**
 * @brief Reads the three-octet information object address.
 */
[[nodiscard]] static quint32 readIoa(QByteArrayView view, qsizetype pos) noexcept
{
  return static_cast<quint32>(iec104Octet(view, pos))
       | (static_cast<quint32>(iec104Octet(view, pos + 1)) << 8)
       | (static_cast<quint32>(iec104Octet(view, pos + 2)) << 16);
}

/**
 * @brief Normalizes the BL/SB/NT/IV flags the SIQ, DIQ and QDS descriptors share in their four
 *        high bits. The OV bit exists only in QDS and is folded in by its own reader.
 */
[[nodiscard]] static std::uint8_t commonQuality(std::uint8_t descriptor) noexcept
{
  using namespace IO::Drivers::Iec104Proto;

  std::uint8_t quality  = QualityGood;
  quality              |= (descriptor & 0x10) ? QualityBlocked : 0;
  quality              |= (descriptor & 0x20) ? QualitySubstituted : 0;
  quality              |= (descriptor & 0x40) ? QualityNotTopical : 0;
  quality              |= (descriptor & 0x80) ? QualityInvalid : 0;
  return quality;
}

/**
 * @brief Normalizes a QDS descriptor, whose low bit reports an overflowed measurand.
 */
[[nodiscard]] static std::uint8_t measurandQuality(std::uint8_t qds) noexcept
{
  using namespace IO::Drivers::Iec104Proto;

  const std::uint8_t overflow = (qds & 0x01) ? QualityOverflow : QualityGood;
  return static_cast<std::uint8_t>(commonQuality(qds) | overflow);
}

/**
 * @brief Normalizes a BCR sequence octet: a counter that carried reads as an overflow and one the
 *        station adjusted reads as substituted, which is what those two conditions mean downstream.
 */
[[nodiscard]] static std::uint8_t counterQuality(std::uint8_t sequence) noexcept
{
  using namespace IO::Drivers::Iec104Proto;

  std::uint8_t quality  = QualityGood;
  quality              |= (sequence & 0x20) ? QualityOverflow : 0;
  quality              |= (sequence & 0x40) ? QualitySubstituted : 0;
  quality              |= (sequence & 0x80) ? QualityInvalid : 0;
  return quality;
}

//--------------------------------------------------------------------------------------------------
// Type table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the value class a type identification carries, or Invalid for anything this build
 *        does not decode.
 */
IO::Drivers::Iec104Proto::PointKind IO::Drivers::Iec104Proto::kindForType(
  std::uint8_t typeId) noexcept
{
  switch (typeId) {
    case kTypeSinglePoint:
    case kTypeSinglePointTime:
      return PointKind::Single;
    case kTypeDoublePoint:
    case kTypeDoublePointTime:
      return PointKind::Double;
    case kTypeMeasuredNormalized:
    case kTypeMeasuredNormalTime:
      return PointKind::Normalized;
    case kTypeMeasuredScaled:
    case kTypeMeasuredScaledTime:
      return PointKind::Scaled;
    case kTypeMeasuredFloat:
    case kTypeMeasuredFloatTime:
      return PointKind::Float;
    case kTypeIntegratedTotals:
    case kTypeIntegratedTotalsTime:
      return PointKind::Counter;
    default:
      break;
  }

  return PointKind::Invalid;
}

/**
 * @brief True for the twelve monitor-direction types this build decodes into points.
 */
bool IO::Drivers::Iec104Proto::isMonitorType(std::uint8_t typeId) noexcept
{
  return kindForType(typeId) != PointKind::Invalid;
}

/**
 * @brief True for a type this build understands, including the two that carry no measurand: the
 *        end-of-initialization report and the interrogation command's own confirmations. Counting
 *        those as skipped would make a healthy session look like it was dropping data.
 */
bool IO::Drivers::Iec104Proto::isRecognizedType(std::uint8_t typeId) noexcept
{
  if (isMonitorType(typeId))
    return true;

  return typeId == kTypeEndOfInitialization || typeId == kTypeInterrogation;
}

/**
 * @brief True for the six type identifications whose element ends in a CP56Time2a stamp.
 */
bool IO::Drivers::Iec104Proto::typeCarriesTime(std::uint8_t typeId) noexcept
{
  switch (typeId) {
    case kTypeSinglePointTime:
    case kTypeDoublePointTime:
    case kTypeMeasuredNormalTime:
    case kTypeMeasuredScaledTime:
    case kTypeMeasuredFloatTime:
    case kTypeIntegratedTotalsTime:
      return true;
    default:
      break;
  }

  return false;
}

/**
 * @brief Returns the encoded width of ONE information element, stamp included, or -1 for a type
 *        this build does not decode.
 */
int IO::Drivers::Iec104Proto::elementBytes(std::uint8_t typeId) noexcept
{
  const auto kind = kindForType(typeId);
  const int stamp = typeCarriesTime(typeId) ? kCp56Bytes : 0;

  switch (kind) {
    case PointKind::Single:
    case PointKind::Double:
      return 1 + stamp;
    case PointKind::Normalized:
    case PointKind::Scaled:
      return 3 + stamp;
    case PointKind::Float:
    case PointKind::Counter:
      return 5 + stamp;
    case PointKind::Invalid:
      break;
  }

  return isRecognizedType(typeId) ? 1 : -1;
}

//--------------------------------------------------------------------------------------------------
// Element decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes the value and the quality of one element; the caller guarantees the element's
 *        payload octets exist. Returns false for a type with no value class.
 */
[[nodiscard]] static bool readElement(QByteArrayView view,
                                      qsizetype pos,
                                      IO::Drivers::Iec104Proto::Point& out) noexcept
{
  using namespace IO::Drivers::Iec104Proto;

  switch (out.kind) {
    case PointKind::Single:
      out.value   = QVariant((iec104Octet(view, pos) & 0x01) != 0);
      out.quality = commonQuality(iec104Octet(view, pos));
      return true;
    case PointKind::Double:
      out.value   = QVariant(static_cast<uint>(iec104Octet(view, pos) & 0x03));
      out.quality = commonQuality(iec104Octet(view, pos));
      return true;
    case PointKind::Normalized:
      out.value   = QVariant(static_cast<qint16>(readLe16(view, pos)) / kNormalizedScale);
      out.quality = measurandQuality(iec104Octet(view, pos + 2));
      return true;
    case PointKind::Scaled:
      out.value   = QVariant(static_cast<int>(static_cast<qint16>(readLe16(view, pos))));
      out.quality = measurandQuality(iec104Octet(view, pos + 2));
      return true;
    case PointKind::Counter:
      out.value   = QVariant(static_cast<int>(static_cast<qint32>(readLe32(view, pos))));
      out.quality = counterQuality(iec104Octet(view, pos + 4));
      return true;
    case PointKind::Float:
      break;
    case PointKind::Invalid:
      return false;
  }

  const quint32 bits = readLe32(view, pos);
  float value        = 0.0F;
  std::memcpy(&value, &bits, sizeof(value));
  out.value   = QVariant(static_cast<double>(value));
  out.quality = measurandQuality(iec104Octet(view, pos + 4));
  return true;
}

/**
 * @brief Decodes one whole information element -- value, quality and optional stamp -- into @p out.
 */
[[nodiscard]] static bool decodeObject(QByteArrayView view,
                                       qsizetype pos,
                                       std::uint8_t typeId,
                                       quint32 ioa,
                                       IO::Drivers::Iec104Proto::Point& out) noexcept
{
  using namespace IO::Drivers::Iec104Proto;

  out        = IO::Drivers::Iec104Proto::Point{};
  out.ioa    = ioa;
  out.typeId = typeId;
  out.kind   = kindForType(typeId);
  if (!readElement(view, pos, out))
    return false;

  if (!typeCarriesTime(typeId))
    return true;

  const int stampAt = elementBytes(typeId) - kCp56Bytes;
  SS_ASSERT(stampAt > 0, return false);

  bool valid    = false;
  out.timeMsecs = decodeCp56Time2a(view, pos + stampAt, valid);
  out.timeValid = valid && out.timeMsecs >= 0;
  return true;
}

//--------------------------------------------------------------------------------------------------
// ASDU decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes the six-octet header. A zero type identification is refused: it is the value a
 *        truncated or padded frame decodes to, and it addresses no object list at all.
 */
IO::Drivers::Iec104Proto::DecodeResult IO::Drivers::Iec104Proto::decodeHeader(QByteArrayView asdu,
                                                                              Header& out) noexcept
{
  out = Header{};
  if (asdu.size() < kAsduHeaderBytes)
    return DecodeResult::Truncated;

  const auto vsq   = iec104Octet(asdu, 1);
  const auto cause = iec104Octet(asdu, 2);

  out.typeId        = iec104Octet(asdu, 0);
  out.sequence      = (vsq & 0x80) != 0;
  out.objectCount   = static_cast<std::uint8_t>(vsq & 0x7F);
  out.cause         = static_cast<std::uint8_t>(cause & 0x3F);
  out.negative      = (cause & 0x40) != 0;
  out.test          = (cause & 0x80) != 0;
  out.originator    = iec104Octet(asdu, 3);
  out.commonAddress = readLe16(asdu, 4);

  if (out.typeId == 0)
    return DecodeResult::Malformed;

  return DecodeResult::Ok;
}

/**
 * @brief Decodes an object list whose entries each carry their own address (SQ = 0).
 */
[[nodiscard]] static IO::Drivers::Iec104Proto::DecodeResult decodeIndividual(
  QByteArrayView asdu,
  const IO::Drivers::Iec104Proto::Header& header,
  int width,
  QList<IO::Drivers::Iec104Proto::Point>& points)
{
  using namespace IO::Drivers::Iec104Proto;

  const qsizetype mark = points.size();
  qsizetype pos        = kAsduHeaderBytes;
  for (int i = 0; i < header.objectCount && i < kMaxObjects; ++i) {
    if (pos + kIoaBytes + width > asdu.size()) {
      points.resize(mark);
      return DecodeResult::Truncated;
    }

    IO::Drivers::Iec104Proto::Point point;
    if (!decodeObject(asdu, pos + kIoaBytes, header.typeId, readIoa(asdu, pos), point)) {
      points.resize(mark);
      return DecodeResult::Malformed;
    }

    points.append(point);
    pos += kIoaBytes + width;
  }

  return DecodeResult::Ok;
}

/**
 * @brief Decodes an object list sharing one base address (SQ = 1), where the address of the n-th
 *        element is the base plus n.
 */
[[nodiscard]] static IO::Drivers::Iec104Proto::DecodeResult decodeSequence(
  QByteArrayView asdu,
  const IO::Drivers::Iec104Proto::Header& header,
  int width,
  QList<IO::Drivers::Iec104Proto::Point>& points)
{
  using namespace IO::Drivers::Iec104Proto;

  if (kAsduHeaderBytes + kIoaBytes > asdu.size())
    return DecodeResult::Truncated;

  const quint32 base   = readIoa(asdu, kAsduHeaderBytes);
  const qsizetype mark = points.size();
  qsizetype pos        = kAsduHeaderBytes + kIoaBytes;
  for (int i = 0; i < header.objectCount && i < kMaxObjects; ++i) {
    if (pos + width > asdu.size()) {
      points.resize(mark);
      return DecodeResult::Truncated;
    }

    IO::Drivers::Iec104Proto::Point point;
    if (!decodeObject(asdu, pos, header.typeId, (base + i) & kMaxIoa, point)) {
      points.resize(mark);
      return DecodeResult::Malformed;
    }

    points.append(point);
    pos += width;
  }

  return DecodeResult::Ok;
}

/**
 * @brief Decodes a whole ASDU, appending every information object to @p points. An unknown type
 *        reports Unsupported with the header still filled, so the caller counts the skip instead
 *        of guessing at a payload layout. A refused ASDU appends NOTHING: a half-walked object
 *        list publishes values assembled out of the next object's bytes, which reads as data.
 */
IO::Drivers::Iec104Proto::DecodeResult IO::Drivers::Iec104Proto::decode(QByteArrayView asdu,
                                                                        Header& header,
                                                                        QList<Point>& points)
{
  const auto parsed = decodeHeader(asdu, header);
  if (parsed != DecodeResult::Ok)
    return parsed;

  if (!isRecognizedType(header.typeId))
    return DecodeResult::Unsupported;

  if (!isMonitorType(header.typeId) || header.objectCount == 0)
    return DecodeResult::Ok;

  const int width = elementBytes(header.typeId);
  SS_ASSERT(width > 0, return DecodeResult::Unsupported);

  if (header.sequence)
    return decodeSequence(asdu, header, width, points);

  return decodeIndividual(asdu, header, width, points);
}

//--------------------------------------------------------------------------------------------------
// Time and encoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes a CP56Time2a stamp into milliseconds since the epoch, read as UTC. The station's
 *        clock carries no zone, and the driver only ever uses the stamp through a connect-time
 *        offset, so what has to hold is that the reading is deterministic and monotonic -- not
 *        that it agrees with the reader's own calendar. @p valid reports the IV bit.
 */
qint64 IO::Drivers::Iec104Proto::decodeCp56Time2a(QByteArrayView view,
                                                  qsizetype pos,
                                                  bool& valid) noexcept
{
  valid = false;
  if (pos < 0 || pos + kCp56Bytes > view.size())
    return -1;

  const int millis = readLe16(view, pos);
  const int minute = iec104Octet(view, pos + 2) & 0x3F;
  const int hour   = iec104Octet(view, pos + 3) & 0x1F;
  const int day    = iec104Octet(view, pos + 4) & 0x1F;
  const int month  = iec104Octet(view, pos + 5) & 0x0F;
  const int year   = (iec104Octet(view, pos + 6) & 0x7F) + 2000;

  if (millis > kMaxMillisecond || minute > 59 || hour > 23)
    return -1;

  const QDate date(year, month, day);
  if (!date.isValid())
    return -1;

  const QTime time(hour, minute, millis / 1000, millis % 1000);
  if (!time.isValid())
    return -1;

  valid = (iec104Octet(view, pos + 2) & 0x80) == 0;
  return QDateTime(date, time, QTimeZone::UTC).toMSecsSinceEpoch();
}

/**
 * @brief Builds a C_IC_NA_1 station interrogation: one object at address zero carrying the
 *        qualifier, sent as an activation. This is the only frame the driver ever writes.
 */
QByteArray IO::Drivers::Iec104Proto::encodeInterrogation(quint16 commonAddress, std::uint8_t qoi)
{
  SS_ASSERT(qoi >= kQoiStation, return {});

  QByteArray asdu;
  asdu.reserve(kAsduHeaderBytes + kIoaBytes + 1);
  asdu.append(static_cast<char>(kTypeInterrogation));
  asdu.append(static_cast<char>(0x01));
  asdu.append(static_cast<char>(kCauseActivation));
  asdu.append(static_cast<char>(0));
  asdu.append(static_cast<char>(commonAddress & 0xFF));
  asdu.append(static_cast<char>((commonAddress >> 8) & 0xFF));
  asdu.append(3, static_cast<char>(0));
  asdu.append(static_cast<char>(qoi));

  SS_ASSERT_LOG(asdu.size() == kAsduHeaderBytes + kIoaBytes + 1);
  return asdu;
}
