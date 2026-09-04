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

#include <QPointer>
#include <QtGlobal>
#include <typeindex>

namespace Core::Bus {
class MessageBus;

/**
 * @brief Identity of one subscriber within its topic. Unique for the lifetime of the bus, never
 *        reused, so a handle outliving its subscription cannot cancel a later one.
 */
using SubscriberId = quint64;

/**
 * @brief Move-only RAII handle to one subscription: destroying it unsubscribes, so a caller who
 *        drops the handle stops receiving messages instead of leaking a handler onto a dead
 *        object. The bus is held through a QPointer, so a handle that outlives its bus destructs
 *        into a no-op, and release() detaches a subscription meant to outlive its handle.
 */
class Subscription final {
public:
  Subscription();
  Subscription(Subscription&& other) noexcept;
  Subscription(const Subscription&)            = delete;
  Subscription& operator=(const Subscription&) = delete;
  Subscription& operator=(Subscription&& other) noexcept;
  ~Subscription();

  void reset();
  void release() noexcept;

  [[nodiscard]] bool isActive() const;
  [[nodiscard]] SubscriberId id() const noexcept;

private:
  friend class MessageBus;
  Subscription(MessageBus* bus, std::type_index topic, SubscriberId id);

  SubscriberId m_id;
  std::type_index m_topic;
  QPointer<MessageBus> m_bus;
};
}  // namespace Core::Bus
