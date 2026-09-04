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

#pragma once

#include <cstdint>
#include <QByteArray>
#include <QByteArrayView>

namespace IO {
namespace Drivers {

/**
 * @brief In-house IEC 60870-5-104 stack (spec 0073), split into the APCI transport layer here and
 *        the ASDU application layer in Asdu.h. Both halves are Qt-Core-only, QObject-free and own
 *        no timer: they consume and produce bytes against an EXTERNAL clock, which is what lets
 *        the ctest tier drive the whole protocol with no socket and no event loop.
 */
namespace Iec104Proto {

inline constexpr std::uint8_t kStartByte = 0x68;
inline constexpr int kControlBytes       = 4;
inline constexpr int kApciBytes          = 6;
inline constexpr int kMinApduLength      = 4;
inline constexpr int kMaxApduLength      = 253;
inline constexpr int kMaxAsduBytes       = kMaxApduLength - kControlBytes;
inline constexpr int kSequenceModulo     = 32768;

inline constexpr int kDefaultPort = 2404;
inline constexpr int kDefaultK    = 12;
inline constexpr int kDefaultW    = 8;
inline constexpr int kDefaultT1Ms = 15000;
inline constexpr int kDefaultT2Ms = 10000;
inline constexpr int kDefaultT3Ms = 20000;

inline constexpr int kMinWindow = 1;
inline constexpr int kMaxWindow = 32767;
inline constexpr int kMinTimeMs = 1000;
inline constexpr int kMaxTimeMs = 255000;

/**
 * @brief APDU shapes the control field selects: information transfer, supervisory acknowledgement
 *        and unnumbered control.
 */
enum class FrameType : std::uint8_t {
  Information = 0,
  Supervisory = 1,
  Unnumbered  = 2,
  Invalid     = 255,
};

/**
 * @brief U-format control functions; the numeric value is the first control octet, so the two
 *        low bits that mark the frame unnumbered are already part of it.
 */
enum class UFunction : std::uint8_t {
  None       = 0x00,
  StartDtAct = 0x07,
  StartDtCon = 0x0B,
  StopDtAct  = 0x13,
  StopDtCon  = 0x23,
  TestFrAct  = 0x43,
  TestFrCon  = 0x83,
};

/**
 * @brief Outcome of one decode attempt against the receive buffer.
 */
enum class ParseResult : std::uint8_t {
  Ok         = 0,
  NeedMore   = 1,
  OutOfOrder = 2,
  Malformed  = 3,
};

/**
 * @brief One decoded APDU. @c asduOffset and @c asduSize address the payload INSIDE the caller's
 *        buffer rather than copying it, so a decoded frame costs no allocation.
 */
struct Apdu {
  FrameType type       = FrameType::Invalid;
  UFunction function   = UFunction::None;
  int sendSeq          = 0;
  int recvSeq          = 0;
  qsizetype apduSize   = 0;
  qsizetype asduOffset = 0;
  qsizetype asduSize   = 0;
};

/**
 * @brief The APCI connection state machine: sequence numbers, the k/w windows and the t1/t2/t3
 *        deadlines. It owns NO QTimer -- every mutator takes the caller's monotonic millisecond
 *        clock and every deadline is answered as a "is this due now" query, so the whole timing
 *        contract is exercised against a fake clock in tst_iec104_apci.
 */
class Connection {
public:
  explicit Connection();

  void reset(qint64 nowMs) noexcept;
  void configure(int k, int w, int t1Ms, int t2Ms, int t3Ms) noexcept;

  [[nodiscard]] int windowK() const noexcept;
  [[nodiscard]] int windowW() const noexcept;
  [[nodiscard]] int t1Ms() const noexcept;
  [[nodiscard]] int t2Ms() const noexcept;
  [[nodiscard]] int t3Ms() const noexcept;

  [[nodiscard]] int sendSeq() const noexcept;
  [[nodiscard]] int recvSeq() const noexcept;
  [[nodiscard]] int ackedSeq() const noexcept;
  [[nodiscard]] int outstanding() const noexcept;
  [[nodiscard]] int unackedReceived() const noexcept;

  [[nodiscard]] QByteArray encodeUnnumbered(UFunction function, qint64 nowMs);
  [[nodiscard]] QByteArray encodeSupervisory(qint64 nowMs);
  [[nodiscard]] QByteArray encodeInformation(QByteArrayView asdu, qint64 nowMs);

  [[nodiscard]] ParseResult consume(QByteArrayView buffer, Apdu& out, qint64 nowMs);

  [[nodiscard]] bool sendWindowOpen() const noexcept;
  [[nodiscard]] bool ackDue(qint64 nowMs) const noexcept;
  [[nodiscard]] bool testDue(qint64 nowMs) const noexcept;
  [[nodiscard]] bool confirmOverdue(qint64 nowMs) const noexcept;

  [[nodiscard]] quint64 malformedFrames() const noexcept;
  [[nodiscard]] quint64 sequenceErrors() const noexcept;
  [[nodiscard]] quint64 framesSent() const noexcept;
  [[nodiscard]] quint64 framesReceived() const noexcept;

  [[nodiscard]] static bool isConfirmation(UFunction function) noexcept;
  [[nodiscard]] static UFunction confirmationFor(UFunction function) noexcept;

private:
  [[nodiscard]] ParseResult acceptInformation(QByteArrayView buffer, Apdu& out, qint64 nowMs);
  [[nodiscard]] ParseResult acceptSupervisory(QByteArrayView buffer, Apdu& out, qint64 nowMs);
  [[nodiscard]] ParseResult acceptUnnumbered(QByteArrayView buffer, Apdu& out);
  [[nodiscard]] bool acknowledge(int recvSeq, qint64 nowMs) noexcept;
  void noteConfirmation(UFunction function) noexcept;

  int m_k;
  int m_w;
  int m_t1;
  int m_t2;
  int m_t3;
  int m_sendSeq;
  int m_recvSeq;
  int m_ackedSeq;
  int m_unackedRx;
  qint64 m_lastTrafficMs;
  qint64 m_firstUnackedRxMs;
  qint64 m_sendPendingMs;
  qint64 m_controlPendingMs;
  quint64 m_malformed;
  quint64 m_sequenceErrors;
  quint64 m_framesSent;
  quint64 m_framesReceived;
};

}  // namespace Iec104Proto
}  // namespace Drivers
}  // namespace IO
