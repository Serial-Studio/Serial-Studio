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

#include "IO/ConnectionManager/ConnectFanOut.h"

#include <QApplication>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the bookkeeping with no request in flight and nothing published yet, so the
 *        first real transition is always reported.
 */
IO::ConnectFanOut::ConnectFanOut()
  : m_fanOut(false)
  , m_pending(false)
  , m_waitCursorActive(false)
  , m_lastConnectedState(false)
  , m_lastConnectingState(false)
  , m_lastConnectedCount(0)
{}

//--------------------------------------------------------------------------------------------------
// Request lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a connect request and marks its fan-out as running, so the per-device concludes
 *        raised while opening cannot settle the request before every device was visited.
 */
void IO::ConnectFanOut::beginRequest() noexcept
{
  m_pending = true;
  m_fanOut  = true;
}

/**
 * @brief Marks the fan-out finished; the request itself concludes on the next attempt.
 */
void IO::ConnectFanOut::endFanOut() noexcept
{
  m_fanOut = false;
}

/**
 * @brief True while a connect request is in flight. toggleConnection() treats this as connected,
 *        so the button aborts the attempt instead of stacking a second one on top of it.
 */
bool IO::ConnectFanOut::requestPending() const noexcept
{
  return m_pending;
}

/**
 * @brief Closes the request if one is open and its fan-out is over, returning whether this call
 *        is the one that closed it. Only that caller restores the cursor and publishes the state,
 *        so a request that opened N devices still concludes exactly once.
 */
bool IO::ConnectFanOut::concludeRequest() noexcept
{
  if (!m_pending || m_fanOut)
    return false;

  m_pending = false;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Wait cursor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Raises the wait cursor at most once, so two overlapping requests cannot stack it. This
 *        is QApplication state, which is what pins the whole class to the GUI thread.
 */
void IO::ConnectFanOut::beginWaitCursor()
{
  if (m_waitCursorActive)
    return;

  m_waitCursorActive = true;
  QApplication::setOverrideCursor(Qt::WaitCursor);
}

/**
 * @brief Restores the wait cursor if this object raised it, so neither a late completion nor a
 *        cancel can leave it up or pop a cursor it does not own.
 */
void IO::ConnectFanOut::endWaitCursor()
{
  if (!m_waitCursorActive)
    return;

  m_waitCursorActive = false;
  QApplication::restoreOverrideCursor();
}

//--------------------------------------------------------------------------------------------------
// Pending dial verdicts
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records that @p deviceId owes an asynchronous dial verdict through openFinished().
 */
void IO::ConnectFanOut::notePendingDial(int deviceId)
{
  SS_ASSERT(deviceId >= 0, return);

  m_pendingDialVerdicts.insert(deviceId);
}

/**
 * @brief Claims @p deviceId's pending verdict, returning whether this call owns it. The erase
 *        happens before the caller reports, so the notify hop inside that report cannot settle
 *        the same attempt twice, and a report with no pending id is a user cancel every caller
 *        ignores. Any id is accepted: the disconnect paths call this before they know.
 */
bool IO::ConnectFanOut::takePendingDial(int deviceId)
{
  return m_pendingDialVerdicts.remove(deviceId);
}

//--------------------------------------------------------------------------------------------------
// Published state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latches the dialing flag and reports whether it moved, so the caller emits
 *        connectingChanged() only on a real transition.
 */
bool IO::ConnectFanOut::noteConnecting(bool connecting) noexcept
{
  if (m_lastConnectingState == connecting)
    return false;

  m_lastConnectingState = connecting;
  return true;
}

/**
 * @brief Latches the connected flag and the open-device count, reporting whether either moved.
 *        Every lifecycle path funnels through here, so callers never reason about whether some
 *        other path already reported: asking twice is always correct and never produces a
 *        duplicate or contradictory notification.
 */
bool IO::ConnectFanOut::noteConnected(bool connected, int deviceCount) noexcept
{
  SS_ASSERT_LOG(deviceCount >= 0);

  if (m_lastConnectedState == connected && m_lastConnectedCount == deviceCount)
    return false;

  m_lastConnectedState = connected;
  m_lastConnectedCount = deviceCount;
  return true;
}
