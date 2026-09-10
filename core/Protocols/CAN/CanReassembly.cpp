/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "Protocols/CAN/CanReassembly.h"

#include "Core/SSAssert.h"

using namespace IO::Drivers::CanReassemblyLimits;

// J1939-21 transport-protocol parameter group numbers
static constexpr quint32 kTpCmPgn = 0xEC00;
static constexpr quint32 kTpDtPgn = 0xEB00;

// TP.CM control bytes (J1939-21 section 5.10)
static constexpr quint8 kCmRequestToSend     = 16;
static constexpr quint8 kCmClearToSend       = 17;
static constexpr quint8 kCmEndOfMessageAck   = 19;
static constexpr quint8 kCmBroadcastAnnounce = 32;
static constexpr quint8 kCmAbort             = 255;

// Bytes of payload each TP.DT packet carries after its sequence number
static constexpr int kTpDataBytesPerPacket = 7;

// ISO 15765-2 protocol control information, taken from the first nibble of byte 0
static constexpr quint8 kPciSingleFrame      = 0;
static constexpr quint8 kPciFirstFrame       = 1;
static constexpr quint8 kPciConsecutiveFrame = 2;
static constexpr quint8 kPciFlowControl      = 3;

// Smallest message an ISO-TP FirstFrame may announce (anything less is a SingleFrame)
static constexpr int kIsoTpMinMultiFrameBytes = 8;

//--------------------------------------------------------------------------------------------------
// J1939 transport protocol
//--------------------------------------------------------------------------------------------------

/**
 * @brief Extracts the parameter group number from a 29-bit identifier per J1939-21: PDU1 groups
 *        (PF below 240) carry a destination address in PS that is not part of the PGN, PDU2 groups
 *        carry a group extension there that is.
 */
quint32 IO::Drivers::J1939TransportReassembler::parameterGroupNumber(const quint32 id29) noexcept
{
  const quint32 data_page = (id29 >> 24) & 0x03u;
  const quint32 format    = (id29 >> 16) & 0xFFu;

  if (format < 240)
    return (data_page << 16) | (format << 8);

  return (data_page << 16) | (format << 8) | ((id29 >> 8) & 0xFFu);
}

/**
 * @brief Returns true when the identifier names TP.CM or TP.DT, the only two groups this class
 *        consumes; the caller uses it to decide whether a frame is a reassembly candidate at all.
 */
bool IO::Drivers::J1939TransportReassembler::isTransportFrame(const quint32 id29) noexcept
{
  const quint32 pgn = parameterGroupNumber(id29);
  return pgn == kTpCmPgn || pgn == kTpDtPgn;
}

/**
 * @brief Packs the addresses a transfer is scoped to into one key, so two ECUs announcing
 *        different messages at the same time never share session state.
 */
quint16 IO::Drivers::J1939TransportReassembler::sessionKey(const quint8 source,
                                                           const quint8 destination) noexcept
{
  return static_cast<quint16>((static_cast<quint16>(source) << 8) | destination);
}

/**
 * @brief Forgets every in-flight transfer and zeroes the counters.
 */
void IO::Drivers::J1939TransportReassembler::reset()
{
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  m_sessions.clear();
  m_counters = CanReassemblyCounters();

  SS_ASSERT_LOG(m_sessions.isEmpty());
}

/**
 * @brief Returns how many transfers are currently open.
 */
int IO::Drivers::J1939TransportReassembler::activeSessions() const noexcept
{
  return static_cast<int>(m_sessions.size());
}

/**
 * @brief Returns the pulled diagnostic counters.
 */
const IO::Drivers::CanReassemblyCounters& IO::Drivers::J1939TransportReassembler::counters()
  const noexcept
{
  return m_counters;
}

/**
 * @brief Feeds one CAN frame into the reassembler and returns the finished message when this
 *        frame completed one. Frames that are not TP.CM or TP.DT are ignored outright; a short
 *        payload is counted as malformed rather than parsed, because every field this class reads
 *        lives in the mandatory eight data bytes.
 */
std::optional<IO::Drivers::J1939TransportReassembler::Completed> IO::Drivers::
  J1939TransportReassembler::feed(const quint32 id29,
                                  const QByteArrayView payload,
                                  const SteadyTimePoint stamp)
{
  SS_ASSERT(m_sessions.size() <= kMaxSessions, return std::nullopt);
  SS_ASSERT_LOG(payload.size() <= 64);

  const quint32 pgn = parameterGroupNumber(id29);
  if (pgn != kTpCmPgn && pgn != kTpDtPgn)
    return std::nullopt;

  evictExpired(stamp);

  if (payload.size() < 8) {
    ++m_counters.malformed;
    return std::nullopt;
  }

  const quint8 source      = static_cast<quint8>(id29 & 0xFFu);
  const quint8 destination = static_cast<quint8>((id29 >> 8) & 0xFFu);

  if (pgn == kTpDtPgn)
    return handleDataTransfer(source, destination, payload, stamp);

  const quint8 priority = static_cast<quint8>((id29 >> 26) & 0x07u);
  handleConnectionManagement(source, destination, priority, payload, stamp);
  return std::nullopt;
}

