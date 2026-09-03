/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <QDateTime>
#include <QSignalSpy>
#include <QTest>

#include "IO/Drivers/OpcUa/OpcUaSubscriptions.h"

using IO::Drivers::OpcUaSession;
using IO::Drivers::OpcUaSubscriptionHost;
using IO::Drivers::OpcUaSubscriptions;
using IO::Drivers::OpcUaTag;

namespace Types = IO::Drivers::OpcUaTypes;
namespace Wire  = IO::Drivers::OpcUaWire;

/**
 * @brief The four things the engine asks its driver for, and the two outlets it cannot reach
 *        itself. Standing in for the facade is what lets the state machine be driven without a
 *        dial: the engine owns no session and settles no open verdict, so a stub is enough.
 */
class StubHost : public OpcUaSubscriptionHost {
public:
  /**
   * @brief Builds a host offering @p session and no tags.
   */
  explicit StubHost(OpcUaSession* session) : m_session(session), m_interval(100), m_frames(0) {}

  /**
   * @brief Counts a published frame and keeps its bytes for the caller to walk.
   */
  void publishFrame(QByteArray&& frame, IO::CapturedData::SteadyTimePoint timestamp) override
  {
    Q_UNUSED(timestamp)
    m_lastFrame = std::move(frame);
    ++m_frames;
  }

  /**
   * @brief Records the driver-facing error text the engine would have logged.
   */
  void reportDriverError(const QString& title, const QString& detail) const override
  {
    Q_UNUSED(detail)
    m_errors.append(title);
  }

  [[nodiscard]] int publishingInterval() const override { return m_interval; }

  [[nodiscard]] OpcUaSession* liveSession() const override { return m_session; }

  [[nodiscard]] const QList<OpcUaTag>& tags() const noexcept override { return m_tags; }

  /**
   * @brief Replaces the tag list the engine subscribes.
   */
  void setTags(const QList<OpcUaTag>& tags) { m_tags = tags; }

  [[nodiscard]] const QStringList& errors() const { return m_errors; }

  [[nodiscard]] int frames() const { return m_frames; }

  [[nodiscard]] const QByteArray& lastFrame() const noexcept { return m_lastFrame; }

private:
  OpcUaSession* m_session;
  QList<OpcUaTag> m_tags;
  QByteArray m_lastFrame;
  mutable QStringList m_errors;
  int m_interval;
  int m_frames;
};

/**
 * @brief One subscribed tag under @p nodeId.
 */
[[nodiscard]] static OpcUaTag tagOf(const char* nodeId)
{
  OpcUaTag tag;
  tag.nodeId = QString::fromLatin1(nodeId);
  tag.name   = tag.nodeId;
  tag.type   = Wire::Type::F64;
  return tag;
}

/**
 * @brief The OPC UA subscription state machine: what a per-item refusal does to the tags around
 *        it, when an all-refused verdict flips the whole session to timed reads, and what a reset
 *        forgets. Every tick is gated on a live session, so a stub host with an unopened session
 *        drives exactly the paths that must survive a server that answers nothing.
 */
class TstOpcUaSubscriptions : public QObject {
  Q_OBJECT

private slots:
  void aRefusedItemDoesNotDragTheOthersIntoPolling();
  void everyItemRefusedFallsBackToPolling();
  void notificationsReachTheValueCache();
  void resetForgetsTheLayoutAndKeepsTheCounters();
  void aRevisedIntervalIsAdopted();
};

//--------------------------------------------------------------------------------------------------
// Per-item verdicts
//--------------------------------------------------------------------------------------------------

/**
 * @brief A refusal is INDIVIDUAL: the refused tag moves to timed reads and the rest keep their
 *        monitored items. Flipping the whole session to polling on one bad node id would drop
 *        every other tag to the poll interval for the life of the session.
 */
void TstOpcUaSubscriptions::aRefusedItemDoesNotDragTheOthersIntoPolling()
{
  OpcUaSession session;
  StubHost host(&session);
  host.setTags({tagOf("ns=2;s=A"), tagOf("ns=2;s=B")});

  OpcUaSubscriptions engine(host);
  engine.bindSession(&session);

  QSignalSpy spy(&engine, &OpcUaSubscriptions::statusChanged);
  QVERIFY(spy.isValid());

  Q_EMIT session.subscribed({Types::kStatusGood, Types::kStatusBadInternal});

  QVERIFY(!engine.pollMode());
  QVERIFY(!engine.subscribing());
  QCOMPARE(engine.refusedTags(), 1);
  QCOMPARE(engine.pendingMonitors(), 0);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(host.errors().size(), 1);
}

