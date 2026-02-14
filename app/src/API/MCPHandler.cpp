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

#include <QMutexLocker>
#include <QJsonArray>
#include <QJsonDocument>

#include "AppInfo.h"
#include "API/MCPHandler.h"
#include "API/CommandRegistry.h"

/**
 * @brief Constructs the MCP handler
 * @param parent Optional parent QObject
 */
API::MCPHandler::MCPHandler(QObject *parent)
  : QObject(parent)
{
}

/**
 * @brief Gets the singleton instance of the MCPHandler
 * @return Reference to the singleton instance
 */
API::MCPHandler &API::MCPHandler::instance()
{
  static MCPHandler singleton;
  return singleton;
}

/**
 * @brief Check if data appears to be an MCP message
 * @param data Raw bytes to check
 * @return true if data looks like an MCP JSON-RPC message
 */
bool API::MCPHandler::isMCPMessage(const QByteArray &data) const
{
  return MCP::isMCPMessage(data);
}

/**
 * @brief Process an incoming MCP message
 * @param data Raw JSON-RPC message bytes
 * @param sessionId Session identifier for tracking client state
 * @return Response as JSON bytes
 */
QByteArray API::MCPHandler::processMessage(const QByteArray &data,
                                           const QString &sessionId)
{
  constexpr int kMaxBatchSize = 256;

  QJsonParseError error;
  const auto doc = QJsonDocument::fromJson(data, &error);

  if (error.error != QJsonParseError::NoError)
  {
    return MCP::MCPResponse::makeError(QVariant(), MCP::ErrorCode::ParseError,
                                       QStringLiteral("JSON parse error"))
        .toJsonBytes();
  }

  if (doc.isArray())
  {
    const auto batchArray = doc.array();

    if (batchArray.isEmpty())
    {
      return MCP::MCPResponse::makeError(
                 QVariant(), MCP::ErrorCode::InvalidRequest,
                 QStringLiteral("Batch cannot be empty"))
          .toJsonBytes();
    }

    if (batchArray.size() > kMaxBatchSize)
    {
      return MCP::MCPResponse::makeError(
                 QVariant(), MCP::ErrorCode::InvalidRequest,
                 QStringLiteral("Batch size exceeds limit of 256"))
          .toJsonBytes();
    }

    QJsonArray responses;

    for (const auto &item : batchArray)
    {
      if (!item.isObject())
      {
        const auto errorResponse
            = MCP::MCPResponse::makeError(QVariant(),
                                          MCP::ErrorCode::InvalidRequest,
                                          QStringLiteral("Batch item must be "
                                                         "object"))
                  .toJson();
        responses.append(errorResponse);
        continue;
      }

      const auto request = MCP::MCPRequest::fromJson(item.toObject());

      if (!request.isValid())
      {
        const auto errorResponse
            = MCP::MCPResponse::makeError(request.id,
                                          MCP::ErrorCode::InvalidRequest,
                                          QStringLiteral("Invalid JSON-RPC "
                                                         "request"))
                  .toJson();
        responses.append(errorResponse);
        continue;
      }

      if (request.isNotification())
      {
        processRequest(request, sessionId);
      }
      else
      {
        const auto response = processRequest(request, sessionId);
        responses.append(response.toJson());
      }
    }

    if (responses.isEmpty())
      return QByteArray();

    const QJsonDocument batchResponse(responses);
    return batchResponse.toJson(QJsonDocument::Compact) + "\n";
  }

  if (!doc.isObject())
  {
    return MCP::MCPResponse::makeError(QVariant(),
                                       MCP::ErrorCode::InvalidRequest,
                                       QStringLiteral("Request must be object"))
        .toJsonBytes();
  }

  const auto request = MCP::MCPRequest::fromJson(doc.object());

  if (!request.isValid())
  {
    return MCP::MCPResponse::makeError(
               request.id, MCP::ErrorCode::InvalidRequest,
               QStringLiteral("Invalid JSON-RPC request"))
        .toJsonBytes();
  }

  if (request.isNotification())
  {
    processRequest(request, sessionId);
    return QByteArray();
  }

  return processRequest(request, sessionId).toJsonBytes();
}

