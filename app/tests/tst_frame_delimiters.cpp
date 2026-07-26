/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "IO/FrameReader.h"

// Every test function here is self-contained: it builds its own reader, so Qt Test's
// declaration-order execution is never load-bearing. The reader is driven entirely on the test's
// own thread, matching the production main-thread contract -- no producer thread in v1.

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
 * @brief QuickPlot never validates a checksum, so it isolates pure delimiter behaviour.
 */
static void configureQuickPlot(IO::FrameReader& reader, const QList<QByteArray>& finishes)
{
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFrameDetectionMode(SerialStudio::EndDelimiterOnly);
  reader.setFinishSequences(finishes);
}

/**
 * @brief ProjectFile mode is the only one that validates checksums, so the checksum must be set
 *        after the operation mode -- leaving ProjectFile clears the cached algorithm.
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
 * @brief Frame extraction for IO::FrameReader: what counts as a frame, what is discarded, and what
 *        the link-diagnostic counters report while it happens.
 */
class TstFrameDelimiters : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void endDelimitedFramesAreExtracted();
  void incompleteTrailingFrameStaysBuffered();
  void delimiterSplitAcrossChunksStillMatches();
  void emptyFramesAreConsumedNotEnqueued();
  void delimiterAsTheFinalByteDrainsTheBuffer();
  void multiByteDelimiterWidths_data();
  void multiByteDelimiterWidths();
  void multipleFinishSequencesEachTerminateAFrame();
  void emptyDelimitersAreIgnored();

  void startDelimitedFramesAreExtracted();
  void multipleStartSequencesOnlyUseTheFirst();
  void startAndEndDelimitedFramesAreExtracted();
  void startAndEndDelimiterDiscardsAnUnopenedFrame();
  void noDelimiterModeEnqueuesTheRawChunk();
  void consoleOnlyModeParsesNothing();

  void validChecksumIsAccepted();
  void invalidChecksumDropsTheFrame();
  void incompleteChecksumWaitsForMoreBytes();

  void timestampsAdvanceByFrameStep();
  void bufferOverflowIsCountedAndDropped();
  void queueSaturationCountsDroppedFrames();
  void diagnosticCountersReset();
  void readyReadTracksTheQueue();
};

/**
 * @brief Pins the settings scope: the dropped-frame path lazily builds the notification center,
 *        whose constructor reads QSettings. A test-only identity keeps that away from the
 *        application's own configuration.
 */
void TstFrameDelimiters::initTestCase()
{
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_frame_delimiters"));
}

//--------------------------------------------------------------------------------------------------
// End-delimited extraction
//--------------------------------------------------------------------------------------------------

void TstFrameDelimiters::endDelimitedFramesAreExtracted()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  reader.processData(capture(QByteArray("one\ntwo\nthree\n")));

  const auto frames = drainFrames(reader);
  QCOMPARE(frames, QList<QByteArray>({QByteArray("one"), QByteArray("two"), QByteArray("three")}));
  QCOMPARE(reader.framesExtracted(), quint64(3));
  QCOMPARE(reader.bytesReceived(), quint64(14));
  QCOMPARE(reader.droppedFrameCount(), quint64(0));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

/**
 * @brief Bytes after the last delimiter are not a frame yet; they must survive until the rest of
 *        the line arrives in a later chunk.
 */
void TstFrameDelimiters::incompleteTrailingFrameStaysBuffered()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  reader.processData(capture(QByteArray("one\ntwo")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("one")}));

  reader.processData(capture(QByteArray("\n")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("two")}));
  QCOMPARE(reader.framesExtracted(), quint64(2));
}

/**
 * @brief A multi-byte delimiter torn between two reads is the classic dropped-frame report.
 */
void TstFrameDelimiters::delimiterSplitAcrossChunksStillMatches()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\r\n")});

  reader.processData(capture(QByteArray("abc\r")));
  QVERIFY(drainFrames(reader).isEmpty());

  reader.processData(capture(QByteArray("\ndef\r\n")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("abc"), QByteArray("def")}));
}

/**
 * @brief Back-to-back delimiters carry no payload: the bytes are consumed, but nothing reaches the
 *        queue and the extraction counter does not move.
 */
void TstFrameDelimiters::emptyFramesAreConsumedNotEnqueued()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  reader.processData(capture(QByteArray("\n\nabc\n\n")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("abc")}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

void TstFrameDelimiters::delimiterAsTheFinalByteDrainsTheBuffer()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  reader.processData(capture(QByteArray("payload\n")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("payload")}));

  reader.processData(capture(QByteArray("next\n")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("next")}));
}

void TstFrameDelimiters::multiByteDelimiterWidths_data()
{
  QTest::addColumn<QByteArray>("delimiter");

  QTest::newRow("one byte") << QByteArray("\n");
  QTest::newRow("two bytes") << QByteArray("\r\n");
  QTest::newRow("seven bytes") << QByteArray("<-END->");
  QTest::newRow("eight bytes (memcmp lane)") << QByteArray("<--END->");
  QTest::newRow("nine bytes (kmp lane)") << QByteArray("<--END-->");
}

