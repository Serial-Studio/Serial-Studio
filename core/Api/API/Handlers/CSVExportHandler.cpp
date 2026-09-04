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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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
  static auto& registry = CommandRegistry::instance();

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
    registry.registerCommand(QStringLiteral("csvExport.setEnabled"),
                             QStringLiteral("Enable or disable CSV export (params: enabled)"),
                             schema,
                             &setEnabled);
  }

  {
    QJsonObject props;
    props[QStringLiteral("intervalMs")] = QJsonObject{
      {       QStringLiteral("type"),QStringLiteral("integer")                                     },
      {QStringLiteral("description"),
       QStringLiteral("Snapshot interval in milliseconds; 0 restores one row per frame")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("intervalMs")};
    registry.registerCommand(
      QStringLiteral("csvExport.setInterval"),
      QStringLiteral("Set the CSV logging cadence (params: intervalMs). 0 (default) writes one "
                     "row per received frame; a positive value switches the recorder to a "
                     "fixed-interval snapshot log where every row carries the latest value of "
                     "every column -- use this for multi-source or high-rate projects where "
                     "per-frame rows explode the file size. Applies live to an open recording "
                     "and persists across restarts."),
      schema,
      &setInterval);
  }

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  registry.registerCommand(QStringLiteral("csvExport.close"),
                           QStringLiteral("Close the current CSV file"),
                           emptySchema,
                           &close);

  registry.registerCommand(QStringLiteral("csvExport.getStatus"),
                           QStringLiteral("Get CSV export status"),
                           emptySchema,
                           &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enable or disable CSV export
 */
API::CommandResponse API::Handlers::CSVExportHandler::setEnabled(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled     = params.value(QStringLiteral("enabled")).toBool();
  static auto& csvExport = CSV::Export::instance();
  csvExport.setExportEnabled(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the CSV snapshot interval (0 = per-frame rows)
 */
API::CommandResponse API::Handlers::CSVExportHandler::setInterval(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("intervalMs"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: intervalMs"));
  }

  const int interval = params.value(QStringLiteral("intervalMs")).toInt(-1);
  if (interval < 0) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid intervalMs: must be an integer >= 0 (0 = one row per frame)"));
  }

  static auto& csvExport = CSV::Export::instance();
  csvExport.setExportInterval(interval);

  QJsonObject result;
  result[QStringLiteral("intervalMs")] = csvExport.exportInterval();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close the current CSV file
 */
API::CommandResponse API::Handlers::CSVExportHandler::close(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& csvExport = CSV::Export::instance();
  csvExport.closeFile();

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

  static auto& csvExport = CSV::Export::instance();

  QJsonObject result;
  result[QStringLiteral("enabled")]    = csvExport.exportEnabled();
  result[QStringLiteral("isOpen")]     = csvExport.isOpen();
  result[QStringLiteral("intervalMs")] = csvExport.exportInterval();

  return CommandResponse::makeSuccess(id, result);
}
