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

#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>

#include "IO/AsyncTcpDial.h"

// Every case runs against a loopback listener this file owns, or against a port it has just
// released, so nothing here depends on the machine's network. The dialer is event-loop driven:
// QSignalSpy::wait() is the only wait, and a case that expects NO verdict waits out a window
// instead of a signal.

/**
 * @brief Binds a listener on an ephemeral loopback port.
 */
static quint16 listenOnLoopback(QTcpServer& server)
{
  const bool listening = server.listen(QHostAddress::LocalHost, 0);
  return listening ? server.serverPort() : 0;
}

/**
 * @brief Verdict contract of IO::AsyncTcpDial: exactly one finished(ok, reason) per start(), one
 *        deadline over resolution/probe/connect, and a cancel that reports nothing.
 */
class TstAsyncTcpDial : public QObject {
  Q_OBJECT

private slots:
  void startsIdle();
  void reportsSuccessOnLiveListener();
  void probeOnlyReportsWithoutASocket();
  void connectsTheCallerSocketOnce();
  void probeDisabledDialsTheSocketDirectly();
  void reportsFailureOnDeadPort();
  void reportsFailureOnUnresolvableHost();
  void cancelReportsNoVerdict();
  void deadlineIsIgnoredWhileActive();
};

//--------------------------------------------------------------------------------------------------
// Idle state
//--------------------------------------------------------------------------------------------------

/**
 * @brief A fresh dialer is idle and holds no resolved address.
 */
void TstAsyncTcpDial::startsIdle()
{
  IO::AsyncTcpDial dial;

  QVERIFY(!dial.active());
  QVERIFY(dial.resolvedAddress().isNull());
  QVERIFY(dial.deadline() > 0);
}

//--------------------------------------------------------------------------------------------------
// Success paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief A listener that is up settles the attempt as successful, exactly once.
 */
void TstAsyncTcpDial::reportsSuccessOnLiveListener()
{
  QTcpServer server;
  const quint16 port = listenOnLoopback(server);
  QVERIFY(port != 0);

  QTcpSocket socket;
  IO::AsyncTcpDial dial;
  QSignalSpy spy(&dial, &IO::AsyncTcpDial::finished);

  dial.start(QStringLiteral("127.0.0.1"), port, &socket);
  QVERIFY(spy.wait(5000));

  QCOMPARE(spy.count(), 1);
  QVERIFY(spy.at(0).at(0).toBool());
  QVERIFY(!dial.active());
}

/**
 * @brief A probe-only attempt settles on the probe's verdict and never needs a caller socket.
 */
void TstAsyncTcpDial::probeOnlyReportsWithoutASocket()
{
  QTcpServer server;
  const quint16 port = listenOnLoopback(server);
  QVERIFY(port != 0);

  IO::AsyncTcpDial dial;
  QSignalSpy spy(&dial, &IO::AsyncTcpDial::finished);

  dial.startProbe(QStringLiteral("127.0.0.1"), port);
  QVERIFY(spy.wait(5000));

  QCOMPARE(spy.count(), 1);
  QVERIFY(spy.at(0).at(0).toBool());
  QCOMPARE(dial.resolvedAddress(), QHostAddress(QHostAddress::LocalHost));
}

/**
 * @brief The caller's socket ends up connected, and the dialer leaves it alone afterwards: the
 *        driver socket dials once and is never aborted or redialed by the helper.
 */
void TstAsyncTcpDial::connectsTheCallerSocketOnce()
{
  QTcpServer server;
  const quint16 port = listenOnLoopback(server);
  QVERIFY(port != 0);

  QTcpSocket socket;
  IO::AsyncTcpDial dial;
  QSignalSpy spy(&dial, &IO::AsyncTcpDial::finished);

  dial.start(QStringLiteral("localhost"), port, &socket);
  QVERIFY(spy.wait(5000));

  QCOMPARE(spy.count(), 1);
  QVERIFY(spy.at(0).at(0).toBool());
  QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);

  QTest::qWait(300);
  QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
  QCOMPARE(spy.count(), 1);
}

/**
 * @brief With the probe off the attempt resolves and connects the caller's socket directly, which
 *        is what a station permitting a single client needs: no second socket ever touches it.
 */
