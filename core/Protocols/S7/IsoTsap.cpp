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

#include "Protocols/S7/IsoTsap.h"

#include "Core/SSAssert.h"

/**
 * @brief Reads one octet as an unsigned byte; the caller has already bounds-checked the position.
 */
[[nodiscard]] static inline std::uint8_t s7TsapOctet(QByteArrayView view, qsizetype pos) noexcept
{
  SS_ASSERT(pos >= 0 && pos < view.size(), return 0);

  return static_cast<std::uint8_t>(view[pos]);
}

/**
 * @brief Appends one big-endian byte to @p out, keeping every wire write in one spelling.
 */
static void appendByte(QByteArray& out, std::uint8_t value)
{
  SS_ASSERT_LOG(out.size() < IO::Drivers::S7Comm::kMaxTpktBytes);

  out.append(static_cast<char>(value));
}

//--------------------------------------------------------------------------------------------------
// Construction and state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a closed transport carrying the TPDU size the connect request asks for, so a
 *        caller that reads the size before a handshake gets the request value rather than zero.
 */
IO::Drivers::S7Comm::Transport::Transport()
  : m_tpduBytes(1 << kTpduSizeCode)
  , m_sourceReference(0)
  , m_framesSent(0)
  , m_framesReceived(0)
  , m_malformed(0)
{}

/**
 * @brief Returns the transport to its opening condition. The counters survive: they are pulled
 *        diagnostics for the whole session (spec 0033), not per-connection scratch.
 */
void IO::Drivers::S7Comm::Transport::reset() noexcept
{
  m_tpduBytes = 1 << kTpduSizeCode;
}

/**
 * @brief Returns the TPDU size the peer confirmed, in bytes.
 */
int IO::Drivers::S7Comm::Transport::tpduBytes() const noexcept
{
  return m_tpduBytes;
}

/**
 * @brief TPKTs this side has encoded since construction.
 */
quint64 IO::Drivers::S7Comm::Transport::framesSent() const noexcept
{
  return m_framesSent;
}

/**
 * @brief TPKTs whose header decoded, including the ones a later check then refused.
 */
quint64 IO::Drivers::S7Comm::Transport::framesReceived() const noexcept
{
  return m_framesReceived;
}

/**
 * @brief Frames refused for a bad version byte, an impossible length or a contradictory TPDU.
 */
quint64 IO::Drivers::S7Comm::Transport::malformedFrames() const noexcept
{
  return m_malformed;
}

/**
 * @brief Returns the low octet of the called TSAP, which is where the CPU sits in the rack.
 */
std::uint8_t IO::Drivers::S7Comm::Transport::calledTsapLow(int rack, int slot) noexcept
{
  SS_ASSERT_LOG(rack >= 0 && rack <= kMaxRack);
  SS_ASSERT_LOG(slot >= 0 && slot <= kMaxSlot);

  const int bounded = qBound(0, rack, kMaxRack) * 0x20 + qBound(0, slot, kMaxSlot);
  return static_cast<std::uint8_t>(bounded & 0xFF);
}

//--------------------------------------------------------------------------------------------------
// Encoders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prepends the four-octet TPKT header to a finished TPDU. The length field covers the whole
 *        packet, header included, which is what lets a reader frame the stream before it knows
 *        anything about the TPDU inside.
 */
QByteArray IO::Drivers::S7Comm::Transport::frameTpkt(QByteArrayView tpdu)
{
  SS_ASSERT(!tpdu.isEmpty(), return {});
  SS_ASSERT(tpdu.size() + kTpktHeaderBytes <= kMaxTpktBytes, return {});

  const auto total = static_cast<int>(tpdu.size()) + kTpktHeaderBytes;

  QByteArray packet;
  packet.reserve(total);
  appendByte(packet, kTpktVersion);
  appendByte(packet, 0);
  appendByte(packet, static_cast<std::uint8_t>((total >> 8) & 0xFF));
  appendByte(packet, static_cast<std::uint8_t>(total & 0xFF));
  packet.append(tpdu);

  ++m_framesSent;
  return packet;
}

