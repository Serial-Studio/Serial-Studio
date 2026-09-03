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

#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

#include "AI/SseEventReader.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Drives the SSE parser with the chunk shapes a real provider stream produces: frames
 *        split at every byte boundary, CRLF pairs straddling two packets, multi-line data
 *        payloads, and the two overflow reasons a reply must end its turn on (spec 0075, M10).
 */
class TstSseEventReader : public QObject {
  Q_OBJECT

private slots:
  void singleFrameCarriesEventAndPayload();
  void frameSplitAcrossChunksIsHeldUntilComplete();
  void crlfPairSplitAcrossChunksIsNotABlankLine();
  void doneSentinelInsideFrameIsNotAPayload();
  void multipleDataLinesConcatenate();
  void invalidJsonReportsRecoverableParseError();
  void oversizedPayloadIsFatal();
  void bufferOverflowResetsAndIsFatal();
  void pingFramesAreDropped();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the event name of one recorded frameReceived emission.
 */
static QString frameName(const QSignalSpy& spy, int index)
{
  return spy.at(index).at(0).toString();
}

/**
 * @brief Returns the decoded payload of one recorded frameReceived emission.
 */
static QJsonObject framePayload(const QSignalSpy& spy, int index)
{
  return spy.at(index).at(1).toJsonObject();
}

//--------------------------------------------------------------------------------------------------
// Frame extraction
//--------------------------------------------------------------------------------------------------

/**
 * @brief A complete frame publishes its event name and parsed object.
 */
void TstSseEventReader::singleFrameCarriesEventAndPayload()
{
  AI::SseEventReader reader;
  QSignalSpy frames(&reader, &AI::SseEventReader::frameReceived);

  reader.feed(QByteArrayLiteral("event: message_start\ndata: {\"a\":1}\n\n"));

  QCOMPARE(frames.count(), 1);
  QCOMPARE(frameName(frames, 0), QStringLiteral("message_start"));
  QCOMPARE(framePayload(frames, 0).value(QStringLiteral("a")).toInt(), 1);
  QCOMPARE(reader.bufferedBytes(), 0);
}

/**
 * @brief A frame arriving one byte at a time publishes exactly once, at the terminator.
 */
void TstSseEventReader::frameSplitAcrossChunksIsHeldUntilComplete()
{
  AI::SseEventReader reader;
  QSignalSpy frames(&reader, &AI::SseEventReader::frameReceived);

  const QByteArray stream = QByteArrayLiteral("event: delta\ndata: {\"n\":7}\n\n");
  for (int i = 0; i < stream.size() - 1; ++i)
    reader.feed(stream.mid(i, 1));

  QCOMPARE(frames.count(), 0);

  reader.feed(stream.right(1));
  QCOMPARE(frames.count(), 1);
  QCOMPARE(framePayload(frames, 0).value(QStringLiteral("n")).toInt(), 7);
}

/**
 * @brief A CRLF pair split across two chunks is one line ending, not a frame terminator.
 */
void TstSseEventReader::crlfPairSplitAcrossChunksIsNotABlankLine()
{
  AI::SseEventReader reader;
  QSignalSpy frames(&reader, &AI::SseEventReader::frameReceived);

  reader.feed(QByteArrayLiteral("data: {\"v\":1}\r\n\r"));
  QCOMPARE(frames.count(), 0);

  reader.feed(QByteArrayLiteral("\ndata: {\"v\":2}\r\n\r\n"));
  QCOMPARE(frames.count(), 2);
  QCOMPARE(framePayload(frames, 0).value(QStringLiteral("v")).toInt(), 1);
  QCOMPARE(framePayload(frames, 1).value(QStringLiteral("v")).toInt(), 2);
}

/**
 * @brief The [DONE] sentinel is dropped rather than reported as a malformed payload.
 */
void TstSseEventReader::doneSentinelInsideFrameIsNotAPayload()
{
  AI::SseEventReader reader;
  QSignalSpy frames(&reader, &AI::SseEventReader::frameReceived);
  QSignalSpy errors(&reader, &AI::SseEventReader::parseError);

  reader.feed(QByteArrayLiteral("data: [DONE]\n\n"));

  QCOMPARE(frames.count(), 0);
  QCOMPARE(errors.count(), 0);
}

/**
 * @brief Multi-line data fields concatenate with newlines before the JSON parse.
 */
void TstSseEventReader::multipleDataLinesConcatenate()
{
  AI::SseEventReader reader;
  QSignalSpy frames(&reader, &AI::SseEventReader::frameReceived);

  reader.feed(QByteArrayLiteral("data: {\"a\":\ndata: 5}\n\n"));

  QCOMPARE(frames.count(), 1);
  QCOMPARE(framePayload(frames, 0).value(QStringLiteral("a")).toInt(), 5);
}

//--------------------------------------------------------------------------------------------------
// Error classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief A malformed frame is recoverable: the reader reports it and keeps parsing.
 */
void TstSseEventReader::invalidJsonReportsRecoverableParseError()
{
  AI::SseEventReader reader;
  QSignalSpy frames(&reader, &AI::SseEventReader::frameReceived);
  QSignalSpy errors(&reader, &AI::SseEventReader::parseError);

  reader.feed(QByteArrayLiteral("data: {not json}\n\ndata: {\"ok\":1}\n\n"));

  QCOMPARE(errors.count(), 1);
  QVERIFY(!AI::SseEventReader::fatalReason(errors.at(0).at(0).toString()));
  QCOMPARE(frames.count(), 1);
  QCOMPARE(framePayload(frames, 0).value(QStringLiteral("ok")).toInt(), 1);
}

/**
 * @brief A payload past the per-frame cap is fatal: the turn cannot be trusted to be complete.
 */
void TstSseEventReader::oversizedPayloadIsFatal()
{
  AI::SseEventReader reader;
  QSignalSpy errors(&reader, &AI::SseEventReader::parseError);

  QByteArray frame = QByteArrayLiteral("data: ");
  frame.append(QByteArray(AI::SseEventReader::kMaxPayloadBytes + 16, 'x'));
  frame.append("\n\n");
  reader.feed(frame);

  QCOMPARE(errors.count(), 1);
  QCOMPARE(errors.at(0).at(0).toString(),
           QString::fromLatin1(AI::SseEventReader::kErrPayloadTooLarge));
  QVERIFY(AI::SseEventReader::fatalReason(errors.at(0).at(0).toString()));
}

/**
 * @brief A frame that would push the buffer past its cap drops the stream state and is fatal.
 */
void TstSseEventReader::bufferOverflowResetsAndIsFatal()
{
  AI::SseEventReader reader;
  QSignalSpy errors(&reader, &AI::SseEventReader::parseError);

  reader.feed(QByteArray(AI::SseEventReader::kMaxBufferBytes - 8, 'x'));
  QCOMPARE(errors.count(), 0);
  QVERIFY(reader.bufferedBytes() > 0);

  reader.feed(QByteArray(64, 'y'));
  QCOMPARE(errors.count(), 1);
  QCOMPARE(errors.at(0).at(0).toString(),
           QString::fromLatin1(AI::SseEventReader::kErrBufferOverflow));
  QVERIFY(AI::SseEventReader::fatalReason(errors.at(0).at(0).toString()));
  QCOMPARE(reader.bufferedBytes(), 0);
}

/**
 * @brief Keep-alive frames never reach the reply state machines.
 */
void TstSseEventReader::pingFramesAreDropped()
{
  AI::SseEventReader reader;
  QSignalSpy frames(&reader, &AI::SseEventReader::frameReceived);

  reader.feed(QByteArrayLiteral("event: ping\ndata: {}\n\n"));

  QCOMPARE(frames.count(), 0);
}

QTEST_APPLESS_MAIN(TstSseEventReader)

#include "tst_sse_event_reader.moc"
