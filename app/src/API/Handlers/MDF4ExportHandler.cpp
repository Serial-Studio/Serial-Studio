/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "API/Handlers/MDF4ExportHandler.h"

#include "API/CommandRegistry.h"
#include "MDF4/Export.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all MDF4 Export commands with the registry
 */
void API::Handlers::MDF4ExportHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Mutation commands
  QJsonObject setEnabledSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "boolean");
    prop.insert("description", "Whether to enable MDF4 export");
    props.insert("enabled", prop);
    setEnabledSchema.insert("type", "object");
    setEnabledSchema.insert("properties", props);
    QJsonArray req;
    req.append("enabled");
    setEnabledSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("mdf4.export.setEnabled"),
                           QStringLiteral("Enable or disable MDF4 export"),
                           setEnabledSchema,
                           &setEnabled);

  QJsonObject closeSchema;
  closeSchema.insert("type", "object");
  closeSchema.insert("properties", QJsonObject());
  registry.registerCommand(QStringLiteral("mdf4.export.close"),
                           QStringLiteral("Close the current MDF4 file"),
                           closeSchema,
                           &close);

  // Query commands
  QJsonObject getStatusSchema;
  getStatusSchema.insert("type", "object");
  getStatusSchema.insert("properties", QJsonObject());
  registry.registerCommand(QStringLiteral("mdf4.export.getStatus"),
                           QStringLiteral("Get MDF4 export status"),
                           getStatusSchema,
                           &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enable or disable MDF4 export
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::MDF4ExportHandler::setEnabled(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  MDF4::Export::instance().setExportEnabled(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close the current MDF4 file
 */
API::CommandResponse API::Handlers::MDF4ExportHandler::close(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  MDF4::Export::instance().closeFile();

  QJsonObject result;
  result[QStringLiteral("closed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get MDF4 export status
 */
API::CommandResponse API::Handlers::MDF4ExportHandler::getStatus(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& mdf4Export = MDF4::Export::instance();

  QJsonObject result;
  result[QStringLiteral("enabled")] = mdf4Export.exportEnabled();
  result[QStringLiteral("isOpen")]  = mdf4Export.isOpen();

  return CommandResponse::makeSuccess(id, result);
}
