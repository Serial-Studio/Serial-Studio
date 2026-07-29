/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <chrono>
#include <QByteArray>
#include <QCoreApplication>
#include <QList>
#include <QSignalSpy>
#include <QString>
#include <QTest>

#include "IO/Checksum.h"
#include "IO/FrameReader.h"

// Companion to tst_frame_delimiters: that suite pins delimiter scanning and the CRC-16 end
// delimited checksum path, so nothing here repeats it. What is covered instead is the product of
// checksum validation with the start-delimited and start+end detection modes, checksum widths
// other than two bytes, the zero-length frame branches, the NoDelimiters raw-chunk fast lane, the
// mode/checksum coupling in setOperationMode(), multi-chunk frame timestamps, and the captured
// slot pool's heap fallback. Every test builds its own reader on the calling thread, matching the
// production main-thread contract.

using SteadyClock     = IO::CapturedData::SteadyClock;
using SteadyTimePoint = IO::CapturedData::SteadyTimePoint;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wraps bytes as a driver capture, which is where a frame's timestamp is owned.
 */
static IO::CapturedDataPtr capture(const QByteArray& bytes,
                                   SteadyTimePoint stamp         = SteadyClock::now(),
                                   std::chrono::nanoseconds step = std::chrono::nanoseconds(1))
{
  return IO::makeCapturedData(bytes, stamp, step);
}

/**
 * @brief Drains every frame the reader has queued, in FIFO order.
 */
static QList<QByteArray> drainFrames(IO::FrameReader& reader)
{
  QList<QByteArray> frames;
  IO::CapturedDataPtr item;
  while (reader.queue().try_dequeue(item))
    frames.append(item->data);

  return frames;
}

/**
 * @brief Drains the queued captures themselves, so a test can inspect their timestamps.
 */
static QList<IO::CapturedDataPtr> drainCaptures(IO::FrameReader& reader)
{
  QList<IO::CapturedDataPtr> items;
  IO::CapturedDataPtr item;
  while (reader.queue().try_dequeue(item))
    items.append(item);

  return items;
}

/**
 * @brief ProjectFile mode is the only one that validates checksums, so the checksum must be set
 *        after the operation mode: leaving ProjectFile clears the cached algorithm.
 */
static void configureProject(IO::FrameReader& reader,
                             SerialStudio::FrameDetection detection,
                             const QList<QByteArray>& starts,
                             const QList<QByteArray>& finishes,
                             const QString& checksum = QString())
{
  reader.setOperationMode(SerialStudio::ProjectFile);
  reader.setFrameDetectionMode(detection);
  reader.setStartSequences(starts);
  reader.setFinishSequences(finishes);
  reader.setChecksum(checksum);
}

/**
 * @brief Produces a trailer that is guaranteed to differ from the real one without ever spelling a
 *        delimiter byte, so a corrupted frame is rejected by the checksum and not by the scan.
 */
static QByteArray corruptTrailer(const QByteArray& trailer)
{
  QByteArray bad = trailer;
  bad[0]         = (trailer.at(0) == char(0x01)) ? char(0x02) : char(0x01);
  return bad;
}

/**
 * @brief Start-delimited framing carries the checksum immediately before the next start sequence,
 *        so a two-byte trailer can only be mistaken for a three-byte delimiter if it spells one:
 *        with "<S>" that is arithmetically impossible, which keeps these tests deterministic.
 */
static const QByteArray kStartSeq = QByteArray("<S>");

/**
 * @brief Checksum validation across the frame-detection modes, checksum widths, degenerate frame
 *        geometries, mode transitions, chunk-spanning timestamps, and slot-pool exhaustion.
 */
