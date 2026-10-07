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

#include "IO/ConnectionManager/BusBridge.h"

#include <utility>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/SSAssert.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the bus and the facade's two cached members; nothing is subscribed until the
 *        wiring pass, so the spec-0044 headless root can seed and read without one.
 */
IO::ConnectionBusBridge::ConnectionBusBridge(Core::Bus::MessageBus& bus,
                                             SerialStudio::OperationMode& operationMode,
                                             IO::FrameConfig& frameConfig,
                                             ProjectSnapshot& project)
  : m_bus(bus), m_operationMode(operationMode), m_frameConfig(frameConfig), m_project(project)
{}

//--------------------------------------------------------------------------------------------------
// Retained state in
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies the operation mode and source-0 framing AppState retained on the bus ahead of the
 *        facade in the pinned order, and the project structure ProjectModel retained even earlier;
 *        a root that published no project reads an empty one rather than a null pointer.
 */
void IO::ConnectionBusBridge::seedFromRetainedState()
{
  const auto mode   = m_bus.latest<Core::Bus::OperationModeChanged>();
  const auto config = m_bus.latest<Core::Bus::FrameConfigChanged>();
  SS_ASSERT_LOG(mode != nullptr);
  SS_ASSERT_LOG(config != nullptr);
  if (mode)
    m_operationMode = static_cast<SerialStudio::OperationMode>(mode->mode);

  if (config)
    m_frameConfig = config->config;

  m_project = m_bus.latest<Core::Bus::ProjectStructureSnapshot>();
  if (!m_project)
    m_project = std::make_shared<Core::Bus::ProjectStructureSnapshot>();
}

/**
 * @brief Keeps the cached members current (direct, the publisher shares the GUI thread), queues
 *        the mode/framing/licence reactions and dispatches the project snapshot (structure and
 *        framing direct, streams queued). The content watch (spec 0094) runs direct after every
 *        snapshot and mode change, so it still sees the dispatch scope of the request behind it.
 */
void IO::ConnectionBusBridge::wire(ConnectionManager& facade, Reactions reactions)
{
  SS_ASSERT(reactions.rebuildDevices != nullptr, return);
  SS_ASSERT(reactions.resetFrameReader != nullptr, return);
  SS_ASSERT(reactions.reconfigureSource != nullptr, return);
  SS_ASSERT(reactions.rebuildStreams != nullptr, return);
  SS_ASSERT(reactions.deferLicenseRebuild != nullptr, return);
  SS_ASSERT(reactions.observeContent != nullptr, return);

  const auto observe   = std::move(reactions.observeContent);
  m_operationModeCache = m_bus.subscribe<Core::Bus::OperationModeChanged>(
    &facade,
    [this, observe](const std::shared_ptr<const Core::Bus::OperationModeChanged>& message) {
      m_operationMode = static_cast<SerialStudio::OperationMode>(message->mode);
      observe();
    },
    Qt::DirectConnection,
    true);
  m_frameConfigCache = m_bus.subscribe<Core::Bus::FrameConfigChanged>(
    &facade,
    [this](const std::shared_ptr<const Core::Bus::FrameConfigChanged>& message) {
      m_frameConfig = message->config;
    },
    Qt::DirectConnection,
    true);

  const auto rebuild          = std::move(reactions.rebuildDevices);
  const auto reset            = std::move(reactions.resetFrameReader);
  const auto defer            = std::move(reactions.deferLicenseRebuild);
  m_resetReaderOnConfigChange = m_bus.subscribe<Core::Bus::FrameConfigChanged>(
    &facade,
    [reset](const std::shared_ptr<const Core::Bus::FrameConfigChanged>&) { reset(); },
    Qt::QueuedConnection);
  m_rebuildOnModeChange = m_bus.subscribe<Core::Bus::OperationModeChanged>(
    &facade,
    [rebuild](const std::shared_ptr<const Core::Bus::OperationModeChanged>&) { rebuild(); },
    Qt::QueuedConnection);
  m_rebuildOnLicenseChange = m_bus.subscribe<Core::Bus::LicenseStateChanged>(
    &facade,
    [rebuild, defer](const std::shared_ptr<const Core::Bus::LicenseStateChanged>&) {
      if (!defer())
        rebuild();
    },
    Qt::QueuedConnection);

  const auto reconfigure    = std::move(reactions.reconfigureSource);
  const auto rebuildStreams = std::move(reactions.rebuildStreams);
  if (const auto latest = m_bus.latest<Core::Bus::ProjectStructureSnapshot>())
    m_project = latest;

  m_disconnectRequests = m_bus.subscribe<Core::Bus::DisconnectRequested>(
    &facade,
    [&facade](const std::shared_ptr<const Core::Bus::DisconnectRequested>&) {
      facade.disconnectDevice();
    },
    Qt::DirectConnection);

  m_projectSnapshot = m_bus.subscribe<Core::Bus::ProjectStructureSnapshot>(
    &facade,
    [this, rebuild, reconfigure, rebuildStreams, observe, &facade](
      const std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>& snapshot) {
      using Snapshot = Core::Bus::ProjectStructureSnapshot;
      m_project      = snapshot;
      if (snapshot->change == Snapshot::Structure)
        rebuild();
      else if (snapshot->change == Snapshot::Source)
        reconfigure(snapshot->sourceId);
      else if (snapshot->change == Snapshot::StreamLane
               || snapshot->change == Snapshot::LuaFastMode)
        QMetaObject::invokeMethod(&facade, rebuildStreams, Qt::QueuedConnection);

      observe();
    },
    Qt::DirectConnection);
}

//--------------------------------------------------------------------------------------------------
// Facts out
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retains the link state of the primary source; the facade calls this from its idempotent
 *        notifier and its pause transition, so it runs at transition rate only.
 */
void IO::ConnectionBusBridge::publishConnectionState(const int sourceId,
                                                     const bool connected,
                                                     const bool paused,
                                                     const bool connecting,
                                                     const int busType)
{
  SS_ASSERT_LOG(sourceId >= 0);
  m_bus.publishState<Core::Bus::ConnectionStateChanged>(
    sourceId, connected, paused, connecting, busType);
}
