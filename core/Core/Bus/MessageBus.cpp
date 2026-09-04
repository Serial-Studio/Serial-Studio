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

#include "Core/Bus/MessageBus.h"

#include <QThread>

//--------------------------------------------------------------------------------------------------
// Process-wide handle
//--------------------------------------------------------------------------------------------------

/**
 * @brief The bus the composition root published, or null before it did. A plain pointer and not a
 *        Meyers singleton on purpose: the root owns the object and controls its lifetime.
 */
static Core::Bus::MessageBus* s_instance = nullptr;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty bus. Ownership stays with the caller (the composition root).
 */
Core::Bus::MessageBus::MessageBus(QObject* parent) : QObject(parent), m_nextId(0)
{
  SS_ASSERT_LOG(m_subscribers.empty());
  SS_ASSERT_LOG(m_retained.empty());
}

/**
 * @brief Drops every subscriber guard and clears the retained state. Outstanding Subscription
 *        handles see a null QPointer afterwards and destruct into no-ops.
 */
Core::Bus::MessageBus::~MessageBus()
{
  if (s_instance == this)
    s_instance = nullptr;

  const std::lock_guard<std::mutex> lock(m_mutex);
  for (const auto& guard : m_guards)
    QObject::disconnect(guard.second);

  m_guards.clear();
  m_subscribers.clear();
  m_retained.clear();
}

/**
 * @brief Returns the bus the composition root published, or null when none was.
 */
Core::Bus::MessageBus* Core::Bus::MessageBus::instance()
{
  return s_instance;
}

/**
 * @brief Publishes the root-owned bus, or withdraws it with a null argument at shutdown.
 */
void Core::Bus::MessageBus::setInstance(MessageBus* bus)
{
  SS_ASSERT_LOG(bus == nullptr || s_instance == nullptr);
  s_instance = bus;
}

//--------------------------------------------------------------------------------------------------
// Subscription table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers one erased handler against @p topic and returns its identity.
 *        BlockingQueuedConnection is rejected because a publisher that blocks on a subscriber's
 *        thread turns every announcement into a rendezvous, and deadlocks the moment two threads
 *        publish to each other; the release recovery downgrades it to a queued delivery.
 */
Core::Bus::SubscriberId Core::Bus::MessageBus::addSubscriber(std::type_index topic,
                                                             QObject* receiver,
                                                             ErasedHandler handler,
                                                             Qt::ConnectionType type)
{
  SS_ASSERT(receiver != nullptr, return 0);
  SS_ASSERT(type != Qt::BlockingQueuedConnection, type = Qt::QueuedConnection);

  const std::lock_guard<std::mutex> lock(m_mutex);
  const SubscriberId id = ++m_nextId;

  Subscriber subscriber;
  subscriber.id       = id;
  subscriber.type     = type;
  subscriber.receiver = receiver;
  subscriber.handler  = std::make_shared<const ErasedHandler>(std::move(handler));
  m_subscribers[topic].push_back(std::move(subscriber));

  if (!m_guards.contains(receiver))
    m_guards[receiver] = QObject::connect(
      receiver,
      &QObject::destroyed,
      this,
      [this](QObject* dying) { purgeReceiver(dying); },
      Qt::DirectConnection);

  return id;
}

/**
 * @brief Drops one subscriber from @p topic. Unknown ids are ignored, which is what makes a
 *        handle that outlived its receiver safe to destroy.
 */
void Core::Bus::MessageBus::unsubscribe(std::type_index topic, SubscriberId id)
{
  SS_ASSERT(id != 0, return);

  const std::lock_guard<std::mutex> lock(m_mutex);
  const auto it = m_subscribers.find(topic);
  if (it == m_subscribers.end())
    return;

  std::erase_if(it->second, [id](const Subscriber& entry) { return entry.id == id; });
  if (it->second.empty())
    m_subscribers.erase(it);
}

/**
 * @brief Drops every entry whose receiver died. QObject clears its QPointers before it emits
 *        destroyed(), so the dying receiver is usually already a null guard rather than a matching
 *        address, and both spellings have to be swept for the purge to be complete.
 */
void Core::Bus::MessageBus::purgeReceiver(QObject* receiver)
{
  SS_ASSERT(receiver != nullptr, return);

  const std::lock_guard<std::mutex> lock(m_mutex);
  for (auto& entry : m_subscribers)
    std::erase_if(entry.second, [receiver](const Subscriber& s) {
      return s.receiver.isNull() || s.receiver.data() == receiver;
    });

  const auto guard = m_guards.find(receiver);
  SS_ASSERT(guard != m_guards.end(), return);

  QObject::disconnect(guard->second);
  m_guards.erase(guard);
}

//--------------------------------------------------------------------------------------------------
// Publication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Hands @p message to every live subscriber of @p topic. The subscriber vector is copied
 *        under the mutex and the handlers run with no lock held, so a handler may publish,
 *        subscribe or unsubscribe re-entrantly; the copy also means a subscriber added by a
 *        handler joins the next publication rather than this one.
 */
void Core::Bus::MessageBus::dispatch(std::type_index topic, const ErasedMessage& message)
{
  SS_ASSERT(message != nullptr, return);

  std::vector<Subscriber> targets;
  {
    const std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_subscribers.find(topic);
    if (it == m_subscribers.end())
      return;

    targets = it->second;
  }

  for (const Subscriber& subscriber : targets)
    deliver(subscriber, message);
}

/**
 * @brief Delivers one message to one subscriber, direct on the receiver's own thread and queued
 *        across threads. The queued lambda captures the message pointer, so the object outlives
 *        every delivery no matter how late a receiver's event loop drains.
 */
void Core::Bus::MessageBus::deliver(const Subscriber& subscriber, const ErasedMessage& message)
{
  QObject* receiver = subscriber.receiver.data();
  if (!receiver)
    return;

  const bool sameThread = receiver->thread() == QThread::currentThread();
  const std::shared_ptr<const ErasedHandler> handler = subscriber.handler;
  SS_ASSERT(handler != nullptr, return);
  if (subscriber.type == Qt::DirectConnection
      || (subscriber.type == Qt::AutoConnection && sameThread)) {
    (*handler)(message);
    return;
  }

  const bool queued = QMetaObject::invokeMethod(
    receiver, [handler, message] { (*handler)(message); }, Qt::QueuedConnection);
  SS_ASSERT_LOG(queued);
}

//--------------------------------------------------------------------------------------------------
// Retained state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores @p message as the current value of @p topic, replacing any earlier one.
 */
void Core::Bus::MessageBus::retain(std::type_index topic, ErasedMessage message)
{
  SS_ASSERT(message != nullptr, return);

  const std::lock_guard<std::mutex> lock(m_mutex);
  m_retained.insert_or_assign(topic, std::move(message));
  SS_ASSERT_LOG(!m_retained.empty());
}

/**
 * @brief Returns the message retained for @p topic, or null when the topic never published state.
 */
Core::Bus::ErasedMessage Core::Bus::MessageBus::retained(std::type_index topic) const
{
  const std::lock_guard<std::mutex> lock(m_mutex);
  const auto it = m_retained.find(topic);
  if (it == m_retained.end())
    return ErasedMessage();

  return it->second;
}
