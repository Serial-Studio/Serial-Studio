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

#include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @brief Registers API commands for IO::Drivers::CANBus operations.
 */
class CANBusHandler {
public:
  static void registerCommands();

private:
  static CommandResponse setPluginIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setInterfaceIndex(const QString& id, const QJsonObject& params);
  static CommandResponse setBitrate(const QString& id, const QJsonObject& params);
  static CommandResponse setCanFD(const QString& id, const QJsonObject& params);

  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
  static CommandResponse getPluginList(const QString& id, const QJsonObject& params);
  static CommandResponse getInterfaceList(const QString& id, const QJsonObject& params);
  static CommandResponse getBitrateList(const QString& id, const QJsonObject& params);
  static CommandResponse getInterfaceError(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
