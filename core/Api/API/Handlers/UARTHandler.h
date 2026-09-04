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

#pragma once

#include "API/CommandProtocol.h"

namespace API {
class CommandRegistry;

namespace Handlers {
/**
 * @brief Registers API commands for IO::Drivers::UART operations.
 */
class UARTHandler {
public:
  static void registerCommands();

private:
  static void registerLineSettings(CommandRegistry& registry);

  static CommandResponse setDevice(const QString& id, const QJsonObject& params);
  static CommandResponse setPortIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setBaudRate(const QString& id, const QJsonObject& params);
  static CommandResponse setParity(const QString& id, const QJsonObject& params);
  static CommandResponse setDataBits(const QString& id, const QJsonObject& params);
  static CommandResponse setStopBits(const QString& id, const QJsonObject& params);
  static CommandResponse setFlowControl(const QString& id, const QJsonObject& params);
  static CommandResponse setDtrEnabled(const QString& id, const QJsonObject& params);
  static CommandResponse setAutoReconnect(const QString& id, const QJsonObject& params);

  static CommandResponse getPortList(const QString& id, const QJsonObject& params);
  static CommandResponse getBaudRateList(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
