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

#include <QApplication>
#include <QTest>

#include "IO/ConnectionManager/ConnectFanOut.h"

// ConnectFanOut queries no device and emits nothing, so every case here is the bookkeeping alone:
// one request that opens N devices concludes exactly once, a pending dial verdict is claimed by
// exactly one caller, and a state is published only when it moved. The wait cursor is QApplication
// state, which is why the suite runs under a widgets application.

/**
 * @brief The connect-request bookkeeping of IO::ConnectFanOut: request lifecycle, pending dial
 *        verdicts and the latched connected/connecting state the manager publishes from.
 */
class TstConnectFanOut : public QObject {
  Q_OBJECT

private slots:
  void startsIdle();
  void concludeNeedsARequest();
  void fanOutHoldsTheRequestOpen();
  void requestConcludesExactlyOnce();

  void pendingDialIsClaimedOnce();
  void pendingDialsAreIndependent();
  void unknownPendingDialIsNotClaimed();

  void connectedStateReportsOnlyTransitions();
  void connectedCountIsPartOfTheState();
  void connectingStateReportsOnlyTransitions();

  void waitCursorIsRaisedOnce();
};

//--------------------------------------------------------------------------------------------------
// Request lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief A fresh object has no request in flight, so the button reads "not connecting".
 */
void TstConnectFanOut::startsIdle()
{
  IO::ConnectFanOut fanOut;

  QVERIFY(!fanOut.requestPending());
}

/**
 * @brief Concluding without a request is a no-op: a per-device conclude raised outside a request
 *        must not restore a cursor nobody raised.
 */
void TstConnectFanOut::concludeNeedsARequest()
{
  IO::ConnectFanOut fanOut;

  QVERIFY(!fanOut.concludeRequest());
}

/**
 * @brief While the fan-out runs, a per-device conclude cannot settle the request: the remaining
 *        devices of the same request have not been visited yet.
 */
void TstConnectFanOut::fanOutHoldsTheRequestOpen()
{
  IO::ConnectFanOut fanOut;
  fanOut.beginRequest();

  QVERIFY(fanOut.requestPending());
  QVERIFY(!fanOut.concludeRequest());
  QVERIFY(fanOut.requestPending());

  fanOut.endFanOut();
  QVERIFY(fanOut.concludeRequest());
  QVERIFY(!fanOut.requestPending());
}

/**
 * @brief Only one caller closes a request, however many devices it opened; that caller is the one
 *        that restores the cursor and publishes the state.
 */
void TstConnectFanOut::requestConcludesExactlyOnce()
{
  IO::ConnectFanOut fanOut;
  fanOut.beginRequest();
  fanOut.endFanOut();

  QVERIFY(fanOut.concludeRequest());
  QVERIFY(!fanOut.concludeRequest());
  QVERIFY(!fanOut.concludeRequest());
}

//--------------------------------------------------------------------------------------------------
// Pending dial verdicts
//--------------------------------------------------------------------------------------------------

/**
 * @brief A device that owes an asynchronous verdict is claimed exactly once, which is what keeps
 *        the same attempt from being reported twice.
 */
void TstConnectFanOut::pendingDialIsClaimedOnce()
{
  IO::ConnectFanOut fanOut;
  fanOut.notePendingDial(3);

  QVERIFY(fanOut.takePendingDial(3));
  QVERIFY(!fanOut.takePendingDial(3));
}

/**
 * @brief Two sources dialing at once own separate verdicts: settling one leaves the other pending.
 */
void TstConnectFanOut::pendingDialsAreIndependent()
{
  IO::ConnectFanOut fanOut;
  fanOut.notePendingDial(0);
  fanOut.notePendingDial(1);

  QVERIFY(fanOut.takePendingDial(1));
  QVERIFY(fanOut.takePendingDial(0));
  QVERIFY(!fanOut.takePendingDial(1));
}

/**
 * @brief Claiming a verdict nobody owes is the user-cancel case every caller ignores.
 */
void TstConnectFanOut::unknownPendingDialIsNotClaimed()
{
  IO::ConnectFanOut fanOut;

  QVERIFY(!fanOut.takePendingDial(7));
}

//--------------------------------------------------------------------------------------------------
// Published state
//--------------------------------------------------------------------------------------------------

/**
 * @brief The connected latch reports a move and nothing else, so asking twice never produces a
 *        duplicate connectedChanged().
 */
void TstConnectFanOut::connectedStateReportsOnlyTransitions()
{
  IO::ConnectFanOut fanOut;

  QVERIFY(fanOut.noteConnected(true, 1));
  QVERIFY(!fanOut.noteConnected(true, 1));
  QVERIFY(fanOut.noteConnected(false, 0));
  QVERIFY(!fanOut.noteConnected(false, 0));
}

/**
 * @brief A second source opening under an already-connected session is a real transition: the
 *        open-device count is part of the published state.
 */
void TstConnectFanOut::connectedCountIsPartOfTheState()
{
  IO::ConnectFanOut fanOut;

  QVERIFY(fanOut.noteConnected(true, 1));
  QVERIFY(fanOut.noteConnected(true, 2));
  QVERIFY(!fanOut.noteConnected(true, 2));
}

/**
 * @brief Same rule for the dialing flag that drives the "Connecting..." label.
 */
void TstConnectFanOut::connectingStateReportsOnlyTransitions()
{
  IO::ConnectFanOut fanOut;

  QVERIFY(fanOut.noteConnecting(true));
  QVERIFY(!fanOut.noteConnecting(true));
  QVERIFY(fanOut.noteConnecting(false));
}

//--------------------------------------------------------------------------------------------------
// Wait cursor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two overlapping requests cannot stack the wait cursor, and a cancel cannot pop one this
 *        object never raised.
 */
void TstConnectFanOut::waitCursorIsRaisedOnce()
{
  IO::ConnectFanOut fanOut;
  fanOut.endWaitCursor();

  fanOut.beginWaitCursor();
  fanOut.beginWaitCursor();
  fanOut.endWaitCursor();
  fanOut.endWaitCursor();

  QVERIFY(QApplication::overrideCursor() == nullptr);
}

QTEST_MAIN(TstConnectFanOut)

#include "tst_connect_fanout.moc"
