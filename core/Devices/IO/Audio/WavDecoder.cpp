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

#include "IO/Audio/WavDecoder.h"

#include <algorithm>
#include <cstring>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QtEndian>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Tunables
//--------------------------------------------------------------------------------------------------

static constexpr int kFormatPcm         = 1;
static constexpr int kFormatFloat       = 3;
static constexpr int kFormatExtensible  = 0xFFFE;
static constexpr qsizetype kRiffHeader  = 12;
static constexpr qsizetype kChunkHeader = 8;
static constexpr qsizetype kMaxFileSize = static_cast<qsizetype>(IO::Audio::WavDecoder::kMaxRate)
                                          * IO::Audio::WavDecoder::kMaxChannels * 4
                                          * IO::Audio::WavDecoder::kMaxSeconds
                                        + 4096;

static constexpr float kInvFullScaleU8  = 0x1p-7f;
static constexpr float kInvFullScaleS16 = 0x1p-15f;
static constexpr float kInvFullScaleS24 = 0x1p-23f;
static constexpr float kInvFullScaleS32 = 0x1p-31f;

//--------------------------------------------------------------------------------------------------
// Public entry points
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates a file without keeping the samples; the reason is what the picker shows.
 */
bool IO::Audio::WavDecoder::probe(const QString& path, QString& reason)
{
  DecodedSound scratch;
  return decode(path, scratch, reason);
}

/**
 * @brief Reads and decodes a file from disk or from a Qt resource path.
 */
bool IO::Audio::WavDecoder::decode(const QString& path, DecodedSound& out, QString& reason)
{
  QByteArray bytes;
  if (!readFile(path, bytes, reason))
    return false;

  return decodeBytes(bytes, out, reason);
}

/**
 * @brief Walks the RIFF chunk list, requiring one fmt chunk before the data chunk; every other
 *        chunk (LIST, cue, fact, bext) is skipped by its declared size.
 */
