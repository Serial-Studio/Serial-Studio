/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is part of Serial Studio Pro. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/MDF4PlayerHandler.h"

#include "API/CommandRegistry.h"
#include "API/PathPolicy.h"
#include "MDF4/Player.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all MDF4 Player commands with the registry
 */
void API::Handlers::MDF4PlayerHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Open command
  QJsonObject openSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "string");
    prop.insert("description", "Path to the MDF4 file to open");
    props.insert("filePath", prop);
    openSchema.insert("type", "object");
    openSchema.insert("properties", props);
    QJsonArray req;
    req.append("filePath");
    openSchema.insert("required", req);
  }
  registry.registerCommand(
    QStringLiteral("mdf4Player.open"), QStringLiteral("Open MDF4 file"), openSchema, &open);

  // No-param commands
  QJsonObject emptySchema;
  emptySchema.insert("type", "object");
  emptySchema.insert("properties", QJsonObject());

  registry.registerCommand(
    QStringLiteral("mdf4Player.close"), QStringLiteral("Close MDF4 file"), emptySchema, &close);

  // setPaused command
  QJsonObject pausedSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "boolean");
    prop.insert("description", "True to pause, false to resume");
    props.insert("paused", prop);
    pausedSchema.insert("type", "object");
    pausedSchema.insert("properties", props);
    QJsonArray req;
    req.append("paused");
    pausedSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("mdf4Player.setPaused"),
                           QStringLiteral("Pause or resume playback (params: paused: bool)"),
                           pausedSchema,
                           &setPaused);

  // step command
  QJsonObject stepSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "integer");
    prop.insert("description", "Frames to advance; negative goes back. Default 1");
    props.insert("delta", prop);
    stepSchema.insert("type", "object");
    stepSchema.insert("properties", props);
  }
  registry.registerCommand(
    QStringLiteral("mdf4Player.step"),
    QStringLiteral("Step delta frames forward (or backward if negative; default 1)"),
    stepSchema,
    &step);

  // SetProgress command
  QJsonObject setProgressSchema;
  {
    QJsonObject props;
    QJsonObject prop;
    prop.insert("type", "number");
    prop.insert("description", "Playback position from 0.0 to 1.0");
    prop.insert("minimum", 0.0);
    prop.insert("maximum", 1.0);
    props.insert("progress", prop);
    setProgressSchema.insert("type", "object");
    setProgressSchema.insert("properties", props);
    QJsonArray req;
    req.append("progress");
    setProgressSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("mdf4Player.setProgress"),
                           QStringLiteral("Seek to position"),
                           setProgressSchema,
                           &setProgress);

  // GetStatus query
  QJsonObject getStatusSchema;
  getStatusSchema.insert("type", "object");
  getStatusSchema.insert("properties", QJsonObject());
  registry.registerCommand(QStringLiteral("mdf4Player.getStatus"),
                           QStringLiteral("Get player status"),
                           getStatusSchema,
                           &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Open MDF4 file
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::open(const QString& id,
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

  MDF4::Player::instance().openFile(file_path);

  QJsonObject result;
  result[QStringLiteral("filePath")] = file_path;
  result[QStringLiteral("isOpen")]   = MDF4::Player::instance().isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close MDF4 file
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::close(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().closeFile();

  QJsonObject result;
  result[QStringLiteral("closed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Pause or resume playback (params: paused: bool)
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::setPaused(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("paused"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: paused"));
  }

  const bool paused = params.value(QStringLiteral("paused")).toBool();
  if (paused)
    MDF4::Player::instance().pause();
  else
    MDF4::Player::instance().play();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = MDF4::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Step delta frames forward (or backward if negative; default 1)
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::step(const QString& id,
                                                            const QJsonObject& params)
{
  const int delta =
    params.contains(QStringLiteral("delta")) ? params.value(QStringLiteral("delta")).toInt() : 1;

  auto& player = MDF4::Player::instance();
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
API::CommandResponse API::Handlers::MDF4PlayerHandler::setProgress(const QString& id,
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

  MDF4::Player::instance().setProgress(progress);

  QJsonObject result;
  result[QStringLiteral("progress")]      = MDF4::Player::instance().progress();
  result[QStringLiteral("framePosition")] = MDF4::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get player status
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::getStatus(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& player = MDF4::Player::instance();

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
