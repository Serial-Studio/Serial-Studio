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

#include "DataModel/FrameBuilder/BuilderSupport.h"

#include <algorithm>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Frame builder support: stateless helpers the facade's parse and sync paths share
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the per-frame cadence carried by a captured chunk, clamped to >=1 ns.
 */
std::chrono::nanoseconds DataModel::BuilderSupport::capturedFrameStep(
  const IO::CapturedDataPtr& data)
{
  if (!data)
    return std::chrono::nanoseconds(1);

  return std::max(std::chrono::nanoseconds(1), data->frameStep);
}

/**
 * @brief Returns whether every dataset of @p frame is table-fed (Computed: no frame index, so the
 *        parse path never carries a value for it). Only such a source may feed the sinks from a
 *        synthetic refresh: one with parsed channels is already recorded on arrival, and would
 *        otherwise duplicate every channel at UI-tick rate under a now() stamp (A11).
 */
bool DataModel::BuilderSupport::frameIsTableFed(const DataModel::Frame& frame)
{
  if (frame.groups.empty())
    return false;

  for (const auto& group : frame.groups)
    for (const auto& dataset : group.datasets)
      if (!dataset.virtual_)
        return false;

  return true;
}

/**
 * @brief Builds the runtime group list from the project, dropping disabled groups and the
 *        disabled datasets of the survivors so frame building never sees them. The editor keeps
 *        the full set; surviving datasets retain their explicit frame index, so no sibling shifts.
 */
std::vector<DataModel::Group> DataModel::BuilderSupport::buildEnabledGroups(
  const std::vector<DataModel::Group>& projectGroups)
{
  std::vector<DataModel::Group> groups;
  groups.reserve(projectGroups.size());

  for (const auto& group : projectGroups) {
    if (!group.enabled)
      continue;

    DataModel::Group runtimeGroup = group;
    std::vector<DataModel::Dataset> datasets;
    datasets.reserve(runtimeGroup.datasets.size());
    for (auto& dataset : runtimeGroup.datasets)
      if (dataset.enabled)
        datasets.push_back(std::move(dataset));

    runtimeGroup.datasets = std::move(datasets);
    groups.push_back(std::move(runtimeGroup));
  }

  return groups;
}
