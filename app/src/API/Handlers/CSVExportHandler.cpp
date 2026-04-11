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

#include "API/Handlers/CSVExportHandler.h"

#include <QJsonArray>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "CSV/Export.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all CSV Export commands with the registry
 */
void API::Handlers::CSVExportHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Mutation commands
  {
    QJsonObject props;
    props[QStringLiteral("enabled")] = QJsonObject{
      {       QStringLiteral("type"),                      QStringLiteral("boolean")},
      {QStringLiteral("description"), QStringLiteral("Enable or disable CSV export")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("enabled")};
    registry.registerCommand(QStringLiteral("csv.export.setEnabled"),
                             QStringLiteral("Enable or disable CSV export (params: enabled)"),
                             schema,
                             &setEnabled);
  }

  // Empty schema for parameterless commands
  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  registry.registerCommand(QStringLiteral("csv.export.close"),
                           QStringLiteral("Close the current CSV file"),
                           emptySchema,
                           &close);

  // Query commands
  registry.registerCommand(QStringLiteral("csv.export.getStatus"),
                           QStringLiteral("Get CSV export status"),
                           emptySchema,
                           &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enable or disable CSV export
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::CSVExportHandler::setEnabled(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  CSV::Export::instance().setExportEnabled(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close the current CSV file
 */
API::CommandResponse API::Handlers::CSVExportHandler::close(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  CSV::Export::instance().closeFile();

  QJsonObject result;
  result[QStringLiteral("closed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get CSV export status
 */
API::CommandResponse API::Handlers::CSVExportHandler::getStatus(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& csvExport = CSV::Export::instance();

  QJsonObject result;
  result[QStringLiteral("enabled")] = csvExport.exportEnabled();
  result[QStringLiteral("isOpen")]  = csvExport.isOpen();

  return CommandResponse::makeSuccess(id, result);
}