/**
 * @brief Clear session state for disconnected client
 * @param sessionId Session identifier to remove
 */
void API::MCPHandler::clearSession(const QString &sessionId)
{
  m_sessions.remove(sessionId);
}

/**
 * @brief Update current frame for resource reads
 * @param frame Latest frame from Serial Studio
 */
void API::MCPHandler::updateCurrentFrame(const DataModel::Frame &frame)
{
  QMutexLocker locker(&m_frameMutex);
  m_currentFrame = frame;

  m_frameHistory.append(frame);
  if (m_frameHistory.size() > kMaxFrameHistory)
    m_frameHistory.removeFirst();
}

/**
 * @brief Slot to receive frames from dashboard
 * @param frame Latest frame
 */
void API::MCPHandler::onFrameReceived(const DataModel::Frame &frame)
{
  updateCurrentFrame(frame);
}

/**
 * @brief Process an MCP request and generate response
 * @param request Parsed MCP request
 * @param sessionId Client session identifier
 * @return MCP response
 */
API::MCP::MCPResponse
API::MCPHandler::processRequest(const MCP::MCPRequest &request,
                                const QString &sessionId)
{
  const auto &method = request.method;

  if (method == MCP::Method::Initialize)
    return handleInitialize(request, sessionId);

  if (method == MCP::Method::Ping)
    return handlePing(request);

  auto &session = m_sessions[sessionId];
  if (!session.initialized && method != MCP::Method::Initialize)
  {
    return MCP::MCPResponse::makeError(
        request.id, MCP::ErrorCode::InvalidRequest,
        QStringLiteral("Session not initialized - call initialize first"));
  }

  if (method == MCP::Method::ToolsList)
    return handleToolsList(request);

  if (method == MCP::Method::ToolsCall)
    return handleToolsCall(request, sessionId);

  if (method == MCP::Method::ResourcesList)
    return handleResourcesList(request);

  if (method == MCP::Method::ResourcesRead)
    return handleResourcesRead(request);

  if (method == MCP::Method::ResourcesSubscribe)
    return handleResourcesSubscribe(request, sessionId);

  if (method == MCP::Method::ResourcesUnsubscribe)
    return handleResourcesUnsubscribe(request, sessionId);

  if (method == MCP::Method::PromptsList)
    return handlePromptsList(request);

  if (method == MCP::Method::PromptsGet)
    return handlePromptsGet(request);

  return MCP::MCPResponse::makeError(
      request.id, MCP::ErrorCode::MethodNotFound,
      QStringLiteral("Unknown method: %1").arg(method));
}

/**
 * @brief Handle MCP initialize request
 */
API::MCP::MCPResponse
API::MCPHandler::handleInitialize(const MCP::MCPRequest &request,
                                  const QString &sessionId)
{
  auto &session = m_sessions[sessionId];

  const auto clientInfo
      = request.params.value(QStringLiteral("clientInfo")).toObject();
  session.clientName = clientInfo.value(QStringLiteral("name")).toString();
  session.clientVersion
      = clientInfo.value(QStringLiteral("version")).toString();
  session.initialized = true;

  QJsonObject result;
  result[QStringLiteral("protocolVersion")] = MCP::kProtocolVersion;

  QJsonObject serverInfo;
  serverInfo[QStringLiteral("name")] = APP_NAME;
  serverInfo[QStringLiteral("version")] = APP_VERSION;
  result[QStringLiteral("serverInfo")] = serverInfo;

  QJsonObject capabilities;
  capabilities[QStringLiteral("tools")] = QJsonObject();
  capabilities[QStringLiteral("resources")]
      = QJsonObject{{QStringLiteral("subscribe"), true}};
  capabilities[QStringLiteral("prompts")] = QJsonObject();
  result[QStringLiteral("capabilities")] = capabilities;

  return MCP::MCPResponse::makeSuccess(request.id, result);
}

