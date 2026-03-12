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
 * @brief Registers API commands for multi-source project management.
 *
 * Provides 9 commands for source (device) management:
 * - project.source.list             - List all sources (GPL)
 * - project.source.add              - Add a new source (Commercial)
 * - project.source.delete           - Delete a source (Commercial)
 * - project.source.update           - Update source fields (Commercial)
 * - project.source.setProperty      - Set a single driver connection property (GPL)
 * - project.source.configure        - Set multiple driver properties at once (GPL)
 * - project.source.getConfiguration - Get full source configuration (GPL)
 * - project.source.setFrameParserCode - Set per-source JS parser (GPL)
 * - project.source.getFrameParserCode - Get per-source JS parser (GPL)
 */
class SourceHandler {
public:
  /**
   * @brief Register all Source commands with the CommandRegistry.
   */
  static void registerCommands();

private:
  static CommandResponse sourceList(const QString& id, const QJsonObject& params);
  static CommandResponse sourceAdd(const QString& id, const QJsonObject& params);
  static CommandResponse sourceDelete(const QString& id, const QJsonObject& params);
  static CommandResponse sourceUpdate(const QString& id, const QJsonObject& params);
  static CommandResponse sourceConfigure(const QString& id, const QJsonObject& params);
  static CommandResponse sourceSetProperty(const QString& id, const QJsonObject& params);
  static CommandResponse sourceGetConfiguration(const QString& id, const QJsonObject& params);
  static CommandResponse sourceSetFrameParserCode(const QString& id, const QJsonObject& params);
  static CommandResponse sourceGetFrameParserCode(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
