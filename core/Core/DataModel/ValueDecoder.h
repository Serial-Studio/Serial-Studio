/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#pragma once

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <QByteArrayView>
#include <QString>

#if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
  && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 130300
#  define SS_APPLE_NO_FLOAT_TO_CHARS 1
#  include <xlocale.h>
#endif

#include "Core/DataModel/Frame.h"
#include "Core/HotpathOptimization.h"
#include "Core/SSAssert.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Value decoders (spec 0108)
//--------------------------------------------------------------------------------------------------

/**
 * @brief The dataset value decoders, as persisted in the dataset's @c format field.
 */
enum ValueFormatKind : int {
  ValueFormatNone       = 0,
  ValueFormatFixedPoint = 1,
  ValueFormatBitField   = 2,
  ValueFormatScale      = 3,
};

/**
 * @brief Which value a decode call in a dataset's value tail is for: Parsed (the call before the
 *        transform), Computed (the call after it, for a computed dataset), or Skip (a replayed
 *        final value, never decoded again).
 */
enum class DecodeStage : quint8 {
  Parsed,
  Computed,
  Skip,
};

/**
 * @brief A dataset's decoder declaration, detached from the dataset so the stream lane's channel
 *        configuration can carry it without the rest of the dataset.
 */
struct ValueDecoderSpec {
  int format       = ValueFormatNone;
  int width        = 16;
  int fractionBits = 0;
  int bitOffset    = 0;
  int bitWidth     = 1;
  bool isSigned    = false;
  double scale     = 1;
  double offset    = 0;
};

/**
 * @brief Whether @p dataset declares a value decoder: any format other than None.
 */
[[nodiscard]] SS_FORCE_INLINE bool has_decoder(const Dataset& dataset) noexcept
{
  return dataset.format != ValueFormatNone;
}

/**
 * @brief The decoder declaration of @p dataset.
 */
[[nodiscard]] SS_FORCE_INLINE ValueDecoderSpec decoder_spec(const Dataset& dataset) noexcept
{
  ValueDecoderSpec spec;
  spec.format       = dataset.format;
  spec.width        = dataset.formatWidth;
  spec.fractionBits = dataset.formatFractionBits;
  spec.bitOffset    = dataset.formatBitOffset;
  spec.bitWidth     = dataset.formatBitWidth;
  spec.isSigned     = dataset.formatSigned;
  spec.scale        = dataset.formatScale;
  spec.offset       = dataset.formatOffset;
  return spec;
}

namespace detail {

inline constexpr double kTwoPow53 = 9007199254740992.0;
inline constexpr double kTwoPow63 = 9223372036854775808.0;
inline constexpr double kTwoPow64 = 18446744073709551616.0;

/**
 * @brief Rounds @p raw to the integer whose bit pattern the decoders read. Returns false for a
 *        value with no 64-bit pattern (NaN, infinity, out of range), which is then left as is.
 */
[[nodiscard]] SS_FORCE_INLINE bool raw_bits(double raw, quint64& bits) noexcept
{
  if (!(raw >= -kTwoPow63 && raw < kTwoPow64))
    return false;

  bits = raw >= kTwoPow63 ? static_cast<quint64>(raw) : static_cast<quint64>(std::llround(raw));
  return true;
}

/**
 * @brief The low @p width bits of @p bits; a width of 64 keeps every bit.
 */
[[nodiscard]] SS_FORCE_INLINE quint64 low_bits(quint64 bits, int width) noexcept
{
  SS_ASSERT_HOTPATH(width >= 1);
  SS_ASSERT_HOTPATH(width <= 64);

  return width >= 64 ? bits : (bits & ((quint64{1} << width) - 1));
}

/**
 * @brief Sign-extends the low @p width bits of @p bits as a two's complement integer.
 */
[[nodiscard]] SS_FORCE_INLINE qint64 sign_extend(quint64 bits, int width) noexcept
{
  SS_ASSERT_HOTPATH(width >= 1);
  SS_ASSERT_HOTPATH(width <= 64);

  if (width >= 64)
    return static_cast<qint64>(bits);

  const quint64 sign = quint64{1} << (width - 1);
  return static_cast<qint64>((bits ^ sign) - sign);
}

}  // namespace detail