/**
 * @brief Acts on one TP.CM packet. BAM and RTS both open a session, an abort tears one down from
 *        either side of the link (hence both key orders), and CTS / EndOfMsgACK only prove the
 *        peer is still talking: they travel in the reverse direction, so they refresh the session
 *        keyed by the swapped address pair instead of opening one.
 */
void IO::Drivers::J1939TransportReassembler::handleConnectionManagement(
  const quint8 source,
  const quint8 destination,
  const quint8 priority,
  const QByteArrayView payload,
  const SteadyTimePoint stamp)
{
  SS_ASSERT(payload.size() >= 8, {
    ++m_counters.malformed;
    return;
  });
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  const quint8 control = static_cast<quint8>(payload.at(0));

  if (control == kCmAbort) {
    dropSession(sessionKey(source, destination), m_counters.aborted);
    dropSession(sessionKey(destination, source), m_counters.aborted);
    return;
  }

  if (control == kCmClearToSend || control == kCmEndOfMessageAck) {
    const auto peer = m_sessions.find(sessionKey(destination, source));
    if (peer != m_sessions.end())
      peer->lastSeen = stamp;

    return;
  }

  if (control != kCmBroadcastAnnounce && control != kCmRequestToSend)
    return;

  openSession(sessionKey(source, destination), priority, payload, stamp);
}

/**
 * @brief Opens a transfer from a BAM or RTS announcement. A second announcement on a live key
 *        abandons the first (counted as an abort), and every declared size is validated against
 *        the J1939 ceiling plus the packet count it implies before a single byte is buffered.
 */
void IO::Drivers::J1939TransportReassembler::openSession(const quint16 key,
                                                         const quint8 priority,
                                                         const QByteArrayView payload,
                                                         const SteadyTimePoint stamp)
{
  SS_ASSERT(payload.size() >= 8, {
    ++m_counters.malformed;
    return;
  });
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  dropSession(key, m_counters.aborted);

  const int total_bytes = static_cast<quint8>(payload.at(1))
                        | (static_cast<int>(static_cast<quint8>(payload.at(2))) << 8);
  const int total_packets = static_cast<quint8>(payload.at(3));

  if (total_bytes < 1 || total_bytes > kJ1939MaxBytes) {
    ++m_counters.sizeOverruns;
    return;
  }

  const int expected_packets = (total_bytes + kTpDataBytesPerPacket - 1) / kTpDataBytesPerPacket;
  if (total_packets != expected_packets || total_packets > kJ1939MaxPackets) {
    ++m_counters.malformed;
    return;
  }

  if (m_sessions.size() >= kMaxSessions) {
    ++m_counters.sessionOverruns;
    return;
  }

  Session session;
  session.pgn = static_cast<quint32>(static_cast<quint8>(payload.at(5)))
              | (static_cast<quint32>(static_cast<quint8>(payload.at(6))) << 8)
              | (static_cast<quint32>(static_cast<quint8>(payload.at(7))) << 16);
  session.priority     = priority;
  session.totalBytes   = total_bytes;
  session.totalPackets = total_packets;
  session.firstSeen    = stamp;
  session.lastSeen     = stamp;
  session.bytes.reserve(total_packets * kTpDataBytesPerPacket);
  m_sessions.insert(key, session);
}

/**
 * @brief Appends one TP.DT packet to its session and yields the finished message on the last one.
 *        Anything other than the exact next sequence number, duplicate or gap alike, destroys the
 *        session: the missing bytes cannot be recovered from a listener's seat, and half a message
 *        decoded against a DBC is worse than none.
 */
