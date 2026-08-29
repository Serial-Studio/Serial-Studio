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
#include <QList>
#include <QString>
#include <QVariant>

#include "IO/Drivers/S7Address.h"

namespace IO {
namespace Drivers {
namespace S7Comm {

inline constexpr std::uint8_t kProtocolId    = 0x32;
inline constexpr std::uint8_t kRosctrJob     = 0x01;
inline constexpr std::uint8_t kRosctrAckData = 0x03;

inline constexpr std::uint8_t kFunctionSetup   = 0xF0;
inline constexpr std::uint8_t kFunctionReadVar = 0x04;

inline constexpr std::uint8_t kSpecificationAny = 0x12;
inline constexpr std::uint8_t kAnyLengthBytes   = 0x0A;
inline constexpr std::uint8_t kSyntaxAny        = 0x10;
inline constexpr std::uint8_t kTransportBit     = 0x01;
inline constexpr std::uint8_t kTransportByte    = 0x02;

inline constexpr std::uint8_t kResultBit     = 0x03;
inline constexpr std::uint8_t kResultBits    = 0x04;
inline constexpr std::uint8_t kResultOctets  = 0x09;
inline constexpr std::uint8_t kReturnSuccess = 0xFF;

inline constexpr int kJobHeaderBytes   = 10;
inline constexpr int kAckHeaderBytes   = 12;
inline constexpr int kReadParamBytes   = 2;
inline constexpr int kSetupParamBytes  = 8;
inline constexpr int kRequestItemBytes = 12;
inline constexpr int kResultItemBytes  = 4;

inline constexpr int kRequestedPduBytes  = 960;
inline constexpr int kMinPduBytes        = 240;
inline constexpr int kMaxPduBytes        = 8192;
inline constexpr int kMaxItemsPerRequest = 20;
inline constexpr quint16 kMaxAmqCount    = 1;

/**
 * @brief Outcome of one decode attempt against a received protocol data unit.
 */
enum class PduResult : std::uint8_t {
  Ok        = 0,
  Truncated = 1,
  Malformed = 2,
  Refused   = 3,
};

/**
 * @brief One variable as the read service addresses it: an area, an element count and a start
 *        address counted in BITS, which is the one addressing unit the wire carries.
 */
struct ReadItem {
  std::uint8_t area    = 0;
  bool bitAccess       = false;
  quint16 dbNumber     = 0;
  quint16 elementCount = 0;
  quint32 startAddress = 0;
};

/**
 * @brief A run of consecutive variables that one request/response pair can carry.
 */
struct Chunk {
  int first = 0;
  int count = 0;
};

/**
 * @brief One item of a read response, addressed INSIDE the caller's buffer rather than copied.
 */
struct ReadResult {
  std::uint8_t returnCode = 0;
  qsizetype offset        = 0;
  qsizetype size          = 0;
};

/**
 * @brief The S7 protocol header: what the message is, which request it answers, and where its
 *        parameter and data sections start inside the buffer that carried it.
 */
struct PduHeader {
  std::uint8_t rosctr       = 0;
  std::uint8_t errorClass   = 0;
  std::uint8_t errorCode    = 0;
  quint16 reference         = 0;
  quint16 parameterBytes    = 0;
  quint16 dataBytes         = 0;
  qsizetype parameterOffset = 0;
  qsizetype dataOffset      = 0;
};

[[nodiscard]] PduResult parseHeader(QByteArrayView pdu, PduHeader& out) noexcept;
[[nodiscard]] ReadItem itemForAddress(const S7Address::Address& address) noexcept;
[[nodiscard]] int resultBytes(const ReadItem& item) noexcept;
[[nodiscard]] QString errorText(std::uint8_t errorClass, std::uint8_t errorCode);
[[nodiscard]] QString returnCodeText(std::uint8_t returnCode);
[[nodiscard]] QVariant decodeValue(const S7Address::Address& address, QByteArrayView raw);

/**
 * @brief The S7 application layer: the setup negotiation that fixes the message budget, the read
 *        service that budget then chunks the variable list against, and the response walk that
 *        turns controller bytes back into per-item results. Qt-Core-only and QObject-free, so the
 *        whole service runs in the unit tier with no socket; refusals are counted, never emitted.
 */
class PduCodec {
public:
  explicit PduCodec();

  void reset() noexcept;

  [[nodiscard]] int pduBytes() const noexcept;
  [[nodiscard]] quint16 nextReference() noexcept;
  [[nodiscard]] QString lastError() const;
  [[nodiscard]] quint64 malformedPdus() const noexcept;
  [[nodiscard]] quint64 refusedItems() const noexcept;
  [[nodiscard]] quint64 oversizedItems() const noexcept;

  [[nodiscard]] QByteArray buildSetupRequest(quint16 reference) const;
  [[nodiscard]] PduResult parseSetupResponse(QByteArrayView pdu);

  [[nodiscard]] Chunk nextChunk(const QList<ReadItem>& items, int first);
  [[nodiscard]] QList<Chunk> planChunks(const QList<ReadItem>& items);
  [[nodiscard]] QByteArray buildReadRequest(quint16 reference,
                                            const QList<ReadItem>& items,
                                            const Chunk& chunk) const;
  [[nodiscard]] PduResult parseReadResponse(QByteArrayView pdu,
                                            int expected,
                                            QList<ReadResult>& out);

private:
  int m_pduBytes;
  quint16 m_reference;
  QString m_lastError;
  quint64 m_malformed;
  quint64 m_refusedItems;
  quint64 m_oversized;
};

}  // namespace S7Comm
}  // namespace Drivers
}  // namespace IO
