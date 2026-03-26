/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @class LicensingHandler
 * @brief Registers API commands for license management operations
 *
 * Provides commands for:
 * - licensing.setLicense       - Set the license key to activate
 * - licensing.activate         - Activate the stored license key
 * - licensing.deactivate       - Deactivate the license on this machine
 * - licensing.validate         - Re-validate the current license with server
 * - licensing.getStatus        - Query current activation status (read-only)
 * - licensing.trial.getStatus  - Query current trial status (read-only)
 * - licensing.trial.enable     - Start a trial period
 * - licensing.guardStatus      - Run all build-time license guards (diagnostic)
 */
class LicensingHandler {
public:
  static void registerCommands();

private:
  static CommandResponse setLicense(const QString& id, const QJsonObject& params);
  static CommandResponse activate(const QString& id, const QJsonObject& params);
  static CommandResponse deactivate(const QString& id, const QJsonObject& params);
  static CommandResponse validate(const QString& id, const QJsonObject& params);
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
  static CommandResponse guardStatus(const QString& id, const QJsonObject& params);
  static CommandResponse trialGetStatus(const QString& id, const QJsonObject& params);
  static CommandResponse trialEnable(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
