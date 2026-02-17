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
 * @class MCPHandler
 * @brief Handler for Model Context Protocol (MCP) messages
 *
 * The MCPHandler implements the MCP specification and provides a bridge
 * between MCP clients (AI models) and Serial Studio's existing API.
 *
 * Key features:
 * - Automatic tool generation from CommandRegistry
 * - Real-time telemetry data as MCP resources
 * - Protocol negotiation and lifecycle management
 * - Stateful sessions per client connection
 *
 * Design: Each TCP client socket can have its own MCP session state.
 * The handler is stateless and creates/manages session objects per socket.
 */
class MCPHandler : public QObject {
  Q_OBJECT

public:
  /**
   * @struct ClientSession
   * @brief Tracks MCP session state for a client connection
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
  /**
   * @brief Gets the singleton instance
   */
  static MCPHandler& instance();

  /**
   * @brief Check if data appears to be an MCP message
   * @param data Raw bytes to check
   * @return true if data looks like an MCP JSON-RPC message
   */
  [[nodiscard]] bool isMCPMessage(const QByteArray& data) const;

  /**
   * @brief Process an incoming MCP message and return the response
   * @param data Raw JSON-RPC message bytes
   * @param sessionId Session identifier (e.g., socket pointer address)
   * @return Response as JSON bytes (ready to send back to client)
   */
  [[nodiscard]] QByteArray processMessage(const QByteArray& data, const QString& sessionId);

  /**
   * @brief Clear session state for a disconnected client
   * @param sessionId Session identifier to clean up
   */
  void clearSession(const QString& sessionId);

  /**
   * @brief Get current frame data for resource reads
   * @param frame Latest frame from Serial Studio
   */
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

  QJsonObject commandToToolSchema(const QString& name, const QString& description) const;
  void tagIoSubmodule(MCP::Tool& tool, const QString& submodule, const QStringList& parts) const;

private:
  QHash<QString, ClientSession> m_sessions;
  DataModel::Frame m_currentFrame;
  QMutex m_frameMutex;
  QVector<DataModel::Frame> m_frameHistory;
  static constexpr int kMaxFrameHistory = 100;
};

}  // namespace API