void TstAsyncTcpDial::probeDisabledDialsTheSocketDirectly()
{
  QTcpServer server;
  const quint16 port = listenOnLoopback(server);
  QVERIFY(port != 0);

  QTcpSocket socket;
  IO::AsyncTcpDial dial;
  dial.setProbeEnabled(false);
  QSignalSpy spy(&dial, &IO::AsyncTcpDial::finished);

  dial.start(QStringLiteral("127.0.0.1"), port, &socket);
  QVERIFY(spy.wait(5000));

  QCOMPARE(spy.count(), 1);
  QVERIFY(spy.at(0).at(0).toBool());
  QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
}

//--------------------------------------------------------------------------------------------------
// Failure paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief A port nothing listens on refuses every paced round and fails inside the deadline.
 */
void TstAsyncTcpDial::reportsFailureOnDeadPort()
{
  QTcpServer server;
  const quint16 port = listenOnLoopback(server);
  QVERIFY(port != 0);
  server.close();

  QTcpSocket socket;
  IO::AsyncTcpDial dial;
  dial.setDeadline(1200);
  QSignalSpy spy(&dial, &IO::AsyncTcpDial::finished);

  dial.start(QStringLiteral("127.0.0.1"), port, &socket);
  QVERIFY(spy.wait(6000));

  QCOMPARE(spy.count(), 1);
  QVERIFY(!spy.at(0).at(0).toBool());
  QVERIFY(!spy.at(0).at(1).toString().isEmpty());
  QVERIFY(socket.state() != QAbstractSocket::ConnectedState);
}

/**
 * @brief A name that cannot resolve fails on the lookup rather than hanging: the whole point of
 *        the helper is that the GUI thread never waits on a resolver.
 */
void TstAsyncTcpDial::reportsFailureOnUnresolvableHost()
{
  QTcpSocket socket;
  IO::AsyncTcpDial dial;
  dial.setDeadline(4000);
  QSignalSpy spy(&dial, &IO::AsyncTcpDial::finished);

  dial.start(QStringLiteral("serial-studio-invalid.invalid"), 9, &socket);
  QVERIFY(spy.wait(8000));

  QCOMPARE(spy.count(), 1);
  QVERIFY(!spy.at(0).at(0).toBool());
  QVERIFY(socket.state() != QAbstractSocket::ConnectedState);
}

//--------------------------------------------------------------------------------------------------
// Cancellation
//--------------------------------------------------------------------------------------------------

/**
 * @brief A cancel is not an open failure: it ends the attempt with no verdict, which is what lets
 *        ConnectionManager drop a pending dial the user gave up on without reporting one.
 */
void TstAsyncTcpDial::cancelReportsNoVerdict()
{
  QTcpServer server;
  const quint16 port = listenOnLoopback(server);
  QVERIFY(port != 0);
  server.close();

  QTcpSocket socket;
  IO::AsyncTcpDial dial;
  dial.setDeadline(3000);
  QSignalSpy spy(&dial, &IO::AsyncTcpDial::finished);

  dial.start(QStringLiteral("127.0.0.1"), port, &socket);
  QVERIFY(dial.active());

  dial.cancel();
  QVERIFY(!dial.active());

  QTest::qWait(1500);
  QCOMPARE(spy.count(), 0);
}

/**
 * @brief A deadline change lands only while idle, so a live attempt keeps the bound it started
 *        with and cannot be stretched under a caller that already published it.
 */
void TstAsyncTcpDial::deadlineIsIgnoredWhileActive()
{
  QTcpServer server;
  const quint16 port = listenOnLoopback(server);
  QVERIFY(port != 0);
  server.close();

  QTcpSocket socket;
  IO::AsyncTcpDial dial;
  dial.setDeadline(1000);
  QCOMPARE(dial.deadline(), 1000);

  dial.start(QStringLiteral("127.0.0.1"), port, &socket);
  dial.setDeadline(60000);
  QCOMPARE(dial.deadline(), 1000);

  dial.cancel();
  dial.setDeadline(2500);
  QCOMPARE(dial.deadline(), 2500);
}

QTEST_GUILESS_MAIN(TstAsyncTcpDial)

#include "tst_async_tcp_dial.moc"
