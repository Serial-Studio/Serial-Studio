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

#include <QSet>

namespace IO {

/**
 * @brief Bookkeeping for one connect request: the in-flight and fan-out flags, the devices that
 *        owe an asynchronous dial verdict, the wait cursor, and the last connected/connecting
 *        state published. It queries no device and emits nothing -- ConnectionManager keeps the
 *        device map and every Q_EMIT, so the spec-0050 verdict lands where the drivers are wired.
 */
class ConnectFanOut {
public:
  ConnectFanOut();
  ConnectFanOut(ConnectFanOut&&)                 = delete;
  ConnectFanOut(const ConnectFanOut&)            = delete;
  ConnectFanOut& operator=(ConnectFanOut&&)      = delete;
  ConnectFanOut& operator=(const ConnectFanOut&) = delete;

  void endFanOut() noexcept;
  void beginRequest() noexcept;
  void notePendingDial(int deviceId);

  void endWaitCursor();
  void beginWaitCursor();

  [[nodiscard]] bool concludeRequest() noexcept;
  [[nodiscard]] bool takePendingDial(int deviceId);
  [[nodiscard]] bool requestPending() const noexcept;

  [[nodiscard]] bool noteConnecting(bool connecting) noexcept;
  [[nodiscard]] bool noteConnected(bool connected, int deviceCount) noexcept;

private:
  bool m_fanOut;
  bool m_pending;
  bool m_waitCursorActive;
  bool m_lastConnectedState;
  bool m_lastConnectingState;
  int m_lastConnectedCount;
  QSet<int> m_pendingDialVerdicts;
};

}  // namespace IO
