/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
 *
 * This file is part of Serial Studio Pro. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/S7Handler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/S7.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers every io.s7.* command with the registry.
 */
void API::Handlers::S7Handler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.s7.getStatus"),
                           QStringLiteral("Get the S7comm session status and counters"),
                           API::emptySchema(),
                           &getStatus);

  registry.registerCommand(QStringLiteral("io.s7.getConfig"),
                           QStringLiteral("Get the S7comm driver configuration"),
                           API::emptySchema(),
                           &getConfig);

  registry.registerCommand(QStringLiteral("io.s7.setProperty"),
                           QStringLiteral("Set an S7comm driver property (params: key, value)"),
                           API::makeSchema({
                             {  QStringLiteral("key"),
                              QStringLiteral("string"),
                              QStringLiteral("Property key (host, rack, slot, pollInterval)")},
                             {QStringLiteral("value"),
                              QStringLiteral("string|integer"),
                              QStringLiteral("New property value")                           },
  }),
                           &setProperty);

  registry.registerCommand(QStringLiteral("io.s7.addVariable"),
                           QStringLiteral("Add an S7 variable (params: name, address)"),
                           API::makeSchema(
                             {
                               {QStringLiteral("address"),
                                QStringLiteral("string"),
                                QStringLiteral("Absolute S7 address, e.g. DB5.DBD20:REAL")}
  },
                             {{QStringLiteral("name"),
                               QStringLiteral("string"),
                               QStringLiteral("Channel name shown on the dashboard")}}),
                           &addVariable);

  API::SchemaProp index;
  index.name        = QStringLiteral("index");
  index.type        = QStringLiteral("integer");
  index.description = QStringLiteral("Zero-based position in the variable list");
  index.minimum     = 0;
  registry.registerCommand(QStringLiteral("io.s7.removeVariable"),
                           QStringLiteral("Remove an S7 variable by index (params: index)"),
                           API::makeSchema({index}),
                           &removeVariable);

  registry.registerCommand(QStringLiteral("io.s7.clearVariables"),
                           QStringLiteral("Remove every configured S7 variable"),
                           API::emptySchema(),
                           &clearVariables);

  registry.registerCommand(QStringLiteral("io.s7.generateProject"),
                           QStringLiteral("Build a project from the configured S7 variables"),
                           API::emptySchema(),
                           &generateProject);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the pulled diagnostics snapshot of the live session.
 */
API::CommandResponse API::Handlers::S7Handler::getStatus(const QString& id,
                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  return CommandResponse::makeSuccess(id, manager.s7()->statusJson());
}

/**
 * @brief Returns the endpoint configuration and the variable list.
 */
API::CommandResponse API::Handlers::S7Handler::getConfig(const QString& id,
                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.s7();

  QJsonObject result;
  result[QStringLiteral("host")]         = driver->host();
  result[QStringLiteral("rack")]         = driver->rack();
  result[QStringLiteral("slot")]         = driver->slot();
  result[QStringLiteral("pollInterval")] = driver->pollInterval();
  result[QStringLiteral("variables")]    = driver->variablesJson();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies one driver property through the generic round-trip surface. The key is checked
 *        against the driver's own property model and the value against that property's editor type
 *        and range, so an unknown key or a wrong-typed value is refused rather than silently
 *        coercing to zero or an empty string.
 */
API::CommandResponse API::Handlers::S7Handler::setProperty(const QString& id,
                                                           const QJsonObject& params)
{
  const auto key = params.value(QStringLiteral("key")).toString();
  if (key.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: key"));
  }

  if (!params.contains(QStringLiteral("value"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: value"));
  }

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.s7();

  const IO::DriverProperty* match = nullptr;
  const auto props                = driver->driverProperties();
  for (const auto& prop : props)
    if (prop.key == key) {
      match = &prop;
      break;
    }

  if (!match) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown property key: %1").arg(key));
  }

  const auto value = params.value(QStringLiteral("value"));
  if (match->type == IO::DriverProperty::IntField) {
    if (!value.isDouble()) {
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("Property \"%1\" expects an integer").arg(key));
    }

    const int number = value.toInt();
    if ((match->min.isValid() && number < match->min.toInt())
        || (match->max.isValid() && number > match->max.toInt())) {
      return CommandResponse::makeError(id,
                                        ErrorCode::InvalidParam,
                                        QStringLiteral("Property \"%1\" must be between %2 and %3")
                                          .arg(key)
                                          .arg(match->min.toInt())
                                          .arg(match->max.toInt()));
    }
  } else if (!value.isString()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Property \"%1\" expects a string").arg(key));
  }

  driver->setDriverProperty(key, value.toVariant());

  QJsonObject result;
  result[QStringLiteral("key")] = key;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Appends one variable; a malformed address is refused with the parser's own reason.
 */
API::CommandResponse API::Handlers::S7Handler::addVariable(const QString& id,
                                                           const QJsonObject& params)
{
  const auto address = params.value(QStringLiteral("address")).toString();
  if (address.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: address"));
  }

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.s7();

  const auto reason = driver->validateAddress(address);
  if (!reason.isEmpty())
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, reason);

  const int before = driver->variableCount();
  driver->addVariable(params.value(QStringLiteral("name")).toString(), address);
  if (driver->variableCount() == before) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("The variable could not be added"));
  }

  QJsonObject result;
  result[QStringLiteral("variableCount")] = driver->variableCount();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Removes the variable at the given position.
 */
API::CommandResponse API::Handlers::S7Handler::removeVariable(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("index"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: index"));
  }

  if (!params.value(QStringLiteral("index")).isDouble()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Parameter \"index\" must be an integer"));
  }

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.s7();

  const int index = params.value(QStringLiteral("index")).toInt();
  if (index < 0 || index >= driver->variableCount()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid variable index"));
  }

  driver->removeVariable(index);

  QJsonObject result;
  result[QStringLiteral("variableCount")] = driver->variableCount();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Drops every configured variable.
 */
API::CommandResponse API::Handlers::S7Handler::clearVariables(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.s7();
  driver->clearVariables();

  QJsonObject result;
  result[QStringLiteral("variableCount")] = driver->variableCount();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Builds a project from the variable list and loads it into the editor.
 */
API::CommandResponse API::Handlers::S7Handler::generateProject(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.s7();
  if (!driver->loadGeneratedProject()) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("The project could not be generated"));
  }

  QJsonObject result;
  result[QStringLiteral("datasets")] = driver->wireSchema().size();
  return CommandResponse::makeSuccess(id, result);
}
