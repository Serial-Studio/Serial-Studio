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

#include "IO/Drivers/OpcUa/OpcUaSubscriptions.h"

#include <chrono>
#include <utility>

#include "Core/SSAssert.h"

static constexpr int kWatchdogMs             = 1000;
static constexpr int kSilenceFactor          = 6;
static constexpr int kMinSilenceMs           = 3000;
static constexpr qint64 kSubscriptionNsPerMs = 1000000LL;

//--------------------------------------------------------------------------------------------------
// Construction and session binding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the engine and its three timers; nothing starts until a session subscribes.
 */
IO::Drivers::OpcUaSubscriptions::OpcUaSubscriptions(OpcUaSubscriptionHost& host, QObject* parent)
  : QObject(parent)
  , m_host(host)
  , m_pollMode(false)
  , m_subscribing(false)
  , m_readInFlight(false)
  , m_pendingMonitors(0)
  , m_failedMonitors(0)
  , m_revisedInterval(0)
  , m_skippedPolls(0)
  , m_framesPublished(0)
  , m_watchdog(new QTimer(this))
  , m_pollTimer(new QTimer(this))
  , m_frameTimer(new QTimer(this))
  , m_lastNotifyNs(0)
  , m_assembler(this)
{
  connect(m_pollTimer, &QTimer::timeout, this, &IO::Drivers::OpcUaSubscriptions::onPollTick);
  connect(m_frameTimer, &QTimer::timeout, this, &IO::Drivers::OpcUaSubscriptions::onFrameTick);
  connect(m_watchdog, &QTimer::timeout, this, &IO::Drivers::OpcUaSubscriptions::onWatchdogTick);
  connect(&m_assembler,
          &OpcUaFrameAssembler::typeMismatch,
          this,
          &IO::Drivers::OpcUaSubscriptions::onTypeMismatch);
}

/**
 * @brief Listens to the value-bearing signals of the session the driver just dialed. The dial
 *        verdict signals are NOT among them: the facade owns the open report, and an engine that
 *        could hear a connect failure could settle an attempt twice.
 */
void IO::Drivers::OpcUaSubscriptions::bindSession(OpcUaSession* session)
{
  SS_ASSERT(session != nullptr, return);

  connect(session, &OpcUaSession::subscribed, this, &IO::Drivers::OpcUaSubscriptions::onSubscribed);
  connect(
    session, &OpcUaSession::valueChanged, this, &IO::Drivers::OpcUaSubscriptions::onValueChanged);
  connect(session,
          &OpcUaSession::subscriptionLost,
          this,
          &IO::Drivers::OpcUaSubscriptions::onSubscriptionLost);
  connect(
    session, &OpcUaSession::readFinished, this, &IO::Drivers::OpcUaSubscriptions::onReadFinished);
}

/**
 * @brief Drops every connection to a session the driver is retiring, so a callback in flight while
 *        the session closes can never reach this object after the facade nulled its pointer.
 */
void IO::Drivers::OpcUaSubscriptions::unbindSession(OpcUaSession* session)
{
  if (!session)
    return;

  disconnect(session, nullptr, this, nullptr);
}

/**
 * @brief Stops every timer and forgets the session's wire layout. The cumulative counters survive:
 *        they are pulled diagnostics for the whole driver, not for one session.
 */
void IO::Drivers::OpcUaSubscriptions::reset()
{
  m_pollTimer->stop();
  m_frameTimer->stop();

  m_nodeIndex.clear();
  m_assembler.reset();
  m_watchdog->stop();
  m_polledTags.clear();
  m_pendingMonitors = 0;
  m_failedMonitors  = 0;
  m_revisedInterval = 0;
  m_readInFlight    = false;
  m_subscribing     = false;
  m_pollMode        = false;
  m_lastNotifyNs    = 0;
}

/**
 * @brief True while the driver's session is established; every tick is gated on it.
 */
bool IO::Drivers::OpcUaSubscriptions::sessionOpen() const
{
  const auto* session = m_host.liveSession();
  return session && session->isOpen();
}

//--------------------------------------------------------------------------------------------------
// Subscription, poll fallback and the publish tick
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the wire layout, reserves the frame once and asks the session for ONE
 *        subscription carrying every tag; the per-item verdicts arrive together in onSubscribed().
 */