/**
 * @brief Handle MCP ping request
 */
API::MCP::MCPResponse
API::MCPHandler::handlePing(const MCP::MCPRequest &request)
{
  return MCP::MCPResponse::makeSuccess(request.id, QJsonObject());
}

/**
 * @brief Handle tools/list request
 */
API::MCP::MCPResponse
API::MCPHandler::handleToolsList(const MCP::MCPRequest &request)
{
  const auto tools = generateToolsFromRegistry();

  QJsonArray toolsArray;
  for (const auto &tool : tools)
    toolsArray.append(tool.toJson());

  QJsonObject result;
  result[QStringLiteral("tools")] = toolsArray;

  return MCP::MCPResponse::makeSuccess(request.id, result);
}

/**
 * @brief Handle tools/call request
 */
API::MCP::MCPResponse
API::MCPHandler::handleToolsCall(const MCP::MCPRequest &request,
                                 const QString &sessionId)
{
  Q_UNUSED(sessionId);

  const auto name = request.params.value(QStringLiteral("name")).toString();
  const auto arguments
      = request.params.value(QStringLiteral("arguments")).toObject();

  if (name.isEmpty())
  {
    return MCP::MCPResponse::makeError(request.id,
                                       MCP::ErrorCode::InvalidParams,
                                       QStringLiteral("Missing tool name"));
  }

  auto &registry = CommandRegistry::instance();
  if (!registry.hasCommand(name))
  {
    return MCP::MCPResponse::makeError(
        request.id, MCP::ErrorCode::InvalidParams,
        QStringLiteral("Unknown tool: %1").arg(name));
  }

  const auto response = registry.execute(name, QString(), arguments);

  QJsonObject contentObj;
  contentObj[QStringLiteral("type")] = QStringLiteral("text");
  contentObj[QStringLiteral("text")] = QString::fromUtf8(
      QJsonDocument(response.result).toJson(QJsonDocument::Compact));

  QJsonObject result;
  result[QStringLiteral("content")] = QJsonArray{contentObj};

  if (!response.success)
  {
    QJsonObject errorData;
    errorData[QStringLiteral("tool")] = name;
    errorData[QStringLiteral("providedArguments")] = arguments;
    errorData[QStringLiteral("commandResult")] = response.result;

    if (response.errorMessage.contains(QStringLiteral("must be"))
        && arguments.size() > 0)
    {
      const auto argKey = arguments.keys().first();
      const auto argValue = arguments.value(argKey);
      errorData[QStringLiteral("parameter")] = argKey;
      errorData[QStringLiteral("receivedValue")] = argValue;
      errorData[QStringLiteral("receivedType")]
          = QString::fromUtf8(argValue.toVariant().typeName());

      if (argValue.isString())
      {
        errorData[QStringLiteral("suggestion")]
            = QStringLiteral("Parameter '%1' appears to be a string but may "
                             "need to be a number. Try passing %2 as integer.")
                  .arg(argKey, argValue.toString());
      }
    }

    return MCP::MCPResponse::makeError(
        request.id, MCP::ErrorCode::InternalError, response.errorMessage,
        QJsonValue(errorData));
  }

  return MCP::MCPResponse::makeSuccess(request.id, result);
}

/**
 * @brief Handle resources/list request
 */
API::MCP::MCPResponse
API::MCPHandler::handleResourcesList(const MCP::MCPRequest &request)
{
  const auto resources = generateResources();

  QJsonArray resourcesArray;
  for (const auto &resource : resources)
    resourcesArray.append(resource.toJson());

  QJsonObject result;
  result[QStringLiteral("resources")] = resourcesArray;

  return MCP::MCPResponse::makeSuccess(request.id, result);
}

/**
 * @brief Handle resources/read request
 */
