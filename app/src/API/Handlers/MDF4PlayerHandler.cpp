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

/**
 * @brief Register all MDF4 Player commands with the registry
 */
void API::Handlers::MDF4PlayerHandler::registerCommands()
{
  auto &registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("mdf4.player.open"),
                           QStringLiteral("Open MDF4 file (params: filePath)"),
                           &open);

  registry.registerCommand(QStringLiteral("mdf4.player.close"),
                           QStringLiteral("Close MDF4 file"), &close);

  registry.registerCommand(QStringLiteral("mdf4.player.play"),
                           QStringLiteral("Start playback"), &play);

  registry.registerCommand(QStringLiteral("mdf4.player.pause"),
                           QStringLiteral("Pause playback"), &pause);

  registry.registerCommand(QStringLiteral("mdf4.player.toggle"),
                           QStringLiteral("Toggle play/pause"), &toggle);

  registry.registerCommand(QStringLiteral("mdf4.player.nextFrame"),
                           QStringLiteral("Advance to next frame"), &nextFrame);

  registry.registerCommand(QStringLiteral("mdf4.player.previousFrame"),
                           QStringLiteral("Go to previous frame"),
                           &previousFrame);

  registry.registerCommand(
      QStringLiteral("mdf4.player.setProgress"),
      QStringLiteral("Seek to position (params: progress: 0.0-1.0)"),
      &setProgress);

  registry.registerCommand(QStringLiteral("mdf4.player.getStatus"),
                           QStringLiteral("Get player status"), &getStatus);
}

/**
 * @brief Open MDF4 file
 * @param params Requires "filePath" (string)
 */
API::CommandResponse
API::Handlers::MDF4PlayerHandler::open(const QString &id,
                                       const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("filePath")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: filePath"));
  }

  const QString file_path = params.value(QStringLiteral("filePath")).toString();
  if (file_path.isEmpty())
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("filePath cannot be empty"));
  }

  if (!API::isPathAllowed(file_path))
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("filePath is not allowed"));
  }

  MDF4::Player::instance().openFile(file_path);

  QJsonObject result;
  result[QStringLiteral("filePath")] = file_path;
  result[QStringLiteral("isOpen")] = MDF4::Player::instance().isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close MDF4 file
 */
API::CommandResponse
API::Handlers::MDF4PlayerHandler::close(const QString &id,
                                        const QJsonObject &params)
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
API::CommandResponse
API::Handlers::MDF4PlayerHandler::play(const QString &id,
                                       const QJsonObject &params)
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
API::CommandResponse
API::Handlers::MDF4PlayerHandler::pause(const QString &id,
                                        const QJsonObject &params)
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
API::CommandResponse
API::Handlers::MDF4PlayerHandler::toggle(const QString &id,
                                         const QJsonObject &params)
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
API::CommandResponse
API::Handlers::MDF4PlayerHandler::nextFrame(const QString &id,
                                            const QJsonObject &params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().nextFrame();

  QJsonObject result;
  result[QStringLiteral("framePosition")]
      = MDF4::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Go to previous frame
 */
API::CommandResponse
API::Handlers::MDF4PlayerHandler::previousFrame(const QString &id,
                                                const QJsonObject &params)
{
  Q_UNUSED(params)

  MDF4::Player::instance().previousFrame();

  QJsonObject result;
  result[QStringLiteral("framePosition")]
      = MDF4::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Seek to position
 * @param params Requires "progress" (double: 0.0-1.0)
 */
API::CommandResponse
API::Handlers::MDF4PlayerHandler::setProgress(const QString &id,
                                              const QJsonObject &params)
{
  if (!params.contains(QStringLiteral("progress")))
  {
    return CommandResponse::makeError(
        id, ErrorCode::MissingParam,
        QStringLiteral("Missing required parameter: progress"));
  }

  const double progress = params.value(QStringLiteral("progress")).toDouble();
  if (progress < 0.0 || progress > 1.0)
  {
    return CommandResponse::makeError(
        id, ErrorCode::InvalidParam,
        QStringLiteral("progress must be between 0.0 and 1.0"));
  }

  MDF4::Player::instance().setProgress(progress);

  QJsonObject result;
  result[QStringLiteral("progress")] = MDF4::Player::instance().progress();
  result[QStringLiteral("framePosition")]
      = MDF4::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get player status
 */
API::CommandResponse
API::Handlers::MDF4PlayerHandler::getStatus(const QString &id,
                                            const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &player = MDF4::Player::instance();

  QJsonObject result;
  result[QStringLiteral("isOpen")] = player.isOpen();
  result[QStringLiteral("isPlaying")] = player.isPlaying();
  result[QStringLiteral("frameCount")] = player.frameCount();
  result[QStringLiteral("framePosition")] = player.framePosition();
  result[QStringLiteral("progress")] = player.progress();
  result[QStringLiteral("timestamp")] = player.timestamp();
  result[QStringLiteral("filename")] = player.filename();

  return CommandResponse::makeSuccess(id, result);
}

#endif // BUILD_COMMERCIAL
