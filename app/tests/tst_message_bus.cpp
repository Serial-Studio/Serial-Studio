/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
#include <memory>
#include <QObject>
#include <QString>
#include <QTest>
#include <QThread>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"

// Every test function here builds its own bus and its own receivers: no state is carried between
// slots, so Qt Test's declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Test topics
//--------------------------------------------------------------------------------------------------

/**
 * @brief A topic no test ever publishes, so latest() has an unknown type to answer for.
 */
struct NeverPublished final {
  int value;
};

/**
 * @brief A second topic, so a handler can publish something other than what it received.
 */
struct Echo final {
  int value;
};

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

class MessageBusTests : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void directDeliveryHandsOverThePublishedPointer();
  void everySubscriberReceivesTheSamePointer();
  void queuedDeliveryRunsOnTheReceiverThread();

  void latestIsNullForATopicNeverPublished();
  void publishStateRetainsTheLatestMessage();
  void replayLatestDeliversTheRetainedMessage();

  void destroyingTheSubscriptionStopsDelivery();
  void destroyingTheReceiverStopsDelivery();

  void aHandlerMayPublishAnotherTopic();
  void blockingQueuedIsDowngradedToQueued();
};

/**
 * @brief Makes SS_ASSERT take its recovery branch instead of aborting, which is the only way a
 *        test can observe what the release build does with a rejected connection type.
 */
void MessageBusTests::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

//--------------------------------------------------------------------------------------------------
// Delivery
//--------------------------------------------------------------------------------------------------

void MessageBusTests::directDeliveryHandsOverThePublishedPointer()
{
  Core::Bus::MessageBus bus;
  QObject receiver;

  int deliveries                                = 0;
  const Core::Bus::ConnectionStateChanged* seen = nullptr;
  auto subscription                             = bus.subscribe<Core::Bus::ConnectionStateChanged>(
    &receiver, [&](const std::shared_ptr<const Core::Bus::ConnectionStateChanged>& message) {
      seen = message.get();
      ++deliveries;
    });

  const auto published = std::make_shared<const Core::Bus::ConnectionStateChanged>(
    Core::Bus::ConnectionStateChanged{3, true, false});
  bus.publish<Core::Bus::ConnectionStateChanged>(published);

  QCOMPARE(deliveries, 1);
  QCOMPARE(seen, published.get());
  QCOMPARE(seen->sourceId, 3);
  QVERIFY(subscription.isActive());
}

void MessageBusTests::everySubscriberReceivesTheSamePointer()
{
  Core::Bus::MessageBus bus;
  QObject receiver;

  std::shared_ptr<const Core::Bus::ProjectLoaded> first;
  std::shared_ptr<const Core::Bus::ProjectLoaded> second;
  auto one = bus.subscribe<Core::Bus::ProjectLoaded>(
    &receiver,
    [&first](const std::shared_ptr<const Core::Bus::ProjectLoaded>& message) { first = message; });
  auto two = bus.subscribe<Core::Bus::ProjectLoaded>(
    &receiver, [&second](const std::shared_ptr<const Core::Bus::ProjectLoaded>& message) {
      second = message;
    });

  bus.publish<Core::Bus::ProjectLoaded>(QStringLiteral("/tmp/x.ssproj"), QStringLiteral("X"));

  QVERIFY(first != nullptr);
  QCOMPARE(first.get(), second.get());
  QCOMPARE(first.use_count(), 2L);
  QCOMPARE(first->title, QStringLiteral("X"));
  QVERIFY(one.id() != two.id());
}

void MessageBusTests::queuedDeliveryRunsOnTheReceiverThread()
{
  Core::Bus::MessageBus bus;
  QThread worker;
  QObject receiver;

  receiver.moveToThread(&worker);
  worker.start();
  QVERIFY(worker.isRunning());

  std::atomic<QThread*> ran{nullptr};
  auto subscription = bus.subscribe<Core::Bus::ProjectModified>(
    &receiver, [&ran](const std::shared_ptr<const Core::Bus::ProjectModified>&) {
      ran.store(QThread::currentThread());
    });

  bus.publish<Core::Bus::ProjectModified>(true);
  QTRY_COMPARE(ran.load(), &worker);
  QVERIFY(ran.load() != QThread::currentThread());

  subscription.reset();
  worker.quit();
  QVERIFY(worker.wait());
}

//--------------------------------------------------------------------------------------------------
// Retained state
//--------------------------------------------------------------------------------------------------

void MessageBusTests::latestIsNullForATopicNeverPublished()
{
  Core::Bus::MessageBus bus;
  QVERIFY(bus.latest<NeverPublished>() == nullptr);
}

