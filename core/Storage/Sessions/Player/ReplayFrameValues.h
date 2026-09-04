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

#  include <QHash>
#  include <QSet>
#  include <QString>
#  include <QVector>
#  include <span>
#  include <vector>

#  include "Sessions/BlockReader.h"
#  include "Sessions/Player/ReplayAlignment.h"

namespace Sessions {

/**
 * @brief One replayed instant: the cell text per dataset unique id, plus the sources those cells
 *        belong to. The two travel together because a frame is injected per source, and deriving
 *        the source set anywhere but where the cells are read is how it goes stale.
 */
struct ReplayRowValues {
  QHash<int, QString> values;
  QSet<int> sources;
};

/**
 * @brief The database-free half of frame synthesis: turning decoded rows into the cells a replayed
 *        frame carries, and scattering them onto a seek window's row grid. Free functions with no
 *        SQL, no pipeline and no singleton, so the selection and gap rules are provable against
 *        synthetic rows.
 */
namespace ReplayFrameValues {

void fillSeekGaps(QVector<double>& values);
[[nodiscard]] QString formatCell(double value);
void applyRow(ReplayRowValues& out, const ReadingRow& row, const ReplayLayout& layout);
void selectRowsAt(ReplayRowValues& out,
                  const std::vector<ReadingRow>& rows,
                  qint64 timestampNs,
                  const ReplayLayout& layout);
void scatterRowsIntoWindow(const std::vector<ReadingRow>& rows,
                           std::span<const qint64> rowTimes,
                           const QHash<int, qint64>& keyByUid,
                           QHash<qint64, QVector<double>>& series);

}  // namespace ReplayFrameValues
}  // namespace Sessions

#endif