class TstFrameReaderModes : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void startDelimitedValidChecksumIsAccepted();
  void startDelimitedInvalidChecksumDropsTheFrame();
  void startDelimitedFrameWaitsForTheNextStartDelimiter();
  void startDelimitedChecksumWiderThanTheFrameIsDiscarded();

  void startEndDelimitedChecksumIsValidated_data();
  void startEndDelimitedChecksumIsValidated();
  void startEndDelimitedIncompleteChecksumWaitsForMoreBytes();
  void startEndDelimiterWithoutAnyStartDiscardsBytes();

  void checksumAlgorithmWidths_data();
  void checksumAlgorithmWidths();
  void unknownChecksumNameDisablesValidation();

  void backToBackStartDelimitersYieldNoFrame();
  void adjacentStartAndEndDelimitersYieldNoFrame();

  void noDelimiterModeCountsDroppedFramesWhenSaturated();
  void nullAndEmptyCapturesAreIgnored();

  void leavingProjectFileClearsTheChecksum();
  void checksumSetAfterTheModeIsHonouredInQuickPlot();

  void quickPlotIgnoresTheFrameDetectionMode_data();
  void quickPlotIgnoresTheFrameDetectionMode();

  void frameTimestampComesFromTheChunkHoldingTheLastByte();
  void frameTimestampsFollowSuccessiveProcessDataCalls();

  void heapFallbackKeepsFramesIntactWhenThePoolIsExhausted();
};

/**
 * @brief Pins the settings scope: the dropped-frame path lazily builds the notification center,
 *        whose constructor reads QSettings. A test-only identity keeps that away from the
 *        application's own configuration.
 */
void TstFrameReaderModes::initTestCase()
{
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_frame_reader_modes"));
}

//--------------------------------------------------------------------------------------------------
// Start-delimited framing with a checksum
//--------------------------------------------------------------------------------------------------

/**
 * @brief In start-delimited framing the trailer sits between the payload and the next start
 *        sequence, so the reader must exclude it from the frame it publishes.
 */
void TstFrameReaderModes::startDelimitedValidChecksumIsAccepted()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartDelimiterOnly, {kStartSeq}, {}, QStringLiteral("CRC-16"));

  const QByteArray one = QByteArray("one");
  const QByteArray two = QByteArray("two");
  reader.processData(capture(kStartSeq + one + IO::checksum(QStringLiteral("CRC-16"), one)
                             + kStartSeq + two + IO::checksum(QStringLiteral("CRC-16"), two)
                             + kStartSeq));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({one, two}));
  QCOMPARE(reader.framesExtracted(), quint64(2));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

/**
 * @brief A bad trailer must consume the whole frame rather than resynchronising on the delimiter,
 *        so the next well-formed frame still lands.
 */
void TstFrameReaderModes::startDelimitedInvalidChecksumDropsTheFrame()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartDelimiterOnly, {kStartSeq}, {}, QStringLiteral("CRC-16"));

  const QByteArray bad  = QByteArray("bad");
  const QByteArray good = QByteArray("good");
  reader.processData(
    capture(kStartSeq + bad + corruptTrailer(IO::checksum(QStringLiteral("CRC-16"), bad))
            + kStartSeq + good + IO::checksum(QStringLiteral("CRC-16"), good) + kStartSeq));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({good}));
  QCOMPARE(reader.checksumErrorCount(), quint64(1));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

/**
 * @brief ChecksumIncomplete is unreachable here: the frame end is the next start sequence, so by
 *        the time the geometry is known the trailer has already arrived. The wait therefore
 *        happens one step earlier, on the missing delimiter, and must not lose the buffered bytes.
 */
