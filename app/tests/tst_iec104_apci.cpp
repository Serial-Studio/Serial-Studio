/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <QTest>

#include "Protocols/Iec104/Apci.h"

using namespace IO::Drivers::Iec104Proto;

/**
 * @brief Builds a U-format APDU carrying @p function, as a peer would send it.
 */
[[nodiscard]] static QByteArray unnumbered(UFunction function)
{
  QByteArray apdu(kApciBytes, static_cast<char>(0));
  apdu[0] = static_cast<char>(kStartByte);
  apdu[1] = static_cast<char>(kMinApduLength);
  apdu[2] = static_cast<char>(function);
  return apdu;
}

/**
 * @brief Builds an S-format APDU acknowledging @p recvSeq, as a peer would send it.
 */
[[nodiscard]] static QByteArray supervisory(int recvSeq)
{
  QByteArray apdu(kApciBytes, static_cast<char>(0));
  apdu[0] = static_cast<char>(kStartByte);
  apdu[1] = static_cast<char>(kMinApduLength);
  apdu[2] = static_cast<char>(0x01);
  apdu[4] = static_cast<char>((recvSeq << 1) & 0xFE);
  apdu[5] = static_cast<char>((recvSeq >> 7) & 0xFF);
  return apdu;
}

/**
 * @brief Builds an I-format APDU carrying @p payload, as a peer would send it.
 */
[[nodiscard]] static QByteArray information(int sendSeq, int recvSeq, const QByteArray& payload)
{
  QByteArray apdu(kApciBytes, static_cast<char>(0));
  apdu[0] = static_cast<char>(kStartByte);
  apdu[1] = static_cast<char>(kMinApduLength + payload.size());
  apdu[2] = static_cast<char>((sendSeq << 1) & 0xFE);
  apdu[3] = static_cast<char>((sendSeq >> 7) & 0xFF);
  apdu[4] = static_cast<char>((recvSeq << 1) & 0xFE);
  apdu[5] = static_cast<char>((recvSeq >> 7) & 0xFF);
  apdu.append(payload);
  return apdu;
}

/**
 * @brief Pins the APCI transport layer of the IEC 60870-5-104 client: the frame formats, the
 *        15-bit sequence counters, the k/w windows and the t1/t2/t3 deadlines. Every deadline is
 *        driven by an explicit millisecond argument, so the whole timing contract is exercised
 *        here against a fake clock with no timer, no socket and no event loop.
 */
class TstIec104Apci : public QObject {
  Q_OBJECT

private slots:
  void startDtHandshakeCompletes();
  void testFrameKeepsTheLinkAlive();
  void informationFramesAdvanceTheSendCounter();
  void receivedFramesAdvanceTheReceiveCounter();
  void outOfOrderSendSequenceIsRefused();
  void acknowledgementBeyondWhatWasSentIsRefused();
  void sendWindowClosesAtK();
  void supervisoryAcknowledgementReopensTheWindow();
  void acknowledgementIsDueAtWindowW();
  void acknowledgementIsDueAtT2();
  void testIsDueAtT3();
  void unconfirmedActivationExpiresAtT1();
  void unacknowledgedInformationExpiresAtT1();
  void sequenceCountersWrapAtFifteenBits();
  void partialFramesAskForMoreData();
  void malformedFramesAreCountedAndRefused_data();
  void malformedFramesAreCountedAndRefused();
  void configurationIsClampedToLegalRanges();
};

//--------------------------------------------------------------------------------------------------
// Handshake
//--------------------------------------------------------------------------------------------------

/**
 * @brief STARTDT act goes out as a well-formed U frame and its confirmation clears the deadline.
 */
void TstIec104Apci::startDtHandshakeCompletes()
{
  Connection link;
  link.reset(0);

  const auto act = link.encodeUnnumbered(UFunction::StartDtAct, 0);
  QCOMPARE(act, unnumbered(UFunction::StartDtAct));
  QCOMPARE(act.size(), qsizetype(kApciBytes));
  QVERIFY(link.confirmOverdue(kDefaultT1Ms));

  Apdu apdu;
  const auto con = unnumbered(UFunction::StartDtCon);
  QCOMPARE(link.consume(con, apdu, 100), ParseResult::Ok);
  QCOMPARE(apdu.type, FrameType::Unnumbered);
  QCOMPARE(apdu.function, UFunction::StartDtCon);
  QCOMPARE(apdu.apduSize, qsizetype(kApciBytes));
  QVERIFY(!link.confirmOverdue(100 + kDefaultT1Ms));
}

/**
 * @brief A TESTFR activation arms t1 and its confirmation disarms it; the pairing table agrees.
 */
