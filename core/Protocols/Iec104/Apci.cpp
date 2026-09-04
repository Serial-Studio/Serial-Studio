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

#include "Protocols/Iec104/Apci.h"

#include "Core/SSAssert.h"

static constexpr qint64 kNotArmed = -1;

/**
 * @brief Reads one 15-bit sequence number from the control-field octet pair at @p pos.
 */
[[nodiscard]] static int readSequence(QByteArrayView buffer, qsizetype pos) noexcept
{
  SS_ASSERT(pos >= 0 && pos + 1 < buffer.size(), return 0);

  const auto low  = static_cast<std::uint8_t>(buffer[pos]);
  const auto high = static_cast<std::uint8_t>(buffer[pos + 1]);
  return ((static_cast<int>(high) << 7) | (low >> 1)) & 0x7FFF;
}

/**
 * @brief Writes one 15-bit sequence number into the control-field octet pair at @p pos.
 */
static void writeSequence(QByteArray& apdu, qsizetype pos, int value) noexcept
{
  SS_ASSERT(pos >= 0 && pos + 1 < apdu.size(), return);

  apdu[pos]     = static_cast<char>((value << 1) & 0xFE);
  apdu[pos + 1] = static_cast<char>((value >> 7) & 0xFF);
}

/**
 * @brief True for the six U-format functions the specification defines; any other control octet is
 *        a malformed frame rather than a function this build has yet to implement.
 */
