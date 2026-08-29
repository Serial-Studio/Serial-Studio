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

#include <QByteArray>
#include <QList>
#include <QTest>

#include "IO/ConnectionManager/ReplyCapture.h"

// Device ids and chunks stay inside the contract the production callers honour (id >= 0, non-empty
// chunk): a debug build's SS_ASSERT aborts, so an out-of-contract row would test the assert, not
// the capture. Every test function builds its own ReplyCapture and carries no state to the next.

/**
 * @brief Feeds @p chunks into @p capture as if they arrived as separate raw-data chunks.
 */
static void feed(IO::ReplyCapture& capture, int deviceId, const QList<QByteArray>& chunks)
{
  for (const auto& chunk : chunks)
    capture.record(deviceId, chunk);
}

/**
 * @brief Arm/record/poll/disarm semantics of IO::ReplyCapture, the transmit-reply tap a control
 *        script's deviceWriteAndWait() drives through ConnectionManager.
 */
class TstConnectionReplyCapture : public QObject {
  Q_OBJECT

private slots:
  void startsDisarmed();
  void armOpensAnEmptyBuffer();
  void recordIsIgnoredWhileDisarmed();

  void accumulatesChunks_data();
  void accumulatesChunks();

  void pollIsNonDestructive();
  void reArmClearsThePreviousBuffer();
  void unarmedDeviceIsNotCaptured();
  void sourcesAreCapturedIndependently();
  void disarmKeepsOtherSourcesArmed();
  void disarmOfUnknownDeviceKeepsState();
};

//--------------------------------------------------------------------------------------------------
// Arming
//--------------------------------------------------------------------------------------------------

/**
 * @brief A fresh capture is disarmed, which is what keeps the raw-data tap free of a lock.
 */
void TstConnectionReplyCapture::startsDisarmed()
{
  IO::ReplyCapture capture;

  QVERIFY(!capture.armed());
  QCOMPARE(capture.poll(0), QByteArray());
}

/**
 * @brief Arming publishes the flag and leaves an empty buffer behind, so a poll before any byte
 *        arrives reads empty rather than stale.
 */
void TstConnectionReplyCapture::armOpensAnEmptyBuffer()
{
  IO::ReplyCapture capture;
  capture.arm(0);

  QVERIFY(capture.armed());
  QCOMPARE(capture.poll(0), QByteArray());
}

/**
 * @brief Bytes seen before any arm are dropped: the tap must not retain traffic nobody asked for.
 */
void TstConnectionReplyCapture::recordIsIgnoredWhileDisarmed()
{
  IO::ReplyCapture capture;
  capture.record(0, QByteArrayLiteral("noise"));

  QVERIFY(!capture.armed());
  QCOMPARE(capture.poll(0), QByteArray());
}

//--------------------------------------------------------------------------------------------------
// Accumulation
//--------------------------------------------------------------------------------------------------

void TstConnectionReplyCapture::accumulatesChunks_data()
{
  QTest::addColumn<QList<QByteArray>>("chunks");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("single chunk") << QList<QByteArray>{QByteArrayLiteral("OK\r\n")}
                                << QByteArrayLiteral("OK\r\n");
  QTest::newRow("split reply") << QList<QByteArray>{QByteArrayLiteral("OK"),
                                                    QByteArrayLiteral("\r\n")}
                               << QByteArrayLiteral("OK\r\n");
  QTest::newRow("many chunks") << QList<QByteArray>{QByteArrayLiteral("a"),
                                                    QByteArrayLiteral("b"),
                                                    QByteArrayLiteral("c"),
                                                    QByteArrayLiteral("d")}
                               << QByteArrayLiteral("abcd");
  QTest::newRow("binary with NUL")
    << QList<QByteArray>{QByteArray("\x01\x00\x02", 3), QByteArray("\x00\x03", 2)}
    << QByteArray("\x01\x00\x02\x00\x03", 5);
  QTest::newRow("large chunk") << QList<QByteArray>{QByteArray(4096, '\xAB')}
                               << QByteArray(4096, '\xAB');
}

/**
 * @brief The capture is an append-only accumulator: chunk boundaries are invisible to the script,
 *        which is what lets it poll until the reply it is waiting for is complete.
 */