void TstFrameReaderModes::startDelimitedFrameWaitsForTheNextStartDelimiter()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartDelimiterOnly, {kStartSeq}, {}, QStringLiteral("CRC-16"));

  const QByteArray payload = QByteArray("waiting");
  reader.processData(
    capture(kStartSeq + payload + IO::checksum(QStringLiteral("CRC-16"), payload)));
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.checksumErrorCount(), quint64(0));

  reader.processData(capture(kStartSeq));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({payload}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

/**
 * @brief When two start sequences are closer together than the trailer is wide the checksum would
 *        begin before the payload does, so the candidate is discarded instead of read backwards.
 */
void TstFrameReaderModes::startDelimitedChecksumWiderThanTheFrameIsDiscarded()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartDelimiterOnly, {kStartSeq}, {}, QStringLiteral("CRC-16"));

  const QByteArray payload = QByteArray("ok");
  reader.processData(capture(kStartSeq + QByteArray("X") + kStartSeq + payload
                             + IO::checksum(QStringLiteral("CRC-16"), payload) + kStartSeq));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({payload}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

//--------------------------------------------------------------------------------------------------
// Start+end delimited framing with a checksum
//--------------------------------------------------------------------------------------------------

void TstFrameReaderModes::startEndDelimitedChecksumIsValidated_data()
{
  QTest::addColumn<QString>("algorithm");

  QTest::newRow("one byte") << QStringLiteral("XOR-8");
  QTest::newRow("two bytes") << QStringLiteral("CRC-16");
  QTest::newRow("four bytes") << QStringLiteral("CRC-32");
  QTest::newRow("four bytes, sum based") << QStringLiteral("Adler-32");
}

/**
 * @brief With both delimiters configured the trailer follows the end sequence, so the checksum
 *        window is offset from the frame body by the delimiter width at every checksum size.
 */
void TstFrameReaderModes::startEndDelimitedChecksumIsValidated()
{
  QFETCH(QString, algorithm);

  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartAndEndDelimiter, {QByteArray("/*")}, {QByteArray("*/")}, algorithm);

  const QByteArray good = QByteArray("abc");
  reader.processData(
    capture(QByteArray("/*") + good + QByteArray("*/") + IO::checksum(algorithm, good)));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({good}));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));

  const QByteArray bad = QByteArray("def");
  reader.processData(capture(QByteArray("/*") + bad + QByteArray("*/")
                             + corruptTrailer(IO::checksum(algorithm, bad))));
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.checksumErrorCount(), quint64(1));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

/**
 * @brief A trailer that arrives one byte at a time must leave the frame buffered rather than
 *        failing validation against the bytes that happen to be present.
 */
void TstFrameReaderModes::startEndDelimitedIncompleteChecksumWaitsForMoreBytes()
{
  IO::FrameReader reader;
  configureProject(reader,
                   SerialStudio::StartAndEndDelimiter,
                   {QByteArray("/*")},
                   {QByteArray("*/")},
                   QStringLiteral("CRC-32"));

  const QByteArray payload = QByteArray("abc");
  const QByteArray trailer = IO::checksum(QStringLiteral("CRC-32"), payload);
  QCOMPARE(trailer.size(), qsizetype(4));

  reader.processData(capture(QByteArray("/*") + payload + QByteArray("*/")));
  QVERIFY(drainFrames(reader).isEmpty());

  for (qsizetype i = 0; i < trailer.size() - 1; ++i) {
    reader.processData(capture(trailer.mid(i, 1)));
    QVERIFY(drainFrames(reader).isEmpty());
    QCOMPARE(reader.checksumErrorCount(), quint64(0));
  }

  reader.processData(capture(trailer.right(1)));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({payload}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

/**
 * @brief An end delimiter with no start sequence anywhere in the buffer closes nothing: the bytes
 *        are dropped up to and including it, leaving the scan able to find the next real frame.
 */
void TstFrameReaderModes::startEndDelimiterWithoutAnyStartDiscardsBytes()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartAndEndDelimiter, {QByteArray("/*")}, {QByteArray("*/")});

  reader.processData(capture(QByteArray("junk*/")));
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.framesExtracted(), quint64(0));

  reader.processData(capture(QByteArray("/*ok*/")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("ok")}));
}

//--------------------------------------------------------------------------------------------------
// Checksum width coverage
//--------------------------------------------------------------------------------------------------

