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

#include "DataModel/Frame.h"
#include "SerialStudio.h"

/**
 * @brief The canonical dataset layouts of the fixed-shape group widgets (accelerometer, gyroscope,
 *        GPS, 3D plot, canvas): three axis datasets with pinned tags, units, titles and ranges. The
 *        table is a property of the widget type, not of any project entity, so it lives apart from
 *        the entity CRUD that asks for it.
 */
namespace DataModel::FixedLayouts {

[[nodiscard]] bool populateFixedLayoutGroup(Group& grp,
                                            SerialStudio::GroupWidget widget,
                                            int baseIndex);

}  // namespace DataModel::FixedLayouts
