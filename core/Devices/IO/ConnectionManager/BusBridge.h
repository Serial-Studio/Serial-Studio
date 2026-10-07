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

#pragma once

#include <functional>
#include <memory>

#include "Core/Bus/Subscription.h"
#include "Core/IO/FrameConfig.h"
#include "Core/SerialStudio.h"

namespace Core::Bus {
class MessageBus;
struct ProjectStructureSnapshot;
}  // namespace Core::Bus

namespace IO {

class ConnectionManager;

/**
 * @brief The connection manager's seam to the message bus (spec 0077): mirrors the retained
 *        operation mode, source-0 framing and project structure into the facade's cached members,
 *        dispatches the facade's mode, framing, licence and project reactions, and publishes the
 *        link state the libraries above read instead of reaching down.
 */
class ConnectionBusBridge {
public:
  /**
   * @brief The facade slots a retained-state change queues onto the GUI thread.
   */
  struct Reactions {
    std::function<void()> rebuildDevices;
    std::function<void()> resetFrameReader;
    std::function<void(int)> reconfigureSource;
    std::function<void()> rebuildStreams;
    std::function<bool()> deferLicenseRebuild;
    std::function<void()> observeContent;
  };

  using ProjectSnapshot = std::shared_ptr<const Core::Bus::ProjectStructureSnapshot>;

  ConnectionBusBridge(Core::Bus::MessageBus& bus,
                      SerialStudio::OperationMode& operationMode,
                      IO::FrameConfig& frameConfig,
                      ProjectSnapshot& project);
  ConnectionBusBridge(ConnectionBusBridge&&)                 = delete;
  ConnectionBusBridge(const ConnectionBusBridge&)            = delete;
  ConnectionBusBridge& operator=(ConnectionBusBridge&&)      = delete;
  ConnectionBusBridge& operator=(const ConnectionBusBridge&) = delete;

  void seedFromRetainedState();
  void wire(ConnectionManager& facade, Reactions reactions);
  void publishConnectionState(
    int sourceId, bool connected, bool paused, bool connecting, int busType);

private:
  Core::Bus::MessageBus& m_bus;
  SerialStudio::OperationMode& m_operationMode;
  IO::FrameConfig& m_frameConfig;
  ProjectSnapshot& m_project;

  Core::Bus::Subscription m_operationModeCache;
  Core::Bus::Subscription m_frameConfigCache;
  Core::Bus::Subscription m_rebuildOnModeChange;
  Core::Bus::Subscription m_resetReaderOnConfigChange;
  Core::Bus::Subscription m_rebuildOnLicenseChange;
  Core::Bus::Subscription m_disconnectRequests;
  Core::Bus::Subscription m_projectSnapshot;
};

}  // namespace IO