void TstFrameReaderModes::checksumAlgorithmWidths_data()
{
  QTest::addColumn<QString>("algorithm");
  QTest::addColumn<qsizetype>("width");

  QTest::newRow("XOR-8") << QStringLiteral("XOR-8") << qsizetype(1);
  QTest::newRow("MOD-256") << QStringLiteral("MOD-256") << qsizetype(1);
  QTest::newRow("CRC-8") << QStringLiteral("CRC-8") << qsizetype(1);
  QTest::newRow("CRC-16-MODBUS") << QStringLiteral("CRC-16-MODBUS") << qsizetype(2);
  QTest::newRow("CRC-16-CCITT") << QStringLiteral("CRC-16-CCITT") << qsizetype(2);
  QTest::newRow("Fletcher-16") << QStringLiteral("Fletcher-16") << qsizetype(2);
  QTest::newRow("CRC-32") << QStringLiteral("CRC-32") << qsizetype(4);
  QTest::newRow("Adler-32") << QStringLiteral("Adler-32") << qsizetype(4);
}

/**
 * @brief The reader caches the trailer width from the algorithm itself, so every registered width
 *        must frame, validate, and consume end to end and not just the two-byte case.
 */
void TstFrameReaderModes::checksumAlgorithmWidths()
{
  QFETCH(QString, algorithm);
  QFETCH(qsizetype, width);

  IO::FrameReader reader;
  configureProject(reader, SerialStudio::EndDelimiterOnly, {}, {QByteArray("\n")}, algorithm);

  const QByteArray first = QByteArray("first");
  QCOMPARE(IO::checksum(algorithm, first).size(), width);

  reader.processData(capture(first + QByteArray("\n") + IO::checksum(algorithm, first)));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({first}));

  const QByteArray bad = QByteArray("broken");
  reader.processData(
    capture(bad + QByteArray("\n") + corruptTrailer(IO::checksum(algorithm, bad))));
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.checksumErrorCount(), quint64(1));

  const QByteArray last = QByteArray("last");
  reader.processData(capture(last + QByteArray("\n") + IO::checksum(algorithm, last)));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({last}));
  QCOMPARE(reader.framesExtracted(), quint64(2));
}

/**
 * @brief A name that is not in the algorithm map leaves the reader with no trailer at all, so
 *        frames must not stall waiting for bytes the sender never sends.
 */
void TstFrameReaderModes::unknownChecksumNameDisablesValidation()
{
  IO::FrameReader reader;
  configureProject(reader,
                   SerialStudio::EndDelimiterOnly,
                   {},
                   {QByteArray("\n")},
                   QStringLiteral("Not-A-Checksum"));

  reader.processData(capture(QByteArray("abc\ndef\n")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("abc"), QByteArray("def")}));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

//--------------------------------------------------------------------------------------------------
// Degenerate frame geometries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two start sequences with nothing between them delimit an empty frame: the bytes are
 *        consumed, nothing is queued, and the scan keeps its place for the real frame behind them.
 */
