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

#pragma once

#include <chrono>
#include <optional>
#include <QByteArray>
#include <QByteArrayView>
#include <QHash>
#include <QtGlobal>

namespace IO {
namespace Drivers {

/**
 * @brief Fixed caps for both CAN reassemblers (spec 0073 R9/R11). The bus decides how many
 *        sessions open and how large each claims to be, so every container here has a ceiling
 *        that is checked before growth and never raised on demand: a session past a cap is
 *        dropped whole and counted, never truncated into a half-decoded frame.
 */
namespace CanReassemblyLimits {
inline constexpr int kMaxSessions         = 16;
inline constexpr int kJ1939MaxBytes       = 1785;
inline constexpr int kJ1939MaxPackets     = 255;
inline constexpr int kIsoTpMaxBytes       = 4095;
inline constexpr qint64 kSessionTimeoutMs = 750;
}  // namespace CanReassemblyLimits

/**
 * @brief Pulled diagnostic counters (spec 0033): plain integers incremented in place and read on
 *        the owner's own cadence, so no drop is silent and nothing signals, allocates or locks
 *        per frame.
 */
struct CanReassemblyCounters {
  quint64 aborted;
  quint64 timeouts;
  quint64 completed;
  quint64 malformed;
  quint64 sizeOverruns;
  quint64 sequenceErrors;
  quint64 sessionOverruns;

  /**
   * @brief Starts every counter at zero.
   */
  CanReassemblyCounters()
    : aborted(0)
    , timeouts(0)
    , completed(0)
    , malformed(0)
    , sizeOverruns(0)
    , sequenceErrors(0)
    , sessionOverruns(0)
  {}
};

/**
 * @brief J1939-21 transport-protocol reassembler: TP.CM (PGN 0xEC00) announcements plus their
 *        TP.DT (PGN 0xEB00) packets, rebuilt into the long message the PGN names. An RTS opens a
 *        session even with no CTS observed, because Serial Studio only watches the bus (spec 0073
 *        open question, listen-only). Sessions key on (source, destination). Qt Core, no QObject.
 */
class J1939TransportReassembler {
public:
  using SteadyClock     = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  /**
   * @brief One fully reassembled transport message. @c firstSeen is the capture time of the
   *        announcing TP.CM packet, not of the packet that completed the session: the source owns
   *        time and the message began when its first byte reached the adapter.
   */
  struct Completed {
    QByteArray bytes;
    SteadyTimePoint firstSeen;
    quint32 pgn;
    quint8 priority;
    quint8 sourceAddr;

    /**
     * @brief Builds an empty result carrying the J1939 default priority.
     */
    Completed() : pgn(0), priority(7), sourceAddr(0) {}
  };

  void reset();

  [[nodiscard]] int activeSessions() const noexcept;
  [[nodiscard]] static bool isTransportFrame(quint32 id29) noexcept;
  [[nodiscard]] static quint32 parameterGroupNumber(quint32 id29) noexcept;
  [[nodiscard]] const CanReassemblyCounters& counters() const noexcept;
  [[nodiscard]] std::optional<Completed> feed(quint32 id29,
                                              QByteArrayView payload,
                                              SteadyTimePoint stamp);

private:
  /**
   * @brief One in-flight transfer: what the announcement promised and what has arrived.
   */
  struct Session {
    QByteArray bytes;
    SteadyTimePoint firstSeen;
    SteadyTimePoint lastSeen;
    quint32 pgn;
    int totalBytes;
    int totalPackets;
    int nextSequence;
    quint8 priority;

    /**
     * @brief Builds an empty session expecting the first (1-based) data packet.
     */
    Session() : pgn(0), totalBytes(0), totalPackets(0), nextSequence(1), priority(7) {}
  };

  [[nodiscard]] static quint16 sessionKey(quint8 source, quint8 destination) noexcept;

  void dropSession(quint16 key, quint64& counter);
  void evictExpired(SteadyTimePoint stamp);
  void openSession(quint16 key, quint8 priority, QByteArrayView payload, SteadyTimePoint stamp);
  void handleConnectionManagement(quint8 source,
                                  quint8 destination,
                                  quint8 priority,
                                  QByteArrayView payload,
                                  SteadyTimePoint stamp);

  [[nodiscard]] std::optional<Completed> handleDataTransfer(quint8 source,
                                                            quint8 destination,
                                                            QByteArrayView payload,
                                                            SteadyTimePoint stamp);

private:
  QHash<quint16, Session> m_sessions;
  CanReassemblyCounters m_counters;
};

/**
 * @brief ISO 15765-2 (ISO-TP) reassembler: joins a FirstFrame and its ConsecutiveFrames into one
 *        diagnostic message. FlowControl is observed, never sent. Only the ISO 15765-4 ranges are
 *        inspected (11-bit 0x7E0-0x7EF, 29-bit 0x18DAxxyy / 0x18DBxxyy), so traffic whose first
 *        nibble reads as a PCI is never misread. SingleFrames stay with the caller, unchanged.
 */
class IsoTpReassembler {
public:
  using SteadyClock     = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  /**
   * @brief One fully reassembled diagnostic message, stamped with the FirstFrame capture time and
   *        carrying the CAN identifier it arrived on so the caller can rebuild a frame header.
   */
  struct Completed {
    QByteArray bytes;
    SteadyTimePoint firstSeen;
    quint32 canId;
    bool extendedId;

    /**
     * @brief Builds an empty result on the standard-format identifier zero.
     */
    Completed() : canId(0), extendedId(false) {}
  };

  void reset();

  [[nodiscard]] int activeSessions() const noexcept;
  [[nodiscard]] static bool isMultiFrame(QByteArrayView payload) noexcept;
  [[nodiscard]] static bool isDiagnosticId(quint32 canId, bool extendedId) noexcept;
  [[nodiscard]] const CanReassemblyCounters& counters() const noexcept;
  [[nodiscard]] std::optional<Completed> feed(quint32 canId,
                                              bool extendedId,
                                              QByteArrayView payload,
                                              SteadyTimePoint stamp);

private:
  /**
   * @brief One in-flight diagnostic message and the sequence number it expects next.
   */
  struct Session {
    QByteArray bytes;
    SteadyTimePoint firstSeen;
    SteadyTimePoint lastSeen;
    int totalBytes;
    int nextSequence;
    bool extendedId;

    /**
     * @brief Builds an empty session expecting the first ConsecutiveFrame.
     */
    Session() : totalBytes(0), nextSequence(1), extendedId(false) {}
  };

  void evictExpired(SteadyTimePoint stamp);
  void startFirstFrame(quint32 canId,
                       bool extendedId,
                       QByteArrayView payload,
                       SteadyTimePoint stamp);

  [[nodiscard]] std::optional<Completed> continueFrame(quint32 canId,
                                                       QByteArrayView payload,
                                                       SteadyTimePoint stamp);

private:
  QHash<quint32, Session> m_sessions;
  CanReassemblyCounters m_counters;
};

}  // namespace Drivers
}  // namespace IO