void TstConnectionReplyCapture::accumulatesChunks()
{
  QFETCH(QList<QByteArray>, chunks);
  QFETCH(QByteArray, expected);

  IO::ReplyCapture capture;
  capture.arm(3);
  feed(capture, 3, chunks);

  QCOMPARE(capture.poll(3), expected);
  QVERIFY(capture.armed());
}

//--------------------------------------------------------------------------------------------------
// Polling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Polling does not consume: a script polls repeatedly until the reply satisfies it, and a
 *        draining poll would hand it one fragment per call.
 */
void TstConnectionReplyCapture::pollIsNonDestructive()
{
  IO::ReplyCapture capture;
  capture.arm(0);
  capture.record(0, QByteArrayLiteral("ping"));

  QCOMPARE(capture.poll(0), QByteArrayLiteral("ping"));
  QCOMPARE(capture.poll(0), QByteArrayLiteral("ping"));

  capture.record(0, QByteArrayLiteral("-pong"));
  QCOMPARE(capture.poll(0), QByteArrayLiteral("ping-pong"));
}

/**
 * @brief Re-arming starts a new exchange from empty, so the next request never reads the previous
 *        reply's tail.
 */
void TstConnectionReplyCapture::reArmClearsThePreviousBuffer()
{
  IO::ReplyCapture capture;
  capture.arm(0);
  capture.record(0, QByteArrayLiteral("first"));
  capture.arm(0);

  QCOMPARE(capture.poll(0), QByteArray());

  capture.record(0, QByteArrayLiteral("second"));
  QCOMPARE(capture.poll(0), QByteArrayLiteral("second"));
}

/**
 * @brief Traffic from a device nobody armed is dropped even while another device is armed: the
 *        armed flag is global, the buffers are per device.
 */
void TstConnectionReplyCapture::unarmedDeviceIsNotCaptured()
{
  IO::ReplyCapture capture;
  capture.arm(0);
  capture.record(1, QByteArrayLiteral("other source"));

  QVERIFY(capture.armed());
  QCOMPARE(capture.poll(1), QByteArray());
  QCOMPARE(capture.poll(0), QByteArray());
}

//--------------------------------------------------------------------------------------------------
// Multiple sources
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two armed sources keep separate buffers, so a multi-source project can run one exchange
 *        per link without either seeing the other's bytes.
 */
void TstConnectionReplyCapture::sourcesAreCapturedIndependently()
{
  IO::ReplyCapture capture;
  capture.arm(0);
  capture.arm(2);

  feed(capture, 0, {QByteArrayLiteral("aa"), QByteArrayLiteral("bb")});
  feed(capture, 2, {QByteArrayLiteral("zz")});

  QCOMPARE(capture.poll(0), QByteArrayLiteral("aabb"));
  QCOMPARE(capture.poll(2), QByteArrayLiteral("zz"));
}

//--------------------------------------------------------------------------------------------------
// Disarming
//--------------------------------------------------------------------------------------------------

/**
 * @brief Disarming one source releases only its buffer; the tap stays armed while another source
 *        is still waiting, and only the last disarm turns it off.
 */
void TstConnectionReplyCapture::disarmKeepsOtherSourcesArmed()
{
  IO::ReplyCapture capture;
  capture.arm(0);
  capture.arm(2);
  capture.record(0, QByteArrayLiteral("aa"));
  capture.record(2, QByteArrayLiteral("zz"));

  capture.disarm(0);
  QVERIFY(capture.armed());
  QCOMPARE(capture.poll(0), QByteArray());
  QCOMPARE(capture.poll(2), QByteArrayLiteral("zz"));

  capture.record(0, QByteArrayLiteral("late"));
  QCOMPARE(capture.poll(0), QByteArray());

  capture.disarm(2);
  QVERIFY(!capture.armed());
  QCOMPARE(capture.poll(2), QByteArray());
}

/**
 * @brief Disarming a device that was never armed is a no-op, so a script's cleanup path can run
 *        unconditionally without disarming somebody else's capture.
 */
void TstConnectionReplyCapture::disarmOfUnknownDeviceKeepsState()
{
  IO::ReplyCapture capture;
  capture.arm(0);
  capture.record(0, QByteArrayLiteral("kept"));

  capture.disarm(7);

  QVERIFY(capture.armed());
  QCOMPARE(capture.poll(0), QByteArrayLiteral("kept"));
}

QTEST_APPLESS_MAIN(TstConnectionReplyCapture)

#include "tst_connection_reply_capture.moc"
