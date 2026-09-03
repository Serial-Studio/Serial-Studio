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

#pragma once

#include <cstdint>
#include <QByteArray>
#include <QByteArrayView>
#include <QList>
#include <QVariant>

namespace IO {
namespace Drivers {
namespace Iec104Proto {

inline constexpr std::uint8_t kTypeSinglePoint          = 1;
inline constexpr std::uint8_t kTypeDoublePoint          = 3;
inline constexpr std::uint8_t kTypeMeasuredNormalized   = 9;
inline constexpr std::uint8_t kTypeMeasuredScaled       = 11;
inline constexpr std::uint8_t kTypeMeasuredFloat        = 13;
inline constexpr std::uint8_t kTypeIntegratedTotals     = 15;
inline constexpr std::uint8_t kTypeSinglePointTime      = 30;
inline constexpr std::uint8_t kTypeDoublePointTime      = 31;
inline constexpr std::uint8_t kTypeMeasuredNormalTime   = 34;
inline constexpr std::uint8_t kTypeMeasuredScaledTime   = 35;
inline constexpr std::uint8_t kTypeMeasuredFloatTime    = 36;
inline constexpr std::uint8_t kTypeIntegratedTotalsTime = 37;
inline constexpr std::uint8_t kTypeEndOfInitialization  = 70;
inline constexpr std::uint8_t kTypeInterrogation        = 100;

inline constexpr std::uint8_t kCauseSpontaneous  = 3;
inline constexpr std::uint8_t kCauseInitialized  = 4;
inline constexpr std::uint8_t kCauseActivation   = 6;
inline constexpr std::uint8_t kCauseActConfirm   = 7;
inline constexpr std::uint8_t kCauseActTerminate = 10;
inline constexpr std::uint8_t kCauseInterrogated = 20;

inline constexpr std::uint8_t kQoiStation = 20;

inline constexpr int kAsduHeaderBytes = 6;
inline constexpr int kIoaBytes        = 3;
inline constexpr int kCp56Bytes       = 7;
inline constexpr int kMaxObjects      = 127;
inline constexpr quint32 kMaxIoa      = 0xFFFFFF;

/**
 * @brief Normalized per-point quality, decoded out of the SIQ, DIQ, QDS and BCR descriptors so
 *        one dashboard rule covers every ASDU type (R29). The wire layouts differ; these do not.
 */
enum Quality : std::uint8_t {
  QualityGood        = 0x00,
  QualityOverflow    = 0x01,
  QualityBlocked     = 0x02,
  QualitySubstituted = 0x04,
  QualityNotTopical  = 0x08,
  QualityInvalid     = 0x10,
};

/**
 * @brief The value class a type identification carries, which is what maps a point onto a wire
 *        type and onto the dashboard widget its project generates.
 */
enum class PointKind : std::uint8_t {
  Single     = 0,
  Double     = 1,
  Normalized = 2,
  Scaled     = 3,
  Float      = 4,
  Counter    = 5,
  Invalid    = 255,
};

/**
 * @brief One decoded information object: where it lives, what it is worth, how much that is worth
 *        trusting, and when the station says it happened.
 */
struct Point {
  quint32 ioa          = 0;
  std::uint8_t typeId  = 0;
  std::uint8_t quality = QualityGood;
  PointKind kind       = PointKind::Invalid;
  QVariant value;
  qint64 timeMsecs = -1;
  bool timeValid   = false;
};

/**
 * @brief The six-octet ASDU header: what the objects are, how many there are, whether they share
 *        one base address, why they were sent and which station sent them.
 */
struct Header {
  std::uint8_t typeId      = 0;
  std::uint8_t objectCount = 0;
  std::uint8_t cause       = 0;
  std::uint8_t originator  = 0;
  quint16 commonAddress    = 0;
  bool sequence            = false;
  bool negative            = false;
  bool test                = false;
};

/**
 * @brief Outcome of one ASDU decode. @c Unsupported is a type this build does not decode and is
 *        counted rather than guessed; @c Malformed is a payload that contradicts its own header.
 */
enum class DecodeResult : std::uint8_t {
  Ok          = 0,
  Truncated   = 1,
  Unsupported = 2,
  Malformed   = 3,
};

/**
 * @brief Identity of one monitored object. IEC 60870-5-104 qualifies an information-object address
 *        by the type identifier that carries it, so the SAME address legitimately names a
 *        single-point input and a measurand; a slot table keyed on the address alone latches the
 *        second into the first one's slot and encodes it with the first one's wire type.
 */
[[nodiscard]] inline constexpr quint64 slotKey(quint32 ioa, std::uint8_t typeId) noexcept
{
  return (static_cast<quint64>(typeId) << 32) | static_cast<quint64>(ioa);
}

[[nodiscard]] bool isMonitorType(std::uint8_t typeId) noexcept;
[[nodiscard]] bool isRecognizedType(std::uint8_t typeId) noexcept;
[[nodiscard]] bool typeCarriesTime(std::uint8_t typeId) noexcept;
[[nodiscard]] PointKind kindForType(std::uint8_t typeId) noexcept;
[[nodiscard]] int elementBytes(std::uint8_t typeId) noexcept;

[[nodiscard]] DecodeResult decodeHeader(QByteArrayView asdu, Header& out) noexcept;
[[nodiscard]] DecodeResult decode(QByteArrayView asdu, Header& header, QList<Point>& points);

[[nodiscard]] qint64 decodeCp56Time2a(QByteArrayView view, qsizetype pos, bool& valid) noexcept;
[[nodiscard]] QByteArray encodeInterrogation(quint16 commonAddress, std::uint8_t qoi);

}  // namespace Iec104Proto
}  // namespace Drivers
}  // namespace IO
