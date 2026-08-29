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

#include "IO/Drivers/Audio/AudioPcm.h"

#include <QDebug>

/**
 * @brief Packs a single CSV sample into native bytes for the given ma_format.
 */
bool IO::Drivers::AudioPcm::packCsvSample(ma_format format,
                                          const QByteArray& token,
                                          QVector<quint8>& out)
{
  bool ok = false;

  if (format == ma_format_u8) {
    int value = token.toInt(&ok);
    if (!ok) {
      qWarning() << "Invalid Unsigned 8-bit number:" << token;
      return false;
    }

    out.append(static_cast<quint8>(qBound(0, value, 255)));
    return true;
  }

  if (format == ma_format_s16) {
    int value = token.toInt(&ok);
    if (!ok) {
      qWarning() << "Invalid Signed 16-bit number:" << token;
      return false;
    }

    const qint16 sample = static_cast<qint16>(qBound(-32768, value, 32767));
    const quint8* bytes = reinterpret_cast<const quint8*>(&sample);
    out.append(bytes[0]);
    out.append(bytes[1]);
    return true;
  }

  if (format == ma_format_s24) {
    int value = token.toInt(&ok);
    if (!ok) {
      qWarning() << "Invalid Signed 24-bit number:" << token;
      return false;
    }

    value = qBound(-8388608, value, 8388607);
    out.append(static_cast<quint8>(value & 0xFF));
    out.append(static_cast<quint8>((value >> 8) & 0xFF));
    out.append(static_cast<quint8>((value >> 16) & 0xFF));
    return true;
  }

  if (format == ma_format_s32) {
    qint32 value = token.toInt(&ok);
    if (!ok) {
      qWarning() << "Invalid Signed 32-bit number:" << token;
      return false;
    }

    value               = qBound(-2147483647, value, 2147483647);
    const quint8* bytes = reinterpret_cast<const quint8*>(&value);
    out.append(bytes[0]);
    out.append(bytes[1]);
    out.append(bytes[2]);
    out.append(bytes[3]);
    return true;
  }

  if (format == ma_format_f32) {
    float value = token.toFloat(&ok);
    if (!ok) {
      qWarning() << "Invalid 32-bit Float number:" << token;
      return false;
    }

    value               = qBound(-1.0f, value, 1.0f);
    const quint8* bytes = reinterpret_cast<const quint8*>(&value);
    for (size_t b = 0; b < sizeof(float); ++b)
      out.append(bytes[b]);

    return true;
  }

  qWarning() << "Unsupported format:" << static_cast<int>(format);
  return false;
}

/**
 * @brief Packs a single normalized -1..1 CSV sample into native bytes for the given ma_format;
 * the integer encodings use the positive full scale so the round trip stays symmetric.
 */
bool IO::Drivers::AudioPcm::packNormalizedSample(ma_format format,
                                                 const QByteArray& token,
                                                 QVector<quint8>& out)
{
  bool ok      = false;
  float sample = token.toFloat(&ok);
  if (!ok) {
    qWarning() << "Invalid normalized sample:" << token;
    return false;
  }

  sample = qBound(-1.0f, sample, 1.0f);

  if (format == ma_format_f32) {
    const quint8* bytes = reinterpret_cast<const quint8*>(&sample);
    for (size_t b = 0; b < sizeof(float); ++b)
      out.append(bytes[b]);

    return true;
  }

  if (format == ma_format_u8) {
    out.append(static_cast<quint8>(qBound(0, qRound(sample * 127.0f) + 128, 255)));
    return true;
  }

  if (format == ma_format_s16) {
    const qint16 value  = static_cast<qint16>(qRound(sample * 32767.0f));
    const quint8* bytes = reinterpret_cast<const quint8*>(&value);
    out.append(bytes[0]);
    out.append(bytes[1]);
    return true;
  }

  if (format == ma_format_s24) {
    const qint32 value = qRound(sample * 8388607.0f);
    out.append(static_cast<quint8>(value & 0xFF));
    out.append(static_cast<quint8>((value >> 8) & 0xFF));
    out.append(static_cast<quint8>((value >> 16) & 0xFF));
    return true;
  }

  if (format == ma_format_s32) {
    const auto value    = static_cast<qint32>(qRound(static_cast<double>(sample) * 2147483647.0));
    const quint8* bytes = reinterpret_cast<const quint8*>(&value);
    out.append(bytes[0]);
    out.append(bytes[1]);
    out.append(bytes[2]);
    out.append(bytes[3]);
    return true;
  }

  qWarning() << "Unsupported format:" << static_cast<int>(format);
  return false;
}