void TstFrameReaderModes::backToBackStartDelimitersYieldNoFrame()
{
  IO::FrameReader reader;
  configureProject(reader, SerialStudio::StartDelimiterOnly, {QByteArray("$")}, {});

  reader.processData(capture(QByteArray("$$ok$")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("ok")}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

/**
 * @brief An end sequence immediately after a start sequence encloses no payload, so the pair is
 *        discarded rather than queued as an empty frame.
 */
void TstFrameReaderModes::adjacentStartAndEndDelimitersYieldNoFrame()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartAndEndDelimiter, {QByteArray("/*")}, {QByteArray("*/")});

  reader.processData(capture(QByteArray("/**//*ok*/")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("ok")}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

//--------------------------------------------------------------------------------------------------
// Raw-chunk fast lane and input guards
//--------------------------------------------------------------------------------------------------

/**
 * @brief The NoDelimiters lane bypasses the scan buffer and enqueues the capture itself, so its
 *        only backpressure is the queue: past saturation every chunk must be counted as dropped,
 *        and the lane must resume once the consumer catches up.
 */
void TstFrameReaderModes::noDelimiterModeCountsDroppedFramesWhenSaturated()
{
  IO::FrameReader reader;
  configureProject(reader, SerialStudio::NoDelimiters, {}, {});

  constexpr int kChunks  = 70000;
  const QByteArray chunk = QByteArray("abcd");
  const auto stamp       = SteadyClock::now();
  for (int i = 0; i < kChunks; ++i)
    reader.processData(capture(chunk, stamp));

  QVERIFY(reader.droppedFrameCount() > 0);
  QVERIFY(reader.framesExtracted() >= 65536);
  QCOMPARE(reader.framesExtracted() + reader.droppedFrameCount(), quint64(kChunks));
  QCOMPARE(reader.bytesReceived(), quint64(kChunks) * quint64(chunk.size()));

  const auto drained = drainFrames(reader);
  QCOMPARE(quint64(drained.size()), reader.framesExtracted());
  QCOMPARE(drained.first(), chunk);

  const quint64 extracted = reader.framesExtracted();
  const quint64 dropped   = reader.droppedFrameCount();
  reader.processData(capture(QByteArray("tail")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("tail")}));
  QCOMPARE(reader.framesExtracted(), extracted + 1);
  QCOMPARE(reader.droppedFrameCount(), dropped);
}

/**
 * @brief A null or empty capture returns before the byte counter, so a driver that signals an
 *        empty read costs the reader nothing and never wakes the consumer.
 */
void TstFrameReaderModes::nullAndEmptyCapturesAreIgnored()
{
  IO::FrameReader reader;
  configureProject(reader, SerialStudio::EndDelimiterOnly, {}, {QByteArray("\n")});

  QSignalSpy spy(&reader, &IO::FrameReader::readyRead);

  reader.processData(IO::CapturedDataPtr());
  reader.processData(capture(QByteArray()));

  QCOMPARE(spy.count(), 0);
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.bytesReceived(), quint64(0));
  QCOMPARE(reader.framesExtracted(), quint64(0));
}

//--------------------------------------------------------------------------------------------------
// Mode transitions and the checksum coupling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Leaving ProjectFile drops the cached algorithm, which releases a frame that was waiting
 *        for trailer bytes; re-entering ProjectFile does not bring the algorithm back.
 */
void TstFrameReaderModes::leavingProjectFileClearsTheChecksum()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::EndDelimiterOnly, {}, {QByteArray("\n")}, QStringLiteral("CRC-16"));

  reader.processData(capture(QByteArray("abc\n")));
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.checksumErrorCount(), quint64(0));

  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.processData(capture(QByteArray("\n")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("abc")}));

  reader.setOperationMode(SerialStudio::ProjectFile);
  reader.processData(capture(QByteArray("def\n")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("def")}));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

/**
 * @brief Current behaviour, and the reason configuration order is load-bearing: only
 *        setOperationMode() clears the algorithm, so a checksum installed afterwards is validated
 *        even in QuickPlot, where the same configuration applied in the other order would not be.
 */
