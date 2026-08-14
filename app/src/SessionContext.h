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

class AppState;

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
 *        adopts it. INV-4: an adopted address never changes, so every frozen reference stays
 *        valid. INV-5: adopt*() asserts an empty slot; the only exit is shutdown(). The ctor
 *        and dtor stay empty or a module ctor re-enters current()'s Meyers guard (spec 0039).
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
  [[nodiscard]] int sessionId() const noexcept;

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
   * @brief Builds a session subsystem whose constructor is private to this context. The
   *        composition root is the only caller, so a module stays unconstructible everywhere
   *        else while the pinned order keeps one construction per line (spec 0039 M2).
   */
  template<typename T>
  [[nodiscard]] static std::unique_ptr<T> create()
  {
    return std::unique_ptr<T>(new T());
  }

  int m_sessionId;
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
