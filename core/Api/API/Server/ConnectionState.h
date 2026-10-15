/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#pragma once

#include <deque>
#include <QByteArray>
#include <QElapsedTimer>
#include <QSet>
#include <QString>

#include "Core/DataModel/DataBlock.h"

namespace API {

/**
 * @brief Everything the server knows about one connected API client. Owned by Server (which keys
 *        it by socket pointer) and mutated in place by the reception machine and by the
 *        connection-scoped mirror and stream verbs, so it lives in its own header rather than
 *        inside any one of them.
 */
struct ConnectionState {
  QString sessionId;
  QString peerAddress;
  quint16 peerPort = 0;
  QByteArray buffer;

  // Newline-less JSON scan over buffer[0, jsonScanned): bracket depth, inside a string, after '\'
  qsizetype jsonScanned = 0;
  int jsonDepth         = 0;
  bool jsonInString     = false;
  bool jsonEscaped      = false;

  QElapsedTimer window;
  int messageCount   = 0;
  int byteCount      = 0;
  bool authenticated = false;
  int authAttempts   = 0;

  // Server-initiated disconnect requested; consumeBytes then drops further bytes (spec 0099 f8)
  bool closing = false;

  // Raw forwarding waits for one valid JSON message; the first chunk is sniffed once (spec 0075 I2)
  bool handshakeSeen  = false;
  bool firstBytesSeen = false;

  // Mirror state; streamFrames defaults true so an unmodified client sees no change at all
  bool streamFrames     = true;
  bool mirrorSubscribed = false;
  int mirrorHz          = 20;
  int mirrorPrecision   = 0;

  // Typed stream-block subscription (spec 0051 M6): ack-paced, drop-oldest, counted
  bool streamSubscribed    = false;
  bool streamWriteInFlight = false;
  QSet<int> streamSources;
  quint64 streamSeq    = 0;
  quint64 streamMissed = 0;
  std::deque<DataModel::DataBlockPtr> streamPending;

  /**
   * @brief Forgets the JSON scan. It describes a prefix of the buffer, so every buffer edit other
   *        than an append must drop it or the next scan would resume at a stale offset.
   */
  void resetJsonScan()
  {
    jsonScanned  = 0;
    jsonDepth    = 0;
    jsonInString = false;
    jsonEscaped  = false;
  }

  /**
   * @brief Empties the receive buffer together with the scan that describes it.
   */
  void clearBuffer()
  {
    buffer.clear();
    resetJsonScan();
  }

  /**
   * @brief Removes the first @p count bytes (a framed line) and the scan that described them.
   */
  void dropFront(const qsizetype count)
  {
    buffer.remove(0, count);
    resetJsonScan();
  }
};

}  // namespace API