/**
 * @brief Builds a class-0 connect request naming the CPU's rack and slot. The calling TSAP is the
 *        fixed 0x0100 an S7 client identifies itself with; the called TSAP's low octet is the one
 *        field an engineer gets wrong, and a wrong one is refused by the CPU rather than ignored.
 */
QByteArray IO::Drivers::S7Comm::Transport::buildConnectRequest(int rack, int slot)
{
  SS_ASSERT_LOG(rack >= 0 && rack <= kMaxRack);
  SS_ASSERT_LOG(slot >= 0 && slot <= kMaxSlot);

  ++m_sourceReference;

  QByteArray tpdu;
  tpdu.reserve(kConnectTpduBytes);
  appendByte(tpdu, static_cast<std::uint8_t>(kConnectTpduBytes - 1));
  appendByte(tpdu, kTpduConnectRequest);
  appendByte(tpdu, 0);
  appendByte(tpdu, 0);
  appendByte(tpdu, static_cast<std::uint8_t>((m_sourceReference >> 8) & 0xFF));
  appendByte(tpdu, static_cast<std::uint8_t>(m_sourceReference & 0xFF));
  appendByte(tpdu, 0);
  appendByte(tpdu, kParamTpduSize);
  appendByte(tpdu, 1);
  appendByte(tpdu, kTpduSizeCode);
  appendByte(tpdu, kParamCallingTsap);
  appendByte(tpdu, 2);
  appendByte(tpdu, kCallingTsapHi);
  appendByte(tpdu, 0);
  appendByte(tpdu, kParamCalledTsap);
  appendByte(tpdu, 2);
  appendByte(tpdu, kCalledTsapHi);
  appendByte(tpdu, calledTsapLow(rack, slot));

  SS_ASSERT_LOG(tpdu.size() == kConnectTpduBytes);
  return frameTpkt(tpdu);
}

/**
 * @brief Wraps one S7 payload in a data TPDU carrying the end-of-transmission flag. The client
 *        never splits a request: the negotiated PDU length already bounds what it sends, so a
 *        continuation TPDU on the send side would only be a second way to be wrong.
 */
QByteArray IO::Drivers::S7Comm::Transport::wrapData(QByteArrayView payload)
{
  SS_ASSERT(!payload.isEmpty(), return {});
  SS_ASSERT(payload.size() + kTpktHeaderBytes + kDataTpduBytes <= kMaxTpktBytes, return {});

  QByteArray tpdu;
  tpdu.reserve(kDataTpduBytes + static_cast<int>(payload.size()));
  appendByte(tpdu, static_cast<std::uint8_t>(kDataTpduBytes - 1));
  appendByte(tpdu, kTpduData);
  appendByte(tpdu, kEndOfTransmission);
  tpdu.append(payload);

  SS_ASSERT_LOG(tpdu.size() == kDataTpduBytes + payload.size());
  return frameTpkt(tpdu);
}

//--------------------------------------------------------------------------------------------------
// Decoders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Accepts a connect confirm and records the TPDU size the peer settled on. A size code
 *        outside the range the standard defines is clamped rather than shifted with: the code is
 *        a shift distance, and an unclamped one taken off the wire is undefined behaviour.
 */
