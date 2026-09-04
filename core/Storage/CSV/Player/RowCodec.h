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

#include <QByteArray>
#include <QByteArrayView>
#include <QVarLengthArray>

#include "CSV/PlayerLoaderWorker.h"
#include "DataModel/Scripting/ReplayRowCodec.h"

namespace CSV {

/**
 * @brief Row-level codec for CSV replay: owns the split scratch and the timestamp-column
 *        exclusion that turns one raw file row into the data cells the replay lanes consume.
 *        Cell views point into the caller's bytes (the player's mapping) and stay valid until
 *        the next split, so an owner must keep the codec alive for as long as those views are.
 */
class RowCodec {
public:
  explicit RowCodec();

  [[nodiscard]] char separator() const;
  [[nodiscard]] int timestampColumn() const;
  [[nodiscard]] qsizetype dataSpanCount() const;
  [[nodiscard]] int dataColumnToFileColumn(int i) const;
  [[nodiscard]] const QByteArrayView* dataSpans() const;
  [[nodiscard]] PlayerTimestampMode timestampMode() const;
  [[nodiscard]] const DataModel::ReplayCellViews& cells() const;
  [[nodiscard]] QByteArray quickPlotPayload(QByteArrayView row);

  void reset();
  void splitCells(QByteArrayView row);
  void setSeparator(char separator);
  void setTimestampMode(PlayerTimestampMode mode, int column);
  [[nodiscard]] qsizetype splitDataCells(QByteArrayView row);

private:
  char m_separator;
  int m_timestampColumn;
  PlayerTimestampMode m_mode;

  DataModel::ReplayCellViews m_cells;
  QByteArray m_splitScratch;
  QVarLengthArray<QByteArrayView, 64> m_dataSpans;
};

}  // namespace CSV
