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

#include <limits>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include "Platform/KioskControl.h"

// Every test function here is self-contained: each one listens on its own server name, so Qt
// Test's declaration-order execution is never load-bearing.

/**
 * @brief The launcher's end of the channel for one test: a local server that keeps the newest
 *        client and collects the lines it sent.
 */
class Launcher : public QObject {
  Q_OBJECT

public:
  explicit Launcher(const QString& name) : m_name(name), m_connections(0)
  {
    QLocalServer::removeServer(name);
    connect(&m_server, &QLocalServer::newConnection, this, &Launcher::adopt);
  }

  [[nodiscard]] bool listen() { return m_server.listen(m_name); }

  [[nodiscard]] int connections() const { return m_connections; }

  [[nodiscard]] QList<QJsonObject> lines() const { return m_lines; }

  void send(const QByteArray& bytes)
  {
    if (m_peer && m_peer->write(bytes) == bytes.size())
      m_peer->flush();
  }

  void shutDown()
  {
    if (m_peer)
      m_peer->abort();

    m_server.close();
  }

private slots:

  void adopt()
  {
    m_peer = m_server.nextPendingConnection();
    ++m_connections;
    connect(m_peer, &QLocalSocket::readyRead, this, &Launcher::collect);
  }

  void collect()
  {
    m_pending.append(m_peer->readAll());
    QByteArray line;
    for (int i = 0; i < 64; ++i) {
      if (Platform::KioskControl::takeLine(m_pending, line) != Platform::KioskControl::Frame::Line)
        return;

      m_lines.append(QJsonDocument::fromJson(line).object());
    }
  }

private:
  QString m_name;
  int m_connections;
  QByteArray m_pending;
  QLocalServer m_server;
  QList<QJsonObject> m_lines;
  QPointer<QLocalSocket> m_peer;
};

/**
 * @brief Protocol coverage of the kiosk control client (spec 0125) against an in-process server:
 *        line framing and its 64 KiB cap, the greeting on every connection, the state event, the
 *        three commands, and the silent redial after the server goes away.
 */
class TstKioskControl : public QObject {
  Q_OBJECT

private slots:
  void takeLine_data();
  void takeLine();

