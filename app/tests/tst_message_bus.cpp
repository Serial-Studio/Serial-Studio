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
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTest>
#include <QThread>
#include <vector>

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

/**
 * @brief A multi-field topic, so a test can read a payload field off the pointer it received.
 */
struct Loaded final {
  QString path;
  QString title;
};

/**
 * @brief A one-field topic for the tests that only care about delivery, not payload. The bus
 *        mechanics are exercised on topics this file owns, so a change to the Messages.h
 *        vocabulary can only break the one test that deliberately names every topic.
 */
struct Flag final {
  bool value;
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

  void everyVocabularyTopicComposesByBracedInit();
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

  std::shared_ptr<const Loaded> first;
  std::shared_ptr<const Loaded> second;
  auto one = bus.subscribe<Loaded>(
    &receiver, [&first](const std::shared_ptr<const Loaded>& message) { first = message; });
  auto two = bus.subscribe<Loaded>(
    &receiver, [&second](const std::shared_ptr<const Loaded>& message) { second = message; });

  bus.publish<Loaded>(QStringLiteral("/tmp/x.ssproj"), QStringLiteral("X"));

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
  auto subscription = bus.subscribe<Flag>(
    &receiver, [&ran](const std::shared_ptr<const Flag>&) { ran.store(QThread::currentThread()); });

  bus.publish<Flag>(true);
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

  bus.publish<Flag>(true);
  QVERIFY(bus.latest<Flag>() == nullptr);
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
    auto subscription = bus.subscribe<Flag>(
      &receiver, [&deliveries](const std::shared_ptr<const Flag>&) { ++deliveries; });

    bus.publish<Flag>(true);
    QCOMPARE(deliveries, 1);
  }

  bus.publish<Flag>(true);
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
  auto outer = bus.subscribe<Flag>(
    &receiver, [&bus](const std::shared_ptr<const Flag>&) { bus.publish<Echo>(5); });

  bus.publish<Flag>(true);

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

  auto subscription = bus.subscribe<Flag>(
    &receiver,
    [&deliveries](const std::shared_ptr<const Flag>&) { ++deliveries; },
    Qt::BlockingQueuedConnection);

  bus.publish<Flag>(true);
  QCOMPARE(deliveries, 0);
  QTRY_COMPARE(deliveries, 1);
  QVERIFY(subscription.isActive());
}

/**
 * @brief Every topic in Messages.h stays a plain aggregate that publish<T>(args...) can brace-init
 *        in field order; a topic that grows a constructor or reorders a field fails here first.
 */
void MessageBusTests::everyVocabularyTopicComposesByBracedInit()
{
  Core::Bus::MessageBus bus;
  bus.publishState<Core::Bus::ConnectionStateChanged>(0, true, false, false, 0);
  bus.publish<Core::Bus::NotificationRaised>(
    Core::Bus::kSeverityWarning, QString(), QString(), QString(), QString());
  bus.publish<Core::Bus::NotificationPosted>(qint64(0), 0, QString(), QString(), QString());
  bus.publishState<Core::Bus::DashboardStructureChanged>(1);
  bus.publish<Core::Bus::DashboardUpdated>(0);
  bus.publish<Core::Bus::DashboardDataReset>(0);
  bus.publishState<Core::Bus::MirrorAttachedChanged>(false);
  bus.publishState<Core::Bus::LicenseStateChanged>(false, 0, false);
  bus.publishState<Core::Bus::OperationModeChanged>(0);
  bus.publishState<Core::Bus::FrameConfigChanged>(IO::FrameConfig());
  bus.publishState<Core::Bus::ReplayPlayerStateChanged>(0, false);
  bus.publishState<Core::Bus::AudioCaptureFormat>(0, 48000, true);
  bus.publishState<Core::Bus::WidgetExtensionCatalog>(QVector<Core::Bus::WidgetExtensionEntry>{
    {QString(), 0, QString(), QString()}
  });
  bus.publishState<Core::Bus::DashboardViewState>(QString());
  bus.publish<Core::Bus::DashboardViewStateRestoreRequested>(QString());
  bus.publish<Core::Bus::DashboardViewStateClearRequested>(0);
  bus.publish<Core::Bus::DisconnectRequested>(0);
  bus.publish<Core::Bus::DeviceOpenAttempted>(0, true, QString());
  bus.publish<Core::Bus::ModbusRegisterGroupsLoaded>(QJsonDocument());
  bus.publish<Core::Bus::LoadGeneratedProjectRequested>(QJsonDocument(), true, quint64(1));
  bus.publish<Core::Bus::GeneratedProjectLoadFinished>(quint64(1), true, true);
  bus.publish<Core::Bus::Source0ConnectionSettingsChanged>(0, QJsonObject(), true, false);
  bus.publishState<Core::Bus::ProjectStructureSnapshot>(
    std::vector<DataModel::Source>(),
    std::vector<DataModel::Group>(),
    QString(),
    false,
    0,
    Core::Bus::ProjectStructureSnapshot::Content,
    -1,
    quint64(1));
  bus.publish<Core::Bus::ConnectionAboutToOpen>(0);
  bus.publishState<Core::Bus::ActiveUiDriverSettings>(0, QJsonObject());
  bus.publish<Core::Bus::SourceSettingsCaptureRequested>(0, 0);
  bus.publish<Core::Bus::SourceConnectionSettingsCaptured>(0, QJsonObject());
  bus.publish<Core::Bus::SourceSettingsRestoreRequested>(0);
  QVERIFY(Core::Bus::allocateRequestId() < Core::Bus::allocateRequestId());
  QVERIFY(bus.latest<Core::Bus::OperationModeChanged>() != nullptr);
  QCOMPARE(bus.latest<Core::Bus::AudioCaptureFormat>()->sampleRate, 48000);
}

QTEST_GUILESS_MAIN(MessageBusTests)

#include "tst_message_bus.moc"