void TstIec104Apci::testFrameKeepsTheLinkAlive()
{
  Connection link;
  link.reset(0);

  QCOMPARE(Connection::confirmationFor(UFunction::TestFrAct), UFunction::TestFrCon);
  QCOMPARE(Connection::confirmationFor(UFunction::StopDtAct), UFunction::StopDtCon);
  QCOMPARE(Connection::confirmationFor(UFunction::TestFrCon), UFunction::None);
  QVERIFY(Connection::isConfirmation(UFunction::StartDtCon));
  QVERIFY(!Connection::isConfirmation(UFunction::StartDtAct));

  (void)link.encodeUnnumbered(UFunction::TestFrAct, 1000);
  QVERIFY(link.confirmOverdue(1000 + kDefaultT1Ms));

  Apdu apdu;
  const auto con = unnumbered(UFunction::TestFrCon);
  QCOMPARE(link.consume(con, apdu, 1200), ParseResult::Ok);
  QVERIFY(!link.confirmOverdue(1200 + kDefaultT1Ms));
  QCOMPARE(link.framesSent(), quint64(1));
  QCOMPARE(link.framesReceived(), quint64(1));
}

//--------------------------------------------------------------------------------------------------
// Sequence numbers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every I frame consumes one send sequence number and carries the current receive count.
 */
void TstIec104Apci::informationFramesAdvanceTheSendCounter()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01\x02\x03", 3);
  for (int i = 0; i < 3; ++i) {
    const auto frame = link.encodeInformation(payload, 0);
    QCOMPARE(frame, information(i, 0, payload));
    QCOMPARE(link.sendSeq(), i + 1);
    QCOMPARE(link.outstanding(), i + 1);
  }

  QCOMPARE(link.ackedSeq(), 0);
}

/**
 * @brief A received I frame advances the receive counter and exposes its payload by offset.
 */
void TstIec104Apci::receivedFramesAdvanceTheReceiveCounter()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x0a\x0b", 2);
  const auto frame = information(0, 0, payload);

  Apdu apdu;
  QCOMPARE(link.consume(frame, apdu, 10), ParseResult::Ok);
  QCOMPARE(apdu.type, FrameType::Information);
  QCOMPARE(apdu.sendSeq, 0);
  QCOMPARE(apdu.asduOffset, qsizetype(kApciBytes));
  QCOMPARE(apdu.asduSize, qsizetype(payload.size()));
  QCOMPARE(frame.mid(apdu.asduOffset, apdu.asduSize), payload);
  QCOMPARE(link.recvSeq(), 1);
  QCOMPARE(link.unackedReceived(), 1);
}

/**
 * @brief A send sequence that skips a frame is a protocol error, counted and refused.
 */
void TstIec104Apci::outOfOrderSendSequenceIsRefused()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01", 1);
  const auto skipped = information(4, 0, payload);

  Apdu apdu;
  QCOMPARE(link.consume(skipped, apdu, 10), ParseResult::OutOfOrder);
  QCOMPARE(link.sequenceErrors(), quint64(1));
  QCOMPARE(link.recvSeq(), 0);
  QCOMPARE(apdu.apduSize, qsizetype(kApciBytes + 1));
}

/**
 * @brief An acknowledgement for frames this side never sent is refused rather than honoured.
 */
void TstIec104Apci::acknowledgementBeyondWhatWasSentIsRefused()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01", 1);
  (void)link.encodeInformation(payload, 0);

  Apdu apdu;
  const auto ahead = supervisory(5);
  QCOMPARE(link.consume(ahead, apdu, 10), ParseResult::OutOfOrder);
  QCOMPARE(link.sequenceErrors(), quint64(1));
  QCOMPARE(link.ackedSeq(), 0);
  QCOMPARE(link.outstanding(), 1);
}

/**
 * @brief The 15-bit counters wrap at 32768 rather than growing without bound.
 */
void TstIec104Apci::sequenceCountersWrapAtFifteenBits()
{
  Connection link;
  link.reset(0);

  Apdu apdu;
  const QByteArray payload("\x01", 1);
  for (int i = 0; i < kSequenceModulo; ++i) {
    QVERIFY(!link.encodeInformation(payload, 0).isEmpty());
    const auto ack = supervisory((i + 1) % kSequenceModulo);
    QCOMPARE(link.consume(ack, apdu, 0), ParseResult::Ok);
  }

  QCOMPARE(link.sendSeq(), 0);
  QCOMPARE(link.ackedSeq(), 0);
  QCOMPARE(link.outstanding(), 0);

  for (int i = 0; i < kSequenceModulo; ++i) {
    const auto frame = information(i, 0, payload);
    QCOMPARE(link.consume(frame, apdu, 0), ParseResult::Ok);
    (void)link.encodeSupervisory(0);
  }

  QCOMPARE(link.recvSeq(), 0);
  QCOMPARE(link.unackedReceived(), 0);
}

