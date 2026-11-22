/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include <QByteArray>
#include <QByteArrayView>
#include <QCoreApplication>
#include <QString>
#include <vector>

namespace Console {

/**
 * @brief Bounds of the bus monitor's Modbus RTU analysis (spec 0127 R12-R14).
 */
inline constexpr int kModbusMaxFrameBytes       = 256;
inline constexpr int kModbusTimeoutMinMs        = 10;
inline constexpr int kModbusTimeoutMaxMs        = 10000;
inline constexpr int kModbusTimeoutDefaultMs    = 500;
inline constexpr qint64 kModbusIdleFlushMs      = 100;
inline constexpr qint64 kModbusLateWindowMs     = 5000;
inline constexpr qsizetype kModbusMaxCarryBytes = 4 * kModbusMaxFrameBytes;

/**
 * @brief Who put a message on the line. Master and Slave come from the Modbus analysis; Sent and
 *        Received are the plain directions of the Messages mode.
 */
enum class BusRole : quint8 {
  Unknown,
  Master,
  Slave,
  Sent,
  Received,
};

/**
 * @brief How bad the worst finding on a message is; it picks the row tint.
 */
enum class BusSeverity : quint8 {
  None,
  Info,
  Warning,
  Error,
};

/**
 * @brief Findings on one message, as bit flags so a row can carry several.
 */
enum BusFinding : quint16 {
  FindingNone             = 0,
  FindingCrcFailed        = 1 << 0,
  FindingIncomplete       = 1 << 1,
  FindingNoResponse       = 1 << 2,
  FindingMismatch         = 1 << 3,
  FindingLateResponse     = 1 << 4,
  FindingOrphanResponse   = 1 << 5,
  FindingEchoDiffers      = 1 << 6,
  FindingByteCountWrong   = 1 << 7,
  FindingException        = 1 << 8,
  FindingUnknownException = 1 << 9,
  FindingInvalidRequest   = 1 << 10,
  FindingBroadcastWrite   = 1 << 11,
  FindingUnknownFunction  = 1 << 12,
};

/**
 * @brief One message the monitor shows: when it completed, who sent it, what was found and its
 *        bytes. The sequence number is what a later finding (a timeout) refers back to.
 */
struct BusFrame {
  quint64 sequence = 0;
  qint64 timeMs    = 0;
  quint16 findings = FindingNone;
  BusRole role     = BusRole::Unknown;
  bool crcOk       = false;
  QByteArray bytes;
};

/**
 * @brief A finding added to a message that was already reported: the request a slave never
 *        answered gets its verdict on its own row.
 */
struct BusUpdate {
  quint64 sequence = 0;
  quint16 findings = FindingNone;
};

/**
 * @brief Which register formats the data summary of a read or write shows.
 */
enum BusValueFormat : quint8 {
  FormatU16 = 1 << 0,
  FormatS16 = 1 << 1,
  FormatU32 = 1 << 2,
  FormatS32 = 1 << 3,
  FormatF32 = 1 << 4,
  FormatBin = 1 << 5,
};

[[nodiscard]] BusSeverity busSeverity(quint16 findings) noexcept;
[[nodiscard]] QString busFindingsText(quint16 findings, const BusFrame& frame);
[[nodiscard]] QString modbusFunctionName(quint8 function);
[[nodiscard]] QString modbusDataSummary(const BusFrame& frame, quint8 formats);

/**
 * @brief Listen-only Modbus RTU analysis over a byte stream (spec 0127 R12-R14). The wire's idle
 *        gaps do not survive into a stream, so frames are found by function code and length and
 *        confirmed by CRC, with one-byte resynchronisation after corrupt bytes; each frame is
 *        then labelled master or slave and paired with its counterpart. No I/O, no timer and no
 *        Qt object: the caller feeds bytes with a time and calls tick() so a request followed by
 *        silence still times out.
 */
class ModbusRtuAnalyzer {
  Q_DECLARE_TR_FUNCTIONS(Console::ModbusRtuAnalyzer)

public:
  ModbusRtuAnalyzer();

  [[nodiscard]] int timeoutMs() const noexcept;
  [[nodiscard]] bool awaitingResponse() const noexcept;
  [[nodiscard]] qsizetype bufferedBytes() const noexcept;

  void reset();
  void setTimeoutMs(int timeoutMs);
  void feed(QByteArrayView bytes,
            qint64 nowMs,
            std::vector<BusFrame>& frames,
            std::vector<BusUpdate>& updates);
  void tick(qint64 nowMs, std::vector<BusFrame>& frames, std::vector<BusUpdate>& updates);

private:
  /**
   * @brief The request the analysis is waiting to see answered.
   */
  struct Pending {
    quint64 sequence = 0;
    qint64 timeMs    = 0;
    quint8 slave     = 0;
    quint8 function  = 0;
    bool active      = false;
    QByteArray bytes;
  };

  [[nodiscard]] qsizetype frameLengthAt(qsizetype at, bool final, bool& needMore) const;
  [[nodiscard]] bool countPlausible(quint8 slave, quint8 function, int counted) const noexcept;
  [[nodiscard]] BusRole roleOf(QByteArrayView frame) const noexcept;
  [[nodiscard]] quint16 judgeRequest(QByteArrayView frame) const noexcept;
  [[nodiscard]] quint16 judgeResponse(QByteArrayView frame) const noexcept;

  void scan(qint64 nowMs,
            bool final,
            std::vector<BusFrame>& frames,
            std::vector<BusUpdate>& updates);
  void emitFrame(QByteArrayView frame,
                 qint64 nowMs,
                 std::vector<BusFrame>& frames,
                 std::vector<BusUpdate>& updates);
  void emitGarbage(qint64 nowMs, bool incomplete, std::vector<BusFrame>& frames);
  void expirePending(std::vector<BusUpdate>& updates, quint16 finding);

private:
  QByteArray m_carry;
  QByteArray m_garbage;
  Pending m_pending;
  quint64 m_nextSequence;
  qint64 m_lastRxMs;
  qint64 m_expiredAtMs;
  int m_timeoutMs;
  quint8 m_expiredSlave;
  quint8 m_expiredFunction;
  bool m_hasExpired;
};

}  // namespace Console
