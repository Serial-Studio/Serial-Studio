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

/**
 * @brief Descriptor for a single column in an exported dataset file.
 */
struct ExportColumn {
  int uniqueId = 0;
  int groupId  = 0;
  int sourceId = 0;
  QString sourceTitle;
  QString groupTitle;
  QString title;
  QString units;
  QString widget;
  bool isNumeric = true;
  bool isVirtual = false;
};

/**
 * @brief Shared column layout used by CSV, MDF4 and session exporters.
 */
struct ExportSchema {
  QString frameTitle;
  std::vector<ExportColumn> columns;
  QMap<int, int> uniqueIdToColumnIndex;
};

[[nodiscard]] inline ExportSchema buildExportSchema(const Frame& frame)
{
  ExportSchema schema;
  schema.frameTitle = frame.title;

  // Source-id -> source-title lookup
  QMap<int, QString> sourceTitles;
  for (const auto& s : frame.sources)
    sourceTitles.insert(s.sourceId, s.title);

  for (const auto& group : frame.groups) {
    if (group.groupType == GroupType::Output)
      continue;

    for (const auto& dataset : group.datasets) {
      ExportColumn col;
      col.uniqueId    = dataset.uniqueId;
      col.groupId     = group.groupId;
      col.sourceId    = dataset.sourceId;
      col.sourceTitle = sourceTitles.value(dataset.sourceId);
      col.groupTitle  = group.title;
      col.title       = dataset.title;
      col.units       = dataset.units;
      col.widget      = dataset.widget;
      col.isNumeric   = dataset.isNumeric;
      col.isVirtual   = dataset.virtual_;
      schema.columns.push_back(col);
    }
  }

  std::sort(schema.columns.begin(),
            schema.columns.end(),
            [](const ExportColumn& a, const ExportColumn& b) { return a.uniqueId < b.uniqueId; });

  for (int i = 0; i < static_cast<int>(schema.columns.size()); ++i)
    schema.uniqueIdToColumnIndex.insert(schema.columns[static_cast<size_t>(i)].uniqueId, i);

  return schema;
}

}  // namespace DataModel
