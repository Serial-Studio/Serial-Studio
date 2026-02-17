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

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/MDF4ExportHandler.h"

#  include "API/CommandRegistry.h"
#  include "MDF4/Export.h"

/**
 * @brief Register all MDF4 Export commands with the registry
 */
void API::Handlers::MDF4ExportHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Mutation commands
  registry.registerCommand(QStringLiteral("mdf4.export.setEnabled"),
                           QStringLiteral("Enable or disable MDF4 export (params: enabled)"),
                           &setEnabled);

  registry.registerCommand(
    QStringLiteral("mdf4.export.close"), QStringLiteral("Close the current MDF4 file"), &close);

  // Query commands
  registry.registerCommand(
    QStringLiteral("mdf4.export.getStatus"), QStringLiteral("Get MDF4 export status"), &getStatus);
}

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

#endif  // BUILD_COMMERCIAL