[[nodiscard]] static bool isKnownFunction(IO::Drivers::Iec104Proto::UFunction function) noexcept
{
  using IO::Drivers::Iec104Proto::UFunction;

  switch (function) {
    case UFunction::StartDtAct:
    case UFunction::StartDtCon:
    case UFunction::StopDtAct:
    case UFunction::StopDtCon:
    case UFunction::TestFrAct:
    case UFunction::TestFrCon:
      return true;
    case UFunction::None:
      break;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Construction and configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a closed connection carrying the specification's default windows and deadlines.
 */
IO::Drivers::Iec104Proto::Connection::Connection()
  : m_k(kDefaultK)
  , m_w(kDefaultW)
  , m_t1(kDefaultT1Ms)
  , m_t2(kDefaultT2Ms)
  , m_t3(kDefaultT3Ms)
  , m_sendSeq(0)
  , m_recvSeq(0)
  , m_ackedSeq(0)
  , m_unackedRx(0)
  , m_lastTrafficMs(kNotArmed)
  , m_firstUnackedRxMs(kNotArmed)
  , m_sendPendingMs(kNotArmed)
  , m_controlPendingMs(kNotArmed)
  , m_malformed(0)
  , m_sequenceErrors(0)
  , m_framesSent(0)
  , m_framesReceived(0)
{}

/**
 * @brief Returns the state machine to its opening condition. The counters survive: they are pulled
 *        diagnostics for the whole session (spec 0033), not per-connection scratch.
 */
void IO::Drivers::Iec104Proto::Connection::reset(qint64 nowMs) noexcept
{
  m_sendSeq          = 0;
  m_recvSeq          = 0;
  m_ackedSeq         = 0;
  m_unackedRx        = 0;
  m_lastTrafficMs    = nowMs;
  m_firstUnackedRxMs = kNotArmed;
  m_sendPendingMs    = kNotArmed;
  m_controlPendingMs = kNotArmed;
}

/**
 * @brief Applies the negotiated windows and deadlines, clamped to the ranges the specification
 *        allows so a hand-edited project cannot configure a window that never opens.
 */
void IO::Drivers::Iec104Proto::Connection::configure(
  int k, int w, int t1Ms, int t2Ms, int t3Ms) noexcept
{
  m_k = qBound(kMinWindow, k, kMaxWindow);
  m_w = qBound(kMinWindow, w, m_k);

  m_t1 = qBound(kMinTimeMs, t1Ms, kMaxTimeMs);
  m_t2 = qBound(kMinTimeMs, t2Ms, m_t1);
  m_t3 = qBound(kMinTimeMs, t3Ms, kMaxTimeMs);
}

//--------------------------------------------------------------------------------------------------
// State inspection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the maximum number of unacknowledged I-frames this side may have outstanding.
 */
int IO::Drivers::Iec104Proto::Connection::windowK() const noexcept
{
  return m_k;
}

/**
 * @brief Returns the number of received I-frames that forces a supervisory acknowledgement.
 */
int IO::Drivers::Iec104Proto::Connection::windowW() const noexcept
{
  return m_w;
}

/**
 * @brief Returns the send/confirm deadline in milliseconds.
 */
int IO::Drivers::Iec104Proto::Connection::t1Ms() const noexcept
{
  return m_t1;
}

/**
 * @brief Returns the acknowledgement deadline in milliseconds.
 */
int IO::Drivers::Iec104Proto::Connection::t2Ms() const noexcept
{
  return m_t2;
}

/**
 * @brief Returns the idle-test deadline in milliseconds.
 */
int IO::Drivers::Iec104Proto::Connection::t3Ms() const noexcept
{
  return m_t3;
}

/**
 * @brief Returns the next send sequence number this side will use.
 */
int IO::Drivers::Iec104Proto::Connection::sendSeq() const noexcept
{
  return m_sendSeq;
}

/**
 * @brief Returns the sequence number this side expects next from the peer.
 */
int IO::Drivers::Iec104Proto::Connection::recvSeq() const noexcept
{
  return m_recvSeq;
}

/**
 * @brief Returns the highest sequence number the peer has acknowledged.
 */
int IO::Drivers::Iec104Proto::Connection::ackedSeq() const noexcept
{
  return m_ackedSeq;
}

/**
 * @brief Returns how many sent I-frames are still unacknowledged, modulo the 15-bit counter.
 */
int IO::Drivers::Iec104Proto::Connection::outstanding() const noexcept
{
  return (m_sendSeq - m_ackedSeq + kSequenceModulo) % kSequenceModulo;
}

/**
 * @brief Returns how many received I-frames this side has not acknowledged yet.
 */
int IO::Drivers::Iec104Proto::Connection::unackedReceived() const noexcept
{
  return m_unackedRx;
}

/**
 * @brief True while the k window still admits another I-frame.
 */
bool IO::Drivers::Iec104Proto::Connection::sendWindowOpen() const noexcept
{
  return outstanding() < m_k;
}

/**
 * @brief True when the w count is reached or t2 has elapsed since the first unacknowledged frame;
 *        either one obliges this side to send a supervisory frame.
 */
bool IO::Drivers::Iec104Proto::Connection::ackDue(qint64 nowMs) const noexcept
{
  if (m_unackedRx <= 0)
    return false;

  if (m_unackedRx >= m_w)
    return true;

  return m_firstUnackedRxMs != kNotArmed && nowMs - m_firstUnackedRxMs >= m_t2;
}

/**
 * @brief True when the link has been silent for t3 and no control frame is already awaiting its
 *        confirmation; a second TESTFR stacked on an unanswered one only hides the real timeout.
 */
bool IO::Drivers::Iec104Proto::Connection::testDue(qint64 nowMs) const noexcept
{
  if (m_lastTrafficMs == kNotArmed || m_controlPendingMs != kNotArmed)
    return false;

  return nowMs - m_lastTrafficMs >= m_t3;
}

/**
 * @brief True when an I-frame or a control activation has gone unconfirmed for t1, which is the
 *        one condition that declares the link dead while bytes may still be flowing.
 */
bool IO::Drivers::Iec104Proto::Connection::confirmOverdue(qint64 nowMs) const noexcept
{
  if (m_sendPendingMs != kNotArmed && nowMs - m_sendPendingMs >= m_t1)
    return true;

  return m_controlPendingMs != kNotArmed && nowMs - m_controlPendingMs >= m_t1;
}

//--------------------------------------------------------------------------------------------------
// Encoders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a U-format APDU. An activation arms the t1 deadline; a confirmation never does,
 *        because nothing answers a confirmation.
 */
QByteArray IO::Drivers::Iec104Proto::Connection::encodeUnnumbered(UFunction function, qint64 nowMs)
{
  SS_ASSERT(isKnownFunction(function), return {});

  QByteArray apdu(kApciBytes, static_cast<char>(0));
  apdu[0] = static_cast<char>(kStartByte);
  apdu[1] = static_cast<char>(kMinApduLength);
  apdu[2] = static_cast<char>(function);

  if (!isConfirmation(function) && m_controlPendingMs == kNotArmed)
    m_controlPendingMs = nowMs;

  m_lastTrafficMs = nowMs;
  ++m_framesSent;
  return apdu;
}

/**
 * @brief Builds an S-format APDU carrying the current receive count, which clears this side's
 *        acknowledgement obligation.
 */
QByteArray IO::Drivers::Iec104Proto::Connection::encodeSupervisory(qint64 nowMs)
{
  QByteArray apdu(kApciBytes, static_cast<char>(0));
  apdu[0] = static_cast<char>(kStartByte);
  apdu[1] = static_cast<char>(kMinApduLength);
  apdu[2] = static_cast<char>(0x01);
  writeSequence(apdu, 4, m_recvSeq);

  m_unackedRx        = 0;
  m_firstUnackedRxMs = kNotArmed;
  m_lastTrafficMs    = nowMs;
  ++m_framesSent;
  return apdu;
}

/**
 * @brief Builds an I-format APDU around @p asdu, consuming one send sequence number. Returns an
 *        empty array when the k window is closed or the payload cannot fit an APDU: the caller
 *        must queue rather than send, because a frame past the window is a protocol violation.
 */
QByteArray IO::Drivers::Iec104Proto::Connection::encodeInformation(QByteArrayView asdu,
                                                                   qint64 nowMs)
{
  SS_ASSERT(!asdu.isEmpty(), return {});

  if (asdu.size() > kMaxAsduBytes) {
    ++m_malformed;
    return {};
  }

  if (!sendWindowOpen())
    return {};

  QByteArray apdu(kApciBytes, static_cast<char>(0));
  apdu[0] = static_cast<char>(kStartByte);
  apdu[1] = static_cast<char>(kControlBytes + static_cast<int>(asdu.size()));
  writeSequence(apdu, 2, m_sendSeq);
  writeSequence(apdu, 4, m_recvSeq);
  apdu.append(asdu);

  m_sendSeq = (m_sendSeq + 1) % kSequenceModulo;
  if (m_sendPendingMs == kNotArmed)
    m_sendPendingMs = nowMs;

  m_unackedRx        = 0;
  m_firstUnackedRxMs = kNotArmed;
  m_lastTrafficMs    = nowMs;
  ++m_framesSent;
  return apdu;
}

//--------------------------------------------------------------------------------------------------
// Decoder
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes the APDU at the front of @p buffer. NeedMore leaves the buffer untouched; every
 *        other outcome fills @c out.apduSize with the bytes the caller must drop, so a hostile
 *        stream can neither stall the reader nor address memory outside the view.
 */
IO::Drivers::Iec104Proto::ParseResult IO::Drivers::Iec104Proto::Connection::consume(
  QByteArrayView buffer, Apdu& out, qint64 nowMs)
{
  out = Apdu{};
  if (buffer.size() < 2)
    return ParseResult::NeedMore;

  if (static_cast<std::uint8_t>(buffer[0]) != kStartByte) {
    ++m_malformed;
    return ParseResult::Malformed;
  }

  const int length = static_cast<std::uint8_t>(buffer[1]);
  if (length < kMinApduLength || length > kMaxApduLength) {
    ++m_malformed;
    return ParseResult::Malformed;
  }

  const qsizetype total = static_cast<qsizetype>(length) + 2;
  if (buffer.size() < total)
    return ParseResult::NeedMore;

  out.apduSize    = total;
  m_lastTrafficMs = nowMs;
  ++m_framesReceived;

  const auto control = static_cast<std::uint8_t>(buffer[2]);
  if ((control & 0x01) == 0)
    return acceptInformation(buffer, out, nowMs);

  if ((control & 0x03) == 0x01)
    return acceptSupervisory(buffer, out, nowMs);

  return acceptUnnumbered(buffer, out);
}

/**
 * @brief Completes an I-format decode: the send sequence must be exactly the one expected, and the
 *        piggybacked receive count must acknowledge frames this side actually sent.
 */
IO::Drivers::Iec104Proto::ParseResult IO::Drivers::Iec104Proto::Connection::acceptInformation(
  QByteArrayView buffer, Apdu& out, qint64 nowMs)
{
  SS_ASSERT(out.apduSize >= kApciBytes, return ParseResult::Malformed);

  out.type       = FrameType::Information;
  out.sendSeq    = readSequence(buffer, 2);
  out.recvSeq    = readSequence(buffer, 4);
  out.asduOffset = kApciBytes;
  out.asduSize   = out.apduSize - kApciBytes;

  if (out.sendSeq != m_recvSeq) {
    ++m_sequenceErrors;
    return ParseResult::OutOfOrder;
  }

  if (!acknowledge(out.recvSeq, nowMs)) {
    ++m_sequenceErrors;
    return ParseResult::OutOfOrder;
  }

  m_recvSeq = (m_recvSeq + 1) % kSequenceModulo;
  if (m_unackedRx == 0)
    m_firstUnackedRxMs = nowMs;

  ++m_unackedRx;
  return ParseResult::Ok;
}

/**
 * @brief Completes an S-format decode; a supervisory frame carrying a payload is malformed.
 */
IO::Drivers::Iec104Proto::ParseResult IO::Drivers::Iec104Proto::Connection::acceptSupervisory(
  QByteArrayView buffer, Apdu& out, qint64 nowMs)
{
  if (out.apduSize != kApciBytes) {
    ++m_malformed;
    return ParseResult::Malformed;
  }

  out.type    = FrameType::Supervisory;
  out.recvSeq = readSequence(buffer, 4);
  if (!acknowledge(out.recvSeq, nowMs)) {
    ++m_sequenceErrors;
    return ParseResult::OutOfOrder;
  }

  return ParseResult::Ok;
}

/**
 * @brief Completes a U-format decode; an unknown control octet is counted and refused rather than
 *        guessed, and a confirmation clears the pending control deadline.
 */
IO::Drivers::Iec104Proto::ParseResult IO::Drivers::Iec104Proto::Connection::acceptUnnumbered(
  QByteArrayView buffer, Apdu& out)
{
  if (out.apduSize != kApciBytes) {
    ++m_malformed;
    return ParseResult::Malformed;
  }

  const auto function = static_cast<UFunction>(static_cast<std::uint8_t>(buffer[2]));
  if (!isKnownFunction(function)) {
    ++m_malformed;
    return ParseResult::Malformed;
  }

  out.type     = FrameType::Unnumbered;
  out.function = function;
  noteConfirmation(function);
  return ParseResult::Ok;
}

/**
 * @brief Applies a received acknowledgement count. A count that walks past the frames this side
 *        actually sent is refused: honouring it would silently retire frames the peer never saw.
 */
bool IO::Drivers::Iec104Proto::Connection::acknowledge(int recvSeq, qint64 nowMs) noexcept
{
  SS_ASSERT(recvSeq >= 0 && recvSeq < kSequenceModulo, return false);

  const int pending = outstanding();
  const int advance = (recvSeq - m_ackedSeq + kSequenceModulo) % kSequenceModulo;
  if (advance > pending)
    return false;

  m_ackedSeq = recvSeq;
  if (outstanding() == 0)
    m_sendPendingMs = kNotArmed;
  else if (advance > 0)
    m_sendPendingMs = nowMs;

  return true;
}

/**
 * @brief Clears the pending control deadline when the peer confirms the activation in flight.
 */
void IO::Drivers::Iec104Proto::Connection::noteConfirmation(UFunction function) noexcept
{
  if (isConfirmation(function))
    m_controlPendingMs = kNotArmed;
}

//--------------------------------------------------------------------------------------------------
// Counters and helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Frames refused for a bad start byte, an impossible length or an unknown control octet.
 */
quint64 IO::Drivers::Iec104Proto::Connection::malformedFrames() const noexcept
{
  return m_malformed;
}

/**
 * @brief Frames refused because a sequence number did not follow the one expected.
 */
quint64 IO::Drivers::Iec104Proto::Connection::sequenceErrors() const noexcept
{
  return m_sequenceErrors;
}

/**
 * @brief APDUs this side has encoded since construction.
 */
quint64 IO::Drivers::Iec104Proto::Connection::framesSent() const noexcept
{
  return m_framesSent;
}

/**
 * @brief APDUs whose header decoded, including the ones a sequence check then refused.
 */
quint64 IO::Drivers::Iec104Proto::Connection::framesReceived() const noexcept
{
  return m_framesReceived;
}

/**
 * @brief True for the three U-format confirmations.
 */
bool IO::Drivers::Iec104Proto::Connection::isConfirmation(UFunction function) noexcept
{
  return function == UFunction::StartDtCon || function == UFunction::StopDtCon
      || function == UFunction::TestFrCon;
}

/**
 * @brief Returns the confirmation a peer owes for @p function, or None when it owes nothing.
 */
IO::Drivers::Iec104Proto::UFunction IO::Drivers::Iec104Proto::Connection::confirmationFor(
  UFunction function) noexcept
{
  switch (function) {
    case UFunction::StartDtAct:
      return UFunction::StartDtCon;
    case UFunction::StopDtAct:
      return UFunction::StopDtCon;
    case UFunction::TestFrAct:
      return UFunction::TestFrCon;
    case UFunction::None:
    case UFunction::StartDtCon:
    case UFunction::StopDtCon:
    case UFunction::TestFrCon:
      break;
  }

  return UFunction::None;
}
