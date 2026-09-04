/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QMap>
#  include <unordered_map>
#  include <vector>

#  include "Sessions/PlayerLoaderWorker.h"

namespace Sessions {

/**
 * @brief The column geometry of a loaded recording: which datasets its columns carry, in which
 *        order the parser expects them, and which source each one belongs to. A value object
 *        shared by the player, its database reader and its frame synthesis, so the three read one
 *        description of the recording instead of three copies that can drift.
 */
struct ReplayLayout {
  bool multiSource;
  std::vector<int> columnUniqueIds;
  QMap<int, int> uidToColumn;
  QMap<int, int> columnToSource;
  QMap<int, std::vector<int>> sourceColumns;
};

/**
 * @brief Where the loaded project puts one dataset: its source and its position inside it.
 */
struct DatasetLocation {
  int sourceId;
  int index;
};

/**
 * @brief Dataset unique id -> its position in the loaded project.
 */
using DatasetLocationMap = QMap<int, DatasetLocation>;

/**
 * @brief Source id -> (dataset unique id -> payload cell index), the map FrameBuilder replays with.
 */
using ReplayColumnMap = std::unordered_map<int, std::unordered_map<int, int>>;

/**
 * @brief Pure arithmetic that reconciles a recording's stored column order with the project the
 *        dashboard is running. Free functions rather than a class: none of it holds state, and
 *        keeping it free of ProjectModel (the caller flattens the groups into a
 *        DatasetLocationMap) is what makes the alignment rules testable without an app.
 */
namespace ReplayAlignment {

void indexColumns(ReplayLayout& layout);
void alignColumnsToProject(ReplayLayout& layout, const DatasetLocationMap& locations);
void mergeStreamBlockTimes(std::vector<qint64>& timeline,
                           const std::vector<PlayerStreamBlockIndex>& blocks);
[[nodiscard]] ReplayColumnMap buildMultiSourceMapping(ReplayLayout& layout,
                                                      const DatasetLocationMap& locations);

}  // namespace ReplayAlignment
}  // namespace Sessions

#endif
