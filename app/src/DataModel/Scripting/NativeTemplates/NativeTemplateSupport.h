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

#include <bit>
#include <QByteArray>
#include <QHash>
#include <QJsonValue>
#include <QList>
#include <QString>
#include <QStringList>

namespace DataModel::TemplateSupport {

/**
 * @brief Hard cap on the bytes one frame contributes to a decoded row; every byte-oriented
 *        template walks at most this many bytes so a hostile frame cannot pin the pipeline.
 */
inline constexpr int kMaxBytesPerFrame = 65536;

/**
 * @brief Hard cap on the fields one text frame may split into.
 */
inline constexpr int kMaxFields = 10000;

/**
 * @brief Hard cap on the frames one multi-frame packet may expand into.
 */
inline constexpr int kMaxFramesPerPacket = 10000;

[[nodiscard]] QString trNativeTemplate(const char* text);

/**
 * @brief Returns the unsigned byte at index i, or 0 when the index falls outside the frame.
 *        Every multi-byte reader below funnels through here, so this single unsigned compare
 *        is what keeps a truncated or hostile frame from reading past the buffer.
 */
[[nodiscard]] inline quint8 u8At(const QByteArray& data, qsizetype i)
{
  if (static_cast<size_t>(i) >= static_cast<size_t>(data.size())) [[unlikely]]
    return 0;

  return static_cast<quint8>(data.at(i));
}

/**
 * @brief Reads a 16-bit unsigned little-endian value at the offset.
 */
[[nodiscard]] inline quint16 u16Le(const QByteArray& data, qsizetype i)
{
  return static_cast<quint16>(u8At(data, i) | (u8At(data, i + 1) << 8));
}

/**
 * @brief Reads a 16-bit signed little-endian value at the offset.
 */
[[nodiscard]] inline qint16 i16Le(const QByteArray& data, qsizetype i)
{
  return static_cast<qint16>(u16Le(data, i));
}

/**
 * @brief Reads a 32-bit unsigned little-endian value at the offset.
 */
[[nodiscard]] inline quint32 u32Le(const QByteArray& data, qsizetype i)
{
  return static_cast<quint32>(u8At(data, i)) | (static_cast<quint32>(u8At(data, i + 1)) << 8)
       | (static_cast<quint32>(u8At(data, i + 2)) << 16)
       | (static_cast<quint32>(u8At(data, i + 3)) << 24);
}

/**
 * @brief Reads a 32-bit signed little-endian value at the offset.
 */
[[nodiscard]] inline qint32 i32Le(const QByteArray& data, qsizetype i)
{
  return static_cast<qint32>(u32Le(data, i));
}

/**
 * @brief Reads a 16-bit unsigned big-endian value at the offset.
 */
[[nodiscard]] inline quint16 u16Be(const QByteArray& data, qsizetype i)
{
  return static_cast<quint16>((u8At(data, i) << 8) | u8At(data, i + 1));
}

/**
 * @brief Reads a 16-bit signed big-endian value at the offset.
 */
[[nodiscard]] inline qint16 i16Be(const QByteArray& data, qsizetype i)
{
  return static_cast<qint16>(u16Be(data, i));
}

/**
 * @brief Reads a 32-bit unsigned big-endian value at the offset.
 */
[[nodiscard]] inline quint32 u32Be(const QByteArray& data, qsizetype i)
{
  return (static_cast<quint32>(u8At(data, i)) << 24)
       | (static_cast<quint32>(u8At(data, i + 1)) << 16)
       | (static_cast<quint32>(u8At(data, i + 2)) << 8) | static_cast<quint32>(u8At(data, i + 3));
}

/**
 * @brief Reads a 32-bit signed big-endian value at the offset.
 */
[[nodiscard]] inline qint32 i32Be(const QByteArray& data, qsizetype i)
{
  return static_cast<qint32>(u32Be(data, i));
}

/**
 * @brief Reads a 32-bit IEEE-754 float (little-endian) at the offset.
 */
[[nodiscard]] inline float f32Le(const QByteArray& data, qsizetype i)
{
  return std::bit_cast<float>(u32Le(data, i));
}

/**
 * @brief Reads a 32-bit IEEE-754 float (big-endian) at the offset.
 */
[[nodiscard]] inline float f32Be(const QByteArray& data, qsizetype i)
{
  return std::bit_cast<float>(u32Be(data, i));
}

[[nodiscard]] QList<QStringList> byteRowFrame(const QByteArray& bytes);

[[nodiscard]] QString byteGroupValue(
  const QByteArray& bytes, qsizetype offset, int bytesPerValue, bool bigEndian, bool signedValues);

[[nodiscard]] QList<QStringList> groupedByteFrame(const QByteArray& bytes,
                                                  int bytesPerValue,
                                                  bool bigEndian,
                                                  bool signedValues);

[[nodiscard]] QList<QStringList> singleFrame(QStringList&& row);

[[nodiscard]] QHash<QString, int> buildKeyIndex(const QStringList& keys);

[[nodiscard]] QString jsonScalar(const QJsonValue& value);

}  // namespace DataModel::TemplateSupport