//--------------------------------------------------------------------------------------------------
// Windows
//--------------------------------------------------------------------------------------------------

/**
 * @brief The k-th unacknowledged I frame closes the send window; the caller must queue, not send.
 */
void TstIec104Apci::sendWindowClosesAtK()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01", 1);
  for (int i = 0; i < kDefaultK; ++i) {
    QVERIFY(link.sendWindowOpen());
    QVERIFY(!link.encodeInformation(payload, 0).isEmpty());
  }

  QVERIFY(!link.sendWindowOpen());
  QVERIFY(link.encodeInformation(payload, 0).isEmpty());
  QCOMPARE(link.outstanding(), kDefaultK);
}

/**
 * @brief A supervisory frame from the peer retires the outstanding frames and reopens the window.
 */
void TstIec104Apci::supervisoryAcknowledgementReopensTheWindow()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01", 1);
  for (int i = 0; i < kDefaultK; ++i)
    (void)link.encodeInformation(payload, 0);

  Apdu apdu;
  const auto ack = supervisory(kDefaultK);
  QCOMPARE(link.consume(ack, apdu, 50), ParseResult::Ok);
  QCOMPARE(apdu.type, FrameType::Supervisory);
  QCOMPARE(apdu.recvSeq, kDefaultK);
  QCOMPARE(link.outstanding(), 0);
  QVERIFY(link.sendWindowOpen());
  QVERIFY(!link.confirmOverdue(50 + kDefaultT1Ms));
}

/**
 * @brief w received I frames oblige this side to acknowledge, and encoding the S frame clears it.
 */
void TstIec104Apci::acknowledgementIsDueAtWindowW()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01", 1);
  Apdu apdu;
  for (int i = 0; i < kDefaultW; ++i) {
    const auto frame = information(i, 0, payload);
    QCOMPARE(link.consume(frame, apdu, 10), ParseResult::Ok);
  }

  QVERIFY(link.ackDue(10));
  QCOMPARE(link.unackedReceived(), kDefaultW);

  const auto ack = link.encodeSupervisory(20);
  QCOMPARE(ack, supervisory(kDefaultW));
  QVERIFY(!link.ackDue(20));
  QCOMPARE(link.unackedReceived(), 0);
}

/**
 * @brief One unacknowledged frame becomes due once t2 has elapsed, well before w is reached.
 */
void TstIec104Apci::acknowledgementIsDueAtT2()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01", 1);
  const auto frame = information(0, 0, payload);

  Apdu apdu;
  QCOMPARE(link.consume(frame, apdu, 1000), ParseResult::Ok);
  QVERIFY(!link.ackDue(1000 + kDefaultT2Ms - 1));
  QVERIFY(link.ackDue(1000 + kDefaultT2Ms));
}

//--------------------------------------------------------------------------------------------------
// Deadlines
//--------------------------------------------------------------------------------------------------

/**
 * @brief An idle link asks for a test frame at t3, and never stacks a second one on an unanswered
 *        activation: that would hide the very timeout the test frame exists to expose.
 */
void TstIec104Apci::testIsDueAtT3()
{
  Connection link;
  link.reset(0);

  QVERIFY(!link.testDue(kDefaultT3Ms - 1));
  QVERIFY(link.testDue(kDefaultT3Ms));

  (void)link.encodeUnnumbered(UFunction::TestFrAct, kDefaultT3Ms);
  QVERIFY(!link.testDue(kDefaultT3Ms * 3));

  Apdu apdu;
  const auto con = unnumbered(UFunction::TestFrCon);
  QCOMPARE(link.consume(con, apdu, kDefaultT3Ms + 10), ParseResult::Ok);
  QVERIFY(!link.testDue(kDefaultT3Ms + 20));
  QVERIFY(link.testDue(kDefaultT3Ms * 2 + 10));
}

/**
 * @brief An activation nobody confirms trips t1, which is what declares the link dead.
 */
void TstIec104Apci::unconfirmedActivationExpiresAtT1()
{
  Connection link;
  link.reset(0);

  QVERIFY(!link.confirmOverdue(kDefaultT1Ms));

  (void)link.encodeUnnumbered(UFunction::StartDtAct, 500);
  QVERIFY(!link.confirmOverdue(500 + kDefaultT1Ms - 1));
  QVERIFY(link.confirmOverdue(500 + kDefaultT1Ms));
}

