/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <bit>
#include <cstring>
#include <QByteArray>
#include <QString>
#include <QtEndian>
#include <span>
#include <vector>

namespace Sessions {

/**
 * @brief Wire format for a stream block's samples (spec 0054): IEEE-754 float64, explicitly
 *        little-endian, `frames * 8` bytes. Session databases move between machines, so host
 *        order would let a foreign file misdecode silently instead of being byte-identical
 *        everywhere. Encoder and decoder live together so the two halves cannot drift apart.
 */
inline constexpr qsizetype kStreamSampleBytes = 8;

/**
 * @brief Packs @p samples into the canonical little-endian float64 blob.
 */
[[nodiscard]] inline QByteArray packStreamSamples(std::span<const double> samples)
{
  QByteArray blob;
  blob.resize(static_cast<qsizetype>(samples.size()) * kStreamSampleBytes);

  char* out = blob.data();
  for (std::size_t i = 0; i < samples.size(); ++i) {
    const quint64 bits = qToLittleEndian(std::bit_cast<quint64>(samples[i]));
    std::memcpy(out + i * sizeof(quint64), &bits, sizeof(quint64));
  }

  return blob;
}

/**
 * @brief Decodes @p blob into @p out, returning false when its length is not `frames * 8`. A
 *        truncated or foreign blob must fail loudly rather than be decoded past its end.
 */
[[nodiscard]] inline bool unpackStreamSamples(const QByteArray& blob,
                                              qint64 frames,
                                              std::vector<double>& out)
{
  if (frames < 0)
    return false;

  if (blob.size() % kStreamSampleBytes != 0
      || blob.size() / kStreamSampleBytes != static_cast<qsizetype>(frames))
    return false;

  out.resize(static_cast<std::size_t>(frames));
  for (std::size_t i = 0; i < out.size(); ++i) {
    quint64 bits = 0;
    std::memcpy(&bits, blob.constData() + i * sizeof(quint64), sizeof(quint64));
    out[i] = std::bit_cast<double>(qFromLittleEndian(bits));
  }

  return true;
}

/**
 * @brief Packs @p times into the canonical little-endian int64 blob. Only an irregular block needs
 *        one: a uniform grid derives its offsets from dt and stores nothing.
 */
[[nodiscard]] inline QByteArray packStreamTimes(std::span<const qint64> times)
{
  QByteArray blob;
  blob.resize(static_cast<qsizetype>(times.size()) * kStreamSampleBytes);

  char* out = blob.data();
  for (std::size_t i = 0; i < times.size(); ++i) {
    const quint64 bits = qToLittleEndian(static_cast<quint64>(times[i]));
    std::memcpy(out + i * sizeof(quint64), &bits, sizeof(quint64));
  }

  return blob;
}

/**
 * @brief Decodes an explicit-times blob, refusing any length that is not `frames * 8`.
 */
[[nodiscard]] inline bool unpackStreamTimes(const QByteArray& blob,
                                            qint64 frames,
                                            std::vector<qint64>& out)
{
  if (frames < 0)
    return false;

  if (blob.size() % kStreamSampleBytes != 0
      || blob.size() / kStreamSampleBytes != static_cast<qsizetype>(frames))
    return false;

  out.resize(static_cast<std::size_t>(frames));
  for (std::size_t i = 0; i < out.size(); ++i) {
    quint64 bits = 0;
    std::memcpy(&bits, blob.constData() + i * sizeof(quint64), sizeof(quint64));
    out[i] = static_cast<qint64>(qFromLittleEndian(bits));
  }

  return true;
}

/**
 * @brief Packs @p strings as length-prefixed UTF-8: a little-endian uint32 byte count per entry,
 *        then the bytes. Length-prefixed rather than delimited because a recorded value may
 *        legitimately contain any byte, separators included.
 */
[[nodiscard]] inline QByteArray packStreamText(std::span<const QString> strings)
{
  QByteArray blob;
  for (const auto& value : strings) {
    const QByteArray utf8 = value.toUtf8();
    quint32 length        = qToLittleEndian(static_cast<quint32>(utf8.size()));
    blob.append(reinterpret_cast<const char*>(&length), sizeof(length));
    blob.append(utf8);
  }

  return blob;
}

/**
 * @brief Decodes a length-prefixed UTF-8 blob into exactly @p frames entries, refusing a payload
 *        that runs short, runs long, or declares a length past its own end.
 */
[[nodiscard]] inline bool unpackStreamText(const QByteArray& blob,
                                           qint64 frames,
                                           std::vector<QString>& out)
{
  if (frames < 0)
    return false;

  if (frames > blob.size() / static_cast<qsizetype>(sizeof(quint32)))
    return false;

  out.clear();
  out.reserve(static_cast<std::size_t>(frames));

  qsizetype offset = 0;
  for (qint64 i = 0; i < frames; ++i) {
    if (offset + static_cast<qsizetype>(sizeof(quint32)) > blob.size())
      return false;

    quint32 length = 0;
    std::memcpy(&length, blob.constData() + offset, sizeof(length));
    length  = qFromLittleEndian(length);
    offset += static_cast<qsizetype>(sizeof(quint32));

    if (offset + static_cast<qsizetype>(length) > blob.size())
      return false;

    out.push_back(QString::fromUtf8(blob.constData() + offset, static_cast<qsizetype>(length)));
    offset += static_cast<qsizetype>(length);
  }

  return offset == blob.size();
}

}  // namespace Sessions
