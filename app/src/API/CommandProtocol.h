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

#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

namespace API
{
/**
 * @brief Message type identifiers for the API protocol
 */
namespace MessageType
{
constexpr const char *Batch = "batch";
constexpr const char *Command = "command";
constexpr const char *Raw = "raw";
constexpr const char *Response = "response";
} // namespace MessageType

/**
 * @brief Error codes for API responses
 */
namespace ErrorCode
{
constexpr const char *InvalidJson = "INVALID_JSON";
constexpr const char *InvalidParam = "INVALID_PARAM";
constexpr const char *MissingParam = "MISSING_PARAM";
constexpr const char *UnknownCommand = "UNKNOWN_COMMAND";
constexpr const char *ExecutionError = "EXECUTION_ERROR";
constexpr const char *OperationFailed = "OPERATION_FAILED";
constexpr const char *InvalidMessageType = "INVALID_MESSAGE_TYPE";

} // namespace ErrorCode

/**
 * @struct CommandRequest
 * @brief Represents a single command request from a client
 */
struct CommandRequest
{
  QString id;
  QString command;
  QJsonObject params;

  bool isValid() const { return !command.isEmpty(); }

  static CommandRequest fromJson(const QJsonObject &json)
  {
    CommandRequest req;
    req.id = json.value(QStringLiteral("id")).toString();
    req.command = json.value(QStringLiteral("command")).toString();
    req.params = json.value(QStringLiteral("params")).toObject();
    return req;
  }
};

/**
 * @struct BatchRequest
 * @brief Represents a batch of commands to execute in order
 */
struct BatchRequest
{
  QString id;
  QVector<CommandRequest> commands;

  bool isValid() const { return !commands.isEmpty(); }

  static BatchRequest fromJson(const QJsonObject &json)
  {
    BatchRequest req;
    req.id = json.value(QStringLiteral("id")).toString();

    const auto commandsArray = json.value(QStringLiteral("commands")).toArray();
    for (const auto &cmd : commandsArray)
      req.commands.append(CommandRequest::fromJson(cmd.toObject()));

    return req;
  }
};

/**
 * @struct CommandResponse
 * @brief Represents a response to a command request
 */
struct CommandResponse
{
  QString id;
  bool success;
  QJsonObject result;
  QString errorCode;
  QString errorMessage;

  QJsonObject toJson() const
  {
    QJsonObject json;
    json[QStringLiteral("type")] = MessageType::Response;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("success")] = success;

    if (success)
    {
      if (!result.isEmpty())
        json[QStringLiteral("result")] = result;
    }
    else
    {
      QJsonObject error;
      error[QStringLiteral("code")] = errorCode;
      error[QStringLiteral("message")] = errorMessage;
      json[QStringLiteral("error")] = error;
    }

    return json;
  }

  QByteArray toJsonBytes() const
  {
    return QJsonDocument(toJson()).toJson(QJsonDocument::Compact) + "\n";
  }

  static CommandResponse makeSuccess(const QString &id,
                                     const QJsonObject &result = QJsonObject())
  {
    CommandResponse resp;
    resp.id = id;
    resp.success = true;
    resp.result = result;
    return resp;
  }

  static CommandResponse makeError(const QString &id, const QString &code,
                                   const QString &message)
  {
    CommandResponse resp;
    resp.id = id;
    resp.success = false;
    resp.errorCode = code;
    resp.errorMessage = message;
    return resp;
  }
};

/**
 * @struct BatchResponse
 * @brief Represents a response to a batch request
 */
struct BatchResponse
{
  QString id;
  bool success;
  QVector<CommandResponse> results;

  QJsonObject toJson() const
  {
    QJsonObject json;
    json[QStringLiteral("type")] = MessageType::Response;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("success")] = success;

    QJsonArray resultsArray;
    for (const auto &result : results)
      resultsArray.append(result.toJson());

    json[QStringLiteral("results")] = resultsArray;
    return json;
  }

  QByteArray toJsonBytes() const
  {
    return QJsonDocument(toJson()).toJson(QJsonDocument::Compact) + "\n";
  }
};

/**
 * @brief Parse incoming data to determine message type
 * @param data Raw JSON data
 * @param type Output: detected message type
 * @param json Output: parsed JSON object
 * @return true if parsing succeeded
 */
inline bool parseMessage(const QByteArray &data, QString &type,
                         QJsonObject &json)
{
  QJsonParseError error;
  const auto doc = QJsonDocument::fromJson(data, &error);

  if (error.error != QJsonParseError::NoError)
    return false;

  if (!doc.isObject())
    return false;

  json = doc.object();
  type = json.value(QStringLiteral("type")).toString();

  return !type.isEmpty();
}

/**
 * @brief Check if data looks like an API message
 * @param data Raw data to check
 * @return true if data appears to be a JSON API message
 */
inline bool isApiMessage(const QByteArray &data)
{
  if (data.isEmpty())
    return false;

  // Quick check: API messages must start with '{'
  const auto trimmed = data.trimmed();
  if (trimmed.isEmpty() || trimmed.at(0) != '{')
    return false;

  // Try to parse and check for "type" field
  QString type;
  QJsonObject json;
  if (!parseMessage(data, type, json))
    return false;

  return type == MessageType::Command || type == MessageType::Batch
         || type == MessageType::Raw;
}

} // namespace API
