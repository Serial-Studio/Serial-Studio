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

#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Core/Bus/Subscription.h"
#include "Core/SSAssert.h"

/**
 * @file MessageBus.h
 * @brief The in-process publish/subscribe bus the libraries under core/ talk over.
 *
 * The bus is how one library announces state to another without reaching for the other's
 * singleton. Topics are C++ types -- no string identifiers anywhere in the API -- and the
 * vocabulary of cross-library topics lives in Messages.h. A publisher constructs one immutable
 * message and every subscriber receives a pointer to that same object, so the coupling between
 * two libraries is a struct rather than a class.
 *
 * Delivery respects receiver affinity: direct on the receiver's own thread, queued across
 * threads, and BlockingQueuedConnection is rejected because a publisher must never wait on a
 * subscriber. The subscriber vector is copied under the mutex and no lock is held while a handler
 * runs, so a handler may publish, subscribe or unsubscribe re-entrantly.
 *
 * State topics (publishState/latest) retain their last message, which is the readable-by-pointer
 * shared state a pull-style accessor on a foreign singleton would otherwise provide.
 *
 * NEVER on the per-frame path. Every publish allocates a message and copies the subscriber
 * vector; frames and blocks keep the pooled SPSC path of spec 0055, and the bus carries command,
 * state and notification rate only.
 *
 * The composition root constructs the bus and hands it to setInstance(); the bus never constructs
 * itself. The singleton census is expected to grow here as the follow-up specs of 0076 replace
 * module reaches with topics, which is the metric of that migration rather than new debt.
 */

namespace Core::Bus {
/**
 * @brief A published message as the subscriber table stores it. The aliasing control block keeps
 *        the object alive for every delivery, and the typed pointer is recovered by static cast.
 */
using ErasedMessage = std::shared_ptr<const void>;

/**
 * @brief A subscriber's handler after type erasure.
 */
using ErasedHandler = std::function<void(const ErasedMessage&)>;

/**
 * @brief Satisfied by a publish() argument pack that is a constructor argument list rather than an
 *        already-built message pointer, which is what keeps the two publish() overloads apart.
 */
template<typename T, typename... Args>
concept MessageArgs =
  !(sizeof...(Args) == 1
    && (std::same_as<std::shared_ptr<const T>, std::remove_cvref_t<Args>> || ...));

/**
 * @brief In-process publish/subscribe bus whose topics are C++ types, described in this file's
 *        @file block. Command, state and notification rate only: never the per-frame path.
 *        A receiver is destroyed on its own thread, after its subscriptions are gone, or it
 *        outlives every publisher on other threads: delivery reads the receiver's thread.
 */
class MessageBus : public QObject {
  Q_OBJECT

public:
  explicit MessageBus(QObject* parent = nullptr);
  MessageBus(MessageBus&&)                 = delete;
  MessageBus(const MessageBus&)            = delete;
  MessageBus& operator=(MessageBus&&)      = delete;
  MessageBus& operator=(const MessageBus&) = delete;
  ~MessageBus() override;

  [[nodiscard]] static MessageBus* instance();
  static void setInstance(MessageBus* bus);

  /**
   * @brief Builds one immutable message of topic @c T and hands it to every live subscriber.
   */
  template<typename T, typename... Args>
    requires MessageArgs<T, Args...>
  void publish(Args&&... args)
  {
    publish<T>(compose<T>(std::forward<Args>(args)...));
  }

  /**
   * @brief Publishes a message the caller already owns, without copying the object.
   */
  template<typename T>
  void publish(std::shared_ptr<const T> message)
  {
    dispatch(std::type_index(typeid(T)), ErasedMessage(std::move(message)));
  }

  /**
   * @brief Publishes and retains, so a later latest<T>() or replaying subscriber sees this object.
   */
  template<typename T, typename... Args>
    requires MessageArgs<T, Args...>
  void publishState(Args&&... args)
  {
    publishState<T>(compose<T>(std::forward<Args>(args)...));
  }

  /**
   * @brief Retaining publish of a message the caller already owns.
   */
  template<typename T>
  void publishState(std::shared_ptr<const T> message)
  {
    const ErasedMessage erased(std::move(message));
    retain(std::type_index(typeid(T)), erased);
    dispatch(std::type_index(typeid(T)), erased);
  }

  /**
   * @brief Returns the last message retained for topic @c T, or null if none ever was.
   */
  template<typename T>
  [[nodiscard]] std::shared_ptr<const T> latest() const
  {
    return std::static_pointer_cast<const T>(retained(std::type_index(typeid(T))));
  }

  /**
   * @brief Subscribes @p receiver to topic @c T and returns the handle that keeps the subscription
   *        alive: it dies with the handle, with @p receiver or with the bus, whichever comes
   *        first. @p replayLatest delivers the retained message, if any, before returning.
   */
  template<typename T>
  [[nodiscard]] Subscription subscribe(QObject* receiver,
                                       std::function<void(const std::shared_ptr<const T>&)> handler,
                                       Qt::ConnectionType type = Qt::AutoConnection,
                                       bool replayLatest       = false)
  {
    SS_ASSERT(receiver != nullptr, return Subscription());
    SS_ASSERT(handler != nullptr, return Subscription());

    const std::type_index topic(typeid(T));
    ErasedHandler erased = [handler](const ErasedMessage& message) {
      handler(std::static_pointer_cast<const T>(message));
    };

    const SubscriberId id = addSubscriber(topic, receiver, std::move(erased), type);
    if (replayLatest) {
      const std::shared_ptr<const T> message = latest<T>();
      if (message)
        handler(message);
    }

    return Subscription(this, topic, id);
  }

private:
  friend class Subscription;

  /**
   * @brief One entry of the subscriber table.
   */
  struct Subscriber {
    SubscriberId id;
    QPointer<QObject> receiver;
    std::shared_ptr<const ErasedHandler> handler;
    Qt::ConnectionType type;
  };

  /**
   * @brief Builds the message through braced initialization, so an aggregate topic needs no
   *        constructor and no parenthesized-aggregate support from the compiler.
   */
  template<typename T, typename... Args>
  [[nodiscard]] static std::shared_ptr<const T> compose(Args&&... args)
  {
    T message{std::forward<Args>(args)...};
    return std::make_shared<const T>(std::move(message));
  }

  void retain(std::type_index topic, ErasedMessage message);
  void purgeReceiver(QObject* receiver);
  void unsubscribe(std::type_index topic, SubscriberId id);
  void dispatch(std::type_index topic, const ErasedMessage& message);
  void deliver(const Subscriber& subscriber, const ErasedMessage& message);

  [[nodiscard]] ErasedMessage retained(std::type_index topic) const;
  [[nodiscard]] SubscriberId addSubscriber(std::type_index topic,
                                           QObject* receiver,
                                           ErasedHandler handler,
                                           Qt::ConnectionType type);

  SubscriberId m_nextId;
  mutable std::mutex m_mutex;
  std::unordered_map<std::type_index, ErasedMessage> m_retained;
  std::unordered_map<QObject*, QMetaObject::Connection> m_guards;
  std::unordered_map<std::type_index, std::vector<Subscriber>> m_subscribers;
};
}  // namespace Core::Bus
