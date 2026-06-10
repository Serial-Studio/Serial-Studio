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
#include "API/Handlers/AssistantHandler.h"
#include "API/Handlers/BluetoothLEHandler.h"
#include "API/Handlers/ConsoleHandler.h"
#include "API/Handlers/ControlScriptHandler.h"
#include "API/Handlers/CSVExportHandler.h"
#include "API/Handlers/CSVPlayerHandler.h"
#include "API/Handlers/DashboardHandler.h"
#include "API/Handlers/DataTablesHandler.h"
#include "API/Handlers/ExtensionHandler.h"
#include "API/Handlers/IOManagerHandler.h"
#include "API/Handlers/NetworkHandler.h"
#include "API/Handlers/ProjectHandler.h"
#include "API/Handlers/ScriptsHandler.h"
#include "API/Handlers/SourceHandler.h"
#include "API/Handlers/UARTHandler.h"
#include "API/Handlers/WindowHandler.h"
#include "API/Handlers/WorkspacesHandler.h"
#include "API/Server.h"

#ifdef BUILD_COMMERCIAL
#  include "API/Handlers/AudioHandler.h"
#  include "API/Handlers/CANBusHandler.h"
#  include "API/Handlers/HIDHandler.h"
#  include "API/Handlers/LicensingHandler.h"
#  include "API/Handlers/MDF4ExportHandler.h"
#  include "API/Handlers/MDF4PlayerHandler.h"
#  include "API/Handlers/ModbusHandler.h"
#  include "API/Handlers/MqttHandler.h"
#  include "API/Handlers/NotificationsHandler.h"
#  include "API/Handlers/ProcessHandler.h"
#  include "API/Handlers/SessionsHandler.h"
#  include "API/Handlers/USBHandler.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kMaxBatchCommands = 256;

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CommandHandler
 */
API::CommandHandler::CommandHandler(QObject* parent) : QObject(parent), m_initialized(false) {}

/**
 * @brief Gets the singleton instance of the CommandHandler
 */
API::CommandHandler& API::CommandHandler::instance()
{
  static CommandHandler singleton;
  if (!singleton.m_initialized)
    singleton.initializeHandlers();

  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Message processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Check if data appears to be an API message
 */
bool API::CommandHandler::isApiMessage(const QByteArray& data) const
{
  return API::isApiMessage(data);
}

/**
 * @brief Process an incoming message and generate a response
 */
QByteArray API::CommandHandler::processMessage(const QByteArray& data, const CommandOrigin origin)
{
  QString type;
  QJsonObject json;

  if (!parseMessage(data, type, json)) {
    return CommandResponse::makeError(
             QString(), ErrorCode::InvalidJson, QStringLiteral("Failed to parse JSON message"))
      .toJsonBytes();
  }

  if (type == MessageType::Command) {
    const auto request = CommandRequest::fromJson(json);
    if (!request.isValid()) {
      return CommandResponse::makeError(
               request.id, ErrorCode::InvalidMessageType, QStringLiteral("Missing 'command' field"))
        .toJsonBytes();
    }
    return processCommand(request, origin).toJsonBytes();
  }

  else if (type == MessageType::Batch) {
    const QString batchId    = json.value(QStringLiteral("id")).toString();
    const auto commandsArray = json.value(QStringLiteral("commands")).toArray();

    if (commandsArray.isEmpty()) {
      return CommandResponse::makeError(batchId,
                                        ErrorCode::InvalidMessageType,
                                        QStringLiteral("Empty or invalid 'commands' array"))
        .toJsonBytes();
    }

    if (commandsArray.size() > kMaxBatchCommands) {
      return CommandResponse::makeError(
               batchId, ErrorCode::InvalidParam, QStringLiteral("Batch size exceeds limit"))
        .toJsonBytes();
    }

    const auto batch = BatchRequest::fromJson(json);
    return processBatch(batch, origin).toJsonBytes();
  }

  return CommandResponse::makeError(QString(),
                                    ErrorCode::InvalidMessageType,
                                    QStringLiteral("Unknown message type: %1").arg(type))
    .toJsonBytes();
}

/**
 * @brief Process a single command request; remote-origin device-write commands must clear the
 *        user consent gate first, matching the raw byte paths.
 */
API::CommandResponse API::CommandHandler::processCommand(const CommandRequest& request,
                                                         const CommandOrigin origin)
{
  const bool remote = (origin == CommandOrigin::Remote);
  if (remote && !Server::instance().authorizeRemoteCommand(request.command)) {
    return CommandResponse::makeError(
      request.id, ErrorCode::ExecutionError, QStringLiteral("Device write denied by user"));
  }

  return CommandRegistry::instance().execute(request.command, request.id, request.params);
}

//--------------------------------------------------------------------------------------------------
// Batch processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Process a batch of commands in sequential order
 */
API::BatchResponse API::CommandHandler::processBatch(const BatchRequest& batch,
                                                     const CommandOrigin origin)
{
  BatchResponse response;
  response.id      = batch.id;
  response.success = true;

  for (const auto& cmd : batch.commands) {
    const auto result = processCommand(cmd, origin);
    response.results.append(result);

    if (!result.success)
      response.success = false;
  }

  return response;
}

//--------------------------------------------------------------------------------------------------
// Command metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get information about all available commands
 */
QJsonObject API::CommandHandler::getAvailableCommands() const
{
  QJsonObject result;
  QJsonArray commandList;

  const auto& commands = CommandRegistry::instance().commands();
  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    QJsonObject cmdInfo;
    cmdInfo[QStringLiteral("name")]        = it.key();
    cmdInfo[QStringLiteral("description")] = it.value().description;
    commandList.append(cmdInfo);
  }

  result[QStringLiteral("commands")] = commandList;
  return result;
}