API::MCP::MCPResponse
API::MCPHandler::handleResourcesRead(const MCP::MCPRequest &request)
{
  const auto uri = request.params.value(QStringLiteral("uri")).toString();

  if (uri.isEmpty())
  {
    return MCP::MCPResponse::makeError(request.id,
                                       MCP::ErrorCode::InvalidParams,
                                       QStringLiteral("Missing resource URI"));
  }

  QMutexLocker locker(&m_frameMutex);

  QJsonObject content;
  content[QStringLiteral("mimeType")] = QStringLiteral("application/json");

  if (uri == QStringLiteral("serialstudio://frame/current"))
  {
    content[QStringLiteral("uri")] = uri;
    const auto frameJson = DataModel::serialize(m_currentFrame);
    content[QStringLiteral("text")] = QString::fromUtf8(
        QJsonDocument(frameJson).toJson(QJsonDocument::Indented));
  }
  else if (uri == QStringLiteral("serialstudio://frame/history"))
  {
    QJsonArray history;
    for (const auto &frame : std::as_const(m_frameHistory))
    {
      history.append(DataModel::serialize(frame));
    }

    content[QStringLiteral("uri")] = uri;
    content[QStringLiteral("text")] = QString::fromUtf8(
        QJsonDocument(history).toJson(QJsonDocument::Indented));
  }
  else
  {
    return MCP::MCPResponse::makeError(
        request.id, MCP::ErrorCode::InvalidParams,
        QStringLiteral("Unknown resource: %1").arg(uri));
  }

  QJsonObject result;
  result[QStringLiteral("contents")] = QJsonArray{content};

  return MCP::MCPResponse::makeSuccess(request.id, result);
}

/**
 * @brief Handle resources/subscribe request
 */
API::MCP::MCPResponse
API::MCPHandler::handleResourcesSubscribe(const MCP::MCPRequest &request,
                                          const QString &sessionId)
{
  const auto uri = request.params.value(QStringLiteral("uri")).toString();

  if (uri.isEmpty())
  {
    return MCP::MCPResponse::makeError(request.id,
                                       MCP::ErrorCode::InvalidParams,
                                       QStringLiteral("Missing resource URI"));
  }

  auto &session = m_sessions[sessionId];
  if (!session.subscribedResources.contains(uri))
    session.subscribedResources.append(uri);

  return MCP::MCPResponse::makeSuccess(request.id, QJsonObject());
}

/**
 * @brief Handle resources/unsubscribe request
 */
API::MCP::MCPResponse
API::MCPHandler::handleResourcesUnsubscribe(const MCP::MCPRequest &request,
                                            const QString &sessionId)
{
  const auto uri = request.params.value(QStringLiteral("uri")).toString();

  if (uri.isEmpty())
  {
    return MCP::MCPResponse::makeError(request.id,
                                       MCP::ErrorCode::InvalidParams,
                                       QStringLiteral("Missing resource URI"));
  }

  auto &session = m_sessions[sessionId];
  session.subscribedResources.removeAll(uri);

  return MCP::MCPResponse::makeSuccess(request.id, QJsonObject());
}

/**
 * @brief Handle prompts/list request
 */
API::MCP::MCPResponse
API::MCPHandler::handlePromptsList(const MCP::MCPRequest &request)
{
  const auto prompts = generatePrompts();

  QJsonArray promptsArray;
  for (const auto &prompt : prompts)
    promptsArray.append(prompt.toJson());

  QJsonObject result;
  result[QStringLiteral("prompts")] = promptsArray;

  return MCP::MCPResponse::makeSuccess(request.id, result);
}

/**
 * @brief Handle prompts/get request
 */
