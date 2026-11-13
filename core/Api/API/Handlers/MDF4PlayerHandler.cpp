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

#include <QJsonArray>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "Core/SerialStudio.h"
#include "MDF4/Player.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all MDF4 Player commands with the registry
 */
void API::Handlers::MDF4PlayerHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("mdf4Player.open"),
                           QStringLiteral("Open MDF4 file (params: filePath)"),
                           makeSchema({
                             {QStringLiteral("filePath"),
                              QStringLiteral("string"),
                              QStringLiteral("Path to the MDF4 file to open")}
  }),
                           &open);

  registry.registerCommand(
    QStringLiteral("mdf4Player.close"), QStringLiteral("Close MDF4 file"), emptySchema(), &close);

  registry.registerCommand(QStringLiteral("mdf4Player.setPaused"),
                           QStringLiteral("Pause or resume playback (params: paused: bool)"),
                           makeSchema({
                             {QStringLiteral("paused"),
                              QStringLiteral("boolean"),
                              QStringLiteral("True to pause, false to resume")}
  }),
                           &setPaused);

  registry.registerCommand(
    QStringLiteral("mdf4Player.step"),
    QStringLiteral("Step delta frames forward (or backward if negative; default 1)"),
    makeSchema(
      {
  },
      {{QStringLiteral("delta"),
        QStringLiteral("integer"),
        QStringLiteral("Frames to advance; negative goes back. Default 1")}}),
    &step);

  registry.registerCommand(QStringLiteral("mdf4Player.setProgress"),
                           QStringLiteral("Seek to position (params: progress: 0.0-1.0)"),
                           makeSchema({
                             {QStringLiteral("progress"),
                              QStringLiteral("number"),
                              QStringLiteral("Playback position from 0.0 to 1.0"),
                              {},
                              0.0, 1.0}
  }),
                           &setProgress);

  registry.registerCommand(QStringLiteral("mdf4Player.getStatus"),
                           QStringLiteral("Get player status"),
                           emptySchema(),
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
  if (!params.contains(QStringLiteral("filePath")))
    return CommandResponse::makeMissingParam(id, "filePath");

  const QString file_path = params.value(QStringLiteral("filePath")).toString();
  if (file_path.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath cannot be empty"));
  }

  auto& player = API::handlerContext().mdf4Player;
  player.openFile(file_path);

  QJsonObject result;
  result[QStringLiteral("filePath")] = file_path;
  result[QStringLiteral("isOpen")]   = player.isOpen();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close MDF4 file
 */
API::CommandResponse API::Handlers::MDF4PlayerHandler::close(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& player = API::handlerContext().mdf4Player;
  player.closeFile();

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
  if (!params.contains(QStringLiteral("paused")))
    return CommandResponse::makeMissingParam(id, "paused");

  auto& player = API::handlerContext().mdf4Player;

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
API::CommandResponse API::Handlers::MDF4PlayerHandler::step(const QString& id,
                                                            const QJsonObject& params)
{
  const int delta =
    params.contains(QStringLiteral("delta")) ? params.value(QStringLiteral("delta")).toInt() : 1;

  auto& player = API::handlerContext().mdf4Player;
  if (delta == 0) {
    QJsonObject result;
    result[QStringLiteral("framePosition")] = player.framePosition();
    return CommandResponse::makeSuccess(id, result);
  }

  const long long maxSteps = player.frameCount() > 0 ? player.frameCount() : 0;
  const bool forward       = delta > 0;

  long long steps = static_cast<long long>(delta);
  if (steps < 0)
    steps = -steps;

  if (steps > maxSteps)
    steps = maxSteps;

  for (long long i = 0; i < steps; ++i)
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
  if (!params.contains(QStringLiteral("progress")))
    return CommandResponse::makeMissingParam(id, "progress");

  const double progress = SerialStudio::toDouble(params.value(QStringLiteral("progress")));
  if (progress < 0.0 || progress > 1.0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("progress must be between 0.0 and 1.0"));
  }

  auto& player = API::handlerContext().mdf4Player;
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
API::CommandResponse API::Handlers::MDF4PlayerHandler::getStatus(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& player = API::handlerContext().mdf4Player;

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
