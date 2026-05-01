/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "API/Handlers/SourceHandler.h"

#include <QJsonArray>
#include <QMetaObject>

#include "API/CommandRegistry.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers all project.source.* commands with the CommandRegistry.
 */
void API::Handlers::SourceHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("project.source.list"),
                           QStringLiteral("List all project sources"),
                           QJsonObject{},
                           &sourceList);

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),             QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source ID to delete")},
      {    QStringLiteral("minimum"),                                     1}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{Keys::SourceId};
    registry.registerCommand(QStringLiteral("project.source.delete"),
                             QStringLiteral("Delete a source (Commercial; sourceId >= 1)"),
                             schema,
                             &sourceDelete);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),             QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source ID to update")},
      {    QStringLiteral("minimum"),                                     0}
    };
    props[QStringLiteral("title")] = QJsonObject{
      {       QStringLiteral("type"),                   QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("New title for the source")}
    };
    props[Keys::BusType] = QJsonObject{
      {       QStringLiteral("type"),                                 QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Bus type index (0=UART, 1=Network, ...)")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{Keys::SourceId};
    registry.registerCommand(QStringLiteral("project.source.update"),
                             QStringLiteral("Update source fields (Commercial)"),
                             schema,
                             &sourceUpdate);
  }

  registry.registerCommand(QStringLiteral("project.source.add"),
                           QStringLiteral("Add a new source (Commercial)"),
                           QJsonObject{},
                           &sourceAdd);

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),   QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source ID")},
      {    QStringLiteral("minimum"),                           0}
    };
    props[QStringLiteral("key")] = QJsonObject{
      {       QStringLiteral("type"),              QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Driver property key")}
    };
    props[QStringLiteral("propertyValue")] = QJsonObject{
      {QStringLiteral("description"), QStringLiteral("Property value")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{Keys::SourceId, QStringLiteral("key"), QStringLiteral("propertyValue")};
    registry.registerCommand(
      QStringLiteral("project.source.setProperty"),
      QStringLiteral("Set a driver connection property (params: sourceId, key, value)"),
      schema,
      &sourceSetProperty);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),   QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source ID")},
      {    QStringLiteral("minimum"),                           0}
    };
    props[QStringLiteral("settings")] = QJsonObject{
      {       QStringLiteral("type"),                               QStringLiteral("object")},
      {QStringLiteral("description"), QStringLiteral("Driver properties as key/value pairs")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{Keys::SourceId, QStringLiteral("settings")};
    registry.registerCommand(
      QStringLiteral("project.source.configure"),
      QStringLiteral("Set multiple driver properties at once (params: sourceId, settings)"),
      schema,
      &sourceConfigure);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),   QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source ID")},
      {    QStringLiteral("minimum"),                           0}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{Keys::SourceId};
    registry.registerCommand(QStringLiteral("project.source.getConfiguration"),
                             QStringLiteral("Get full source configuration (params: sourceId)"),
                             schema,
                             &sourceGetConfiguration);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),   QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source ID")},
      {    QStringLiteral("minimum"),                           0}
    };
    props[QStringLiteral("code")] = QJsonObject{
      {       QStringLiteral("type"),                              QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("JavaScript frame parser source code")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{Keys::SourceId, QStringLiteral("code")};
    registry.registerCommand(
      QStringLiteral("project.source.setFrameParserCode"),
      QStringLiteral("Set per-source JS frame parser (params: sourceId, code)"),
      schema,
      &sourceSetFrameParserCode);
  }

  {
    QJsonObject props;
    props[Keys::SourceId] = QJsonObject{
      {       QStringLiteral("type"),   QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Source ID")},
      {    QStringLiteral("minimum"),                           0}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{Keys::SourceId};
    registry.registerCommand(QStringLiteral("project.source.getFrameParserCode"),
                             QStringLiteral("Get per-source JS frame parser (params: sourceId)"),
                             schema,
                             &sourceGetFrameParserCode);
  }
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns all sources in the current project.
 */
API::CommandResponse API::Handlers::SourceHandler::sourceList(const QString& id,
                                                              const QJsonObject& params)
{
  (void)params;

  // Build per-source metadata array
  const auto& sources = DataModel::ProjectModel::instance().sources();
  QJsonArray arr;
  for (const auto& src : sources) {
    QJsonObject obj;
    obj[Keys::SourceId]                   = src.sourceId;
    obj[Keys::Title]                      = src.title;
    obj[Keys::BusType]                    = src.busType;
    obj[Keys::FrameStart]                 = src.frameStart;
    obj[Keys::FrameEnd]                   = src.frameEnd;
    obj[Keys::ChecksumAlgorithm]          = src.checksumAlgorithm;
    obj[Keys::FrameDetection]             = src.frameDetection;
    obj[Keys::DecoderMethod]              = src.decoderMethod;
    obj[Keys::HexadecimalDelimiters]      = src.hexadecimalDelimiters;
    obj[QStringLiteral("hasFrameParser")] = !src.frameParserCode.isEmpty();
    arr.append(obj);
  }

  QJsonObject result;
  result[Keys::Sources]           = arr;
  result[QStringLiteral("count")] = static_cast<int>(sources.size());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Adds a new source (Commercial only).
 */
API::CommandResponse API::Handlers::SourceHandler::sourceAdd(const QString& id,
                                                             const QJsonObject& params)
{
  (void)params;

#ifndef BUILD_COMMERCIAL
  return CommandResponse::makeError(id,
                                    QStringLiteral("COMMERCIAL_REQUIRED"),
                                    QStringLiteral("Multiple data sources require a Pro license"));
#else
  const int countBefore = DataModel::ProjectModel::instance().sourceCount();
  QMetaObject::invokeMethod(&DataModel::ProjectModel::instance(), "addSource");

  const int countAfter = DataModel::ProjectModel::instance().sourceCount();
  if (countAfter <= countBefore)
    return CommandResponse::makeError(
      id, QStringLiteral("OPERATION_FAILED"), QStringLiteral("Failed to add source"));

  QJsonObject result;
  result[Keys::SourceId] = countAfter - 1;
  return CommandResponse::makeSuccess(id, result);
#endif
}

/**
 * @brief Deletes a source (Commercial only; sourceId must be >= 1).
 */
API::CommandResponse API::Handlers::SourceHandler::sourceDelete(const QString& id,
                                                                const QJsonObject& params)
{
#ifndef BUILD_COMMERCIAL
  (void)params;
  return CommandResponse::makeError(id,
                                    QStringLiteral("COMMERCIAL_REQUIRED"),
                                    QStringLiteral("Multiple data sources require a Pro license"));
#else
  if (!params.contains(Keys::SourceId))
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("sourceId is required"));

  const int sourceId = params[Keys::SourceId].toInt(-1);
  if (sourceId <= 0)
    return CommandResponse::makeError(
      id,
      QStringLiteral("INVALID_PARAM"),
      QStringLiteral("sourceId must be >= 1 (cannot delete primary source)"));

  QMetaObject::invokeMethod(&DataModel::ProjectModel::instance(),
                            "deleteSource",
                            Qt::DirectConnection,
                            Q_ARG(int, sourceId));

  return CommandResponse::makeSuccess(id);
#endif
}

/**
 * @brief Updates source fields (Commercial only).
 */
API::CommandResponse API::Handlers::SourceHandler::sourceUpdate(const QString& id,
                                                                const QJsonObject& params)
{
#ifndef BUILD_COMMERCIAL
  (void)params;
  return CommandResponse::makeError(id,
                                    QStringLiteral("COMMERCIAL_REQUIRED"),
                                    QStringLiteral("Multiple data sources require a Pro license"));
#else
  if (!params.contains(Keys::SourceId))
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("sourceId is required"));

  // Copy source and apply field updates
  const int sourceId    = params[Keys::SourceId].toInt(-1);
  const auto& sources   = DataModel::ProjectModel::instance().sources();
  const int sourceCount = static_cast<int>(sources.size());

  if (sourceId < 0 || sourceId >= sourceCount)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAM"), QStringLiteral("Invalid sourceId"));

  DataModel::Source updated = sources[sourceId];

  if (params.contains(Keys::Title))
    updated.title = params[Keys::Title].toString();

  if (params.contains(Keys::BusType))
    updated.busType = params[Keys::BusType].toInt(updated.busType);

  if (params.contains(Keys::FrameStart))
    updated.frameStart = params[Keys::FrameStart].toString();

  if (params.contains(Keys::FrameEnd))
    updated.frameEnd = params[Keys::FrameEnd].toString();

  if (params.contains(Keys::ChecksumAlgorithm))
    updated.checksumAlgorithm = params[Keys::ChecksumAlgorithm].toString();

  if (params.contains(Keys::FrameDetection))
    updated.frameDetection = params[Keys::FrameDetection].toInt(updated.frameDetection);

  if (params.contains(Keys::DecoderMethod))
    updated.decoderMethod = params[Keys::DecoderMethod].toInt(updated.decoderMethod);

  if (params.contains(Keys::HexadecimalDelimiters))
    updated.hexadecimalDelimiters = params[Keys::HexadecimalDelimiters].toBool();

  QMetaObject::invokeMethod(&DataModel::ProjectModel::instance(),
                            "updateSource",
                            Qt::DirectConnection,
                            Q_ARG(int, sourceId),
                            Q_ARG(DataModel::Source, updated));

  return CommandResponse::makeSuccess(id);
#endif
}

/**
 * @brief Applies multiple driver connection properties to a source in one call.
 *
 * Iterates over all key/value pairs in @p params["settings"] and delegates each
 * to sourceSetProperty, using the same UI-driver or editing-driver routing logic.
 */
API::CommandResponse API::Handlers::SourceHandler::sourceConfigure(const QString& id,
                                                                   const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(Keys::SourceId) || !params.contains(QStringLiteral("settings")))
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("sourceId and settings are required"));

  const int sourceId    = params[Keys::SourceId].toInt(-1);
  const auto& model     = DataModel::ProjectModel::instance();
  const int sourceCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= sourceCount)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAM"), QStringLiteral("Invalid sourceId"));

  const QJsonObject settings = params[QStringLiteral("settings")].toObject();
  const bool usesUiDriver    = sourceId == 0 && sourceCount == 1
                         && AppState::instance().operationMode() == SerialStudio::ProjectFile;

  if (usesUiDriver) {
    for (auto it = settings.constBegin(); it != settings.constEnd(); ++it)
      IO::ConnectionManager::instance().setUiDriverProperty(it.key(), it.value().toVariant());

    return CommandResponse::makeSuccess(id);
  }

  IO::HAL_Driver* driver = IO::ConnectionManager::instance().driverForEditing(sourceId);
  if (!driver)
    return CommandResponse::makeError(
      id, QStringLiteral("OPERATION_FAILED"), QStringLiteral("No driver for source"));

  for (auto it = settings.constBegin(); it != settings.constEnd(); ++it)
    driver->setDriverProperty(it.key(), it.value().toVariant());

  DataModel::ProjectModel::instance().captureSourceSettings(sourceId);

  // Sync to the live driver so projectConfigurationOk() sees the new values
  IO::HAL_Driver* live = IO::ConnectionManager::instance().driver(sourceId);
  if (live && live != driver)
    for (auto it = settings.constBegin(); it != settings.constEnd(); ++it)
      live->setDriverProperty(it.key(), it.value().toVariant());

  return CommandResponse::makeSuccess(id);
}