/**
 * @brief An all-refused verdict is a server that does not do subscriptions, so the session falls
 *        back to timed reads instead of showing a dashboard that never updates.
 */
void TstOpcUaSubscriptions::everyItemRefusedFallsBackToPolling()
{
  OpcUaSession session;
  StubHost host(&session);
  host.setTags({tagOf("ns=2;s=A"), tagOf("ns=2;s=B")});

  OpcUaSubscriptions engine(host);
  engine.bindSession(&session);

  Q_EMIT session.subscribed({Types::kStatusBadInternal, Types::kStatusBadTimeout});

  QVERIFY(engine.pollMode());
  QCOMPARE(engine.refusedTags(), 2);
  QVERIFY(host.errors().size() >= 3);
  QCOMPARE(host.frames(), 0);
}

//--------------------------------------------------------------------------------------------------
// Value cache
//--------------------------------------------------------------------------------------------------

/**
 * @brief A monitored-item update carries its own tag index and the server's timestamps, so it
 *        reaches the cache without a lookup; the counters it feeds are the pulled diagnostics.
 */
void TstOpcUaSubscriptions::notificationsReachTheValueCache()
{
  OpcUaSession session;
  StubHost host(&session);
  host.setTags({tagOf("ns=2;s=A")});

  OpcUaSubscriptions engine(host);
  engine.bindSession(&session);
  engine.subscribeAll();
  QVERIFY(!engine.idle());

  Types::MonitoredValue update;
  update.tag             = 0;
  update.value           = QVariant(12.5);
  update.status          = Types::kStatusGood;
  update.sourceTimestamp = QDateTime::currentDateTimeUtc();
  update.serverTimestamp = update.sourceTimestamp;

  Q_EMIT session.valueChanged(update);
  QCOMPARE(engine.valuesReceived(), quint64(1));
  QCOMPARE(engine.badStatusCount(), quint64(0));
  QVERIFY(engine.badTags().isEmpty());

  update.status = Types::kStatusBadTimeout;
  Q_EMIT session.valueChanged(update);
  QCOMPARE(engine.valuesReceived(), quint64(2));
  QCOMPARE(engine.badStatusCount(), quint64(1));
  QCOMPARE(engine.badTags(), QStringList{QStringLiteral("ns=2;s=A")});
}

//--------------------------------------------------------------------------------------------------
// Session lifetime
//--------------------------------------------------------------------------------------------------

/**
 * @brief A reset forgets the session's wire layout and its mode, and KEEPS the lifetime counters:
 *        they are pulled diagnostics for the whole driver, read as deltas, not per session.
 */
void TstOpcUaSubscriptions::resetForgetsTheLayoutAndKeepsTheCounters()
{
  OpcUaSession session;
  StubHost host(&session);
  host.setTags({tagOf("ns=2;s=A")});

  OpcUaSubscriptions engine(host);
  engine.bindSession(&session);
  engine.subscribeAll();

  Types::MonitoredValue update;
  update.tag             = 0;
  update.value           = QVariant(1.0);
  update.status          = Types::kStatusGood;
  update.sourceTimestamp = QDateTime::currentDateTimeUtc();
  Q_EMIT session.valueChanged(update);

  Q_EMIT session.subscribed({Types::kStatusBadInternal});
  QVERIFY(engine.pollMode());

  engine.reset();
  QVERIFY(engine.idle());
  QVERIFY(!engine.pollMode());
  QVERIFY(!engine.subscribing());
  QCOMPARE(engine.refusedTags(), 0);
  QCOMPARE(engine.revisedInterval(), 0);
  QCOMPARE(engine.valuesReceived(), quint64(1));

  engine.unbindSession(&session);
  Q_EMIT session.valueChanged(update);
  QCOMPARE(engine.valuesReceived(), quint64(1));
}

/**
 * @brief A live interval change is adopted in place rather than tearing the subscription down;
 *        the pane reads the adopted value, so a stale one claims a rate nothing is running at.
 */
void TstOpcUaSubscriptions::aRevisedIntervalIsAdopted()
{
  OpcUaSession session;
  StubHost host(&session);
  host.setTags({tagOf("ns=2;s=A")});

  OpcUaSubscriptions engine(host);
  engine.bindSession(&session);
  engine.subscribeAll();
  QCOMPARE(engine.revisedInterval(), host.publishingInterval());

  engine.applyPublishingInterval(250);
  QCOMPARE(engine.revisedInterval(), 250);
}

QTEST_GUILESS_MAIN(TstOpcUaSubscriptions)

#include "tst_opcua_subscriptions.moc"