  void greetsOnConnect();
  void greetsUnderItsOwnIdentity();
  void sendsState();
  void decodesCommands();
  void reserveLeftValues_data();
  void reserveLeftValues();
  void handlesABurstOfLines();
  void ignoresWhatItDoesNotKnow();
  void dropsAnOverlongLine();
  void redialsAndGreetsAgain();
  void staysQuietWithoutAServer();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief A server name no other test and no other run of this binary uses.
 */
[[nodiscard]] static QString serverName(const char* test)
{
  return QStringLiteral("ss-tst-kiosk-%1-%2")
    .arg(QCoreApplication::applicationPid())
    .arg(QLatin1String(test));
}

/**
 * @brief Greets on every connection, the way the session does.
 */
static void greetOnLink(Platform::KioskControl& control)
{
  QObject::connect(&control, &Platform::KioskControl::linkEstablished, &control, [&control] {
    control.sendHello(QStringLiteral("Bench"), QStringLiteral("Screen A"));
  });
}

//--------------------------------------------------------------------------------------------------
// Framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Buffers around the line cap: nothing complete, one line, two lines, and lines one byte
 *        over the cap with and without their terminator.
 */
void TstKioskControl::takeLine_data()
{
  QTest::addColumn<QByteArray>("buffer");
  QTest::addColumn<int>("frame");
  QTest::addColumn<QByteArray>("line");
  QTest::addColumn<QByteArray>("rest");

  const int cap  = Platform::KioskControl::kMaxLineBytes;
  const int none = static_cast<int>(Platform::KioskControl::Frame::None);
  const int line = static_cast<int>(Platform::KioskControl::Frame::Line);
  const int over = static_cast<int>(Platform::KioskControl::Frame::Overflow);

  QTest::newRow("empty") << QByteArray() << none << QByteArray() << QByteArray();
  QTest::newRow("partial") << QByteArray("{\"cmd\"") << none << QByteArray()
                           << QByteArray("{\"cmd\"");
  QTest::newRow("one line") << QByteArray("{}\n") << line << QByteArray("{}") << QByteArray();
  QTest::newRow("two lines") << QByteArray("a\nb\n") << line << QByteArray("a")
                             << QByteArray("b\n");
  QTest::newRow("empty line") << QByteArray("\nx") << line << QByteArray() << QByteArray("x");
  QTest::newRow("at the cap, open")
    << QByteArray(cap, 'x') << none << QByteArray() << QByteArray(cap, 'x');
  QTest::newRow("at the cap, closed") << QByteArray(cap, 'x') + '\n'
                                      << line << QByteArray(cap, 'x') << QByteArray();
  QTest::newRow("over the cap, open")
    << QByteArray(cap + 1, 'x') << over << QByteArray() << QByteArray(cap + 1, 'x');
  QTest::newRow("over the cap, closed") << QByteArray(cap + 1, 'x') + '\n'
                                        << over << QByteArray() << QByteArray(cap + 1, 'x') + '\n';
}

/**
 * @brief A line comes out without its terminator and leaves the rest; an over-long one is
 *        reported whether or not its terminator arrived.
 */
void TstKioskControl::takeLine()
{
  QFETCH(QByteArray, buffer);
  QFETCH(int, frame);
  QFETCH(QByteArray, line);
  QFETCH(QByteArray, rest);

  QByteArray taken;
  const auto result = Platform::KioskControl::takeLine(buffer, taken);
  QCOMPARE(static_cast<int>(result), frame);
  QCOMPARE(taken, line);
  QCOMPARE(buffer, rest);
}

//--------------------------------------------------------------------------------------------------
// Outgoing lines
//--------------------------------------------------------------------------------------------------

/**
 * @brief The first line after connecting is the greeting, with every field of the contract.
 */
void TstKioskControl::greetsOnConnect()
{
  Launcher launcher(serverName("hello"));
  QVERIFY(launcher.listen());

  Platform::KioskControl control;
  greetOnLink(control);
  control.start(serverName("hello"));

  QTRY_COMPARE(launcher.lines().size(), 1);
  const QJsonObject hello = launcher.lines().constFirst();
  QCOMPARE(hello.value("hello").toInt(), 1);
  QCOMPARE(hello.value("id").toString(), QStringLiteral("dashboard"));
  QCOMPARE(hello.value("title").toString(), QStringLiteral("Bench"));
  QCOMPARE(hello.value("screen").toString(), QStringLiteral("Screen A"));
  QCOMPARE(hello.value("pid").toInteger(), QCoreApplication::applicationPid());
  QCOMPARE(hello.value("can").toArray(), (QJsonArray{"raise", "screen", "close"}));
}

/**
 * @brief A client that stands for an external dashboard greets under the identity it was given,
 *        which is what lets a launcher list it beside the main window.
 */
void TstKioskControl::greetsUnderItsOwnIdentity()
{
  Launcher launcher(serverName("identity"));
  QVERIFY(launcher.listen());

  Platform::KioskControl control;
  control.setIdentity(QStringLiteral("dashboard-2"));
  greetOnLink(control);
  control.start(serverName("identity"));

  QTRY_COMPARE(launcher.lines().size(), 1);
  QCOMPARE(launcher.lines().constFirst().value("id").toString(), QStringLiteral("dashboard-2"));
}

/**
 * @brief The state event carries the screen and the active flag, and nothing is written before
 *        there is a link.
 */
void TstKioskControl::sendsState()
{
  Platform::KioskControl control;
  control.sendState(QStringLiteral("Nowhere"), true);
  QVERIFY(!control.isLinked());

  Launcher launcher(serverName("state"));
  QVERIFY(launcher.listen());
  control.start(serverName("state"));
  QTRY_VERIFY(control.isLinked());

  control.sendState(QStringLiteral("Screen B"), false);
  QTRY_COMPARE(launcher.lines().size(), 1);
  const QJsonObject state = launcher.lines().constFirst();
  QCOMPARE(state.value("event").toString(), QStringLiteral("state"));
  QCOMPARE(state.value("screen").toString(), QStringLiteral("Screen B"));
  QCOMPARE(state.value("active").toBool(true), false);
  QVERIFY(!state.contains("title"));

  control.sendState(QStringLiteral("Screen B"), true, QStringLiteral("Engine Test"));
  QTRY_COMPARE(launcher.lines().size(), 2);
  QCOMPARE(launcher.lines().constLast().value("title").toString(), QStringLiteral("Engine Test"));
}

//--------------------------------------------------------------------------------------------------
// Incoming lines
//--------------------------------------------------------------------------------------------------

/**
 * @brief The three commands reach their signals, a line split across two writes included, and a
 *        screen command without reserve_left reports -1.
 */
void TstKioskControl::decodesCommands()
{
  Launcher launcher(serverName("cmd"));
  QVERIFY(launcher.listen());

  Platform::KioskControl control;
  QSignalSpy raise(&control, &Platform::KioskControl::raiseRequested);
  QSignalSpy close(&control, &Platform::KioskControl::closeRequested);
  QSignalSpy screen(&control, &Platform::KioskControl::screenRequested);
  control.start(serverName("cmd"));
  QTRY_COMPARE(launcher.connections(), 1);

  launcher.send("{\"cmd\":\"raise\"}\n{\"cmd\":\"screen\",\"name\":\"LG\",\"reserve_left\":64}\n");
  launcher.send("{\"cmd\":\"screen\",\"na");
  QTRY_COMPARE(screen.size(), 1);
  launcher.send("me\":\"DELL\"}\n{\"cmd\":\"close\"}\n");

  QTRY_COMPARE(close.size(), 1);
  QCOMPARE(raise.size(), 1);
  QCOMPARE(screen.size(), 2);
  QCOMPARE(screen.at(0).at(0).toString(), QStringLiteral("LG"));
  QCOMPARE(screen.at(0).at(1).toInt(), 64);
  QCOMPARE(screen.at(1).at(0).toString(), QStringLiteral("DELL"));
  QCOMPARE(screen.at(1).at(1).toInt(), -1);
}

/**
 * @brief reserve_left as the launcher might send it: numbers are truncated and held at zero or
 *        above, anything that is not a number counts as absent.
 */
void TstKioskControl::reserveLeftValues_data()
{
  QTest::addColumn<QByteArray>("value");
  QTest::addColumn<int>("expected");

  QTest::newRow("whole") << QByteArray("64") << 64;
  QTest::newRow("zero") << QByteArray("0") << 0;
  QTest::newRow("negative") << QByteArray("-5") << 0;
  QTest::newRow("fraction") << QByteArray("2.7") << 2;
  QTest::newRow("huge") << QByteArray("1e300") << std::numeric_limits<int>::max();
  QTest::newRow("string") << QByteArray("\"64\"") << -1;
  QTest::newRow("null") << QByteArray("null") << -1;
  QTest::newRow("bool") << QByteArray("true") << -1;
}

/**
 * @brief Each value arrives at the screen signal as the documented width, or -1 for absent.
 */
void TstKioskControl::reserveLeftValues()
{
  QFETCH(QByteArray, value);
  QFETCH(int, expected);

  Launcher launcher(serverName(QTest::currentDataTag()));
  QVERIFY(launcher.listen());

  Platform::KioskControl control;
  QSignalSpy screen(&control, &Platform::KioskControl::screenRequested);
  control.start(serverName(QTest::currentDataTag()));
  QTRY_COMPARE(launcher.connections(), 1);

  launcher.send("{\"cmd\":\"screen\",\"name\":\"LG\",\"reserve_left\":" + value + "}\n");
  QTRY_COMPARE(screen.size(), 1);
  QCOMPARE(screen.at(0).at(1).toInt(), expected);
}

/**
 * @brief A burst longer than one read pass handles is delivered whole, in more than one pass.
 */
void TstKioskControl::handlesABurstOfLines()
{
  constexpr int kLines = 1000;

  Launcher launcher(serverName("burst"));
  QVERIFY(launcher.listen());

  Platform::KioskControl control;
  QSignalSpy raise(&control, &Platform::KioskControl::raiseRequested);
  control.start(serverName("burst"));
  QTRY_COMPARE(launcher.connections(), 1);

  QByteArray burst;
  for (int i = 0; i < kLines; ++i)
    burst.append("{\"cmd\":\"raise\"}\n");

  launcher.send(burst);
  QTRY_COMPARE(raise.size(), kLines);
  QVERIFY(control.isLinked());
}

/**
 * @brief Unknown commands, unknown keys, lines that are not JSON objects and a screen command
 *        without a name do nothing, and the link survives them.
 */
void TstKioskControl::ignoresWhatItDoesNotKnow()
{
  Launcher launcher(serverName("unknown"));
  QVERIFY(launcher.listen());

  Platform::KioskControl control;
  QSignalSpy raise(&control, &Platform::KioskControl::raiseRequested);
  QSignalSpy close(&control, &Platform::KioskControl::closeRequested);
  QSignalSpy screen(&control, &Platform::KioskControl::screenRequested);
  control.start(serverName("unknown"));
  QTRY_COMPARE(launcher.connections(), 1);

  launcher.send("{\"cmd\":\"minimize\"}\nnot json\n[1,2]\n\n{\"cmd\":\"screen\"}\n");
  launcher.send("{\"cmd\":\"raise\",\"later\":{\"x\":1}}\n");

  QTRY_COMPARE(raise.size(), 1);
  QCOMPARE(close.size(), 0);
  QCOMPARE(screen.size(), 0);
  QVERIFY(control.isLinked());
}

/**
 * @brief More than 64 KiB without a terminator drops the connection, and the client dials again.
 */
void TstKioskControl::dropsAnOverlongLine()
{
  Launcher launcher(serverName("overlong"));
  QVERIFY(launcher.listen());

  Platform::KioskControl control;
  QSignalSpy raise(&control, &Platform::KioskControl::raiseRequested);
  control.setRetryInterval(20);
  control.start(serverName("overlong"));
  QTRY_COMPARE(launcher.connections(), 1);

  launcher.send(QByteArray(Platform::KioskControl::kMaxLineBytes + 4096, 'x'));
  QTRY_COMPARE(launcher.connections(), 2);

  launcher.send("{\"cmd\":\"raise\"}\n");
  QTRY_COMPARE(raise.size(), 1);
}

//--------------------------------------------------------------------------------------------------
// Reconnection
//--------------------------------------------------------------------------------------------------

/**
 * @brief The client outlives its server: it keeps dialing while there is none, greets when one
 *        appears, and greets again after that one is replaced.
 */
void TstKioskControl::redialsAndGreetsAgain()
{
  Platform::KioskControl control;
  greetOnLink(control);
  control.setRetryInterval(20);
  control.start(serverName("redial"));
  QTest::qWait(80);
  QVERIFY(!control.isLinked());

  Launcher first(serverName("redial"));
  QVERIFY(first.listen());
  QTRY_COMPARE(first.lines().size(), 1);

  first.shutDown();
  QTRY_VERIFY(!control.isLinked());

  Launcher second(serverName("redial"));
  QVERIFY(second.listen());
  QTRY_COMPARE(second.lines().size(), 1);
  QCOMPARE(second.lines().constFirst().value("hello").toInt(), 1);
}

/**
 * @brief An empty server name opens nothing, which is the run without --kiosk-control.
 */
void TstKioskControl::staysQuietWithoutAServer()
{
  Platform::KioskControl control;
  QSignalSpy link(&control, &Platform::KioskControl::linkEstablished);
  control.setRetryInterval(10);
  control.start(QString());

  QTest::qWait(60);
  QCOMPARE(link.size(), 0);
  QVERIFY(!control.isLinked());
}

QTEST_GUILESS_MAIN(TstKioskControl)

#include "tst_kiosk_control.moc"
