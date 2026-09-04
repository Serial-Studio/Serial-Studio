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

#include "Protocols/S7/IsoTsap.h"

using namespace IO::Drivers::S7Comm;

/**
 * @brief Builds a byte array out of an initializer list, which is how every golden packet in this
 *        suite is spelled.
 */
[[nodiscard]] static QByteArray bytes(std::initializer_list<int> values)
{
  QByteArray out;
  out.reserve(static_cast<qsizetype>(values.size()));
  for (const int value : values)
    out.append(static_cast<char>(value & 0xFF));

  return out;
}

/**
 * @brief Wraps a TPDU in a TPKT header, as a peer would send it.
 */
[[nodiscard]] static QByteArray packet(const QByteArray& tpdu)
{
  const int total = static_cast<int>(tpdu.size()) + kTpktHeaderBytes;

  QByteArray out;
  out.append(static_cast<char>(kTpktVersion));
  out.append(static_cast<char>(0));
  out.append(static_cast<char>((total >> 8) & 0xFF));
  out.append(static_cast<char>(total & 0xFF));
  out.append(tpdu);
  return out;
}

/**
 * @brief Builds a connect confirm carrying the given TPDU size code, as a controller answers one.
 */
[[nodiscard]] static QByteArray confirm(int sizeCode)
{
  return bytes({0x0D,
                kTpduConnectConfirm,
                0x00,
                0x01,
                0x00,
                0x02,
                0x00,
                kParamTpduSize,
                0x01,
                sizeCode,
                kParamCalledTsap,
                0x02,
                0x03,
                0x01});
}

/**
 * @brief Builds a data TPDU carrying @p payload, with the end-of-transmission flag set when
 *        @p last is true.
 */
[[nodiscard]] static QByteArray dataTpdu(const QByteArray& payload, bool last)
{
  QByteArray tpdu;
  tpdu.append(static_cast<char>(kDataTpduBytes - 1));
  tpdu.append(static_cast<char>(kTpduData));
  tpdu.append(static_cast<char>(last ? kEndOfTransmission : 0x00));
  tpdu.append(payload);
  return tpdu;
}

/**
 * @brief Pins the ISO-on-TCP transport of the S7comm client: the TPKT framing an incremental
 *        reader has to survive, the class-0 connect handshake that carries the rack and slot, and
 *        the data TPDUs an S7 message is reassembled out of. No socket and no event loop: the
 *        receive buffer is fed by hand, one arrival at a time, exactly as a stream delivers it.
 */
class TstS7CommIsoTsap : public QObject {
  Q_OBJECT

private slots:
  void connectRequestCarriesTheRackAndSlot();
  void connectRequestSpellsEveryRackSlotPair_data();
  void connectRequestSpellsEveryRackSlotPair();
  void connectConfirmRecordsTheNegotiatedTpduSize();
  void connectConfirmClampsAnImpossibleSizeCode_data();
  void connectConfirmClampsAnImpossibleSizeCode();
  void connectConfirmWithARunawayParameterIsRefused();
  void dataTpduWrapsThePayload();
  void tpktExtractionLocatesOneCompletePacket();
  void partialPacketsAskForMoreData();
  void malformedPacketsAreCountedAndRefused_data();
  void malformedPacketsAreCountedAndRefused();
  void dataIsReassembledUntilEndOfTransmission();
  void anOverlongAssemblyIsRefusedAndRolledBack();
  void aTpduThatIsNotDataIsRefused();
};

//--------------------------------------------------------------------------------------------------
// Connection handshake
//--------------------------------------------------------------------------------------------------

/**
 * @brief The connect request goes out as the golden 22-octet packet: a TPKT header, the class-0
 *        connect TPDU, and the three parameters that name the TPDU size and both TSAPs.
 */
