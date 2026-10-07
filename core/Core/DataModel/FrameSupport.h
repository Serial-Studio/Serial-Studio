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

#include <QMap>
#include <QVector>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"

/**
 * @file FrameSupport.h
 * @brief The SerialStudio helpers that take a frame type: pure logic over Group and Dataset that
 *        the vocabulary header cannot hold because Core does not know the frame (spec 0077).
 */

namespace SerialStudio {
/**
 * @brief Counts of the project content that needs an entitlement to run (spec 0094): datasets
 *        with non-blank transform code and user tables. Both zero means the project is free.
 */
struct ProContentSummary {
  int transforms;
  int tables;
};

[[nodiscard]] bool commercialCfg(const QVector<DataModel::Group>& g);
[[nodiscard]] bool commercialCfg(const std::vector<DataModel::Group>& g);
[[nodiscard]] ProContentSummary proContentSummary(const std::vector<DataModel::Group>& g,
                                                  int tableCount);
[[nodiscard]] bool authorsTransform(const DataModel::Dataset& before,
                                    const DataModel::Dataset& after);
[[nodiscard]] bool groupEligibleForWorkspace(const DataModel::Group& g);
[[nodiscard]] XAxisMode groupXAxisMode(const DataModel::Group& g);
[[nodiscard]] XAxisPolicy resolveXAxisPolicy(const DataModel::Dataset& d,
                                             const QMap<int, DataModel::Dataset>& datasets);
}  // namespace SerialStudio
