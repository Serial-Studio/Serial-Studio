/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include <cstring>
#include <QByteArray>
#include <QtEndian>
#include <QVector>

#include "ThirdParty/miniaudio.h"

namespace IO {
namespace Drivers {

/**
 * @brief The audio driver's PCM sample codec: one interleaved sample in or out of miniaudio's
 *        native formats. Stateless and device-free, so both publish lanes (the CSV text and the
 *        typed SampleBlock) decode through the same two functions and can never disagree about a
 *        magnitude.
 *
 *        decodeSample() and normalizationTerms() run once per sample on the capture path and are
 *        therefore defined here rather than in a translation unit of their own: the driver's
 *        render loops must keep inlining them. The two CSV packers run once per written frame on
 *        the playback path and live in the implementation file.
 */
namespace AudioPcm {

// Reciprocal full-scale factors, as exact powers of two: normalizing is one multiply, not a divide
inline constexpr float kInvFullScaleU8  = 0x1p-7f;
inline constexpr float kInvFullScaleS16 = 0x1p-15f;
inline constexpr float kInvFullScaleS24 = 0x1p-23f;
inline constexpr float kInvFullScaleS32 = 0x1p-31f;

[[nodiscard]] bool packCsvSample(ma_format format, const QByteArray& token, QVector<quint8>& out);

[[nodiscard]] bool packNormalizedSample(ma_format format,
                                        const QByteArray& token,
                                        QVector<quint8>& out);

/**
 * @brief Yields the affine terms mapping a raw PCM magnitude to the normalized -1..1 range, as
 * normalized = (raw + offset) * scale. Float32 is already normalized and a disabled normalization
 * yields the identity, so every decode path can apply the terms unconditionally.
 */
inline void normalizationTerms(ma_format format, bool enabled, float& offset, float& scale)
{
  offset = 0.0f;
  scale  = 1.0f;
  if (!enabled)
    return;

  switch (format) {
    case ma_format_u8:
      offset = -128.0f;
      scale  = kInvFullScaleU8;
      break;
    case ma_format_s16:
      scale = kInvFullScaleS16;
      break;
    case ma_format_s24:
      scale = kInvFullScaleS24;
      break;
    case ma_format_s32:
      scale = kInvFullScaleS32;
      break;
    default:
      break;
  }
}

/**
 * @brief Decodes one interleaved PCM sample of the given format into its raw float magnitude;
 * an unknown format cannot reach here because a zero bytes-per-sample rejects the buffer first.
 */
[[nodiscard]] inline float decodeSample(ma_format format, const char* ptr)
{
  switch (format) {
    case ma_format_u8:
      return static_cast<float>(static_cast<quint8>(*ptr));
    case ma_format_s16:
      return static_cast<float>(qFromLittleEndian<qint16>(reinterpret_cast<const quint8*>(ptr)));
    case ma_format_s24: {
      const quint8* b  = reinterpret_cast<const quint8*>(ptr);
      const qint32 s24 = static_cast<qint32>(b[0]) | (static_cast<qint32>(b[1]) << 8)
                       | (static_cast<qint32>(b[2]) << 16);
      return static_cast<float>((s24 & 0x800000) ? (s24 | static_cast<qint32>(0xFF000000)) : s24);
    }
    case ma_format_s32:
      return static_cast<float>(qFromLittleEndian<qint32>(reinterpret_cast<const quint8*>(ptr)));
    case ma_format_f32: {
      float sample;
      std::memcpy(&sample, ptr, sizeof(float));
      return sample;
    }
    default:
      return 0.0f;
  }
}

}  // namespace AudioPcm
}  // namespace Drivers
}  // namespace IO
