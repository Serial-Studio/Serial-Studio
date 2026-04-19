/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <algorithm>
#include <QMap>
#include <QString>
#include <vector>

#include "DataModel/Frame.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Export column descriptor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Describes a single exportable dataset column.
 */
struct ExportColumn {
  int uniqueId = 0;
  int groupId  = 0;
  int sourceId = 0;
  QString groupTitle;
  QString title;
  QString units;
  QString widget;
  bool isNumeric = true;
  bool isVirtual = false;
};

//--------------------------------------------------------------------------------------------------
// Export schema
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pre-computed column layout shared by all export workers.
 *
 * Built once from a template Frame when an export file is created. Eliminates
 * duplicated header-derivation logic across CSV, MDF4, and session exporters.
 */
struct ExportSchema {
  QString frameTitle;
  std::vector<ExportColumn> columns;
  QMap<int, int> uniqueIdToColumnIndex;
};

//--------------------------------------------------------------------------------------------------
// Schema builder
//--------------------------------------------------------------------------------------------------

/**
 * @brief Derives an ExportSchema from a Frame, sorted by uniqueId.
 *
 * Iterates groups and datasets, skipping Output groups. Each dataset becomes
 * one ExportColumn. The result is sorted by uniqueId for deterministic column
 * order across all export formats.
 *
 * @param frame  Template or first-data frame.
 * @return Fully populated ExportSchema.
 */
[[nodiscard]] inline ExportSchema buildExportSchema(const Frame& frame)
{
  ExportSchema schema;
  schema.frameTitle = frame.title;

  // Collect columns from all input groups
  for (const auto& group : frame.groups) {
    if (group.groupType == GroupType::Output)
      continue;

    for (const auto& dataset : group.datasets) {
      ExportColumn col;
      col.uniqueId   = dataset.uniqueId;
      col.groupId    = group.groupId;
      col.sourceId   = dataset.sourceId;
      col.groupTitle = group.title;
      col.title      = dataset.title;
      col.units      = dataset.units;
      col.widget     = dataset.widget;
      col.isNumeric  = dataset.isNumeric;
      col.isVirtual  = dataset.virtual_;
      schema.columns.push_back(col);
    }
  }

  // Sort by uniqueId for deterministic column order
  std::sort(schema.columns.begin(),
            schema.columns.end(),
            [](const ExportColumn& a, const ExportColumn& b) { return a.uniqueId < b.uniqueId; });

  // Build reverse index
  for (int i = 0; i < static_cast<int>(schema.columns.size()); ++i)
    schema.uniqueIdToColumnIndex.insert(schema.columns[static_cast<size_t>(i)].uniqueId, i);

  return schema;
}

}  // namespace DataModel
