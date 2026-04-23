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
 * @brief Registers API commands for CSV::Player operations.
 */
class CSVPlayerHandler {
public:
  static void registerCommands();

private:
  static CommandResponse open(const QString& id, const QJsonObject& params);
  static CommandResponse close(const QString& id, const QJsonObject& params);

  static CommandResponse play(const QString& id, const QJsonObject& params);
  static CommandResponse pause(const QString& id, const QJsonObject& params);
  static CommandResponse toggle(const QString& id, const QJsonObject& params);
  static CommandResponse nextFrame(const QString& id, const QJsonObject& params);
  static CommandResponse previousFrame(const QString& id, const QJsonObject& params);
  static CommandResponse setProgress(const QString& id, const QJsonObject& params);

  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