void TstS7CommIsoTsap::connectRequestCarriesTheRackAndSlot()
{
  Transport transport;

  const auto request = transport.buildConnectRequest(0, 1);
  QCOMPARE(request, bytes({0x03, 0x00, 0x00, 0x16, 0x11, 0xE0, 0x00, 0x00, 0x00, 0x01, 0x00,
                           0xC0, 0x01, 0x0A, 0xC1, 0x02, 0x01, 0x00, 0xC2, 0x02, 0x03, 0x01}));
  QCOMPARE(request.size(), qsizetype(kTpktHeaderBytes + kConnectTpduBytes));
  QCOMPARE(transport.framesSent(), quint64(1));
}

/**
 * @brief The called TSAP's low octet is rack * 0x20 + slot, which is the one field that decides
 *        whether a reachable controller answers at all.
 */
void TstS7CommIsoTsap::connectRequestSpellsEveryRackSlotPair_data()
{
  QTest::addColumn<int>("rack");
  QTest::addColumn<int>("slot");
  QTest::addColumn<int>("expected");

  QTest::newRow("s7-1200 rack 0 slot 1") << 0 << 1 << 0x01;
  QTest::newRow("s7-300 rack 0 slot 2") << 0 << 2 << 0x02;
  QTest::newRow("rack 1 slot 0") << 1 << 0 << 0x20;
  QTest::newRow("rack 2 slot 3") << 2 << 3 << 0x43;
  QTest::newRow("highest pair") << 7 << 31 << 0xFF;
}

/**
 * @brief Drives the table above through the encoder itself, not only through the helper.
 */
void TstS7CommIsoTsap::connectRequestSpellsEveryRackSlotPair()
{
  QFETCH(int, rack);
  QFETCH(int, slot);
  QFETCH(int, expected);

  Transport transport;
  const auto request = transport.buildConnectRequest(rack, slot);
  QCOMPARE(request.size(), qsizetype(kTpktHeaderBytes + kConnectTpduBytes));
  QCOMPARE(static_cast<std::uint8_t>(request.at(request.size() - 1)),
           static_cast<std::uint8_t>(expected));
  QCOMPARE(Transport::calledTsapLow(rack, slot), static_cast<std::uint8_t>(expected));
}

/**
 * @brief A confirm's TPDU-size parameter is what the peer settled on, and it is recorded rather
 *        than assumed: the request only ever proposes a size.
 */
void TstS7CommIsoTsap::connectConfirmRecordsTheNegotiatedTpduSize()
{
  Transport transport;
  QCOMPARE(transport.tpduBytes(), 1024);

  QVERIFY(transport.parseConnectConfirm(confirm(0x0B)));
  QCOMPARE(transport.tpduBytes(), 2048);
  QCOMPARE(transport.malformedFrames(), quint64(0));

  transport.reset();
  QCOMPARE(transport.tpduBytes(), 1024);
}

/**
 * @brief The size parameter is a SHIFT DISTANCE. One taken off the wire unclamped is undefined
 *        behaviour, so both ends of the range are pinned here.
 */
void TstS7CommIsoTsap::connectConfirmClampsAnImpossibleSizeCode_data()
{
  QTest::addColumn<int>("code");
  QTest::addColumn<int>("expected");

  QTest::newRow("zero") << 0x00 << kMinTpduBytes;
  QTest::newRow("below the range") << 0x03 << kMinTpduBytes;
  QTest::newRow("smallest legal") << 0x07 << 128;
  QTest::newRow("default") << 0x0A << 1024;
  QTest::newRow("largest legal") << 0x0D << 8192;
  QTest::newRow("above the range") << 0x20 << kMaxTpduBytes;
  QTest::newRow("all ones") << 0xFF << kMaxTpduBytes;
}

/**
 * @brief Drives the clamping table.
 */
void TstS7CommIsoTsap::connectConfirmClampsAnImpossibleSizeCode()
{
  QFETCH(int, code);
  QFETCH(int, expected);

  Transport transport;
  QVERIFY(transport.parseConnectConfirm(confirm(code)));
  QCOMPARE(transport.tpduBytes(), expected);
}

/**
 * @brief A parameter whose declared length walks past the TPDU is refused and counted, because
 *        honouring it would read the octets of whatever followed in the receive buffer.
 */
