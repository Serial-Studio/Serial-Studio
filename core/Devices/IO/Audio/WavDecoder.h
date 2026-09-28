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

#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <vector>

namespace IO {
namespace Audio {

/**
 * @brief One decoded sound: interleaved float samples at the file's own rate and channel count.
 */
struct DecodedSound {
  int sampleRate;
  int channels;
  std::vector<float> samples;

  [[nodiscard]] qint64 frames() const noexcept
  {
    return channels > 0 ? static_cast<qint64>(samples.size()) / channels : 0;
  }
};

/**
 * @brief First-party RIFF/WAVE reader for the aural alert system (spec 0087): PCM 8/16/24/32-bit
 *        integer and 32-bit float, one or two channels, 8 to 96 kHz, at most ten seconds. Every
 *        refusal names its reason so a preference page can show it; nothing else is accepted,
 *        because the vendored miniaudio is built without its decoders on purpose.
 */
class WavDecoder {
public:
  static constexpr int kMaxSeconds  = 10;
  static constexpr int kMinRate     = 8000;
  static constexpr int kMaxRate     = 96000;
  static constexpr int kMaxChannels = 2;

  [[nodiscard]] static bool probe(const QString& path, QString& reason);
  [[nodiscard]] static bool decode(const QString& path, DecodedSound& out, QString& reason);
  [[nodiscard]] static bool decodeBytes(const QByteArray& bytes,
                                        DecodedSound& out,
                                        QString& reason);

private:
  struct Format {
    int tag;
    int channels;
    int sampleRate;
    int bitsPerSample;
  };

  [[nodiscard]] static bool failDataBeforeFormat(QString& reason);
  [[nodiscard]] static float sampleAt(const Format& fmt, const char* src) noexcept;
  [[nodiscard]] static bool readFile(const QString& path, QByteArray& bytes, QString& reason);
  [[nodiscard]] static bool readFormat(const QByteArray& chunk, Format& fmt, QString& reason);
  [[nodiscard]] static bool convertSamples(const Format& fmt,
                                           const QByteArray& data,
                                           DecodedSound& out,
                                           QString& reason);
};

}  // namespace Audio
}  // namespace IO
