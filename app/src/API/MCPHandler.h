/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>

#include "API/MCPProtocol.h"
#include "DataModel/Frame.h"

namespace API {
/**
 * @brief Handler for Model Context Protocol (MCP) messages.
 */
class MCPHandler : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Tracks MCP session state for a client connection.
   */
  struct ClientSession {
    bool initialized = false;
    QString clientName;
    QString clientVersion;
    QStringList subscribedResources;
  };

private:
  explicit MCPHandler(QObject* parent = nullptr);
  MCPHandler(MCPHandler&&)                 = delete;
  MCPHandler(const MCPHandler&)            = delete;
  MCPHandler& operator=(MCPHandler&&)      = delete;
  MCPHandler& operator=(const MCPHandler&) = delete;

public:
  [[nodiscard]] static MCPHandler& instance();

  [[nodiscard]] bool isMCPMessage(const QByteArray& data) const;
  [[nodiscard]] QByteArray processMessage(const QByteArray& data, const QString& sessionId);

  void clearSession(const QString& sessionId);
  void updateCurrentFrame(const DataModel::Frame& frame);

public slots:
  void onFrameReceived(const DataModel::Frame& frame);

private:
  MCP::MCPResponse processRequest(const MCP::MCPRequest& request, const QString& sessionId);

  MCP::MCPResponse handleInitialize(const MCP::MCPRequest& request, const QString& sessionId);
  MCP::MCPResponse handlePing(const MCP::MCPRequest& request);
  MCP::MCPResponse handleToolsList(const MCP::MCPRequest& request);
  MCP::MCPResponse handleToolsCall(const MCP::MCPRequest& request, const QString& sessionId);
  MCP::MCPResponse handleResourcesList(const MCP::MCPRequest& request);
  MCP::MCPResponse handleResourcesRead(const MCP::MCPRequest& request);
  MCP::MCPResponse handleResourcesSubscribe(const MCP::MCPRequest& request,
                                            const QString& sessionId);
  MCP::MCPResponse handleResourcesUnsubscribe(const MCP::MCPRequest& request,
                                              const QString& sessionId);
  MCP::MCPResponse handlePromptsList(const MCP::MCPRequest& request);
  MCP::MCPResponse handlePromptsGet(const MCP::MCPRequest& request);

  QVector<MCP::Tool> generateToolsFromRegistry() const;
  QVector<MCP::Resource> generateResources() const;
  QVector<MCP::Prompt> generatePrompts() const;

  void tagIoSubmodule(MCP::Tool& tool, const QString& submodule, const QStringList& parts) const;
  void tagToolFromCommandName(MCP::Tool& tool, const QStringList& parts) const;
  void tagToolModule(MCP::Tool& tool,
                     const QString& module,
                     const QString& submodule,
                     const QStringList& parts) const;
  void tagToolCommand(MCP::Tool& tool, const QString& command) const;

private:
  QHash<QString, ClientSession> m_sessions;
  DataModel::Frame m_currentFrame;
  QMutex m_frameMutex;
  QVector<DataModel::Frame> m_frameHistory;
  static constexpr int kMaxFrameHistory = 100;
};

}  // namespace API