void TstS7CommIsoTsap::connectConfirmWithARunawayParameterIsRefused()
{
  Transport transport;

  const auto hostile = bytes({0x0A,
                              kTpduConnectConfirm,
                              0x00,
                              0x01,
                              0x00,
                              0x02,
                              0x00,
                              kParamTpduSize,
                              0x40,
                              0x0A,
                              0x00,
                              0x00});
  QVERIFY(!transport.parseConnectConfirm(hostile));
  QCOMPARE(transport.malformedFrames(), quint64(1));

  const auto wrongCode = bytes({0x06, kTpduConnectRequest, 0x00, 0x01, 0x00, 0x02, 0x00});
  QVERIFY(!transport.parseConnectConfirm(wrongCode));
  QCOMPARE(transport.malformedFrames(), quint64(2));
}

//--------------------------------------------------------------------------------------------------
// Framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief A payload goes out as one data TPDU with the end-of-transmission flag set; the client
 *        never splits a request across TPDUs.
 */
void TstS7CommIsoTsap::dataTpduWrapsThePayload()
{
  Transport transport;

  const auto wrapped = transport.wrapData(bytes({0xAA, 0xBB}));
  QCOMPARE(wrapped, bytes({0x03, 0x00, 0x00, 0x09, 0x02, 0xF0, 0x80, 0xAA, 0xBB}));
  QCOMPARE(transport.framesSent(), quint64(1));
}

/**
 * @brief Extraction reports the whole packet as the bytes to drop and addresses the TPDU inside
 *        the caller's buffer, so two packets arriving together are consumed one at a time.
 */
void TstS7CommIsoTsap::tpktExtractionLocatesOneCompletePacket()
{
  Transport transport;

  QByteArray buffer = packet(dataTpdu(bytes({0x01, 0x02}), true));
  buffer.append(packet(dataTpdu(bytes({0x03}), true)));

  Tpkt frame;
  QCOMPARE(transport.extractTpkt(buffer, frame), TpktResult::Ok);
  QCOMPARE(frame.totalBytes, qsizetype(9));
  QCOMPARE(frame.tpduOffset, qsizetype(kTpktHeaderBytes));
  QCOMPARE(frame.tpduSize, qsizetype(5));
  QCOMPARE(buffer.mid(frame.tpduOffset, frame.tpduSize), bytes({0x02, 0xF0, 0x80, 0x01, 0x02}));

  buffer.remove(0, frame.totalBytes);
  QCOMPARE(transport.extractTpkt(buffer, frame), TpktResult::Ok);
  QCOMPARE(frame.totalBytes, qsizetype(8));
  QCOMPARE(transport.framesReceived(), quint64(2));
}

/**
 * @brief Every prefix of a packet asks for more data and leaves the buffer untouched: a reader
 *        that consumed a partial arrival would resynchronise onto the middle of a message.
 */
void TstS7CommIsoTsap::partialPacketsAskForMoreData()
{
  Transport transport;
  const auto whole = packet(dataTpdu(bytes({0x01, 0x02, 0x03}), true));

  for (qsizetype size = 0; size < whole.size(); ++size) {
    Tpkt frame;
    QCOMPARE(transport.extractTpkt(whole.first(size), frame), TpktResult::NeedMore);
    QCOMPARE(frame.totalBytes, qsizetype(0));
  }

  Tpkt frame;
  QCOMPARE(transport.extractTpkt(whole, frame), TpktResult::Ok);
  QCOMPARE(transport.malformedFrames(), quint64(0));
}

/**
 * @brief The hostile shapes a stream can carry: a wrong version, a dirty reserved octet, a length
 *        that cannot hold a TPDU, and a length indicator that contradicts the packet around it.
 */
