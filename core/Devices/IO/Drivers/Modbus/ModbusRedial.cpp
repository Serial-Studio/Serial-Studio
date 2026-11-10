/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "IO/Drivers/Modbus/ModbusRedial.h"

#include <QDebug>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"

static constexpr int kRedialIntervalMs = 10000;

/**
 * @brief Builds the timer idle; the driver arms it from its failure paths only, and its timeout
 *        is the due() signal itself.
 */
IO::Drivers::ModbusRedial::ModbusRedial(QObject* parent) : QObject(parent)
{
  m_timer.setSingleShot(true);
  m_timer.setInterval(kRedialIntervalMs);
  connect(&m_timer, &QTimer::timeout, this, &IO::Drivers::ModbusRedial::due);
}

/**
 * @brief Schedules one re-dial; arming again restarts the wait instead of stacking a second.
 */
void IO::Drivers::ModbusRedial::arm()
{
  m_timer.start();
}

/**
 * @brief Drops the scheduled re-dial: what close() calls, so the user's disconnect ends it.
 */
void IO::Drivers::ModbusRedial::cancel()
{
  m_timer.stop();
}

/**
 * @brief An established link closed under the driver: say so once, raise dropped() on the next
 *        event-loop turn (nothing tears a driver down from inside its own handler; the manager's
 *        close() answers it synchronously), then arm the re-dial when @p redial says TCP.
 */
void IO::Drivers::ModbusRedial::reportDrop(Core::Bus::MessageBus* bus,
                                           const QString& target,
                                           const QString& error,
                                           const bool redial)
{
  const QString reason = error.isEmpty() ? tr("the connection was closed") : error;
  qWarning().noquote()
    << QStringLiteral("[%1] \"%2\": %3").arg(tr("Modbus Connection Lost"), target, reason);

  m_raisedTitle = tr("\"%1\" dropped").arg(target);
  if (bus)
    bus->publish<Core::Bus::NotificationRaised>(
      Core::Bus::kSeverityWarning, QStringLiteral("Modbus"), QString(), m_raisedTitle, reason);

  QMetaObject::invokeMethod(
    this,
    [this, redial] {
      Q_EMIT dropped();
      if (redial)
        arm();
    },
    Qt::QueuedConnection);
}

/**
 * @brief The link is back after a reported drop: posts the "Resolved: <title>" Info the alarm
 *        annunciator pairs with the drop's point (same channel, same title), once per drop.
 */
void IO::Drivers::ModbusRedial::reportRecovered(Core::Bus::MessageBus* bus)
{
  if (m_raisedTitle.isEmpty())
    return;

  if (bus)
    bus->publish<Core::Bus::NotificationRaised>(Core::Bus::kSeverityInfo,
                                                QStringLiteral("Modbus"),
                                                QString(),
                                                QStringLiteral("Resolved: %1").arg(m_raisedTitle),
                                                tr("The connection was restored"));

  m_raisedTitle.clear();
}

/**
 * @brief Whether a re-dial is scheduled.
 */
bool IO::Drivers::ModbusRedial::armed() const
{
  return m_timer.isActive();
}

/**
 * @brief The wait before a re-dial: slow enough that a server which is simply absent sees one
 *        connection attempt every ten seconds, never a retry storm.
 */
int IO::Drivers::ModbusRedial::intervalMs() noexcept
{
  return kRedialIntervalMs;
}
