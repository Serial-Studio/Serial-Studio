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

#include "UI/Dashboard/DashboardWiring.h"

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/SSAssert.h"
#include "UI/Dashboard.h"

/**
 * @brief Binds the facade; nothing is connected until wire() runs at the end of its constructor.
 */
UI::DashboardWiring::DashboardWiring(Dashboard& owner) : m_owner(owner), m_structureGeneration(0) {}

/**
 * @brief Publishes the structure generation on every widget-count change and the view state on
 *        every real edit (retaining the current one so a recorder wiring later replays it), and
 *        serves the two view-state requests and the licence fact for the facade.
 */
void UI::DashboardWiring::wire()
{
  SS_ASSERT(!m_viewStateRestore.isActive(), return);
  auto& bus = m_owner.m_bus;

  QObject::connect(&m_owner, &Dashboard::widgetCountChanged, &m_owner, [this, &bus] {
    ++m_structureGeneration;
    bus.publishState<Core::Bus::DashboardStructureChanged>(m_structureGeneration);
  });
  QObject::connect(&m_owner, &Dashboard::viewStateChanged, &m_owner, [this, &bus] {
    bus.publishState<Core::Bus::DashboardViewState>(m_owner.viewStateJson());
  });
  bus.publishState<Core::Bus::DashboardViewState>(m_owner.viewStateJson());
  QObject::connect(&m_owner, &Dashboard::updated, &m_owner, [&bus] {
    bus.publish<Core::Bus::DashboardUpdated>(0);
  });
  QObject::connect(&m_owner, &Dashboard::dataReset, &m_owner, [&bus] {
    bus.publish<Core::Bus::DashboardDataReset>(0);
  });

  m_replayState = bus.subscribe<Core::Bus::ReplayPlayerStateChanged>(
    &m_owner,
    [this](const std::shared_ptr<const Core::Bus::ReplayPlayerStateChanged>& state) {
      applyReplayState(*state);
    },
    Qt::DirectConnection);
  m_mirrorAttached = bus.subscribe<Core::Bus::MirrorAttachedChanged>(
    &m_owner,
    [this](const std::shared_ptr<const Core::Bus::MirrorAttachedChanged>&) {
      m_owner.updateStreamAvailable();
    },
    Qt::DirectConnection);
  m_license = bus.subscribe<Core::Bus::LicenseStateChanged>(
    &m_owner, [this](const std::shared_ptr<const Core::Bus::LicenseStateChanged>& license) {
      if (!license->activated)
        m_owner.m_plotControls.disableSweeps();

      Q_EMIT m_owner.frozenChanged();
    });
  m_viewStateClear = bus.subscribe<Core::Bus::DashboardViewStateClearRequested>(
    &m_owner,
    [this](const std::shared_ptr<const Core::Bus::DashboardViewStateClearRequested>&) {
      m_owner.clearViewState();
    },
    Qt::DirectConnection);
  m_viewStateRestore = bus.subscribe<Core::Bus::DashboardViewStateRestoreRequested>(
    &m_owner,
    [this](const std::shared_ptr<const Core::Bus::DashboardViewStateRestoreRequested>& request) {
      m_owner.setViewStateJson(request->json);
    },
    Qt::DirectConnection);
}

/**
 * @brief Records one replay player's open state in the facade's mask, refreshes the cached stream
 *        flag in the same turn (the player publishes before its first frame) and queues the data
 *        reset the player's open/close used to trigger through its signal.
 */
void UI::DashboardWiring::applyReplayState(const Core::Bus::ReplayPlayerStateChanged& state)
{
  SS_ASSERT(state.playerId >= 0 && state.playerId < 8, return);
  const auto bit = static_cast<quint8>(1u << state.playerId);
  if (state.open)
    m_owner.m_openReplayPlayers |= bit;
  else
    m_owner.m_openReplayPlayers &= static_cast<quint8>(~bit);

  m_owner.updateStreamAvailable();
  QMetaObject::invokeMethod(&m_owner, [this] { m_owner.resetData(true); }, Qt::QueuedConnection);
}
