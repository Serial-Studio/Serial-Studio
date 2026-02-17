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

namespace API {
namespace Handlers {
/**
 * @class ConsoleHandler
 * @brief Registers API commands for Console::Handler operations
 *
 * Provides commands for:
 * - console.setEcho - Enable/disable echo
 * - console.setShowTimestamp - Show/hide timestamps
 * - console.setDisplayMode - Set display mode (PlainText/Hex)
 * - console.setDataMode - Set data mode (UTF8/Hex)
 * - console.setLineEnding - Set line ending
 * - console.setFontFamily - Set font family
 * - console.setFontSize - Set font size
 * - console.setChecksumMethod - Set checksum method
 * - console.clear - Clear console
 * - console.send - Send data
 * - console.export.setEnabled - Enable/disable console export
 * - console.export.close - Close console export file
 * - console.export.getStatus - Get console export status
 * - console.getConfiguration - Get all settings
 */
class ConsoleHandler {
public:
  /**
   * @brief Register all Console commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Settings commands
  static CommandResponse setEcho(const QString& id, const QJsonObject& params);
  static CommandResponse setShowTimestamp(const QString& id, const QJsonObject& params);
  static CommandResponse setDisplayMode(const QString& id, const QJsonObject& params);
  static CommandResponse setDataMode(const QString& id, const QJsonObject& params);
  static CommandResponse setLineEnding(const QString& id, const QJsonObject& params);
  static CommandResponse setFontFamily(const QString& id, const QJsonObject& params);
  static CommandResponse setFontSize(const QString& id, const QJsonObject& params);
  static CommandResponse setChecksumMethod(const QString& id, const QJsonObject& params);

  // Operations
  static CommandResponse clear(const QString& id, const QJsonObject& params);
  static CommandResponse send(const QString& id, const QJsonObject& params);

  // Query commands
  static CommandResponse exportSetEnabled(const QString& id, const QJsonObject& params);
  static CommandResponse exportClose(const QString& id, const QJsonObject& params);
  static CommandResponse exportGetStatus(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