/**
 * @brief Initialize all command handlers
 */
void API::CommandHandler::initializeHandlers()
{
  if (m_initialized)
    return;

  CommandRegistry::instance().registerCommand(QStringLiteral("api.getCommands"),
                                              QStringLiteral("Get list of all available commands"),
                                              [this](const QString& id, const QJsonObject&) {
                                                return CommandResponse::makeSuccess(
                                                  id, getAvailableCommands());
                                              });

  Handlers::IOManagerHandler::registerCommands();
  Handlers::UARTHandler::registerCommands();
  Handlers::NetworkHandler::registerCommands();
  Handlers::BluetoothLEHandler::registerCommands();
  Handlers::CSVExportHandler::registerCommands();
  Handlers::ProjectHandler::registerCommands();
  Handlers::ConsoleHandler::registerCommands();
  Handlers::CSVPlayerHandler::registerCommands();
  Handlers::DashboardHandler::registerCommands();
  Handlers::WindowHandler::registerCommands();
  Handlers::SourceHandler::registerCommands();
  Handlers::ExtensionHandler::registerCommands();
  Handlers::DataTablesHandler::registerCommands();
  Handlers::WorkspacesHandler::registerCommands();
  Handlers::ScriptsHandler::registerCommands();
  Handlers::ControlScriptHandler::registerCommands();
  Handlers::AssistantHandler::registerCommands();

#ifdef BUILD_COMMERCIAL
  Handlers::ModbusHandler::registerCommands();
  Handlers::CANBusHandler::registerCommands();
  Handlers::MDF4ExportHandler::registerCommands();
  Handlers::AudioHandler::registerCommands();
  Handlers::MDF4PlayerHandler::registerCommands();
  Handlers::HIDHandler::registerCommands();
  Handlers::USBHandler::registerCommands();
  Handlers::ProcessHandler::registerCommands();
  Handlers::LicensingHandler::registerCommands();
  Handlers::NotificationsHandler::registerCommands();
  Handlers::SessionsHandler::registerCommands();
  Handlers::MqttHandler::registerCommands();
#endif

  m_initialized = true;
}