void IO::Drivers::OpcUaSubscriptions::subscribeAll()
{
  auto* session = m_host.liveSession();
  SS_ASSERT(session != nullptr, return);

  const auto& tags = m_host.tags();
  SS_ASSERT(!tags.isEmpty(), return);

  const int interval = m_host.publishingInterval();
  const auto steady  = std::chrono::steady_clock::now().time_since_epoch();
  m_lastNotifyNs     = std::chrono::duration_cast<std::chrono::nanoseconds>(steady).count();
  m_assembler.beginSession();
  m_assembler.reserve(tags);

  m_pendingMonitors = static_cast<int>(tags.size());
  m_failedMonitors  = 0;
  m_revisedInterval = interval;
  m_pollMode        = false;
  m_subscribing     = true;
  m_polledTags.clear();

  QStringList nodeIds;
  nodeIds.reserve(tags.size());
  for (int i = 0; i < tags.size(); ++i) {
    nodeIds.append(tags.at(i).nodeId);
    m_nodeIndex.insert(tags.at(i).nodeId, i);
  }

  if (!session->subscribe(nodeIds, interval))
    onSubscribed(QList<OpcUaTypes::StatusCode>(tags.size(), OpcUaTypes::kStatusBadInternal));

  m_frameTimer->start(interval);
  m_watchdog->start(kWatchdogMs);
}

/**
 * @brief The batch verdict, one status per tag. A refusal stays INDIVIDUAL: the refused tags move
 *        to timed reads and the rest keep their subscription, and only an all-refused verdict
 *        flips the whole session into poll mode.
 */
void IO::Drivers::OpcUaSubscriptions::onSubscribed(
  const QList<OpcUaTypes::StatusCode>& perItemStatus)
{
  const auto& tags  = m_host.tags();
  m_subscribing     = false;
  m_pendingMonitors = 0;
  m_failedMonitors  = 0;
  m_polledTags.clear();

  for (int i = 0; i < perItemStatus.size() && i < tags.size(); ++i) {
    if (OpcUaTypes::isGood(perItemStatus.at(i)))
      continue;

    ++m_failedMonitors;
    m_polledTags.append(i);
    m_host.reportDriverError(
      tr("OPC UA Monitored Item Refused"),
      tr("\"%1\": %2").arg(tags.at(i).nodeId, OpcUaSession::describeStatus(perItemStatus.at(i))));
  }

  adoptRevisedInterval();

  if (m_failedMonitors >= tags.size()) {
    enterPollMode(tr("the server refused every monitored item"));
    return;
  }

  if (!m_polledTags.isEmpty()) {
    m_pollTimer->start(m_revisedInterval);
    onPollTick();
  }

  Q_EMIT statusChanged();
}

/**
 * @brief The server dropped the subscription on its own; timed reads take over.
 */
void IO::Drivers::OpcUaSubscriptions::onSubscriptionLost(const QString& reason)
{
  if (!sessionOpen())
    return;

  enterPollMode(reason.isEmpty() ? tr("the server retired the subscription") : reason);
}

/**
 * @brief Adopts the interval the server revised the subscription to; a PLC that floors publishing
 *        at 100 ms must not leave the pane claiming the rate the user asked for.
 */
void IO::Drivers::OpcUaSubscriptions::adoptRevisedInterval()
{
  const auto* session = m_host.liveSession();
  SS_ASSERT(session != nullptr, return);

  const int revised = session->revisedInterval();
  if (revised <= 0 || revised == m_revisedInterval)
    return;

  m_revisedInterval = qBound(kOpcUaMinIntervalMs, revised, kOpcUaMaxIntervalMs);
  if (m_frameTimer->isActive())
    m_frameTimer->start(m_revisedInterval);

  if (m_pollTimer->isActive())
    m_pollTimer->start(m_revisedInterval);
}

/**
 * @brief Switches to timed reads at the publishing interval.
 */
void IO::Drivers::OpcUaSubscriptions::enterPollMode(const QString& reason)
{
  if (m_pollMode)
    return;

  const auto& tags = m_host.tags();
  m_pollMode       = true;
  m_polledTags.clear();
  for (int i = 0; i < tags.size(); ++i)
    m_polledTags.append(i);

  m_host.reportDriverError(tr("OPC UA Subscription Unavailable"),
                           tr("Falling back to polling: %1.").arg(reason));

  m_pollTimer->start(m_revisedInterval > 0 ? m_revisedInterval : m_host.publishingInterval());
  onPollTick();
  Q_EMIT statusChanged();
}

/**
 * @brief Nothing has arrived for several publishing periods while the session is still up: a
 *        server that silently dropped the subscription (project reload, PLC stop) looks healthy
 *        otherwise, so the session falls back to polling rather than freezing the dashboard.
 */
void IO::Drivers::OpcUaSubscriptions::onWatchdogTick()
{
  if (!sessionOpen() || m_pollMode || m_subscribing || m_lastNotifyNs == 0)
    return;

  const auto now        = std::chrono::steady_clock::now().time_since_epoch();
  const qint64 nowNs    = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
  const qint64 silentMs = (nowNs - m_lastNotifyNs) / kSubscriptionNsPerMs;
  const qint64 budgetMs =
    qMax<qint64>(kMinSilenceMs, static_cast<qint64>(m_revisedInterval) * kSilenceFactor);

  if (silentMs < budgetMs)
    return;

  enterPollMode(tr("no notification for %1 s").arg(silentMs / 1000));
}