API::MCP::MCPResponse
API::MCPHandler::handlePromptsGet(const MCP::MCPRequest &request)
{
  const auto name = request.params.value(QStringLiteral("name")).toString();

  if (name.isEmpty())
  {
    return MCP::MCPResponse::makeError(request.id,
                                       MCP::ErrorCode::InvalidParams,
                                       QStringLiteral("Missing prompt name"));
  }

  if (name == QStringLiteral("analyze_telemetry"))
  {
    QJsonObject result;
    result[QStringLiteral("description")]
        = QStringLiteral("Analyze telemetry data from Serial Studio");

    QJsonArray messages;
    QJsonObject message;
    message[QStringLiteral("role")] = QStringLiteral("user");

    QJsonObject content;
    content[QStringLiteral("type")] = QStringLiteral("text");
    content[QStringLiteral("text")]
        = QStringLiteral("Analyze the current telemetry frame and provide "
                         "insights about the data.");

    message[QStringLiteral("content")] = content;
    messages.append(message);

    result[QStringLiteral("messages")] = messages;

    return MCP::MCPResponse::makeSuccess(request.id, result);
  }

  return MCP::MCPResponse::makeError(
      request.id, MCP::ErrorCode::InvalidParams,
      QStringLiteral("Unknown prompt: %1").arg(name));
}

/**
 * @brief Generate MCP tools from CommandRegistry
 *
 * Automatically categorizes and tags tools based on their command names
 * to help AI models organize and discover related functionality.
 */
QVector<API::MCP::Tool> API::MCPHandler::generateToolsFromRegistry() const
{
  QVector<MCP::Tool> tools;

  const auto &commands = CommandRegistry::instance().commands();
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it)
  {
    const auto &name = it.key();
    MCP::Tool tool;
    tool.name = name;
    tool.description = it.value().description;
    tool.inputSchema = commandToToolSchema(name, it.value().description);

    const auto parts = name.split(QLatin1Char('.'));
    if (parts.size() >= 2)
    {
      const auto &module = parts[0];
      const auto &submodule = parts.size() >= 3 ? parts[1] : QString();
      const auto &command = parts.last();

      if (module == QStringLiteral("io"))
      {
        tool.category = QStringLiteral("io");
        if (submodule == QStringLiteral("manager"))
        {
          tool.tags << QStringLiteral("connection") << QStringLiteral("device");
        }
        else if (submodule == QStringLiteral("driver"))
        {
          if (parts.size() >= 4)
          {
            const auto &driver = parts[2];
            tool.category = QStringLiteral("io.") + driver;
            tool.tags << driver << QStringLiteral("configuration");
          }
        }
      }
      else if (module == QStringLiteral("project"))
      {
        tool.category = QStringLiteral("project");
        tool.tags << QStringLiteral("project")
                  << QStringLiteral("configuration");
      }
      else if (module == QStringLiteral("csv"))
      {
        tool.category = QStringLiteral("export.csv");
        tool.tags << QStringLiteral("export") << QStringLiteral("csv")
                  << QStringLiteral("data");
      }
      else if (module == QStringLiteral("mdf4"))
      {
        tool.category = QStringLiteral("export.mdf4");
        tool.tags << QStringLiteral("export") << QStringLiteral("mdf4")
                  << QStringLiteral("automotive");
      }
      else if (module == QStringLiteral("dashboard"))
      {
        tool.category = QStringLiteral("dashboard");
        tool.tags << QStringLiteral("visualization")
                  << QStringLiteral("dashboard");
      }
      else if (module == QStringLiteral("console"))
      {
        tool.category = QStringLiteral("console");
        tool.tags << QStringLiteral("console") << QStringLiteral("debug");
      }
      else if (module == QStringLiteral("api"))
      {
        tool.category = QStringLiteral("api");
        tool.tags << QStringLiteral("api") << QStringLiteral("meta");
      }

      if (command.startsWith(QStringLiteral("get")))
        tool.tags << QStringLiteral("query");
      else if (command.startsWith(QStringLiteral("set")))
        tool.tags << QStringLiteral("configuration");
      else if (command.contains(QStringLiteral("start")))
        tool.tags << QStringLiteral("start");
      else if (command.contains(QStringLiteral("stop")))
        tool.tags << QStringLiteral("stop");
      else if (command.contains(QStringLiteral("connect")))
        tool.tags << QStringLiteral("connection");
    }

    tools.append(tool);
  }

  return tools;
}

