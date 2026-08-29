/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/InfluxHandler.h"

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "InfluxDB/Export.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers every InfluxDB sink command with the registry.
 */
void API::Handlers::InfluxHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("influx.setConfig"),
    QStringLiteral("Configure the InfluxDB 2.x sink: server URL, organization, bucket and the "
                   "measurement every point is written under. Every field is optional, so a call "
                   "changes only what it names. The token is write-only: it goes to the "
                   "machine-bound credential vault, never to the project file, and no verb ever "
                   "reads it back."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("url"),
        QStringLiteral("string"),
        QStringLiteral("Server URL, e.g. http://localhost:8086")},
       {QStringLiteral("org"),
        QStringLiteral("string"),
        QStringLiteral("Organization that owns the bucket")},
       {QStringLiteral("bucket"), QStringLiteral("string"), QStringLiteral("Destination bucket")},
       {QStringLiteral("measurement"),
        QStringLiteral("string"),
        QStringLiteral("Measurement name (default: serial_studio)")},
       {QStringLiteral("token"),
        QStringLiteral("string"),
        QStringLiteral("API token; stored in the credential vault, write-only")}}),
    &setConfig);

  registry.registerCommand(
    QStringLiteral("influx.setEnabled"),
    QStringLiteral("Enable or disable the InfluxDB sink for the current project. Requires a "
                   "valid commercial licence; an unlicensed build reports enabled=false."),
    API::makeSchema({
      {QStringLiteral("enabled"),
       QStringLiteral("boolean"),
       QStringLiteral("Whether to write points to InfluxDB")}
  }),
    &setEnabled);

  registry.registerCommand(
    QStringLiteral("influx.getStatus"),
    QStringLiteral("Snapshot the sink: enabled/isOpen, the endpoint fields, whether a token is "
                   "stored, and the pulled counters (pointsWritten, pointsDropped, "
                   "fieldsSkipped, httpErrors) plus the last write error."),
    API::emptySchema(),
    &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the endpoint configuration; absent fields are left untouched. A present field of
 *        the wrong type, or an insecure URL, is rejected wholesale rather than silently coerced.
 */
API::CommandResponse API::Handlers::InfluxHandler::setConfig(const QString& id,
                                                             const QJsonObject& params)
{
  for (const QString& key : {QStringLiteral("url"),
                             QStringLiteral("org"),
                             QStringLiteral("bucket"),
                             QStringLiteral("measurement"),
                             QStringLiteral("token")}) {
    if (params.contains(key) && !params.value(key).isString())
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("Parameter '%1' must be a string").arg(key));
  }

  const QString url = params.value(QStringLiteral("url")).toString();
  if (params.contains(QStringLiteral("url")) && !url.trimmed().isEmpty()
      && !InfluxDB::Export::urlSchemeAllowed(url))
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("URL must use https, or http only for a loopback host"));

  static auto& sink = InfluxDB::Export::instance();

  if (params.contains(QStringLiteral("url")))
    sink.setUrl(url);

  if (params.contains(QStringLiteral("org")))
    sink.setOrganization(params.value(QStringLiteral("org")).toString());

  if (params.contains(QStringLiteral("bucket")))
    sink.setBucket(params.value(QStringLiteral("bucket")).toString());

  if (params.contains(QStringLiteral("measurement")))
    sink.setMeasurement(params.value(QStringLiteral("measurement")).toString());

  const QString token = params.value(QStringLiteral("token")).toString();
  if (params.contains(QStringLiteral("token")) && !token.isEmpty())
    sink.setToken(token);

  QJsonObject result;
  result[QStringLiteral("url")]         = sink.url();
  result[QStringLiteral("org")]         = sink.organization();
  result[QStringLiteral("bucket")]      = sink.bucket();
  result[QStringLiteral("measurement")] = sink.measurement();
  result[QStringLiteral("hasToken")]    = sink.hasToken();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enables or disables the sink; the reported state is what the licence actually allowed.
 */
API::CommandResponse API::Handlers::InfluxHandler::setEnabled(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const QJsonValue enabled = params.value(QStringLiteral("enabled"));
  if (!enabled.isBool()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Parameter 'enabled' must be a boolean"));
  }

  static auto& sink = InfluxDB::Export::instance();
  sink.setExportEnabled(enabled.toBool());

  QJsonObject result;
  result[QStringLiteral("enabled")] = sink.exportEnabled();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports the sink's configuration and pulled counters. The token is never echoed back.
 */
API::CommandResponse API::Handlers::InfluxHandler::getStatus(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  static const auto& sink = InfluxDB::Export::instance();

  QJsonObject result;
  result[QStringLiteral("enabled")]     = sink.exportEnabled();
  result[QStringLiteral("isOpen")]      = sink.isOpen();
  result[QStringLiteral("url")]         = sink.url();
  result[QStringLiteral("org")]         = sink.organization();
  result[QStringLiteral("bucket")]      = sink.bucket();
  result[QStringLiteral("measurement")] = sink.measurement();
  result[QStringLiteral("hasToken")]    = sink.hasToken();

  result[QStringLiteral("pointsWritten")] = static_cast<qint64>(sink.pointsWritten());
  result[QStringLiteral("pointsDropped")] = static_cast<qint64>(sink.pointsDropped());
  result[QStringLiteral("fieldsSkipped")] = static_cast<qint64>(sink.fieldsSkipped());
  result[QStringLiteral("httpErrors")]    = static_cast<qint64>(sink.httpErrors());
  result[QStringLiteral("lastError")]     = sink.lastError();

  return CommandResponse::makeSuccess(id, result);
}
