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

#include <atomic>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>

#include "API/Server/ServerWorker.h"

/**
 * @brief Every outbound lane of the API worker against a client that never reads (spec 0075 I6).
 *        The pair is real loopback but no event loop runs, so nothing drains the socket: what the
 *        worker queues stays queued and bytesToWrite() is an exact measurement of what each lane
 *        decided to send.
 */
class TstServerWorkerCaps : public QObject {
  Q_OBJECT

private slots:
  void init();
  void cleanup();

  void capBoundaryIsInclusive();
  void responsesReachAClientUnderTheCap();
  void broadcastsAreSkippedOverTheCap();
  void mirrorPushesAreSkippedOverTheCap();
  void responsesDropTheClientOverTheCap();

private:
  void fillBacklogPastTheCap();

  QTcpServer m_listener;
  QTcpSocket* m_serverSide                                        = nullptr;
  QTcpSocket* m_client                                            = nullptr;
  moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* m_queue = nullptr;
  API::ServerWorker* m_worker                                     = nullptr;
  std::atomic<bool> m_enabled{true};
  std::atomic<size_t> m_queueSize{0};
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Brings up one loopback connection and hands the server side to a worker.
 */
void TstServerWorkerCaps::init()
{
  QVERIFY(m_listener.listen(QHostAddress::LocalHost, 0));

  m_client = new QTcpSocket(this);
  m_client->connectToHost(QHostAddress::LocalHost, m_listener.serverPort());
  QVERIFY(m_client->waitForConnected(5000));
  QVERIFY(m_listener.waitForNewConnection(5000));

  m_serverSide = m_listener.nextPendingConnection();
  QVERIFY(m_serverSide != nullptr);

  m_queue  = new moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>(16);
  m_worker = new API::ServerWorker(m_queue, &m_enabled, &m_queueSize);
  m_worker->addSocket(m_serverSide, QStringLiteral("s1"));
}

/**
 * @brief Tears the pair down without ever running an event loop.
 */
void TstServerWorkerCaps::cleanup()
{
  m_worker->closeResources();
  delete m_worker;
  delete m_queue;
  m_worker = nullptr;
  m_queue  = nullptr;

  m_client->abort();
  delete m_client;
  m_client     = nullptr;
  m_serverSide = nullptr;
  m_listener.close();
}

/**
 * @brief Queues more than the cap on the socket itself, which is what a client that stops reading
 *        does to the server over time.
 */
void TstServerWorkerCaps::fillBacklogPastTheCap()
{
  const qint64 cap = API::ServerWorker::maxPendingWriteBytes();
  m_serverSide->write(QByteArray(static_cast<qsizetype>(cap) + 1, 'x'));
  QVERIFY(API::ServerWorker::exceedsWriteCap(m_serverSide->bytesToWrite()));
}

//--------------------------------------------------------------------------------------------------
// The cap itself
//--------------------------------------------------------------------------------------------------

/**
 * @brief A backlog exactly at the cap still passes; one byte more does not. Restating the value
 *        here is the point: widening it silently has to break this suite.
 */
void TstServerWorkerCaps::capBoundaryIsInclusive()
{
  const qint64 cap = API::ServerWorker::maxPendingWriteBytes();
  QCOMPARE(cap, qint64(16 * 1024 * 1024));
  QVERIFY(!API::ServerWorker::exceedsWriteCap(0));
  QVERIFY(!API::ServerWorker::exceedsWriteCap(cap));
  QVERIFY(API::ServerWorker::exceedsWriteCap(cap + 1));
}

//--------------------------------------------------------------------------------------------------
// Lanes under the cap
//--------------------------------------------------------------------------------------------------

/**
 * @brief The ordinary case: a reading client gets its response and nothing is counted.
 */
void TstServerWorkerCaps::responsesReachAClientUnderTheCap()
{
  const QByteArray payload = QByteArrayLiteral("{\"type\":\"response\",\"success\":true}\n");
  const qint64 before      = m_serverSide->bytesToWrite();

  m_worker->writeToSocket(m_serverSide, QStringLiteral("s1"), payload);

  QCOMPARE(m_serverSide->bytesToWrite() - before, qint64(payload.size()));
  QCOMPARE(m_worker->backlogDisconnects(), quint64(0));
  QCOMPARE(m_worker->droppedBroadcasts(), quint64(0));
}

//--------------------------------------------------------------------------------------------------
// Lanes over the cap
//--------------------------------------------------------------------------------------------------

/**
 * @brief A lifecycle broadcast is producer-paced, so an over-cap client is skipped and counted --
 *        it rejoins the broadcast as soon as it drains (spec 0075 I6).
 */
void TstServerWorkerCaps::broadcastsAreSkippedOverTheCap()
{
  fillBacklogPastTheCap();
  const qint64 before = m_serverSide->bytesToWrite();

  QJsonObject event;
  event.insert(QStringLiteral("event"), QStringLiteral("connected"));
  m_worker->broadcastEvent(event);

  QCOMPARE(m_serverSide->bytesToWrite(), before);
  QCOMPARE(m_worker->droppedBroadcasts(), quint64(1));
  QCOMPARE(m_worker->backlogDisconnects(), quint64(0));
}

/**
 * @brief The mirror lane was already capped and stays capped, so the fan-out has no gap.
 */
void TstServerWorkerCaps::mirrorPushesAreSkippedOverTheCap()
{
  fillBacklogPastTheCap();
  const qint64 before = m_serverSide->bytesToWrite();

  m_worker->writeMirrorPayload(
    m_serverSide, QStringLiteral("s1"), QByteArrayLiteral("{\"mirror\":{}}\n"));

  QCOMPARE(m_serverSide->bytesToWrite(), before);
  QCOMPARE(m_worker->droppedBroadcasts(), quint64(1));
}

/**
 * @brief A response is not producer-paced: the client asked for it, so skipping would leave it
 *        waiting forever and queueing would grow without bound. The client is dropped instead,
 *        and the payload is never appended.
 */
void TstServerWorkerCaps::responsesDropTheClientOverTheCap()
{
  fillBacklogPastTheCap();

  const QByteArray payload = QByteArrayLiteral("{\"type\":\"response\",\"success\":true}\n");
  m_worker->writeToSocket(m_serverSide, QStringLiteral("s1"), payload);

  QCOMPARE(m_worker->backlogDisconnects(), quint64(1));
  QCOMPARE(m_worker->droppedBroadcasts(), quint64(0));
  QVERIFY(m_serverSide->state() != QAbstractSocket::ConnectedState);
}

QTEST_GUILESS_MAIN(TstServerWorkerCaps)

#include "tst_server_worker_caps.moc"