/**
 * @brief Generate MCP resources
 */
QVector<API::MCP::Resource> API::MCPHandler::generateResources() const
{
  QVector<MCP::Resource> resources;

  MCP::Resource currentFrame;
  currentFrame.uri = QStringLiteral("serialstudio://frame/current");
  currentFrame.name = QStringLiteral("Current Frame");
  currentFrame.description
      = QStringLiteral("The most recent telemetry frame received");
  currentFrame.mimeType = QStringLiteral("application/json");
  resources.append(currentFrame);

  MCP::Resource frameHistory;
  frameHistory.uri = QStringLiteral("serialstudio://frame/history");
  frameHistory.name = QStringLiteral("Frame History");
  frameHistory.description = QStringLiteral("Last 100 telemetry frames");
  frameHistory.mimeType = QStringLiteral("application/json");
  resources.append(frameHistory);

  return resources;
}

/**
 * @brief Generate MCP prompts
 */
QVector<API::MCP::Prompt> API::MCPHandler::generatePrompts() const
{
  QVector<MCP::Prompt> prompts;

  MCP::Prompt analyzeTelemetry;
  analyzeTelemetry.name = QStringLiteral("analyze_telemetry");
  analyzeTelemetry.description
      = QStringLiteral("Analyze current telemetry data");
  prompts.append(analyzeTelemetry);

  return prompts;
}

/**
 * @brief Convert command to JSON Schema for MCP tool
 *
 * This function generates proper JSON Schema definitions for MCP tools,
 * enabling AI models to understand parameter types, constraints, and
 * requirements.
 */
