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

#include "DataModel/FrameBuilder/ExternalWiring.h"

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/PipelineHost.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the player mask it refreshes; nothing is subscribed until the root
 *        runs the builder's external wiring pass.
 */
DataModel::ExternalWiring::ExternalWiring(FrameBuilder& owner,
                                          std::array<bool, kPlayerSlots>& playerOpenMask)
  : m_owner(owner), m_playerOpenMask(playerOpenMask)
{
  SS_ASSERT_LOG(m_playerOpenMask.size() == static_cast<size_t>(kPlayerSlots));
}

//--------------------------------------------------------------------------------------------------
// Replay players
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors each player's open state into its mask slot on the builder's thread. The topic
 *        retains one message, so the seed covers the player that reported last; no player is
 *        open before the wiring pass in any root that runs one.
 */
void DataModel::ExternalWiring::watchPlayers()
{
  const auto apply = [this](const Core::Bus::ReplayPlayerStateChanged& state) {
    SS_ASSERT(state.playerId >= 0 && state.playerId < kPlayerSlots, return);
    m_playerOpenMask[static_cast<size_t>(state.playerId)] = state.open;
  };

  m_playerState = m_owner.m_bus.subscribe<Core::Bus::ReplayPlayerStateChanged>(
    &m_owner,
    [this, apply](const std::shared_ptr<const Core::Bus::ReplayPlayerStateChanged>& state) {
      apply(*state);
      m_owner.onPlayerOpenChanged();
    });

  m_playerOpenMask.fill(false);
  if (const auto last = m_owner.m_bus.latest<Core::Bus::ReplayPlayerStateChanged>())
    apply(*last);
}

//--------------------------------------------------------------------------------------------------
// Operation mode
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes PipelineHost's mode mirror directly at publish time (seeded from the retained
 *        state), then delivers the change to the builder on its own thread; the builder reads the
 *        mode from that mirror, so the queued hop always sees the refreshed value.
 */
void DataModel::ExternalWiring::watchOperationMode()
{
  auto& pipeline        = IO::PipelineHost::instance();
  m_operationModeMirror = m_owner.m_bus.subscribe<Core::Bus::OperationModeChanged>(
    &pipeline,
    [&pipeline](const std::shared_ptr<const Core::Bus::OperationModeChanged>& message) {
      pipeline.refreshOperationModeMirror(message->mode);
    },
    Qt::DirectConnection,
    true);
  m_operationMode = m_owner.m_bus.subscribe<Core::Bus::OperationModeChanged>(
    &m_owner, [this](const std::shared_ptr<const Core::Bus::OperationModeChanged>&) {
      m_owner.onOperationModeChanged();
    });
}

//--------------------------------------------------------------------------------------------------
// Link state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Same shape for the link state: the connected and paused mirrors are written directly on
 *        the GUI thread, then the builder's two transition slots run on the pipeline thread and
 *        decide from the mirrors whether anything moved.
 */
void DataModel::ExternalWiring::watchLinkState()
{
  auto& pipeline = IO::PipelineHost::instance();
  m_linkMirror   = m_owner.m_bus.subscribe<Core::Bus::ConnectionStateChanged>(
    &pipeline,
    [&pipeline](const std::shared_ptr<const Core::Bus::ConnectionStateChanged>& state) {
      pipeline.refreshLinkMirror(state->connected, state->paused);
    },
    Qt::DirectConnection,
    true);
  m_linkState = m_owner.m_bus.subscribe<Core::Bus::ConnectionStateChanged>(
    &m_owner, [this](const std::shared_ptr<const Core::Bus::ConnectionStateChanged>&) {
      m_owner.onConnectedChanged();
      m_owner.onPausedChanged();
    });
}

//--------------------------------------------------------------------------------------------------
// Licence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-derives the frame from the project on every licence transition (spec 0042), so a
 *        late or offline activation ships the licensed widgets instead of their fallbacks.
 */
void DataModel::ExternalWiring::watchLicense()
{
  m_license = m_owner.m_bus.subscribe<Core::Bus::LicenseStateChanged>(
    &m_owner, [this](const std::shared_ptr<const Core::Bus::LicenseStateChanged>&) {
      m_owner.syncFromProjectModel();
    });
}

//--------------------------------------------------------------------------------------------------
// Project scripting inputs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the transform engines when an input they bake in at compile time moves: the
 *        Safe/Fast Lua mode recompiles in place, and the shared Lua library (spec 0083) re-syncs
 *        the project snapshot first, because the library string is carried in that snapshot
 *        rather than read from the GUI-owned model. Both run on the builder's own thread.
 */
void DataModel::ExternalWiring::watchProjectScripting()
{
  auto& project = DataModel::ProjectModel::instance();
  QObject::connect(&project, &DataModel::ProjectModel::luaFastModeChanged, &m_owner, [this] {
    m_owner.compileTransforms();
  });
  QObject::connect(&project, &DataModel::ProjectModel::transformLibraryChanged, &m_owner, [this] {
    m_owner.syncFromProjectModel();
  });
  QObject::connect(&project, &DataModel::ProjectModel::transformLibraryJsChanged, &m_owner, [this] {
    m_owner.syncFromProjectModel();
  });
}
