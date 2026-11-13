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

#include "API/Handlers/CSVPlayerHandler.h"

#include <limits>
#include <QJsonArray>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "Core/SerialStudio.h"
#include "CSV/Player.h"

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
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("csvPlayer.open"),
                           QStringLiteral("Open CSV file (params: filePath)"),
                           makeSchema({
                             {QStringLiteral("filePath"),
                              QStringLiteral("string"),
                              QStringLiteral("Path to the CSV file to open")}
  }),
                           &open);

  registry.registerCommand(
    QStringLiteral("csvPlayer.close"), QStringLiteral("Close CSV file"), emptySchema(), &close);
}

/**
 * @brief Register csvPlayer.setPaused/step/setProgress/getStatus commands.
 */
void API::Handlers::CSVPlayerHandler::registerPlaybackCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("csvPlayer.setPaused"),
                           QStringLiteral("Pause or resume playback (params: paused: bool)"),
                           makeSchema({
                             {QStringLiteral("paused"),
                              QStringLiteral("boolean"),
                              QStringLiteral("True to pause, false to resume")}
  }),
                           &setPaused);

  registry.registerCommand(
    QStringLiteral("csvPlayer.step"),
    QStringLiteral("Step delta frames forward (or backward if negative; default 1)"),
    makeSchema(
      {
  },
      {{QStringLiteral("delta"),
        QStringLiteral("integer"),
        QStringLiteral("Frames to advance; negative goes back. Default 1")}}),
    &step);

  registry.registerCommand(QStringLiteral("csvPlayer.setProgress"),
                           QStringLiteral("Seek to position (params: progress: 0.0-1.0)"),
                           makeSchema({
                             {QStringLiteral("progress"),
                              QStringLiteral("number"),
                              QStringLiteral("Playback position from 0.0 to 1.0"),
                              {},
                              0.0, 1.0}
  }),
                           &setProgress);

  registry.registerCommand(QStringLiteral("csvPlayer.getStatus"),
                           QStringLiteral("Get player status"),
                           emptySchema(),
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
  if (!params.contains(QStringLiteral("filePath")))
    return CommandResponse::makeMissingParam(id, "filePath");

  const QString file_path = params.value(QStringLiteral("filePath")).toString();
  if (file_path.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath cannot be empty"));
  }

  auto& player = API::handlerContext().csvPlayer;
  player.openFile(file_path);

  QJsonObject result;
  result[QStringLiteral("filePath")] = file_path;
  result[QStringLiteral("isOpen")]   = player.isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close CSV file
 */
API::CommandResponse API::Handlers::CSVPlayerHandler::close(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& player = API::handlerContext().csvPlayer;
  player.closeFile();

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
  if (!params.contains(QStringLiteral("paused")))
    return CommandResponse::makeMissingParam(id, "paused");

  auto& player = API::handlerContext().csvPlayer;

  const bool paused = params.value(QStringLiteral("paused")).toBool();
  if (paused)
    player.pause();
  else
    player.play();

  QJsonObject result;
  result[QStringLiteral("isPlaying")] = player.isPlaying();
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

  auto& player = API::handlerContext().csvPlayer;
  if (delta == 0) {
    QJsonObject result;
    result[QStringLiteral("framePosition")] = player.framePosition();
    return CommandResponse::makeSuccess(id, result);
  }

  const int absDelta =
    (delta == std::numeric_limits<int>::min()) ? std::numeric_limits<int>::max() : std::abs(delta);
  const int steps    = qBound(0, absDelta, player.frameCount());
  const bool forward = delta > 0;
  for (int i = 0; i < steps; ++i)
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
  if (!params.contains(QStringLiteral("progress")))
    return CommandResponse::makeMissingParam(id, "progress");

  const double progress = SerialStudio::toDouble(params.value(QStringLiteral("progress")));
  if (progress < 0.0 || progress > 1.0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("progress must be between 0.0 and 1.0"));
  }

  auto& player = API::handlerContext().csvPlayer;
  player.setProgress(progress);

  QJsonObject result;
  result[QStringLiteral("progress")]      = player.progress();
  result[QStringLiteral("framePosition")] = player.framePosition();
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

  auto& player = API::handlerContext().csvPlayer;

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
