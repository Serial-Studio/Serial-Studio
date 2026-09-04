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

#include <QTest>

#include "Protocols/CAN/CanReassembly.h"

using namespace IO::Drivers;
using SteadyTimePoint = J1939TransportReassembler::SteadyTimePoint;

// Parameter-group format bytes of the two J1939 transport groups, as they sit in bits 23..16
static constexpr quint32 kCmFormat = 0xEC;
static constexpr quint32 kDtFormat = 0xEB;

// Broadcast destination address (BAM sessions are addressed to everyone)
static constexpr quint8 kGlobalAddress = 0xFF;

/**
 * @brief Builds a 29-bit J1939 identifier for a PDU1 transport frame at priority 7.
 */
static quint32 tpId(const quint32 format, const quint8 destination, const quint8 source)
{
  return (7u << 26) | (format << 16) | (static_cast<quint32>(destination) << 8) | source;
}

/**
 * @brief Builds a TP.CM packet: control byte, declared size, packet count and transported PGN.
 */
static QByteArray tpCm(const quint8 control,
                       const quint16 size,
                       const quint8 packets,
                       const quint32 pgn)
{
  QByteArray data(8, static_cast<char>(0xFF));
  data[0] = static_cast<char>(control);
  data[1] = static_cast<char>(size & 0xFF);
  data[2] = static_cast<char>((size >> 8) & 0xFF);
  data[3] = static_cast<char>(packets);
  data[5] = static_cast<char>(pgn & 0xFF);
  data[6] = static_cast<char>((pgn >> 8) & 0xFF);
  data[7] = static_cast<char>((pgn >> 16) & 0xFF);
  return data;
}

/**
 * @brief Builds a TP.DT packet carrying a 1-based sequence number and seven filler bytes whose
 *        value encodes the sequence, so a shifted or duplicated packet is visible in the result.
 */
static QByteArray tpDt(const quint8 sequence, const char fill)
{
  QByteArray data(8, fill);
  data[0] = static_cast<char>(sequence);
  return data;
}

/**
 * @brief Builds an ISO-TP FirstFrame announcing @p total bytes plus its six payload bytes.
 */
static QByteArray isoTpFirstFrame(const quint16 total, const char fill)
{
  QByteArray data(8, fill);
  data[0] = static_cast<char>(0x10 | ((total >> 8) & 0x0F));
  data[1] = static_cast<char>(total & 0xFF);
  return data;
}

/**
 * @brief Builds an ISO-TP ConsecutiveFrame with the given four-bit sequence number.
 */
static QByteArray isoTpConsecutiveFrame(const quint8 sequence, const char fill)
{
  QByteArray data(8, fill);
  data[0] = static_cast<char>(0x20 | (sequence & 0x0F));
  return data;
}

/**
 * @brief J1939 transport-protocol and ISO 15765-2 reassembly (spec 0073 T12-T14): completion,
 *        interleaving, and every discipline that must drop a whole session rather than emit a
 *        partially decoded message.
 */
class TstCanReassembly : public QObject {
  Q_OBJECT

private slots:
  void bamHappyPath();
  void rtsWithoutCtsCompletes();
  void rtsWithObservedCtsCompletes();
  void concurrentSessionsInterleave();
  void abortDropsSession();
  void sequenceGapDropsSession();
  void duplicatePacketDropsSession();
  void timeoutEvictsSession();
  void sessionCapIsCounted();
  void sizeCapIsCounted();
  void isoTpMultiFrameCompletes();
  void isoTpWrongSequenceDrops();
  void isoTpIgnoresOutOfRangeIds();
  void isoTpSingleFramePassesThrough();
  void isoTpFlowControlIsObservedOnly();
  void isoTpSessionCapIsCounted();
};

/**
 * @brief A BAM announcement plus its data packets yields the declared byte count, the transported
 *        PGN, the announcing source address, and the capture time of the FIRST packet.
 */
