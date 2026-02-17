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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVariant>

namespace API {
/**
 * @brief Model Context Protocol (MCP) message types and structures
 *
 * MCP is Anthropic's protocol for connecting AI models to external tools
 * and data sources. This implementation supports the core MCP specification
 * and maps Serial Studio's existing API commands to MCP tools.
 *
 * Specification: https://spec.modelcontextprotocol.io/
 */
namespace MCP {
/**
 * @brief MCP protocol version
 */
constexpr const char* kProtocolVersion = "2024-11-05";

/**
 * @brief MCP method names
 */
namespace Method {
constexpr const char* Initialize           = "initialize";
constexpr const char* Ping                 = "ping";
constexpr const char* ToolsList            = "tools/list";
constexpr const char* ToolsCall            = "tools/call";
constexpr const char* ResourcesList        = "resources/list";
constexpr const char* ResourcesRead        = "resources/read";
constexpr const char* ResourcesSubscribe   = "resources/subscribe";
constexpr const char* ResourcesUnsubscribe = "resources/unsubscribe";
constexpr const char* PromptsList          = "prompts/list";
constexpr const char* PromptsGet           = "prompts/get";
}  // namespace Method

/**
 * @brief MCP error codes
 */
namespace ErrorCode {
constexpr int ParseError     = -32700;
constexpr int InvalidRequest = -32600;
constexpr int MethodNotFound = -32601;
constexpr int InvalidParams  = -32602;
constexpr int InternalError  = -32603;
}  // namespace ErrorCode

/**
 * @struct MCPRequest
 * @brief Represents an MCP JSON-RPC 2.0 request
 */
struct MCPRequest {
  QString jsonrpc;
  QVariant id;
  QString method;
  QJsonObject params;

  bool isValid() const { return jsonrpc == QStringLiteral("2.0") && !method.isEmpty(); }

  bool isNotification() const { return id.isNull(); }

  static MCPRequest fromJson(const QJsonObject& json)
  {
    MCPRequest req;
    req.jsonrpc = json.value(QStringLiteral("jsonrpc")).toString();
    req.method  = json.value(QStringLiteral("method")).toString();
    req.params  = json.value(QStringLiteral("params")).toObject();

    const auto idValue = json.value(QStringLiteral("id"));
    if (!idValue.isNull() && !idValue.isUndefined()) {
      if (idValue.isDouble())
        req.id = idValue.toInt();
      else
        req.id = idValue.toString();
    }

    return req;
  }
};

/**
 * @struct MCPResponse
 * @brief Represents an MCP JSON-RPC 2.0 response
 */
struct MCPResponse {
  QString jsonrpc;
  QVariant id;
  QJsonValue result;
  QJsonObject error;

  bool isError() const { return !error.isEmpty(); }

  QJsonObject toJson() const
  {
    QJsonObject json;
    json[QStringLiteral("jsonrpc")] = QStringLiteral("2.0");

    if (id.typeId() == QMetaType::Int)
      json[QStringLiteral("id")] = id.toInt();
    else if (id.typeId() == QMetaType::QString)
      json[QStringLiteral("id")] = id.toString();
    else
      json[QStringLiteral("id")] = QJsonValue::Null;

    if (isError())
      json[QStringLiteral("error")] = error;
    else
      json[QStringLiteral("result")] = result;

    return json;
  }

  QByteArray toJsonBytes() const
  {
    return QJsonDocument(toJson()).toJson(QJsonDocument::Compact) + "\n";
  }

  static MCPResponse makeSuccess(const QVariant& id, const QJsonValue& result)
  {
    MCPResponse resp;
    resp.jsonrpc = QStringLiteral("2.0");
    resp.id      = id;
    resp.result  = result;
    return resp;
  }

  static MCPResponse makeError(const QVariant& id,
                               int code,
                               const QString& message,
                               const QJsonValue& data = QJsonValue::Undefined)
  {
    MCPResponse resp;
    resp.jsonrpc = QStringLiteral("2.0");
    resp.id      = id;

    QJsonObject errorObj;
    errorObj[QStringLiteral("code")]    = code;
    errorObj[QStringLiteral("message")] = message;
    if (!data.isUndefined())
      errorObj[QStringLiteral("data")] = data;

    resp.error = errorObj;
    return resp;
  }
};

/**
 * @struct Tool
 * @brief Represents an MCP tool definition
 */
struct Tool {
  QString name;
  QString description;
  QJsonObject inputSchema;
  QString category;
  QStringList tags;

  QJsonObject toJson() const
  {
    QJsonObject json;
    json[QStringLiteral("name")]        = name;
    json[QStringLiteral("description")] = description;
    json[QStringLiteral("inputSchema")] = inputSchema;

    if (!category.isEmpty())
      json[QStringLiteral("category")] = category;

    if (!tags.isEmpty())
      json[QStringLiteral("tags")] = QJsonArray::fromStringList(tags);

    return json;
  }
};

/**
 * @struct Resource
 * @brief Represents an MCP resource
 */
struct Resource {
  QString uri;
  QString name;
  QString description;
  QString mimeType;

  QJsonObject toJson() const
  {
    QJsonObject json;
    json[QStringLiteral("uri")]  = uri;
    json[QStringLiteral("name")] = name;
    if (!description.isEmpty())
      json[QStringLiteral("description")] = description;
    if (!mimeType.isEmpty())
      json[QStringLiteral("mimeType")] = mimeType;
    return json;
  }
};

/**
 * @struct Prompt
 * @brief Represents an MCP prompt template
 */
struct Prompt {
  QString name;
  QString description;
  QJsonArray arguments;

  QJsonObject toJson() const
  {
    QJsonObject json;
    json[QStringLiteral("name")] = name;
    if (!description.isEmpty())
      json[QStringLiteral("description")] = description;
    if (!arguments.isEmpty())
      json[QStringLiteral("arguments")] = arguments;
    return json;
  }
};

/**
 * @brief Check if data appears to be an MCP message
 * @param data Raw data to check
 * @return true if data appears to be an MCP JSON-RPC 2.0 message
 */
inline bool isMCPMessage(const QByteArray& data)
{
  if (data.isEmpty())
    return false;

  const auto trimmed = data.trimmed();
  if (trimmed.isEmpty())
    return false;

  const char firstChar = trimmed.at(0);
  if (firstChar != '{' && firstChar != '[')
    return false;

  QJsonParseError error;
  const auto doc = QJsonDocument::fromJson(trimmed, &error);

  if (error.error != QJsonParseError::NoError)
    return false;

  if (doc.isArray())
    return true;

  if (!doc.isObject())
    return false;

  const auto json = doc.object();

  const auto jsonrpc = json.value(QStringLiteral("jsonrpc")).toString();
  const auto method  = json.value(QStringLiteral("method")).toString();

  return jsonrpc == QStringLiteral("2.0") && !method.isEmpty();
}

}  // namespace MCP
}  // namespace API