/**
 * @brief Issues one batched read of every tag's Value attribute.
 */
void IO::Drivers::OpcUaSubscriptions::onPollTick()
{
  if (!sessionOpen() || m_polledTags.isEmpty())
    return;

  if (m_readInFlight) {
    ++m_skippedPolls;
    return;
  }

  issueRead(m_polledTags);
}

/**
 * @brief Issues the batched Value read for a tag subset. Exactly one read is outstanding at a
 *        time: queueing them behind a slow PLC grows latency without bound and eventually times
 *        the session out, so the session drops the overflow rather than queueing it.
 */
void IO::Drivers::OpcUaSubscriptions::issueRead(const QList<int>& tags)
{
  auto* session = m_host.liveSession();
  SS_ASSERT(session != nullptr, return);
  SS_ASSERT(!tags.isEmpty(), return);

  const auto& known = m_host.tags();

  QStringList nodeIds;
  nodeIds.reserve(tags.size());
  for (const int tag : tags) {
    if (tag < 0 || tag >= known.size())
      continue;

    nodeIds.append(known.at(tag).nodeId);
  }

  if (!nodeIds.isEmpty())
    m_readInFlight = session->readValues(nodeIds);
}

/**
 * @brief Routes batched read results into the slot cache by node id.
 */
void IO::Drivers::OpcUaSubscriptions::onReadFinished(quint32 token,
                                                     const QList<OpcUaTypes::ReadRow>& rows,
                                                     OpcUaTypes::StatusCode status)
{
  Q_UNUSED(token)

  m_readInFlight = false;
  if (!OpcUaTypes::isGood(status)) {
    m_host.reportDriverError(tr("OPC UA Read Failed"), OpcUaSession::describeStatus(status));
    return;
  }

  for (const auto& row : rows) {
    const int tag = m_nodeIndex.value(row.nodeId, -1);
    if (tag < 0)
      continue;

    m_assembler.storeValue(tag, row.value, row.status, row.sourceTimestamp);
  }
}

/**
 * @brief Monitored-item update: the tag index and the server's own timestamps travel with the
 *        notification, so nothing has to be looked up or re-stamped here.
 */
void IO::Drivers::OpcUaSubscriptions::onValueChanged(const OpcUaTypes::MonitoredValue& value)
{
  SS_ASSERT(value.tag >= 0, return);
  if (value.tag >= m_host.tags().size())
    return;

  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  m_lastNotifyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

  m_assembler.noteServerTimestamp(value.serverTimestamp);
  m_assembler.storeValue(value.tag, value.value, value.status, value.sourceTimestamp);
}

/**
 * @brief Publishing tick: hands the assembled delta frame to the pipeline, stamped with the
 *        earliest source time it carries. No dirty slot, no frame.
 */
void IO::Drivers::OpcUaSubscriptions::onFrameTick()
{
  if (!sessionOpen())
    return;

  QByteArray frame;
  CapturedData::SteadyTimePoint timestamp;
  if (!m_assembler.assemble(frame, timestamp))
    return;

  ++m_framesPublished;
  m_host.publishFrame(std::move(frame), timestamp);
}

/**
 * @brief Re-arms the subscription and both timers at a newly configured interval; a live session
 *        adopts it in place rather than being torn down and rebuilt.
 */
void IO::Drivers::OpcUaSubscriptions::applyPublishingInterval(const int interval)
{
  auto* session = m_host.liveSession();
  if (session)
    (void)session->modifyPublishingInterval(interval);

  m_revisedInterval = interval;
  if (m_frameTimer->isActive())
    m_frameTimer->start(interval);

  if (m_pollTimer->isActive())
    m_pollTimer->start(interval);
}

//--------------------------------------------------------------------------------------------------
// Pulled diagnostics
//--------------------------------------------------------------------------------------------------

/**
 * @brief The node ids whose newest value carried a Bad status (R11 diagnostics).
 */
QStringList IO::Drivers::OpcUaSubscriptions::badTags() const
{
  return m_assembler.badTags(m_host.tags());
}

/**
 * @brief A slot whose values do not match the type the tag was declared with, reported once per
 *        slot. The warning is raised here rather than in the encoder because a driver's console
 *        line is the session's to write.
 */
void IO::Drivers::OpcUaSubscriptions::onTypeMismatch(int index,
                                                     const QString& declared,
                                                     const QString& actual)
{
  m_host.reportDriverError(
    tr("OPC UA Type Mismatch"),
    tr("Channel %1 is declared %2 but the server sends %3; the value is coerced.")
      .arg(QString::number(index), declared, actual));
}