void TstCanReassembly::bamHappyPath()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;
  const auto t1 = t0 + std::chrono::milliseconds(10);

  QVERIFY(!tp.feed(tpId(kCmFormat, kGlobalAddress, 0x21), tpCm(32, 16, 3, 0x00FEE5), t0));
  QCOMPARE(tp.activeSessions(), 1);

  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(1, 0x11), t1));
  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(2, 0x22), t1));

  const auto done = tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(3, 0x33), t1);
  QVERIFY(done.has_value());
  QCOMPARE(done->bytes.size(), qsizetype(16));
  QCOMPARE(done->pgn, 0x00FEE5u);
  QCOMPARE(done->sourceAddr, quint8(0x21));
  QVERIFY(done->firstSeen == t0);
  QCOMPARE(done->bytes.at(0), char(0x11));
  QCOMPARE(done->bytes.at(7), char(0x22));
  QCOMPARE(done->bytes.at(15), char(0x33));

  QCOMPARE(tp.counters().completed, 1ull);
  QCOMPARE(tp.activeSessions(), 0);
}

/**
 * @brief An RTS opens a session even though no CTS is ever observed: Serial Studio only listens,
 *        so the handshake may be invisible while the data packets are not (spec 0073 resolution).
 */
void TstCanReassembly::rtsWithoutCtsCompletes()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  QVERIFY(!tp.feed(tpId(kCmFormat, 0x17, 0x0B), tpCm(16, 9, 2, 0x00EF00), t0));
  QCOMPARE(tp.activeSessions(), 1);

  QVERIFY(!tp.feed(tpId(kDtFormat, 0x17, 0x0B), tpDt(1, 0x41), t0));

  const auto done = tp.feed(tpId(kDtFormat, 0x17, 0x0B), tpDt(2, 0x42), t0);
  QVERIFY(done.has_value());
  QCOMPARE(done->bytes.size(), qsizetype(9));
  QCOMPARE(done->sourceAddr, quint8(0x0B));
  QCOMPARE(tp.counters().completed, 1ull);
}

/**
 * @brief A CTS travels back from the receiver, so it neither opens a session nor disturbs the one
 *        the RTS opened; the transfer still completes on its data packets.
 */
void TstCanReassembly::rtsWithObservedCtsCompletes()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  QVERIFY(!tp.feed(tpId(kCmFormat, 0x17, 0x0B), tpCm(16, 9, 2, 0x00EF00), t0));
  QVERIFY(!tp.feed(tpId(kCmFormat, 0x0B, 0x17), tpCm(17, 2, 1, 0x00EF00), t0));
  QCOMPARE(tp.activeSessions(), 1);

  QVERIFY(!tp.feed(tpId(kDtFormat, 0x17, 0x0B), tpDt(1, 0x41), t0));

  const auto done = tp.feed(tpId(kDtFormat, 0x17, 0x0B), tpDt(2, 0x42), t0);
  QVERIFY(done.has_value());
  QCOMPARE(done->bytes.size(), qsizetype(9));
  QCOMPARE(tp.counters().completed, 1ull);
}

/**
 * @brief Two ECUs transferring at the same time keep separate state, so interleaved data packets
 *        land in the session named by their own source address.
 */