QJsonObject
API::MCPHandler::commandToToolSchema(const QString &name,
                                     const QString &description) const
{
  Q_UNUSED(description);

  QJsonObject schema;
  schema[QStringLiteral("type")] = QStringLiteral("object");

  QJsonObject properties;
  QJsonArray required;

  if (name == QStringLiteral("io.driver.uart.setBaudRate"))
  {
    properties[QStringLiteral("baudRate")] = QJsonObject{
        {QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("description"),
         QStringLiteral("Baud rate in bits per second")},
        {QStringLiteral("enum"),
         QJsonArray{110, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600,
                    115200, 230400, 460800, 921600}},
        {QStringLiteral("default"), 115200}};
    required.append(QStringLiteral("baudRate"));
  }
  else if (name == QStringLiteral("io.driver.uart.setPortIndex"))
  {
    properties[QStringLiteral("portIndex")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
                      {QStringLiteral("description"),
                       QStringLiteral("Serial port index (0-based)")},
                      {QStringLiteral("minimum"), 0}};
    required.append(QStringLiteral("portIndex"));
  }
  else if (name == QStringLiteral("io.driver.uart.setDataBits"))
  {
    properties[QStringLiteral("dataBits")] = QJsonObject{
        {QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("description"), QStringLiteral("Number of data bits")},
        {QStringLiteral("enum"), QJsonArray{5, 6, 7, 8}},
        {QStringLiteral("default"), 8}};
    required.append(QStringLiteral("dataBits"));
  }
  else if (name == QStringLiteral("io.driver.uart.setParity"))
  {
    properties[QStringLiteral("parity")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
                      {QStringLiteral("description"),
                       QStringLiteral("Parity mode (0=None, 2=Even, 3=Odd)")},
                      {QStringLiteral("enum"), QJsonArray{0, 2, 3}},
                      {QStringLiteral("default"), 0}};
    required.append(QStringLiteral("parity"));
  }
  else if (name == QStringLiteral("io.driver.uart.setStopBits"))
  {
    properties[QStringLiteral("stopBits")] = QJsonObject{
        {QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("description"),
         QStringLiteral("Number of stop bits (1=One, 2=OneAndHalf, 3=Two)")},
        {QStringLiteral("enum"), QJsonArray{1, 2, 3}},
        {QStringLiteral("default"), 1}};
    required.append(QStringLiteral("stopBits"));
  }
  else if (name == QStringLiteral("io.driver.network.setTcpHost"))
  {
    properties[QStringLiteral("host")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                      {QStringLiteral("description"),
                       QStringLiteral("TCP host address (IP or hostname)")},
                      {QStringLiteral("default"), QStringLiteral("localhost")}};
    required.append(QStringLiteral("host"));
  }
  else if (name == QStringLiteral("io.driver.network.setTcpPort")
           || name == QStringLiteral("io.driver.network.setUdpLocalPort")
           || name == QStringLiteral("io.driver.network.setUdpRemotePort"))
  {
    properties[QStringLiteral("port")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
                      {QStringLiteral("description"),
                       QStringLiteral("Port number (1-65535)")},
                      {QStringLiteral("minimum"), 1},
                      {QStringLiteral("maximum"), 65535}};
    required.append(QStringLiteral("port"));
  }
  else if (name == QStringLiteral("io.driver.network.setUdpMulticastEnabled"))
  {
    properties[QStringLiteral("enabled")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")},
                      {QStringLiteral("description"),
                       QStringLiteral("Enable UDP multicast mode")}};
    required.append(QStringLiteral("enabled"));
  }
  else if (name == QStringLiteral("io.driver.network.setSocketType"))
  {
    properties[QStringLiteral("socketType")] = QJsonObject{
        {QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("description"),
         QStringLiteral("Socket type (0=TCP Client, 1=TCP Server, 2=UDP)")},
        {QStringLiteral("enum"), QJsonArray{0, 1, 2}},
        {QStringLiteral("default"), 0}};
    required.append(QStringLiteral("socketType"));
  }
  else if (name == QStringLiteral("io.manager.setBusType"))
  {
    properties[QStringLiteral("busType")] = QJsonObject{
        {QStringLiteral("type"), QStringLiteral("integer")},
        {QStringLiteral("description"),
         QStringLiteral(
             "Bus type (0=UART, 1=Network, 2=BLE, 3=Audio, 4=Modbus, 5=CAN)")},
        {QStringLiteral("enum"), QJsonArray{0, 1, 2, 3, 4, 5}},
        {QStringLiteral("default"), 0}};
    required.append(QStringLiteral("busType"));
  }
  else if (name == QStringLiteral("io.manager.writeData"))
  {
    properties[QStringLiteral("data")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                      {QStringLiteral("description"),
                       QStringLiteral("Data to send to connected device")}};
    required.append(QStringLiteral("data"));
  }
  else if (name == QStringLiteral("project.openFromFile"))
  {
    properties[QStringLiteral("filePath")] = QJsonObject{
        {QStringLiteral("type"), QStringLiteral("string")},
        {QStringLiteral("description"),
         QStringLiteral("Absolute path to project file (.json or .ssproj)")}};
    required.append(QStringLiteral("filePath"));
  }
  else if (name == QStringLiteral("project.setTitle"))
  {
    properties[QStringLiteral("title")] = QJsonObject{
        {QStringLiteral("type"), QStringLiteral("string")},
        {QStringLiteral("description"), QStringLiteral("Project title")}};
    required.append(QStringLiteral("title"));
  }
  else if (name == QStringLiteral("csv.export.start"))
  {
    properties[QStringLiteral("filePath")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                      {QStringLiteral("description"),
                       QStringLiteral("Optional custom CSV file path")}};
  }
  else if (name.contains(QStringLiteral("set"))
           || name.contains(QStringLiteral("write"))
           || name.contains(QStringLiteral("create"))
           || name.contains(QStringLiteral("add")))
  {
    properties[QStringLiteral("value")]
        = QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                      {QStringLiteral("description"),
                       QStringLiteral("Value to set or data to write")}};
  }

  schema[QStringLiteral("properties")] = properties;
  if (!required.isEmpty())
    schema[QStringLiteral("required")] = required;

  return schema;
}
