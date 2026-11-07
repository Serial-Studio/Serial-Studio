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

#include <functional>
#include <QJsonArray>
#include <vector>

#include "Core/DataModel/Frame.h"

/**
 * @file LedPanelGroup.h
 * @brief The one definition of an LED Panel split off a group (spec 0114): the project loader's
 *        legacy migration and every project generator that turns boolean tags into lights call
 *        it, so a migrated file and a generated one have the same shape.
 */

namespace DataModel {
inline constexpr auto kLedPanelWidget = "ledpanel";

[[nodiscard]] Group deriveLedPanelGroup(const Group& source, const std::vector<int>& positions);
void appendGroupWithLedPanel(QJsonArray& out, const Group& group, const std::vector<int>& leds);
void appendGroupWithLedPanel(std::vector<Group>& out, Group group, const std::vector<int>& leds);
[[nodiscard]] std::vector<int> legacyLedPositions(const Group& group, const QJsonArray& rawGroups);
[[nodiscard]] bool insertLegacyLedPanels(std::vector<Group>& groups,
                                         std::vector<Workspace>& workspaces,
                                         const QJsonArray& rawGroups,
                                         const std::function<int()>& allocateUniqueId);
}  // namespace DataModel