/**
 * @brief The scan switches from the memcmp lane to the KMP lane above eight bytes, so delimiter
 *        width is a behavioural boundary rather than a cosmetic detail.
 */
void TstFrameDelimiters::multiByteDelimiterWidths()
{
  QFETCH(QByteArray, delimiter);

  IO::FrameReader reader;
  configureQuickPlot(reader, {delimiter});

  reader.processData(capture(QByteArray("alpha") + delimiter + QByteArray("beta") + delimiter));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("alpha"), QByteArray("beta")}));
  QCOMPARE(reader.framesExtracted(), quint64(2));
}

/**
 * @brief With more than one end delimiter configured, the reader takes whichever occurs first.
 */
void TstFrameDelimiters::multipleFinishSequencesEachTerminateAFrame()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n"), QByteArray(";")});

  reader.processData(capture(QByteArray("one;two\nthree;")));

  QCOMPARE(drainFrames(reader),
           QList<QByteArray>({QByteArray("one"), QByteArray("two"), QByteArray("three")}));
}

/**
 * @brief An empty delimiter in the configured list is skipped rather than stored: keeping it would
 *        leave a pattern that can never match at the head of the list.
 */
void TstFrameDelimiters::emptyDelimitersAreIgnored()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray(), QByteArray("\n")});

  reader.processData(capture(QByteArray("one\n")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("one")}));
}

//--------------------------------------------------------------------------------------------------
// Start-delimited and start+end extraction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Start-delimited frames are bounded by consecutive start sequences, so the last opened
 *        frame stays buffered until the next one begins.
 */
void TstFrameDelimiters::startDelimitedFramesAreExtracted()
{
  IO::FrameReader reader;
  configureProject(reader, SerialStudio::StartDelimiterOnly, {QByteArray("$")}, {});

  reader.processData(capture(QByteArray("$one$two$")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("one"), QByteArray("two")}));

  reader.processData(capture(QByteArray("three$")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("three")}));
}

/**
 * @brief Current behaviour: only the first configured start sequence is scanned for. The second
 *        entry is stored but never used, so bytes containing it are framed as ordinary payload.
 */
void TstFrameDelimiters::multipleStartSequencesOnlyUseTheFirst()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartDelimiterOnly, {QByteArray("$"), QByteArray("#")}, {});

  reader.processData(capture(QByteArray("$one$#two#")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("one")}));

  reader.processData(capture(QByteArray("$")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("#two#")}));
}

void TstFrameDelimiters::startAndEndDelimitedFramesAreExtracted()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartAndEndDelimiter, {QByteArray("/*")}, {QByteArray("*/")});

  reader.processData(capture(QByteArray("/*one*/junk/*two*/")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("one"), QByteArray("two")}));
  QCOMPARE(reader.framesExtracted(), quint64(2));
}

/**
 * @brief An end delimiter with no start before it closes nothing; the bytes up to and including it
 *        are discarded so the next well-formed frame is still found.
 */
void TstFrameDelimiters::startAndEndDelimiterDiscardsAnUnopenedFrame()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::StartAndEndDelimiter, {QByteArray("/*")}, {QByteArray("*/")});

  reader.processData(capture(QByteArray("orphan*//*ok*/")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("ok")}));
}

/**
 * @brief With delimiters disabled, the captured chunk is the frame; no scan happens at all.
 */
void TstFrameDelimiters::noDelimiterModeEnqueuesTheRawChunk()
{
  IO::FrameReader reader;
  configureProject(reader, SerialStudio::NoDelimiters, {}, {});

  reader.processData(capture(QByteArray("raw bytes")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("raw bytes")}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
}

/**
 * @brief Console-only mode returns before the byte counter, so the terminal path costs the reader
 *        nothing at all.
 */
void TstFrameDelimiters::consoleOnlyModeParsesNothing()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});
  reader.setOperationMode(SerialStudio::ConsoleOnly);

  reader.processData(capture(QByteArray("one\ntwo\n")));

  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.bytesReceived(), quint64(0));
  QCOMPARE(reader.framesExtracted(), quint64(0));
}

//--------------------------------------------------------------------------------------------------
// Checksum validation
//--------------------------------------------------------------------------------------------------

void TstFrameDelimiters::validChecksumIsAccepted()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::EndDelimiterOnly, {}, {QByteArray("\n")}, QStringLiteral("CRC-16"));

  reader.processData(capture(QByteArray("abc\n") + QByteArray::fromHex("514a")));

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("abc")}));
  QCOMPARE(reader.framesExtracted(), quint64(1));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

/**
 * @brief A mismatching checksum consumes the frame and its trailer without queueing anything, and
 *        raises the error counter that the link diagnostics report.
 */
void TstFrameDelimiters::invalidChecksumDropsTheFrame()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::EndDelimiterOnly, {}, {QByteArray("\n")}, QStringLiteral("CRC-16"));

  reader.processData(capture(QByteArray("abc\n") + QByteArray::fromHex("0000")));
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.checksumErrorCount(), quint64(1));
  QCOMPARE(reader.framesExtracted(), quint64(0));

  reader.processData(capture(QByteArray("def\n") + QByteArray::fromHex("7388")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("def")}));
  QCOMPARE(reader.checksumErrorCount(), quint64(1));
}