bool IO::Drivers::S7Comm::Transport::parseConnectConfirm(QByteArrayView tpdu)
{
  if (tpdu.size() < kConnectHeaderBytes || s7TsapOctet(tpdu, 1) != kTpduConnectConfirm) {
    ++m_malformed;
    return false;
  }

  const qsizetype end = static_cast<qsizetype>(s7TsapOctet(tpdu, 0)) + 1;
  if (end > tpdu.size() || end < kConnectHeaderBytes) {
    ++m_malformed;
    return false;
  }

  qsizetype pos = kConnectHeaderBytes;
  for (int guard = 0; guard < kMaxParameters && pos + 2 <= end; ++guard) {
    const auto code   = s7TsapOctet(tpdu, pos);
    const qsizetype n = s7TsapOctet(tpdu, pos + 1);
    if (pos + 2 + n > end) {
      ++m_malformed;
      return false;
    }

    if (code == kParamTpduSize && n == 1)
      m_tpduBytes = 1 << qBound(kMinTpduCode, s7TsapOctet(tpdu, pos + 2), kMaxTpduCode);

    pos += 2 + n;
  }

  SS_ASSERT_LOG(m_tpduBytes >= kMinTpduBytes && m_tpduBytes <= kMaxTpduBytes);
  return true;
}

/**
 * @brief Locates the TPKT at the front of @p buffer. NeedMore leaves the buffer untouched; Ok
 *        fills @c totalBytes with the bytes the caller must drop. A Malformed verdict is terminal
 *        for the stream: the length field is the only way to find the next packet boundary, so a
 *        packet that contradicts it cannot be resynchronised, only abandoned.
 */
IO::Drivers::S7Comm::TpktResult IO::Drivers::S7Comm::Transport::extractTpkt(QByteArrayView buffer,
                                                                            Tpkt& out)
{
  out = Tpkt{};
  if (buffer.size() < kTpktHeaderBytes)
    return TpktResult::NeedMore;

  if (s7TsapOctet(buffer, 0) != kTpktVersion || s7TsapOctet(buffer, 1) != 0) {
    ++m_malformed;
    return TpktResult::Malformed;
  }

  const int total = (static_cast<int>(s7TsapOctet(buffer, 2)) << 8) | s7TsapOctet(buffer, 3);
  if (total < kMinTpktBytes || total > kMaxTpktBytes) {
    ++m_malformed;
    return TpktResult::Malformed;
  }

  if (buffer.size() < total)
    return TpktResult::NeedMore;

  const qsizetype indicator = s7TsapOctet(buffer, kTpktHeaderBytes);
  if (indicator < 2 || indicator + 1 > total - kTpktHeaderBytes) {
    ++m_malformed;
    return TpktResult::Malformed;
  }

  out.totalBytes = total;
  out.tpduOffset = kTpktHeaderBytes;
  out.tpduSize   = total - kTpktHeaderBytes;
  ++m_framesReceived;
  return TpktResult::Ok;
}

/**
 * @brief Appends one data TPDU's user data to @p assembly and reports through @p complete whether
 *        the end-of-transmission flag closed the message. A continuation that would push the
 *        assembly past its cap is refused with the assembly rolled back to what it held on entry:
 *        a message half-built out of a hostile stream reads downstream as controller data.
 */
IO::Drivers::S7Comm::TpktResult IO::Drivers::S7Comm::Transport::acceptData(QByteArrayView tpdu,
                                                                           QByteArray& assembly,
                                                                           bool& complete)
{
  complete = false;
  if (tpdu.size() < kDataTpduBytes || s7TsapOctet(tpdu, 1) != kTpduData) {
    ++m_malformed;
    return TpktResult::Malformed;
  }

  const qsizetype header = static_cast<qsizetype>(s7TsapOctet(tpdu, 0)) + 1;
  if (header < kDataTpduBytes || header > tpdu.size()) {
    ++m_malformed;
    return TpktResult::Malformed;
  }

  const qsizetype mark = assembly.size();
  if (mark + tpdu.size() - header > kMaxAssemblyBytes) {
    assembly.resize(mark);
    ++m_malformed;
    return TpktResult::Malformed;
  }

  if (tpdu.size() > header)
    assembly.append(tpdu.sliced(header));

  complete = (s7TsapOctet(tpdu, 2) & kEndOfTransmission) != 0;
  SS_ASSERT_LOG(assembly.size() >= mark);
  return TpktResult::Ok;
}