/**
 * @brief Decodes @p raw through @p spec: fixed point and bit field read the rounded integer's low
 *        @c width bits (fixed point scaled by 2^-fractionBits, signed when declared; a bit field
 *        never sign-extended), scale is @c raw*scale+offset. Declarations the editor refuses are
 *        clamped, so a hand-edited project never shifts past 63 bits.
 */
[[nodiscard]] SS_FORCE_INLINE double decode_value(const ValueDecoderSpec& spec, double raw) noexcept
{
  if (spec.format == ValueFormatScale)
    return raw * spec.scale + spec.offset;

  if (spec.format != ValueFormatFixedPoint && spec.format != ValueFormatBitField)
    return raw;

  quint64 bits = 0;
  if (!detail::raw_bits(raw, bits))
    return raw;

  const int width = std::clamp(spec.width, 1, 64);
  bits            = detail::low_bits(bits, width);
  if (spec.format == ValueFormatBitField) {
    const int offset = std::clamp(spec.bitOffset, 0, width - 1);
    const int field  = std::clamp(spec.bitWidth, 1, width - offset);
    return static_cast<double>(detail::low_bits(bits >> offset, field));
  }

  const double unit = std::ldexp(1.0, -std::clamp(spec.fractionBits, 0, 63));
  if (spec.isSigned)
    return static_cast<double>(detail::sign_extend(bits, width)) * unit;

  return static_cast<double>(bits) * unit;
}

/**
 * @brief Decodes @p raw through @p dataset's declared format.
 */
[[nodiscard]] SS_FORCE_INLINE double decode_value(const Dataset& dataset, double raw) noexcept
{
  return decode_value(decoder_spec(dataset), raw);
}

/**
 * @brief Renders @p value as the shortest decimal that reads back bit-identically, straight into
 *        @p dst's buffer. An exact integer prints as one on every platform, since libstdc++'s
 *        general form writes 4000000001 as 4.000000001e+09. Apple ships float std::to_chars only
 *        from macOS 13.3, so older targets print 15 significant digits through the C locale.
 */
SS_FORCE_INLINE void render_number_in_place(QString& dst, double value)
{
  char buf[32];
  const double magnitude = std::fabs(value);
  if (magnitude >= 1.0 && magnitude < detail::kTwoPow53 && value == std::trunc(value)) {
    const auto res = std::to_chars(buf, buf + sizeof(buf), static_cast<long long>(value));
    SS_ASSERT_HOTPATH(res.ec == std::errc());
    SS_ASSERT_HOTPATH(res.ptr > buf);
    assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(res.ptr - buf)));
    return;
  }

#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  const int len = snprintf_l(buf, sizeof(buf), nullptr, "%.15g", value);
  SS_ASSERT_HOTPATH(len > 0);
  SS_ASSERT_HOTPATH(static_cast<std::size_t>(len) < sizeof(buf));
  assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(len)));
#else
  const auto res = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general);
  SS_ASSERT_HOTPATH(res.ec == std::errc());
  SS_ASSERT_HOTPATH(res.ptr > buf);
  assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(res.ptr - buf)));
#endif
}

/**
 * @brief Replaces a numeric dataset's value with its decoded value, number and text together, in
 *        place, when it declares a format and @p stage is its stage: a parsed value decodes before
 *        its transform, a computed one after it, a replayed final value never. Text is left as is.
 */
SS_FORCE_INLINE void decode_dataset_in_place(Dataset& dataset, DecodeStage stage)
{
  if (!has_decoder(dataset) || stage == DecodeStage::Skip) [[likely]]
    return;

  if (dataset.virtual_ != (stage == DecodeStage::Computed))
    return;

  if (!dataset.isNumeric) [[unlikely]]
    return;

  dataset.numericValue = decode_value(dataset, dataset.numericValue);
  render_number_in_place(dataset.value, dataset.numericValue);
}

}  // namespace DataModel
