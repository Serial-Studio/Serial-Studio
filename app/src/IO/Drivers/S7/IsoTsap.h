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
 * @brief In-house S7comm client stack (spec 0073), split into the ISO-on-TCP transport here and
 *        the S7 protocol data units in S7Pdu.h. Both halves are Qt-Core-only, QObject-free and own
 *        no socket: they turn bytes into bytes against a buffer the caller supplies, which is what
 *        lets the ctest tier drive the whole protocol with no controller and no event loop.
 */
namespace S7Comm {

inline constexpr std::uint8_t kTpktVersion = 0x03;

inline constexpr int kIsoTsapPort     = 102;
inline constexpr int kTpktHeaderBytes = 4;
inline constexpr int kMinTpktBytes    = 6;
inline constexpr int kMaxTpktBytes    = 65535;

inline constexpr std::uint8_t kTpduConnectRequest = 0xE0;
inline constexpr std::uint8_t kTpduConnectConfirm = 0xD0;
inline constexpr std::uint8_t kTpduData           = 0xF0;
inline constexpr std::uint8_t kEndOfTransmission  = 0x80;

inline constexpr std::uint8_t kParamTpduSize    = 0xC0;
inline constexpr std::uint8_t kParamCallingTsap = 0xC1;
inline constexpr std::uint8_t kParamCalledTsap  = 0xC2;

inline constexpr std::uint8_t kTpduSizeCode  = 0x0A;
inline constexpr std::uint8_t kMinTpduCode   = 0x07;
inline constexpr std::uint8_t kMaxTpduCode   = 0x0D;
inline constexpr std::uint8_t kCallingTsapHi = 0x01;
inline constexpr std::uint8_t kCalledTsapHi  = 0x03;

inline constexpr int kConnectTpduBytes   = 18;
inline constexpr int kDataTpduBytes      = 3;
inline constexpr int kConnectHeaderBytes = 7;
inline constexpr int kMaxParameters      = 16;

inline constexpr int kMinTpduBytes     = 128;
inline constexpr int kMaxTpduBytes     = 8192;
inline constexpr int kMaxAssemblyBytes = 16384;

inline constexpr int kMaxRack = 7;
inline constexpr int kMaxSlot = 31;

/**
 * @brief Outcome of one decode attempt against the receive buffer.
 */
enum class TpktResult : std::uint8_t {
  Ok        = 0,
  NeedMore  = 1,
  Malformed = 2,
};

/**
 * @brief One complete TPKT located INSIDE the caller's receive buffer. @c totalBytes is what the
 *        caller must drop; @c tpduOffset and @c tpduSize address the COTP TPDU without copying it.
 */
struct Tpkt {
  qsizetype totalBytes = 0;
  qsizetype tpduOffset = 0;
  qsizetype tpduSize   = 0;
};

/**
 * @brief The ISO-on-TCP transport: RFC 1006 TPKT framing, the ISO 8073 class-0 connection
 *        handshake and the data TPDUs an S7 protocol data unit rides in. It performs no I/O and
 *        holds no receive buffer of its own (one is handed in on every decode), so a hostile or
 *        half-arrived stream is driven in the unit tier exactly as a socket delivers it.
 */
class Transport {
public:
  explicit Transport();

  void reset() noexcept;

  [[nodiscard]] int tpduBytes() const noexcept;
  [[nodiscard]] quint64 framesSent() const noexcept;
  [[nodiscard]] quint64 framesReceived() const noexcept;
  [[nodiscard]] quint64 malformedFrames() const noexcept;

  [[nodiscard]] QByteArray wrapData(QByteArrayView payload);
  [[nodiscard]] QByteArray buildConnectRequest(int rack, int slot);
  [[nodiscard]] bool parseConnectConfirm(QByteArrayView tpdu);

  [[nodiscard]] TpktResult extractTpkt(QByteArrayView buffer, Tpkt& out);
  [[nodiscard]] TpktResult acceptData(QByteArrayView tpdu, QByteArray& assembly, bool& complete);

  [[nodiscard]] static std::uint8_t calledTsapLow(int rack, int slot) noexcept;

private:
  [[nodiscard]] QByteArray frameTpkt(QByteArrayView tpdu);

  int m_tpduBytes;
  quint16 m_sourceReference;
  quint64 m_framesSent;
  quint64 m_framesReceived;
  quint64 m_malformed;
};

}  // namespace S7Comm
}  // namespace Drivers
}  // namespace IO