void MessageBusTests::publishStateRetainsTheLatestMessage()
{
  Core::Bus::MessageBus bus;
  QVERIFY(bus.latest<Core::Bus::LicenseStateChanged>() == nullptr);

  bus.publishState<Core::Bus::LicenseStateChanged>(true);
  const auto activated = bus.latest<Core::Bus::LicenseStateChanged>();
  QVERIFY(activated != nullptr);
  QCOMPARE(activated->activated, true);

  bus.publishState<Core::Bus::LicenseStateChanged>(false);
  QCOMPARE(bus.latest<Core::Bus::LicenseStateChanged>()->activated, false);

  bus.publish<Core::Bus::ProjectModified>(true);
  QVERIFY(bus.latest<Core::Bus::ProjectModified>() == nullptr);
}

void MessageBusTests::replayLatestDeliversTheRetainedMessage()
{
  Core::Bus::MessageBus bus;
  QObject receiver;

  bus.publishState<Core::Bus::DashboardStructureChanged>(7);

  int seen          = -1;
  auto subscription = bus.subscribe<Core::Bus::DashboardStructureChanged>(
    &receiver,
    [&seen](const std::shared_ptr<const Core::Bus::DashboardStructureChanged>& message) {
      seen = message->generation;
    },
    Qt::AutoConnection,
    true);

  QCOMPARE(seen, 7);
  QVERIFY(subscription.isActive());
}

//--------------------------------------------------------------------------------------------------
// Lifetime
//--------------------------------------------------------------------------------------------------

void MessageBusTests::destroyingTheSubscriptionStopsDelivery()
{
  Core::Bus::MessageBus bus;
  QObject receiver;
  int deliveries = 0;

  {
    auto subscription = bus.subscribe<Core::Bus::SettingsChanged>(
      &receiver,
      [&deliveries](const std::shared_ptr<const Core::Bus::SettingsChanged>&) { ++deliveries; });

    bus.publish<Core::Bus::SettingsChanged>(QStringLiteral("io.baud"));
    QCOMPARE(deliveries, 1);
  }

  bus.publish<Core::Bus::SettingsChanged>(QStringLiteral("io.baud"));
  QCOMPARE(deliveries, 1);
}

void MessageBusTests::destroyingTheReceiverStopsDelivery()
{
  Core::Bus::MessageBus bus;
  int deliveries = 0;

  auto* receiver    = new QObject();
  auto subscription = bus.subscribe<Core::Bus::NotificationRaised>(
    receiver,
    [&deliveries](const std::shared_ptr<const Core::Bus::NotificationRaised>&) { ++deliveries; });

  bus.publish<Core::Bus::NotificationRaised>(1, QStringLiteral("t"), QStringLiteral("b"));
  QCOMPARE(deliveries, 1);

  delete receiver;
  bus.publish<Core::Bus::NotificationRaised>(1, QStringLiteral("t"), QStringLiteral("b"));
  QCOMPARE(deliveries, 1);
  QVERIFY(subscription.isActive());
}

//--------------------------------------------------------------------------------------------------
// Re-entrancy and connection types
//--------------------------------------------------------------------------------------------------

void MessageBusTests::aHandlerMayPublishAnotherTopic()
{
  Core::Bus::MessageBus bus;
  QObject receiver;
  int echoes = 0;

  auto inner = bus.subscribe<Echo>(
    &receiver, [&echoes](const std::shared_ptr<const Echo>& message) { echoes += message->value; });
  auto outer = bus.subscribe<Core::Bus::ProjectModified>(
    &receiver,
    [&bus](const std::shared_ptr<const Core::Bus::ProjectModified>&) { bus.publish<Echo>(5); });

  bus.publish<Core::Bus::ProjectModified>(true);

  QCOMPARE(echoes, 5);
  QVERIFY(inner.isActive());
  QVERIFY(outer.isActive());
}

/**
 * @brief BlockingQueuedConnection would make every publisher wait on a subscriber's thread, so the
 *        bus rejects it: the debug build aborts, and the release recovery this test observes
 *        downgrades the subscriber to a plain queued delivery.
 */
void MessageBusTests::blockingQueuedIsDowngradedToQueued()
{
  Core::Bus::MessageBus bus;
  QObject receiver;
  int deliveries = 0;

  auto subscription = bus.subscribe<Core::Bus::RecordingSessionBoundary>(
    &receiver,
    [&deliveries](const std::shared_ptr<const Core::Bus::RecordingSessionBoundary>&) {
      ++deliveries;
    },
    Qt::BlockingQueuedConnection);

  bus.publish<Core::Bus::RecordingSessionBoundary>(true, false);
  QCOMPARE(deliveries, 0);
  QTRY_COMPARE(deliveries, 1);
  QVERIFY(subscription.isActive());
}

QTEST_GUILESS_MAIN(MessageBusTests)

#include "tst_message_bus.moc"
