/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QList>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"

/**
 * @file WidgetResolution.h
 * @brief Resolves a group or dataset to the dashboard widget kinds it produces. The built-in
 *        widget strings decide directly; the extension fallback consults the installed widget
 *        catalog, which spec 0077 phase 2 turns into a retained bus topic so this file stops
 *        naming the Ui layer.
 */

namespace SerialStudio {
[[nodiscard]] DashboardWidget getDashboardWidget(const DataModel::Group& group);
[[nodiscard]] QList<DashboardWidget> getDashboardWidgets(const DataModel::Dataset& dataset);
[[nodiscard]] int extensionGroupWidgetCount(const std::vector<DataModel::Group>& groups);
[[nodiscard]] bool datasetRendersAlarmBands(const DataModel::Dataset& dataset,
                                            const DataModel::Group& group);
[[nodiscard]] bool datasetRaisesBandAlarms(const DataModel::Dataset& dataset,
                                           const DataModel::Group& group);
}  // namespace SerialStudio
