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

#include "API/Handlers/CSVPlayerHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/PathPolicy.h"
#include "CSV/Player.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all CSV Player commands with the registry
 */
void API::Handlers::CSVPlayerHandler::registerCommands()
{
  registerFileCommands();
  registerPlaybackCommands();
}

/**
 * @brief Register csvPlayer.open + csvPlayer.close commands.
 */
void API::Handlers::CSVPlayerHandler::registerFileCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  QJsonObject openFilePathProp;
  openFilePathProp.insert(QStringLiteral("type"), QStringLiteral("string"));
  openFilePathProp.insert(QStringLiteral("description"),
                          QStringLiteral("Path to the CSV file to open"));

  QJsonObject openProps;
  openProps.insert(QStringLiteral("filePath"), openFilePathProp);

  QJsonArray openRequired;
  openRequired.append(QStringLiteral("filePath"));

  QJsonObject openSchema;
  openSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  openSchema.insert(QStringLiteral("properties"), openProps);
  openSchema.insert(QStringLiteral("required"), openRequired);

  registry.registerCommand(QStringLiteral("csvPlayer.open"),
                           QStringLiteral("Open CSV file (params: filePath)"),
                           openSchema,
                           &open);

  registry.registerCommand(
    QStringLiteral("csvPlayer.close"), QStringLiteral("Close CSV file"), emptySchema, &close);
}

/**
 * @brief Register csvPlayer.setPaused/step/setProgress/getStatus commands.
 */
void API::Handlers::CSVPlayerHandler::registerPlaybackCommands()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  QJsonObject progressProp;
  progressProp.insert(QStringLiteral("type"), QStringLiteral("number"));
  progressProp.insert(QStringLiteral("description"),
                      QStringLiteral("Playback position from 0.0 to 1.0"));
  progressProp.insert(QStringLiteral("minimum"), 0.0);
  progressProp.insert(QStringLiteral("maximum"), 1.0);

  QJsonObject progressProps;
  progressProps.insert(QStringLiteral("progress"), progressProp);

  QJsonArray progressRequired;
  progressRequired.append(QStringLiteral("progress"));

  QJsonObject progressSchema;
  progressSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  progressSchema.insert(QStringLiteral("properties"), progressProps);
  progressSchema.insert(QStringLiteral("required"), progressRequired);

  QJsonObject pausedProp;
  pausedProp.insert(QStringLiteral("type"), QStringLiteral("boolean"));
  pausedProp.insert(QStringLiteral("description"),
                    QStringLiteral("True to pause, false to resume"));

  QJsonObject pausedProps;
  pausedProps.insert(QStringLiteral("paused"), pausedProp);

  QJsonArray pausedRequired;
  pausedRequired.append(QStringLiteral("paused"));

  QJsonObject pausedSchema;
  pausedSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  pausedSchema.insert(QStringLiteral("properties"), pausedProps);
  pausedSchema.insert(QStringLiteral("required"), pausedRequired);

  QJsonObject deltaProp;
  deltaProp.insert(QStringLiteral("type"), QStringLiteral("integer"));
  deltaProp.insert(QStringLiteral("description"),
                   QStringLiteral("Frames to advance; negative goes back. Default 1"));

  QJsonObject stepProps;
  stepProps.insert(QStringLiteral("delta"), deltaProp);

  QJsonObject stepSchema;
  stepSchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  stepSchema.insert(QStringLiteral("properties"), stepProps);

  registry.registerCommand(QStringLiteral("csvPlayer.setPaused"),
                           QStringLiteral("Pause or resume playback (params: paused: bool)"),
                           pausedSchema,
                           &setPaused);

  registry.registerCommand(
    QStringLiteral("csvPlayer.step"),
    QStringLiteral("Step delta frames forward (or backward if negative; default 1)"),
    stepSchema,
    &step);

  registry.registerCommand(QStringLiteral("csvPlayer.setProgress"),
                           QStringLiteral("Seek to position (params: progress: 0.0-1.0)"),
                           progressSchema,
                           &setProgress);

  registry.registerCommand(QStringLiteral("csvPlayer.getStatus"),
                           QStringLiteral("Get player status"),
                           emptySchema,
                           &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Open CSV file
 */
API::CommandResponse API::Handlers::CSVPlayerHandler::open(const QString& id,
                                                           const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("filePath"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: filePath"));
  }

  const QString file_path = params.value(QStringLiteral("filePath")).toString();
  if (file_path.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath cannot be empty"));
  }

  if (!API::isPathAllowed(file_path)) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath is not allowed"));
  }

  CSV::Player::instance().openFile(file_path);

  QJsonObject result;
  result[QStringLiteral("filePath")] = file_path;
  result[QStringLiteral("isOpen")]   = CSV::Player::instance().isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close CSV file
 */
API::CommandResponse API::Handlers::CSVPlayerHandler::close(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  CSV::Player::instance().closeFile();

  QJsonObject result;
  result[QStringLiteral("closed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Pause or resume playback (params: paused: bool)
 */
API::CommandResponse API::Handlers::CSVPlayerHandler::setPaused(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("paused"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: paused"));
  }

  const bool paused = params.value(QStringLiteral("paused")).toBool();
  if (paused)
    CSV::Player::instance().pause();
  else
    CSV::Player::instance().play();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = CSV::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Step delta frames forward (or backward if negative; default 1)
 */
API::CommandResponse API::Handlers::CSVPlayerHandler::step(const QString& id,
                                                           const QJsonObject& params)
{
  const int delta =
    params.contains(QStringLiteral("delta")) ? params.value(QStringLiteral("delta")).toInt() : 1;

  auto& player = CSV::Player::instance();
  if (delta == 0) {
    QJsonObject result;
    result[QStringLiteral("framePosition")] = player.framePosition();
    return CommandResponse::makeSuccess(id, result);
  }

  const int absDelta = std::abs(delta);
  const bool forward = delta > 0;
  for (int i = 0; i < absDelta; ++i)
    if (forward)
      player.nextFrame();
    else
      player.previousFrame();

  QJsonObject result;
  result[QStringLiteral("framePosition")] = player.framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Seek to position
 */
API::CommandResponse API::Handlers::CSVPlayerHandler::setProgress(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("progress"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: progress"));
  }

  const double progress = SerialStudio::toDouble(params.value(QStringLiteral("progress")));
  if (progress < 0.0 || progress > 1.0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("progress must be between 0.0 and 1.0"));
  }

  CSV::Player::instance().setProgress(progress);

  QJsonObject result;
  result[QStringLiteral("progress")]      = CSV::Player::instance().progress();
  result[QStringLiteral("framePosition")] = CSV::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get player status
 */
API::CommandResponse API::Handlers::CSVPlayerHandler::getStatus(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& player = CSV::Player::instance();

  QJsonObject result;
  result[QStringLiteral("isOpen")]        = player.isOpen();
  result[QStringLiteral("isPlaying")]     = player.isPlaying();
  result[QStringLiteral("frameCount")]    = player.frameCount();
  result[QStringLiteral("framePosition")] = player.framePosition();
  result[QStringLiteral("progress")]      = player.progress();
  result[QStringLiteral("timestamp")]     = player.timestamp();
  result[QStringLiteral("filename")]      = player.filename();

  return CommandResponse::makeSuccess(id, result);
}
