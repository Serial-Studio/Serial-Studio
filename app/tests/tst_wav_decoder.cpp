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

#include <cstring>
#include <QtEndian>
#include <QtTest>

#include "IO/Audio/WavDecoder.h"

using IO::Audio::DecodedSound;
using IO::Audio::WavDecoder;

/**
 * @brief The first-party RIFF/WAVE reader (spec 0087 R10): every accepted layout decodes to
 *        normalized floats, every refusal names its reason, nothing else is accepted.
 */
class TstWavDecoder : public QObject {
  Q_OBJECT

private slots:
  void pcm16MonoDecodes();
  void pcm8IsUnsignedAndCentred();
  void pcm24AndPcm32Normalize();
  void float32PassesThrough();
  void stereoInterleaves();
  void extensibleHeaderResolvesSubformat();
  void unknownChunksAreSkipped();
  void rejectsNonRiff();
  void rejectsCompressedFormat();
  void rejectsThreeChannels();
  void rejectsOutOfRangeRate();
  void rejectsOverlongSound();
  void rejectsTruncatedChunk();
  void rejectsDataBeforeFormat();

private:
  [[nodiscard]] static QByteArray chunk(const char* id, const QByteArray& body);
  [[nodiscard]] static QByteArray fmt(int tag, int channels, int rate, int bits);
  [[nodiscard]] static QByteArray riff(const QByteArray& chunks);
  [[nodiscard]] static QByteArray le16(int v);
  [[nodiscard]] static QByteArray le32(qint32 v);
};

QByteArray TstWavDecoder::le16(int v)
{
  QByteArray b(2, '\0');
  qToLittleEndian<quint16>(static_cast<quint16>(v), b.data());
  return b;
}

QByteArray TstWavDecoder::le32(qint32 v)
{
  QByteArray b(4, '\0');
  qToLittleEndian<qint32>(v, b.data());
  return b;
}

QByteArray TstWavDecoder::chunk(const char* id, const QByteArray& body)
{
  QByteArray out(id, 4);
  out += le32(body.size());
  out += body;
  if (body.size() & 1)
    out += '\0';

  return out;
}

QByteArray TstWavDecoder::fmt(int tag, int channels, int rate, int bits)
{
  QByteArray body;
  body += le16(tag);
  body += le16(channels);
  body += le32(rate);
  body += le32(rate * channels * bits / 8);
  body += le16(channels * bits / 8);
  body += le16(bits);
  return chunk("fmt ", body);
}

QByteArray TstWavDecoder::riff(const QByteArray& chunks)
{
  QByteArray out("RIFF", 4);
  out += le32(chunks.size() + 4);
  out += "WAVE";
  out += chunks;
  return out;
}

void TstWavDecoder::pcm16MonoDecodes()
{
  QByteArray data = le16(0) + le16(16384) + le16(-32768) + le16(32767);
  DecodedSound out;
  QString reason;
  QVERIFY2(WavDecoder::decodeBytes(riff(fmt(1, 1, 48000, 16) + chunk("data", data)), out, reason),
           qPrintable(reason));
  QCOMPARE(out.sampleRate, 48000);
  QCOMPARE(out.channels, 1);
  QCOMPARE(out.frames(), qint64{4});
  QCOMPARE(out.samples[1], 0.5f);
  QCOMPARE(out.samples[2], -1.0f);
  QVERIFY(out.samples[3] > 0.999f);
}

void TstWavDecoder::pcm8IsUnsignedAndCentred()
{
  QByteArray data;
  data += static_cast<char>(128);
  data += static_cast<char>(0);
  data += static_cast<char>(255);
  DecodedSound out;
  QString reason;
  QVERIFY(WavDecoder::decodeBytes(riff(fmt(1, 1, 8000, 8) + chunk("data", data)), out, reason));
  QCOMPARE(out.samples[0], 0.0f);
  QCOMPARE(out.samples[1], -1.0f);
  QVERIFY(out.samples[2] > 0.99f);
}

void TstWavDecoder::pcm24AndPcm32Normalize()
{
  QByteArray data24;
  data24 += static_cast<char>(0x00);
  data24 += static_cast<char>(0x00);
  data24 += static_cast<char>(0x40);
  DecodedSound out;
  QString reason;
  QVERIFY(WavDecoder::decodeBytes(riff(fmt(1, 1, 44100, 24) + chunk("data", data24)), out, reason));
  QCOMPARE(out.samples[0], 0.5f);

  const QByteArray data32 = le32(-1073741824);
  QVERIFY(WavDecoder::decodeBytes(riff(fmt(1, 1, 44100, 32) + chunk("data", data32)), out, reason));
  QCOMPARE(out.samples[0], -0.5f);
}

