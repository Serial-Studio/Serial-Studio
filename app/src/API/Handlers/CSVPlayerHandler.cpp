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
#include "API/CommandRegistry.h"
#include "API/PathPolicy.h"
#include "CSV/Player.h"

/**
 * @brief Register all CSV Player commands with the registry
 */
void API::Handlers::CSVPlayerHandler::registerCommands()
{
  auto &registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("csv.player.open"),
                           QStringLiteral("Open CSV file (params: filePath)"),
                           &open);

  registry.registerCommand(QStringLiteral("csv.player.close"),
                           QStringLiteral("Close CSV file"), &close);

  registry.registerCommand(QStringLiteral("csv.player.play"),
                           QStringLiteral("Start playback"), &play);

  registry.registerCommand(QStringLiteral("csv.player.pause"),
                           QStringLiteral("Pause playback"), &pause);

  registry.registerCommand(QStringLiteral("csv.player.toggle"),
                           QStringLiteral("Toggle play/pause"), &toggle);

  registry.registerCommand(QStringLiteral("csv.player.nextFrame"),
                           QStringLiteral("Advance to next frame"), &nextFrame);

  registry.registerCommand(QStringLiteral("csv.player.previousFrame"),
                           QStringLiteral("Go to previous frame"),
                           &previousFrame);

  registry.registerCommand(
      QStringLiteral("csv.player.setProgress"),
      QStringLiteral("Seek to position (params: progress: 0.0-1.0)"),
      &setProgress);

  registry.registerCommand(QStringLiteral("csv.player.getStatus"),
                           QStringLiteral("Get player status"), &getStatus);
}

/**
 * @brief Open CSV file
 * @param params Requires "filePath" (string)
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::open(const QString &id,
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
        id, ErrorCode::InvalidParam, QStringLiteral("filePath is not allowed"));
  }

  CSV::Player::instance().openFile(file_path);

  QJsonObject result;
  result[QStringLiteral("filePath")] = file_path;
  result[QStringLiteral("isOpen")] = CSV::Player::instance().isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close CSV file
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::close(const QString &id,
                                       const QJsonObject &params)
{
  Q_UNUSED(params)

  CSV::Player::instance().closeFile();

  QJsonObject result;
  result[QStringLiteral("closed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Start playback
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::play(const QString &id,
                                      const QJsonObject &params)
{
  Q_UNUSED(params)

  CSV::Player::instance().play();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = CSV::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Pause playback
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::pause(const QString &id,
                                       const QJsonObject &params)
{
  Q_UNUSED(params)

  CSV::Player::instance().pause();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = CSV::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle play/pause
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::toggle(const QString &id,
                                        const QJsonObject &params)
{
  Q_UNUSED(params)

  CSV::Player::instance().toggle();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = CSV::Player::instance().isPlaying();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Advance to next frame
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::nextFrame(const QString &id,
                                           const QJsonObject &params)
{
  Q_UNUSED(params)

  CSV::Player::instance().nextFrame();

  QJsonObject result;
  result[QStringLiteral("framePosition")]
      = CSV::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Go to previous frame
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::previousFrame(const QString &id,
                                               const QJsonObject &params)
{
  Q_UNUSED(params)

  CSV::Player::instance().previousFrame();

  QJsonObject result;
  result[QStringLiteral("framePosition")]
      = CSV::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Seek to position
 * @param params Requires "progress" (double: 0.0-1.0)
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::setProgress(const QString &id,
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

  CSV::Player::instance().setProgress(progress);

  QJsonObject result;
  result[QStringLiteral("progress")] = CSV::Player::instance().progress();
  result[QStringLiteral("framePosition")]
      = CSV::Player::instance().framePosition();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get player status
 */
API::CommandResponse
API::Handlers::CSVPlayerHandler::getStatus(const QString &id,
                                           const QJsonObject &params)
{
  Q_UNUSED(params)

  auto &player = CSV::Player::instance();

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
