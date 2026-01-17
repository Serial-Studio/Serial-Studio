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

#include "API/CommandHandler.h"
#include "API/CommandRegistry.h"
#include "API/Handlers/IOManagerHandler.h"
#include "API/Handlers/NetworkHandler.h"
#include "API/Handlers/UARTHandler.h"

/**
 * @brief Constructs the CommandHandler
 * @param parent Optional parent QObject
 */
API::CommandHandler::CommandHandler(QObject *parent)
  : QObject(parent)
  , m_initialized(false)
{
}

/**
 * @brief Gets the singleton instance of the CommandHandler
 * @return Reference to the singleton instance
 */
API::CommandHandler &API::CommandHandler::instance()
{
  static CommandHandler singleton;
  if (!singleton.m_initialized)
    singleton.initializeHandlers();
  return singleton;
}

/**
 * @brief Check if data appears to be an API message
 * @param data Raw bytes to check
 * @return true if data looks like a JSON API command
 */
bool API::CommandHandler::isApiMessage(const QByteArray &data) const
{
  return API::isApiMessage(data);
}

/**
 * @brief Process an incoming message and generate a response
 * @param data Raw JSON message bytes
 * @return JSON response bytes ready to send to client
 */
QByteArray API::CommandHandler::processMessage(const QByteArray &data)
{
  QString type;
  QJsonObject json;

  // Parse the incoming JSON
  if (!parseMessage(data, type, json))
  {
    return CommandResponse::makeError(QString(), ErrorCode::InvalidJson,
                                      QStringLiteral("Failed to parse JSON "
                                                     "message"))
        .toJsonBytes();
  }

  // Route based on message type
  if (type == MessageType::Command)
  {
    const auto request = CommandRequest::fromJson(json);
    if (!request.isValid())
    {
      return CommandResponse::makeError(request.id,
                                        ErrorCode::InvalidMessageType,
                                        QStringLiteral("Missing 'command' field"))
          .toJsonBytes();
    }
    return processCommand(request).toJsonBytes();
  }

  else if (type == MessageType::Batch)
  {
    const auto batch = BatchRequest::fromJson(json);
    if (!batch.isValid())
    {
      return CommandResponse::makeError(batch.id, ErrorCode::InvalidMessageType,
                                        QStringLiteral("Empty or invalid "
                                                       "'commands' array"))
          .toJsonBytes();
    }
    return processBatch(batch).toJsonBytes();
  }

  // Unknown message type
  return CommandResponse::makeError(QString(), ErrorCode::InvalidMessageType,
                                    QStringLiteral("Unknown message type: %1")
                                        .arg(type))
      .toJsonBytes();
}

/**
 * @brief Process a single command request
 * @param request The parsed command request
 * @return CommandResponse with result or error
 */
API::CommandResponse
API::CommandHandler::processCommand(const CommandRequest &request)
{
  return CommandRegistry::instance().execute(request.command, request.id,
                                             request.params);
}

/**
 * @brief Process a batch of commands in sequential order
 * @param batch The parsed batch request
 * @return BatchResponse with all individual command results
 *
 * Commands are executed in the order they appear in the batch.
 * Execution continues even if individual commands fail.
 */
API::BatchResponse API::CommandHandler::processBatch(const BatchRequest &batch)
{
  BatchResponse response;
  response.id = batch.id;
  response.success = true;

  for (const auto &cmd : batch.commands)
  {
    const auto result = processCommand(cmd);
    response.results.append(result);

    // Track overall batch success
    if (!result.success)
      response.success = false;
  }

  return response;
}

/**
 * @brief Get information about all available commands
 * @return JSON object with command list and descriptions
 */
QJsonObject API::CommandHandler::getAvailableCommands() const
{
  QJsonObject result;
  QJsonArray commandList;

  const auto &commands = CommandRegistry::instance().commands();
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it)
  {
    QJsonObject cmdInfo;
    cmdInfo[QStringLiteral("name")] = it.key();
    cmdInfo[QStringLiteral("description")] = it.value().description;
    commandList.append(cmdInfo);
  }

  result[QStringLiteral("commands")] = commandList;
  return result;
}

/**
 * @brief Initialize all command handlers
 *
 * This registers all available commands from the various handler classes.
 * Called automatically on first access to the CommandHandler.
 */
void API::CommandHandler::initializeHandlers()
{
  if (m_initialized)
    return;

  // Register built-in api.* commands
  CommandRegistry::instance().registerCommand(
      QStringLiteral("api.getCommands"),
      QStringLiteral("Get list of all available commands"),
      [this](const QString &id, const QJsonObject &) {
        return CommandResponse::makeSuccess(id, getAvailableCommands());
      });

  // Initialize all handler modules
  Handlers::IOManagerHandler::registerCommands();
  Handlers::UARTHandler::registerCommands();
  Handlers::NetworkHandler::registerCommands();

  m_initialized = true;
}