/**
 * @brief A frame whose checksum bytes have not arrived yet is neither accepted nor discarded: the
 *        scan stops and resumes once the trailer lands.
 */
void TstFrameDelimiters::incompleteChecksumWaitsForMoreBytes()
{
  IO::FrameReader reader;
  configureProject(
    reader, SerialStudio::EndDelimiterOnly, {}, {QByteArray("\n")}, QStringLiteral("CRC-16"));

  reader.processData(capture(QByteArray("abc\n") + QByteArray::fromHex("51")));
  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
  QCOMPARE(reader.framesExtracted(), quint64(0));

  reader.processData(capture(QByteArray::fromHex("4a")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("abc")}));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
}

//--------------------------------------------------------------------------------------------------
// Timing and diagnostics
//--------------------------------------------------------------------------------------------------

/**
 * @brief The source owns time: the first frame in a chunk keeps the driver's stamp verbatim, and
 *        the following frames are spaced by the chunk's frameStep instead of being re-stamped.
 */
void TstFrameDelimiters::timestampsAdvanceByFrameStep()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  const auto step = std::chrono::nanoseconds(1000);
  const auto base = SteadyClock::now() - std::chrono::seconds(5);
  reader.processData(capture(QByteArray("a\nb\nc\n"), base, step));

  const auto captures = drainCaptures(reader);
  QCOMPARE(captures.size(), qsizetype(3));
  QVERIFY(captures[0]->timestamp == base);
  QCOMPARE(std::chrono::duration_cast<std::chrono::nanoseconds>(captures[1]->timestamp - base),
           step);
  QCOMPARE(std::chrono::duration_cast<std::chrono::nanoseconds>(captures[2]->timestamp - base),
           2 * step);
}

/**
 * @brief A chunk larger than the scan buffer counts the bytes it could not hold. The ring encodes
 *        "full" as "empty", so the whole chunk is lost, and the reader keeps working afterwards.
 */
void TstFrameDelimiters::bufferOverflowIsCountedAndDropped()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  const qsizetype capacity = 1024 * 1024;
  reader.processData(capture(QByteArray(capacity + 100, 'A')));

  QVERIFY(drainFrames(reader).isEmpty());
  QCOMPARE(reader.overflowBytes(), quint64(100));
  QCOMPARE(reader.overflowCount(), qsizetype(0));
  QCOMPARE(reader.bytesReceived(), quint64(capacity + 100));

  reader.processData(capture(QByteArray("tail\n")));
  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("tail")}));
}

/**
 * @brief Every extracted frame is either queued or counted as dropped, so the two counters must
 *        add up to the number of frames the scan found even once the 65536-slot queue is full.
 */
void TstFrameDelimiters::queueSaturationCountsDroppedFrames()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  constexpr int kFrames = 70000;
  reader.processData(capture(QByteArray("a\n").repeated(kFrames)));
  reader.processData(capture(QByteArray("a\n")));
  reader.processData(capture(QByteArray("a\n")));

  const quint64 total = quint64(kFrames) + 2;
  QVERIFY(reader.droppedFrameCount() > 0);
  QVERIFY(reader.framesExtracted() >= 65536);
  QCOMPARE(reader.framesExtracted() + reader.droppedFrameCount(), total);
}

void TstFrameDelimiters::diagnosticCountersReset()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  reader.processData(capture(QByteArray("one\ntwo\n")));
  QVERIFY(reader.bytesReceived() > 0);
  QVERIFY(reader.framesExtracted() > 0);

  reader.resetDiagnosticCounters();

  QCOMPARE(reader.bytesReceived(), quint64(0));
  QCOMPARE(reader.framesExtracted(), quint64(0));
  QCOMPARE(reader.droppedFrameCount(), quint64(0));
  QCOMPARE(reader.checksumErrorCount(), quint64(0));
  QCOMPARE(reader.overflowBytes(), quint64(0));

  reader.resetDroppedFrameCount();
  reader.resetOverflowCount();
  QCOMPARE(reader.droppedFrameCount(), quint64(0));
  QCOMPARE(reader.overflowCount(), qsizetype(0));
}

/**
 * @brief readyRead() is the consumer's only wake-up, so it must not fire when the scan produced
 *        nothing and the queue is already empty.
 */
void TstFrameDelimiters::readyReadTracksTheQueue()
{
  IO::FrameReader reader;
  configureQuickPlot(reader, {QByteArray("\n")});

  QSignalSpy spy(&reader, &IO::FrameReader::readyRead);

  reader.processData(capture(QByteArray("partial")));
  QCOMPARE(spy.count(), 0);

  reader.processData(capture(QByteArray(" frame\n")));
  QCOMPARE(spy.count(), 1);

  QCOMPARE(drainFrames(reader), QList<QByteArray>({QByteArray("partial frame")}));

  reader.processData(capture(QByteArray("still partial")));
  QCOMPARE(spy.count(), 1);
}

QTEST_GUILESS_MAIN(TstFrameDelimiters)

#include "tst_frame_delimiters.moc"
