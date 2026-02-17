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
 * @class CSVExportHandler
 * @brief Registers API commands for CSV::Export operations
 *
 * Provides commands for:
 * - csv.export.setEnabled - Enable/disable CSV export
 * - csv.export.close - Close current CSV file
 * - csv.export.getStatus - Query export status
 */
class CSVExportHandler {
public:
  /**
   * @brief Register all CSV Export commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse setEnabled(const QString& id, const QJsonObject& params);
  static CommandResponse close(const QString& id, const QJsonObject& params);

  // Query commands
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
