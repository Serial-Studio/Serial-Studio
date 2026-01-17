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

#pragma once

#include "API/CommandProtocol.h"

namespace API
{
namespace Handlers
{
/**
 * @class IOManagerHandler
 * @brief Registers API commands for IO::Manager operations
 *
 * Provides commands for:
 * - io.manager.connect - Connect to configured device
 * - io.manager.disconnect - Disconnect from device
 * - io.manager.setPaused - Pause/resume data streaming
 * - io.manager.setBusType - Set bus type (UART, Network, BLE)
 * - io.manager.writeData - Write raw data to device
 * - io.manager.getStatus - Query connection status
 * - io.manager.getAvailableBuses - Query available bus types
 */
class IOManagerHandler
{
public:
  /**
   * @brief Register all IO::Manager commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Command handlers
  static CommandResponse connect(const QString &id, const QJsonObject &params);
  static CommandResponse disconnect(const QString &id,
                                    const QJsonObject &params);
  static CommandResponse setPaused(const QString &id, const QJsonObject &params);
  static CommandResponse setBusType(const QString &id,
                                    const QJsonObject &params);
  static CommandResponse writeData(const QString &id, const QJsonObject &params);
  static CommandResponse getStatus(const QString &id, const QJsonObject &params);
  static CommandResponse getAvailableBuses(const QString &id,
                                           const QJsonObject &params);
};

} // namespace Handlers
} // namespace API

