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

namespace API
{
namespace Handlers
{
/**
 * @class MDF4ExportHandler
 * @brief Registers API commands for MDF4::Export operations
 *
 * Provides commands for:
 * - mdf4.export.setEnabled - Enable/disable MDF4 export
 * - mdf4.export.close - Close current MDF4 file
 * - mdf4.export.getStatus - Query export status
 */
class MDF4ExportHandler
{
public:
  /**
   * @brief Register all MDF4 Export commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Mutation commands
  static CommandResponse setEnabled(const QString &id,
                                    const QJsonObject &params);
  static CommandResponse close(const QString &id, const QJsonObject &params);

  // Query commands
  static CommandResponse getStatus(const QString &id,
                                   const QJsonObject &params);
};

} // namespace Handlers
} // namespace API

#endif // BUILD_COMMERCIAL