/**
 * @brief Sets a driver connection property for a source.
 *
 * Uses driverForEditing() to resolve the correct UI-config driver for the
 * source's bus type (avoiding activeUiDriver() which depends on m_busType
 * and may not match the source's bus type). Also syncs the change to the
 * live driver so projectConfigurationOk() sees the new value.
 */
API::CommandResponse API::Handlers::SourceHandler::sourceSetProperty(const QString& id,
                                                                     const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(Keys::SourceId) || !params.contains(QStringLiteral("key")))
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("sourceId and key are required"));

  const bool hasValue =
    params.contains(QStringLiteral("propertyValue")) || params.contains(QStringLiteral("value"));
  if (!hasValue)
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("propertyValue is required"));

  const int sourceId = params[Keys::SourceId].toInt(-1);
  if (sourceId < 0)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAM"), QStringLiteral("Invalid sourceId"));

  const QString key  = params[QStringLiteral("key")].toString();
  const QVariant val = params.contains(QStringLiteral("propertyValue"))
                       ? params[QStringLiteral("propertyValue")].toVariant()
                       : params[QStringLiteral("value")].toVariant();

  IO::HAL_Driver* driver = IO::ConnectionManager::instance().driverForEditing(sourceId);
  if (!driver)
    return CommandResponse::makeError(
      id, QStringLiteral("OPERATION_FAILED"), QStringLiteral("No driver for source"));

  driver->setDriverProperty(key, val);
  DataModel::ProjectModel::instance().captureSourceSettings(sourceId);

  // Sync to the live driver so projectConfigurationOk() sees the new value
  IO::HAL_Driver* live = IO::ConnectionManager::instance().driver(sourceId);
  if (live && live != driver)
    live->setDriverProperty(key, val);

  return CommandResponse::makeSuccess(id);
}

