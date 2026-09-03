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
#include <chrono>
#include <memory>
#include <QTest>
#include <QThread>
#include <vector>

#include "API/GRPC/PendingCall.h"

/**
 * @brief The abortable marshal a gRPC handler uses to reach the GUI thread (spec 0075 I5). The
 *        suite plays the two threads directly: a "handler" that waits, and a "GUI" that either
 *        dispatches or never gets the chance because the server is stopping.
 */
class TstGrpcPendingCall : public QObject {
  Q_OBJECT

private slots:
  void dispatchRunsTheFunctorAndWakesTheWaiter();
  void abandonedCallsWakeAndNeverRun();
  void abandonAfterDispatchKeepsTheResult();
  void timeoutLeavesTheCallUnrun();
  void stoppingWhileACallIsParkedReleasesIt();
};

//--------------------------------------------------------------------------------------------------
// The happy path
//--------------------------------------------------------------------------------------------------

/**
 * @brief The ordinary command: the GUI thread runs the functor, the handler wakes up and reads
 *        what the functor wrote into its own frame.
 */
void TstGrpcPendingCall::dispatchRunsTheFunctorAndWakesTheWaiter()
{
  int answer = 0;
  auto call  = std::make_shared<API::GRPC::PendingCall>([&answer] { answer = 42; });

  const std::unique_ptr<QThread> gui(QThread::create([call] {
    QThread::msleep(20);
    call->dispatch();
  }));

  gui->start();
  const bool ran = call->wait(std::chrono::milliseconds(5000));
  call->abandon();
  QVERIFY(gui->wait(5000));

  QVERIFY(ran);
  QCOMPARE(answer, 42);
}

//--------------------------------------------------------------------------------------------------
// Abandonment
//--------------------------------------------------------------------------------------------------

/**
 * @brief An abandoned call wakes its waiter immediately and its functor never runs afterwards --
 *        which is what makes a by-reference capture safe once the waiter has left.
 */
void TstGrpcPendingCall::abandonedCallsWakeAndNeverRun()
{
  std::atomic<int> runs{0};
  auto call = std::make_shared<API::GRPC::PendingCall>([&runs] { runs.fetch_add(1); });

  const std::unique_ptr<QThread> stopper(QThread::create([call] {
    QThread::msleep(20);
    call->abandon();
  }));

  stopper->start();
  const bool ran = call->wait(std::chrono::milliseconds(5000));
  QVERIFY(stopper->wait(5000));

  QVERIFY(!ran);
  QCOMPARE(runs.load(), 0);

  call->dispatch();
  QCOMPARE(runs.load(), 0);
}

/**
 * @brief Abandoning a call that already ran does not undo it: the waiter still reports success,
 *        so a command answered milliseconds before a stop is not turned into an error.
 */
void TstGrpcPendingCall::abandonAfterDispatchKeepsTheResult()
{
  int answer = 0;
  auto call  = std::make_shared<API::GRPC::PendingCall>([&answer] { answer = 7; });

  call->dispatch();
  QVERIFY(call->wait(std::chrono::milliseconds(0)));

  call->abandon();
  QCOMPARE(answer, 7);
}

//--------------------------------------------------------------------------------------------------
// Deadline
//--------------------------------------------------------------------------------------------------

/**
 * @brief A GUI thread that never answers releases the handler at its deadline instead of parking
 *        a gRPC thread forever.
 */
void TstGrpcPendingCall::timeoutLeavesTheCallUnrun()
{
  std::atomic<int> runs{0};
  auto call = std::make_shared<API::GRPC::PendingCall>([&runs] { runs.fetch_add(1); });

  QVERIFY(!call->wait(std::chrono::milliseconds(20)));
  call->abandon();
  QCOMPARE(runs.load(), 0);
}

//--------------------------------------------------------------------------------------------------
// Shutdown
//--------------------------------------------------------------------------------------------------

/**
 * @brief The I5 shape: several handlers parked on the GUI thread, the user toggles the API off.
 *        Every parked call is released before the server would wait for those handlers.
 */
void TstGrpcPendingCall::stoppingWhileACallIsParkedReleasesIt()
{
  std::vector<std::shared_ptr<API::GRPC::PendingCall>> parked;
  std::vector<std::unique_ptr<QThread>> handlers;
  std::atomic<int> released{0};

  for (int i = 0; i < 4; ++i) {
    auto call = std::make_shared<API::GRPC::PendingCall>([] {});
    parked.push_back(call);
    handlers.emplace_back(QThread::create([call, &released] {
      if (!call->wait(std::chrono::milliseconds(5000)))
        released.fetch_add(1);
    }));
    handlers.back()->start();
  }

  QThread::msleep(20);
  for (auto& call : parked)
    call->abandon();

  for (auto& handler : handlers)
    QVERIFY(handler->wait(5000));

  QCOMPARE(released.load(), 4);
}

QTEST_GUILESS_MAIN(TstGrpcPendingCall)

#include "tst_grpc_pending_call.moc"
