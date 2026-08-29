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
#include <QList>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QVarLengthArray>

namespace DataModel {

/**
 * @brief RFC-4180 row codec for synthesized replay rows (CSV / MDF4 / session players). Kept apart
 *        from the parser pipeline because no script engine is involved: a replay row is written by
 *        an exporter and read back by a player, so the quote, trim and injection-guard semantics
 *        of the two halves are one contract that has to be verified as a pair.
 */

/**
 * @brief Joins replay cells into one comma-separated row, RFC-4180-quoting any cell that
 *        contains a comma, quote or newline. Counterpart of splitReplayRow.
 */
[[nodiscard]] QByteArray joinReplayRow(const QStringList& cells);

/**
 * @brief Quote-aware comma split of one synthesized replay row (RFC-4180 double-quote escape).
 */
[[nodiscard]] QStringList splitReplayRow(QStringView row);

/**
 * @brief Reusable per-cell view buffer for splitReplayRowSpans (stack storage for the
 *        common column counts).
 */
using ReplayCellViews = QVarLengthArray<QByteArrayView, 64>;

/**
 * @brief Byte-level twin of splitReplayRow for disk-backed replay rows: identical quote,
 *        trim and injection-guard semantics, but cells come back as views into @p row (or
 *        into @p scratch for cells that need RFC-4180 unescaping). A trailing CR is chopped
 *        (QTextStream did that upstream); trimming covers ASCII whitespace, the one
 *        documented divergence from QString::trimmed for exotic Unicode spaces. Views stay
 *        valid while @p row's bytes and @p scratch are alive and untouched; scratch-backed
 *        cells are resolved after the final append, so they survive a scratch reallocation.
 *        @p separator swaps the cell boundary (CSV player sniffing, spec 0048); quoting,
 *        trimming and guard semantics are separator-independent.
 */
void splitReplayRowSpans(QByteArrayView row,
                         ReplayCellViews& out,
                         QByteArray& scratch,
                         char separator = ',');

/**
 * @brief Replay twin of splitQuickPlotChannels: one quote-aware row per non-empty line.
 */
void splitReplayChannels(const QByteArray& rawFrame, QList<QStringList>& outChannels);

}  // namespace DataModel
