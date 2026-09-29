/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "UI/Alarms/AppEventSounds.h"

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/SSAssert.h"
#include "IO/ConnectionManager.h"
#include "UI/Alarms/SoundTheme.h"

//--------------------------------------------------------------------------------------------------
// Constructor & wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Holds the references; wiring waits for setupExternalConnections().
 */
UI::Alarms::AppEventSounds::AppEventSounds(Core::Bus::MessageBus& bus,
                                           IO::ConnectionManager& connectionManager,
                                           QObject* parent)
  : QObject(parent)
  , m_armed(false)
  , m_wasConnected(false)
  , m_lastCloseWasDrop(false)
  , m_bus(bus)
  , m_connectionManager(connectionManager)
{}

/**
 * @brief Allows the Connected sound: a session that auto-connects during project restore is
 *        not an event the operator caused (spec R18).
 */
void UI::Alarms::AppEventSounds::arm()
{
  m_armed        = true;
  m_wasConnected = m_connectionManager.isConnected();
}

/**
 * @brief Subscribes to the connection edge and the bus events; direct-thread, command rate.
 */
void UI::Alarms::AppEventSounds::setupExternalConnections()
{
  SS_ASSERT(!m_appEvents.isActive(), return);

  connect(&m_connectionManager,
          &IO::ConnectionManager::connectedChanged,
          this,
          &AppEventSounds::onConnectedChanged);

  m_appEvents = m_bus.subscribe<Core::Bus::AppEventRaised>(
    this, [this](const std::shared_ptr<const Core::Bus::AppEventRaised>& event) {
      if (event->kind == Core::Bus::kAppEventErrorDialogShown)
        Q_EMIT eventRequested(static_cast<int>(Slot::Error));
      else if (event->kind == Core::Bus::kAppEventExportFinished)
        Q_EMIT eventRequested(static_cast<int>(Slot::ExportFinished));
    });
}

/**
 * @brief Records a sink's initial state and returns its index.
 */
std::size_t UI::Alarms::AppEventSounds::registerSink(bool open)
{
  m_sinkOpen.push_back(open);
  return m_sinkOpen.size() - 1;
}

/**
 * @brief Emits Recording Started or Stopped when a sink's open state actually changed.
 */
void UI::Alarms::AppEventSounds::onSinkEdge(std::size_t index, bool open)
{
  SS_ASSERT(index < m_sinkOpen.size(), return);
  if (open == m_sinkOpen[index])
    return;

  m_sinkOpen[index] = open;
  Q_EMIT eventRequested(static_cast<int>(open ? Slot::RecordingStarted : Slot::RecordingStopped));
}

//--------------------------------------------------------------------------------------------------
// Connection edges
//--------------------------------------------------------------------------------------------------

/**
 * @brief Classifies the edge: connect after a drop is Reconnected, a close the operator asked
 *        for is Disconnected, any other close is Link Lost.
 */
void UI::Alarms::AppEventSounds::onConnectedChanged()
{
  const bool connected = m_connectionManager.isConnected();
  if (connected == m_wasConnected)
    return;

  m_wasConnected = connected;
  if (connected) {
    if (!m_armed)
      return;

    Q_EMIT eventRequested(
      static_cast<int>(m_lastCloseWasDrop ? Slot::Reconnected : Slot::Connected));
    m_lastCloseWasDrop = false;
    return;
  }

  m_lastCloseWasDrop = !m_connectionManager.lastCloseRequested();
  Q_EMIT linkClosed(m_lastCloseWasDrop);
  if (m_armed)
    Q_EMIT eventRequested(
      static_cast<int>(m_lastCloseWasDrop ? Slot::LinkLost : Slot::Disconnected));
}
