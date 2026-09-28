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

#pragma once

#include <functional>
#include <QObject>
#include <vector>

#include "Core/Bus/Subscription.h"

namespace Core::Bus {
class MessageBus;
}  // namespace Core::Bus

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace UI {
namespace Alarms {

/**
 * @brief Turns application events into theme slot requests (spec 0087 R16): the connection
 *        edges (Connected, Reconnected, Disconnected, Link Lost), every recording sink's open
 *        edges, and the bus events other libraries raise. Publishes nothing per frame.
 */
class AppEventSounds : public QObject {
  Q_OBJECT

signals:
  void eventRequested(int slot);
  void linkClosed();

public:
  explicit AppEventSounds(Core::Bus::MessageBus& bus,
                          IO::ConnectionManager& connectionManager,
                          QObject* parent = nullptr);

  void arm();
  void setupExternalConnections();

  /**
   * @brief Watches one recording sink's open edges; the sinks share no base class, so the
   *        signal arrives as a typed member pointer and @p isOpen reads the current state.
   */
  template<typename Sink>
  void watchRecordingSink(Sink* sink, void (Sink::*signal)(), std::function<bool()> isOpen)
  {
    const std::size_t index = registerSink(isOpen());
    connect(sink, signal, this, [this, index, isOpen = std::move(isOpen)] {
      onSinkEdge(index, isOpen());
    });
  }

private slots:
  void onConnectedChanged();

private:
  [[nodiscard]] std::size_t registerSink(bool open);
  void onSinkEdge(std::size_t index, bool open);

  bool m_armed;
  bool m_wasConnected;
  bool m_lastCloseWasDrop;
  Core::Bus::MessageBus& m_bus;
  IO::ConnectionManager& m_connectionManager;
  Core::Bus::Subscription m_appEvents;
  std::vector<bool> m_sinkOpen;
};

}  // namespace Alarms
}  // namespace UI
