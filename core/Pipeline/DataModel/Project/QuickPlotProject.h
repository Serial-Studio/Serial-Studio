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

#include <QJsonObject>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "DataModel/Scripting/LineFormat.h"

/**
 * @brief Turns a live Quick Plot structure into an editable project (spec 0100 R8/R9): one device
 *        with Quick Plot's framing and the free Built-In template that reproduces the detected line
 *        format (spec 0106 R12), and Quick Plot's own groups rebuilt as plain project groups. Pure
 *        and free of Pro content, so it is unit-testable and the result connects in every edition.
 */
namespace DataModel::QuickPlotProject {

[[nodiscard]] QJsonObject build(const std::vector<Group>& quickPlotGroups,
                                int busType,
                                const QJsonObject& connectionSettings,
                                const LineFormat& format);

}  // namespace DataModel::QuickPlotProject