std::optional<IO::Drivers::J1939TransportReassembler::Completed> IO::Drivers::
  J1939TransportReassembler::handleDataTransfer(const quint8 source,
                                                const quint8 destination,
                                                const QByteArrayView payload,
                                                const SteadyTimePoint stamp)
{
  SS_ASSERT(payload.size() >= 8, {
    ++m_counters.malformed;
    return std::nullopt;
  });
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  const auto it = m_sessions.find(sessionKey(source, destination));
  if (it == m_sessions.end())
    return std::nullopt;

  if (static_cast<int>(static_cast<quint8>(payload.at(0))) != it->nextSequence) {
    m_sessions.erase(it);
    ++m_counters.sequenceErrors;
    return std::nullopt;
  }

  it->bytes.append(payload.constData() + 1, kTpDataBytesPerPacket);
  it->lastSeen = stamp;
  ++it->nextSequence;

  if (it->bytes.size() > kJ1939MaxBytes) {
    m_sessions.erase(it);
    ++m_counters.sizeOverruns;
    return std::nullopt;
  }

  if (it->nextSequence <= it->totalPackets)
    return std::nullopt;

  Completed done;
  done.bytes      = it->bytes.left(it->totalBytes);
  done.firstSeen  = it->firstSeen;
  done.pgn        = it->pgn;
  done.priority   = it->priority;
  done.sourceAddr = source;

  m_sessions.erase(it);
  ++m_counters.completed;
  return done;
}

/**
 * @brief Erases a session by key and counts the drop, doing nothing when no such session is open
 *        so an unsolicited abort or a duplicate announcement never inflates the counters.
 */
void IO::Drivers::J1939TransportReassembler::dropSession(const quint16 key, quint64& counter)
{
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  const auto it = m_sessions.find(key);
  if (it == m_sessions.end())
    return;

  m_sessions.erase(it);
  ++counter;
}

/**
 * @brief Drops every session whose last packet is older than the inter-packet timeout. The loop is
 *        bounded by the session cap, so a silent bus costs a fixed walk per frame and nothing more.
 */
void IO::Drivers::J1939TransportReassembler::evictExpired(const SteadyTimePoint stamp)
{
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  if (m_sessions.isEmpty())
    return;

  const auto timeout = std::chrono::milliseconds(kSessionTimeoutMs);
  for (auto it = m_sessions.begin(); it != m_sessions.end();)
    if (stamp >= it->lastSeen + timeout) {
      it = m_sessions.erase(it);
      ++m_counters.timeouts;
    } else
      ++it;
}

//--------------------------------------------------------------------------------------------------
// ISO 15765-2 transport protocol
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the identifier falls inside an ISO 15765-4 standardized diagnostic
 *        range. Restricting reassembly to those ranges is what keeps a payload whose first nibble
 *        happens to read as a FirstFrame from swallowing ordinary application traffic.
 */
bool IO::Drivers::IsoTpReassembler::isDiagnosticId(const quint32 canId,
                                                   const bool extendedId) noexcept
{
  if (!extendedId)
    return canId >= 0x7E0u && canId <= 0x7EFu;

  const quint32 base = canId & 0x1FFF0000u;
  return base == 0x18DA0000u || base == 0x18DB0000u;
}

/**
 * @brief Returns true for the three protocol control information values that belong to a
 *        multi-frame exchange. A SingleFrame answers false: the caller publishes it unchanged.
 */
bool IO::Drivers::IsoTpReassembler::isMultiFrame(const QByteArrayView payload) noexcept
{
  if (payload.isEmpty())
    return false;

  const quint8 pci = static_cast<quint8>(static_cast<quint8>(payload.at(0)) >> 4);
  return pci >= kPciFirstFrame && pci <= kPciFlowControl;
}

/**
 * @brief Forgets every in-flight message and zeroes the counters.
 */
void IO::Drivers::IsoTpReassembler::reset()
{
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  m_sessions.clear();
  m_counters = CanReassemblyCounters();

  SS_ASSERT_LOG(m_sessions.isEmpty());
}

/**
 * @brief Returns how many diagnostic messages are currently open.
 */
int IO::Drivers::IsoTpReassembler::activeSessions() const noexcept
{
  return static_cast<int>(m_sessions.size());
}

/**
 * @brief Returns the pulled diagnostic counters.
 */
const IO::Drivers::CanReassemblyCounters& IO::Drivers::IsoTpReassembler::counters() const noexcept
{
  return m_counters;
}

/**
 * @brief Feeds one CAN frame into the reassembler and returns the finished message when this frame
 *        completed one. Identifiers outside the standardized diagnostic ranges are ignored without
 *        touching any counter, SingleFrames are left to the caller, and FlowControl is observed
 *        only: a listener never answers on someone else's diagnostic session.
 */