void TstCanReassembly::concurrentSessionsInterleave()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  QVERIFY(!tp.feed(tpId(kCmFormat, kGlobalAddress, 0x01), tpCm(32, 9, 2, 0x00FEE5), t0));
  QVERIFY(!tp.feed(tpId(kCmFormat, kGlobalAddress, 0x02), tpCm(32, 9, 2, 0x00FEF1), t0));
  QCOMPARE(tp.activeSessions(), 2);

  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x01), tpDt(1, 0x11), t0));
  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x02), tpDt(1, 0x22), t0));

  const auto first = tp.feed(tpId(kDtFormat, kGlobalAddress, 0x01), tpDt(2, 0x11), t0);
  QVERIFY(first.has_value());
  QCOMPARE(first->pgn, 0x00FEE5u);
  QCOMPARE(first->sourceAddr, quint8(0x01));
  QCOMPARE(first->bytes.at(0), char(0x11));

  const auto second = tp.feed(tpId(kDtFormat, kGlobalAddress, 0x02), tpDt(2, 0x22), t0);
  QVERIFY(second.has_value());
  QCOMPARE(second->pgn, 0x00FEF1u);
  QCOMPARE(second->sourceAddr, quint8(0x02));
  QCOMPARE(second->bytes.at(0), char(0x22));

  QCOMPARE(tp.counters().completed, 2ull);
  QCOMPARE(tp.activeSessions(), 0);
}

/**
 * @brief An abort tears the session down from either direction and nothing is emitted afterwards.
 */
void TstCanReassembly::abortDropsSession()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  QVERIFY(!tp.feed(tpId(kCmFormat, 0x17, 0x0B), tpCm(16, 9, 2, 0x00EF00), t0));
  QVERIFY(!tp.feed(tpId(kDtFormat, 0x17, 0x0B), tpDt(1, 0x41), t0));

  QVERIFY(!tp.feed(tpId(kCmFormat, 0x0B, 0x17), tpCm(255, 0, 0, 0x00EF00), t0));
  QCOMPARE(tp.counters().aborted, 1ull);
  QCOMPARE(tp.activeSessions(), 0);

  QVERIFY(!tp.feed(tpId(kDtFormat, 0x17, 0x0B), tpDt(2, 0x42), t0));
  QCOMPARE(tp.counters().completed, 0ull);
}

/**
 * @brief A missing data packet destroys the whole session rather than shifting the payload.
 */
void TstCanReassembly::sequenceGapDropsSession()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  QVERIFY(!tp.feed(tpId(kCmFormat, kGlobalAddress, 0x21), tpCm(32, 16, 3, 0x00FEE5), t0));
  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(1, 0x11), t0));
  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(3, 0x33), t0));

  QCOMPARE(tp.counters().sequenceErrors, 1ull);
  QCOMPARE(tp.counters().completed, 0ull);
  QCOMPARE(tp.activeSessions(), 0);
}

/**
 * @brief A repeated data packet is as fatal as a gap: the sequence number must be the exact next.
 */
void TstCanReassembly::duplicatePacketDropsSession()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  QVERIFY(!tp.feed(tpId(kCmFormat, kGlobalAddress, 0x21), tpCm(32, 16, 3, 0x00FEE5), t0));
  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(1, 0x11), t0));
  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(1, 0x11), t0));

  QCOMPARE(tp.counters().sequenceErrors, 1ull);
  QCOMPARE(tp.activeSessions(), 0);
}

/**
 * @brief A transfer whose packets stop arriving is evicted once the inter-packet timeout passes.
 */
void TstCanReassembly::timeoutEvictsSession()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;
  const auto late = t0 + std::chrono::milliseconds(800);

  QVERIFY(!tp.feed(tpId(kCmFormat, kGlobalAddress, 0x21), tpCm(32, 16, 3, 0x00FEE5), t0));
  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(1, 0x11), t0));
  QCOMPARE(tp.activeSessions(), 1);

  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(2, 0x22), late));
  QCOMPARE(tp.counters().timeouts, 1ull);
  QCOMPARE(tp.counters().completed, 0ull);
  QCOMPARE(tp.activeSessions(), 0);
}

/**
 * @brief The seventeenth concurrent announcement is refused and counted; the sixteen that fit are
 *        untouched, so a flood cannot evict a live transfer.
 */
