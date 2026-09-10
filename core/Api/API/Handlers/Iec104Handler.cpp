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

#include "API/Handlers/Iec104Handler.h"

#include <algorithm>
#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/Iec104.h"

/**
 * @brief The setProperty command schema: the key is constrained to the known property names and
 *        the value accepts either the string endpoint or an integer parameter.
 */
[[nodiscard]] static QJsonObject setPropertyIec104Schema()
{
  API::SchemaProp key;
  key.name        = QStringLiteral("key");
  key.type        = QStringLiteral("string");
  key.description = QStringLiteral("Property key");
  key.enumValues  = QJsonArray{QStringLiteral("host"),
                              QStringLiteral("port"),
                              QStringLiteral("commonAddress"),
                              QStringLiteral("windowK"),
                              QStringLiteral("windowW"),
                              QStringLiteral("timeoutT1"),
                              QStringLiteral("timeoutT2"),
                              QStringLiteral("timeoutT3")};

  API::SchemaProp value;
  value.name        = QStringLiteral("value");
  value.type        = QStringLiteral("string|integer");
  value.description = QStringLiteral("New property value");

  return API::makeSchema({key, value});
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers every io.iec104.* command with the registry.
 */
void API::Handlers::Iec104Handler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.iec104.getStatus"),
                           QStringLiteral("Get the IEC 60870-5-104 link status and counters"),
                           API::emptySchema(),
                           &getStatus);

  registry.registerCommand(QStringLiteral("io.iec104.getConfig"),
                           QStringLiteral("Get the IEC 60870-5-104 driver configuration"),
                           API::emptySchema(),
                           &getConfig);

  registry.registerCommand(QStringLiteral("io.iec104.getPoints"),
                           QStringLiteral("List the information objects the station has reported"),
                           API::emptySchema(),
                           &getPoints);

  registry.registerCommand(
    QStringLiteral("io.iec104.setProperty"),
    QStringLiteral("Set an IEC 60870-5-104 driver property (params: key, value)"),
    setPropertyIec104Schema(),
    &setProperty);

  registry.registerCommand(QStringLiteral("io.iec104.clearPoints"),
                           QStringLiteral("Forget every discovered information object"),
                           API::emptySchema(),
                           &clearPoints);

  registry.registerCommand(QStringLiteral("io.iec104.generateProject"),
                           QStringLiteral("Build a project from the discovered points"),
                           API::emptySchema(),
                           &generateProject);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the pulled diagnostics snapshot of the live session.
 */
API::CommandResponse API::Handlers::Iec104Handler::getStatus(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = API::handlerContext().connectionManager;
  return CommandResponse::makeSuccess(id, manager.iec104()->statusJson());
}

/**
 * @brief Returns the endpoint and the protocol parameters.
 */
API::CommandResponse API::Handlers::Iec104Handler::getConfig(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = API::handlerContext().connectionManager;
  auto* driver  = manager.iec104();

  QJsonObject result;
  result[QStringLiteral("host")]          = driver->host();
  result[QStringLiteral("port")]          = driver->port();
  result[QStringLiteral("commonAddress")] = driver->commonAddress();
  result[QStringLiteral("windowK")]       = driver->windowK();
  result[QStringLiteral("windowW")]       = driver->windowW();
  result[QStringLiteral("timeoutT1")]     = driver->timeoutT1();
  result[QStringLiteral("timeoutT2")]     = driver->timeoutT2();
  result[QStringLiteral("timeoutT3")]     = driver->timeoutT3();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the discovered point table in wire order; the position of an entry is the wire
 *        slot its dataset reads, so the order is part of the answer.
 */
API::CommandResponse API::Handlers::Iec104Handler::getPoints(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = API::handlerContext().connectionManager;
  auto* driver  = manager.iec104();

  QJsonObject result;
  result[QStringLiteral("points")]     = driver->pointsJson();
  result[QStringLiteral("pointCount")] = driver->pointCount();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies one driver property through the generic round-trip surface. The key and value are
 *        validated against the driver's own property model FIRST: a bad key or a non-integer value
 *        for a numeric field is rejected here, because the setters silently coerce a wrong-typed
 *        value to zero and then clamp and persist it, which would corrupt the stored configuration.
 */
API::CommandResponse API::Handlers::Iec104Handler::setProperty(const QString& id,
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

  auto& manager = API::handlerContext().connectionManager;
  auto* driver  = manager.iec104();

  const auto props = driver->driverProperties();
  const auto match = std::find_if(
    props.cbegin(), props.cend(), [&key](const IO::DriverProperty& p) { return p.key == key; });
  if (match == props.cend()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown property: %1").arg(key));
  }

  const auto value = params.value(QStringLiteral("value")).toVariant();
  if (match->type == IO::DriverProperty::IntField) {
    bool ok        = false;
    const int cast = value.toInt(&ok);
    if (!ok) {
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("Property %1 expects an integer").arg(key));
    }

    if ((match->min.isValid() && cast < match->min.toInt())
        || (match->max.isValid() && cast > match->max.toInt())) {
      return CommandResponse::makeError(id,
                                        ErrorCode::InvalidParam,
                                        QStringLiteral("Property %1 must be between %2 and %3")
                                          .arg(key)
                                          .arg(match->min.toInt())
                                          .arg(match->max.toInt()));
    }
  }

  driver->setDriverProperty(key, value);

  QJsonObject result;
  result[QStringLiteral("key")] = key;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Drops the discovered point table so the next session rediscovers it from scratch.
 */
API::CommandResponse API::Handlers::Iec104Handler::clearPoints(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = API::handlerContext().connectionManager;
  auto* driver  = manager.iec104();
  driver->clearPoints();

  QJsonObject result;
  result[QStringLiteral("pointCount")] = driver->pointCount();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Builds a project from the discovered points and loads it into the editor.
 */
API::CommandResponse API::Handlers::Iec104Handler::generateProject(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& manager = API::handlerContext().connectionManager;
  auto* driver  = manager.iec104();
  if (!driver->loadGeneratedProject()) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("The project could not be generated"));
  }

  QJsonObject result;
  result[QStringLiteral("datasets")] = driver->wireSchema().size();
  return CommandResponse::makeSuccess(id, result);
}