std::optional<IO::Drivers::IsoTpReassembler::Completed> IO::Drivers::IsoTpReassembler::feed(
  const quint32 canId,
  const bool extendedId,
  const QByteArrayView payload,
  const SteadyTimePoint stamp)
{
  SS_ASSERT(m_sessions.size() <= kMaxSessions, return std::nullopt);
  SS_ASSERT_LOG(payload.size() <= 64);

  if (!isDiagnosticId(canId, extendedId))
    return std::nullopt;

  evictExpired(stamp);

  if (payload.isEmpty()) {
    ++m_counters.malformed;
    return std::nullopt;
  }

  const quint8 pci = static_cast<quint8>(static_cast<quint8>(payload.at(0)) >> 4);
  if (pci == kPciSingleFrame || pci == kPciFlowControl)
    return std::nullopt;

  if (pci == kPciConsecutiveFrame)
    return continueFrame(canId, payload, stamp);

  if (pci == kPciFirstFrame)
    startFirstFrame(canId, extendedId, payload, stamp);

  return std::nullopt;
}

/**
 * @brief Opens a message from a FirstFrame. A FirstFrame arriving on a live identifier abandons
 *        whatever was in flight there (counted as an abort), and a declared length below the
 *        SingleFrame boundary or above the ISO-TP ceiling is refused before any byte is buffered.
 */
void IO::Drivers::IsoTpReassembler::startFirstFrame(const quint32 canId,
                                                    const bool extendedId,
                                                    const QByteArrayView payload,
                                                    const SteadyTimePoint stamp)
{
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  if (payload.size() < 2) {
    ++m_counters.malformed;
    return;
  }

  const auto live = m_sessions.find(canId);
  if (live != m_sessions.end()) {
    m_sessions.erase(live);
    ++m_counters.aborted;
  }

  const int total =
    ((static_cast<quint8>(payload.at(0)) & 0x0F) << 8) | static_cast<quint8>(payload.at(1));

  if (total < kIsoTpMinMultiFrameBytes) {
    ++m_counters.malformed;
    return;
  }

  if (total > kIsoTpMaxBytes) {
    ++m_counters.sizeOverruns;
    return;
  }

  if (m_sessions.size() >= kMaxSessions) {
    ++m_counters.sessionOverruns;
    return;
  }

  Session session;
  session.totalBytes = total;
  session.extendedId = extendedId;
  session.firstSeen  = stamp;
  session.lastSeen   = stamp;
  session.bytes.reserve(total);
  session.bytes.append(payload.constData() + 2, qMin<qsizetype>(payload.size() - 2, total));
  m_sessions.insert(canId, session);
}

/**
 * @brief Appends one ConsecutiveFrame and yields the message once the announced length is reached.
 *        The sequence number is four bits wide and wraps from 15 back to 0; any other value means
 *        a lost or repeated frame, which destroys the session rather than shifting the payload.
 */
std::optional<IO::Drivers::IsoTpReassembler::Completed> IO::Drivers::IsoTpReassembler::
  continueFrame(const quint32 canId, const QByteArrayView payload, const SteadyTimePoint stamp)
{
  SS_ASSERT(!payload.isEmpty(), {
    ++m_counters.malformed;
    return std::nullopt;
  });
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  const auto it = m_sessions.find(canId);
  if (it == m_sessions.end())
    return std::nullopt;

  if ((static_cast<quint8>(payload.at(0)) & 0x0F) != it->nextSequence) {
    m_sessions.erase(it);
    ++m_counters.sequenceErrors;
    return std::nullopt;
  }

  const qsizetype room = it->totalBytes - it->bytes.size();
  const qsizetype take = qMin<qsizetype>(payload.size() - 1, room);
  if (take > 0)
    it->bytes.append(payload.constData() + 1, take);

  it->lastSeen     = stamp;
  it->nextSequence = (it->nextSequence + 1) & 0x0F;

  if (it->bytes.size() < it->totalBytes)
    return std::nullopt;

  Completed done;
  done.bytes      = it->bytes;
  done.firstSeen  = it->firstSeen;
  done.canId      = canId;
  done.extendedId = it->extendedId;

  m_sessions.erase(it);
  ++m_counters.completed;
  return done;
}

/**
 * @brief Drops every message whose last frame is older than the inter-frame timeout. The loop is
 *        bounded by the session cap, so an abandoned exchange costs a fixed walk and nothing more.
 */
void IO::Drivers::IsoTpReassembler::evictExpired(const SteadyTimePoint stamp)
{
  SS_ASSERT_LOG(m_sessions.size() <= kMaxSessions);

  if (m_sessions.isEmpty())
    return;

  const auto timeout = std::chrono::milliseconds(kSessionTimeoutMs);
  for (auto it = m_sessions.begin(); it != m_sessions.end();)
    if (stamp >= it->lastSeen + timeout) {
      it = m_sessions.erase(it);
      ++m_counters.timeouts;
    } else
      ++it;
}