void TstFrameReaderModes::checksumSetAfterTheModeIsHonouredInQuickPlot()
{
  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFrameDetectionMode(SerialStudio::EndDelimiterOnly);
  reader.setFinishSequences({QByteArray("\n")});
  reader.setChecksum(QStringLiteral("CRC-16"));

  const QByteArray payload = QByteArray("abc");
  reader.processData(capture(payload + QByteArray("\n")));
  QVERIFY(drainFrames(reader).isEmpty());

  reader.processData(capture(IO::checksum(QStringLiteral("CRC-16"), payload)));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({payload}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

void TstFrameReaderModes::quickPlotIgnoresTheFrameDetectionMode_data()
{
  QTest::addColumn<int>("detection");

  QTest::newRow("end delimiter only") << int(SerialStudio::EndDelimiterOnly);
  QTest::newRow("start and end delimiter") << int(SerialStudio::StartAndEndDelimiter);
  QTest::newRow("no delimiters") << int(SerialStudio::NoDelimiters);
  QTest::newRow("start delimiter only") << int(SerialStudio::StartDelimiterOnly);
}

/**
 * @brief QuickPlot dispatches on the operation mode alone: the detection mode and any configured
 *        start sequences are inert, and NoDelimiters does not divert to the raw-chunk lane, which
 *        belongs to ProjectFile.
 */
void TstFrameReaderModes::quickPlotIgnoresTheFrameDetectionMode()
{
  QFETCH(int, detection);

  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFrameDetectionMode(static_cast<SerialStudio::FrameDetection>(detection));
  reader.setStartSequences({QByteArray("$")});
  reader.setFinishSequences({QByteArray("\n")});

  reader.processData(capture(QByteArray("$one\n$two\n")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("$one"), QByteArray("$two")}));
  QCOMPARE(reader.framesExtracted(), quint64(2));
}

//--------------------------------------------------------------------------------------------------
// Frame timing across chunks
//--------------------------------------------------------------------------------------------------

/**
 * @brief The source owns time: a frame assembled from three reads is stamped from the chunk that
 *        carried its last byte, not from the one that opened it and not from the wall clock.
 */
void TstFrameReaderModes::frameTimestampComesFromTheChunkHoldingTheLastByte()
{
  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFrameDetectionMode(SerialStudio::EndDelimiterOnly);
  reader.setFinishSequences({QByteArray("\n")});

  const auto first  = SteadyClock::now() - std::chrono::seconds(5);
  const auto second = first + std::chrono::milliseconds(1);
  const auto third  = first + std::chrono::milliseconds(2);

  reader.processData(capture(QByteArray("ab"), first));
  reader.processData(capture(QByteArray("cd"), second));
  QVERIFY(drainFrames(reader).isEmpty());

  reader.processData(capture(QByteArray("ef\n"), third));

  const auto captures = drainCaptures(reader);
  QCOMPARE(captures.size(), qsizetype(1));
  QCOMPARE(captures[0]->data, QByteArray("abcdef"));
  QVERIFY(captures[0]->timestamp == third);
}

/**
 * @brief Two frames split unevenly across two reads keep one stamp each: the frame completed by
 *        the second read must not inherit the first read's stamp.
 */
void TstFrameReaderModes::frameTimestampsFollowSuccessiveProcessDataCalls()
{
  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFrameDetectionMode(SerialStudio::EndDelimiterOnly);
  reader.setFinishSequences({QByteArray("\n")});

  const auto first  = SteadyClock::now() - std::chrono::seconds(5);
  const auto second = first + std::chrono::milliseconds(7);

  reader.processData(capture(QByteArray("one\ntw"), first));
  reader.processData(capture(QByteArray("o\n"), second));

  const auto captures = drainCaptures(reader);
  QCOMPARE(captures.size(), qsizetype(2));
  QCOMPARE(captures[0]->data, QByteArray("one"));
  QCOMPARE(captures[1]->data, QByteArray("two"));
  QVERIFY(captures[0]->timestamp == first);
  QVERIFY(captures[1]->timestamp == second);
}

//--------------------------------------------------------------------------------------------------
// Captured-slot pool
//--------------------------------------------------------------------------------------------------

/**
 * @brief A consumer that never drains pins all 4096 pooled slots, after which every frame comes
 *        from the heap fallback. The fallback is a correctness path, not just a pressure valve:
 *        the frames it produces must be complete, ordered, and none of them dropped.
 */
void TstFrameReaderModes::heapFallbackKeepsFramesIntactWhenThePoolIsExhausted()
{
  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFrameDetectionMode(SerialStudio::EndDelimiterOnly);
  reader.setFinishSequences({QByteArray("\n")});

  constexpr int kFrames = 5000;
  QByteArray stream;
  stream.reserve(kFrames * 5);
  for (int i = 0; i < kFrames; ++i)
    stream += QByteArray::number(i).rightJustified(4, '0') + QByteArray("\n");

  reader.processData(capture(stream));

  const auto frames = drainFrames(reader);
  QCOMPARE(frames.size(), qsizetype(kFrames));
  QCOMPARE(reader.framesExtracted(), quint64(kFrames));
  QCOMPARE(reader.droppedFrameCount(), quint64(0));

  for (int i = 0; i < kFrames; ++i)
    QCOMPARE(frames.at(i), QByteArray::number(i).rightJustified(4, '0'));
}

QTEST_GUILESS_MAIN(TstFrameReaderModes)

#include "tst_frame_reader_modes.moc"
