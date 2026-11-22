/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "IO/Drivers/UART/UartLineWatch.h"

#include <QJsonObject>

#include "Core/SSAssert.h"
#include "IO/Drivers/UART.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the watch to its driver as a child object, so it follows the driver's thread and
 *        lifetime; the timer stays stopped until a view attaches.
 */
IO::Drivers::UartLineWatch::UartLineWatch(UART& driver)
  : QObject(&driver)
  , m_driver(driver)
  , m_timer(this)
  , m_watchers(0)
  , m_available(false)
  , m_cts(false)
  , m_dsr(false)
  , m_dcd(false)
  , m_ri(false)
{
  m_timer.setInterval(kPollIntervalMs);
  connect(&m_timer, &QTimer::timeout, this, &UartLineWatch::poll);
}

/**
 * @brief Names where the open port is. The setup pane binds to the settings driver, which never
 *        opens one; the connection manager points its watch at the live driver of the first
 *        source. Asked on every poll, because a live driver is rebuilt on each connect.
 */
void IO::Drivers::UartLineWatch::bindSource(Source source)
{
  m_source = std::move(source);
}

//--------------------------------------------------------------------------------------------------
// Line levels
//--------------------------------------------------------------------------------------------------

/**
 * @brief Level of the Ring Indicator input at the last poll.
 */
bool IO::Drivers::UartLineWatch::ri() const noexcept
{
  return m_ri;
}

/**
 * @brief Level of the Clear To Send input at the last poll.
 */
bool IO::Drivers::UartLineWatch::cts() const noexcept
{
  return m_cts;
}

/**
 * @brief Level of the Data Set Ready input at the last poll.
 */
bool IO::Drivers::UartLineWatch::dsr() const noexcept
{
  return m_dsr;
}

/**
 * @brief Level of the Data Carrier Detect input at the last poll.
 */
bool IO::Drivers::UartLineWatch::dcd() const noexcept
{
  return m_dcd;
}

/**
 * @brief Whether the levels are those of an open port; false hides the indicators.
 */
bool IO::Drivers::UartLineWatch::available() const noexcept
{
  return m_available;
}

/**
 * @brief Views currently asking for the levels.
 */
int IO::Drivers::UartLineWatch::watchers() const noexcept
{
  return m_watchers;
}

//--------------------------------------------------------------------------------------------------
// Viewer gate and polling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Counts one more view on screen; the first one starts the poll and reads at once, so the
 *        indicators do not show a stale level for a quarter of a second.
 */
void IO::Drivers::UartLineWatch::attach()
{
  SS_ASSERT(m_watchers >= 0, m_watchers = 0);

  ++m_watchers;
  if (m_watchers != 1)
    return;

  m_timer.start();
  poll();
}

/**
 * @brief Counts one view gone; the last one stops the poll and withdraws the levels.
 */
void IO::Drivers::UartLineWatch::detach()
{
  SS_ASSERT(m_watchers > 0, return);

  --m_watchers;
  if (m_watchers > 0)
    return;

  m_timer.stop();
  store(false, false, false, false, false);
}

/**
 * @brief Reads the modem lines once. A closed port, or one a Connection Sequence still owns,
 *        reports no levels instead of the last ones it had.
 */
void IO::Drivers::UartLineWatch::poll()
{
  SS_ASSERT(m_watchers >= 0, return);
  auto* port = m_source ? m_source() : &m_driver;
  if (m_watchers == 0 || port == nullptr || !port->isOpen()) {
    store(false, false, false, false, false);
    return;
  }

  const auto states = port->lineStates();
  store(!states.isEmpty(),
        states.value(QStringLiteral("cts")).toBool(),
        states.value(QStringLiteral("dsr")).toBool(),
        states.value(QStringLiteral("dcd")).toBool(),
        states.value(QStringLiteral("ri")).toBool());
}

/**
 * @brief Stores the levels and notifies once, only when one of them moved.
 */
void IO::Drivers::UartLineWatch::store(
  const bool available, const bool cts, const bool dsr, const bool dcd, const bool ri)
{
  SS_ASSERT(available || !(cts || dsr || dcd || ri), return);
  if (available == m_available && cts == m_cts && dsr == m_dsr && dcd == m_dcd && ri == m_ri)
    return;

  m_available = available;
  m_cts       = cts;
  m_dsr       = dsr;
  m_dcd       = dcd;
  m_ri        = ri;
  Q_EMIT linesChanged();
}