void TstWavDecoder::float32PassesThrough()
{
  float v     = 0.25f;
  quint32 raw = 0;
  std::memcpy(&raw, &v, sizeof(raw));
  QByteArray data(4, '\0');
  qToLittleEndian<quint32>(raw, data.data());
  DecodedSound out;
  QString reason;
  QVERIFY(WavDecoder::decodeBytes(riff(fmt(3, 1, 96000, 32) + chunk("data", data)), out, reason));
  QCOMPARE(out.samples[0], 0.25f);
}

void TstWavDecoder::stereoInterleaves()
{
  QByteArray data = le16(16384) + le16(-16384) + le16(0) + le16(8192);
  DecodedSound out;
  QString reason;
  QVERIFY(WavDecoder::decodeBytes(riff(fmt(1, 2, 48000, 16) + chunk("data", data)), out, reason));
  QCOMPARE(out.channels, 2);
  QCOMPARE(out.frames(), qint64{2});
  QCOMPARE(out.samples[1], -0.5f);
  QCOMPARE(out.samples[3], 0.25f);
}

void TstWavDecoder::extensibleHeaderResolvesSubformat()
{
  QByteArray body;
  body                  += le16(0xFFFE);
  body                  += le16(1);
  body                  += le32(48000);
  body                  += le32(96000);
  body                  += le16(2);
  body                  += le16(16);
  body                  += le16(22);
  body                  += le16(16);
  body                  += le32(4);
  body                  += le16(1);
  body                  += QByteArray(14, '\0');
  const QByteArray data  = le16(16384);
  DecodedSound out;
  QString reason;
  QVERIFY(WavDecoder::decodeBytes(riff(chunk("fmt ", body) + chunk("data", data)), out, reason));
  QCOMPARE(out.samples[0], 0.5f);
}

void TstWavDecoder::unknownChunksAreSkipped()
{
  const QByteArray data = le16(16384);
  const QByteArray file = riff(chunk("LIST", QByteArray("abc", 3)) + fmt(1, 1, 48000, 16)
                               + chunk("fact", le32(1)) + chunk("data", data));
  DecodedSound out;
  QString reason;
  QVERIFY2(WavDecoder::decodeBytes(file, out, reason), qPrintable(reason));
  QCOMPARE(out.frames(), qint64{1});
}

void TstWavDecoder::rejectsNonRiff()
{
  DecodedSound out;
  QString reason;
  QVERIFY(!WavDecoder::decodeBytes(QByteArray("not a wav file at all"), out, reason));
  QVERIFY(!reason.isEmpty());
}

void TstWavDecoder::rejectsCompressedFormat()
{
  DecodedSound out;
  QString reason;
  QVERIFY(
    !WavDecoder::decodeBytes(riff(fmt(85, 1, 48000, 16) + chunk("data", le16(0))), out, reason));
  QVERIFY(reason.contains(QStringLiteral("encoding")));
}

void TstWavDecoder::rejectsThreeChannels()
{
  DecodedSound out;
  QString reason;
  QVERIFY(
    !WavDecoder::decodeBytes(riff(fmt(1, 3, 48000, 16) + chunk("data", le16(0))), out, reason));
  QVERIFY(reason.contains(QStringLiteral("channels")));
}

void TstWavDecoder::rejectsOutOfRangeRate()
{
  DecodedSound out;
  QString reason;
  QVERIFY(
    !WavDecoder::decodeBytes(riff(fmt(1, 1, 192000, 16) + chunk("data", le16(0))), out, reason));
  QVERIFY(reason.contains(QStringLiteral("sample rate")));
}

void TstWavDecoder::rejectsOverlongSound()
{
  const QByteArray data(8000 * 2 * 11, '\0');
  DecodedSound out;
  QString reason;
  QVERIFY(!WavDecoder::decodeBytes(riff(fmt(1, 1, 8000, 16) + chunk("data", data)), out, reason));
  QVERIFY(reason.contains(QStringLiteral("longer")));
}

void TstWavDecoder::rejectsTruncatedChunk()
{
  QByteArray file = riff(fmt(1, 1, 48000, 16) + chunk("data", le16(0) + le16(0)));
  file.chop(2);
  DecodedSound out;
  QString reason;
  QVERIFY(!WavDecoder::decodeBytes(file, out, reason));
  QVERIFY(reason.contains(QStringLiteral("Truncated")));
}

void TstWavDecoder::rejectsDataBeforeFormat()
{
  DecodedSound out;
  QString reason;
  QVERIFY(
    !WavDecoder::decodeBytes(riff(chunk("data", le16(0)) + fmt(1, 1, 48000, 16)), out, reason));
  QVERIFY(reason.contains(QStringLiteral("precedes")));
}

QTEST_APPLESS_MAIN(TstWavDecoder)
#include "tst_wav_decoder.moc"