void TstCanReassembly::sessionCapIsCounted()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  for (int i = 0; i < CanReassemblyLimits::kMaxSessions; ++i) {
    const auto id = tpId(kCmFormat, kGlobalAddress, static_cast<quint8>(0x10 + i));
    QVERIFY(!tp.feed(id, tpCm(32, 16, 3, 0x00FEE5), t0));
  }

  QCOMPARE(tp.activeSessions(), CanReassemblyLimits::kMaxSessions);

  const auto extra = tpId(kCmFormat, kGlobalAddress, 0x40);
  QVERIFY(!tp.feed(extra, tpCm(32, 16, 3, 0x00FEE5), t0));

  QCOMPARE(tp.counters().sessionOverruns, 1ull);
  QCOMPARE(tp.activeSessions(), CanReassemblyLimits::kMaxSessions);
}

/**
 * @brief An announcement above the J1939 message ceiling opens nothing and is counted; the data
 *        packets that follow it find no session and emit nothing.
 */
void TstCanReassembly::sizeCapIsCounted()
{
  J1939TransportReassembler tp;
  const SteadyTimePoint t0;

  QVERIFY(!tp.feed(tpId(kCmFormat, kGlobalAddress, 0x21), tpCm(32, 2000, 255, 0x00FEE5), t0));
  QCOMPARE(tp.counters().sizeOverruns, 1ull);
  QCOMPARE(tp.activeSessions(), 0);

  QVERIFY(!tp.feed(tpId(kDtFormat, kGlobalAddress, 0x21), tpDt(1, 0x11), t0));
  QCOMPARE(tp.counters().completed, 0ull);
}

/**
 * @brief A FirstFrame plus its ConsecutiveFrames on a standardized diagnostic identifier yields
 *        the announced byte count, stamped with the FirstFrame capture time.
 */
void TstCanReassembly::isoTpMultiFrameCompletes()
{
  IsoTpReassembler iso;
  const SteadyTimePoint t0;
  const auto t1 = t0 + std::chrono::milliseconds(5);

  QVERIFY(!iso.feed(0x7E8, false, isoTpFirstFrame(20, 0x51), t0));
  QCOMPARE(iso.activeSessions(), 1);

  QVERIFY(!iso.feed(0x7E8, false, isoTpConsecutiveFrame(1, 0x52), t1));

  const auto done = iso.feed(0x7E8, false, isoTpConsecutiveFrame(2, 0x53), t1);
  QVERIFY(done.has_value());
  QCOMPARE(done->bytes.size(), qsizetype(20));
  QCOMPARE(done->canId, 0x7E8u);
  QCOMPARE(done->extendedId, false);
  QVERIFY(done->firstSeen == t0);
  QCOMPARE(done->bytes.at(0), char(0x51));
  QCOMPARE(done->bytes.at(6), char(0x52));
  QCOMPARE(done->bytes.at(13), char(0x53));

  QCOMPARE(iso.counters().completed, 1ull);
  QCOMPARE(iso.activeSessions(), 0);
}

/**
 * @brief An out-of-order ConsecutiveFrame destroys the session instead of shifting the payload.
 */
void TstCanReassembly::isoTpWrongSequenceDrops()
{
  IsoTpReassembler iso;
  const SteadyTimePoint t0;

  QVERIFY(!iso.feed(0x18DAF110u, true, isoTpFirstFrame(20, 0x51), t0));
  QCOMPARE(iso.activeSessions(), 1);

  QVERIFY(!iso.feed(0x18DAF110u, true, isoTpConsecutiveFrame(2, 0x52), t0));
  QCOMPARE(iso.counters().sequenceErrors, 1ull);
  QCOMPARE(iso.counters().completed, 0ull);
  QCOMPARE(iso.activeSessions(), 0);
}

/**
 * @brief Identifiers outside the ISO 15765-4 ranges are never inspected, so ordinary traffic whose
 *        first nibble reads as a FirstFrame is left alone and no counter moves.
 */
