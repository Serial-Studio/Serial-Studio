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

#include "Core/Bus/Subscription.h"

#include "Core/Bus/MessageBus.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a detached handle: it refers to no subscription and destructs into a no-op.
 */
Core::Bus::Subscription::Subscription() : m_id(0), m_topic(typeid(void)), m_bus(nullptr)
{
  SS_ASSERT_LOG(!isActive());
  SS_ASSERT_LOG(m_id == 0);
}

/**
 * @brief Constructs the handle the bus hands back from subscribe(); only the bus may call this.
 */
Core::Bus::Subscription::Subscription(MessageBus* bus, std::type_index topic, SubscriberId id)
  : m_id(id), m_topic(topic), m_bus(bus)
{
  SS_ASSERT_LOG(bus != nullptr);
  SS_ASSERT_LOG(id != 0);
}

/**
 * @brief Takes over @p other's subscription and leaves it detached.
 */
Core::Bus::Subscription::Subscription(Subscription&& other) noexcept
  : m_id(other.m_id), m_topic(other.m_topic), m_bus(other.m_bus)
{
  other.release();
}

/**
 * @brief Cancels this handle's subscription, then takes over @p other's.
 */
Core::Bus::Subscription& Core::Bus::Subscription::operator=(Subscription&& other) noexcept
{
  if (this == &other)
    return *this;

  reset();

  m_id    = other.m_id;
  m_topic = other.m_topic;
  m_bus   = other.m_bus;
  other.release();
  return *this;
}

/**
 * @brief Cancels the subscription the handle owns, if it still owns one.
 */
Core::Bus::Subscription::~Subscription()
{
  reset();
}

//--------------------------------------------------------------------------------------------------
// Cancellation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Unsubscribes now and detaches. Safe from inside a handler: the bus holds no lock there.
 */
void Core::Bus::Subscription::reset()
{
  if (!isActive())
    return;

  m_bus->unsubscribe(m_topic, m_id);
  release();
}

/**
 * @brief Detaches without unsubscribing, for a subscription meant to live as long as its receiver.
 */
void Core::Bus::Subscription::release() noexcept
{
  m_id    = 0;
  m_topic = std::type_index(typeid(void));
  m_bus.clear();
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the handle still refers to a subscription on a live bus.
 */
bool Core::Bus::Subscription::isActive() const
{
  return !m_bus.isNull() && m_id != 0;
}

/**
 * @brief Returns the subscriber identity, or zero when the handle is detached.
 */
Core::Bus::SubscriberId Core::Bus::Subscription::id() const noexcept
{
  return m_id;
}