void TstS7CommIsoTsap::malformedPacketsAreCountedAndRefused_data()
{
  QTest::addColumn<QByteArray>("buffer");

  QTest::newRow("wrong version") << bytes({0x04, 0x00, 0x00, 0x07, 0x02, 0xF0, 0x80});
  QTest::newRow("dirty reserved") << bytes({0x03, 0x01, 0x00, 0x07, 0x02, 0xF0, 0x80});
  QTest::newRow("length below the minimum") << bytes({0x03, 0x00, 0x00, 0x05, 0x02, 0xF0, 0x80});
  QTest::newRow("zero length") << bytes({0x03, 0x00, 0x00, 0x00, 0x02, 0xF0, 0x80});
  QTest::newRow("indicator of zero") << bytes({0x03, 0x00, 0x00, 0x07, 0x00, 0xF0, 0x80});
  QTest::newRow("indicator past the packet") << bytes({0x03, 0x00, 0x00, 0x07, 0x20, 0xF0, 0x80});
}

/**
 * @brief Every row above is counted and refused; none of them addresses memory outside the view.
 */
void TstS7CommIsoTsap::malformedPacketsAreCountedAndRefused()
{
  QFETCH(QByteArray, buffer);

  Transport transport;
  Tpkt frame;
  QCOMPARE(transport.extractTpkt(buffer, frame), TpktResult::Malformed);
  QCOMPARE(transport.malformedFrames(), quint64(1));
  QCOMPARE(frame.totalBytes, qsizetype(0));
}

//--------------------------------------------------------------------------------------------------
// Reassembly
//--------------------------------------------------------------------------------------------------

/**
 * @brief A message split across data TPDUs is complete only when the end-of-transmission flag
 *        arrives; the parts before it accumulate and report incomplete.
 */
void TstS7CommIsoTsap::dataIsReassembledUntilEndOfTransmission()
{
  Transport transport;
  QByteArray assembly;
  bool complete = true;

  QCOMPARE(transport.acceptData(dataTpdu(bytes({0x41, 0x42}), false), assembly, complete),
           TpktResult::Ok);
  QVERIFY(!complete);
  QCOMPARE(assembly, bytes({0x41, 0x42}));

  QCOMPARE(transport.acceptData(dataTpdu(bytes({0x43}), false), assembly, complete),
           TpktResult::Ok);
  QVERIFY(!complete);

  QCOMPARE(transport.acceptData(dataTpdu(bytes({0x44}), true), assembly, complete), TpktResult::Ok);
  QVERIFY(complete);
  QCOMPARE(assembly, bytes({0x41, 0x42, 0x43, 0x44}));
  QCOMPARE(transport.malformedFrames(), quint64(0));
}

/**
 * @brief A peer that never stops sending continuations cannot grow the assembly without bound: the
 *        cap refuses the part that would cross it and leaves the assembly exactly as it was.
 */
void TstS7CommIsoTsap::anOverlongAssemblyIsRefusedAndRolledBack()
{
  Transport transport;
  QByteArray assembly(kMaxAssemblyBytes, static_cast<char>(0x5A));
  bool complete = true;

  QCOMPARE(transport.acceptData(dataTpdu(bytes({0x01}), false), assembly, complete),
           TpktResult::Malformed);
  QVERIFY(!complete);
  QCOMPARE(assembly.size(), qsizetype(kMaxAssemblyBytes));
  QCOMPARE(transport.malformedFrames(), quint64(1));
}

/**
 * @brief A TPDU that is not a data TPDU never reaches the assembly, even when its payload would
 *        decode as a plausible S7 message.
 */
void TstS7CommIsoTsap::aTpduThatIsNotDataIsRefused()
{
  Transport transport;
  QByteArray assembly;
  bool complete = true;

  QCOMPARE(transport.acceptData(confirm(0x0A), assembly, complete), TpktResult::Malformed);
  QVERIFY(assembly.isEmpty());
  QVERIFY(!complete);

  QCOMPARE(transport.acceptData(bytes({0x02}), assembly, complete), TpktResult::Malformed);
  QCOMPARE(transport.malformedFrames(), quint64(2));
}

QTEST_APPLESS_MAIN(TstS7CommIsoTsap)

#include "tst_s7comm_isotsap.moc"