void TstCanReassembly::isoTpIgnoresOutOfRangeIds()
{
  IsoTpReassembler iso;
  const SteadyTimePoint t0;

  QVERIFY(!IsoTpReassembler::isDiagnosticId(0x123u, false));
  QVERIFY(!IsoTpReassembler::isDiagnosticId(0x7DFu, false));
  QVERIFY(!IsoTpReassembler::isDiagnosticId(0x18DC0000u, true));
  QVERIFY(IsoTpReassembler::isDiagnosticId(0x7E0u, false));
  QVERIFY(IsoTpReassembler::isDiagnosticId(0x7EFu, false));
  QVERIFY(IsoTpReassembler::isDiagnosticId(0x18DB33F1u, true));

  QVERIFY(!iso.feed(0x123u, false, isoTpFirstFrame(20, 0x51), t0));
  QVERIFY(!iso.feed(0x18DC0000u, true, isoTpFirstFrame(20, 0x51), t0));

  QCOMPARE(iso.activeSessions(), 0);
  QCOMPARE(iso.counters().malformed, 0ull);
  QCOMPARE(iso.counters().completed, 0ull);
}

/**
 * @brief A SingleFrame is not a multi-frame exchange, so the caller keeps publishing it verbatim
 *        and the reassembler never opens a session for it.
 */
void TstCanReassembly::isoTpSingleFramePassesThrough()
{
  IsoTpReassembler iso;
  const SteadyTimePoint t0;

  QByteArray single(8, static_cast<char>(0x00));
  single[0] = static_cast<char>(0x03);
  single[1] = static_cast<char>(0x41);

  QVERIFY(!IsoTpReassembler::isMultiFrame(single));
  QVERIFY(!iso.feed(0x7E8, false, single, t0));

  QCOMPARE(iso.activeSessions(), 0);
  QCOMPARE(iso.counters().completed, 0ull);
  QCOMPARE(iso.counters().malformed, 0ull);
}

/**
 * @brief FlowControl is claimed as part of the exchange but only observed: it never opens, closes
 *        or advances a session.
 */
void TstCanReassembly::isoTpFlowControlIsObservedOnly()
{
  IsoTpReassembler iso;
  const SteadyTimePoint t0;

  QByteArray flow(8, static_cast<char>(0x00));
  flow[0] = static_cast<char>(0x30);

  QVERIFY(IsoTpReassembler::isMultiFrame(flow));

  QVERIFY(!iso.feed(0x7E8, false, isoTpFirstFrame(20, 0x51), t0));
  QVERIFY(!iso.feed(0x7E0, false, flow, t0));
  QCOMPARE(iso.activeSessions(), 1);

  QVERIFY(!iso.feed(0x7E8, false, isoTpConsecutiveFrame(1, 0x52), t0));

  const auto done = iso.feed(0x7E8, false, isoTpConsecutiveFrame(2, 0x53), t0);
  QVERIFY(done.has_value());
  QCOMPARE(iso.counters().completed, 1ull);
}

/**
 * @brief The ISO-TP session table has the same ceiling, and the announcement past it is refused
 *        and counted rather than growing the table.
 */
void TstCanReassembly::isoTpSessionCapIsCounted()
{
  IsoTpReassembler iso;
  const SteadyTimePoint t0;

  for (int i = 0; i < CanReassemblyLimits::kMaxSessions; ++i) {
    const quint32 id = 0x18DA0000u | static_cast<quint32>(i);
    QVERIFY(!iso.feed(id, true, isoTpFirstFrame(20, 0x51), t0));
  }

  QCOMPARE(iso.activeSessions(), CanReassemblyLimits::kMaxSessions);

  QVERIFY(!iso.feed(0x18DA1000u, true, isoTpFirstFrame(20, 0x51), t0));
  QCOMPARE(iso.counters().sessionOverruns, 1ull);
  QCOMPARE(iso.activeSessions(), CanReassemblyLimits::kMaxSessions);
}

QTEST_APPLESS_MAIN(TstCanReassembly)

#include "tst_can_reassembly.moc"