/**
 * @brief Returns the full configuration for a source.
 */
API::CommandResponse API::Handlers::SourceHandler::sourceGetConfiguration(const QString& id,
                                                                          const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(Keys::SourceId))
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("sourceId is required"));

  const int sourceId    = params[Keys::SourceId].toInt(-1);
  const auto& sources   = DataModel::ProjectModel::instance().sources();
  const int sourceCount = static_cast<int>(sources.size());

  if (sourceId < 0 || sourceId >= sourceCount)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAM"), QStringLiteral("Invalid sourceId"));

  const auto& src = sources[sourceId];
  QJsonObject obj = DataModel::serialize(src);
  return CommandResponse::makeSuccess(id, obj);
}

/**
 * @brief Sets the per-source JavaScript frame parser code.
 */
API::CommandResponse API::Handlers::SourceHandler::sourceSetFrameParserCode(
  const QString& id, const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(Keys::SourceId) || !params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("sourceId and code are required"));

  const int sourceId = params[Keys::SourceId].toInt(-1);
  const QString code = params[QStringLiteral("code")].toString();

  const auto& sources   = DataModel::ProjectModel::instance().sources();
  const int sourceCount = static_cast<int>(sources.size());

  if (sourceId < 0 || sourceId >= sourceCount)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAM"), QStringLiteral("Invalid sourceId"));

  QMetaObject::invokeMethod(&DataModel::ProjectModel::instance(),
                            "updateSourceFrameParser",
                            Qt::DirectConnection,
                            Q_ARG(int, sourceId),
                            Q_ARG(QString, code));

  return CommandResponse::makeSuccess(id);
}

/**
 * @brief Returns the per-source JavaScript frame parser code.
 */
API::CommandResponse API::Handlers::SourceHandler::sourceGetFrameParserCode(
  const QString& id, const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(Keys::SourceId))
    return CommandResponse::makeError(
      id, QStringLiteral("MISSING_PARAM"), QStringLiteral("sourceId is required"));

  const int sourceId    = params[Keys::SourceId].toInt(-1);
  const auto& sources   = DataModel::ProjectModel::instance().sources();
  const int sourceCount = static_cast<int>(sources.size());

  if (sourceId < 0 || sourceId >= sourceCount)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAM"), QStringLiteral("Invalid sourceId"));

  QJsonObject result;
  result[QStringLiteral("code")] = sources[sourceId].frameParserCode;
  return CommandResponse::makeSuccess(id, result);
}
