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

#include <QByteArrayView>
#include <QString>
#include <unordered_map>

#include "DataModel/DataTable.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder/TransformCompiler.h"

namespace DataModel {

/**
 * @brief The replay half of the dataset apply (spec 0075, R12.8): the per-source uniqueId to
 *        column map a file player installs, and the two cell writers the CSV and MDF4 lanes use.
 *        Non-virtual and called directly, because this runs per dataset per replayed row.
 */
class ReplayIngest {
public:
  /**
   * @brief One replay cell for the typed lane: a borrowed text pointer for string channels, or a
   *        native double (text == nullptr) for numeric channels.
   */
  struct Cell {
    const QString* text;
    double number;
  };

  ReplayIngest(const bool& captureDatasetValues,
               DataTableStore& tableStore,
               TransformEngine* const& exprEngine);

  ReplayIngest(ReplayIngest&&)                 = delete;
  ReplayIngest(const ReplayIngest&)            = delete;
  ReplayIngest& operator=(ReplayIngest&&)      = delete;
  ReplayIngest& operator=(const ReplayIngest&) = delete;

  void setColumnMap(std::unordered_map<int, std::unordered_map<int, int>> map);
  void applySpanValue(Dataset& dataset,
                      const QByteArrayView* cells,
                      qsizetype count,
                      const std::unordered_map<int, int>* columns);
  void applyTypedValue(Dataset& dataset,
                       const Cell* cells,
                       qsizetype count,
                       const std::unordered_map<int, int>* columns);

  [[nodiscard]] const std::unordered_map<int, int>* columnsFor(int sourceId) const;

private:
  void publishDatasetValue(Dataset& dataset);

private:
  // All three bind FrameBuilder members declared above the sub-object, so no address ever moves
  const bool& m_captureDatasetValues;
  DataTableStore& m_tableStore;
  TransformEngine* const& m_exprEngine;

  std::unordered_map<int, std::unordered_map<int, int>> m_replayColumnMap;
};

}  // namespace DataModel