/**
 * @brief An I frame nobody acknowledges trips t1 too, and its acknowledgement disarms it.
 */
void TstIec104Apci::unacknowledgedInformationExpiresAtT1()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01", 1);
  (void)link.encodeInformation(payload, 300);
  QVERIFY(!link.confirmOverdue(300 + kDefaultT1Ms - 1));
  QVERIFY(link.confirmOverdue(300 + kDefaultT1Ms));

  Apdu apdu;
  const auto ack = supervisory(1);
  QCOMPARE(link.consume(ack, apdu, 400), ParseResult::Ok);
  QVERIFY(!link.confirmOverdue(400 + kDefaultT1Ms));
}

//--------------------------------------------------------------------------------------------------
// Hostile input
//--------------------------------------------------------------------------------------------------

/**
 * @brief A partially arrived APDU asks for more bytes and leaves every counter untouched.
 */
void TstIec104Apci::partialFramesAskForMoreData()
{
  Connection link;
  link.reset(0);

  const QByteArray payload("\x01\x02\x03\x04", 4);
  const auto frame = information(0, 0, payload);

  Apdu apdu;
  for (qsizetype cut = 0; cut < frame.size(); ++cut) {
    const auto partial = frame.left(cut);
    QCOMPARE(link.consume(partial, apdu, 0), ParseResult::NeedMore);
  }

  QCOMPARE(link.framesReceived(), quint64(0));
  QCOMPARE(link.malformedFrames(), quint64(0));
  QCOMPARE(link.consume(frame, apdu, 0), ParseResult::Ok);
}

/**
 * @brief Hostile and impossible APDUs are refused and counted, never decoded on a guess.
 */
void TstIec104Apci::malformedFramesAreCountedAndRefused_data()
{
  QTest::addColumn<QByteArray>("frame");

  QTest::newRow("bad start byte") << QByteArray("\x69\x04\x07\x00\x00\x00", 6);
  QTest::newRow("length below minimum") << QByteArray("\x68\x03\x07\x00\x00\x00", 6);
  QTest::newRow("length above maximum") << QByteArray("\x68\xfe\x07\x00\x00\x00", 6);
  QTest::newRow("unknown U function") << QByteArray("\x68\x04\x0f\x00\x00\x00", 6);
  QTest::newRow("reserved U function") << QByteArray("\x68\x04\xc3\x00\x00\x00", 6);
  QTest::newRow("supervisory with payload") << QByteArray("\x68\x05\x01\x00\x00\x00\xaa", 7);
  QTest::newRow("unnumbered with payload") << QByteArray("\x68\x05\x07\x00\x00\x00\xaa", 7);
}

/**
 * @brief Every hostile frame reports Malformed, bumps the counter and moves no sequence number.
 */
void TstIec104Apci::malformedFramesAreCountedAndRefused()
{
  QFETCH(QByteArray, frame);

  Connection link;
  link.reset(0);

  Apdu apdu;
  QCOMPARE(link.consume(frame, apdu, 0), ParseResult::Malformed);
  QCOMPARE(link.malformedFrames(), quint64(1));
  QCOMPARE(link.recvSeq(), 0);
  QCOMPARE(link.sendSeq(), 0);
  QCOMPARE(link.sequenceErrors(), quint64(0));
}

/**
 * @brief Out-of-range windows and deadlines are clamped, so no configuration can close the send
 *        window permanently or ask for an acknowledgement later than the timeout that kills it.
 */
void TstIec104Apci::configurationIsClampedToLegalRanges()
{
  Connection link;

  link.configure(0, 0, 0, 0, 0);
  QCOMPARE(link.windowK(), kMinWindow);
  QCOMPARE(link.windowW(), kMinWindow);
  QCOMPARE(link.t1Ms(), kMinTimeMs);
  QCOMPARE(link.t2Ms(), kMinTimeMs);
  QCOMPARE(link.t3Ms(), kMinTimeMs);

  link.configure(999999, 999999, 999999, 999999, 999999);
  QCOMPARE(link.windowK(), kMaxWindow);
  QCOMPARE(link.windowW(), kMaxWindow);
  QCOMPARE(link.t1Ms(), kMaxTimeMs);
  QCOMPARE(link.t2Ms(), kMaxTimeMs);
  QCOMPARE(link.t3Ms(), kMaxTimeMs);

  link.configure(4, 12, 10000, 30000, 20000);
  QCOMPARE(link.windowW(), 4);
  QCOMPARE(link.t2Ms(), 10000);
}

QTEST_APPLESS_MAIN(TstIec104Apci)

#include "tst_iec104_apci.moc"
