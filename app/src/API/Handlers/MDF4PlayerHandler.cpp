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

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/MDF4PlayerHandler.h"

#  include "API/CommandRegistry.h"
#  include "API/PathPolicy.h"
#  include "MDF4/Player.h"

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
  registry.registerCommand(QStringLiteral("mdf4.player.open"),
                           QStringLiteral("Open MDF4 file"),
                           openSchema, &open);

  // No-param commands
  QJsonObject emptySchema;
  emptySchema.insert("type", "object");
  emptySchema.insert("properties", QJsonObject());

  registry.registerCommand(QStringLiteral("mdf4.player.close"),
                           QStringLiteral("Close MDF4 file"),
                           emptySchema, &close);

  registry.registerCommand(QStringLiteral("mdf4.player.play"),
                           QStringLiteral("Start playback"),
                           emptySchema, &play);

  registry.registerCommand(QStringLiteral("mdf4.player.pause"),
                           QStringLiteral("Pause playback"),
                           emptySchema, &pause);

  registry.registerCommand(QStringLiteral("mdf4.player.toggle"),
                           QStringLiteral("Toggle play/pause"),
                           emptySchema, &toggle);

  registry.registerCommand(QStringLiteral("mdf4.player.nextFrame"),
                           QStringLiteral("Advance to next frame"),
                           emptySchema, &nextFrame);

  registry.registerCommand(QStringLiteral("mdf4.player.previousFrame"),
                           QStringLiteral("Go to previous frame"),
                           emptySchema, &previousFrame);

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
  registry.registerCommand(QStringLiteral("mdf4.player.setProgress"),
                           QStringLiteral("Seek to position"),
                           setProgressSchema, &setProgress);

  // GetStatus query
  QJsonObject getStatusSchema;
  getStatusSchema.insert("type", "object");
  getStatusSchema.insert("properties", QJsonObject());
  registry.registerCommand(QStringLiteral("mdf4.player.getStatus"),
                           QStringLiteral("Get player status"),
                           getStatusSchema, &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Open MDF4 file
 * @param params Requires "filePath" (string)
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
 * @brief Start playback
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::play(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().play();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = MDF4::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Pause playback
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::pause(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().pause();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = MDF4::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle play/pause
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::toggle(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().toggle();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = MDF4::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Advance to next frame
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::nextFrame(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().nextFrame();

  QJsonObject result;
  result[QStringLiteral("framePosition")] = MDF4::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Go to previous frame
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::previousFrame(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().previousFrame();

  QJsonObject result;
  result[QStringLiteral("framePosition")] = MDF4::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Seek to position
 * @param params Requires "progress" (double: 0.0-1.0)
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::setProgress(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("progress"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: progress"));
  }

  const double progress = params.value(QStringLiteral("progress")).toDouble();
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

#endif  // BUILD_COMMERCIAL
