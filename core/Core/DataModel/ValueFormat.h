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

#include <QString>

namespace DataModel {
struct Dataset;
}  // namespace DataModel

/**
 * @file ValueFormat.h
 * @brief The one presentation formatter for dataset values (spec 0108). Labels and notation live
 *        here only: the pipeline never sees them, so every numeric consumer keeps the number.
 *        Pure functions for GUI-thread callers; nothing is cached.
 */

namespace DataModel::ValueFormat {

[[nodiscard]] QString autoNumber(double value, double min, double max);
[[nodiscard]] QString formatNumber(double value, const Dataset& d);
[[nodiscard]] QString labelFor(const Dataset& d, double value);
[[nodiscard]] QString formatValue(const Dataset& d);
[[nodiscard]] QString captionText(const Dataset& d);

}  // namespace DataModel::ValueFormat