bool IO::Audio::WavDecoder::decodeBytes(const QByteArray& bytes, DecodedSound& out, QString& reason)
{
  if (bytes.size() < kRiffHeader || bytes.left(4) != "RIFF" || bytes.mid(8, 4) != "WAVE") {
    reason = QObject::tr("Not a RIFF/WAVE file");
    return false;
  }

  Format fmt{0, 0, 0, 0};
  bool haveFormat           = false;
  qsizetype offset          = kRiffHeader;
  const qsizetype maxChunks = 64;
  for (qsizetype i = 0; i < maxChunks && offset + kChunkHeader <= bytes.size(); ++i) {
    const QByteArray id = bytes.mid(offset, 4);
    const auto size =
      static_cast<qsizetype>(qFromLittleEndian<quint32>(bytes.constData() + offset + 4));
    const qsizetype body = offset + kChunkHeader;
    if (size < 0 || body + size > bytes.size()) {
      reason = QObject::tr("Truncated WAV chunk '%1'").arg(QString::fromLatin1(id));
      return false;
    }

    if (id == "data")
      return haveFormat ? convertSamples(fmt, bytes.mid(body, size), out, reason)
                        : failDataBeforeFormat(reason);

    if (id == "fmt " && !readFormat(bytes.mid(body, size), fmt, reason))
      return false;

    haveFormat = haveFormat || id == "fmt ";
    offset     = body + size + (size & 1);
  }

  reason = QObject::tr("WAV file has no data chunk");
  return false;
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the whole file; the size cap is the largest accepted stream plus chunk overhead.
 */
bool IO::Audio::WavDecoder::readFile(const QString& path, QByteArray& bytes, QString& reason)
{
  if (path.isEmpty()) {
    reason = QObject::tr("No file selected");
    return false;
  }

  const QFileInfo info(path);
  if (!info.exists()) {
    reason = QObject::tr("File not found: %1").arg(path);
    return false;
  }

  if (!info.isFile()) {
    reason = QObject::tr("Not a regular file: %1").arg(path);
    return false;
  }

  QFile file(path);
  if (file.size() > kMaxFileSize) {
    reason = QObject::tr("File is larger than a %1 s sound can be").arg(kMaxSeconds);
    return false;
  }

  if (!file.open(QIODevice::ReadOnly)) {
    reason = QObject::tr("Cannot open %1").arg(path);
    return false;
  }

  bytes = file.read(kMaxFileSize + 1);
  if (bytes.size() > kMaxFileSize) {
    reason = QObject::tr("File is larger than a %1 s sound can be").arg(kMaxSeconds);
    return false;
  }

  return true;
}

/**
 * @brief Parses the fmt chunk; WAVE_FORMAT_EXTENSIBLE resolves to its sub-format tag.
 */
bool IO::Audio::WavDecoder::readFormat(const QByteArray& chunk, Format& fmt, QString& reason)
{
  if (chunk.size() < 16) {
    reason = QObject::tr("WAV format chunk is too short");
    return false;
  }

  const char* p     = chunk.constData();
  fmt.tag           = qFromLittleEndian<quint16>(p);
  fmt.channels      = qFromLittleEndian<quint16>(p + 2);
  fmt.sampleRate    = static_cast<int>(qFromLittleEndian<quint32>(p + 4));
  fmt.bitsPerSample = qFromLittleEndian<quint16>(p + 14);

  if (fmt.tag == kFormatExtensible) {
    if (chunk.size() < 26) {
      reason = QObject::tr("WAV extensible format chunk is too short");
      return false;
    }

    fmt.tag = qFromLittleEndian<quint16>(p + 24);
  }

  const bool pcm      = fmt.tag == kFormatPcm
                     && (fmt.bitsPerSample == 8 || fmt.bitsPerSample == 16 || fmt.bitsPerSample == 24
                         || fmt.bitsPerSample == 32);
  const bool floating = fmt.tag == kFormatFloat && fmt.bitsPerSample == 32;
  if (!pcm && !floating) {
    reason = QObject::tr("Unsupported WAV encoding (only PCM integer and 32-bit float)");
    return false;
  }

  if (fmt.channels < 1 || fmt.channels > kMaxChannels) {
    reason = QObject::tr("WAV has %1 channels; mono or stereo only").arg(fmt.channels);
    return false;
  }

  if (fmt.sampleRate < kMinRate || fmt.sampleRate > kMaxRate) {
    reason = QObject::tr("WAV sample rate %1 Hz is outside 8 to 96 kHz").arg(fmt.sampleRate);
    return false;
  }

  return true;
}

/**
 * @brief Reports the one chunk-order error the walker cannot express inline.
 */
bool IO::Audio::WavDecoder::failDataBeforeFormat(QString& reason)
{
  reason = QObject::tr("WAV data chunk precedes the format chunk");
  return false;
}

/**
 * @brief Decodes one sample through an aligned copy, so odd chunk offsets never become
 *        unaligned loads; integer widths normalize by their exact full-scale power of two.
 */
float IO::Audio::WavDecoder::sampleAt(const Format& fmt, const char* src) noexcept
{
  SS_ASSERT(src != nullptr, return 0.0f);

  if (fmt.tag == kFormatFloat) {
    float value       = 0.0f;
    const quint32 raw = qFromLittleEndian<quint32>(src);
    std::memcpy(&value, &raw, sizeof(value));
    return value;
  }

  switch (fmt.bitsPerSample) {
    case 8:
      return (static_cast<float>(static_cast<quint8>(*src)) - 128.0f) * kInvFullScaleU8;
    case 16:
      return static_cast<float>(qFromLittleEndian<qint16>(src)) * kInvFullScaleS16;
    case 24:
      return static_cast<float>(static_cast<qint32>(static_cast<quint8>(src[0]))
                                | (static_cast<qint32>(static_cast<quint8>(src[1])) << 8)
                                | (static_cast<qint32>(static_cast<qint8>(src[2])) << 16))
           * kInvFullScaleS24;
    default:
      return static_cast<float>(qFromLittleEndian<qint32>(src)) * kInvFullScaleS32;
  }
}

/**
 * @brief Converts the data chunk to normalized, clamped, interleaved floats.
 */
bool IO::Audio::WavDecoder::convertSamples(const Format& fmt,
                                           const QByteArray& data,
                                           DecodedSound& out,
                                           QString& reason)
{
  SS_ASSERT(fmt.channels >= 1, return false);
  SS_ASSERT(fmt.bitsPerSample >= 8, return false);

  const int bytesPerSample  = fmt.bitsPerSample / 8;
  const qsizetype frameSize = static_cast<qsizetype>(bytesPerSample) * fmt.channels;
  const qsizetype frames    = data.size() / frameSize;
  if (frames <= 0) {
    reason = QObject::tr("WAV data chunk is empty");
    return false;
  }

  if (frames > static_cast<qsizetype>(fmt.sampleRate) * kMaxSeconds) {
    reason = QObject::tr("Sound is longer than %1 s").arg(kMaxSeconds);
    return false;
  }

  out.sampleRate = fmt.sampleRate;
  out.channels   = fmt.channels;
  out.samples.assign(static_cast<std::size_t>(frames * fmt.channels), 0.0f);

  const char* src       = data.constData();
  const qsizetype count = frames * fmt.channels;
  for (qsizetype i = 0; i < count; ++i, src += bytesPerSample)
    out.samples[static_cast<std::size_t>(i)] = std::clamp(sampleAt(fmt, src), -1.0f, 1.0f);

  return true;
}
