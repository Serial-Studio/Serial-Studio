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

#include <memory>
#include <utility>

#include "Core/ModuleConstruction.h"

class AppState;

namespace Core::Bus {
class MessageBus;
}  // namespace Core::Bus

namespace Console {
class Handler;
}  // namespace Console

namespace Misc {
class ModuleManager;
}  // namespace Misc

namespace DataModel {
class FrameBuilder;
class FrameParser;
class NotificationCenter;
class ProjectModel;
}  // namespace DataModel

namespace IO {
class ConnectionManager;
class PipelineHost;
}  // namespace IO

namespace UI {
class Dashboard;
}  // namespace UI

/**
 * @brief Names the subsystems of one capture session and owns each once the composition root
 *        adopts it. INV-4: an adopted address never changes. INV-5: adopt*() asserts an empty
 *        slot; the only exit is shutdown(). Ctor and dtor stay empty (spec 0039). Slot 0 is the
 *        message bus (spec 0077): adopted first, released last, handed to every module ctor.
 */
class SessionContext {
public:
  explicit SessionContext(int session_id = 0);
  virtual ~SessionContext();

  SessionContext(SessionContext&&)                 = delete;
  SessionContext(const SessionContext&)            = delete;
  SessionContext& operator=(SessionContext&&)      = delete;
  SessionContext& operator=(const SessionContext&) = delete;

  [[nodiscard]] static SessionContext& current();

  [[nodiscard]] bool sealed() const noexcept;
  [[nodiscard]] bool hasBus() const noexcept;
  [[nodiscard]] int sessionId() const noexcept;

  [[nodiscard]] virtual Core::Bus::MessageBus& bus() const;
  [[nodiscard]] virtual AppState& appState() const;
  [[nodiscard]] virtual Console::Handler& console() const;
  [[nodiscard]] virtual UI::Dashboard& dashboard() const;
  [[nodiscard]] virtual DataModel::FrameParser& frameParser() const;
  [[nodiscard]] virtual DataModel::FrameBuilder& frameBuilder() const;
  [[nodiscard]] virtual DataModel::ProjectModel& projectModel() const;
  [[nodiscard]] virtual IO::PipelineHost& pipelineHost() const;
  [[nodiscard]] virtual IO::ConnectionManager& connectionManager() const;
  [[nodiscard]] virtual DataModel::NotificationCenter& notifications() const;

  void shutdown();

  void adoptBus(std::unique_ptr<Core::Bus::MessageBus> bus);
  void adoptAppState(std::unique_ptr<AppState> module);
  void adoptDashboard(std::unique_ptr<UI::Dashboard> module);
  void adoptConsole(std::unique_ptr<Console::Handler> module);
  void adoptFrameParser(std::unique_ptr<DataModel::FrameParser> module);
  void adoptFrameBuilder(std::unique_ptr<DataModel::FrameBuilder> module);
  void adoptProjectModel(std::unique_ptr<DataModel::ProjectModel> module);
  void adoptPipelineHost(std::unique_ptr<IO::PipelineHost> module);
  void adoptConnectionManager(std::unique_ptr<IO::ConnectionManager> module);
  void adoptNotifications(std::unique_ptr<DataModel::NotificationCenter> module);

private:
  friend class Misc::ModuleManager;

  /**
   * @brief Builds a session subsystem whose constructor is private to this context; the composition
   *        root is the only caller and the pinned order keeps one construction per line (spec 0039
   *        M2). The bus and earlier slots' interfaces are handed in by reference. A QObject module
   *        is marked under construction meanwhile, so a reach into it fails by name (spec 0095).
   */
  template<typename T, typename... Args>
  [[nodiscard]] static std::unique_ptr<T> create(Args&&... args)
  {
    if constexpr (requires { T::staticMetaObject; }) {
      const Core::ModuleConstruction::Scope scope(T::staticMetaObject);
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    } else
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  int m_sessionId;
  std::unique_ptr<Core::Bus::MessageBus> m_bus;
  std::unique_ptr<AppState> m_appState;
  std::unique_ptr<UI::Dashboard> m_dashboard;
  std::unique_ptr<Console::Handler> m_console;
  std::unique_ptr<DataModel::FrameParser> m_frameParser;
  std::unique_ptr<DataModel::FrameBuilder> m_frameBuilder;
  std::unique_ptr<DataModel::ProjectModel> m_projectModel;
  std::unique_ptr<IO::PipelineHost> m_pipelineHost;
  std::unique_ptr<IO::ConnectionManager> m_connectionManager;
  std::unique_ptr<DataModel::NotificationCenter> m_notifications;
};
