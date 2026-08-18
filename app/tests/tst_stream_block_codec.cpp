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

#include <bit>
#include <cmath>
#include <limits>
#include <QByteArray>
#include <QTest>
#include <vector>

#include "Sessions/StreamBlockCodec.h"

// The session stream-block wire format (spec 0054). Recording is only useful if what comes back
// is bit-identical to what went in -- AC2 and AC5 compare recorded samples for exact equality,
// with no tolerance, so a lossy or endian-dependent codec would silently fail those instead of
// failing here.

//--------------------------------------------------------------------------------------------------
// Test suite
//--------------------------------------------------------------------------------------------------

class StreamBlockCodecTest : public QObject {
  Q_OBJECT

private slots:
  void roundTripIsBitExact();
  void roundTripCarriesSpecialValues();
  void blobLengthIsEightBytesPerSample();
  void encodingIsExplicitlyLittleEndian();
  void truncatedBlobIsRejected();
  void misalignedBlobIsRejected();
  void frameCountMismatchIsRejected();
  void emptyBlockRoundTrips();
  void timesRoundTripBitExact();
  void textRoundTripsWithEmbeddedSeparators();
  void truncatedTextBlobIsRejected();
  void overlongTextBlobIsRejected();
};

/**
 * @brief Every sample survives the round trip with its exact bit pattern.
 */
void StreamBlockCodecTest::roundTripIsBitExact()
{
  const std::vector<double> input{0.0,
                                  -0.0,
                                  1.0,
                                  -1.0,
                                  0.1,
                                  -0.004729501903,
                                  1.0e-300,
                                  1.0e300,
                                  std::numeric_limits<double>::min(),
                                  std::numeric_limits<double>::max()};

  const QByteArray blob = Sessions::packStreamSamples(input);

  std::vector<double> output;
  QVERIFY(Sessions::unpackStreamSamples(blob, static_cast<qint64>(input.size()), output));
  QCOMPARE(output.size(), input.size());

  for (std::size_t i = 0; i < input.size(); ++i)
    QCOMPARE(std::bit_cast<quint64>(output[i]), std::bit_cast<quint64>(input[i]));
}

/**
 * @brief Infinities and NaN survive as themselves: the codec must not canonicalise, because the
 *        recorded value has to match what the pipeline produced.
 */
void StreamBlockCodecTest::roundTripCarriesSpecialValues()
{
  const std::vector<double> input{std::numeric_limits<double>::infinity(),
                                  -std::numeric_limits<double>::infinity(),
                                  std::numeric_limits<double>::quiet_NaN()};

  const QByteArray blob = Sessions::packStreamSamples(input);

  std::vector<double> output;
  QVERIFY(Sessions::unpackStreamSamples(blob, static_cast<qint64>(input.size()), output));
  QVERIFY(std::isinf(output[0]) && output[0] > 0);
  QVERIFY(std::isinf(output[1]) && output[1] < 0);
  QVERIFY(std::isnan(output[2]));
}

/**
 * @brief The blob is exactly `frames * 8` bytes, which is what the stored `frames` column and
 *        the explorer's SUM(frames) aggregate both rely on.
 */
void StreamBlockCodecTest::blobLengthIsEightBytesPerSample()
{
  const std::vector<double> input(97, 1.5);
  const QByteArray blob = Sessions::packStreamSamples(input);
  QCOMPARE(blob.size(), static_cast<qsizetype>(97) * Sessions::kStreamSampleBytes);
  QCOMPARE(Sessions::kStreamSampleBytes, static_cast<qsizetype>(8));
}

/**
 * @brief The byte order is little-endian regardless of host order: session databases move
 *        between machines, so a host-order blob would misdecode on a big-endian reader.
 */
void StreamBlockCodecTest::encodingIsExplicitlyLittleEndian()
{
  const std::vector<double> input{1.0};
  const QByteArray blob = Sessions::packStreamSamples(input);
  QCOMPARE(blob.size(), static_cast<qsizetype>(8));

  // IEEE-754 1.0 is 0x3FF0000000000000; little-endian puts the low byte first.
  const QByteArray expected = QByteArray::fromHex(QByteArrayLiteral("000000000000f03f"));
  QCOMPARE(blob, expected);
}

/**
 * @brief A blob shorter than `frames * 8` is refused rather than decoded past its end.
 */
