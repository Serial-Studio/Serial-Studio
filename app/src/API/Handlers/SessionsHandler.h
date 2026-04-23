/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {

/**
 * @brief Registers API commands for SQLite session recording (Pro).
 */
class SessionsHandler {
public:
  static void registerCommands();

private:
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
  static CommandResponse setExportEnabled(const QString& id, const QJsonObject& params);
  static CommandResponse close(const QString& id, const QJsonObject& params);
  static CommandResponse getCanonicalDbPath(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
