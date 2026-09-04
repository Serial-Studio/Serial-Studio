/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
namespace Handlers {

/**
 * @brief Stateless half of the remote-dashboard mirror API (spec 0040): what a viewer can ask
 *        without holding a subscription. The connection-scoped commands live in API::Server
 *        because they mutate per-socket state the command registry cannot see.
 */
class MirrorHandler {
public:
  static void registerCommands();

private:
  static CommandResponse getInfo(const QString& id, const QJsonObject& params);
  static CommandResponse getStructure(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