void StreamBlockCodecTest::truncatedBlobIsRejected()
{
  const std::vector<double> input(8, 3.25);
  QByteArray blob = Sessions::packStreamSamples(input);
  blob.chop(8);

  std::vector<double> output;
  QVERIFY(!Sessions::unpackStreamSamples(blob, 8, output));
}

/**
 * @brief A blob whose length is not a multiple of eight cannot be a valid sample array.
 */
void StreamBlockCodecTest::misalignedBlobIsRejected()
{
  QByteArray blob(13, '\0');

  std::vector<double> output;
  QVERIFY(!Sessions::unpackStreamSamples(blob, 1, output));
  QVERIFY(!Sessions::unpackStreamSamples(blob, 2, output));
}

/**
 * @brief A well-formed blob paired with the wrong frame count is refused: the row's `frames`
 *        column and its blob must agree or the sample times would be wrong.
 */
void StreamBlockCodecTest::frameCountMismatchIsRejected()
{
  const std::vector<double> input(4, 2.0);
  const QByteArray blob = Sessions::packStreamSamples(input);

  std::vector<double> output;
  QVERIFY(!Sessions::unpackStreamSamples(blob, 3, output));
  QVERIFY(!Sessions::unpackStreamSamples(blob, 5, output));
  QVERIFY(!Sessions::unpackStreamSamples(blob, -1, output));
  QVERIFY(Sessions::unpackStreamSamples(blob, 4, output));
}

/**
 * @brief A zero-sample block is representable and decodes to an empty vector.
 */
void StreamBlockCodecTest::emptyBlockRoundTrips()
{
  const QByteArray blob = Sessions::packStreamSamples({});
  QVERIFY(blob.isEmpty());

  std::vector<double> output{1.0, 2.0};
  QVERIFY(Sessions::unpackStreamSamples(blob, 0, output));
  QVERIFY(output.empty());
}

/**
 * @brief Explicit per-sample times survive bit-exact, including negatives: an irregular block's
 *        offsets are what reconstruct its clock on replay.
 */
void StreamBlockCodecTest::timesRoundTripBitExact()
{
  const std::vector<qint64> input{
    0, 1, -1, 4096, std::numeric_limits<qint64>::max(), std::numeric_limits<qint64>::min()};

  const QByteArray blob = Sessions::packStreamTimes(input);
  QCOMPARE(blob.size(), static_cast<qsizetype>(input.size()) * Sessions::kStreamSampleBytes);

  std::vector<qint64> output;
  QVERIFY(Sessions::unpackStreamTimes(blob, static_cast<qint64>(input.size()), output));
  QCOMPARE(output, input);
}

/**
 * @brief Text is length-prefixed, so a recorded value containing commas, quotes, newlines or NULs
 *        survives intact -- a delimited encoding would corrupt exactly the values worth recording.
 */
void StreamBlockCodecTest::textRoundTripsWithEmbeddedSeparators()
{
  const std::vector<QString> input{QStringLiteral("plain"),
                                   QStringLiteral("a,b"),
                                   QStringLiteral("say \"hi\""),
                                   QStringLiteral("line1\nline2"),
                                   QString(),
                                   QStringLiteral("uni\u00e7\u00f8de")};

  const QByteArray blob = Sessions::packStreamText(input);

  std::vector<QString> output;
  QVERIFY(Sessions::unpackStreamText(blob, static_cast<qint64>(input.size()), output));
  QCOMPARE(output, input);
}

/**
 * @brief A blob whose last entry is cut short is refused rather than decoded past its end.
 */
void StreamBlockCodecTest::truncatedTextBlobIsRejected()
{
  const std::vector<QString> pair{QStringLiteral("abcd"), QStringLiteral("efgh")};
  QByteArray blob = Sessions::packStreamText(pair);
  blob.chop(2);

  std::vector<QString> output;
  QVERIFY(!Sessions::unpackStreamText(blob, 2, output));
}

/**
 * @brief Trailing bytes past the declared entry count are refused: the row's frame count and its
 *        blob must agree, exactly as they must for the sample array.
 */
void StreamBlockCodecTest::overlongTextBlobIsRejected()
{
  const std::vector<QString> single{QStringLiteral("abcd")};
  QByteArray blob = Sessions::packStreamText(single);
  blob.append('x');

  std::vector<QString> output;
  QVERIFY(!Sessions::unpackStreamText(blob, 1, output));
  QVERIFY(!Sessions::unpackStreamText(blob, 2, output));
}

QTEST_APPLESS_MAIN(StreamBlockCodecTest)

#include "tst_stream_block_codec.moc"
