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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API
{
namespace Handlers
{
/**
 * @class MDF4PlayerHandler
 * @brief Registers API commands for MDF4::Player operations (Pro feature)
 *
 * Provides commands for:
 * - mdf4.player.open - Open MDF4 file
 * - mdf4.player.close - Close MDF4 file
 * - mdf4.player.play - Start playback
 * - mdf4.player.pause - Pause playback
 * - mdf4.player.toggle - Toggle play/pause
 * - mdf4.player.nextFrame - Advance to next frame
 * - mdf4.player.previousFrame - Go to previous frame
 * - mdf4.player.setProgress - Seek to position
 * - mdf4.player.getStatus - Query player status
 */
class MDF4PlayerHandler
{
public:
  /**
   * @brief Register all MDF4 Player commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // File operations
  static CommandResponse open(const QString &id, const QJsonObject &params);
  static CommandResponse close(const QString &id, const QJsonObject &params);

  // Playback control
  static CommandResponse play(const QString &id, const QJsonObject &params);
  static CommandResponse pause(const QString &id, const QJsonObject &params);
  static CommandResponse toggle(const QString &id, const QJsonObject &params);
  static CommandResponse nextFrame(const QString &id,
                                   const QJsonObject &params);
  static CommandResponse previousFrame(const QString &id,
                                       const QJsonObject &params);
  static CommandResponse setProgress(const QString &id,
                                     const QJsonObject &params);

  // Query commands
  static CommandResponse getStatus(const QString &id,
                                   const QJsonObject &params);
};

} // namespace Handlers
} // namespace API

#endif // BUILD_COMMERCIAL
